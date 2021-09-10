#include "mongo.h"

int main() {
	logger = log_create("mongo.log", "MONGO", 1, LOG_LEVEL_INFO);
	config = config_create("imongostore.config");
	punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");
	block_size = config_get_long_value(config, "BLOCK_SIZE");
	blocks_amount = config_get_long_value(config, "BLOCKS_AMOUNT");
	salir_proceso = true;
	int id_trip_global, id_patota_global = 0;
	lista_tripulantes = list_create();

	int server_fd = crear_conexion_servidor(IP_MONGO, config_get_int_value(config, "PUERTO"), 1);

	if(!validar_socket(server_fd, logger)) {
		close(server_fd);
		log_destroy(logger);
		return ERROR_CONEXION;
	}

	log_info(logger, "Servidor listo");

	int socket_discord = esperar_cliente(server_fd);

	log_info(logger, "Conexión establecida con el discordiador");

	mkdir(punto_montaje,0755); //se crea el directorio /FileSystem/

	if(crear_superBloque() && crear_blocks()) { //creo el superBloque si no esta creado, los bloques y si to-do esta bien continua el programa

		pthread_create(&hilo_actualizador_block,NULL,uso_blocks,&blocks);

		generar_directorio("/Files");
		generar_directorio("/Files/Bitacoras");

		salir_proceso = false;
		//imprimir_bitmap(bitmap);
	}

	while(!salir_proceso) {
		t_list* mensaje_in = recibir_mensaje(socket_discord);
		t_mensaje* mensaje_out;

		if (!validar_mensaje(mensaje_in, logger)) {
			log_error(logger, "Cliente desconectado");
			salir_proceso = true;
		} else {
			switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
				case INIT_S:
					log_info(logger, "Iniciando el detector de sabotajes");

					inicializar_detector_sabotajes(socket_discord);

					break;
				case INIT_P:
					log_info(logger, "Discordiador inicio una patota");

					id_patota_global++;
					id_trip_global = 1;

					mensaje_out = crear_mensaje(TODOOK);
					enviar_mensaje(socket_discord, mensaje_out);
					liberar_mensaje_out(mensaje_out);
					break;
				case INIT_T:
					log_info(logger, "Discordiador solicito iniciar un tripulante");
					tripulante* trip = malloc(sizeof(tripulante));

					trip->id_patota = id_patota_global;
					trip->id_trip = id_trip_global;
					trip->posicion_x = (int)list_get(mensaje_in, 1);
					trip->posicion_y = (int)list_get(mensaje_in, 2);
					trip->socket_discord = crear_conexion_servidor(IP_MONGO, 0, 1);
					trip->dir_bitacora = crear_bitacora(trip->id_trip, trip->id_patota);

					pthread_t hilo_nuevo;
					pthread_create(&hilo_nuevo, NULL, rutina_trip, trip);

					list_add(lista_tripulantes, trip);

					mensaje_out = crear_mensaje(SND_PO);
					agregar_parametro_a_mensaje(mensaje_out, (void *)puerto_desde_socket(trip->socket_discord), ENTERO);
					enviar_mensaje(socket_discord, mensaje_out);
					liberar_mensaje_out(mensaje_out);

					id_trip_global++;
					break;
				case BITA_D:
					log_info(logger, "Discordiador solicito la bitacora del tripulante %d de la patota %d", (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));

					mensaje_out = crear_mensaje(BITA_C);

					char** bitacora = obtener_bitacora((int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));

					if(bitacora != NULL) {
						int cantidad_lineas = 0;

						while(bitacora[cantidad_lineas] != NULL)
							cantidad_lineas++;

						log_info(logger, "Cantidad de lineas a enviar %d", cantidad_lineas -1);
						agregar_parametro_a_mensaje(mensaje_out, (void*)cantidad_lineas -1, ENTERO);

						for(int i = 0; i < cantidad_lineas-1; i++)
							agregar_parametro_a_mensaje(mensaje_out, (void*)bitacora[i], BUFFER);

						enviar_mensaje(socket_discord, mensaje_out);
						liberar_split(bitacora);
					} else {
						mensaje_out = crear_mensaje(NO_SPC);
						enviar_mensaje(socket_discord, mensaje_out);
					}

					liberar_mensaje_out(mensaje_out);
					break;
				default:
					log_warning(logger, "No entendi el mensaje");
					break;
			}
			liberar_mensaje_in(mensaje_in);;
		}
	}

	log_warning(logger, "FINALIZANDO MONGO");

	while(!list_is_empty(lista_tripulantes))
		free((tripulante*)list_remove(lista_tripulantes, 0));

	pthread_mutex_destroy(&actualizar_blocks);
	pthread_mutex_destroy(&actualizar_bitmap);

	//free(blocks);
	//free(blocks_copy);
	free(punto_montaje);

	close(server_fd);
	pthread_cancel(hilo_actualizador_block);
	log_destroy(logger);



	return 0;
}

