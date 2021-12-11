#ifndef _MARCOS_H_
#define _MARCOS_H_

#include "swap.h"
#include "tlb.h"

t_marco* obtener_marco_libre();

t_marco* obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina);
t_marco *incorporar_pagina(t_entrada_tp *entrada_tp);

uint32_t cant_marcos_faltantes(uint32_t id, uint32_t tamanio);

void reasignar_marco(t_marco *, t_entrada_tp *);

void soltar_marco(t_marco *);
void reservar_marco(t_marco *);

bool asignacion_fija(t_carpincho*);

void suspend(uint32_t id);
void unsuspend(uint32_t id);

t_entrada_tp* agregar_pagina(uint32_t id_carpincho);
void eliminar_pagina(uint32_t id, uint32_t nro_pagina);

void liberar_paginas_carpincho(uint32_t id_carpincho, uint32_t desplazamiento);

t_entrada_tlb *actualizar_entradas(t_marco *marco, t_entrada_tp *entrada_tp);

#endif /* _MARCOS_H_ */
