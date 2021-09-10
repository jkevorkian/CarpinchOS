#include "segmentos.h"

bool iniciar_memoria_segmentada(t_config* config) {
	memoria_ram.esquema_memoria = SEGMENTACION;
	memoria_ram.mapa_segmentos = list_create();
	t_segmento* segmento_memoria = malloc(sizeof(t_segmento));
	segmento_memoria->n_segmento = 0;
	segmento_memoria->duenio = 0;
	segmento_memoria->inicio = 0;
	segmento_memoria->tamanio = memoria_ram.tamanio_memoria;
    sem_init(&segmento_memoria->mutex, 0, 1);
	list_add(memoria_ram.mapa_segmentos, segmento_memoria);

	if(!strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "FF"))
		memoria_ram.criterio_seleccion = FIRST_FIT;
	if(!strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "BF"))
		memoria_ram.criterio_seleccion = BEST_FIT;

    sem_init(&mutex_lista_segmentos, 0, 1);
    sem_init(&mutex_compactacion, 0, 1);
	return true;
}

bool condicion_segmento_libre(void* segmento_memoria) {
    if(((t_segmento*)segmento_memoria)->duenio == 0)
        return true;
    else
        return false;
}

void* minimo_tamanio(void* elemento1, void* elemento2) {
    if ( ((t_segmento *)elemento1)->tamanio < ((t_segmento *)elemento2)->tamanio ) {
        return elemento1;
    }
    else
        return elemento2;
}

void* suma_tamanios(void* seed, void* elemento) {
    return (void *)(seed + ((t_segmento *)elemento)->tamanio);
}

t_segmento* crear_segmento(uint32_t nuevo_tamanio) {
	t_segmento *segmento_nuevo = malloc(sizeof(t_segmento));
    
    bool segmentos_aptos(void* segmento_memoria) {
        if(((t_segmento*)segmento_memoria)->duenio == 0 && ((t_segmento*)segmento_memoria)->tamanio >= nuevo_tamanio)
            return true;
        else
            return false;
    }

    t_list* segmentos_validos = list_filter(memoria_ram.mapa_segmentos, (*segmentos_aptos));

    if(list_size(segmentos_validos) == 0) {
        realizar_compactacion();
        segmentos_validos = list_filter(memoria_ram.mapa_segmentos, (*segmentos_aptos));
    }

    if(memoria_ram.criterio_seleccion == FIRST_FIT) {
        memcpy(segmento_nuevo, (t_segmento *)list_get(segmentos_validos, 0), sizeof(t_segmento));
    }
    if(memoria_ram.criterio_seleccion == BEST_FIT) {
        memcpy(segmento_nuevo, (t_segmento *)list_get_minimum(segmentos_validos, (void *)(*minimo_tamanio)), sizeof(t_segmento));
        // No testeado
    }
    list_add_in_index(memoria_ram.mapa_segmentos, segmento_nuevo->n_segmento, segmento_nuevo);
    
    t_segmento* segmento_siguiente = (t_segmento *)list_get(memoria_ram.mapa_segmentos, segmento_nuevo->n_segmento + 1);
    segmento_siguiente->tamanio -= nuevo_tamanio;
    
    if(segmento_siguiente->tamanio == 0) {
        list_remove(memoria_ram.mapa_segmentos, segmento_nuevo->n_segmento + 1);
        // free(segmento_siguiente);
    }
    else {
        segmento_siguiente->inicio += nuevo_tamanio;
        
        t_link_element* iterador = memoria_ram.mapa_segmentos->head;
        // Hago que el iterador apunte a la posicion del segmento_siguiente
        for(int i = 0; i < segmento_nuevo->n_segmento + 1; i++) {
            iterador = iterador->next;
        }
        // A cada segmento que le sigue incremento el nro_segmento
        while (iterador->next != NULL) {
            ((t_segmento *)iterador->data)->n_segmento++;
            iterador = iterador->next;
        }
        ((t_segmento *)iterador->data)->n_segmento++;
    }
    segmento_nuevo->tamanio = nuevo_tamanio;
    sem_init(&segmento_nuevo->mutex, 0, 1);
    list_destroy(segmentos_validos);
    return segmento_nuevo;
}

