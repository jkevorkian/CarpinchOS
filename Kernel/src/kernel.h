#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <utils/sockets.h>

t_log* logger;
t_config* config;

char* ip_memoria;
int socket_memoria;

char* algoritmo_planificacion;
int grado_multiprogramacion, grado_multiprocesamiento, alfa, estimacion_inicial;

#endif /* KERNEL_H_ */
