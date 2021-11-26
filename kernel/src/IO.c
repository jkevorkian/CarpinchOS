#include "IO.h"

void inicializar_io()
{
	lista_IO = list_create();

	char **nombres = config_get_array_value(config, "DISPOSITIVOS_IO");
	char **duraciones = config_get_array_value(config, "DURACIONES_IO");

	int i = 0;

	while (nombres[i] != NULL)
	{
		IO *disp = malloc(sizeof(IO));

		disp->nombre = string_duplicate(nombres[i]);
		disp->duracion = atoi(duraciones[i]);
		disp->cola_espera = queue_create();

		pthread_mutex_init(&disp->mutex_espera, NULL);
		sem_init(&disp->carpinchos_esperando, 0, 0);

		pthread_create(&disp->hilo_IO, NULL, manejador_io, disp);

		list_add(lista_IO, disp);
		i++;
	}

	liberar_split(nombres);
	liberar_split(duraciones);
}

void *manejador_io(void *d)
{
	IO *disp = (IO *)d;

	if (LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tIniciado el hilo de io: %s - duracion: %d ", disp->nombre, disp->duracion);

	while (1)
	{
		sem_wait(&disp->carpinchos_esperando);

		pthread_mutex_lock(&disp->mutex_espera);
		carpincho *carp = (carpincho *)queue_pop(disp->cola_espera);
		pthread_mutex_unlock(&disp->mutex_espera);

		sleep(disp->duracion);

		log_info(logger, "El carpincho %d finalizo io", carp->id);
		carp->responder = true;

		desbloquear(carp);
	}
}
