#include "swap.h"

t_marco** paginas_reemplazo(uint32_t id_carpincho);
uint32_t nro_paginas_reemplazo();
t_marco *marco_viejo(t_marco *marco1, t_marco *marco2);
uint32_t obtener_tiempo(char tipo, t_marco *marco);

t_movimiento *obtener_movimiento();

void hacer_swap_in(int socket, t_movimiento *mov);
void hacer_swap_out(int socket, t_movimiento *mov);

void* manejar_swap(void* socket_swap) {
    sem_init(sem_movimiento, 0, 0);
    movimientos_pendientes = queue_create();
    pthread_mutex_init(&mutex_movimientos, NULL);

	int socket = (int)socket_swap;

    t_movimiento* mov;
	t_mensaje *mensaje_out;
	t_list *mensaje_in;

    while(true) {
    	mov = obtener_movimiento();
        switch(mov->accion) {
        case EXIT_C:
        	break;
		case NEW_PAGE:
			mensaje_out = crear_mensaje(mov->accion);
			agregar_a_mensaje(mensaje_out, "%d%d", mov->id_carpincho, mov->nro_pagina);
			enviar_mensaje(socket, mensaje_out);
			
			mensaje_in = recibir_mensaje(socket);
			if((uint32_t)list_get(mensaje_in, 0) == TODOOK)
				mov->respuesta = true;
			if((uint32_t)list_get(mensaje_in, 0) == NO_MEMORY)
				mov->respuesta = false;
			break;
        case SET_PAGE:
        	hacer_swap_out((int)socket_swap, mov);
			// if(list_get(mensaje_in, 0) == TODOOK)
			// 	mov->respuesta = true;
        	break;
        case GET_PAGE:
        	hacer_swap_in((int)socket_swap, mov);
			// if(list_get(mensaje_in, 0) == TODOOK)
			// 	mov->respuesta = true;
        	break;
		case RM_PAGE:
			mensaje_out = crear_mensaje(mov->accion);
			agregar_a_mensaje(mensaje_out, "%d", mov->id_carpincho);
			enviar_mensaje(socket, mensaje_out);
			
			mensaje_in = recibir_mensaje(socket);
			if((uint32_t)list_get(mensaje_in, 0) == TODOOK)
				mov->respuesta = true;
			break;
        default:
        	break;
        }
		// sem_post(&mov->sem_respuesta);
    }
    return NULL;
}

t_movimiento *obtener_movimiento() {
	sem_wait(sem_movimiento);
	pthread_mutex_lock(&mutex_movimientos);
	t_movimiento *mov = queue_pop(movimientos_pendientes);
	pthread_mutex_unlock(&mutex_movimientos);
	return mov;
}

void hacer_swap_out(int socket, t_movimiento *mov) {
	t_mensaje *mensaje_out = crear_mensaje(mov->accion);
	agregar_a_mensaje(mensaje_out, "%d%d%s", mov->id_carpincho, mov->nro_pagina, mov->buffer);
	enviar_mensaje(socket, mensaje_out);
	free(mov->buffer);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(socket);
	mov->respuesta = true;
	liberar_mensaje_in(mensaje_in);
}

