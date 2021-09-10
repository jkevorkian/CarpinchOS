#ifndef UTILIDADES_H_
#define UTILIDADES_H_

#include "global.h"

//char* crear_MD5(char, int);
char* crear_MD5(char* bloques_str, char caracter_llenado, int cantidad_bloques);
tripulante* obtener_tripulante(int id_trip, int id_patota);
int roundUp(int, int);
char* juntar_posiciones(int, int);
void generar_directorio(char*);
char* obtener_directorio(char*);
void imprimir_bitmap(t_bitarray*);
void liberar_split(char** split);

void calcularBloquesUsadosRecursos(t_bitarray*bitmap_copy, char* DIR_metadata);
void calcularBloquesUsadosBitacoras(t_bitarray*bitmap_copy, char* DIR_metadata);

#endif /* UTILIDADES_H_ */
