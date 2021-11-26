#ifndef KERNEL_H_
#define KERNEL_H_

#include "global.h"
#include "inicializador.h"
#include "colas_planificacion.h"
#include "deadlock.h"

int conectar_memoria();
int crear_socket_carpincho(int socket_auxiliar_carpincho);

#endif /* KERNEL_H_ */
