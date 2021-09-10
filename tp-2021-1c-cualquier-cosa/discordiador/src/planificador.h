#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "global.h"
#include "tripulante.h"
#include "colas_planificacion.h"
#include "sabotajes.h"

void inicializar_planificador(int, char*);

void* planificador(void*);
void* planificador_io();
void exit_planificacion();
void* detector_sabotaje(void* socket);

#endif /* PLANIFICADOR_H_ */
