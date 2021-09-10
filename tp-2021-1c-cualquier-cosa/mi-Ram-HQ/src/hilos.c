#include "hilos.h"

void* rutina_hilos(void* data) {
	// t_log* logger = log_create("miramhq.log", "HILOX", 1, LOG_LEVEL_INFO);
	// log_info(logger, "HOLA MUNDO, SOY UN HILO %d", variable);
	int variable = 0;
    trip_data* tripulante = (trip_data *)data;
	// patota_data* segmento_patota = (patota_data *)list_get(lista_patotas, tripulante->PID - 1);
	tareas_data* segmento_tareas = (tareas_data *)list_get(lista_tareas, tripulante->PID - 1);
	uint32_t ip;
	char* tarea_nueva;

	int socket_cliente = esperar_cliente(tripulante->socket);
    bool cliente_conectado = true;
    if(socket_cliente < 0)
        cliente_conectado = false;
	
    uint32_t posicion_x;	// Para consola
	uint32_t posicion_y;	// Para consola
    
	t_mensaje* mensaje_out;
	t_list* mensaje_in;

	while(cliente_conectado) {
		mensaje_in = recibir_mensaje(socket_cliente);
		// log_info(logger, "SOY UN HILO: ENTRO AL SEM_WAIT\n");
        // sem_wait(tripulante->semaforo_hilo);
		// printf("Sobrevivi al sem_wait\n");
		switch ((int)list_get(mensaje_in, 0)) {
		case NEXT_T:
			// printf("RECIBI NEXT_T\n");
			ip = obtener_valor_tripulante(tripulante->PID, tripulante->TID, INS_POINTER);
			if(ip + 1 > segmento_tareas->cant_tareas) {
				// IP fuera de rango
				mensaje_out = crear_mensaje(ER_MSJ);
			}
			else {
				// IP valido
				// tarea_nueva = obtener_tarea_p(memoria_ram.inicio + segmento_patota->inicio_elementos[1], segmento_tareas, ip);
				tarea_nueva = obtener_tarea(tripulante->PID, ip);
				actualizar_valor_tripulante(tripulante->PID, tripulante->TID, INS_POINTER, ip + 1);
				mensaje_out = crear_mensaje(TASK_T);
				agregar_parametro_a_mensaje(mensaje_out, tarea_nueva, BUFFER);
				free(tarea_nueva);
			}
            enviar_mensaje(socket_cliente, mensaje_out);
		    liberar_mensaje_out(mensaje_out);
			break;
		case ACTU_P:
			posicion_x = (uint32_t)list_get(mensaje_in, 1);
			posicion_y = (uint32_t)list_get(mensaje_in, 2);
			
			actualizar_valor_tripulante(tripulante->PID, tripulante->TID, POS_X, posicion_x);
			actualizar_valor_tripulante(tripulante->PID, tripulante->TID, POS_Y, posicion_y);

			// Creo movimiento para que lo capte la consola
			t_movimiento* nuevo_movimiento = malloc(sizeof(t_movimiento));
			nuevo_movimiento->PID = tripulante->PID;
			nuevo_movimiento->TID = tripulante->TID;
			nuevo_movimiento->pos_x = posicion_x;
			nuevo_movimiento->pos_y = posicion_y;
			nuevo_movimiento->seguir = true;
			sem_wait(&mutex_movimiento);
			list_add(movimientos_pendientes, nuevo_movimiento);
			sem_post(&mutex_movimiento);
			sem_post(&semaforo_consola);

			mensaje_out = crear_mensaje(TODOOK);
            enviar_mensaje(socket_cliente, mensaje_out);
		    liberar_mensaje_out(mensaje_out);
			break;
		case ACTU_E:
			actualizar_estado(tripulante->PID, tripulante->TID, caracter_de_estado((uint32_t)list_get(mensaje_in, 1)));

			mensaje_out = crear_mensaje(TODOOK);
            enviar_mensaje(socket_cliente, mensaje_out);
		    liberar_mensaje_out(mensaje_out);
			break;
		case ELIM_T:
        case ER_SOC:
        case ER_RCV:
		default:
			log_info(logger, "Ultima vuelta. Nos vamos despidiendo");
			cliente_conectado = false;
			break;
		}
        // sem_post(tripulante->semaforo_hilo);
        liberar_mensaje_in(mensaje_in);
		variable++;
	}

	if(CONSOLA_ACTIVA) {
		t_movimiento* nuevo_movimiento = malloc(sizeof(t_movimiento));
		nuevo_movimiento->PID = tripulante->PID;
		nuevo_movimiento->TID = tripulante->TID;
		nuevo_movimiento->seguir = false;
		sem_wait(&mutex_movimiento);
		list_add(movimientos_pendientes, nuevo_movimiento);
		sem_post(&mutex_movimiento);
		sem_post(&semaforo_consola);
		log_info(logger, "Cree movimiento letal");
	}
	
	// sem_wait(&mutex_lista_tripulantes);
	// list_remove(lista_tripulantes, posicion_trip(tripulante->PID, tripulante->TID));
	// sem_post(&mutex_lista_tripulantes);
	// log_info(logger, "Me quité de la lista de tripulantes");
	// list_remove(lista_tripulantes, posicion_trip(tripulante->PID, tripulante->TID));
	// sem_wait(&mutex_segmentacion);
	// eliminar_segmento(mapa_segmentos, nro_segmento_tripulante(tripulante->inicio));
    // sem_post(&mutex_segmentacion);
	// log_info(logger, "Elimine mi propio segmento");
	// close(socket_cliente);
    // close(tripulante->socket);
    // sem_destroy(tripulante->semaforo_hilo);
	// free(tripulante->semaforo_hilo);
	// free(tripulante);
	// log_info(logger, "ME MUEROOOOOO %d\n", variable);
	return 0;
}

char caracter_de_estado(estado nuevo_estado) {
	switch (nuevo_estado) {
		case NEW:		return 'N';
		case BLOCKED:	return 'B';
		case READY:		return 'R';
		case RUNNING:	return 'E';
		default:		return 0;
	}
}