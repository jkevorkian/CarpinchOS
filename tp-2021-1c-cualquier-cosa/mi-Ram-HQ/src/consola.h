#ifndef _CONSOLA_H_
#define _CONSOLA_H_

#include "memoria_ram.h"
#include "tripulante.h"

#include <utils/nivel-gui.h>
#include <utils/tad_nivel.h>

#include <stdlib.h>
#include <stdint.h>
#include <curses.h>
#include <commons/collections/list.h>
#include <commons/log.h>

pthread_t* iniciar_mapa(bool* continuar_consola);
void* dibujar_mapa(void* continuar_consola);
char obtener_id_tripulante(NIVEL* nivel, uint32_t id_patota, uint32_t id_trip);

#endif /* _CONSOLA_H_ */