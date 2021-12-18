#ifndef _REEMPLAZOS_H_
#define _REEMPLAZOS_H_

#include "memoria.h"
#include <commons/temporal.h>

t_marco *buscar_por_clock(t_marco **lista_paginas, uint32_t nro_paginas);

t_marco *buscar_por_lru(uint32_t id, uint32_t pagina);

void actualizar_info_algoritmo(t_marco *marco_auxiliar, bool modificado);

bool primer_tiempo_mas_chico(char *tiempo1, char *tiempo2);

uint32_t obtener_tiempo(char tipo, char *tiempo);

#endif /* _REEMPLAZOS_H_ */
