#ifndef _HILOS_H_
#define _HILOS_H_

#include <commons/collections/list.h>
#include <stdint.h>
#include <commons/log.h>
#include <semaphore.h>

#include "memoria_ram.h"
#include "segmentos.h"
#include <utils/utils-mensajes.h>
#include <utils/utils-sockets.h>
#include "tripulante.h"

void* rutina_hilos(void* data);

typedef enum {
    NEW,
    BLOCKED,
    READY,
    RUNNING,
    EXIT,
    EMERGENCY
}estado;

char caracter_de_estado(estado nuevo_estado);

#endif /* _HILOS_H_ */