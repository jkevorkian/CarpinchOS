#ifndef CONSOLA_H_
#define CONSOLA_H_


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <commons/string.h>


typedef enum{
	INICIAR_PATOTA,
	LISTAR_TRIPULANTES,
	EXPULSAR_TRIPULANTE,
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	OBTENER_BITACORA,
	EXIT_DISCORDIADOR,
	ERROR
}command_code;

command_code mapStringToEnum(char*);

char* leer_consola();
char* seleccionar_funcion(char*);

#endif /* CONSOLA_H_ */
