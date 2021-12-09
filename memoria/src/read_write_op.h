#ifndef _READ_WRITE_OP_H_
#define _READ_WRITE_OP_H_

#include "marcos.h"

void* obtener_bloque_paginacion(uint32_t id, uint32_t desplazamiento, uint32_t tamanio);
void actualizar_bloque_paginacion(uint32_t id, uint32_t desplazamiento, void* data, uint32_t tamanio);

bool get_isFree(uint32_t id_carpincho, uint32_t dir_logica_heap);
void set_isFree(uint32_t id_carpincho, uint32_t dir_logica_heap);
void reset_isFree(uint32_t id_carpincho, uint32_t dir_logica_heap);

uint32_t get_prevAlloc(uint32_t id_carpincho, uint32_t dir_logica_heap);
uint32_t get_nextAlloc(uint32_t id_carpincho, uint32_t dir_logica_heap);

void set_prevAlloc(uint32_t id_carpincho, uint32_t dir_logica_heap, uint32_t nuevo_valor);
void set_nextAlloc(uint32_t id_carpincho, uint32_t dir_logica_heap, uint32_t nuevo_valor);

#endif /* _READ_WRITE_OP_H_ */
