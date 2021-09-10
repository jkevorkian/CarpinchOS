#ifndef _PAGINAS_H_
#define _PAGINAS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "memoria_ram.h"
#include "logs.h"

#include <utils/utils-mensajes.h>
#include <commons/config.h>

// #define ENTERO 0
// #define CARACTER 1
// #define BUFFER 2

bool iniciar_memoria_paginada(t_config* config);

uint32_t marcos_logicos_disponibles();
uint32_t marcos_reales_disponibles();

uint32_t frames_necesarios(uint32_t memoria_libre_ultimo_frame, uint32_t tamanio);
void asignar_frames(uint32_t id_patota, uint32_t cant_frames);
void reasignar_frames(uint32_t id_patota);

t_marco* obtener_marco_libre_fisico();
t_marco* obtener_marco_libre_virtual();

void* inicio_marco(uint32_t nro_marco);
uint32_t inicio_marco_logico(uint32_t nro_marco);
void borrar_marco(uint32_t nro_marco);

void incorporar_marco(uint32_t nro_marco);
t_marco* reemplazo_por_lru(uint32_t nro_marco);
t_marco* reemplazo_por_clock(uint32_t nro_marco);

void actualizar_entero_paginacion(uint32_t id_patota, uint32_t desplazamiento, uint32_t valor);
uint32_t obtener_entero_paginacion(uint32_t id_patota, uint32_t desplazamiento);

void* obtener_bloque_paginacion(uint32_t id_patota, uint32_t desplazamiento, uint32_t tamanio);
void actualizar_bloque_paginacion(uint32_t id_patota, uint32_t desplazamiento, void* data, uint32_t tamanio);

uint32_t nro_pagina_de_patota(uint32_t patota, uint32_t nro_marco);

#endif /* _PAGINAS_H_ */