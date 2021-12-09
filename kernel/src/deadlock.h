#ifndef DEADLOCK_H_
#define DEADLOCK_H_

#include <time.h>

#include "global.h"
#include "utilidades.h"




int iniciar_deteccion_deadlock(int tiempo_deadlock);
void *detectar_deadlock(void* d);
bool tiene_asignado(carpincho *carp, int id_semaforo);
//bool es_bloqueado_por_algun_semaforo(carpincho *carp, t_list *lista_de_semaforos);
bool *ordenador_carpinchos(carpincho* carp1, carpincho* carp2);
int matar_proximo_carpincho(t_list *carpinchos_deadlock);
int finalizar_deteccion_deadlock();

void algoritmo_deteccion();

t_list *carpinchos_que_cumplen_condicion();
bool cumple_condiciones_deadlock(carpincho *carp);

t_list *carps_en_deadlock();
bool esta_en_deadlock(carpincho *carp, t_list* cadena_de_deadlock);
carpincho* carpincho_de_lista_con_sem_bloq_asignado(t_list* lista_carpinchos, carpincho* carp);
#endif /* DEADLOCK_H_ */
