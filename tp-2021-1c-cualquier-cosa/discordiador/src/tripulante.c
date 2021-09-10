#include "tripulante.h"

tripulante* crear_tripulante(int x, int y, int patota, int id, int socket_ram, int socket_mongo) {
	tripulante* nuevo_tripulante = malloc(sizeof(tripulante));

	nuevo_tripulante->id_trip = id;
	nuevo_tripulante->id_patota = patota;
	nuevo_tripulante->estado = NEW;
	nuevo_tripulante->posicion[0] = x;
	nuevo_tripulante->posicion[1] = y;
	nuevo_tripulante->socket_ram = socket_ram;
	nuevo_tripulante->socket_mongo = socket_mongo;
	nuevo_tripulante->contador_ciclos = 0;
	nuevo_tripulante->continuar = true;

	sem_init(&nuevo_tripulante->sem_blocked, 0, 0);
	sem_init(&nuevo_tripulante->sem_running, 0, 0);

	pthread_create(&nuevo_tripulante->hilo, NULL, rutina_tripulante, nuevo_tripulante);

	return nuevo_tripulante;
}

void* rutina_tripulante(void* t) {
	tripulante* trip = (tripulante*) t;
	char* tarea;

	tarea = solicitar_tarea(trip);

	if(strcmp(tarea, "no_task") != 0) {
		agregar_ready(trip);
		sem_wait(&trip->sem_running);
	}

	if(!continuar_planificacion)
		sem_wait(&trip->sem_running);

	while(trip->continuar && strcmp(tarea, "no_task") != 0) {
		ejecutar(tarea, trip);

		if(trip->continuar)
			tarea = solicitar_tarea(trip);
	}

	if(trip->continuar) {
		log_error(logger,"Tripulante %d finalizando trabajo", trip->id_trip);

		quitar_running(trip);
		actualizar_estado(trip, EXIT);
	} else
		sem_post(&trip->sem_blocked);

	sem_destroy(&trip->sem_blocked);
	sem_destroy(&trip->sem_running);

	return 0;
}

void ejecutar(char* input, tripulante* trip) {
	log_info(logger,"Tripulante %d va a ejecutar tarea %s", trip->id_trip, input);

	char** buffer = string_split(input, ";");

	if(MONGO_ACTIVADO) {
		t_mensaje* mensaje_out = crear_mensaje(EXEC_1);
		agregar_parametro_a_mensaje(mensaje_out, (void*)input, BUFFER);
		enviar_y_verificar(mensaje_out, trip->socket_mongo, "Fallo en comunicacion con el mongo");
	}

	moverse(trip, atoi(buffer[1]), atoi(buffer[2]));

	if(trip->continuar) {
		char** comando_tarea = string_split(buffer[0], " ");
		tareas tarea = stringToEnum(comando_tarea[0]);

		if(tarea == ESPERAR)
			esperar(atoi(buffer[3]), trip);
		else
			ejecutar_io(trip, tarea, atoi(comando_tarea[1]), atoi(buffer[3]));

		liberar_split(comando_tarea);

		if(trip->continuar) {
			log_warning(logger,"Tripulante %d termino de ejecutar", trip->id_trip);

			if(MONGO_ACTIVADO)
				enviar_y_verificar(crear_mensaje(EXEC_0), trip->socket_mongo, "Fallo en comunicacion con el mongo");
		}
	}

	liberar_split(buffer);
}

void moverse(tripulante* trip, int pos_x, int pos_y) {

	while(trip->continuar && trip->posicion[0] != pos_x) {

		(trip->posicion[0] < pos_x) ? trip->posicion[0]++ : trip->posicion[0]--;
		avisar_movimiento(trip);
		sleep(ciclo_CPU);

		puede_continuar(trip);
	}

	while(trip->continuar && trip->posicion[1] != pos_y) {

		(trip->posicion[1] < pos_y) ? trip->posicion[1]++ : trip->posicion[1]--;
		avisar_movimiento(trip);
		sleep(ciclo_CPU);

		puede_continuar(trip);
	}

	if(!trip->continuar)
		log_info(logger,"Tripulante %d quitado de running (movimiento)", trip->id_trip);
	else
		log_info(logger,"Tripulante %d llego a %d|%d", trip->id_trip, trip->posicion[0], trip->posicion[1]);

}

