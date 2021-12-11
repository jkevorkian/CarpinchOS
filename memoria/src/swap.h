#ifndef _SWAP_H_
#define _SWAP_H_

#include <commons/collections/queue.h>
#include <unistd.h>

#include "memoria.h"

typedef struct {
	uint32_t accion;
	uint32_t id_carpincho;
	uint32_t nro_pagina;
	char *buffer;

	sem_t sem_respuesta;
	bool respuesta;
} t_movimiento;

sem_t sem_movimiento;
sem_t sem_respuesta;

t_queue *movimientos_pendientes;
pthread_mutex_t mutex_movimientos;

void* manejar_swap(void* socket_swap);
bool crear_movimiento_swap(uint32_t, uint32_t, uint32_t, char *);

#endif /* _SWAP_H_ */
