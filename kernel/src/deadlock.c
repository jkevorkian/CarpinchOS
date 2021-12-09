#include "deadlock.h"

int iniciar_deteccion_deadlock(int tiempo_deadlock) {
	pthread_t *detector = malloc(sizeof(pthread_t));
	pthread_create(detector, NULL, detectar_deadlock, NULL);

	lista_a_evaluar = list_create();
	carpinchos_en_deadlock = list_create();

	return 0;
}

void *detectar_deadlock(void* d) {
	if (LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tDetector de deadLock iniciado exitosamente");

	while (1) {
		sleep(tiempo_deadlock / 1000);
		log_warning(logger, "Corriendo algoritmo de deteccion de DeadLock");
		algoritmo_deteccion();

		while (list_size(carpinchos_en_deadlock)) {
			matar_proximo_carpincho(carpinchos_en_deadlock);
			algoritmo_deteccion();
		}
	}
}

void algoritmo_deteccion() {
	liberar_lista(lista_a_evaluar);
	liberar_lista(carpinchos_en_deadlock);

	lista_a_evaluar = carpinchos_que_cumplen_condicion();
	carpinchos_en_deadlock = carps_en_deadlock();
}

t_list *carpinchos_que_cumplen_condicion() {
	int index = list_size(lista_blocked) - 1;

	t_list *lista_auxiliar = list_create();

	while (index >= 0) {
		carpincho *carp = (carpincho *) list_get(lista_blocked, index);

		if (cumple_condiciones_deadlock(carp)) {
			if(LOGUEAR_MENSAJES_DEADLOCK)
				log_warning(logger, "El carpincho %d cumple condicion de deadlock", carp->id);

			list_add(lista_auxiliar, carp);
		} else if(LOGUEAR_MENSAJES_DEADLOCK)
			log_info(logger, "El carpincho %d no cumple condicion de deadlock", carp->id);

		index--;
	}

	return lista_auxiliar;
}

t_list *carps_en_deadlock() {
		int index = list_size(lista_a_evaluar) - 1;

		t_list *lista_auxiliar = list_create();
		while (index >= 0) {
			carpincho *carp = (carpincho *) list_get(lista_a_evaluar, index);


			if (esta_en_deadlock(carp, lista_auxiliar)) {
				if(LOGUEAR_MENSAJES_DEADLOCK)
					log_info(logger, "El carpincho %d esta en deadlock", carp->id);

				//list_add(lista_auxiliar, carp);   ahora se agregan los carps en deadlock a la lista auxiliar en esta_en_deadlock
			} else if(LOGUEAR_MENSAJES_DEADLOCK)
				log_info(logger, "El carpincho %d no esta deadlock", carp->id);

			index--;
		}

		return lista_auxiliar;
}

bool cumple_condiciones_deadlock(carpincho *carp) {
	int id_bloqueante = carp->id_semaforo_bloqueante;

	if (id_bloqueante != -1) {

		semaforo *aux = buscar_sem_por_id(lista_semaforos, id_bloqueante);

		if (aux == NULL)
			log_error(logger, "Error en la deteccion del deadlock: id del semaforo bloqueante de un carpincho no encontrado");

		return aux != NULL;
	} else
		return false; //si sale por aca entonces el carpincho no se bloqueo por un semaforo.
}

bool esta_en_deadlock(carpincho *carp, t_list* cadena_de_deadlock) {
	t_list* lista_auxiliar = list_create();
	carpincho* carpincho_n;
	if(carpincho_de_lista_con_sem_bloq_asignado(lista_a_evaluar, carp) != NULL){
		carpincho_n = carpincho_de_lista_con_sem_bloq_asignado(lista_a_evaluar, carp);
	}else return false;

	while(!tiene_asignado(carp, carpincho_n->id_semaforo_bloqueante)){
		if(carpincho_de_lista_con_sem_bloq_asignado(lista_a_evaluar, carpincho_n) == NULL){
			list_clean(lista_auxiliar);
			return false;
		}else {
			list_add(lista_auxiliar, carpincho_n);
			carpincho* carpincho_n = carpincho_de_lista_con_sem_bloq_asignado(lista_a_evaluar, carpincho_n);
		}
	}

	list_add_all(cadena_de_deadlock, lista_auxiliar);
	liberar_lista(lista_auxiliar);
	return true;
}

bool tiene_asignado(carpincho *carp, int id_semaforo) {
	int index = list_size(carp->semaforos_asignados) - 1;
	while (index >= 0) {
		semaforo_asignado *semAux = (semaforo_asignado*)list_get(carp->semaforos_asignados, index);
		if (semAux->sem->id == id_semaforo)
			return true;

		index--;
	}
	return false;
}

carpincho* carpincho_de_lista_con_sem_bloq_asignado(t_list* lista_carpinchos, carpincho* carp){
	int index = list_size(lista_carpinchos) - 1;

	while (index >= 0) {
		carpincho *carpincho_lista = (carpincho *) list_get(lista_carpinchos, index);

		if(carpincho_lista->id != carp->id){
			if (tiene_asignado(carpincho_lista, carp->id_semaforo_bloqueante)){
				return carpincho_lista;
			}
		}
		index--;
	}
	return NULL;
}

/*
bool es_bloqueado_por_algun_semaforo(carpincho *carp, t_list *lista_de_semaforos) {
	int index = list_size(lista_de_semaforos) - 1;

	while (index >= 0) {
		semaforo_asignado *semAux = (semaforo_asignado *) list_get(lista_de_semaforos, index);

		if (carp->id_semaforo_bloqueante == semAux->sem->id)
			return true;

		index--;
	}
	return false;
}
*/

bool *ordenador_carpinchos(carpincho* carp1, carpincho* carp2) {
	return (bool*)(carp1->id < carp1->id);
}

int matar_proximo_carpincho(t_list *carpinchos_deadlock) {  //TODO: que pasa si un semaforo que debe morir por deadlock estaba en la lista de espera de un semaforo?
	list_sort(carpinchos_deadlock, (void*)ordenador_carpinchos); //ordena la lista de carpinchos en deadlock de menor a mayor ID
	carpincho *carp = list_get(carpinchos_deadlock, 1);
	if(LOGUEAR_MENSAJES_DEADLOCK)
		log_info(logger, "matando a carpincho %d", carp->id);

	carp->debe_morir = true;
	list_remove(carpinchos_deadlock, 1);

	int index = list_size(carp->semaforos_asignados) - 1;

	while (index >= 0) {
		semaforo_asignado *semAux = (semaforo_asignado *) list_get(carp->semaforos_asignados,index);
		hacer_posts_semaforo(semAux, carp);
		index--;
	}

	//TODO: liberar los otros recursos que tenga asignados el carpincho y revisar el todo de hacer_post_semaforo
	return 0;
}

int finalizar_deteccion_deadlock() { //se llama a esta funcion en en inicializador
	return pthread_cancel(detector);
}


