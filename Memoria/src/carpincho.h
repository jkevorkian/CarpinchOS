#ifndef _CARPINCHO_H_
#define _CARPINCHO_H_

//#include <utils/sockets.h>
#include "memoria.h"

typedef struct {
	int socket;
} data_carpincho;

typedef enum {
	MEM_ALLOC,
	MEM_FREE,
	MEM_READ,
	MEM_WRITE,
} op_code;

void*			rutina_carpincho(void* info_carpincho);
t_carpincho* 	crear_carpincho(uint32_t);
bool 			asignacion_fija(t_carpincho*);
bool 			asignacion_global(t_carpincho*);

#endif /* _CARPINCHO_H_ */
