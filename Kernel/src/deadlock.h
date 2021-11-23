#include "global.h"
#include "utilidades.h"
#include <time.h>

typedef struct{
        int milisegundos_entre_detecciones;
        void* semaforosMaybe;
} t_deadlock;

t_list *carpinchos_en_deadlock;
t_list *lista_a_evaluar;

int iniciar_deteccion_deadlock(void *semaforosMaybe, int tiempo_deadlock);
void detectar_deadlock(t_deadlock *deadlock);
bool tiene_asignado(carpincho *carp, int id_semaforo);
bool es_bloqueado_por_algun_semaforo(carpincho *carp, t_list *lista_de_semaforos);
bool cumple_condiciones_deadlock(carpincho *carp);
bool ordenador_carpinchos(carpincho carp1, carpincho carp2);
int matar_proximo_carpincho(t_list *carpinchos_deadlock);
int finalizar_deteccion_deadlock();
bool esta_en_deadlock(carpincho *carp);
int algoritmo_deteccion();







pthread_t detector;