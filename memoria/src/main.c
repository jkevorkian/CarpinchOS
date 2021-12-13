#include "main.h"

int main(int argc, char *argv[]) {
	char* direccion_config;

	if(argc < 2)
		direccion_config = "memoria.config";
	else {
		direccion_config = argv[1];
	}
	t_config *config = config_create(direccion_config);

	if(!iniciar_memoria(config)) {
		log_info(logger, "FALLO EN EL ARCHIVO DE CONFIGURACION");
		exit(1);
	}

	log_info(logger, "Memoria iniciada correctamente. Nro marcos: %d. Tamanio marco: %d bytes",
		config_memoria.cant_marcos, config_memoria.tamanio_pagina);

	signal(SIGUSR1, &signal_handler_1);
	signal(SIGUSR2, &signal_handler_2);
	signal(SIGINT,  &signal_handler_3);

	if(!iniciar_swap(config_get_string_value(config, "IP_SWAP"), config_get_string_value(config, "PUERTO_SWAP")))
		return 0;
	
	iniciar_servidor(config_get_string_value(config, "IP"), config_get_int_value(config, "PUERTO"));
	config_destroy(config);
	terminar_programa(config);
	// exit(1);
	return 0;
}

void signal_handler_1(int sig) {	// SIGUSR1
	// print_tlb();
	print_marcos();
}

void signal_handler_2(int sig) {	// SIGUSR2
	resetear_tlb();
}

void signal_handler_3(int sig) {	// SIGINT
	print_hit_miss();
	exit(1);
}

void terminar_programa(t_config* config) {
	if (logger != NULL)
		log_destroy(logger);
}
