#include "consola.h"

char* seleccionar_funcion(char* buffer) {
	int iterador = 0;
	while(iterador < strlen(buffer) && buffer[iterador] != ' ' && buffer[iterador] != '\n' && buffer[iterador] != '\0') {
		iterador++;
	}
	char* palabra = malloc((iterador * sizeof(char))+1);
	strncpy(palabra, buffer, iterador);
	palabra[iterador] = '\0';
	return palabra;
}

char* leer_consola() {
	char* buffer = readline(">");
	if(buffer)
		add_history(buffer);

	return buffer;
}

command_code mapStringToEnum(char *string){
	//char* listaDeStrings[]={"ini", "L", "E", "IP", "P", "O", "EXIT"};

	char* listaDeStrings[]={"INICIAR_PATOTA", "LISTAR_TRIPULANTES", "EXPULSAR_TRIPULANTE", "INICIAR_PLANIFICACION", "PAUSAR_PLANIFICACION", "OBTENER_BITACORA", "EXIT"};

	for(int i=0;i<7;i++){
		if(!strcasecmp(string,listaDeStrings[i])) {
			return i;
		}		
	}
	return ERROR;
}

//"INICIAR_PATOTA", "LISTAR_TRIPULANTES", "EXPULSAR_TRIPULANTE", "INICIAR_PLANIFICACION", "PAUSAR_PLANIFICACION", "OBTENER_BITACORA", "EXIT"
