#ifndef _CARPINCHO_H_
#define _CARPINCHO_H_

#include <utils/sockets.h>

typedef struct {
	int socket;
} data_carpincho;

void *rutina_carpincho(void* info_carpincho);

#endif /* _CARPINCHO_H_ */
