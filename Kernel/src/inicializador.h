#ifndef INICIALIZADOR_H_
#define INICIALIZADOR_H_

#include "global.h"
#include "planificadores.h"
#include "hilos_cpu.h"
#include "IO.h"
#include "utilidades.h"

int inicializar_kernel();

void leer_configuraciones();
void crear_estructuras_planificacion();
void inicializar_semaforos_planificacion();

#endif /* INICIALIZADOR_H_ */
