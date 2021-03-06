#include "planificadores.h"

void iniciar_planificadores() {
	pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL);
	pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo, NULL);

	if(LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tIniciados los planificadores");
}

void* planificador_largo_plazo() {
	while(1){
		sem_wait(&carpinchos_new);
		sem_wait(&multiprogramacion);
		grado_multiprogramacion--;

		carpincho* carp;
		queue_is_empty(cola_suspendidosReady) ? (carp = quitar_new()) : (carp = quitar_suspendidosReady());

		agregar_ready(carp);
	}
}

void* planificador_corto_plazo() {
	while(1){
		sem_wait(&carpinchos_ready);
		sem_wait(&multiprocesamiento);
		agregar_running(quitar_ready());
	}
}
