#include "swap.h"

t_movimiento *obtener_movimiento();
void hacer_swap_in(int socket, t_movimiento *mov);
void hacer_swap_out(int socket, t_movimiento *mov);

void* manejar_swap(void* socket_swap) {
    sem_init(&sem_movimiento, 0, 0);
    movimientos_pendientes = queue_create();
    pthread_mutex_init(&mutex_movimientos, NULL);

    t_movimiento* movimiento_pendiente;
    while (true) {
    	movimiento_pendiente = obtener_movimiento();
        switch(movimiento_pendiente->accion) {
        case NEW_C:
        	break;
        case EXIT_C:
        	break;
        case SET_PAGE:
        	hacer_swap_out(movimiento_pendiente);
        	break;
        case GET_PAGE:
        	hacer_swap_in(movimiento_pendiente);
        	break;
        case SUSPEND:
        	suspender_proceso(movimiento_pendiente->id_carpincho);
        	break;
        // case MORIR:
        // 	morir();
        default:
        	break;
        }
        if(movimiento_pendiente->buffer)	free(movimiento_pendiente->buffer);
        	free(movimiento_pendiente);
    }
    // free(continuar_consola);
    return NULL;
}

t_movimiento *obtener_movimiento() {
	sem_wait(&sem_movimiento);
	pthread_mutex_lock(&mutex_movimientos);
	t_movimiento *movimiento_pendiente = queue_pop(movimientos_pendientes);
	pthread_mutex_unlock(&mutex_movimientos);
	return movimiento_pendiente;
}

void hacer_swap_out(int socket, t_movimiento *mov) {
	t_mensaje *mensaje_out = crear_mensaje(mov->accion);
	agregar_a_mensaje(mensaje_out, "%d%d%s", mov->id_carpincho, mov->nro_pagina, mov->buffer);
	enviar_mensaje(socket, mensaje_out);
	free(mov->buffer);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(socket);
	// if((int)list_get(0, mensaje_in) == TODOOK)
	// 	return EXITO;
	// else
	// 	return ;
	liberar_mensaje_in(mensaje_in);
}

void hacer_swap_in(int socket, t_movimiento *mov) {
	t_mensaje *mensaje_out = crear_mensaje(GET_PAGE);
	agregar_a_mensaje(mensaje_out, "%d%d", mov->id_carpincho, mov->nro_pagina);
	enviar_mensaje(socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(socket);

	if((int)list_get(0, mensaje_in) != DATA_PAGE) {
		// ERROR
	}
	// else
	// TODO Ver si se puede simplificar la copia del buffer
	char *buffer = malloc(TAMANIO_PAGINA);
	memcpy(buffer, (char *)list_get(mensaje_in, 1), TAMANIO_PAGINA);

	//pagina = asignar_pagina_memoria();
	//escribir_pagina(pagina, buffer);
	liberar_mensaje_in(mensaje_in);
}

void crear_movimiento_swap(uint8_t accion, uint32_t id_carpincho, uint32_t nro_pagina, char *buffer) {
	t_movimiento *nuevo_mov = malloc(sizeof(t_movimiento *));
	nuevo_mov->accion = accion;
	nuevo_mov->id_carpincho = id_carpincho;
	nuevo_mov->nro_pagina = nro_pagina;
	if(accion == SET_PAGE)
		nuevo_mov->buffer = strdup(buffer);

	pthread_mutex_lock(&mutex_movimientos);
	queue_push(&movimientos_pendientes, nuevo_mov);
	pthread_mutex_unlock(&mutex_movimientos);

	sem_post(&sem_movimiento);
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
				reset_bit_uso(marco_referencia);	// Importante por concurrencia ?
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

	t_marco *marco_a_reemplazar;
	if(config_memoria.algoritmo_reemplazo == LRU)
		marco_a_reemplazar = buscar_por_lru(lista_paginas, nro_paginas);
	if(config_memoria.algoritmo_reemplazo == CLOCK)
		marco_a_reemplazar = buscar_por_clock(lista_paginas, nro_paginas);

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
	for(int i = 0; nro_marcos > nro_marcos_encontrados; i++) {
		if(memoria_ram.mapa_fisico[i]->nro_real == id_carpincho) {
			marcos_proceso[nro_marcos_encontrados] = memoria_ram.mapa_fisico[i];
			nro_marcos_encontrados++;
		}
	}
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
	if(obtener_tiempo('H', marco1) > obtener_hora('H', marco2))
		return marco1;
	if(obtener_tiempo('H', marco1) < obtener_hora('H', marco2))
		return marco2;
	if(obtener_tiempo('M', marco1) > obtener_hora('M', marco2))
		return marco1;
	if(obtener_tiempo('M', marco1) < obtener_hora('M', marco2))
		return marco2;
	if(obtener_tiempo('S', marco1) > obtener_hora('S', marco2))
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
