#include "tripulante.h"

uint32_t creo_segmento_tcb(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

int iniciar_tripulante(uint32_t id_trip, uint32_t id_patota, uint32_t pos_x, uint32_t pos_y) {
	patota_data* patota = (patota_data *)list_get(lista_patotas, id_patota - 1);

	#define espacio_ocupado_ultimo_frame (patota->memoria_ocupada % memoria_ram.tamanio_pagina)

	uint32_t inicio_pcb;
	
	trip_data* nuevo_trip = malloc(sizeof(trip_data));
	nuevo_trip->PID = id_patota;
	nuevo_trip->TID = id_trip;

	sem_wait(&mutex_lista_tripulantes);
	list_add(lista_tripulantes, nuevo_trip);
	sem_post(&mutex_lista_tripulantes);

	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		t_segmento* segmento_tcb = crear_segmento(TAMANIO_TRIPULANTE);
		segmento_tcb->duenio = id_patota;
		segmento_tcb->indice = id_trip + 1;
		patota->inicio_elementos[id_trip + 1] = segmento_tcb->inicio;
		nuevo_trip->inicio = segmento_tcb->inicio;
		inicio_pcb = patota->inicio_elementos[0];
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		asignar_frames(id_patota, frames_necesarios(TAMANIO_PAGINA - espacio_ocupado_ultimo_frame, TAMANIO_TRIPULANTE));
		patota->inicio_elementos[id_trip + 1] = patota->memoria_ocupada;
		nuevo_trip->inicio = patota->memoria_ocupada;
		patota->memoria_ocupada += TAMANIO_TRIPULANTE;
		inicio_pcb = inicio_marco_logico(patota->frames[0]);
	}

	actualizar_valor_tripulante(id_patota, id_trip, TRIP_IP, id_trip);
	actualizar_estado(id_patota, id_trip, 'N');
	actualizar_valor_tripulante(id_patota, id_trip, POS_X, pos_x);
	actualizar_valor_tripulante(id_patota, id_trip, POS_Y, pos_y);
	actualizar_valor_tripulante(id_patota, id_trip, INS_POINTER, 0);
	actualizar_valor_tripulante(id_patota, id_trip, PCB_POINTER, inicio_pcb);

	int socket_nuevo = crear_conexion_servidor(IP_RAM, 0, 1);
	pthread_t* hilo_nuevo = malloc(sizeof(pthread_t));
	nuevo_trip->socket = socket_nuevo;
	nuevo_trip->hilo = hilo_nuevo;
	
	log_info(logger, "CREO ESTRUCTURA TRIPULANTE PARA GUARDAR EN TABLA");
	pthread_create(hilo_nuevo, NULL, rutina_hilos, (void *)nuevo_trip);
	log_info(logger, "CREO HILO");
	// pthread_detach(*hilo_nuevo);
	if(CONSOLA_ACTIVA) {
		log_info(logger, "Entro a crear movimiento");
		t_movimiento* nuevo_movimiento = malloc(sizeof(t_movimiento));
		nuevo_movimiento->PID = id_patota;
		nuevo_movimiento->TID = id_trip;
		nuevo_movimiento->pos_x = pos_x;
		nuevo_movimiento->pos_y = pos_y;
		nuevo_movimiento->seguir = true;
		sem_wait(&mutex_movimiento);
		list_add(movimientos_pendientes, nuevo_movimiento);
		sem_post(&mutex_movimiento);
		sem_post(&semaforo_consola);
	}

	return puerto_desde_socket(socket_nuevo);
}

