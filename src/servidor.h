#ifndef _SERVIDOR_H_
#define _SERVIDOR_H_

#include "carpincho.h"

void iniciar_servidor(char *ip, int puerto);
bool iniciar_swap(char *ip_swap, char *puerto_swap);

void *rutina_creador_movimientos(void *pepe);

#endif /* _SERVIDOR_H_ */
