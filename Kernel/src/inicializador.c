#include "inicializador.h"

int inicializar_kernel() {
	logger = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);
	config = config_create("kernel.config");

	leer_configuraciones();

	log_info(logger, "Creando el socket en la IP %s con Puerto %d", ip_kernel, config_get_int_value(config, "PUERTO_ESCUCHA"));
	socket_kernel = crear_conexion_servidor(ip_kernel, config_get_int_value(config, "PUERTO_ESCUCHA"), 1);

	if(!validar_socket(socket_kernel, logger)) {
		close(socket_kernel);
		log_destroy(logger);
		return 1;
	}

	log_info(logger, "Socket funcionando");

	crear_estructuras_planificacion();
	inicializar_semaforos_planificacion();
	iniciar_planificadores();
	iniciar_hilos_cpu();
	inicializar_io();

	lista_semaforos = list_create();
	pthread_mutex_init(&mutex_lista_semaforos, NULL);

	id_proximo_carpincho = 0;

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
	alfa 						= config_get_double_value(config, "ALFA");
	estimacion_inicial 			= config_get_int_value(config, "ESTIMACION_INICIAL");
}

void crear_estructuras_planificacion() {
	cola_new = queue_create();
	cola_suspendidosReady = queue_create();
	cola_running = queue_create();

	lista_ready = list_create();
	lista_blocked = list_create();

}

void inicializar_semaforos_planificacion() {
	pthread_mutex_init(&mutex_cola_new, NULL);
	pthread_mutex_init(&mutex_lista_ready, NULL);
	pthread_mutex_init(&mutex_lista_running, NULL);
	pthread_mutex_init(&mutex_lista_blocked, NULL);
	pthread_mutex_init(&mutex_cola_suspendidosReady, NULL);

	sem_init(&carpinchos_new, 0, 0);
	sem_init(&carpinchos_ready, 0, 0);
	sem_init(&carpinchos_running, 0, 0);

	sem_init(&multiprogramacion, 0, grado_multiprogramacion);
	sem_init(&multiprocesamiento, 0, grado_multiprocesamiento);
}
