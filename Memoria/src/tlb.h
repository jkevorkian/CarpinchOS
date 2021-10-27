#ifndef _TLB_H_
#define _TLB_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>   // sprintf
#include <arpa/inet.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <utils/sockets.h>
#include "memoria.h"

typedef struct {
	uint32_t id_car;
	uint32_t cant_paginas;
	uint32_t *paginas;
} tabla_paginas;

typedef struct {
	uint32_t id_car;	// si id_car == 0, entonces está vacía la entrada
	uint32_t pagina;
	uint32_t marco;
} t_entrada_tlb;

typedef struct {
    uint32_t algoritmo_reemplazo;
    uint32_t retardo_acierto;
    uint32_t retardo_fallo;   
	uint32_t cant_entradas;
    t_entrada_tlb** mapa;
} t_tlb;

t_tlb tlb;

t_entrada_tlb *solicitar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina);
void asignar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina);
void borrar_entrada_tlb(uint32_t nro_entrada);
void obtener_control_tlb();
void liberar_control_tlb();
t_entrada_tlb *es_entrada(uint32_t, uint32_t, uint32_t);

#endif /* _TLB_H_ */
