#include "memoria.h"


int main(void) {
	t_config* config = config_create("miramhq.config");
	t_log* logger = log_create("miramhq.log", "MEMORIA", 1, LOG_LEVEL_INFO);

	/* if(!iniciar_memoria(config)) {
		log_info(logger, "FALLO EN EL ARCHIVO DE CONFIGURACIÓN");
		return 0;
	}*/

	// signal(SIGUSR1, &...);
	// signal(SIGUSR2, &...);
	// signal(SIGUSR2, &...);

	iniciar_servidor(config_get_string_value(config, "DIRECCION"), config_get_int_value(config, "PUERTO"));
	exit(1);
}
