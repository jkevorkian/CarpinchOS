#ifndef _SERVIDOR_H_
#define _SERVIDOR_H_

#include "carpincho.h"
#include <stdbool.h>

void iniciar_servidor(char *ip, int puerto);
bool iniciar_swap(char *ip_swap, char *puerto_swap);

#endif /* _SERVIDOR_H_ */
