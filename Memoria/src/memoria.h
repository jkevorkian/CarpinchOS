#ifndef _MEMORIA_H_
#define _MEMORIA_H_

#include <utils/sockets.h>
#include <semaphore.h>
#include <signal.h>
#include "servidor.h"
#include "tlb.h"

void signal_handler_1(int);
void signal_handler_2(int);
void signal_handler_3(int);

typedef struct {
	sem_t *sem_tlb;
}t_carpincho;

t_list *lista_carpinchos;
uint32_t cant_carpinchos;	// TODO crear función para obtener

#endif /* _MEMORIA_H_ */
