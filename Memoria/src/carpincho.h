#ifndef _CARPINCHO_H_
#define _CARPINCHO_H_

#include <utils/sockets.h>
#include "memoria.h"

typedef struct {
	int socket;
	int id;
} data_carpincho;

void *rutina_carpincho(void* info_carpincho);
t_carpincho* crear_carpincho(int);

#endif /* _CARPINCHO_H_ */
