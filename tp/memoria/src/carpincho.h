#ifndef _CARPINCHO_H_
#define _CARPINCHO_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>   // sprintf
#include <arpa/inet.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <utils/sockets.h>

typedef struct {
	int socket;
	int8_t kernel;
} data_carpincho;

void *rutina_carpincho(void* info_carpincho);

#endif /* _CARPINCHO_H_ */
