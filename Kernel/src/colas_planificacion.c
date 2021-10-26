#include "colas_planificacion.h"

void agregar_new(carpincho* carp) {
	pthread_mutex_lock(&mutex_cola_new);
	queue_push(cola_new_carpinchos, carp);
	sem_post(&carpinchos_new);
	pthread_mutex_unlock(&mutex_cola_new);
}
