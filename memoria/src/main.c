#include "main.h"
#include "tlb.h"

int main(void) {
	t_config *config = config_create("memoria.config");

	if(!iniciar_memoria(config)) {
		log_info(logger, "FALLO EN EL ARCHIVO DE CONFIGURACION");
		exit(1);
	}

	log_info(logger, "Memoria iniciada correctamente. Nro marcos: %d. Tamanio marco: %d bytes",
		config_memoria.cant_marcos, config_memoria.tamanio_pagina);

	signal(SIGUSR1, &signal_handler_1);
	signal(SIGUSR2, &signal_handler_2);
	signal(SIGINT,  &signal_handler_3);

	// char* ip;

	iniciar_swap(config_get_string_value(config, "IP_SWAP"), config_get_string_value(config, "PUERTO_SWAP"));
	
	// ip = config_get_string_value(config, "IP");
	// int puerto_serv = config_get_int_value(config, "PUERTO");
	// iniciar_servidor(ip, puerto_serv);
	iniciar_servidor(config_get_string_value(config, "IP"), config_get_int_value(config, "PUERTO"));
	// ip = config_get_string_value(config, "IP_SWAP");
	// char* puerto = config_get_string_value(config, "PUERTO_SWAP");
	// iniciar_swap(ip, puerto);
	config_destroy(config);
	
	// log_info(logger, "Voy a cancelar hilo");
	// pthread_cancel(nuevo_carpincho);
	// terminar_programa(logger, config);
	// exit(1);
	return 0;
}

void signal_handler_1(int sig) {	// SIGUSR1
	print_tlb();
}

void signal_handler_2(int sig) {	// SIGUSR2
	resetear_tlb();
}

void signal_handler_3(int sig) {	// SIGINT
	// print_hit_miss();
	exit(1);
}

void terminar_programa(t_log* logger, t_config* config) {
	if (logger != NULL)
		log_destroy(logger);
}
