/*
 * memoria2.h
 *
 *  Created on: 31 oct. 2021
 *      Author: utnso
 */

#ifndef SRC_MEMORIA2_H_
#define SRC_MEMORIA2_H_

#include <stdbool.h>
#include "swap.h"
#include "memoria.h"

#define TAMANIO_HEAP 9
#define HEAP_NULL 0xFFFFFFFF

bool mem_free(uint32_t id_carpincho, uint32_t dir_logica);

uint32_t pagina_segun_posicion(uint32_t posicion);
uint32_t offset_segun_posicion(uint32_t posicion);

t_marco *obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina);

t_carpincho *carpincho_de_lista(uint32_t id_carpincho);

#endif /* SRC_MEMORIA2_H_ */