void eliminar_tripulante(uint32_t id_patota, uint32_t id_tripulante) {
	int posicion_de_lista = posicion_trip(id_patota, id_tripulante);
	if(posicion_de_lista == -1) {
		return;
	}
	log_info(logger, "Entro a eliminar_tripulante %d de la patota %d", id_tripulante, id_patota);
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	uint32_t inicio_tripulante = mi_patota->inicio_elementos[id_tripulante + 1];
	
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		sem_wait(&mutex_compactacion);
		eliminar_segmento(nro_segmento_desde_inicio(inicio_tripulante));
		sem_post(&mutex_compactacion);
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		eliminar_tripulante_paginacion(id_patota, id_tripulante);
	}

	trip_data* trip_to_kill = (trip_data *)list_get(lista_tripulantes, posicion_de_lista);
	log_info(logger, "Entro a quitar de la lista");

	sem_wait(&mutex_lista_tripulantes);
	list_remove(lista_tripulantes, posicion_de_lista);
	sem_post(&mutex_lista_tripulantes);

	pthread_cancel(*trip_to_kill->hilo);
	pthread_join(*trip_to_kill->hilo, NULL);
	log_info(logger, "Pthread cancel");
	liberar_tripulante(trip_to_kill);
	log_info(logger, "Libero tripulante");

	if(CONSOLA_ACTIVA) {
		t_movimiento* nuevo_movimiento = malloc(sizeof(t_movimiento));
		nuevo_movimiento->PID = id_patota;
		nuevo_movimiento->TID = id_tripulante;
		nuevo_movimiento->seguir = false;
		sem_wait(&mutex_movimiento);
		list_add(movimientos_pendientes, nuevo_movimiento);
		sem_post(&mutex_movimiento);
		sem_post(&semaforo_consola);
	}
}

uint32_t obtener_valor_tripulante(uint32_t id_patota, uint32_t id_trip, uint32_t nro_parametro) {
	uint32_t valor_tripulante;
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	uint32_t inicio_tripulante = mi_patota->inicio_elementos[id_trip + 1];
	
	uint32_t desplazamiento = desplazamiento_parametro_trip(nro_parametro);

	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		valor_tripulante = obtener_entero_segmentacion(segmento_desde_inicio(inicio_tripulante), desplazamiento);
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		valor_tripulante = obtener_entero_paginacion(id_patota, inicio_tripulante + desplazamiento);
	}
	return valor_tripulante;
}

char obtener_estado(uint32_t id_patota, uint32_t id_tripulante) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	uint32_t inicio_logico_tripulante = mi_patota->inicio_elementos[id_tripulante + 1];
	char valor_char;
	void* inicio_tripulante;
	t_marco* marco_auxiliar;
	
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		valor_char = obtener_caracter_segmentacion(segmento_desde_inicio(inicio_logico_tripulante), sizeof(uint32_t));
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		div_t posicion_compuesta = div(inicio_logico_tripulante, memoria_ram.tamanio_pagina);
		uint32_t pagina_actual = posicion_compuesta.quot;
		marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
		sem_wait(&marco_auxiliar->semaforo_mutex);
		incorporar_marco(mi_patota->frames[posicion_compuesta.quot]);
		inicio_tripulante = inicio_marco(mi_patota->frames[posicion_compuesta.quot]/*posicion_compuesta.quot*/) + posicion_compuesta.rem;
		memcpy(&valor_char, inicio_tripulante + sizeof(uint32_t), sizeof(char));
		marco_auxiliar->bit_uso = true;
		sem_post(&marco_auxiliar->semaforo_mutex);
	}
	return valor_char;
}

void actualizar_valor_tripulante(uint32_t id_patota, uint32_t id_trip, para_trip nro_parametro, uint32_t nuevo_valor) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	uint32_t inicio_tripulante = mi_patota->inicio_elementos[id_trip + 1];
	
	uint32_t desplazamiento = desplazamiento_parametro_trip(nro_parametro);

	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		actualizar_entero_segmentacion(segmento_desde_inicio(inicio_tripulante), desplazamiento, nuevo_valor);
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		actualizar_entero_paginacion(id_patota, inicio_tripulante + desplazamiento, nuevo_valor);
	}
}

