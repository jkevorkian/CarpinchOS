#include "swap.h"

bool marco_mas_viejo(t_marco *marco1, t_marco *marco2);

t_movimiento *obtener_movimiento();

void* manejar_swap(void* socket_swap) {
    sem_init(&sem_movimiento, 0, 0);
    
	movimientos_pendientes = queue_create();
    pthread_mutex_init(&mutex_movimientos, NULL);

	int socket = (int)socket_swap;

    t_movimiento* mov;
	t_mensaje *mensaje_out;
	t_list *mensaje_in;
	
	bool seguir = true;

    while(seguir) {
    	mov = obtener_movimiento();
        switch(mov->accion) {
        case EXIT_C:
			mensaje_out = crear_mensaje(mov->accion);
			agregar_a_mensaje(mensaje_out, "%d", mov->id_carpincho);
			enviar_mensaje(socket, mensaje_out);

			mensaje_in = recibir_mensaje(socket);
			if((uint32_t)list_get(mensaje_in, 0) == TODOOK)
				mov->respuesta = true;
			
			// PARA TESTEAR
			// seguir = false;
			break;
		case NEW_PAGE:
			mensaje_out = crear_mensaje(mov->accion);
			agregar_a_mensaje(mensaje_out, "%d%d", mov->id_carpincho, mov->nro_pagina);
			enviar_mensaje(socket, mensaje_out);

			mensaje_in = recibir_mensaje(socket);
			if((uint32_t)list_get(mensaje_in, 0) == TODOOK)
				mov->respuesta = true;
			break;
        case SET_PAGE:
			mensaje_out = crear_mensaje(mov->accion);
			agregar_a_mensaje(mensaje_out, "%d%d%sd", mov->id_carpincho, mov->nro_pagina, config_memoria.tamanio_pagina, mov->buffer);
			enviar_mensaje(socket, mensaje_out);
			free(mov->buffer);

			mensaje_in = recibir_mensaje(socket);
			if((uint32_t)list_get(mensaje_in, 0) == TODOOK)
				mov->respuesta = true;
        	break;
        case GET_PAGE:
			mensaje_out = crear_mensaje(GET_PAGE);
			agregar_a_mensaje(mensaje_out, "%d%d", mov->id_carpincho, mov->nro_pagina);
			enviar_mensaje(socket, mensaje_out);

			mensaje_in = recibir_mensaje(socket);
			if((uint32_t)list_get(mensaje_in, 0) == DATA_PAGE) {
				mov->respuesta = true;
				mov->buffer = malloc(config_memoria.tamanio_pagina);
				memcpy(mov->buffer, (void *)list_get(mensaje_in, 2), config_memoria.tamanio_pagina);
			}
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
			mov->respuesta = false;
        	break;
        }
		sem_post(&mov->sem_respuesta);
		liberar_mensaje_out(mensaje_out);
		liberar_mensaje_in(mensaje_in);
    }
	return NULL;
}

t_movimiento *obtener_movimiento() {
	sem_wait(&sem_movimiento);
	pthread_mutex_lock(&mutex_movimientos);
	t_movimiento *mov = queue_pop(movimientos_pendientes);
	pthread_mutex_unlock(&mutex_movimientos);
	return mov;
}

bool crear_movimiento_swap(uint32_t accion, uint32_t id_carpincho, uint32_t nro_pagina, char *buffer) {
	t_movimiento *nuevo_mov = malloc(sizeof(t_movimiento));
	nuevo_mov->accion = accion;
	nuevo_mov->id_carpincho = id_carpincho;
	nuevo_mov->nro_pagina = nro_pagina;
	nuevo_mov->respuesta = false;
	nuevo_mov->buffer = NULL;

	if(accion == SET_PAGE) {
		nuevo_mov->buffer = malloc(config_memoria.tamanio_pagina);
		memcpy(nuevo_mov->buffer, buffer, config_memoria.tamanio_pagina);
	}
		
	sem_init(&nuevo_mov->sem_respuesta, 0 , 0);

	pthread_mutex_lock(&mutex_movimientos);
	queue_push(movimientos_pendientes, nuevo_mov);
	pthread_mutex_unlock(&mutex_movimientos);

	sem_post(&sem_movimiento);
	sem_wait(&nuevo_mov->sem_respuesta);

	bool respuesta = nuevo_mov->respuesta;

	if(accion == GET_PAGE) {
		memcpy(buffer, nuevo_mov->buffer, config_memoria.tamanio_pagina);
		free(nuevo_mov->buffer);
	}
	
	sem_close(&nuevo_mov->sem_respuesta);
	free(nuevo_mov);

	log_info(logger, "Finalizo movimiento. Respuesta = %d", respuesta);

	return respuesta;
}