#ifndef UTILIDADES_H_
#define UTILIDADES_H_

#include "global.h"
#include "colas_planificacion.h"

int obtener_rafaga_real(char *tiempo_inicio, char *tiempo_fin);
double obtener_estimacion_proxima_rafaga(int rafaga_real, int estimacion);
void liberar_split(char** split);
int buscar_semaforo(char* nombre);
int buscar_io(char* nombre);
int buscar(t_list *lista, char* nombre);
void iniciar_semaforo(char* nombre, int valor);
float calcular_HRRN(carpincho* carp, char* tiempo_actual);
int encontrar_carpincho(t_list *lista, carpincho *carp_quitar);
void desbloquear(carpincho* carp);
semaforo* buscar_sem_por_id(t_list *lista, int id);
void hacer_post_semaforo(semaforo* sem);

#endif /* UTILIDADES_H_ */
