#include "inicializador.h"

int inicializar_kernel(char* direccion_config) {
	logger = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);
	config = config_create(direccion_config);

	if(config == NULL) {
		log_error(logger, "FALLO EN EL ARCHIVO DE CONFIGURACION");
		exit(1);
	}

	leer_configuraciones();

	if(LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tCreando el socket en la IP %s con Puerto %d", ip_kernel, config_get_int_value(config, "PUERTO_ESCUCHA"));

	socket_kernel = crear_conexion_servidor(ip_kernel, config_get_int_value(config, "PUERTO_ESCUCHA"), 1);

	if(!validar_socket(socket_kernel, logger)) {
		close(socket_kernel);
		log_destroy(logger);
		return 1;
	}

	if(LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tSocket funcionando");

	crear_estructuras_planificacion();
	inicializar_semaforos_planificacion();
	iniciar_planificadores();
	iniciar_hilos_cpu();
	inicializar_io();

	if(DEADLOCK_ACTIVADO)
		iniciar_deteccion_deadlock(tiempo_deadlock);

	lista_semaforos = list_create();
	pthread_mutex_init(&mutex_lista_semaforos, NULL);

	id_proximo_carpincho = 0;
	id_proximo_semaforo = 0;

	if(INFORMADOR_LISTAS)
		pthread_create(&hilo_informador, NULL, informador, NULL);

	if(LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tKernel listo");

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
	tiempo_deadlock = config_get_int_value(config, "TIEMPO_DEADLOCK");

	if(LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tConfiguracion leida correctamente");
}

void crear_estructuras_planificacion() {
	cola_new = queue_create();
	cola_suspendidosReady = queue_create();
	cola_running = queue_create();

	lista_ready = list_create();
	lista_blocked = list_create();
	lista_suspendidosBlocked = list_create();

	logger_colas = log_create("colas.log", "COLAS", LOGUEAR_MENSAJES_COLAS, LOG_LEVEL_INFO);
	log_warning(logger_colas, "Kernel iniciado, comenzando con la planificacion");

	if(LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tCreadas las estructuras de planificacion");
}

void inicializar_semaforos_planificacion() {
	pthread_mutex_init(&mutex_cola_new, NULL);
	pthread_mutex_init(&mutex_cola_suspendidosReady, NULL);

	pthread_mutex_init(&mutex_lista_ready, NULL);
	pthread_mutex_init(&mutex_lista_running, NULL);
	pthread_mutex_init(&mutex_lista_blocked, NULL);
	pthread_mutex_init(&mutex_lista_suspendidosBlocked, NULL);

	sem_init(&carpinchos_new, 0, 0);
	sem_init(&carpinchos_ready, 0, 0);
	sem_init(&carpinchos_running, 0, 0);

	sem_init(&multiprogramacion, 0, grado_multiprogramacion);
	sem_init(&multiprocesamiento, 0, grado_multiprocesamiento);

	if(LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tIniciados los semaforos para la planificacion");
}
