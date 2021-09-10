#ifndef _TRIPULANTE_H_
#define _TRIPULANTE_H_

#include <commons/collections/list.h>
#include <stdint.h>
#include <commons/log.h>
#include <semaphore.h>

#include <utils/utils-server.h>

#include "memoria_ram.h"
#include "segmentos.h"
#include "paginas.h"
#include "hilos.h"

#define TAMANIO_TRIPULANTE (5 * sizeof(uint32_t) + sizeof(char))

typedef enum {
    TRIP_IP,
    ESTADO,
    POS_X,
    POS_Y,
    INS_POINTER,
    PCB_POINTER
} para_trip;

typedef struct {
    uint32_t PID;
    uint32_t TID;
    uint32_t inicio;
    int socket;
    pthread_t* hilo;        // Creo que no hace falta
} trip_data;

int iniciar_tripulante(uint32_t id_trip, uint32_t id_patota, uint32_t pos_x, uint32_t pos_y);
void eliminar_tripulante(uint32_t id_patota, uint32_t id_tripulante);
void eliminar_tripulante_paginacion(uint32_t id_patota, uint32_t id_tripulante);

uint32_t obtener_valor_tripulante(uint32_t id_patota, uint32_t id_trip, para_trip nro_parametro);
void actualizar_valor_tripulante(uint32_t id_patota, uint32_t id_trip, para_trip nro_parametro, uint32_t nuevo_valor);

char obtener_estado(uint32_t id_patota, uint32_t id_tripulante);
void actualizar_estado(uint32_t id_patota, uint32_t id_tripulante, char nuevo_valor);

trip_data* tripulante_de_lista(uint32_t id_patota, uint32_t id_trip);
int posicion_trip(uint32_t id_patota, uint32_t id_trip);

void liberar_tripulante(trip_data* trip_to_kill);
// uint32_t nro_segmento_tripulante(uint32_t);

uint32_t desplazamiento_parametro_trip(para_trip parametro);

#endif /* _TRIPULANTE_H_ */