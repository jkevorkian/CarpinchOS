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

#define ip_kernel "127.0.0.1"
#define ip_memoria "127.0.0.1"
#define puerto_kernel "10216"
#define puerto_memoria "11355"  //TODO: este puerto fue elegido al azar y no funciona, para mas placer

t_log *logger;


typedef struct {
	int socket;
} mate_instance;



typedef int32_t mate_pointer;

enum mate_errors {
	MATE_FREE_FAULT = -5, MATE_READ_FAULT = -6, MATE_WRITE_FAULT = -7
};

// TODO: Docstrings

//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config);

int mate_close(mate_instance *lib_ref);

//-----------------Semaphore Functions---------------------/
/*
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value);

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem);

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg);
*/

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size);

int mate_memfree(mate_instance *lib_ref, mate_pointer addr);

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size);

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size);

#endif /* MATELIB_H_ */
