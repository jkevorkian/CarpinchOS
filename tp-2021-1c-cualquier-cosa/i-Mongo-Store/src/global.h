#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>//open
#include <pthread.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

#include <utils/utils-server.h>
#include <utils/utils-sockets.h>
#include <utils/utils-mensajes.h>

#define IP_MONGO "127.0.0.1"
#define ERROR_CONEXION -1
#define MAX(x,y) (((x)>(y)) ? (x) : (y))

typedef struct {
	int posicion_x;
	int posicion_y;
	int id_trip;
	int id_patota;
	int socket_discord;
	char* dir_bitacora;
}tripulante;

t_config* config;
t_log* logger;
t_list* lista_tripulantes;

pthread_t hilo_actualizador_block;//, hilo_detector_sabotajes;

pthread_mutex_t actualizar_blocks, actualizar_bitmap;

t_bitarray *bitmap;
uint32_t block_size, blocks_amount;
void *blocks, *blocks_copy;
char* punto_montaje;

bool salir_proceso;

#endif /* GLOBAL_H_ */
