#ifndef _MARCOS_H_
#define _MARCOS_H_

#include <commons/temporal.h>
#include <pthread.h>

#include "memoria.h"
#include "carpincho.h"
#include "swap.h"

bool tengo_marcos_suficientes(uint32_t);

// TODO: son lo mismo? elegir una
// SON DISTINTOS, OBTENER MARCO DEVUELVE UN MARCO PREVIAMENTE ASIGNADO
t_marco* obtener_marco_libre();
t_marco* obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina);

uint32_t cant_marcos_necesarios(uint32_t);

void reasignar_marco(uint32_t id_carpincho, uint32_t nro_pagina, t_marco* marco);
void actualizar_info_algoritmo(t_marco *marco_auxiliar, bool modificado);

void soltar_marco(t_marco *marco_auxiliar);
void reservar_marco(t_marco *marco_auxiliar);


#endif /* _MARCOS_H_ */
