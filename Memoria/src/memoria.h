#ifndef _MEMORIA_H_
#define _MEMORIA_H_

#include <utils/sockets.h>
#include <semaphore.h>
#include <signal.h>
<<<<<<< HEAD
#include "servidor.h"
#include "tlb.h"

void signal_handler_1(int);
void signal_handler_2(int);
void signal_handler_3(int);
=======
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <math.h>

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

typedef struct {
    uint32_t tamanio_memoria;
    uint32_t tamanio_pagina;
    uint32_t algoritmo_reemplazo;
    uint32_t tipo_asignacion;
    uint32_t cant_marcos;
} t_config_memoria;

typedef struct __attribute__((packed)){
	uint32_t alloc_prev;
    uint32_t alloc_sig;
    bool libre;
} t_heap_metadata;

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
>>>>>>> 6c0d826f3802c0cb7bad2a60716508bb6a355d32


// Agrego id_carpincho
typedef struct {
	uint32_t id_carpincho;
    uint32_t nro_pagina;
    uint32_t nro_marco;    
    bool presencia;
    bool uso;
    bool modificado;
    uint32_t tiempo;
} t_entrada_tp;

// Cambio tabla de paginas
typedef struct {
    uint32_t id;
	sem_t* sem_tlb;
	t_entrada_tp **tabla_paginas;
    // t_list* tabla_paginas;
} t_carpincho;

t_list* lista_carpinchos;
uint32_t cant_carpinchos;	// TODO crear función para obtener

<<<<<<< HEAD
=======
bool iniciar_memoria(t_config*);
void iniciar_marcos(uint32_t);
void iniciar_heap(void);

void signal_handler_1(int);
void signal_handler_2(int);
void signal_handler_3(int);

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void terminar_programa(t_log*, t_config*);

//MEM_ALLOC
uint32_t mem_alloc(t_carpincho*, uint32_t);
t_marco* obtener_marco_libre();
uint32_t cant_frames_necesarios(uint32_t);
t_entrada_tp* crear_nueva_pagina(uint32_t);
t_heap_metadata* buscar_alloc_libre(uint32_t carpincho_id);

t_memoria_ram memoria_ram;

t_log* logger;
t_config* config;
t_config_memoria config_memoria;

>>>>>>> 6c0d826f3802c0cb7bad2a60716508bb6a355d32
#endif /* _MEMORIA_H_ */
