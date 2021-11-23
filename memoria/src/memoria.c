#include "memoria.h"
#include "tlb.h"

bool iniciar_memoria(t_config* config) {
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
		// memoria_ram.puntero_clock = 0;
	}

	char * tipo_asignacion = config_get_string_value(config, "TIPO_ASIGNACION");
	if(!strcmp(tipo_asignacion, "FIJA"))
		config_memoria.tipo_asignacion = FIJA_LOCAL;
	if(!strcmp(tipo_asignacion, "DINAMICA")) {
		config_memoria.tipo_asignacion = DINAMICA_GLOBAL;
	}

	pthread_mutex_init(&mutex_asignacion_marcos, NULL);

	iniciar_marcos(config_memoria.cant_marcos);
	iniciar_tlb();  

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

void* dir_fisica_proceso(t_list* tabla_paginas) {
    t_entrada_tp* pagina = (t_entrada_tp*) list_get(tabla_paginas, 0);
	// TODO: obtener marco
    return memoria_ram.inicio + pagina->nro_marco * config_memoria.tamanio_pagina;;
}
