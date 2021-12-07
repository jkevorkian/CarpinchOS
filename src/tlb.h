#ifndef _TLB_H_
#define _TLB_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>   // sprintf
#include <arpa/inet.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils/sockets.h>

#include "memoria.h"
#include "carpincho.h"

typedef struct {
	uint32_t id_car;
	uint32_t pagina;
	uint32_t marco;
    char* tiempo_lru;
    // pthread_mutex_t mutex;
} t_entrada_tlb;

typedef struct {
    uint32_t id_proceso;
    uint32_t cant_hit;
    uint32_t cant_miss;
} t_tlb_por_proceso;

typedef struct {
	uint32_t cant_hit;
    uint32_t cant_miss;
	uint32_t cant_entradas;
    uint32_t algoritmo_reemplazo;
    uint32_t retardo_acierto;
    uint32_t retardo_fallo;
    uint32_t puntero_fifo;
    char* path_dump;
    t_entrada_tlb** mapa;
    t_list* hit_miss_proceso; 
} t_tlb;

t_tlb 				tlb;

void 				iniciar_tlb(t_config* config);
t_entrada_tlb*		solicitar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina);
t_entrada_tlb* 		asignar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina);
void                borrar_pagina_carpincho_tlb(uint32_t id_carpincho, uint32_t nro_pagina);
void 				borrar_entrada_tlb(uint32_t nro_entrada);
t_entrada_tlb*		es_entrada(uint32_t, uint32_t, uint32_t);
void                entrada_nueva(uint32_t id_carpincho, uint32_t nro_pagina, t_entrada_tlb* entrada);
uint32_t			leer_tlb(uint32_t id_carpincho, uint32_t nro_pagina);
t_entrada_tlb*		leer_tlb2(uint32_t id_carpincho, uint32_t nro_pagina);

void 				obtener_control_tlb();
void 				liberar_control_tlb();

t_tlb_por_proceso* 	get_hit_miss_proceso(uint32_t id_carpincho);
void                flush_proceso_tlb(uint32_t id_carpincho);

//LRU
bool                es_mas_vieja(t_entrada_tlb* entrada1, t_entrada_tlb* entrada2);
uint32_t            tiempo_a_milisegundos(t_entrada_tlb* entrada);
uint32_t            obtener_tiempo_lru(char tipo, t_entrada_tlb* entrada);
//void                print_tiempo(t_entrada_tlb* entrada);

// SEÑALES
void 				print_tlb(void);
void 				resetear_tlb(void);
void 				print_hit_miss(void);
void 				cant_hit_carpincho(void* item);
void 				cant_miss_carpincho(void* item);

// Pato
void actualizar_entrada_tlb(t_entrada_tlb* entrada_tlb, uint32_t id_carpincho, uint32_t nro_pagina);


#endif /* _TLB_H_ */
