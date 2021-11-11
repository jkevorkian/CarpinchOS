#include "memoria.h"

int main(void) {
	logger = iniciar_logger();
	config = iniciar_config();
	lista_carpinchos = list_create();

	/* if(!iniciar_memoria(config)) {
		log_info(logger, "FALLO EN EL ARCHIVO DE CONFIGURACIÓN");
		exit(1);
	}*/

	signal(SIGUSR1, &signal_handler_1);
	signal(SIGUSR2, &signal_handler_2);
	signal(SIGINT,  &signal_handler_3);

	iniciar_swap(config_get_string_value(config, "IP_SWAP"), config_get_string_value(config, "PUERTO_SWAP"));
	iniciar_servidor(config_get_string_value(config, "IP"), config_get_int_value(config, "PUERTO"));
	exit(1);
}

uint32_t cant_frames_necesarios(uint32_t tamanio) {
	div_t nro_frames = div(tamanio, config_memoria.tamanio_pagina); 
	uint32_t nro_frames_q = nro_frames.quot;
	if(nro_frames.rem > 0) nro_frames_q++;
	return nro_frames_q;
}

uint32_t mem_alloc(t_carpincho* carpincho, uint32_t tamanio) {
	log_info(logger, "El proceso #%d solicito %d bytes de memoria.", carpincho->id, tamanio);
	uint32_t nro_frames_asignados = 0;
	uint32_t nro_frames_necesarios = cant_frames_necesarios(tamanio);
	uint32_t dir_logica = 0;

    while(nro_frames_necesarios > nro_frames_asignados) {
		t_marco* marco = obtener_marco_libre();
		/* 
		if(marco == NULL) { 
			nuevo_marco = pedir_frame_swap(); 
			if(nuevo_marco == NULL) { 
				log_info(logger, "No se pudo asignar memoria");
				return NULL;
			}
		}; 
		*/

		t_entrada_tp* nueva_pagina = crear_nueva_pagina(marco->nro_real);
		// list_add(carpincho->tabla_paginas, nueva_pagina);

		// log_info(logger, "Asigno frame. Cant frames del carpincho #%d: %d", carpincho->id, list_size(carpincho->tabla_paginas));
		log_info(logger, "Datos pagina. Marco:%d P:%d M:%d U:%d", nueva_pagina->nro_marco,nueva_pagina->presencia,nueva_pagina->modificado,nueva_pagina->uso);
		
		nro_frames_asignados++;
	}

    t_heap_metadata* metadata = buscar_alloc_libre(carpincho->id);

    dir_logica = metadata->alloc_prev + sizeof(t_heap_metadata); 
	log_info(logger, "Direccion logica asignada: %d", dir_logica);
	return dir_logica;
}

t_heap_metadata* buscar_alloc_libre(uint32_t carpincho_id){
/* if(tam < heap_actual->alloc_sig - sizeof(t_heap_metadata)) {

    t_heap_metadata* nuevo_heap = malloc(sizeof(t_heap_metadata));
    heap_actual->alloc_sig = sizeof(t_heap_metadata) + tam;
    nuevo_heap->alloc_prev = heap_actual->alloc_prev;
    nuevo_heap->alloc_sig = 0;
    nuevo_heap->libre = true;

    log_info(logger, "Heap metadata: |%d|%d|%d|%d|", heap_actual->alloc_prev, heap_actual->alloc_sig, nuevo_heap->alloc_prev, nuevo_heap->alloc_sig);
}
 return heap_actual; 
*/
	return NULL;
}

t_entrada_tp* crear_nueva_pagina(uint32_t nro_marco){
		t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));
		
		pagina->nro_marco = nro_marco;
		pagina->presencia = true; 
		pagina->modificado = false;
        pagina->uso = true;

		return pagina;
}


