#include "memoria.h"

int main(void) {
	logger = iniciar_logger();
	config = iniciar_config();

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

bool iniciar_memoria(t_config* config){
	memoria_ram.tamanio_memoria = config_get_int_value(config, "TAMANIO");
	memoria_ram.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");

	uint32_t cant_marcos = memoria_ram.tamanio_memoria / memoria_ram.tamanio_pagina;

	if(memoria_ram.tamanio_memoria % memoria_ram.tamanio_pagina > 0)
		return false;
	
	log_info(logger, "Estoy en paginación, con entrada valida. Nro marcos: %d", cant_marcos);
	
	memoria_ram.mapa_fisico = calloc(cant_marcos, memoria_ram.tamanio_pagina);

	char * algoritmo_reemplazo_mmu = config_get_string_value(config, "ALGORITMO_REEMPLAZO_MMU");
	if(!strcmp(algoritmo_reemplazo_mmu, "LRU"))
		memoria_ram.algoritmo_reemplazo = LRU;
	if(!strcmp(algoritmo_reemplazo_mmu, "CLOCK-M")) {
		memoria_ram.algoritmo_reemplazo = CLOCK;
		memoria_ram.puntero_clock = 0;
	}

	char * tipo_asignacion = config_get_string_value(config, "TIPO_ASIGNACION");
	if(!strcmp(tipo_asignacion, "FIJA"))
		memoria_ram.tipo_asignacion = FIJA_LOCAL;
	if(!strcmp(tipo_asignacion, "DINAMICA")) {
		memoria_ram.tipo_asignacion = DINAMICA_GLOBAL;
	}

	iniciar_marcos(cant_marcos);
	//iniciar_tlb(config);  

	return true;
}

void iniciar_marcos(uint32_t cant_marcos){
	for(int i = 0; i < cant_marcos; i++) {
		t_marco* marco_auxiliar = malloc(sizeof(t_marco));

		memoria_ram.mapa_fisico[i] = marco_auxiliar;
		
		marco_auxiliar->nro_real = i;
		marco_auxiliar->presencia = true;
		marco_auxiliar->id_proceso = 0;
		marco_auxiliar->modificado = false;
        marco_auxiliar->uso = false;
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