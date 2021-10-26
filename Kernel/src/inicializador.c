#include "inicializador.h"

int inicializar_kernel() {
	logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);
	config = config_create("discordiador.config");

	//creacion del socket por el cual me van a llegar todos los mensajes
	socket_kernel = crear_conexion_servidor(ip_kernel, config_get_int_value(config, "PUERTO_ESCUCHA"), 1);

	if(!validar_socket(socket_kernel, logger)) {
		close(socket_kernel);
		log_destroy(logger);
		return 1;
	}

	leer_configuraciones();
	crear_colas_planificacion();
	inicializar_semaforos_planificacion();
	iniciar_planificadores();

	log_info(logger, "Kernel listo");

	return 0;
}

void leer_configuraciones() {
	ip_kernel 					= config_get_string_value(config, "IP_KERNEL");
	ip_memoria 					= config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria 				= config_get_string_value(config, "PUERTO_MEMORIA");

	algoritmo_planificacion 	= config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	grado_multiprogramacion 	= config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	grado_multiprocesamiento 	= config_get_int_value(config, "GRADO_MULTIPROCESAMIENTO");
	alfa 						= config_get_int_value(config, "ALFA");
	estimacion_inicial 			= config_get_int_value(config, "ESTIMACION_INICIAL");
}

void crear_colas_planificacion() {
	cola_new_carpinchos = queue_create();
}

void inicializar_semaforos_planificacion() {
	pthread_mutex_init(&mutex_cola_new, NULL);
	pthread_mutex_init(&mutex_cola_ready, NULL);

	sem_init(&carpinchos_new, 0, 0);
	sem_init(&carpinchos_ready, 0, 0);

	sem_init(&multiprogramacion, 0, grado_multiprogramacion);
	sem_init(&multiprocesamiento, 0, grado_multiprocesamiento);
}
