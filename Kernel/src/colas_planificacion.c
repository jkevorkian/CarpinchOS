#include "colas_planificacion.h"

void agregar_new(carpincho* carp) {
	pthread_mutex_lock(&mutex_cola_new);
	queue_push(cola_new, carp);
	sem_post(&carpinchos_new);
	pthread_mutex_unlock(&mutex_cola_new);
}

void agregar_ready(carpincho* carp) {
	pthread_mutex_lock(&mutex_lista_ready);
	list_add(lista_ready, carp);
	sem_post(&carpinchos_ready);
	pthread_mutex_unlock(&mutex_lista_ready);
}

void agregar_running(carpincho* carp) {
	pthread_mutex_lock(&mutex_lista_running);
	list_add(lista_running, carp);
	sem_post(&carpinchos_running);
	pthread_mutex_unlock(&mutex_lista_running);
}

carpincho* quitar_new() {
	pthread_mutex_lock(&mutex_cola_new);
	carpincho* carp = (carpincho*)queue_pop(cola_new);
	pthread_mutex_unlock(&mutex_cola_new);

	return carp;
}

carpincho* quitar_ready() {
	carpincho* carp;

	if(!strcmp(algoritmo_planificacion,"SJF")) {
		int index = list_size(lista_ready)-1;
		carp = (carpincho*)list_get(lista_ready, index);
		int indice_carp = index;
		index--;

		while(index >= 0) {
			carpincho* posible_carp = (carpincho*)list_get(lista_ready, index);

			if(posible_carp->estimacion_proxima_rafaga <= carp->estimacion_proxima_rafaga) {
				carp = posible_carp;
				indice_carp = index;
			}

			index--;
		}

		list_remove(lista_ready, indice_carp);
	} else { //HRRN
		int index = list_size(lista_ready)-1;
		carp = (carpincho*)list_get(lista_ready, index);
		int indice_carp = index;
		index--;

		while(index >= 0) {
			carpincho* posible_carp = (carpincho*)list_get(lista_ready, index);

			if(calcular_HRRN(posible_carp) >= calcular_HRRN(carp)) {
				carp = posible_carp;
				indice_carp = index;
			}

			index--;
		}

		list_remove(lista_ready, indice_carp);
	}

	return carp;
}

carpincho* quitar_suspendidosReady() {
	pthread_mutex_lock(&mutex_cola_suspendidosReady);
	carpincho* carp = (carpincho*)queue_pop(cola_suspendidosReady);
	pthread_mutex_unlock(&mutex_cola_suspendidosReady);

	return carp;
}

int calcular_HRRN(carpincho* carp) {
	return (carp->estimacion_proxima_rafaga + carp->tiempo_espera)/carp->estimacion_proxima_rafaga;
}

