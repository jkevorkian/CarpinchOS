#include "IO.h"

void inicializar_io() {
	lista_IO = list_create();

	char **nombres = config_get_array_value(config, "DISPOSITIVOS_IO");
	char **duraciones = config_get_array_value(config, "DURACIONES_IO");

	int i = 0;

	while(nombres[i] != NULL) {
		IO *disp = malloc(sizeof(IO*));
		disp->cola_espera = queue_create();
		disp->nombre = string_duplicate(nombres[i]);
		disp->duracion = atoi(duraciones[i]);
		pthread_create(&disp->hilo_IO, NULL, manejador_io, disp);
		pthread_mutex_init(&disp->mutex_espera, NULL);
	}

	liberar_split(nombres);
	liberar_split(duraciones);
}

void* manejador_io(void* d) {
	IO *disp = (IO*) d;

	while(1) {
		sem_wait(&disp->carpinchos_esperando);

		pthread_mutex_lock(&disp->mutex_espera);
		carpincho *carp = queue_pop(disp->cola_espera);
		pthread_mutex_unlock(&disp->mutex_espera);

		sleep(disp->duracion);

		desbloquear(carp);
		carp->responder_IO = true;
	}
}
