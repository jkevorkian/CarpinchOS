#include "global.h"
#include "utilidades.h"
#include <time.h>

typedef struct{
        int milisegundos_entre_detecciones;
        void* semaforosMaybe;
} t_deadlock;

t_list *carpinchos_en_deadlock;
t_list *lista_a_evaluar;

pthread_t detector;