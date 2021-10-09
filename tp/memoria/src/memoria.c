#include "memoria.h"

int main(void) {
	t_config* config = config_create("miramhq.config");
	// t_log* logger = log_create("miramhq.log", "MEMORIA", 1, LOG_LEVEL_INFO);

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

void signal_handler_1(int sig) {

}

void signal_handler_2(int sig) {

}

void signal_handler_3(int sig) {

}
