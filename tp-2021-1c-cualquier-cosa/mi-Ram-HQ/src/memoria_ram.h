#ifndef _MEMORIA_RAM_H_
#define _MEMORIA_RAM_H_

#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include <stdint.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>

#define IP_RAM "127.0.0.1"
#define CONSOLA_ACTIVA  1

#define SEGMENTACION 0
#define PAGINACION 1

#define FIRST_FIT 0
#define BEST_FIT 1

#define LRU 0
#define CLOCK 1

typedef struct {
    uint32_t nro_virtual;
    uint32_t nro_real;
    uint32_t duenio;
    uint32_t tiempo;    // LRU
    bool bit_uso;   // CLOCK
    // uint32_t espacio_libre;
    bool presencia;
    bool modificado;
    sem_t semaforo_mutex;
} t_marco;

typedef struct {
    void* inicio;
    uint32_t tamanio_memoria;
    uint32_t esquema_memoria;
    // Segmentación
    uint32_t criterio_seleccion;
    t_list* mapa_segmentos;
    // Paginación
    uint32_t tamanio_pagina;
    t_marco** mapa_logico;
    t_marco** mapa_fisico;
    
    uint32_t algoritmo_reemplazo;
    uint32_t puntero_clock;
    // t_bitarray* bitmap;
    // FILE* inicio_swap;
    // void* inicio_swap;
    int fd_swap;
    uint32_t tamanio_swap;
    
} t_memoria_ram;

typedef struct {
    uint32_t PID;
    uint32_t cantidad_elementos;
    uint32_t* inicio_elementos;
    // Segmentacion
    // Paginacion
    uint32_t memoria_ocupada;
    uint32_t cant_frames;
    uint32_t* frames;
} patota_data;

typedef struct {
    uint32_t PID;
    uint32_t TID;
    uint32_t pos_x;
    uint32_t pos_y;
    bool seguir;
} t_movimiento;

t_list* lista_patotas;
t_list* lista_tareas;
t_list* lista_tripulantes;
t_list* movimientos_pendientes;

t_memoria_ram memoria_ram;

t_log* logger;

sem_t semaforo_consola;
sem_t mutex_movimiento;
sem_t mutex_lista_tripulantes;
sem_t mutex_lista_segmentos;
sem_t mutex_incorporar_marco;
sem_t mutex_compactacion;

#define TAMANIO_PAGINA memoria_ram.tamanio_pagina

#endif /* _MEMORIA_RAM_H_ */