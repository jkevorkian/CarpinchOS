#ifndef COLAS_PLANIFICACION_H_
#define COLAS_PLANIFICACION_H_

#include "global.h"
#include "hilos_cpu.h"

void agregar_new(carpincho* carp);
void agregar_ready(carpincho* carp);
void agregar_running(carpincho* carp);

carpincho* quitar_new();
carpincho* quitar_ready();
carpincho* quitar_suspendidosReady();
carpincho* quitar_running();

float calcular_HRRN(carpincho* carp, char* tiempo_actual);

#endif /* COLAS_PLANIFICACION_H_ */