void eliminar_segmento(uint32_t nro_segmento) {
    t_segmento* segmento = list_get(memoria_ram.mapa_segmentos, nro_segmento);
    t_segmento* segmento_anterior;
    t_segmento* segmento_siguiente;
    bool compactar_segmento_anterior = false;
    bool compactar_segmento_siguiente = false;

    segmento->duenio = 0;

    if(nro_segmento > 0) {
        segmento_anterior = list_get(memoria_ram.mapa_segmentos, nro_segmento - 1);
        if(segmento_anterior->duenio == 0) {
            compactar_segmento_anterior = true;
        }
    }
    if (nro_segmento < list_size(memoria_ram.mapa_segmentos) - 1) {
        segmento_siguiente = list_get(memoria_ram.mapa_segmentos, nro_segmento + 1);
        if(segmento_siguiente->duenio == 0) {
            compactar_segmento_siguiente = true;
        }
    }
    t_link_element* iterador = memoria_ram.mapa_segmentos->head;
    if (compactar_segmento_siguiente) {
        segmento->tamanio += segmento_siguiente->tamanio;
        free(list_remove(memoria_ram.mapa_segmentos, segmento_siguiente->n_segmento));  // REVISAR FREE
        // list_remove(memoria_ram.mapa_segmentos, segmento_siguiente->n_segmento);
        for(int i = 0; i < segmento->n_segmento; i++) {
            iterador = iterador->next;
        }
        iterador = iterador->next;
        if(iterador) {
        while (iterador->next != NULL) {
            ((t_segmento *)iterador->data)->n_segmento--;
            iterador = iterador->next;
        }
        ((t_segmento *)iterador->data)->n_segmento--;
        }
    }
    
    iterador = memoria_ram.mapa_segmentos->head;
    if (compactar_segmento_anterior) {
        segmento_anterior->tamanio += segmento->tamanio;
        free(list_remove(memoria_ram.mapa_segmentos, segmento->n_segmento));            // REVISAR FREE
        // list_remove(mapa_segmentos, segmento->n_segmento);
        for(int i = 0; i < segmento_anterior->n_segmento; i++) {
            iterador = iterador->next;
        }
        iterador = iterador->next;
        while (iterador->next != NULL) {
            ((t_segmento *)iterador->data)->n_segmento--;
            iterador = iterador->next;
        }
        ((t_segmento *)iterador->data)->n_segmento--;
    }
}

void segmentar_bloque(void* memoria, uint32_t posicion, void* data, uint32_t tamanio) {
    memcpy(memoria + posicion, data, tamanio);
}

void* obtener_bloque_memoria(void* memoria, t_segmento* segmento) {
    void* bloque = malloc(segmento->tamanio);
    memcpy(bloque, memoria + segmento->inicio, segmento->tamanio);
    return bloque;
}

