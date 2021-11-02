#include "global.h"
#include <time.h>

typedef struct{
        int milisegundos_entre_detecciones;
        void* semaforosMaybe;
} t_deadlock;

pthread_t detector;