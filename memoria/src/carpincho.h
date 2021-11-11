#ifndef _CARPINCHO_H_
#define _CARPINCHO_H_

//#include <utils/sockets.h>
#include "memoria.h"
#include "marcos.h"
#include "mem_op.h"

typedef struct {
	int socket;
	int id;
} data_carpincho;

/*
typedef struct {
    uint32_t id;
	sem_t* sem_tlb;
	t_list* tabla_paginas;
} t_carpincho;
*/

void*			rutina_carpincho(void* info_carpincho);
t_carpincho* 	crear_carpincho(uint32_t);
bool 			asignacion_fija(t_carpincho*);
bool 			asignacion_global(t_carpincho*);


#endif /* _CARPINCHO_H_ */