void actualizar_estado(uint32_t id_patota, uint32_t id_tripulante, char nuevo_valor) {
	if(nuevo_valor != 0) {
		patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
		uint32_t inicio_logico_tripulante = mi_patota->inicio_elementos[id_tripulante + 1];
		t_marco* marco_auxiliar;

		void* inicio_tripulante;
		if(memoria_ram.esquema_memoria == SEGMENTACION) {
			actualizar_caracter_segmentacion(segmento_desde_inicio(inicio_logico_tripulante), sizeof(uint32_t), nuevo_valor);
		}
		if(memoria_ram.esquema_memoria == PAGINACION) {
			div_t posicion_compuesta = div(inicio_logico_tripulante, memoria_ram.tamanio_pagina);
			uint32_t pagina_actual = posicion_compuesta.quot;
			marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
			char valor = nuevo_valor;

			sem_wait(&marco_auxiliar->semaforo_mutex);
			incorporar_marco(mi_patota->frames[posicion_compuesta.quot]);
			inicio_tripulante = inicio_marco(mi_patota->frames[posicion_compuesta.quot]/*posicion_compuesta.quot*/) + posicion_compuesta.rem;
			memcpy(inicio_tripulante + sizeof(uint32_t), &valor, sizeof(char));
			sem_post(&marco_auxiliar->semaforo_mutex);
		}
	}
}

trip_data* tripulante_de_lista(uint32_t id_patota, uint32_t id_trip) {
	return (trip_data *)list_get(lista_tripulantes, posicion_trip(id_patota, id_trip));
}

int posicion_trip(uint32_t id_patota, uint32_t id_trip) {
	int posicion = -1;
	bool encontre = false;
	sem_wait(&mutex_lista_tripulantes);
	t_link_element* iterador_tripulante = lista_tripulantes->head;
	trip_data* trip_auxiliar;
	while(iterador_tripulante) {
		posicion++;
		trip_auxiliar = (trip_data *)iterador_tripulante->data;
		if(trip_auxiliar->PID == id_patota && trip_auxiliar->TID == id_trip) {
			encontre = true;
			break;
		}
		iterador_tripulante = iterador_tripulante->next;
	}
	sem_post(&mutex_lista_tripulantes);
	if(encontre)
		return posicion;
	else
		return -1;
}

void liberar_tripulante(trip_data* trip_to_kill) {
    close(trip_to_kill->socket);
	free(trip_to_kill->hilo);
	free(trip_to_kill);
}

void eliminar_tripulante_paginacion(uint32_t id_patota, uint32_t id_tripulante) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	for(int i = 2; i < mi_patota->cantidad_elementos; i++) {
		if(i <= id_tripulante + 1)
			continue;
		if(i > id_tripulante + 1 && posicion_trip(id_patota, i - 1) != -1) {
			trip_data * trip_auxiliar = (trip_data *)list_get(lista_tripulantes, posicion_trip(id_patota, i - 1));
			uint32_t pid = obtener_valor_tripulante(id_patota, i - 1, TRIP_IP);
			char estado = obtener_estado(id_patota, i - 1);
			uint32_t px = obtener_valor_tripulante(id_patota, i - 1, POS_X);
			uint32_t py = obtener_valor_tripulante(id_patota, i - 1, POS_Y);
			uint32_t ip = obtener_valor_tripulante(id_patota, i - 1, INS_POINTER);
			uint32_t pcb_ip = obtener_valor_tripulante(id_patota, i - 1, PCB_POINTER);
			mi_patota->inicio_elementos[i] -= TAMANIO_TRIPULANTE;
			trip_auxiliar->inicio -= TAMANIO_TRIPULANTE;
			actualizar_valor_tripulante(id_patota, i - 1, TRIP_IP, pid);
			actualizar_estado(id_patota, i - 1, estado);
			actualizar_valor_tripulante(id_patota, i - 1, POS_X, px);
			actualizar_valor_tripulante(id_patota, i - 1, POS_Y, py);
			actualizar_valor_tripulante(id_patota, i - 1, INS_POINTER, ip);
			actualizar_valor_tripulante(id_patota, i - 1, PCB_POINTER, pcb_ip);
		}
	}
	mi_patota->memoria_ocupada -= TAMANIO_TRIPULANTE;
	reasignar_frames(id_patota);
}

uint32_t desplazamiento_parametro_trip(para_trip parametro) {
	uint32_t desplazamiento = 0;
	switch(parametro) {
		case PCB_POINTER:	desplazamiento += sizeof(uint32_t);
		case INS_POINTER:	desplazamiento += sizeof(uint32_t);
		case POS_Y:			desplazamiento += sizeof(uint32_t);
		case POS_X:			desplazamiento += sizeof(uint32_t);
		case ESTADO:		desplazamiento += sizeof(char);
		case TRIP_IP:
			break;
	}
	return desplazamiento;
}