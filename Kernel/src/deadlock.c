#include "deadlock.h"

void detectar_deadlock(t_deadlock* deadlock){

    float segundos_entre_detecciones = (deadlock->milisegundos_entre_detecciones)/100;
    while(1){
        while (algoritmo_deteccion()){
            matar_proximo_carpincho(carpinchos_en_deadlock);
        }

        sleep(segundos_entre_detecciones);
    }
}

bool tiene_asignado(carpincho* carp, int id_semaforo){

    int index = list_size(carp->semaforos_asignados) - 1;

    while(index >= 0){
        semaforo* semAux = (semaforo*)list_get(carp->semaforos_asignados, index);
        if(semAux->id == id_semaforo){
            return true;
        }

        index--;
    }
    return false;
}

bool es_bloqueado_por_algun_semaforo(carpincho* carp, t_list* lista_de_semaforos){
    int index = list_size(lista_de_semaforos);

    while(index >= 0){
        semaforo* semAux = (semaforo*)list_get(lista_de_semaforos, index);
        if(carp->id_semaforo_bloqueante == semAux->id){
            return true;
        }
        index--;
    }
    return false;
}


bool cumple_condiciones_deadlock(carpincho* carp){
    int id_bloqueante = carp->id_semaforo_bloqueante;

    if(id_bloqueante != -1){

        semaforo* aux = buscar_sem_por_id(lista_semaforos ,id_bloqueante);

        if (aux != NULL){
            return true;
        }else {
            log_error(logger, "Error en la deteccion del deadlock: id del semaforo bloqueante de un carpincho no encontrado");
            return false;
        }


    }else {
        return false;  //si sale por aca entonces el carpincho no se bloqueo por un semaforo.
    }
}

int matar_proximo_carpincho(t_list* carpinchos_deadlock){
    carpincho* carpincho_a_matar = (carpincho*)list_get(carpinchos_deadlock, 1);

    close(carpincho_a_matar->socket_mateLib);
	close(carpincho_a_matar->socket_memoria);

    //TODO: liberar los semaforos y otros recursos que tenga asignados el carpincho
}

int iniciar_deteccion_deadlock(void* semaforosMaybe, int tiempo_deadlock){

    t_deadlock* deadlock;
    deadlock->milisegundos_entre_detecciones = tiempo_deadlock;
    deadlock->semaforosMaybe = semaforosMaybe;
    
    int retorno = pthread_create(&detector, NULL, detectar_deadlock, deadlock);
    if(retorno != 1){
    	/*
        if(retorno == EAGAIN){
            log_error(logger, "Error al crear hilo de deteccion de deadlock: Insufficient resources to create another thread");
            return retorno;
        }
        if(retorno == EINVAL){
            log_error(logger, "Error al crear hilo de deteccion de deadlock: Invalid settings in attr");
            return retorno;
        }
        if(retorno == EPERM){
            log_error(logger, "Error al crear hilo de deteccion de deadlock: No permission to set the scheduling policy and parameters specified in attr.");
            return retorno;
        } */
    	log_error(logger, "error al crear hilo de deteccion de deadlock");
    	return retorno;
    }else return 0;
}

int finalizar_deteccion_deadlock(){ //TODO: implementar esta funcion donde corresponda
    return pthread_cancel(detector);
}


bool esta_en_deadlock (carpincho* carp){

    int index = list_size(lista_a_evaluar) - 1;

	while(index >= 0) {
		carpincho *carpincho_lista = (carpincho*)list_get(lista_a_evaluar, index);

		if(tiene_asignado(carpincho_lista, carp->id_semaforo_bloqueante) && es_bloqueado_por_algun_semaforo(carpincho_lista, carp->semaforos_asignados)){
			return true;
        }

		index--;
	}

    return false;

}

int algoritmo_deteccion(){
    int deadlock_detectado = 0;
    t_list* lista_auxiliar = lista_blocked;
        
    lista_a_evaluar = filter(lista_auxiliar,cumple_condiciones_deadlock);

    carpinchos_en_deadlock = map(lista_a_evaluar, esta_en_deadlock);

    if(list_size(carpinchos_en_deadlock) > 0){
        deadlock_detectado = 1;
    }else deadlock_detectado = 0;

    return deadlock_detectado;
}





