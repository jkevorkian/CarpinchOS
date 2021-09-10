#include "sabotajes.h"

void emergency_trips_running() {
	while(!list_is_empty(tripulantes_running)) {

		pthread_mutex_lock(&mutex_tripulantes_running);
		tripulante* trip_quitado = (tripulante*) list_remove(tripulantes_running, seleccionar_trip(tripulantes_running));
		pthread_mutex_unlock(&mutex_tripulantes_running);

		log_info(logger, "Quitado de running el trip %d", trip_quitado->id_trip);

		agregar_emergencia(trip_quitado);

		sem_wait(&trip_quitado->sem_blocked);
	}
}

void emergency_trips_ready() {
	while(!list_is_empty(cola_ready)) {

		pthread_mutex_lock(&mutex_cola_ready);
		tripulante* trip_quitado = (tripulante*) list_remove(cola_ready, seleccionar_trip(cola_ready));
		pthread_mutex_unlock(&mutex_cola_ready);

		log_info(logger, "Quitado de ready el trip %d", trip_quitado->id_trip);

		agregar_emergencia(trip_quitado);
	}
}

int seleccionar_trip(t_list* lista) {
	int index = list_size(lista)-1;

	tripulante* trip_quitar = (tripulante*)list_get(lista, index);
	int indice_trip_quitar = index;
	index--;

	while(index >= 0) {
		tripulante* nuevo_tripulante = (tripulante*)list_get(lista, index);

		if(nuevo_tripulante->id_trip < trip_quitar->id_trip || (nuevo_tripulante->id_trip == trip_quitar->id_trip && nuevo_tripulante->id_patota < trip_quitar->id_patota)) {
			trip_quitar = nuevo_tripulante;
			indice_trip_quitar = index;
		}

		index--;
	}

	return indice_trip_quitar;
}

void* detector_sabotaje(void* socket_mongo) {
	hay_sabotaje = false;
	int socket_sabotajes = *(int*)socket_mongo;
	sem_init(&sabotaje_pausado, 0, 0);
	sem_init(&finalizo_sabotaje, 0, 0);

	while(!salir) {
		t_list* mensaje_sabotaje = recibir_mensaje(socket_sabotajes);

		if((int)list_get(mensaje_sabotaje, 0) == SABO_P) {
			hay_sabotaje = true;

			int pos_x = (int)list_get(mensaje_sabotaje, 1);
			int pos_y = (int) list_get(mensaje_sabotaje, 2);
			liberar_mensaje_in(mensaje_sabotaje);

			log_warning(logger, "Hubo un sabotaje en %d|%d", pos_x, pos_y);

			if(!continuar_planificacion) {
				log_warning(logger, "Esperando a que se reanude la planificacion para resolver sabotaje");
				sem_wait(&sabotaje_pausado);
			}

			emergency_trips_running();
			emergency_trips_ready();

			if(!list_is_empty(cola_emergencia)){
				resolver_sabotaje(pos_x, pos_y, socket_sabotajes);

				while(!list_is_empty(cola_emergencia)) {
					tripulante* trip = (tripulante*)list_remove(cola_emergencia, 0);
					agregar_ready(trip);
					log_info(logger, "Tripulante %d en ready", trip->id_trip);
				}

				hay_sabotaje = false;
				sem_post(&finalizo_sabotaje); 		//para que se reactive la planificacion

				if(trip_block != NULL)				//si hay un tripulante bloqueado le aviso que finalizo
					sem_post(&finalizo_sabotaje);	//el sabotaje para que contiue trabajando

			}else {
				log_error(logger, "No hay tripulantes disponibles para resolver el sabotaje");

				t_mensaje* mensaje_error = crear_mensaje(NO_SPC);
				enviar_mensaje(socket_sabotajes, mensaje_error);
				liberar_mensaje_out(mensaje_error);
			}
		}else
			log_warning(logger, "No entendi el mensaje");
	}
	return 0;
}

tripulante* encontrar_mas_cercano(int pos_x, int pos_y) {
	int index = list_size(cola_emergencia)-1;
	tripulante* resolvedor = (tripulante*)list_get(cola_emergencia, index);
	int indice_resolvedor = index;
	index--;

	while(index >= 0) {
		tripulante* posible_resolvedor = (tripulante*)list_get(cola_emergencia, index);

		if(distancia_a(posible_resolvedor, pos_x, pos_y) <= distancia_a(resolvedor, pos_x, pos_y)) {
			resolvedor = posible_resolvedor;
			indice_resolvedor = index;
		}

		index--;
	}

	log_info(logger, "El tripulante %d va a resolver el savotaje", resolvedor->id_trip);

	list_remove(cola_emergencia, indice_resolvedor);
	list_add(cola_emergencia, resolvedor);

	log_info(logger, "Resolvedor mandado al final de la cola");

	return resolvedor;
}

void resolver_sabotaje(int pos_x, int pos_y, int socket_sabotajes) {
	tripulante* trip = encontrar_mas_cercano(pos_x, pos_y);

	t_mensaje* mensaje_ini = crear_mensaje(SABO_I);
	agregar_parametro_a_mensaje(mensaje_ini, (void*)trip->id_trip, ENTERO);
	agregar_parametro_a_mensaje(mensaje_ini, (void*)trip->id_patota, ENTERO);
	enviar_y_verificar(mensaje_ini, socket_sabotajes, "Fallo al iniciar resolucion del sabotaje");

	while(trip->posicion[0] != pos_x) {
		(trip->posicion[0] < pos_x) ? trip->posicion[0]++ : trip->posicion[0]--;
		avisar_movimiento(trip);
		sleep(ciclo_CPU);
	}

	while(trip->posicion[1] != pos_y) {
		(trip->posicion[1] < pos_y) ? trip->posicion[1]++ : trip->posicion[1]--;
		avisar_movimiento(trip);
		sleep(ciclo_CPU);
	}

	t_mensaje* mensaje_fin = crear_mensaje(SABO_F);
	enviar_y_verificar(mensaje_fin, socket_sabotajes, "Fallo al finalizar resolucion del sabotaje");
}

void exit_sabotajes() {
	pthread_cancel(hilo_detector_sabotaje);
	list_destroy(cola_emergencia);
	sem_destroy(&finalizo_sabotaje);
	sem_destroy(&sabotaje_pausado);
}
