#ifndef COLAS_PLANIFICACION_H_
#define COLAS_PLANIFICACION_H_

#include "global.h"

void agregar_new(carpincho* carp);
void agregar_ready(carpincho* carp);

carpincho* quitar_new();
carpincho* quitar_suspendidosReady();

#endif /* COLAS_PLANIFICACION_H_ */
