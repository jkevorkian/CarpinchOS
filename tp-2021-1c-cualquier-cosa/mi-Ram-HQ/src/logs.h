#ifndef _LOGGEAR_PRUEBAS_H_
#define _LOGGEAR_PRUEBAS_H_

#include <commons/collections/list.h>
#include <stdint.h>
#include <commons/log.h>
#include <semaphore.h>
#include <commons/temporal.h>

#include "memoria_ram.h"
#include "tripulante.h"

void dump(void);
void dump_paginacion_(int archivo_dump);
void dump_segmentacion_(int archivo_dump);
void dump_paginacion(FILE* archivo_dump);
void dump_segmentacion(FILE* archivo_dump);

void loggear_data();
void loggear_tripulantes();
void loggear_patotas();

void loggear_marcos_fisicos(FILE*);
void loggear_marco_fisico(FILE*, uint32_t nro_marco_fisico);


void loggear_segmentos(FILE*);
void loggear_segmento(FILE*, uint32_t nro_seg_absoluto, uint32_t nro_seg_relativo);

#endif /* _LOGGEAR_PRUEBAS_H_ */

void loggear_patotas();
void loggear_tripulantes();
// void loggear_prueba_segmentos(uint32_t nro_segmento);
void loggear_segmento(FILE* archivo, uint32_t nro_segmento, uint32_t nro_segmento_relativo);
void loggear_segmentos(FILE* archivo);
void loggear_marcos_logicos(FILE* archivo);

void loggear_data();