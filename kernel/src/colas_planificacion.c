#include "colas_planificacion.h"

void agregar_new(carpincho* carp) {
	pthread_mutex_lock(&mutex_cola_new);
	queue_push(cola_new, carp);
	sem_post(&carpinchos_new);
	pthread_mutex_unlock(&mutex_cola_new);

	if(list_is_empty(lista_ready) && !list_is_empty(lista_blocked) && grado_multiprogramacion < 1) {
		pthread_mutex_lock(&mutex_lista_blocked);
		carpincho* carp = (carpincho*)list_remove(lista_blocked, list_size(lista_blocked)-1); //remuevo el ultimo de la lista de bloqueados
		pthread_mutex_unlock(&mutex_lista_blocked);
		agregar_suspendidosBlocked(carp); //para que se añada un nuevo carpincho a ready
	}

	log_info(logger_colas, " \tAgregado a new el carpincho %d", carp->id);
}

void agregar_ready(carpincho* carp) {
	pthread_mutex_lock(&mutex_lista_ready);
	list_add(lista_ready, carp);
	sem_post(&carpinchos_ready);
	pthread_mutex_unlock(&mutex_lista_ready);

	if(carp->tiempo_llegada)
		free(carp->tiempo_llegada);

	carp->tiempo_llegada = temporal_get_string_time("%H:%M:%S:%MS");

	log_info(logger_colas, " \tAgregado a ready el carpincho %d", carp->id);
}

void agregar_running(carpincho* carp) {
	pthread_mutex_lock(&mutex_lista_running);
	queue_push(cola_running, carp);
	sem_post(&carpinchos_running);
	grado_multiprocesamiento--;
	pthread_mutex_unlock(&mutex_lista_running);

	log_info(logger_colas, " \tAgregado a running el carpincho %d", carp->id);
}

void agregar_blocked(carpincho* carp) {
	//if(list_is_empty(lista_ready) && !queue_is_empty(cola_new))
	if(list_is_empty(lista_ready) && (!queue_is_empty(cola_new) || !queue_is_empty(cola_suspendidosReady)))
		agregar_suspendidosBlocked(carp); //para que se añada un nuevo carpincho a ready
	else {
		pthread_mutex_lock(&mutex_lista_blocked);
		list_add(lista_blocked, carp);
		pthread_mutex_unlock(&mutex_lista_blocked);

		log_info(logger_colas, "\tAgregado a blocked el carpincho %d", carp->id);
	}
}

void agregar_suspendidosReady(carpincho* carp) {
	pthread_mutex_lock(&mutex_cola_suspendidosReady);
	queue_push(cola_suspendidosReady, carp);
	sem_post(&carpinchos_new);
	pthread_mutex_unlock(&mutex_cola_suspendidosReady);

	log_info(logger_colas, " \tAgregado a suspendidosReady el carpincho %d", carp->id);
}

void agregar_suspendidosBlocked(carpincho* carp) {
	if(MEMORIA_ACTIVADA) {
		t_mensaje* mensaje_out = crear_mensaje(SUSPEND);
		enviar_mensaje(carp->socket_memoria, mensaje_out);
		liberar_mensaje_out(mensaje_out);
		liberar_mensaje_in(recibir_mensaje(carp->socket_memoria));
	}

	carp->esta_suspendido = true;
	pthread_mutex_lock(&mutex_lista_suspendidosBlocked);
	list_add(lista_suspendidosBlocked, carp);
	sem_post(&multiprogramacion);
	grado_multiprogramacion++;
	pthread_mutex_unlock(&mutex_lista_suspendidosBlocked);

	log_error(logger_colas, " \tAgregado a suspendidosBlocked el carpincho %d", carp->id);
}

carpincho* quitar_new() {
	pthread_mutex_lock(&mutex_cola_new);
	carpincho* carp = (carpincho*)queue_pop(cola_new);
	pthread_mutex_unlock(&mutex_cola_new);

	log_info(logger_colas, " \t   Quitado de new el carpincho %d", carp->id);

	return carp;
}

