/*
 * mem_op.h
 *
 *  Created on: 8 nov. 2021
 *      Author: utnso
 */

#ifndef MEM_OP_H_
#define MEM_OP_H_

#include "memoria.h"
#include "read_write_op.h"

//MEM_ALLOC
uint32_t mem_alloc(uint32_t carpincho, uint32_t tamanio);
//MEM_FREE
bool mem_free(uint32_t id_carpincho, uint32_t dir_logica);

#endif /* MEM_OP_H_ */
