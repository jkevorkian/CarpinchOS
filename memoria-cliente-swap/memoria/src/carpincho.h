#ifndef _CARPINCHO_H_
#define _CARPINCHO_H_

#include "mem_op.h"

typedef struct {
	int socket;
	int id;
} data_carpincho;

void*			rutina_carpincho(void* info_carpincho);
t_carpincho* 	crear_carpincho(uint32_t);
bool 			asignacion_fija(t_carpincho*);
bool 			asignacion_global(t_carpincho*);

void rutina_test_carpincho(data_carpincho *info_carpincho);

#endif /* _CARPINCHO_H_ */

