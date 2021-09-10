#ifndef BLOCKS_H_
#define BLOCKS_H_

#include "global.h"
#include "mongo.h"

bool crear_superBloque();
bool crear_blocks();
void* uso_blocks();

void sumar_caracteres(char, int);
void quitar_caracteres(char, int);
char* proximo_bloque_libre();
int escribir_caracter_en_bloque(char,int,char* ,int );
int borrar_caracter_en_bloque(char ,int ,char* ,int );

#endif /* BLOCKS_H_ */
