#ifndef _INICIO_PATOTA_H_
#define _INICIO_PATOTA_H_

#include <commons/collections/list.h>
#include <stdint.h>
#include <commons/log.h>

#include "memoria_ram.h"
#include "segmentos.h"
#include "paginas.h"

#define TAMANIO_PATOTA (2 * sizeof(uint32_t))

typedef struct {
    uint32_t PID;
    uint32_t cant_tareas;
    uint32_t* inicio_tareas;
    uint32_t* tamanio_tareas;
} tareas_data;

char* obtener_tarea(uint32_t id_patota, uint32_t nro_tarea);

bool iniciar_patota(uint32_t id_patota, t_list* parametros);

void segmentar_pcb_p(uint32_t id_patota, uint32_t cant_tareas, char**);

bool patota_sin_tripulantes(uint32_t id_patota);

#endif /* _INICIO_PATOTA_H_ */