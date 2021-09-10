#include "validaciones.h"

/////////////////////VALIDACIONES//////////////////////

void puede_continuar(tripulante* trip) {

	if(analizar_quantum) {
		trip->contador_ciclos++;

		if(trip->contador_ciclos == quantum) {
			log_info(logger,"Tripulante %d se quedo sin quantum", trip->id_trip);

			quitar_running(trip);
			agregar_ready(trip);
			trip->contador_ciclos = 0;

			sem_wait(&trip->sem_running);
		}
	}

	if(!continuar_planificacion) {
		log_info(logger,"Tripulante %d pausado", trip->id_trip);
		sem_wait(&trip->sem_running);
		log_info(logger,"Tripulante %d reactivado", trip->id_trip);
	}

	if(trip->estado == EMERGENCY) {
		sem_post(&trip->sem_blocked);
		sem_post(&multiprocesamiento);
		sem_wait(&trip->sem_running);
	}
}


/////////////////////UTILIDADES//////////////////////
char* estado_enumToString(int estadoEnum) {
	char* listaDeStrings[] = {"NEW", "BLOCKED", "READY", "RUNNING", "EXIT"};

	return listaDeStrings[estadoEnum];
}

tareas stringToEnum(char *string){
	char* listaDeStrings[]={"GENERAR_OXIGENO", "CONSUMIR_OXIGENO", "GENERAR_COMIDA", "CONSUMIR_COMIDA", "GENERAR_BASURA", "DESCARTAR_BASURA"};

	for(int i=0;i<6;i++){
		if(!strcasecmp(string,listaDeStrings[i])) {
			return i;
		}
	}
	return ESPERAR;
}

void liberar_split(char** input) {
	int i = 0;

	while(input[i] != NULL) {
		free(input[i]);
		i++;
	}

	free(input);
}

int distancia_a(tripulante* trip, int pos_x, int pos_y) {
	//return (int) sqrt(pow(pos_x-trip->posicion[0], 2) + pow(pos_y-trip->posicion[1], 2));
	int distancia, resta_x = pos_x-trip->posicion[0], resta_y = pos_y-trip->posicion[1];

	if(resta_x >= 0)
		distancia = resta_x;
	else
		distancia = -resta_x;

	if(resta_y >= 0)
		distancia += resta_y;
	else
		distancia += -resta_y;

	return distancia;

}
