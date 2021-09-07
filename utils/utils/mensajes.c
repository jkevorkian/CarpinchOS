#include "mensajes.h"

void crear_buffer(t_mensaje* mensaje) {
	mensaje->buffer = malloc(sizeof(t_buffer));
	mensaje->buffer->tamanio = 0;
	mensaje->buffer->contenido = NULL;
}

t_mensaje* crear_mensaje(protocolo_msj cod_op) {
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->op_code = cod_op;
	crear_buffer(mensaje);
	return mensaje;
}

void agregar_a_mensaje(t_mensaje *mensaje, char *formato, ...) {
	char *ptr_form = formato;
	va_list ptr_arg;
	va_start(ptr_arg, formato);
	void* parametro_nuevo;

	while(*ptr_form) {
		if(*ptr_form != '%') {
			ptr_form++;
			continue;
		}
		ptr_form++;

		uint32_t valor_parametro;
		uint32_t tamanio_buffer;
		switch(*ptr_form) {
		case 'd':
			mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(uint32_t));
			valor_parametro = va_arg(ptr_arg, uint32_t);
			parametro_nuevo = &valor_parametro;
			tamanio_buffer = sizeof(uint32_t);
			break;
		case 's':
			parametro_nuevo = (void *)va_arg(ptr_arg, char *);
			tamanio_buffer = strlen((char *)parametro_nuevo) + 1;
			mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(uint32_t) + tamanio_buffer);

			memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, &tamanio_buffer, sizeof(uint32_t));
			mensaje->buffer->tamanio = mensaje->buffer->tamanio + sizeof(uint32_t);
			break;
		}
		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, parametro_nuevo, tamanio_buffer);
		mensaje->buffer->tamanio = mensaje->buffer->tamanio + tamanio_buffer;
	}
}

void enviar_mensaje(int socket, t_mensaje* mensaje) {
	uint32_t tamanio_buffer = mensaje->buffer->tamanio + sizeof(uint32_t);
	void * buffer_to_send = malloc(tamanio_buffer);
	memcpy(buffer_to_send, &(mensaje->op_code), sizeof(uint32_t));
	memcpy(buffer_to_send + sizeof(uint32_t), mensaje->buffer->contenido, mensaje->buffer->tamanio);
	send(socket, buffer_to_send, tamanio_buffer, 0);
	free(buffer_to_send);
}

void* recibir_parametro(int socket, tipo_msj tipo) {
	uint32_t parametro;
	uint32_t tamanio_buffer;
	switch (tipo) {
	case ENTERO:
		recv(socket, &parametro, sizeof(uint32_t), MSG_WAITALL);
		return (void *)parametro;
		break;
	case BUFFER:
		recv(socket, &tamanio_buffer, sizeof(uint32_t), MSG_WAITALL);
		char *buffer = malloc(tamanio_buffer * sizeof(char));
		recv(socket, buffer, tamanio_buffer, MSG_WAITALL);
		return buffer;
		break;
	}
	return NULL;
}

