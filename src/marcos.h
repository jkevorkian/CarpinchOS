#ifndef _MARCOS_H_
#define _MARCOS_H_

#include <commons/temporal.h>
#include <pthread.h>            // Para mutex

#include "memoria.h"
#include "swap.h"
#include "tlb.h"

bool tengo_marcos_suficientes(uint32_t);

// Permite obtener un nuevo marco libre de mp, para usarlo es necesario bloquear el mutex de asignacion de marcos
t_marco* obtener_marco_libre();

// Permiten obtener un marco ya asignado de forma general o exclusivamente de mp respectivamente.
t_marco* obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina);
t_marco* obtener_marco_mp(uint32_t id_carpincho, uint32_t nro_pagina);

uint32_t cant_marcos_necesarios(uint32_t);

void asignar_marco_libre(t_marco *, uint32_t, uint32_t);
void reasignar_marco(t_marco* marco, uint32_t id_carpincho, uint32_t nro_pagina);

void actualizar_info_algoritmo(t_marco *marco_auxiliar, bool modificado);

void soltar_marco(t_marco *);
void reservar_marco(t_marco *);
void liberar_marco(t_marco *marco);

//////////////////////////////////////
// Revisar
// bool agregar_pagina(uint32_t id_carpincho);
t_entrada_tp* agregar_pagina(uint32_t id_carpincho);
///////////////////////////////////////

t_entrada_tp* crear_nueva_pagina(uint32_t, t_carpincho*);

t_entrada_tp *pagina_de_carpincho(uint32_t id, uint32_t nro_pagina);

void suspend(uint32_t id);
void unsuspend(uint32_t id);

#endif /* _MARCOS_H_ */
