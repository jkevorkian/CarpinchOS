#include "utilidades.h"

int obtener_rafaga_real(char *tiempo_i, char *tiempo_f) { //El tiempo tiene el formato "12:51:59:331" Hora:Minuto:Segundo:MiliSegundo
	char** tiempo_inicio = string_split(tiempo_i, ":");
	char** tiempo_fin = string_split(tiempo_f, ":");
	int tiempo_rafaga[4];

	for(int i=0; i<4; i++)
		tiempo_rafaga[i] = atoi(tiempo_fin[i]) - atoi(tiempo_inicio[i]);

	liberar_split(tiempo_inicio);
	liberar_split(tiempo_fin);

	return (((tiempo_rafaga[0]*60 + tiempo_rafaga[1])*60 + tiempo_rafaga[2]))*1000 + tiempo_rafaga[3];
}

double obtener_estimacion_proxima_rafaga(int rafaga_real, int estimacion) {
	return alfa*rafaga_real + (1-alfa)*estimacion;
}

void liberar_split(char** split) {
	int i = 0;

	while(split[i] != NULL) {
		free(split[i]);
		i++;
	}

	free(split);
}

int buscar(t_list *lista, char* nombre) {
	int index = list_size(lista) - 1;

	while(index >= 0) {
		semaforo *sem = (semaforo*)list_get(lista, index);

		if(!strcmp(nombre, sem->nombre))
			return index;

		index--;
	}

	return index;
}

semaforo* buscar_sem_por_id(t_list *lista, int id) {
	int index = list_size(lista) - 1;

	while(index >= 0) {
		semaforo *sem = (semaforo*)list_get(lista, index);

		if(id == sem->id){
			return sem;
		}

		index--;
	}

	return NULL;  //retorna NULL si falla al encontrar un semaforo con el id dado
}

int iniciar_semaforo(char* nombre, int valor) {
	if(buscar(lista_semaforos, nombre) == -1) {
		semaforo *sem = malloc(sizeof(semaforo*));
		sem->cola_espera = queue_create();
		sem->instancias_disponibles = valor;
		sem->nombre = string_duplicate(nombre);
		sem->id = id_proximo_semaforo;
		id_proximo_semaforo++;
		pthread_mutex_init(&sem->mutex_espera, NULL);

		pthread_mutex_lock(&mutex_lista_semaforos);
		list_add(lista_semaforos, sem);
		pthread_mutex_unlock(&mutex_lista_semaforos);
		return sem->id;
	}
}

float calcular_HRRN(carpincho* carp, char* tiempo_actual) {
	int tiempo_espera = obtener_rafaga_real(carp->tiempo_llegada, tiempo_actual);
	return 1 + (tiempo_espera / carp->estimacion_proxima_rafaga); // (s+w)/s = 1 + w/s
}

int encontrar_carpincho(t_list *lista, carpincho *carp_quitar) {
	bool carpincho_encontrado = false;
	int index = list_size(lista) - 1;
	carpincho *carp;

	while(index >= 0 && !carpincho_encontrado) {
		carp = (carpincho*)list_get(lista, index); //verificar si tengo que meterlo en el mutex

		if(carp->id == carp_quitar->id)
			carpincho_encontrado = true;
		else
			index--;
	}

	return index;
}

void desbloquear(carpincho* carp) {
	if(carp->esta_suspendido)
		agregar_suspendidosReady(quitar_suspendidosBlocked(carp));
	else
		agregar_ready(quitar_blocked(carp));
}


