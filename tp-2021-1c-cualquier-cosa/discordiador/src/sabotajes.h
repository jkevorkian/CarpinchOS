#ifndef SABOTAJES_H_
#define SABOTAJES_H_

#include "global.h"
#include "planificador.h"

void emergency_trips_running();
void emergency_trips_ready();
int seleccionar_trip(t_list* lista);
tripulante* encontrar_mas_cercano(int pos_x, int pos_y);
void resolver_sabotaje(int pos_x, int pos_y, int socket_sabotajes);
void exit_sabotajes();

#endif /* SABOTAJES_H_ */
