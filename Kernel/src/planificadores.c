#include "planificadores.h"

void iniciar_planificadores() {
	pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL);
}

void* planificador_largo_plazo() {
	return 0;
}
