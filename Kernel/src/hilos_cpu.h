#ifndef HILOS_CPU_H_
#define HILOS_CPU_H_

#include "global.h"
#include "colas_planificacion.h"

void iniciar_hilos_cpu();
void* cpu();

int obtener_rafaga_real(char *tiempo_inicio, char *tiempo_fin);
double obtener_estimacion_proxima_rafaga(int rafaga_real, int estimacion);
void liberar_split(char** split);

#endif /* HILOS_CPU_H_ */
