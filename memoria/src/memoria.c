#include "memoria.h"
#include "tlb.h"

bool iniciar_memoria(t_config* config) {
	config_memoria.tamanio_memoria = config_get_int_value(config, "TAMANIO");
	config_memoria.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
	config_memoria.cant_marcos_carpincho = config_get_int_value(config, "MARCOS_POR_CARPINCHO");
	
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
	iniciar_tlb(config);  

	return true;
}

void iniciar_marcos(uint32_t cant_marcos){
	for(int i = 0; i < cant_marcos; i++) {
		t_marco* marco_auxiliar = malloc(sizeof(t_marco));
		memoria_ram.mapa_fisico[i] = marco_auxiliar;

		marco_auxiliar->libre = true;
		marco_auxiliar->nro_real = i;
		marco_auxiliar->bit_uso = false;
		marco_auxiliar->bit_modificado = false;
		marco_auxiliar->temporal = NULL;
		pthread_mutex_init(&marco_auxiliar->mutex_espera_uso, NULL);
		pthread_mutex_init(&marco_auxiliar->mutex_info_algoritmo, NULL);
	}
}

uint32_t cant_frames_necesarios(uint32_t tamanio) {
	div_t nro_frames = div(tamanio, config_memoria.tamanio_pagina);
	uint32_t nro_frames_q = nro_frames.quot;
	if(nro_frames.rem > 0) nro_frames_q++;
	return nro_frames_q;
}

t_carpincho *carpincho_de_lista(uint32_t id_carpincho) {
	t_carpincho *carpincho;
	
	bool mi_carpincho(void *un_carpincho) {
		if(((t_carpincho *)un_carpincho)->id == id_carpincho)
			return true;
		else
			return false;
	}

	pthread_mutex_lock(&mutex_lista_carpinchos);
	carpincho = list_find(lista_carpinchos, mi_carpincho);
	pthread_mutex_unlock(&mutex_lista_carpinchos);

	return carpincho;
}

void *inicio_memoria(uint32_t nro_marco, uint32_t offset) {
	return memoria_ram.inicio + nro_marco * config_memoria.tamanio_pagina + offset;
}

void* dir_fisica_proceso(t_list* tabla_paginas) {
    t_entrada_tp* pagina = (t_entrada_tp*) list_get(tabla_paginas, 0);
	// TODO: obtener marco
    return memoria_ram.inicio + pagina->nro_marco * config_memoria.tamanio_pagina;;
}

void loggear_pagina(t_log *logger, void *pagina) {
	uint8_t byte;
	for(int i = 0; i < 32; i++) {
		memcpy(&byte, pagina + i, 1);
		printf("%3d|", byte);

		div_t barra_n = div(i + 1, 4);
		if(i > 0 && !barra_n.rem)
			printf("\n");
	}
}

t_marco** obtener_marcos_proceso(uint32_t id_carpincho, uint32_t *nro_marcos_encontrados) {
	uint32_t nro_marcos_proceso = nro_paginas_reemplazo();
	t_marco **marcos_proceso = calloc(nro_marcos_proceso, sizeof(t_marco *));

	uint32_t nro_marcos = 0;
	for(int i = 0; i < nro_marcos_proceso; i++) {
		if(memoria_ram.mapa_fisico[i]->duenio == id_carpincho) {
			marcos_proceso[nro_marcos] = memoria_ram.mapa_fisico[i];
			nro_marcos++;
		}
	}
	if(nro_marcos < nro_marcos_proceso)
		marcos_proceso = realloc(marcos_proceso, sizeof(t_marco *) * nro_marcos);
	if(nro_marcos_encontrados)
		*nro_marcos_encontrados = nro_marcos;
	return marcos_proceso;
}

uint32_t nro_paginas_reemplazo() {
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)
		return config_memoria.cant_marcos_carpincho;
	else
		return config_memoria.cant_marcos;
}