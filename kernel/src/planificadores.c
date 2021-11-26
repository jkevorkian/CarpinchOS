#include "planificadores.h"

void iniciar_planificadores()
{
	pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL);
	pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo, NULL);
	pthread_create(&hilo_planificador_mediano_plazo, NULL, planificador_mediano_plazo, NULL);

	if (LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tIniciados los planificadores");
}

void *planificador_largo_plazo()
{
	while (1)
	{
		sem_wait(&carpinchos_new);
		sem_wait(&multiprogramacion);
		carpincho *carp;

		queue_is_empty(cola_suspendidosReady) ? (carp = quitar_new()) : (carp = quitar_suspendidosReady());

		carp->tiempo_llegada = temporal_get_string_time("%H:%M:%S:%MS");
		agregar_ready(carp);
	}
}

void *planificador_corto_plazo()
{
	while (1)
	{
		sem_wait(&carpinchos_ready);
		sem_wait(&multiprocesamiento);
		//carpincho* carp = quitar_ready();
		agregar_running(quitar_ready());
	}
}

void *planificador_mediano_plazo()
{
	return 0;
}
