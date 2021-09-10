#ifndef PARAMETROS_H_
#define PARAMETROS_H_

#include "global.h"
#include "validaciones.h"

parametros_iniciar_patota* obtener_parametros(char** input);
void liberar_parametros(parametros_iniciar_patota* parametros);
void loggear_parametros(parametros_iniciar_patota* parametros);

#endif /* PARAMETROS_H_ */
