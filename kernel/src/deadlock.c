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
			//matar_proximo_carpincho(carpinchos_en_deadlock);
			algoritmo_deteccion();
		}
	}
}

void algoritmo_deteccion() {
	liberar_lista(lista_a_evaluar);
	liberar_lista(carpinchos_en_deadlock);

	lista_a_evaluar = carpinchos_bloqueados_sem();
	carpinchos_en_deadlock = carps_en_deadlock();
}

t_list *carpinchos_bloqueados_sem() {
	int index = list_size(lista_blocked) - 1;

	t_list *lista_auxiliar = list_create();

	while (index >= 0) {
		carpincho *carp = (carpincho *) list_get(lista_blocked, index);

		if (carp->id_semaforo_bloqueante != -1) {
			if(LOGUEAR_MENSAJES_DEADLOCK)
				log_info(logger, "El carpincho %d podria tener deadLock (esta bloqueado por un semaforo)", carp->id);

			list_add(lista_auxiliar, carp);
		}

		index--;
	}

	return lista_auxiliar;
}

t_list *carps_en_deadlock() {
		int index = list_size(lista_a_evaluar) - 1;

		if (LOGUEAR_MENSAJES_DEADLOCK)
			log_info(logger, "Cantidad de carpinchos a evaluar %d", index+1);

		t_list *lista_auxiliar = list_create();

		while (index >= 0) {
			carpincho *carp = (carpincho *) list_get(lista_a_evaluar, index);

			if(esta_en_deadlock(carp, lista_auxiliar))
				index = 0;
			else if (LOGUEAR_MENSAJES_DEADLOCK)
				log_info(logger, "El carpincho %d no esta en deadlock", carp->id);

			index--;
		}

		index = list_size(lista_auxiliar) - 1;

		while (index >= 0) {
			carpincho *carp = (carpincho *) list_get(lista_a_evaluar, index);
			if (LOGUEAR_MENSAJES_DEADLOCK)
				log_warning(logger, "El carpincho %d esta en deadlock", carp->id);
			index--;
		}

		return lista_auxiliar;
}

bool esta_en_deadlock(carpincho *carp, t_list* cadena_de_deadlock) {
	t_list* lista_auxiliar = list_create();
	carpincho* carpincho_n = carpincho_de_lista_con_sem_bloq_asignado(lista_a_evaluar, carp);

	if(carpincho_n == NULL) return false;

	while(!tiene_asignado(carp, carpincho_n->id_semaforo_bloqueante)){
		list_add(lista_auxiliar, carpincho_n);
		carpincho_n = carpincho_de_lista_con_sem_bloq_asignado(lista_a_evaluar, carpincho_n);

		if(carpincho_n == NULL) {
			liberar_lista(lista_auxiliar);
			return false;
		}
	}

	list_add_all(cadena_de_deadlock, lista_auxiliar);
	liberar_lista(lista_auxiliar);
	return true;
}

bool tiene_asignado(carpincho *carp, int id_semaforo) {
	int index = list_size(carp->semaforos_asignados) - 1;
	while (index >= 0) {
		sem_deadlock *semAux = (sem_deadlock*)list_get(carp->semaforos_asignados, index);
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

		if(carpincho_lista->id != carp->id && tiene_asignado(carpincho_lista, carp->id_semaforo_bloqueante))
			return carpincho_lista;

		index--;
	}
	return NULL;
}

bool *ordenador_carpinchos(carpincho* carp1, carpincho* carp2) {
	return (bool*)(carp1->id < carp1->id);
}

int matar_proximo_carpincho(t_list *carpinchos_deadlock) {  //TODO: que pasa si un semaforo que debe morir por deadlock estaba en la lista de espera de un semaforo?
	//list_sort(carpinchos_deadlock, (void*)ordenador_carpinchos); //ordena la lista de carpinchos en deadlock de menor a mayor ID
	carpincho *carp = list_get(carpinchos_deadlock, 1);

	if(LOGUEAR_MENSAJES_DEADLOCK)
		log_info(logger, "matando a carpincho %d", carp->id);

	carp->debe_morir = true;
	list_remove(carpinchos_deadlock, 1);

	int index = list_size(carp->semaforos_asignados) - 1;

	while (index >= 0) {
		sem_deadlock *semAux = (sem_deadlock *) list_get(carp->semaforos_asignados,index);
		hacer_posts_semaforo(semAux, carp);
		index--;
	}

	//TODO: liberar los otros recursos que tenga asignados el carpincho y revisar el todo de hacer_post_semaforo
	return 0;
}


