#include "colas_planificacion.h"

void agregar_ready(tripulante* trip) {
	actualizar_estado(trip, READY);

	pthread_mutex_lock(&mutex_cola_ready);
		list_add(cola_ready, trip);
		sem_post(&tripulantes_ready);
	pthread_mutex_unlock(&mutex_cola_ready);
}

void agregar_running(tripulante* trip) {
	actualizar_estado(trip, RUNNING);

	pthread_mutex_lock(&mutex_tripulantes_running);
		list_add(tripulantes_running, trip);
		sem_post(&trip->sem_running);
	pthread_mutex_unlock(&mutex_tripulantes_running);
}

void agregar_blocked(tripulante* trip) {
	actualizar_estado(trip, BLOCKED);

	pthread_mutex_lock(&mutex_cola_blocked);
		list_add(cola_blocked, trip);
		sem_post(&tripulantes_blocked);
	pthread_mutex_unlock(&mutex_cola_blocked);
}

void agregar_emergencia(tripulante* trip) {
	actualizar_estado(trip, EMERGENCY);
	list_add(cola_emergencia, trip);
}

tripulante* quitar_ready() {
	pthread_mutex_lock(&mutex_cola_ready);
		tripulante* trip = (tripulante*)list_remove(cola_ready, 0);
	pthread_mutex_unlock(&mutex_cola_ready);
	return trip;
}

void quitar_running(tripulante* trip) {
	pthread_mutex_lock(&mutex_tripulantes_running);
		quitar(trip, tripulantes_running);
		sem_post(&multiprocesamiento);
	pthread_mutex_unlock(&mutex_tripulantes_running);
}

void quitar(tripulante* trip, t_list* list) {
	int index = 0;
	bool continuar = true;

	while(continuar) {
		tripulante* nuevo_tripulante = (tripulante*)list_get(list, index);

		if(nuevo_tripulante->id_trip == trip->id_trip && nuevo_tripulante->id_patota == trip->id_patota) {
			continuar = false;
			list_remove(list, index);
		}
		index++;
	}
}
