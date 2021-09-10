#ifndef VALIDACIONES_H_
#define VALIDACIONES_H_

#include "global.h"
#include "colas_planificacion.h"

/////////////////////VALIDACIONES//////////////////////
void puede_continuar(tripulante*);


/////////////////////UTILIDADES//////////////////////
char* estado_enumToString(int);
tareas stringToEnum(char *);
void liberar_split(char** input);
int distancia_a(tripulante* trip, int pos_x, int pos_y);

#endif /* VALIDACIONES_H_ */
