#include "memoria.h"

int main(void) {
	logger = iniciar_logger();
	config = iniciar_config();
	lista_carpinchos = list_create();

	if(!iniciar_memoria(config)) {
		log_info(logger, "FALLO EN EL ARCHIVO DE CONFIGURACIÓN");
		exit(1);
	}

	signal(SIGUSR1, &signal_handler_1);
	signal(SIGUSR2, &signal_handler_2);
	signal(SIGINT,  &signal_handler_3);

	//iniciar_swap(config_get_string_value(config, "IP_SWAP"), config_get_string_value(config, "PUERTO_SWAP"));
	//iniciar_servidor(config_get_string_value(config, "IP"), config_get_int_value(config, "PUERTO"));

	terminar_programa(logger, config);
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
		t_heap_metadata* metadata = malloc(sizeof(t_heap_metadata));
		uint32_t nro_marco = obtener_marco_libre(tamanio, metadata);
		/* 
		if(nuevo_marco == -1) { 
			nuevo_marco = pedir_frame_swap(); 
			if(nuevo_marco == NULL) { 
				log_info(logger, "No se pudo asignar memoria");
				return NULL;
			}
		}; 
		*/

		t_entrada_tp* nueva_pagina = crear_nueva_pagina(nro_marco);
		list_add(carpincho->tabla_paginas, nueva_pagina);

		log_info(logger, "Asigno frame. Cant frames del carpincho #%d: %d", carpincho->id, list_size(carpincho->tabla_paginas));
		log_info(logger, "Datos pagina. Marco:%d P:%d M:%d U:%d", nueva_pagina->nro_marco,nueva_pagina->presencia,nueva_pagina->modificado,nueva_pagina->uso);
		
		//devuelvo la direccion logica del alloc, sin contar el heap metadata
		if(nro_frames_asignados == 0) dir_logica = metadata->alloc_prev + sizeof(t_heap_metadata); 
		nro_frames_asignados++;
	}

	log_info(logger, "Direccion logica asignada: %d", dir_logica);
	return dir_logica;
}

t_entrada_tp* crear_nueva_pagina(uint32_t nro_marco){
		t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));
		
		pagina->nro_marco = nro_marco;
		pagina->presencia = true; 
		pagina->modificado = false;
        pagina->uso = true;

		return pagina;
}

uint32_t obtener_marco_libre(uint32_t tam, t_heap_metadata* metadata) {
    for(int i = 0; i < config_memoria.cant_marcos; i++) {
		//while(recorro todos los heap de la pag) {
		t_heap_metadata* heap_actual = memoria_ram.mapa_fisico[i];
		if (heap_actual->libre) {
			heap_actual->libre = false; //mutex
			if(tam < heap_actual->alloc_sig - sizeof(t_heap_metadata)) {

				t_heap_metadata* nuevo_heap = malloc(sizeof(t_heap_metadata));
				heap_actual->alloc_sig = sizeof(t_heap_metadata) + tam;
				nuevo_heap->alloc_prev = heap_actual->alloc_prev;
				nuevo_heap->alloc_sig = 0;
				nuevo_heap->libre = true;

				log_info(logger, "Heap metadata: |%d|%d|%d|%d|", heap_actual->alloc_prev, heap_actual->alloc_sig, nuevo_heap->alloc_prev, nuevo_heap->alloc_sig);
			}
			*metadata = *heap_actual;
			return i; 
		}
    }
	return -1;
}


bool iniciar_memoria(t_config* config){
	config_memoria.tamanio_memoria = config_get_int_value(config, "TAMANIO");
	config_memoria.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");

	config_memoria.cant_marcos = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;

	if(config_memoria.tamanio_memoria % config_memoria.tamanio_pagina > 0)
		return false;
	
	log_info(logger, "Estoy en paginación, con entrada valida. Nro marcos: %d. Tamaño marco: %d bytes", config_memoria.cant_marcos, config_memoria.tamanio_pagina);
	
	memoria_ram.mapa_fisico = calloc(config_memoria.cant_marcos, config_memoria.tamanio_pagina);

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
		t_heap_metadata* heap = malloc(sizeof(t_heap_metadata));
		memoria_ram.mapa_fisico[i] = heap;

		heap->libre = true;
		heap->alloc_prev = 0;
		heap->alloc_sig = 0;
	}
}

void signal_handler_1(int sig) {

}

void signal_handler_2(int sig) {

}

void signal_handler_3(int sig) {

}

t_log* iniciar_logger(void) {
	t_log* nuevo_logger;

	nuevo_logger = log_create("memoria.log", "memoria", 1, LOG_LEVEL_DEBUG);
	if (nuevo_logger == NULL)
		printf("Falla en la creación del Logger");

	return nuevo_logger;
}

t_config* iniciar_config(void) {
	t_config* nuevo_config;

	nuevo_config = config_create("memoria.config");

	return nuevo_config;
}

void terminar_programa(t_log* logger, t_config* config) {
	if (logger != NULL)
		log_destroy(logger);
	if (config != NULL)
		config_destroy(config);

}