carpincho* quitar_ready() {
	pthread_mutex_lock(&mutex_lista_ready);

	int index = list_size(lista_ready)-1;
	carpincho* carp = (carpincho*)list_get(lista_ready, index);
	int indice_carp = index;
	index--;

	if(!strcmp(algoritmo_planificacion,"SJF")) {
		log_info(logger_colas, "Proximo carpincho a eliminar elejido mediante SJF");

		while(index >= 0) {
			carpincho* posible_carp = (carpincho*)list_get(lista_ready, index);

			if(posible_carp->estimacion_proxima_rafaga <= carp->estimacion_proxima_rafaga) {
				carp = posible_carp;
				indice_carp = index;
			}

			index--;
		}
	} else { //HRRN
		log_info(logger_colas, "Proximo carpincho a eliminar elejido mediante HRRN");

		char* tiempo_actual = temporal_get_string_time("%H:%M:%S:%MS");
		float HRRN_carp = calcular_HRRN(carp, tiempo_actual);

		while(index >= 0) {
			carpincho* posible_carp = (carpincho*)list_get(lista_ready, index);
			float HRRN_posible_carp = calcular_HRRN(posible_carp, tiempo_actual);

			log_info(logger_colas, "HRRN carp %d: %f - HRRN carp %d: %f", carp->id, HRRN_carp, posible_carp->id, HRRN_posible_carp);

			if(HRRN_posible_carp >= HRRN_carp) {
				carp = posible_carp;
				HRRN_carp = HRRN_posible_carp;
				indice_carp = index;
			}

			index--;
		}

		free(tiempo_actual);
	}

	list_remove(lista_ready, indice_carp);
	pthread_mutex_unlock(&mutex_lista_ready);

	log_info(logger_colas, " \t   Quitado de ready el carpincho %d", carp->id);

	return carp;
}

carpincho* quitar_suspendidosReady() {
	pthread_mutex_lock(&mutex_cola_suspendidosReady);
	carpincho* carp = (carpincho*)queue_pop(cola_suspendidosReady);
	pthread_mutex_unlock(&mutex_cola_suspendidosReady);

	if(MEMORIA_ACTIVADA) {
		t_mensaje* mensaje_out = crear_mensaje(UNSUSPEND);
		enviar_mensaje(carp->socket_memoria, mensaje_out);
		liberar_mensaje_out(mensaje_out);
		liberar_mensaje_in(recibir_mensaje(carp->socket_memoria));
	}

	carp->esta_suspendido = false;

	log_info(logger_colas, " \t   Quitado de suspendidosReady el carpincho %d", carp->id);

	return carp;
}

carpincho* quitar_blocked(carpincho* carp_quitar) {
	pthread_mutex_lock(&mutex_lista_blocked);
	int index = encontrar_carpincho(lista_blocked, carp_quitar);
	carpincho *carp = list_remove(lista_blocked, index);
	pthread_mutex_unlock(&mutex_lista_blocked);

	log_info(logger_colas, " \t   Quitado de blocked el carpincho %d", carp->id);

	return carp;
}

carpincho* quitar_suspendidosBlocked(carpincho* carp_quitar) {
	pthread_mutex_lock(&mutex_lista_suspendidosBlocked);
	int index = encontrar_carpincho(lista_suspendidosBlocked, carp_quitar);
	carpincho *carp = list_remove(lista_suspendidosBlocked, index);
	pthread_mutex_unlock(&mutex_lista_suspendidosBlocked);

	log_info(logger_colas, " \t   Quitado de suspendidosBlocked el carpincho %d", carp->id);

	return carp;
}

carpincho* quitar_running() {
	pthread_mutex_lock(&mutex_lista_running);
	carpincho* carp = (carpincho*)queue_pop(cola_running);
	pthread_mutex_unlock(&mutex_lista_running);

	log_info(logger_colas, " \t   Quitado de running el carpincho %d", carp->id);

	return carp;
}
