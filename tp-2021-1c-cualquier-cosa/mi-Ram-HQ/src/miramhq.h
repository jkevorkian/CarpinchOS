#ifndef _MIRAMHQ_H_
#define _MIRAMHQ_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<utils/utils-server.h>
#include<utils/utils-sockets.h>
#include<utils/utils-mensajes.h>

#include<commons/collections/list.h>
#include<commons/config.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include "memoria_ram.h"
#include "segmentos.h"
#include "patota.h"
#include "tripulante.h"
#include "consola.h"
#include "logs.h"

#include <errno.h>

#define ERROR_CONEXION -1

pthread_t* iniciar_mapa(bool*);
bool iniciar_memoria(t_config*);
void liberar_metadata(t_config* config, int socket_discord);

void signal_compactacion(int sig);
void signal_dump(int sig);

// void liberar_segmentos();
// void liberar_patotas();
// void liberar_tareas();
// void liberar_tripulantes();

#endif /* _MIRAMHQ_H_ */