#include "planificador.h"

void inicializar_planificador(int grado_multiprocesamiento, char* algoritmo) {
	cola_ready = list_create();
	cola_blocked = list_create();
	cola_emergencia = list_create();
	tripulantes_running = list_create();

	pthread_mutex_init(&mutex_cola_ready, NULL);
	pthread_mutex_init(&mutex_tripulantes_running, NULL);
	pthread_mutex_init(&mutex_cola_blocked, NULL);

	sem_init(&activar_planificacion, 0, 0);

	sem_init(&tripulantes_ready, 0, 0);
	sem_init(&multiprocesamiento, 0, grado_multiprocesamiento);

	sem_init(&tripulantes_blocked, 0, 0);
	sem_init(&io_disponible, 0, 1);

	pthread_create(&hilo_planificador, NULL, planificador, algoritmo);
	pthread_create(&hilo_planificador_io, NULL, planificador_io, NULL);
}

void* planificador(void* algoritmo) {
	sem_wait(&activar_planificacion);
	continuar_planificacion = true;

	if(!strcmp(algoritmo,"FIFO")) {
		log_info(logger,"Panificando con algoritmo FIFO ...");
		analizar_quantum = false;
	}
	else {
		log_info(logger,"Panificando con algoritmo RR quantum %d ...", quantum);
		analizar_quantum = true;
	}

	while(!salir) {
		sem_wait(&tripulantes_ready);
		sem_wait(&multiprocesamiento);

		if(hay_sabotaje)
			sem_wait(&finalizo_sabotaje);

		if(list_size(cola_ready) > 0)			//hago esto porque al haber un sabotaje, cuando agrego
			agregar_running(quitar_ready());	//los tripulantes a ready, se le estan sumando valores al semaforo
		else									//de tripulantes que ya estaban en ready. Lo ideal seria poner el
			sem_post(&multiprocesamiento);		//semaforo en 0 al quitar a todos de ready.

	}
	return 0;
}

void* planificador_io() {
	while(!salir) {
		sem_wait(&tripulantes_blocked);
		sem_wait(&io_disponible);

		pthread_mutex_lock(&mutex_cola_blocked);
		trip_block = (tripulante*)list_remove(cola_blocked, 0);
		pthread_mutex_unlock(&mutex_cola_blocked);

		sem_post(&trip_block->sem_blocked);
	}

	return 0;
}

void exit_planificacion() {
	list_destroy(tripulantes_running);
	list_destroy(cola_ready);
	list_destroy(cola_blocked);

	pthread_mutex_destroy(&mutex_cola_ready);
	pthread_mutex_destroy(&mutex_tripulantes_running);
	pthread_mutex_destroy(&mutex_cola_blocked);

	sem_destroy(&activar_planificacion);
	sem_destroy(&multiprocesamiento);
	sem_destroy(&tripulantes_ready);
	sem_destroy(&io_disponible);
	sem_destroy(&tripulantes_blocked);

	pthread_cancel(hilo_planificador);
	pthread_cancel(hilo_planificador_io);
}
