#include "utilidades.h"

float calcular_HRRN(carpincho* carp, char* tiempo_actual) {
	int tiempo_espera = obtener_rafaga_real(carp->tiempo_llegada, tiempo_actual);
	log_info(logger_colas, "Carp %d: Tiempo_espera = %d - Estimacion proxima rafaga = %d", carp->id, tiempo_espera, (int)carp->estimacion_proxima_rafaga);
	return 1 + (tiempo_espera / carp->estimacion_proxima_rafaga); // (s+w)/s = 1 + w/s
}

double obtener_estimacion_proxima_rafaga(int rafaga_real, int estimacion) {
	return alfa*rafaga_real + (1-alfa)*estimacion;
}

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

void liberar_split(char** split) {
	int i = 0;

	while(split[i] != NULL) {
		free(split[i]);
		i++;
	}

	free(split);
}

void iniciar_semaforo(char* nombre, int valor) {
	if(buscar_semaforo(nombre) == -1) {
		semaforo *sem = malloc(sizeof(semaforo));

		sem->nombre = string_duplicate(nombre);
		sem->instancias_iniciadas = valor;
		sem->cola_espera = queue_create();
		pthread_mutex_init(&sem->mutex_espera, NULL);

		pthread_mutex_lock(&mutex_lista_semaforos);
			sem->id = id_proximo_semaforo;
			id_proximo_semaforo++;
			list_add(lista_semaforos, sem);
			pthread_mutex_unlock(&mutex_lista_semaforos);
		log_info(logger, "Iniciado exitosamente el semaforo: %s - Valor inicial %d", sem->nombre, sem->instancias_iniciadas);
	} else
		log_info(logger, "El semaforo ya estaba inicializado");
}

int encontrar_carpincho(t_list *lista, carpincho *carp_buscar) {
	bool carpincho_encontrado = false;
	int index = list_size(lista) - 1;
	carpincho *carp;

	while(index >= 0 && !carpincho_encontrado) {
		carp = (carpincho*)list_get(lista, index);

		if(carp->id == carp_buscar->id)
			carpincho_encontrado = true;
		else
			index--;
	}

	return index;
}

int buscar_semaforo(char* nombre) {
	//pthread_mutex_lock(&mutex_lista_semaforos);

	int index = list_size(lista_semaforos) - 1;

	while(index >= 0) {
		semaforo *sem = (semaforo*)list_get(lista_semaforos, index);
		if(!strcmp(nombre, sem->nombre))
			return index;

		index--;
	}

	//pthread_mutex_unlock(&mutex_lista_semaforos);

	return index;
}

int buscar_io(char* nombre) {
	int index = list_size(lista_IO) - 1;

	while(index >= 0) {
		IO *io = (IO*)list_get(lista_IO, index);

		if(!strcmp(nombre, io->nombre))
			return index;

		index--;
	}

	return index;
}

void desbloquear(carpincho* carp) {
	if(LOGUEAR_MENSAJES_COLAS)
		log_info(logger, "   Desbloqueando al carpincho %d", carp->id);

	if(carp->esta_suspendido)
		agregar_suspendidosReady(quitar_suspendidosBlocked(carp));
	else
		agregar_ready(quitar_blocked(carp));
}

int buscar_sem_en_lista(t_list *lista, char *nombre) { //la lista debe estar conformada por nodos de semaforo_asignado, no del semaforo* pelado
	int index = list_size(lista) - 1;

	while (index >= 0) {
		sem_deadlock *sem_asignado = (sem_deadlock *)list_get(lista, index);

		if (!strcmp(nombre, sem_asignado->sem->nombre))
			return index;

		index--;
	}
	return -1;
}

semaforo *buscar_sem_por_id(t_list *lista, int id) {
	int index = list_size(lista) - 1;

	while (index >= 0) {
		semaforo *sem = (semaforo *)list_get(lista, index);

		if (id == sem->id)
			return sem;

		index--;
	}

	return NULL; //retorna NULL si falla al encontrar un semaforo con el id dado
}

void hacer_posts_semaforo(sem_deadlock *semaforo_asignado, carpincho* carp) {

	semaforo* sem = semaforo_asignado->sem;

	pthread_mutex_lock(&sem->mutex_espera);
		if(queue_is_empty(sem->cola_espera)) {
			log_error(logger, "ERROR EN EL DEADLOCK: nunca se deberia haber ingresado aca, sumando instancias iniciadas a un semaforo que bloqueaba un proceso en deadlock!");
			sem->instancias_iniciadas++;
		} else {
			carpincho *carp_desbloquear;

			while(semaforo_asignado->cantidad_asignada > 0) {
				carp_desbloquear = queue_pop(sem->cola_espera);

				if(carp_desbloquear->id == -1) {
					free(carp_desbloquear);

					if(queue_is_empty(sem->cola_espera))
						sem->instancias_iniciadas++;
				} else {
					desbloquear(carp_desbloquear);
					carp_desbloquear->responder = true;
					carp_desbloquear->id_semaforo_bloqueante = -1;
				}//

				semaforo_asignado->cantidad_asignada--;
			}
		}
	pthread_mutex_unlock(&sem->mutex_espera);

	/*pthread_mutex_lock(&sem->mutex_espera);
	if (queue_is_empty(sem->cola_espera)){
		log_error(logger, "ERROR EN EL DEADLOCK: nunca se deberia haber ingresado aca, sumando instancias iniciadas a un semaforo que bloqueaba un proceso en deadlock!");
		sem->instancias_iniciadas++;
	}
	else {
		while(semaforo_asignado->cantidad_asignada > 0){
			carpincho *carp_a_desbloquear = queue_pop(sem->cola_espera);
			desbloquear(carp_a_desbloquear);
			carp_a_desbloquear->responder = true;
			carp_a_desbloquear->id_semaforo_bloqueante = -1;
			semaforo_asignado->cantidad_asignada--;
		}

	}
	pthread_mutex_unlock(&sem->mutex_espera);*/
}

void liberar_lista(t_list* lista) {
	int index = list_size(lista) - 1;

	while (index >= 0) {
		list_remove(lista, index);
		index--;
	}

	list_destroy(lista);
}

bool no_tiene_asignado_este_semaforo(carpincho* carp,semaforo* sem){
	return buscar_sem_en_lista(carp->semaforos_asignados,sem->nombre)==-1;
}

void* informador() {
	int socket_informador = crear_conexion_servidor(ip_kernel, 10217, 1);
	while(1) {
		int socket = esperar_cliente(socket_informador);

		log_info(logger_colas, "--------------------------------------------------------");
		log_info(logger_colas, "LISTANDO INFORMACION SOBRE LA PLANIFICACION");
		log_info(logger_colas, "Grado Multiprogramacion Actual: %d", grado_multiprogramacion);
		log_info(logger_colas, "Grado Multiprocesamiento Actual: %d", grado_multiprocesamiento);
		log_info(logger_colas, "Cantidad de carpinchos en new: %d", queue_size(cola_new));
		log_info(logger_colas, "Cantidad de carpinchos en ready: %d", list_size(lista_ready));
		log_info(logger_colas, "Cantidad de carpinchos en bloqued: %d", list_size(lista_blocked));
		log_info(logger_colas, "Cantidad de carpinchos en suspendidosReady: %d", queue_size(cola_suspendidosReady));
		log_info(logger_colas, "Cantidad de carpinchos en suspendidosBloqued: %d", list_size(lista_suspendidosBlocked));
		log_info(logger_colas, "--------------------------------------------------------");

		close(socket);
	}
}

