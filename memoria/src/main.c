#include "main.h"

int main(void) {
	logger = iniciar_logger();
	config = iniciar_config();
	lista_carpinchos = list_create();

	if(!iniciar_memoria(config)) {
		log_info(logger, "FALLO EN EL ARCHIVO DE CONFIGURACIÓN");
		exit(1);
	}

	// t_carpincho* carpincho1 = crear_carpincho(1);

	signal(SIGUSR1, &signal_handler_1);
	signal(SIGUSR2, &signal_handler_2);
	signal(SIGINT,  &signal_handler_3);

	// iniciar_swap(config_get_string_value(config, "IP_SWAP"), config_get_string_value(config, "PUERTO_SWAP"));
	iniciar_servidor(config_get_string_value(config, "IP"), config_get_int_value(config, "PUERTO"));

	// iniciar_swap(config_get_string_value(config, "IP_SWAP"), config_get_int_value(config, "PUERTO_SWAP"));

	terminar_programa(logger, config);
	exit(1);
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
