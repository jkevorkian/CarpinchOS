#include "miramhq.h"

int main(void) {
	if(CONSOLA_ACTIVA)
		logger = log_create("miramhq.log", "Mi-RAM-HQ", 0, LOG_LEVEL_INFO);
	else
		logger = log_create("miramhq.log", "Mi-RAM-HQ", 1, LOG_LEVEL_INFO);
	
	t_config* config = config_create("miramhq.config");
	lista_patotas = list_create();
	lista_tareas = list_create();
	lista_tripulantes = list_create();
	movimientos_pendientes = list_create();

	if(!iniciar_memoria(config)) {
		log_info(logger, "FALLO EN EL ARCHIVO DE CONFIGURACIÓN");
		return 0;
	}
	
	sem_init(&semaforo_consola, 0, 0);
	sem_init(&mutex_movimiento, 0, 1);
	sem_init(&mutex_lista_tripulantes, 0, 1);

	bool* continuar_consola = malloc(sizeof(bool));
	pthread_t* hilo_consola;
	if(CONSOLA_ACTIVA) {
		log_info(logger, "CONSOLA ACTIVA");
		*continuar_consola = true;
		hilo_consola = iniciar_mapa(continuar_consola);
	}

	int server_fd = crear_conexion_servidor(IP_RAM,	config_get_int_value(config, "PUERTO"), 1);

	if(!validar_socket(server_fd, logger)) {
		close(server_fd);
		log_destroy(logger);
		// liberar_memoria(config, socket_discord, hilo_consola);
		return ERROR_CONEXION;
	}
	log_info(logger, "Servidor listo");
	int socket_discord = esperar_cliente(server_fd);
	close(server_fd);
	
	log_info(logger, "Conexión establecida con el discordiador");
	signal(SIGUSR1, &signal_dump);
	signal(SIGUSR2, &signal_compactacion);
	
	t_list* mensaje_in;
	t_mensaje* mensaje_out;
	
	bool inicio_correcto;
	bool conexion_activa_discord = true;

	uint32_t patota_actual = 0;
	uint32_t nro_tripulante;

	uint32_t id_trip;	// Maldito c
	uint32_t id_patota;	// Maldito c

	while(conexion_activa_discord == true) {
		log_info(logger, "Esperando información del discordiador");
		mensaje_in = recibir_mensaje(socket_discord);
		log_info(logger, "Recibir mensaje");
		if (!validar_mensaje(mensaje_in, logger)) {
			// liberar_mensaje_in(mensaje_in);
			// log_info(logger, "Cliente desconectado dentro del while");
			// close(server_fd);
			// log_destroy(logger);
			// // liberar_metadata(config, socket_discord);
			// return ERROR_CONEXION;
			break;
		}
		
		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
		case INIT_P:
			log_info(logger, "Discordiador solicitó iniciar_patota. Cant tripulantes: %d", (int)list_get(mensaje_in, 1));
			patota_actual++;
			inicio_correcto = iniciar_patota(patota_actual, mensaje_in);
			if(!inicio_correcto) {
				mensaje_out = crear_mensaje(NO_SPC);
				patota_actual--;
			}
			else {
				mensaje_out = crear_mensaje(TODOOK);
			}

			enviar_mensaje(socket_discord, mensaje_out);
			liberar_mensaje_out(mensaje_out);		// debe estar fuera del switch
			nro_tripulante = 1;
			break;
		case INIT_T:
			log_info(logger, "Discordiador solicitó iniciar_tripulante");
			uint32_t posicion_x = (uint32_t)list_get(mensaje_in, 1);
			uint32_t posicion_y = (uint32_t)list_get(mensaje_in, 2);
			log_info(logger, "Entro a iniciar_tripulante");
			int puerto = iniciar_tripulante(nro_tripulante, patota_actual, posicion_x, posicion_y);
			if(puerto == 0) {
				mensaje_out = crear_mensaje(NO_SPC);
			}
			else {
				mensaje_out = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(mensaje_out, (void *)puerto, ENTERO);
			}

			enviar_mensaje(socket_discord, mensaje_out);
			liberar_mensaje_out(mensaje_out);
			nro_tripulante++;
			break;
		case ELIM_T:
			log_info(logger, "Discordiador solicitó expulsar_tripulante");
			id_trip = (uint32_t)list_get(mensaje_in, 1);
			id_patota = (uint32_t)list_get(mensaje_in, 2);
			eliminar_tripulante(id_patota, id_trip);
			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket_discord, mensaje_out);
			liberar_mensaje_out(mensaje_out);
			
			// if(patota_sin_tripulantes(id_patota)) {
			// 	// eliminar_tareas(id_patota);
			// }

			break;
		case 64:
			log_info(logger, "Cliente desconectado 64");
			conexion_activa_discord = false;
			break;
		default:
			log_info(logger, "Cliente desconectado default");
			conexion_activa_discord = false;
			break;
		}
		liberar_mensaje_in(mensaje_in);
		loggear_data();
	}
    log_info(logger, "Paso a cambiar continuar_consola");
	*continuar_consola = false;
	sem_post(&semaforo_consola);

	// liberar_metadata(config, socket_discord);
	if(CONSOLA_ACTIVA) {
		pthread_join(*hilo_consola, 0);
		free(hilo_consola);
	}
	// log_info(logger, "Recibi la consola");
	// config_destroy(config);
	// log_destroy(logger);
	// close(socket_discord);
	
	return EXIT_SUCCESS;
}

