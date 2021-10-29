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
