#ifndef _SWAP_H_
#define _SWAP_H_

#include "utils/sockets.h"
#include <commons/collections/queue.h>
#include <commons/temporal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>

#include "memoria.h"
#include "marcos.h"

typedef struct {
	uint32_t accion;
	uint32_t id_carpincho;
	uint32_t nro_pagina;
	char *buffer;

	sem_t sem_respuesta;
	bool respuesta;
}t_movimiento;

// sem_t *sem_movimiento;
// sem_t *sem_respuesta;
sem_t sem_movimiento;
sem_t sem_respuesta;

t_queue *movimientos_pendientes;
pthread_mutex_t mutex_movimientos;

void* manejar_swap(void* socket_swap);
bool crear_movimiento_swap(uint32_t, uint32_t, uint32_t, char *);

t_marco *incorporar_pagina(uint32_t id_carpincho, uint32_t nro_pagina);

uint32_t obtener_tiempo(char tipo, t_marco *marco);

void* manejar_test_swap(void* socket_swap);

t_marco** obtener_marcos_proceso(uint32_t id_carpincho, uint32_t *nro_marcos_encontrados);

#endif /* _SWAP_H_ */
