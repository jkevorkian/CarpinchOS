#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include <stdlib.h>

#include "global.h"
#include "planificador.h"
#include "mensajes_trip.h"
#include "validaciones.h"

tripulante* crear_tripulante(int, int, int, int, int, int);
void* rutina_tripulante(void*);
void ejecutar(char*, tripulante*);
void moverse(tripulante*, int, int);
void ejecutar_io(tripulante* trip, tareas tarea, int cantidad, int tiempo_io);
void esperar(int, tripulante*);

#endif /* TRIPULANTE_H_ */
