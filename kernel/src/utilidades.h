#ifndef UTILIDADES_H_
#define UTILIDADES_H_

#include "global.h"
#include "colas_planificacion.h"

int obtener_rafaga_real(char *tiempo_inicio, char *tiempo_fin);
double obtener_estimacion_proxima_rafaga(int rafaga_real, int estimacion);
void liberar_split(char** split);
void iniciar_semaforo(char* nombre, int valor);
float calcular_HRRN(carpincho* carp, char* tiempo_actual);
void hacer_posts_semaforo(sem_deadlock *semaforo_asignado, carpincho* carp);
void liberar_lista(t_list* lista);

int buscar_semaforo(char* nombre);
int buscar_io(char* nombre);
int encontrar_carpincho(t_list *lista, carpincho *carp_quitar);
int buscar_sem_en_lista(t_list *lista, char* nombre);
semaforo* buscar_sem_por_id(t_list *lista, int id);
bool no_tiene_asignado_este_semaforo(carpincho* carp,semaforo* sem);

void desbloquear(carpincho* carp);
void* informador();

#endif /* UTILIDADES_H_ */