void hacer_swap_in(int socket, t_movimiento *mov) {
	t_mensaje *mensaje_out = crear_mensaje(GET_PAGE);
	agregar_a_mensaje(mensaje_out, "%d%d", mov->id_carpincho, mov->nro_pagina);
	enviar_mensaje(socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(socket);
	mov->respuesta = true;
	mov->buffer = malloc(config_memoria.tamanio_pagina);
	memcpy(mov->buffer, (char *)list_get(mensaje_in, 1), config_memoria.tamanio_pagina);
	liberar_mensaje_in(mensaje_in);
}

bool crear_movimiento_swap(uint32_t accion, uint32_t id_carpincho, uint32_t nro_pagina, char *buffer) {
	t_movimiento *nuevo_mov = malloc(sizeof(t_movimiento *));
	nuevo_mov->accion = accion;
	nuevo_mov->id_carpincho = id_carpincho;
	nuevo_mov->nro_pagina = nro_pagina;
	if(accion == SET_PAGE) {
		nuevo_mov->buffer = malloc(config_memoria.tamanio_pagina);
		memcpy(nuevo_mov->buffer, buffer, config_memoria.tamanio_pagina);
	}
	log_info(logger, "Accion: %d/%d", nuevo_mov->accion, accion);
	log_info(logger, "ID: %d/%d", nuevo_mov->id_carpincho, id_carpincho);
	log_info(logger, "NRO_PAGINA: %d/%d", nuevo_mov->nro_pagina, nro_pagina);

	pthread_mutex_lock(&mutex_movimientos);
	queue_push(movimientos_pendientes, nuevo_mov);
	pthread_mutex_unlock(&mutex_movimientos);

	log_info(logger, "Posteo movimiento");

	sem_post(sem_movimiento);

	log_info(logger, "Espero respuesta");
	//sem_wait(&nuevo_mov->sem_respuesta);
	sem_wait(sem_respuesta);
	
	// int error = sem_wait(nuevo_mov->sem_respuesta);
	//printf("%d", errno);
	//perror("Falla semaforo");
	// usleep(3500000);
	log_info(logger, "Recibo respuesta");

	bool respuesta = nuevo_mov->respuesta;
	log_info(logger, "0");
	if(nuevo_mov->buffer) {
		log_info(logger, "0");
		if(accion == GET_PAGE) {
			log_info(logger, "1");
			memcpy(buffer, nuevo_mov->buffer, config_memoria.tamanio_pagina);
			log_info(logger, "2");
			log_info(logger, "buffer %s/%s", nuevo_mov->buffer, buffer);
		}
	}
	else {
		log_warning(logger, "buffer vacio");
	}
	log_info(logger, "0");
	// if(nuevo_mov->buffer) free(nuevo_mov->buffer);
	// free(nuevo_mov);

	log_info(logger, "Finalizo movimiento. Respuesta = %d", respuesta);

	return respuesta;
}

t_marco *buscar_por_clock(t_marco **lista_paginas, uint32_t nro_paginas) {

	bool encontre_marco = false;
	t_marco* marco_referencia;
	uint32_t puntero_clock = 0;
	uint32_t ciclo = 0;
	while(!encontre_marco) {
		marco_referencia = lista_paginas[puntero_clock];

		switch(ciclo) {
		case 0:
			if(!marco_referencia->bit_uso && !marco_referencia->bit_modificado)
				encontre_marco = true;
			break;
		case 1:
			if(!marco_referencia->bit_uso && !marco_referencia->bit_modificado)
				encontre_marco = true;
			else
				// TODO reset_bit_uso(marco_referencia);	// Importante por concurrencia ?
			break;
		}
		puntero_clock++;
		if(puntero_clock == nro_paginas) {
			puntero_clock = 0;
			ciclo = ciclo ? false : true;
		}
	}
	return marco_referencia;
}

t_marco *buscar_por_lru(t_marco **lista_paginas, uint32_t nro_paginas) {
	t_marco *marco_referencia = lista_paginas[0];

	for(int i = 1; i < nro_paginas; i++) {
		marco_referencia = marco_viejo(marco_referencia, lista_paginas[i]);
	}
	return marco_referencia;
}

t_marco *realizar_algoritmo_reemplazo(uint32_t id_carpincho) {
	t_marco** lista_paginas = paginas_reemplazo(id_carpincho);
	uint32_t nro_paginas = nro_paginas_reemplazo();

	t_marco* marco_a_reemplazar;
	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL) {
		marco_a_reemplazar = obtener_marco_libre();
	}
	if(!marco_a_reemplazar) {
		if(config_memoria.algoritmo_reemplazo == LRU)
			marco_a_reemplazar = buscar_por_lru(lista_paginas, nro_paginas);
		if(config_memoria.algoritmo_reemplazo == CLOCK)
			marco_a_reemplazar = buscar_por_clock(lista_paginas, nro_paginas);
	}
	reservar_marco(marco_a_reemplazar); // ?????
	return marco_a_reemplazar;
}

