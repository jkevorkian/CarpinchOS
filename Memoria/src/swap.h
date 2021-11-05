#ifndef _SWAMP_H_
#define _SWAMP_H_

#include "utils/sockets.h"
#include <commons/collections/queue.h>
#include <commons/temporal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#include "memoria.h"

typedef struct {
	uint32_t accion;
	uint32_t id_carpincho;
	uint32_t nro_pagina;
	char *buffer;
}t_movimiento;

sem_t sem_movimiento;
t_queue *movimientos_pendientes;
pthread_mutex_t mutex_movimientos;

void* manejar_swap(void* socket_swap);
void crear_movimiento_swap(uint8_t, uint32_t, uint32_t, char *);

t_marco *realizar_algoritmo_reemplazo(uint32_t id_carpincho);

void actualizar_info_algoritmo(t_marco *marco_auxiliar, bool modificado);

#endif /* _SWAMP_H_ */
