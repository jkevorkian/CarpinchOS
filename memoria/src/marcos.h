#ifndef _MARCOS_H_
#define _MARCOS_H_

#include <commons/temporal.h>
#include <pthread.h>            // Para mutex

#include "memoria.h"
#include "swap.h"

bool tengo_marcos_suficientes(uint32_t);

// SON DISTINTOS, OBTENER MARCO DEVUELVE UN MARCO PREVIAMENTE ASIGNADO
t_marco* obtener_marco_libre();
t_marco* obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina);

uint32_t cant_marcos_necesarios(uint32_t);

void reasignar_marco(t_marco* marco, uint32_t id_carpincho, uint32_t nro_pagina);
void actualizar_info_algoritmo(t_marco *marco_auxiliar, bool modificado);

void soltar_marco(t_marco *marco_auxiliar);
void reservar_marco(t_marco *marco_auxiliar);

t_marco* asignar_marco_libre(uint32_t nro_marco, uint32_t);
t_entrada_tp* crear_nueva_pagina(uint32_t, t_carpincho*);		// REVISAR

t_marco* obtener_marco_libre_mp();
t_marco* obtener_marco_mp(t_carpincho *mi_carpincho, uint32_t nro_pagina);

void suspend(uint32_t id);
void unsuspend(uint32_t id);

#endif /* _MARCOS_H_ */
