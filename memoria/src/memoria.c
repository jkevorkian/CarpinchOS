#include "memoria.h"
#include "tlb.h"

void iniciar_marcos(uint32_t);

bool iniciar_memoria(t_config *config) {
	logger = log_create("memoria.log", "memoria", 1, LOG_LEVEL_DEBUG);

	if(config == NULL) {
		return false;
	}
	
	config_memoria.tamanio_memoria = config_get_int_value(config, "TAMANIO");
	config_memoria.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
	config_memoria.cant_marcos_carpincho = config_get_int_value(config, "MARCOS_POR_CARPINCHO");
	
	if(config_memoria.tamanio_memoria % config_memoria.tamanio_pagina > 0) {
        return false;
    }

	config_memoria.cant_marcos = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;
	memoria_ram.mapa_fisico = calloc(config_memoria.cant_marcos, config_memoria.tamanio_pagina);

    memoria_ram.inicio = (void*) malloc(config_memoria.tamanio_memoria);

	memset(memoria_ram.inicio, 0, config_memoria.tamanio_memoria);
	
	char * algoritmo_reemplazo_mmu = config_get_string_value(config, "ALGORITMO_REEMPLAZO_MMU");
	if(!strcmp(algoritmo_reemplazo_mmu, "LRU")) {
		log_info(logger, "Algoritmo de reemplazo MMU: LRU");
		config_memoria.algoritmo_reemplazo = LRU;
	}
	else {
		log_info(logger, "Algoritmo de reemplazo MMU: Clock modificado");
		config_memoria.algoritmo_reemplazo = CLOCK;
		memoria_ram.puntero_clock = 0;
	}
	free(algoritmo_reemplazo_mmu);

	char * tipo_asignacion = config_get_string_value(config, "TIPO_ASIGNACION");
	if(!strcmp(tipo_asignacion, "FIJA")) {
		log_info(logger, "Tipo de asignacion de marcos: Fija y Local");
		config_memoria.tipo_asignacion = FIJA_LOCAL;
	}
	else {
		log_info(logger, "Tipo de asignacion de marcos: Dinamica y global");
		config_memoria.tipo_asignacion = DINAMICA_GLOBAL;
	}
	free(tipo_asignacion);

	iniciar_marcos(config_memoria.cant_marcos);
	iniciar_tlb(config);

	lista_carpinchos = list_create();
	pthread_mutex_init(&mutex_lista_carpinchos, NULL);

	return true;
}

void iniciar_marcos(uint32_t cant_marcos){
	for(int i = 0; i < cant_marcos; i++) {
		t_marco* marco_auxiliar = malloc(sizeof(t_marco));
		memoria_ram.mapa_fisico[i] = marco_auxiliar;
		marco_auxiliar->duenio = 0;
		marco_auxiliar->nro_real = i;
		marco_auxiliar->bit_uso = false;
		marco_auxiliar->bit_modificado = false;
		marco_auxiliar->temporal = NULL;
		pthread_mutex_init(&marco_auxiliar->mutex_espera_uso, NULL);
		pthread_mutex_init(&marco_auxiliar->mutex_info_algoritmo, NULL);
	}
	pthread_mutex_init(&mutex_asignacion_marcos, NULL);
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

void loggear_pagina(t_log *logger, void *pagina) {
	return;
	
	if(config_memoria.tamanio_pagina != 32) {
		log_info(logger, "No puedo loggear paginas de tamanio distinto de 32");
		return;
	}
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
	for(int i = 0; i < config_memoria.cant_marcos; i++) {
		if(memoria_ram.mapa_fisico[i]->duenio == id_carpincho) {
			marcos_proceso[nro_marcos] = memoria_ram.mapa_fisico[i];
			nro_marcos++;
		}
		if(nro_marcos == nro_marcos_proceso)
			break;
	}
	if(nro_marcos < nro_marcos_proceso)
		marcos_proceso = realloc(marcos_proceso, sizeof(t_marco *) * nro_marcos);
	if(nro_marcos_encontrados)
		*nro_marcos_encontrados = nro_marcos;
	return marcos_proceso;
}

uint32_t nro_paginas_reemplazo() {		// Mmm, no deberia ir aca esto. O si, no si, que se yo.
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)
		return config_memoria.cant_marcos_carpincho;
	else
		return config_memoria.cant_marcos;
}

t_entrada_tp *pagina_de_carpincho(uint32_t id, uint32_t nro_pagina) {
	if(nro_pagina == -1) {
		return NULL;
	}

	t_carpincho *carpincho = carpincho_de_lista(id);
	if(carpincho) {
		pthread_mutex_lock(&carpincho->mutex_tabla);
		t_entrada_tp *entrada = (t_entrada_tp *)list_get(carpincho->tabla_paginas, nro_pagina);
		pthread_mutex_unlock(&carpincho->mutex_tabla);

		pthread_mutex_lock(&entrada->mutex);
		return entrada;
	}
	else
		return NULL;
}

void print_marcos() {
	log_info(logger, "Loggeo marcos de la memoria");
	pthread_mutex_lock(&mutex_asignacion_marcos);
	for(int i = 0; i < config_memoria.cant_marcos; i++) {
		log_info(logger, "Marco %d, Id %d, Nro. Pagina %d",
			memoria_ram.mapa_fisico[i]->nro_real,
			memoria_ram.mapa_fisico[i]->duenio,
			memoria_ram.mapa_fisico[i]->pagina_duenio
			);
	}
	pthread_mutex_unlock(&mutex_asignacion_marcos);
}

void print_marcos_clock() {
	log_info(logger, "Loggeo marcos de la memoria");
	for(int i = 0; i < config_memoria.cant_marcos; i++) {
		log_info(logger, "Marco %d. Id %d. Nro. pagina %d. Modif: %d. Uso: %d",
			memoria_ram.mapa_fisico[i]->nro_real,
			memoria_ram.mapa_fisico[i]->duenio,
			memoria_ram.mapa_fisico[i]->pagina_duenio,
			memoria_ram.mapa_fisico[i]->bit_modificado,
			memoria_ram.mapa_fisico[i]->bit_uso
			);
	}
}

void loggear_data(void *data, uint32_t tamanio) {
	// log_info(logger, "Contenido data:");
	printf("Contenido data: ");
	uint8_t byte;
	for(int i = 0; i < tamanio; i++) {
		memcpy(&byte, data + i, 1);
		printf("%3d|", byte);
		// log_info(logger, "\t\t%d", byte);
	}
	printf("\n");
}