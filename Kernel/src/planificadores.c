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
		carpincho* carp;
		if(!queue_is_empty(cola_suspendidosReady))
			carp = quitar_suspendidosReady();
		else {
			carp = quitar_new();
			//log_info(logger, "Carpincho quitado de new - carpinchos en new %d", queue_size(cola_new));
		}

		carp->tiempo_llegada = temporal_get_string_time("%H:%M:%S:%MS");
		agregar_ready(carp);
		//log_info(logger, "Carpincho agregado a ready - carpinchos en ready %d", list_size(lista_ready));
	}
}

void* planificador_corto_plazo() {
	while(1){
		sem_wait(&carpinchos_ready);
		sem_wait(&multiprocesamiento);
		carpincho* carp = quitar_ready();
		//log_info(logger, "Carpincho quitado de ready - carpinchos en ready %d", list_size(lista_ready));
		agregar_running(carp);
		//log_info(logger, "Carpincho agregado a running");
	}
}

void* planificador_mediano_plazo() {
	return 0;
}