void realizar_compactacion() {
    uint32_t segmentos_quitados = 0;
    uint32_t tamanio_total = 0;
    uint32_t corrimiento_inicio = 0;
    uint32_t cant_segmentos = list_size(memoria_ram.mapa_segmentos);
    t_link_element* aux_segmento = memoria_ram.mapa_segmentos->head;
    log_info(logger, "Realizo compactacion");
    sem_wait(&mutex_compactacion);
    log_info(logger, "Semaforo");
    for(int i = 0; i < list_size(memoria_ram.mapa_segmentos); i++) {
        t_segmento* un_segmento = (t_segmento *)list_get(memoria_ram.mapa_segmentos, i);
        sem_wait(&un_segmento->mutex);
    }
    log_info(logger, "SemaforoSSS");
    // Realizo compactacion
    for(int i = 1; i < cant_segmentos + 1; i++) {
        // Ignoro los segmentos hasta que aparezca uno vacio
        if(((t_segmento *)aux_segmento->data)->duenio != 0  && corrimiento_inicio == 0) {
            tamanio_total += ((t_segmento *)aux_segmento->data)->tamanio;
            aux_segmento = aux_segmento->next;
            continue;
        }

        // Tomo un segmento no vacio y lo desplazo
        if(((t_segmento *)aux_segmento->data)->duenio != 0 && corrimiento_inicio != 0) {
            // Actualizo el t_segmento
            ((t_segmento *)aux_segmento->data)->n_segmento -= segmentos_quitados;
            uint32_t nuevo_inicio = ((t_segmento *)aux_segmento->data)->inicio - corrimiento_inicio;
            
            // Obtengo el bloque de memoria y lo pego en el primer lugar libre a la izquierda (siempre va a haber espacio)
            void* bloque_memoria = obtener_bloque_memoria(memoria_ram.inicio, (t_segmento *)aux_segmento->data);
            segmentar_bloque(memoria_ram.inicio, nuevo_inicio, bloque_memoria, ((t_segmento *)aux_segmento->data)->tamanio);
            tamanio_total += ((t_segmento *)aux_segmento->data)->tamanio;
            free(bloque_memoria);

            // Actualizo la posición del segmento en la patota_data que corresponde (duenio)
            ((t_segmento *)aux_segmento->data)->inicio = nuevo_inicio;
            patota_data* patota_auxiliar = (patota_data *)list_get(lista_patotas, ((t_segmento *)aux_segmento->data)->duenio - 1);
            uint32_t indice_segmento = (uint32_t)((t_segmento *)aux_segmento->data)->indice;
            patota_auxiliar->inicio_elementos[indice_segmento] = nuevo_inicio;

            // De acuerdo al tipo de segmento que sea, realizo los cambios necesarios
            if(indice_segmento == 0) {
                // Debo encontrar todos los tripulantes de la patota y actualizar su puntero a pcb
                t_list* tripulantes_a_actualizar = seg_tripulantes_de_patota(((t_segmento *)aux_segmento->data)->duenio);
                t_link_element* trip_auxiliar = tripulantes_a_actualizar->head;
                uint32_t inicio_auxiliar;
                for(int i = 0; i < list_size(tripulantes_a_actualizar); i++) {
                    inicio_auxiliar = (uint32_t)((t_segmento *)trip_auxiliar->data)->inicio + desplazamiento_parametro_trip(PCB_POINTER);
                    // actualizar_valor_tripulante(memoria_ram.inicio + inicio_auxiliar, PCB_POINTER, nuevo_inicio);
                    // actualizar_valor_tripulante(((t_segmento *)aux_segmento->data)->duenio, trip_de_segmento(inicio_auxiliar), PCB_POINTER, nuevo_inicio);
                    memcpy(memoria_ram.inicio + inicio_auxiliar, &nuevo_inicio, sizeof(uint32_t));
                    trip_auxiliar = trip_auxiliar->next;
                }
                list_destroy(tripulantes_a_actualizar);
            }
            if(indice_segmento == 1) {  // El segmento que se mueve es el segmento de tareas
                actualizar_ubicacion_tareas(memoria_ram.inicio + patota_auxiliar->inicio_elementos[0], nuevo_inicio);
            }
            
            if(indice_segmento > 1) {   // El segmento que se mueve es el segmento de un tripulante
                trip_data* aux_tripulante = tripulante_de_lista(((t_segmento *)aux_segmento->data)->duenio, indice_segmento - 1);
                aux_tripulante->inicio = nuevo_inicio;
            }
            aux_segmento = aux_segmento->next;
            continue;
        }
        
        // Tomo los segmentos vacios, los elimino y agrego sus tamanios al corrimiento de los siguientes segmentos
        if(((t_segmento *)aux_segmento->data)->duenio == 0) {
            corrimiento_inicio += ((t_segmento *)aux_segmento->data)->tamanio;
            if(i != cant_segmentos) {
                free(aux_segmento->data);
                aux_segmento = aux_segmento->next;
            }
            segmentos_quitados++;
            list_remove(memoria_ram.mapa_segmentos, i - segmentos_quitados);
        }
    }
    log_info(logger, "SEGMENTO FINAL");
    t_segmento * segmento_final = malloc(sizeof(t_segmento));
    segmento_final->duenio = 0;
    segmento_final->n_segmento = list_size(memoria_ram.mapa_segmentos);
    segmento_final->tamanio = memoria_ram.tamanio_memoria - tamanio_total;
    segmento_final->inicio = tamanio_total;
    sem_init(&segmento_final->mutex, 0, 1);
    list_add(memoria_ram.mapa_segmentos, segmento_final);
    
    log_info(logger, "SemaforoSSS");
    for(int i = 0; i < list_size(memoria_ram.mapa_segmentos); i++) {
        t_segmento* un_segmento = (t_segmento *)list_get(memoria_ram.mapa_segmentos, i);
        log_info(logger, "Semaforeo %d", i);
        sem_post(&un_segmento->mutex);
    }
    log_info(logger, "Ultimo semaforo");
    sem_post(&mutex_compactacion);
    log_info(logger, "TERMINO");
}

uint32_t memoria_libre_segmentacion() {
    t_list* segmentos_libres = list_filter(memoria_ram.mapa_segmentos, (*condicion_segmento_libre));
    uint32_t espacio_libre = 0;
    if(segmentos_libres)
        espacio_libre = (uint32_t)list_fold(segmentos_libres, 0, (*suma_tamanios));
    list_destroy(segmentos_libres);
    return espacio_libre;
}

t_list* seg_tripulantes_de_patota(uint32_t id_patota) {
	bool seg_trip_de_patota(void* segmento) {
		if(((t_segmento*)segmento)->duenio == id_patota && ((t_segmento*)segmento)->indice > 1) {
			return true;
		}
		else
			return false;
	}
	return list_filter(memoria_ram.mapa_segmentos, (*seg_trip_de_patota));
}

