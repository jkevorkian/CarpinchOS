#ifndef COLAS_PLANIFICACION_H_
#define COLAS_PLANIFICACION_H_

#include "global.h"
#include "hilos_cpu.h"
#include "utilidades.h"

void agregar_new(carpincho* carp);
void agregar_ready(carpincho* carp);
void agregar_running(carpincho* carp);
void agregar_blocked(carpincho* carp);
void agregar_suspendidosReady(carpincho* carp);
void agregar_suspendidosBlocked(carpincho* carp);

carpincho* quitar_new();
carpincho* quitar_ready();
carpincho* quitar_suspendidosReady();
carpincho* quitar_blocked(carpincho* carp_quitar);
carpincho* quitar_suspendidosBlocked(carpincho* carp_quitar);
carpincho* quitar_running();

#endif /* COLAS_PLANIFICACION_H_ */