t_marco** paginas_reemplazo(uint32_t id_carpincho) {
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)
		return obtener_marcos_proceso(id_carpincho);
	else
		return memoria_ram.mapa_fisico;
}

t_marco** obtener_marcos_proceso(uint32_t id_carpincho) {
	uint32_t nro_marcos = config_memoria.cant_marcos;
	t_marco **marcos_proceso = calloc(nro_marcos, sizeof(t_marco *));
	uint32_t nro_marcos_encontrados = 0;
	uint32_t marcos_memoria = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina - 1;
	for(int i = 0; nro_marcos > nro_marcos_encontrados && nro_marcos > marcos_memoria; i++) {
		if(memoria_ram.mapa_fisico[i]->nro_real == id_carpincho) {
			marcos_proceso[nro_marcos_encontrados] = memoria_ram.mapa_fisico[i];
			nro_marcos_encontrados++;
		}
	}
	if(nro_marcos_encontrados < nro_marcos)
		marcos_proceso = realloc(marcos_proceso, sizeof(t_marco *) * nro_marcos_encontrados);
	return marcos_proceso;
}

uint32_t nro_paginas_reemplazo() {
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)
		return config_memoria.cant_marcos;
	else
		return config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;
}

t_marco *marco_viejo(t_marco *marco1, t_marco *marco2) {
	// Formato temporal para LRU de marcos: HH:MM:SS
	if(obtener_tiempo('H', marco1) > obtener_tiempo('H', marco2))
		return marco1;
	if(obtener_tiempo('H', marco1) < obtener_tiempo('H', marco2))
		return marco2;
	if(obtener_tiempo('M', marco1) > obtener_tiempo('M', marco2))
		return marco1;
	if(obtener_tiempo('M', marco1) < obtener_tiempo('M', marco2))
		return marco2;
	if(obtener_tiempo('S', marco1) > obtener_tiempo('S', marco2))
		return marco1;
	//if(obtener_tiempo('S', marco1) < obtener_hora('S', marco2))
	else
		return marco2;
}

uint32_t obtener_tiempo(char tipo, t_marco *marco) {
	char tiempo[2];
	switch(tipo) {
	case 'H':
		memcpy(tiempo, marco->temporal, 2);
		break;
	case 'M':
		memcpy(tiempo, marco->temporal + 3, 2);
		break;
	case 'S':
		memcpy(tiempo, marco->temporal + 6, 2);
		break;
	}

	return atoi(tiempo);
}

