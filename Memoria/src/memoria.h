#ifndef _MEMORIA_H_
#define _MEMORIA_H_

#include <utils/sockets.h>
#include <semaphore.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <math.h>
#include "servidor.h"
#include "tlb.h"

//#include "servidor.h"
//#include "tlb.h"
//#include <utils/sockets.h>

#define IP_RAM "127.0.0.1"
#define CONSOLA_ACTIVA  1

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
} t_config_memoria;

typedef struct{
	uint32_t nro_real;
    bool libre;
    bool bit_uso;
    bool bit_modificado;
    char *temporal;
} t_marco;

typedef struct {
    void* inicio;
    t_marco** mapa_fisico;
    uint32_t puntero_clock;
} t_memoria_ram;

typedef struct {
	uint32_t id_carpincho;
    uint32_t nro_pagina;
    uint32_t nro_marco;    
    bool presencia;
    bool uso;
    bool modificado;
    uint32_t tiempo;
} t_entrada_tp;

typedef struct {
    uint32_t id;
	sem_t* sem_tlb;
	t_list* tabla_paginas;
} t_carpincho;

t_list*             lista_carpinchos;
uint32_t            cant_carpinchos;	// TODO crear función para obtener

bool                iniciar_memoria(t_config*);
void                iniciar_marcos(uint32_t);

//MEM_ALLOC

//MEM_FREE
bool                mem_free(uint32_t id_carpincho, uint32_t dir_logica);

// MARCOS
bool                tengo_marcos_suficientes(uint32_t);
// TODO: son lo mismo? elegir una
t_marco*            obtener_marco_libre();
t_marco*            obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina);

uint32_t            cant_marcos_necesarios(uint32_t);

// PAGINAS
t_entrada_tp*       crear_nueva_pagina(uint32_t, t_carpincho*);
uint32_t            pagina_segun_posicion(uint32_t posicion);
uint32_t            offset_segun_posicion(uint32_t posicion);

// TODO: mover a CARPINCHOS
t_carpincho*        carpincho_de_lista(uint32_t id_carpincho);

// HEAP METADATA
bool                get_isFree(uint32_t id_carpincho, uint32_t dir_logica_heap);
void                set_isFree(uint32_t id_carpincho, uint32_t dir_logica_heap);
void                reset_isFree(uint32_t id_carpincho, uint32_t dir_logica_heap);

uint32_t            get_prevAlloc(uint32_t id_carpincho, uint32_t dir_logica_heap);
uint32_t            get_nextAlloc(uint32_t id_carpincho, uint32_t dir_logica_heap);

void                set_prevAlloc(uint32_t id_carpincho, uint32_t dir_logica_heap, uint32_t nuevo_valor);
void                set_nextAlloc(uint32_t id_carpincho, uint32_t dir_logica_heap, uint32_t nuevo_valor);

t_memoria_ram memoria_ram;

t_log* logger;
t_config* config;
t_config_memoria config_memoria;

#endif /* _MEMORIA_H_ */
