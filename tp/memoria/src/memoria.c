#include "memoria.h"

int main(void) {
	t_config* config = config_create("miramhq.config");
	// t_log* logger = log_create("miramhq.log", "MEMORIA", 1, LOG_LEVEL_INFO);

	/* if(!iniciar_memoria(config)) {
		log_info(logger, "FALLO EN EL ARCHIVO DE CONFIGURACIÓN");
		return 0;
	}*/

	signal(SIGUSR1, &signal_handler_1);
	signal(SIGUSR2, &signal_handler_2);
	signal(SIGINT,  &signal_handler_3);

	iniciar_servidor(config_get_string_value(config, "DIRECCION"), config_get_int_value(config, "PUERTO"));
	exit(1);
}

void signal_handler_1(int sig) {

}

void signal_handler_2(int sig) {

}

void signal_handler_3(int sig) {

}