void liberar_segmentos() {
	list_destroy(memoria_ram.mapa_segmentos);
	// void destruir_segmento(void* segmento) {
	// 	free(segmento);
	// }
	
	// list_destroy_and_destroy_elements(lista_patotas, destruir_segmento);
}

void liberar_patotas() {
	void destruir_patota(void* patota) {
		free(((patota_data *)patota)->inicio_elementos);
		free(patota);
	}
	
	list_destroy_and_destroy_elements(lista_patotas, destruir_patota);
}

void liberar_tareas() {
	void destruir_tarea(void* tarea) {
		free(((tareas_data *)tarea)->inicio_tareas);
		free(((tareas_data *)tarea)->tamanio_tareas);
		free(tarea);
	}

	list_destroy_and_destroy_elements(lista_patotas, destruir_tarea);
}

void liberar_tripulantes() {
	trip_data* trip_aux;
	for(int i = 0; i < list_size(lista_tripulantes); i++) {
		sem_wait(&mutex_lista_tripulantes);
		trip_aux = list_remove(lista_tripulantes, 0);
		sem_post(&mutex_lista_tripulantes);
		
		// trip_aux = list_remove(lista_tripulantes, 0);
		pthread_cancel(*trip_aux->hilo);
		pthread_join(*trip_aux->hilo, NULL);
	}
	list_destroy(lista_tripulantes);
	// void destruir_tripulante(void* tripulante) {
	// 	free(((trip_data *)tripulante)->semaforo_hilo);
	// 	free(((trip_data *)tripulante)->hilo);
	// 	free(tripulante);
	// }
	// list_destroy_and_destroy_elements(lista_patotas, destruir_tripulante);
}

bool iniciar_memoria(t_config* config) {
	memoria_ram.tamanio_memoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	memoria_ram.inicio = malloc(memoria_ram.tamanio_memoria);
	char *esquema_memoria = config_get_string_value(config, "ESQUEMA_MEMORIA");
	bool inicio_correcto = false;
	if(!strcmp(esquema_memoria, "SEGMENTACION")) {
		inicio_correcto = iniciar_memoria_segmentada(config);
	}
	if(!strcmp(esquema_memoria, "PAGINACION")) {
		inicio_correcto = iniciar_memoria_paginada(config);
	}
	free(esquema_memoria);
	return inicio_correcto;
}

void liberar_metadata(t_config* config, int socket_discord) {
	log_info(logger, "Segmentos");
	liberar_segmentos();
	log_info(logger, "Patotas");
	liberar_patotas();
	log_info(logger, "Tareas");    // Rompe
	// liberar_tareas();
	log_info(logger, "Tripulantes");
	liberar_tripulantes();
	log_info(logger, "Consola");
	log_info(logger, "Recibi la consola");
	config_destroy(config);
	log_destroy(logger);
	close(socket_discord);
}


void signal_compactacion(int sig){
	log_info(logger, "Recibi la senial de compactacion");
	if(memoria_ram.esquema_memoria == SEGMENTACION)
		realizar_compactacion();
}

void signal_dump(int sig){
	log_info(logger, "Recibi la senial de dump");
	dump();
}