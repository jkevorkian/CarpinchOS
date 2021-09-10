#ifndef COLAS_PLANIFICACION_H_
#define COLAS_PLANIFICACION_H_

#include "global.h"
#include "planificador.h"

void agregar_ready(tripulante* trip);
void agregar_running(tripulante* trip);
void agregar_blocked(tripulante* trip);
void agregar_emergencia(tripulante* trip);
tripulante* quitar_ready();
void quitar_running(tripulante* trip);
void quitar(tripulante*, t_list*);

#endif /* COLAS_PLANIFICACION_H_ */
