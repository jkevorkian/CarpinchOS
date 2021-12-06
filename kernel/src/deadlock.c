#include "deadlock.h"

int iniciar_deteccion_deadlock(int tiempo_deadlock) {
	t_deadlock *deadlock = malloc(sizeof(deadlock));
	deadlock->milisegundos_entre_detecciones = tiempo_deadlock;

	//pthread_create(&detector, NULL, detectar_deadlock, deadlock);
	return 0;
}
void *detectar_deadlock(void* d) {
	if (LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tDetector de deadLock iniciado exitosamente");

	float segundos_entre_detecciones = (tiempo_deadlock) / 100;
	while (1) {
		while (algoritmo_deteccion())
			matar_proximo_carpincho(carpinchos_en_deadlock);

		sleep(segundos_entre_detecciones);
	}
}

bool tiene_asignado(carpincho *carp, int id_semaforo) {

	int index = list_size(carp->semaforos_asignados) - 1;

	while (index >= 0) {
		semaforo *semAux = (semaforo *) list_get(carp->semaforos_asignados,
				index);
		if (semAux->id == id_semaforo) {
			return true;
		}

		index--;
	}
	return false;
}

bool es_bloqueado_por_algun_semaforo(carpincho *carp,
		t_list *lista_de_semaforos) {
	int index = list_size(lista_de_semaforos);

	while (index >= 0) {
		semaforo *semAux = (semaforo *) list_get(lista_de_semaforos, index);
		if (carp->id_semaforo_bloqueante == semAux->id) {
			return true;
		}
		index--;
	}
	return false;
}

bool cumple_condiciones_deadlock(void *carp) {
	carpincho* carp_casteado = (carpincho*) carp;

	int id_bloqueante = carp_casteado->id_semaforo_bloqueante;

	if (id_bloqueante != -1) {

		semaforo *aux = buscar_sem_por_id(lista_semaforos, id_bloqueante);

		if (aux != NULL) {
			return true;
		} else {
			log_error(logger,
					"Error en la deteccion del deadlock: id del semaforo bloqueante de un carpincho no encontrado");
			return false;
		}
	} else {
		return false; //si sale por aca entonces el carpincho no se bloqueo por un semaforo.
	}
}

bool *ordenador_carpinchos(carpincho* carp1, carpincho* carp2) {
	return (bool*)(carp1->id < carp1->id);
}

int matar_proximo_carpincho(t_list *carpinchos_deadlock) {
	list_sort(carpinchos_deadlock, (void*)ordenador_carpinchos); //ordena la lista de carpinchos en deadlock de menor a mayor ID
	carpincho *carp = list_get(carpinchos_deadlock, 1);
	carp->debe_morir = true;
	list_remove(carpinchos_deadlock, 1);

	int index = list_size(carp->semaforos_asignados);

	while (index >= 0) {
		semaforo *semAux = (semaforo *) list_get(carp->semaforos_asignados,index);
		hacer_post_semaforo(semAux);
		index--;
	}

	//TODO: liberar los otros recursos que tenga asignados el carpincho y revisar el todo de hacer_post_semaforo
	return 0;
}

int finalizar_deteccion_deadlock() { //se llama a esta funcion en en inicializador
	return pthread_cancel(detector);
}

bool esta_en_deadlock(void *carp) {
	carpincho* carp_casteado = (carpincho*) carp;

	int index = list_size(lista_a_evaluar) - 1;

	while (index >= 0) {
		carpincho *carpincho_lista = (carpincho *) list_get(lista_a_evaluar,
				index);

		if (tiene_asignado(carpincho_lista, carp_casteado->id_semaforo_bloqueante)
				&& es_bloqueado_por_algun_semaforo(carpincho_lista,
						carp_casteado->semaforos_asignados)) {
			return true;
		}

		index--;
	}

	return false;
}

int algoritmo_deteccion() {
	int deadlock_detectado = 0;
	t_list *lista_auxiliar = lista_blocked;

	lista_a_evaluar = list_filter(lista_auxiliar, cumple_condiciones_deadlock);

	carpinchos_en_deadlock = list_filter(lista_a_evaluar, esta_en_deadlock);

	if (list_size(carpinchos_en_deadlock) > 0) {
		deadlock_detectado = 1;
	} else
		deadlock_detectado = 0;

	return deadlock_detectado;
}
