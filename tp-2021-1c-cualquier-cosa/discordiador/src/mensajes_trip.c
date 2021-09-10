#include "mensajes_trip.h"

bool respuesta_OK(t_list* respuesta, char* mensaje_fallo) {
	if(!validar_mensaje(respuesta, logger)) {
		log_warning(logger, "FALLO EN COMUNICACION");
		return false;
	}else if((int)list_get(respuesta, 0) != TODOOK) {
		log_warning(logger, "%s", mensaje_fallo);
		return false;
	}
	return true;
}

bool enviar_y_verificar(t_mensaje* mensaje_out, int socket, char* mensaje_error) {
	bool resultado;

	enviar_mensaje(socket, mensaje_out);
	t_list* mensaje_in = recibir_mensaje(socket);
	resultado = respuesta_OK(mensaje_in, mensaje_error);

	liberar_mensaje_in(mensaje_in);
	liberar_mensaje_out(mensaje_out);

	return resultado;
}

char* solicitar_tarea(tripulante* trip) {
	char* tarea = "no_task";

	if(RAM_ACTIVADA) {
		t_mensaje* mensaje_out = crear_mensaje(NEXT_T);
		enviar_mensaje(trip->socket_ram, mensaje_out);
		t_list* mensaje_in = recibir_mensaje(trip->socket_ram);

		if(!validar_mensaje(mensaje_in, logger))
			log_warning(logger, "FALLO EN MENSAJE CON HILO RAM\n");
		else if((int)list_get(mensaje_in, 0) == TASK_T)
			tarea = (char*)list_get(mensaje_in, 1);

		liberar_mensaje_out(mensaje_out);
		liberar_mensaje_in(mensaje_in);
	} else {
		switch(trip->posicion[0]) {
			case 1:
				tarea = "ESPERAR;2;3;3";
				break;
			case 2:
				tarea = "GENERAR_OXIGENO 10;3;2;3";
				break;
			case 3:
				tarea = "CONSUMIR_OXIGENO 5;4;1;3";
				break;
			case 4:
				tarea = "GENERAR_BASURA 8;5;2;3";
				break;
			case 5:
				tarea = "DESCARTAR_BASURA 6;6;3;3";
				break;
			case 6:
				tarea = "GENERAR_COMIDA 6;7;2;3";
				break;
			case 7:
				tarea = "CONSUMIR_COMIDA 6;8;1;3";
				break;
			case 8:
				tarea = "GENERAR_BASURA 1;9;2;8";
				break;
			default: break;
		}
	}

	return tarea;
}

void avisar_movimiento(tripulante* trip) {
	if(RAM_ACTIVADA) {
		t_mensaje* mensaje_out = crear_mensaje(ACTU_P);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[0], ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[1], ENTERO);

		enviar_y_verificar(mensaje_out, trip->socket_ram, "Fallo en la actualizacion de posicion en RAM");
	}

	if(MONGO_ACTIVADO) {
		t_mensaje* mensaje_out = crear_mensaje(ACTU_P);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[0], ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[1], ENTERO);

		enviar_y_verificar(mensaje_out, trip->socket_mongo, "Fallo en la actualizacion de posicion en MONGO");
	}
}

void actualizar_estado(tripulante* trip, estado estado_trip) {
	trip->estado = estado_trip;

	if(RAM_ACTIVADA) {
		t_mensaje* mensaje_out = crear_mensaje(ACTU_E);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->estado, ENTERO);

		enviar_y_verificar(mensaje_out, trip->socket_ram, "Fallo en la actualizacion del estado en RAM");
	}
}
