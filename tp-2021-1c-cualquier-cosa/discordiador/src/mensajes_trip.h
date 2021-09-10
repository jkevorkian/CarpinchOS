#ifndef MENSAJES_TRIP_H_
#define MENSAJES_TRIP_H_

#include "global.h"

bool respuesta_OK(t_list* respuesta, char* mensaje_fallo);
bool enviar_y_verificar(t_mensaje* mensaje_out, int socket, char* mensaje_error);
char* solicitar_tarea(tripulante*);
void avisar_movimiento(tripulante*);
void actualizar_estado(tripulante* trip, estado estado_trip);

#endif /* MENSAJES_TRIP_H_ */