void* manejar_test_swap(void* socket_swap) {
    sem_movimiento = malloc(sizeof(sem_t));
	sem_respuesta = malloc(sizeof(sem_t));
	sem_init(sem_movimiento, 0, 0);
	sem_init(sem_respuesta, 0, 0);
    movimientos_pendientes = queue_create();
    pthread_mutex_init(&mutex_movimientos, NULL);

	int socket = (int)socket_swap;

    t_movimiento* mov;
	t_mensaje *mensaje_out;
	t_list *mensaje_in;
	int var_switch;
	data_socket(socket, logger);
	bool seguir = true;
    while(seguir) {
		log_info(logger, "Espero nuevo movimiento");
    	mov = obtener_movimiento();

		log_info(logger, "Accion: %d", mov->accion);
		var_switch = mov->accion;
		log_info(logger, "ID: %d", mov->id_carpincho);
		log_info(logger, "NRO_PAGINA: %d", mov->nro_pagina);
        switch(var_switch) {
        case EXIT_C:
			mensaje_out = crear_mensaje(mov->accion);
			log_info(logger, "Creo mensaje");
			agregar_a_mensaje(mensaje_out, "%d", mov->id_carpincho);
			log_info(logger, "Agrego a mensaje");
			enviar_mensaje(socket, mensaje_out);
			log_info(logger, "Envio mensaje");

			mensaje_in = recibir_mensaje(socket);
			log_info(logger, "La swap me devuelve %d", (uint32_t)list_get(mensaje_in, 0));
			if((uint32_t)list_get(mensaje_in, 0) == TODOOK)
				mov->respuesta = true;
			if((uint32_t)list_get(mensaje_in, 0) == NO_MEMORY)
				mov->respuesta = false;
			
			seguir = false;
        	continue;
		case NEW_PAGE:
			log_info(logger, "Recibo un new_page");
			
			mensaje_out = crear_mensaje(mov->accion);
			log_info(logger, "Creo mensaje");
			agregar_a_mensaje(mensaje_out, "%d%d", mov->id_carpincho, mov->nro_pagina);
			log_info(logger, "Agrego a mensaje");
			enviar_mensaje(socket, mensaje_out);
			log_info(logger, "Envio mensaje");

			mensaje_in = recibir_mensaje(socket);
			log_info(logger, "La swap me devuelve %d", (uint32_t)list_get(mensaje_in, 0));
			if((uint32_t)list_get(mensaje_in, 0) == TODOOK)
				mov->respuesta = true;
			if((uint32_t)list_get(mensaje_in, 0) == NO_MEMORY)
				mov->respuesta = false;
			break;
        case SET_PAGE:
        	log_info(logger, "Recibo un set_page");

			mensaje_out = crear_mensaje(mov->accion);
			agregar_a_mensaje(mensaje_out, "%d%d%s", mov->id_carpincho, mov->nro_pagina, mov->buffer);
			enviar_mensaje(socket, mensaje_out);
			if(!mov->buffer)	free(mov->buffer);
			else				log_warning(logger, "No hay nada en el buffer che");
			// liberar_mensaje_out(mensaje_out);

			mensaje_in = recibir_mensaje(socket);
			log_info(logger, "La swap me devuelve %d", (uint32_t)list_get(mensaje_in, 0));
			if((uint32_t)list_get(mensaje_in, 0) == TODOOK)
				mov->respuesta = true;
			if((uint32_t)list_get(mensaje_in, 0) == NO_MEMORY)
				mov->respuesta = false;
			liberar_mensaje_in(mensaje_in);
        	break;
        case GET_PAGE:
			log_info(logger, "Recibo un get_page");

			mensaje_out = crear_mensaje(GET_PAGE);
			agregar_a_mensaje(mensaje_out, "%d%d", mov->id_carpincho, mov->nro_pagina);
			enviar_mensaje(socket, mensaje_out);
			// liberar_mensaje_out(mensaje_out);

			mensaje_in = recibir_mensaje(socket);
			log_info(logger, "La swap me devuelve %d", (uint32_t)list_get(mensaje_in, 0));
			log_info(logger, "Contenido: %s", (char *)list_get(mensaje_in, 1));
			if((uint32_t)list_get(mensaje_in, 0) == DATA_CHAR) {
				log_info(logger, "Cargo la pagina");
				mov->respuesta = true;
				mov->buffer = malloc(config_memoria.tamanio_pagina);
				memcpy(mov->buffer, (char *)list_get(mensaje_in, 1), config_memoria.tamanio_pagina);
			}	
			if((uint32_t)list_get(mensaje_in, 0) == NO_MEMORY)
				mov->respuesta = false;			
			// liberar_mensaje_in(mensaje_in);
        	break;
		case RM_PAGE:
			log_info(logger, "Recibo un rm_page");

			mensaje_out = crear_mensaje(mov->accion);
			agregar_a_mensaje(mensaje_out, "%d", mov->id_carpincho);
			enviar_mensaje(socket, mensaje_out);
			liberar_mensaje_out(mensaje_out);
			
			mensaje_in = recibir_mensaje(socket);
			if((uint32_t)list_get(mensaje_in, 0) == TODOOK)
				mov->respuesta = true;
			else
				mov->respuesta = false;
			liberar_mensaje_in(mensaje_in);
			break;
        default:
			log_info(logger, "Recibo un default");			
			mov->respuesta = false;
        	break;
        }
		log_info(logger, "Actualizo respuesta");
		sem_post(sem_respuesta);
		log_info(logger, "Finalizo movimiento");
    }
    return NULL;
}