void ejecutar_io(tripulante* trip, tareas tarea, int cantidad, int tiempo_io) {
	puede_continuar(trip);

	quitar_running(trip);
	agregar_blocked(trip);

	log_info(logger,"Tripulante %d blockeado por IO", trip->id_trip);
	sem_wait(&trip->sem_blocked);

	if(!continuar_planificacion) {
		log_info(logger,"Tripulante %d pausado", trip->id_trip);
		sem_wait(&trip->sem_running);
		log_info(logger,"Tripulante %d reactivado", trip->id_trip);
	}

	if(trip->continuar) {
		log_info(logger,"Tripulante %d desbloqueado por IO", trip->id_trip);

		if(MONGO_ACTIVADO) {
			if(hay_sabotaje)
				sem_wait(&finalizo_sabotaje);

			t_mensaje* mensaje_out;

			switch(tarea){
				case GENERAR_OXIGENO:
					mensaje_out = crear_mensaje(GEN_OX);
					break;
				case CONSUMIR_OXIGENO:
					mensaje_out = crear_mensaje(CON_OX);
					break;
				case GENERAR_COMIDA:
					mensaje_out = crear_mensaje(GEN_CO);
					break;
				case CONSUMIR_COMIDA:
					mensaje_out = crear_mensaje(CON_CO);
					break;
				case GENERAR_BASURA:
					mensaje_out = crear_mensaje(GEN_BA);
					break;
				case DESCARTAR_BASURA:
					mensaje_out = crear_mensaje(DES_BA);
					break;
				case ESPERAR: break;
			}

			agregar_parametro_a_mensaje(mensaje_out, (void*)cantidad, ENTERO);
			enviar_y_verificar(mensaje_out, trip->socket_mongo, "Fallo al cargar respuesta en el MONGO");
		}
		else {
			switch(tarea){
				case GENERAR_OXIGENO:
					log_info(logger,"Tripulante %d: Generando %d de oxigeno",trip->id_trip, cantidad);
					break;
				case CONSUMIR_OXIGENO:
					log_info(logger,"Tripulante %d: Consumiendo %d de oxigeno",trip->id_trip, cantidad);
					break;
				case GENERAR_COMIDA:
					log_info(logger,"Tripulante %d: Generando %d de comida",trip->id_trip, cantidad);
					break;
				case CONSUMIR_COMIDA:
					log_info(logger,"Tripulante %d: Consumiendo %d de comida",trip->id_trip, cantidad);
					break;
				case GENERAR_BASURA:
					log_info(logger,"Tripulante %d: Generando %d de basura",trip->id_trip, cantidad);
					break;
				case DESCARTAR_BASURA:
					log_info(logger,"Tripulante %d: Descartando %d de basura",trip->id_trip, cantidad);
					break;
				case ESPERAR: break;
			}
		}

		for(int i = 0; i < tiempo_io && trip->continuar; i++) {
			log_info(logger, "Trip %d ejecuto %d de I/O", trip->id_trip, i+1);
			sleep(ciclo_CPU);

			if(hay_sabotaje)
				sem_wait(&finalizo_sabotaje);

			if(!continuar_planificacion) {
				log_info(logger,"Tripulante %d pausado", trip->id_trip);
				sem_wait(&trip->sem_running);
				log_info(logger,"Tripulante %d reactivado", trip->id_trip);
			}
		}

		trip_block = NULL;
		sem_post(&io_disponible);

		if(trip->continuar) {
			agregar_ready(trip);
			sem_wait(&trip->sem_running);
		} else
			log_info(logger, "Trip %d quitado de blocked", trip->id_trip);

	} else {
		quitar(trip, cola_blocked);
	}
}

void esperar(int tiempo, tripulante* trip) {
	for(int i = 0; i < tiempo; i++) {
		if(!trip->continuar) {
			i = tiempo;
			log_info(logger,"Tripulante %d quitado de running (esperar)", trip->id_trip);
		}
		else {
			log_info(logger,"Tripulante %d: Espero %d de %d",trip->id_trip, i+1, tiempo);
			sleep(ciclo_CPU);
			puede_continuar(trip);
		}
	}
}