t_list* seg_ordenados_de_patota(uint32_t id_patota) {
	bool seg_de_patota(void* segmento) {
		if(((t_segmento*)segmento)->duenio == id_patota) {
			return true;
		}
		else
			return false;
	}

    t_list* segmentos_de_patota = list_filter(memoria_ram.mapa_segmentos, (*seg_de_patota));
    uint32_t nro_segmentos_de_patota = list_size(segmentos_de_patota);
    t_list* segmentos_ordenados = list_create();
    // uint32_t  = 0;
    for (int indice = 0; indice < nro_segmentos_de_patota; indice++) {
        bool segmento_de_indice(void *segmento) {
            return ((t_segmento *)segmento)->indice == indice ? true : false;
        }

        t_segmento* segmento_siguiente = (t_segmento *)list_find(segmentos_de_patota, (*segmento_de_indice));
        if(segmento_siguiente)
            list_add(segmentos_ordenados, segmento_siguiente);
    }
    list_destroy(segmentos_de_patota);
	return segmentos_ordenados;
}

t_segmento* segmento_desde_inicio(uint32_t inicio_segmento) {
    bool segmento_inicio(void* segmento) {
		if(((t_segmento*)segmento)->inicio == inicio_segmento) {
			return true;
		}
		else
			return false;
	}
    sem_wait(&mutex_lista_segmentos);
    t_list* mi_segmento = list_filter(memoria_ram.mapa_segmentos, (*segmento_inicio));
    t_segmento* segmento = (t_segmento *)list_get(mi_segmento, 0);
    sem_post(&mutex_lista_segmentos);
    list_destroy(mi_segmento);
    return segmento;
}

uint32_t nro_segmento_desde_inicio(uint32_t inicio_segmento) {
	t_link_element* iterador = memoria_ram.mapa_segmentos->head;
	while(((t_segmento *)iterador->data)->inicio != inicio_segmento) {
		iterador = iterador->next;
	}
	return ((t_segmento *)iterador->data)->n_segmento;
}

uint32_t trip_de_segmento(uint32_t inicio_segmento) {
    bool trip_con_inicio(void * tripulante) {
        return ((trip_data *)tripulante)->inicio == inicio_segmento ? true : false;
    }

    return ((trip_data *)list_find(lista_tripulantes, (*trip_con_inicio)))->TID;
}

void actualizar_ubicacion_tareas(void* segmento, uint32_t nueva_ubicacion) {
	uint32_t valor_int = nueva_ubicacion;
	memcpy(segmento + sizeof(int), &valor_int, sizeof(uint32_t));
}




void actualizar_caracter_segmentacion(t_segmento* mi_segmento, uint32_t desplazamiento, char data) {
    char valor = data;
    sem_wait(&mi_segmento->mutex);
    memcpy(memoria_ram.inicio + mi_segmento->inicio + desplazamiento, &valor, sizeof(char));
    sem_post(&mi_segmento->mutex);
}

void actualizar_entero_segmentacion(t_segmento* mi_segmento, uint32_t desplazamiento, uint32_t data) {
    uint32_t valor = data;
    sem_wait(&mi_segmento->mutex);
    memcpy(memoria_ram.inicio + mi_segmento->inicio + desplazamiento, &valor, sizeof(uint32_t));
    sem_post(&mi_segmento->mutex);
}

char obtener_caracter_segmentacion(t_segmento* mi_segmento, uint32_t desplazamiento) {
    char valor;
    sem_wait(&mi_segmento->mutex);
    memcpy(&valor, memoria_ram.inicio + mi_segmento->inicio + desplazamiento, sizeof(char));
    sem_post(&mi_segmento->mutex);
    
    return valor;
}

uint32_t obtener_entero_segmentacion(t_segmento* mi_segmento, uint32_t desplazamiento) {
    uint32_t valor;
    sem_wait(&mi_segmento->mutex);
    memcpy(&valor, memoria_ram.inicio + mi_segmento->inicio + desplazamiento, sizeof(uint32_t));
    sem_post(&mi_segmento->mutex);

    return valor;
}

void actualizar_string_segmentacion(t_segmento* mi_segmento, uint32_t desplazamiento, char* data) {
    sem_wait(&mi_segmento->mutex);
    memcpy(memoria_ram.inicio + mi_segmento->inicio + desplazamiento, data, strlen(data) + 1);
    sem_post(&mi_segmento->mutex);
}

void actualizar_bloque_segmentacion(t_segmento* mi_segmento, uint32_t desplazamiento, void* data, uint32_t tamanio) {
    sem_wait(&mi_segmento->mutex);
    memcpy(memoria_ram.inicio + mi_segmento->inicio + desplazamiento, data, tamanio);
    sem_post(&mi_segmento->mutex);
}

void* obtener_bloque_segmentacion(t_segmento* mi_segmento) {
    void* bloque = malloc(mi_segmento->tamanio);
    sem_wait(&mi_segmento->mutex);
    memcpy(bloque, memoria_ram.inicio + mi_segmento->inicio, mi_segmento->tamanio);
    sem_wait(&mi_segmento->mutex);
    return bloque;
}
