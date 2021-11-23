#ifndef _MEMORIA_H_
#define _MEMORIA_H_

#include <utils/sockets.h>
#include <semaphore.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <math.h>
#include <stdbool.h>

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
	uint32_t duenio;		// 0 si está libre
	uint32_t pagina_duenio;	// sirve para poder modificar la tabla de paginas del proceso, marcando presencia en false
	uint32_t nro_real;
    bool libre;				// to remove o cambiar lo de duenio
    bool bit_uso;
    bool bit_modificado;
    char *temporal;
    pthread_mutex_t mutex;	// TODO instanciar al inicializar memoria
} t_marco;

typedef struct {
    void* inicio;
    t_marco** mapa_fisico;
    pthread_mutex_t mutex_mapa;
    // uint32_t puntero_clock;
} t_memoria_ram;

typedef struct {
	bool presencia;		// demuestra que está actualizada la entrada
	uint32_t nro_marco;
} t_entrada_tp;

typedef struct {
    uint32_t id;
	sem_t* sem_tlb;
	t_list* tabla_paginas;
} t_carpincho;

// pthread_t nuevo_carpincho;

t_memoria_ram memoria_ram;
t_config* config;			// Creo que no tiene que ser global
t_config_memoria config_memoria;

t_log* logger;

t_list* lista_carpinchos;

// TOREMOVE ya existe list_size
uint32_t cant_carpinchos;	// TODO crear función para obtener

void*			inicio_memoria(uint32_t nro_marco, uint32_t offset);

bool			iniciar_memoria(t_config*);
void		    iniciar_marcos(uint32_t);

uint32_t		pagina_segun_posicion(uint32_t posicion);
uint32_t		offset_segun_posicion(uint32_t posicion);
t_carpincho*	carpincho_de_lista(uint32_t id_carpincho);

#endif /* _MEMORIA_H_ */
