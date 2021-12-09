#ifndef DEADLOCK_H_
#define DEADLOCK_H_

#include <time.h>

#include "global.h"
#include "utilidades.h"




int iniciar_deteccion_deadlock(int tiempo_deadlock);
void *detectar_deadlock(void* d);
bool tiene_asignado(carpincho *carp, int id_semaforo);
bool *ordenador_carpinchos(carpincho* carp1, carpincho* carp2);
int matar_proximo_carpincho(t_list *carpinchos_deadlock);

void algoritmo_deteccion();

t_list *carpinchos_bloqueados_sem();

t_list *carps_en_deadlock();
bool esta_en_deadlock(carpincho *carp, t_list* cadena_de_deadlock);
carpincho* carpincho_de_lista_con_sem_bloq_asignado(t_list* lista_carpinchos, carpincho* carp);
#endif /* DEADLOCK_H_ */