bool iniciar_memoria(t_config* config){
	config_memoria.tamanio_memoria = config_get_int_value(config, "TAMANIO");
	config_memoria.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");

bool iniciar_memoria(t_config* config){
	config_memoria.tamanio_memoria = config_get_int_value(config, "TAMANIO");
	config_memoria.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
	
	if(config_memoria.tamanio_memoria % config_memoria.tamanio_pagina > 0) {
        log_error(logger, "Hubo un error al crear la memoria.");
        return false;
    }

    memoria_ram.inicio = (void*) malloc(config_memoria.tamanio_memoria);   
    if (memoria_ram.inicio == NULL) {
        log_error(logger, "Hubo un error al crear la memoria.");
        return false;
    }

	memset(memoria_ram.inicio, 0, config_memoria.tamanio_memoria);

	config_memoria.cant_marcos = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;
	memoria_ram.mapa_fisico = calloc(config_memoria.cant_marcos, config_memoria.tamanio_pagina);
	
	log_info(logger, "Estoy en paginación, con entrada valida. Nro marcos: %d. Tamaño marco: %d bytes", config_memoria.cant_marcos, config_memoria.tamanio_pagina);
	
	char * algoritmo_reemplazo_mmu = config_get_string_value(config, "ALGORITMO_REEMPLAZO_MMU");
	if(!strcmp(algoritmo_reemplazo_mmu, "LRU"))
		config_memoria.algoritmo_reemplazo = LRU;
	if(!strcmp(algoritmo_reemplazo_mmu, "CLOCK-M")) {
		config_memoria.algoritmo_reemplazo = CLOCK;
		memoria_ram.puntero_clock = 0;
	}

	char * tipo_asignacion = config_get_string_value(config, "TIPO_ASIGNACION");
	if(!strcmp(tipo_asignacion, "FIJA"))
		config_memoria.tipo_asignacion = FIJA_LOCAL;
	if(!strcmp(tipo_asignacion, "DINAMICA")) {
		config_memoria.tipo_asignacion = DINAMICA_GLOBAL;
	}

	iniciar_marcos(config_memoria.cant_marcos);
	//iniciar_tlb(config);  

	return true;
}

void iniciar_marcos(uint32_t cant_marcos){
	for(int i = 0; i < cant_marcos; i++) {
		t_marco* marco_auxiliar = malloc(sizeof(t_marco));
		memoria_ram.mapa_fisico[i] = marco_auxiliar;

		marco_auxiliar->libre = true;
		marco_auxiliar->nro_real = i;
	}
}

t_entrada_tp* crear_nueva_pagina(uint32_t nro_marco, t_carpincho* carpincho){
	t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));
	list_add(carpincho->tabla_paginas, pagina);

	pagina->nro_marco = nro_marco;
	pagina->presencia = true;
	pagina->modificado = false;
	pagina->uso = true;

	log_info(logger, "Asigno frame. Cant marcos del carpincho #%d: %d", carpincho->id, list_size(carpincho->tabla_paginas));
	log_info(logger, "Datos pagina. Marco:%d P:%d M:%d U:%d", pagina->nro_marco,pagina->presencia,pagina->modificado,pagina->uso);

	return pagina;
}

uint32_t cant_frames_necesarios(uint32_t tamanio) {
	div_t nro_frames = div(tamanio, config_memoria.tamanio_pagina);
	uint32_t nro_frames_q = nro_frames.quot;
	if(nro_frames.rem > 0) nro_frames_q++;
	return nro_frames_q;
}

uint32_t pagina_segun_posicion(uint32_t posicion) {
	div_t div_posicion = div(posicion, config_memoria.tamanio_pagina);
	return div_posicion.quot;
}

uint32_t offset_segun_posicion(uint32_t posicion) {
	div_t div_posicion = div(posicion, config_memoria.tamanio_pagina);
	return div_posicion.rem;
}

t_carpincho *carpincho_de_lista(uint32_t id_carpincho) {
	bool mi_carpincho(void *un_carpincho) {
		if(((t_carpincho *)un_carpincho)->id == id_carpincho)
			return true;
		else
			return false;
	}

	return list_find(lista_carpinchos, mi_carpincho);
}

void *inicio_memoria(uint32_t nro_marco, uint32_t offset) {
	return memoria_ram.inicio + nro_marco * config_memoria.tamanio_pagina + offset;
}