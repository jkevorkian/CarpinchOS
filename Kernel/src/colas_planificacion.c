#include "colas_planificacion.h"

void agregar_new(carpincho* carp) {
	pthread_mutex_lock(&mutex_cola_new);
	queue_push(cola_new, carp);
	sem_post(&carpinchos_new);
	pthread_mutex_unlock(&mutex_cola_new);
}

void agregar_ready(carpincho* carp) {
	pthread_mutex_lock(&mutex_cola_ready);
	queue_push(cola_ready, carp);
	sem_post(&carpinchos_ready);
	pthread_mutex_unlock(&mutex_cola_ready);
}

carpincho* quitar_new() {
	pthread_mutex_lock(&mutex_cola_new);
	carpincho* carp = (carpincho*)queue_pop(cola_new);
	pthread_mutex_unlock(&mutex_cola_new);

	return carp;
}

carpincho* quitar_suspendidosReady() {
	pthread_mutex_lock(&mutex_cola_suspendidosReady);
	carpincho* carp = (carpincho*)queue_pop(cola_suspendidosReady);
	pthread_mutex_unlock(&mutex_cola_suspendidosReady);

	return carp;
}
