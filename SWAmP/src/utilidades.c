#include "utilidades.h"

void liberar_split(char** split) {
	int i = 0;

	while(split[i] != NULL) {
		free(split[i]);
		i++;
	}

	free(split);
}

char* string_desde_mensaje(int mensaje) {
	char* listaDeStrings[] = {
			// Validaciones
			"ER_SOC", "ER_RCV", "TODOOK", "NO_MEMORY", "SEG_FAULT",
			// Inicializacion
			"MATE_INIT", "MATE_CLOSE",
			// Memoria
			"MEM_ALLOC", "MEM_FREE", "MEM_READ", "MEM_WRITE",
			// SWAMP
			"GET_PAGE", "SET_PAGE", "SUSPEND", "UNSUSPEND", "NEW_C", "EXIT_C",
			// Semaforos
			"SEM_INIT", "SEM_WAIT", "SEM_POST", "SEM_DESTROY",
			// Otros
			"CALL_IO", "DATA", "SEND_PORT"};

	return listaDeStrings[mensaje];
}

