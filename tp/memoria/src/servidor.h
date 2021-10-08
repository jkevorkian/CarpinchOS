#ifndef _SERVIDOR_H_
#define _SERVIDOR_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>   //sprintf
#include <arpa/inet.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <utils/sockets.h>
#include <pthread.h>

#include "carpincho.h"

void iniciar_servidor(char *ip, int puerto);

#endif /* _SERVIDOR_H_ */
