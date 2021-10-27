#include "planificadores.h"

void iniciar_planificadores() {
	pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL);
	pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo, NULL);
	pthread_create(&hilo_planificador_mediano_plazo, NULL, planificador_mediano_plazo, NULL);
}

void* planificador_largo_plazo() {
	while(1){
		sem_wait(&carpinchos_new);
		sem_wait(&multiprogramacion);
		if(!queue_is_empty(cola_suspendidosReady)){
			agregar_ready(quitar_suspendidosReady());
		} else
			agregar_ready(quitar_new());
	}
}

void* planificador_corto_plazo() {
	return 0;
}

void* planificador_mediano_plazo() {
	return 0;
}
