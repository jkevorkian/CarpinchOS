#ifndef _MEMORIA_H_
#define _MEMORIA_H_

#include <stdlib.h>
#include <utils/sockets.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

// ALGORITMOS REEMPLAZO 
#define LRU 0
#define CLOCK 1
#define FIFO 2

// TIPO ASIGNACION
#define FIJA_LOCAL 0
#define DINAMICA_GLOBAL 1

#define TAMANIO_HEAP 9
#define HEAP_NULL 0xFFFFFFFF

typedef struct {
    uint32_t tamanio_memoria;
    uint32_t tamanio_pagina;
    uint32_t algoritmo_reemplazo;
    uint32_t tipo_asignacion;
    uint32_t cant_marcos;
    uint32_t cant_marcos_carpincho;
} t_config_memoria;

typedef struct{
    uint32_t nro_real;
    
	uint32_t duenio;
	uint32_t pagina_duenio;
    bool libre;
    pthread_mutex_t mutex_espera_uso;
    // Para reemplazo de paginas
    bool bit_uso;
    bool bit_modificado;
    char *temporal;         // "%H:%M:%S:%MS" => "12:51:59:331"
    pthread_mutex_t mutex_info_algoritmo;
} t_marco;

typedef struct {
    void* inicio;
    t_marco** mapa_fisico;
    // uint32_t puntero_clock;
} t_memoria_ram;

typedef struct {
	bool presencia;		// demuestra que está actualizada la entrada
	bool esta_vacia;
    uint32_t nro_marco;
    pthread_mutex_t mutex;
} t_entrada_tp;

typedef struct {
    uint32_t id;
	t_list* tabla_paginas;
    pthread_mutex_t mutex_tabla;
    
    uint32_t offset;
    // TLB
    sem_t* sem_tlb; // Revisar
    uint32_t cant_hit;
    uint32_t cant_miss;
} t_carpincho;

pthread_mutex_t mutex_asignacion_marcos;

t_memoria_ram memoria_ram;
t_config_memoria config_memoria;

t_config *config;
t_log* logger;

pthread_mutex_t mutex_lista_carpinchos;
t_list* lista_carpinchos;

void*			inicio_memoria(uint32_t nro_marco, uint32_t offset);

bool			iniciar_memoria(t_config*);
void		    iniciar_marcos(uint32_t);

t_carpincho*	carpincho_de_lista(uint32_t id_carpincho);

void*           dir_fisica_proceso(t_list* tabla_paginas);

void loggear_pagina(t_log *logger, void *pagina);

t_marco** obtener_marcos_proceso(uint32_t id_carpincho, uint32_t *nro_marcos_encontrados);
uint32_t nro_paginas_reemplazo();

t_entrada_tp *pagina_de_carpincho(uint32_t id, uint32_t nro_pagina);

#endif /* _MEMORIA_H_ */
