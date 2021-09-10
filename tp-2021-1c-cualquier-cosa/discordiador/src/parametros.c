#include "parametros.h"

parametros_iniciar_patota* obtener_parametros(char** input) {//todo realizar validaciones para lectura de archivos y parametros validos
	log_info(logger,"Obteniendo parametros...");

	parametros_iniciar_patota* parametros = malloc(sizeof(parametros_iniciar_patota));
	bool valida = true;

	int cantidad_tripulantes = atoi(input[1]);

	parametros->cantidad_tripulantes = cantidad_tripulantes;
	parametros->posiciones_x = malloc(cantidad_tripulantes * sizeof(int));
	parametros->posiciones_y = malloc(cantidad_tripulantes * sizeof(int));

	for(int iterador = 0; iterador < cantidad_tripulantes; iterador++) { // completo las posiciones del struct

		if(valida && input[iterador+3] != NULL) { //iterador+3 nos estaria dando la ubicacion de inicio del tripulante
			char** auxiliar = string_split(input[iterador+3], "|"); //divide la posicion de "x|y" a auxiliar[0]=x y auxiliar[1]=y
			parametros->posiciones_x[iterador] = atoi(auxiliar[0]);
			parametros->posiciones_y[iterador] = atoi(auxiliar[1]);
			liberar_split(auxiliar);
		}
		else {
			parametros->posiciones_x[iterador] = 0;
			parametros->posiciones_y[iterador] = 0;
			valida = false;
		}
	}

	/*
	char* direccion;

	if(string_starts_with(input[2], "/"))
		direccion = input[2];
	else {
		direccion = string_new();
		string_append(&direccion, "/home/utnso/tp-2021-1c-cualquier-cosa/");
		string_append(&direccion, input[2]);
	}

	log_info(logger, "Direccion tareas: %s", direccion);
	FILE *archivo_tareas = fopen (direccion, "r");
	free(direccion);*/

	FILE *archivo_tareas = fopen (input[2], "r");

	if(archivo_tareas != NULL) {
		char buffer_tarea[40];
		parametros->cantidad_tareas = 0;
		parametros->tareas = NULL;

		while (fgets(buffer_tarea, 100, archivo_tareas)) {
			strtok(buffer_tarea, "\n"); //strtok le saca el \n al string de buffer_tarea que es agregado por fgets al leer del archivo

			parametros->tareas = realloc(parametros->tareas, (parametros->cantidad_tareas + 1) * sizeof(char*)); //le agrego a mi vector de tareas[str] un nuevo valor para colocar una nueva tarea
			parametros->tareas[parametros->cantidad_tareas] = malloc(sizeof(buffer_tarea));

			memcpy(parametros->tareas[parametros->cantidad_tareas], &buffer_tarea, sizeof(buffer_tarea));

			parametros->cantidad_tareas++;
		}

		fclose(archivo_tareas);
	} else
		log_error(logger, "Esta mal la ubicacion del archivo");

	return parametros;
}

void liberar_parametros(parametros_iniciar_patota* parametros) {
	free(parametros->posiciones_x);
	free(parametros->posiciones_y);

	while(parametros->cantidad_tareas > 0){
		free(parametros->tareas[parametros->cantidad_tareas-1]);
		parametros->cantidad_tareas--;
	}

	free(parametros->tareas);
	free(parametros);
}

void loggear_parametros(parametros_iniciar_patota* parametros) {
	log_info(logger,"Cantidad de tripulantes: %d", parametros->cantidad_tripulantes);

	for(int i = 0; i < parametros->cantidad_tripulantes; i++)
		log_info(logger,"	Tripulante: %d  |  Posicion x: %d  |  Posicion y: %d", i+1, parametros->posiciones_x[i], parametros->posiciones_y[i]);

	log_info(logger,"Cantidad de tareas: %d", parametros->cantidad_tareas);

	for(int i = 0; i < parametros->cantidad_tareas; i++)
		log_info(logger,"	Tarea %d: %s", i, parametros->tareas[i]);

}
