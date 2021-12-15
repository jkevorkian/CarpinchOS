#ifndef MATELIB_H_
#define MATELIB_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <commons/string.h>

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/temporal.h>
#include <utils/sockets.h>
#include <commons/temporal.h>

#define ip_kernel "127.0.0.1"
#define puerto_kernel "10216"

//carpincho
typedef struct {
	int socket;
	int id;
}mateLib;

t_log *logger;

char* leer_consola();
void* respuesta(void* s);

#endif /* MATELIB_H_ */
