#ifndef PLANIFICADORES_H_
#define PLANIFICADORES_H_

#include "global.h"
#include "colas_planificacion.h"

void iniciar_planificadores();

void* planificador_largo_plazo();
void* planificador_corto_plazo();
void* planificador_mediano_plazo();

#endif /* PLANIFICADORES_H_ */
