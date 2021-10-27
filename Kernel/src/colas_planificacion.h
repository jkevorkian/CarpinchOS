#ifndef COLAS_PLANIFICACION_H_
#define COLAS_PLANIFICACION_H_

#include "global.h"

void agregar_new(carpincho* carp);
void agregar_ready(carpincho* carp);
void agregar_running(carpincho* carp);

carpincho* quitar_new();
carpincho* quitar_ready();
carpincho* quitar_suspendidosReady();
carpincho* quitar_running();

int calcular_HRRN(carpincho* carp);

#endif /* COLAS_PLANIFICACION_H_ */
