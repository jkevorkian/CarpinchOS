#ifndef _TLB_H_
#define _TLB_H_

#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>   // sprintf
#include <arpa/inet.h>
#include <commons/string.h>
#include <sys/stat.h>

#include "reemplazos.h"

typedef struct {
    uint32_t nro_entrada;
	uint32_t id;
	uint32_t pagina;
	uint32_t marco;
    char* tiempo_lru;
    pthread_mutex_t mutex;
} t_entrada_tlb;

typedef struct {
    uint32_t id_carpincho;
    uint32_t cant_hit;
    uint32_t cant_miss;
} t_hit_miss_tlb;

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

t_tlb tlb;
t_list *cola_fifo_tlb;
pthread_mutex_t mutex_fifo_tlb;
pthread_mutex_t mutex_asignacion_tlb;

t_list *lista_lru_tlb;
pthread_mutex_t mutex_lista_lru_tlb;

t_list *historico_hit_miss;
pthread_mutex_t mutex_historico_hit_miss;

void 				iniciar_tlb(t_config*);
// t_entrada_tlb* 		asignar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina);
void                borrar_pagina_carpincho_tlb(uint32_t id_carpincho, uint32_t nro_pagina);

// void                entrada_nueva(uint32_t id_carpincho, uint32_t nro_pagina, t_entrada_tlb* entrada);

t_entrada_tlb*		leer_tlb(t_entrada_tp *entrada_tp);

void 				obtener_control_tlb();
void 				liberar_control_tlb();

// SENIALES
void 				print_tlb(void);
void 				resetear_tlb(void);
void 				print_hit_miss(void);
void 				cant_hit_carpincho(void* item);
void 				cant_miss_carpincho(void* item);

t_entrada_tlb* reemplazar_entrada_tlb(t_entrada_tp *entrada_vieja_tp, t_entrada_tp *entrada_nueva_tp);

void entrada_nueva(t_entrada_tlb*, t_entrada_tp *);
t_entrada_tlb* asignar_entrada_tlb(t_entrada_tp *);

void loggear_tlb();

#endif /* _TLB_H_ */
