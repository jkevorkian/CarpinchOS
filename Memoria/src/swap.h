#ifndef _SWAMP_H_
#define _SWAMP_H_

#include "utils/sockets.h"
#include <commons/collections/queue.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#include "memoria.h"

typedef struct {
	uint8_t accion;
	uint32_t id_carpincho;
	uint32_t nro_pagina;
	char *buffer;
}t_movimiento;

sem_t sem_movimiento;
t_queue *movimientos_pendientes;
pthread_mutex_t mutex_movimientos;

void* manejar_swap(void* socket_swap);
void crear_movimiento_swap(uint8_t, uint32_t, uint32_t, char *);

#endif /* _SWAMP_H_ */