void recibir_parametros(int socket, t_list* parametros, char *formato) {
	uint32_t parametro;
	char *ptr_form = formato;
	while(*ptr_form) {
		if(*ptr_form != '%') {
			ptr_form++;
			continue;
		}
		ptr_form++;

		uint32_t tamanio_buffer;
		uint32_t num_cadenas;
		switch(*ptr_form) {
		case 'd':
			tamanio_buffer = sizeof(uint32_t);
			recv(socket, &parametro, tamanio_buffer, MSG_WAITALL);
			list_add(parametros, (void *)parametro);
			break;
		case 's':
			if(*(ptr_form + 1) == 's') num_cadenas = (uint32_t)list_get(parametros, list_size(parametros) - 1);
			else num_cadenas = 1;
			for(int i = 0; i < num_cadenas; i++) {
				tamanio_buffer = sizeof(uint32_t);
				recv(socket, &tamanio_buffer, tamanio_buffer, MSG_WAITALL);
				char *buffer = malloc(tamanio_buffer * sizeof(char));
				recv(socket, buffer, tamanio_buffer, MSG_WAITALL);
				list_add(parametros, buffer);
			}
			break;
		}
	}
}
/*
while(*ptr_form) {
	if(*ptr_form != '%') {
		ptr_form++;
		continue;
	}
	ptr_form++;

	uint32_t valor_parametro;
	uint32_t tamanio_buffer;
	switch(*ptr_form) {
	case 'd':
		mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(uint32_t));
		valor_parametro = va_arg(ptr_arg, uint32_t);
		parametro_nuevo = &valor_parametro;
		tamanio_buffer = sizeof(uint32_t);
		break;
	case 's':
		parametro_nuevo = (void *)va_arg(ptr_arg, char *);
		tamanio_buffer = strlen((char *)parametro_nuevo) + 1;
		mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(uint32_t) + tamanio_buffer);

		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, &tamanio_buffer, sizeof(uint32_t));
		mensaje->buffer->tamanio = mensaje->buffer->tamanio + sizeof(uint32_t);
		break;
	}
	memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, parametro_nuevo, tamanio_buffer);
	mensaje->buffer->tamanio = mensaje->buffer->tamanio + tamanio_buffer;
*/
t_list* recibir_mensaje(int socket) {
	protocolo_msj op_code;
	t_list* lista_parametros = list_create();
	int error;
	error = recv(socket, &op_code, sizeof(uint32_t), MSG_WAITALL);
	
	if(error == 0) {
		op_code = ER_SOC;
		list_add(lista_parametros, (void *)ER_SOC);
		// fprintf(stderr, "ERROR %d: ", error);
		// perror("read");
	}
	
	if(error == -1) {
		op_code = ER_RCV;
		list_add(lista_parametros, (void *)ER_RCV);
		// fprintf(stderr, "ERROR %d: ", error);
		// perror("read");
	}
	
	if(error > 0)
		list_add(lista_parametros, (void *)op_code);

	switch(op_code) {
	case INIT_P:
		recibir_parametros(socket, lista_parametros, "%d%d%ss");
		/*list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		uint32_t cant_tareas = (uint32_t)list_get(lista_parametros, 2);
		for(int iterador = 0; iterador < cant_tareas; iterador++) {
			list_add(lista_parametros, recibir_parametro(socket, BUFFER));
		}*/
		break;

	case BITA_C:
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		uint32_t cant_lineas = (uint32_t)list_get(lista_parametros, 1);
		for(int iterador = 0; iterador < cant_lineas; iterador++) {
			list_add(lista_parametros, recibir_parametro(socket, BUFFER));
		}
		break;

	case SHOW_T:
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;
	case SABO_I:
	case SABO_P:
	case INIT_T:
	case DATA_T:
	case ELIM_T:
	case ACTU_P:
	case BITA_D:
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;
	
	case GEN_OX:
	case CON_OX:
	case GEN_CO:
	case CON_CO:
	case GEN_BA:
	case DES_BA:
	case SND_PO:
	case ACTU_E:
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;
	
	case EXEC_1:
	case TASK_T:
		list_add(lista_parametros, recibir_parametro(socket, BUFFER));
		break;

	case ER_RCV:
		// printf("ERROR ER_RCV\n");
		break;
	case ER_SOC:
		// printf("ERROR ER_SOC\n");
		break;
	
	case SABO_F:
	case INIT_S:
	case EXEC_0:
	case NEW_PO:
	case NEXT_T:
	case TODOOK:
	case NO_SPC:
	case ER_MSJ:
	default:
		break;
	}
	return lista_parametros;
}

void liberar_mensaje_out(t_mensaje* mensaje) {
	free(mensaje->buffer->contenido);
	free(mensaje->buffer);
	free(mensaje);
}

/*static void destruir_buffer(void* elemento) {
	free(elemento);
}*/

void liberar_mensaje_in(t_list* mensaje) {
	protocolo_msj protocolo = (protocolo_msj)list_remove(mensaje, 0);
	// printf("Liberar mensaje_in\n");
	// printf("------%d------\n", protocolo);
	switch (protocolo) {
	case INIT_P:
	case BITA_C:
		list_remove(mensaje, 0);
		// list_destroy_and_destroy_elements(mensaje, destruir_buffer);
		list_destroy(mensaje);
		break;

	case TASK_T:
		// list_destroy_and_destroy_elements(mensaje, destruir_buffer);
		list_destroy(mensaje);
		break;

	case SABO_P:
	case SHOW_T:	
	case INIT_T:
	case DATA_T:
	case ELIM_T:
	case ACTU_P:
	case BITA_D:	
	case SND_PO:	
	case ACTU_E:
	case NEW_PO:
	case NEXT_T:
	case TODOOK:
	case NO_SPC:
	case ER_MSJ:
	case ER_RCV:
	case ER_SOC:
		// printf("CUAL\n");
		list_destroy(mensaje);
		// printf("CUAL\n");
		break;
	default:
		list_destroy(mensaje);
		break;
	}
}

bool validar_mensaje(t_list* mensaje_in, t_log* logger) {
	switch ((int)list_get(mensaje_in, 0)) {
	case ER_RCV:
		log_warning(logger, "Ha ocurrido un fallo inesperado en la recepción del mensaje.");
		return false;
		break;
	case ER_SOC:
		log_warning(logger, "La conexión remota se ha desconectado.");
		return false;
		break;
	}
	return true;
}
