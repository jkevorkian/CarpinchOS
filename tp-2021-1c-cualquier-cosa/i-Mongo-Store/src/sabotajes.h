#ifndef SABOTAJES_H_
#define SABOTAJES_H_

#include "global.h"
#include "mongo.h"

char** ubicaciones_sabotajes;
int socket_sabotajes, contador_sabotajes;

void inicializar_detector_sabotajes(int socket_discord);
void analizador_sabotajes(int senial);

void arreglar_BlockCount_superBloque();
void arreglar_Bitmap_superBloque();
void arreglar_blocks_recursos(char* DIR_metadata);
void arreglar_size_recursos(char* DIR_metadata);
void arreglar_blockcount_recursos(char* DIR_metadata);

#endif /* SABOTAJES_H_ */
