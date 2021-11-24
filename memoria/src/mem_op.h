/*
 * mem_op.h
 *
 *  Created on: 8 nov. 2021
 *      Author: utnso
 */

#ifndef MEM_OP_H_
#define MEM_OP_H_

#include "read_write_op.h"

uint32_t mem_alloc(uint32_t carpincho, uint32_t tamanio);
bool mem_free(uint32_t id_carpincho, uint32_t dir_logica);
void *mem_read(uint32_t id_carpincho, uint32_t dir_logica);
bool mem_write(uint32_t id_carpincho, uint32_t dir_logica, void* contenido);

uint32_t heap_header(t_carpincho* carpincho, uint32_t tam, uint32_t desplazamiento);
uint32_t heap_footer(t_carpincho* carpincho, uint32_t tam, uint32_t desplazamiento, uint32_t alloc_sig);

void liberar_paginas_carpincho(uint32_t id_carpincho, uint32_t desplazamiento);

#endif /* MEM_OP_H_ */