void* rutina_trip(void* t) {
	tripulante* trip = (tripulante*) t;

	int socket_cliente = esperar_cliente(trip->socket_discord);
	log_info(logger, "Iniciado el tripulante %d de la patota %d", trip->id_trip, trip->id_patota);

	char* ultima_tarea;
	bool continuar = true;

	while(continuar) {
		t_mensaje* mensaje_out;
		t_list* mensaje_in = recibir_mensaje(socket_cliente);

		if(!validar_mensaje(mensaje_in, logger)) {
			log_info(logger, "Finalizo el discordiador");
			continuar = false;
		} else {
			switch ((int)list_get(mensaje_in, 0)) {
				case EXEC_1:
					log_info(logger, "EXEC_1 - Tripulante %d de la patota %d ejecutando tarea: %s", trip->id_trip, trip->id_patota, (char*)list_get(mensaje_in, 1));

					ultima_tarea = (char*)list_get(mensaje_in, 1);
					comienza_tarea(ultima_tarea,trip->dir_bitacora);

					mensaje_out = crear_mensaje(TODOOK);
					enviar_mensaje(socket_cliente, mensaje_out);
					break;
				case EXEC_0:
					log_info(logger, "EXEC_0 - Tripulante %d de la patota %d detuvo ejecucion", trip->id_trip, trip->id_patota);
					mensaje_out = crear_mensaje(TODOOK);

					finaliza_tarea(ultima_tarea, trip->dir_bitacora);
					free(ultima_tarea);

					enviar_mensaje(socket_cliente, mensaje_out);
					break;
				case GEN_OX:
					log_info(logger, "GEN_OX - Tripulante %d de la patota %d generando %d de oxigeno", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));

					sumar_caracteres('O',(int)list_get(mensaje_in, 1));

					mensaje_out = crear_mensaje(TODOOK);
					enviar_mensaje(socket_cliente, mensaje_out);
					break;
				case CON_OX:
					log_info(logger, "CON_OX - Tripulante %d de la patota %d consumiendo %d de oxigeno", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));

					quitar_caracteres('O',(int)list_get(mensaje_in, 1));

					mensaje_out = crear_mensaje(TODOOK);
					enviar_mensaje(socket_cliente, mensaje_out);
					break;
				case GEN_CO:
					log_info(logger, "GEN_CO - Tripulante %d de la patota %d generando %d de comida", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
					sumar_caracteres('C',(int)list_get(mensaje_in, 1));

					mensaje_out = crear_mensaje(TODOOK);
					enviar_mensaje(socket_cliente, mensaje_out);
					break;
				case CON_CO:
					log_info(logger, "CON_CO - Tripulante %d de la patota %d consumiendo %d de comida", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));

					quitar_caracteres('C',(int)list_get(mensaje_in, 1));

					mensaje_out = crear_mensaje(TODOOK);
					enviar_mensaje(socket_cliente, mensaje_out);
					break;
				case GEN_BA:
					log_info(logger, "GEN_BA - Tripulante %d de la patota %d generando %d de basura", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));

					sumar_caracteres('B',(int)list_get(mensaje_in, 1));

					mensaje_out = crear_mensaje(TODOOK);
					enviar_mensaje(socket_cliente, mensaje_out);
					break;
				case DES_BA:
					log_info(logger, "DES_BA - Tripulante %d de la patota %d desechando %d de basura", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));

					quitar_caracteres('B',(int)list_get(mensaje_in, 1));

					mensaje_out = crear_mensaje(TODOOK);
					enviar_mensaje(socket_cliente, mensaje_out);
					break;
				case ACTU_P:
					log_info(logger, "ACTU_P - Tripulante %d de la patota %d nueva posicion: %d|%d", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));

					actualizar_posicion(trip,(int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), trip->dir_bitacora);

					mensaje_out = crear_mensaje(TODOOK);
					enviar_mensaje(socket_cliente, mensaje_out);
					break;
			}
		}
		liberar_mensaje_out(mensaje_out);
		liberar_mensaje_in(mensaje_in);
	}
	free(trip->dir_bitacora);//cambiarle el nombre al archivo para que se mantenga el bitmap


	return 0;
}

void crear_metadata(char* DIR_metadata, char caracter_llenado){
	log_info(logger, "Buscando archivos de recursos ya existentes");

	FILE* metadata = fopen(DIR_metadata,"rb");

	if(metadata != NULL) {
		log_info(logger, "Archivo existente encontrado");
		fclose(metadata);
	} else {
		log_info(logger, "Archivos previos no encontrados, Generando metadata");

		metadata = fopen(DIR_metadata,"wb");
		t_config* temp = config_create(DIR_metadata);

		temp->path = string_duplicate(DIR_metadata);
		char str_caracter_llenado[2] = {caracter_llenado , '\0'};

		char* md5 = crear_MD5("[]", caracter_llenado, 0);
		config_save_in_file(temp, DIR_metadata);

		config_set_value(temp,"SIZE","0");
		config_set_value(temp,"BLOCK_COUNT","0");
		config_set_value(temp,"BLOCKS","[]");
		config_set_value(temp,"CARACTER_LLENADO",str_caracter_llenado);
		config_set_value(temp,"MD5_ARCHIVO",md5);
		config_save(temp);
		config_destroy(temp);
		free(md5);

		log_info(logger, "Archivo de metadata generado");
		fclose(metadata);
	}
}
