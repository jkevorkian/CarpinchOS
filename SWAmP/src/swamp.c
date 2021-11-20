#include "swamp.h"


int main(){
	logger = log_create("swamp.log", "SWAMP", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando SWAmP");
	config = config_create("swamp.config");
	file_locations = config_get_array_value(config, "ARCHIVOS_SWAP"); //es char**
	cantidad_archivos = config_get_int_value(config, "CANTIDAD_ARCHIVOS");
	ip_swamp = config_get_string_value(config, "IP");
	marcos_maximos = config_get_int_value(config, "MARCOS_MAXIMOS");
	pagina_size = config_get_int_value(config, "TAMANIO_PAGINA");
	particion_size = config_get_int_value(config, "TAMANIO_SWAP");
	tipo_asignacion = config_get_int_value(config, "ASIGNACION");
	swamp_size = particion_size * cantidad_archivos;
	cantidad_total_paginas = (swamp_size / pagina_size);
	paginas_por_particion = (particion_size / pagina_size);


	remove("/home/utnso/SWAmP_files/swap2.bin");
	remove("/home/utnso/SWAmP_files/swap1.bin");

	int file_amount = cantidad_archivos;
	void** particiones = malloc(cantidad_archivos * sizeof(void*));
	espacio_disponible = malloc(cantidad_archivos * sizeof(int));
	int aux = 0;

	char tabla_paginas[3][cantidad_total_paginas];
	crear_tabla(tabla_paginas);

	while(file_amount > 0){
		log_info(logger, "Generando Particiones");
		printf("El Path del archivo es %s \n",file_locations[aux]);
			void* temp = crear_archivo(file_locations[aux]);
			particiones[aux] = temp;
			espacio_disponible[aux] = particion_size;
			cargar_particion_en_tabla(tabla_paginas, aux);
		file_amount--;
		aux++;
	}

	//-------------------------------------Conexiones-------------------------------------
	int puerto = config_get_int_value(config, "PUERTO");

	int server_fd = crear_conexion_servidor(ip_swamp,puerto , SOMAXCONN);

	if((server_fd < 0)) {

		close(server_fd);
		log_destroy(logger);
		return ERROR_CONEXION;
	}

	log_info(logger, "Servidor listo");

	int socket_memoria = esperar_cliente(server_fd);

	log_info(logger, "Conexion establecida con la memoria");
	while(1) {
			t_list* mensaje_in = recibir_mensaje(socket_memoria);
			t_mensaje* mensaje_out;

			if (!validar_mensaje(mensaje_in, logger)) {
				log_error(logger, "Cliente desconectado");
				//salir_proceso = true;
			} else {
				usleep(config_get_int_value(config, "RETARDO_SWAP"));
				switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
					//case INIT_S:
					case NEW_C:
						log_info(logger, "Reservando marcos");
						reservar_marcos(list_get(mensaje_in, 1), list_get(mensaje_in, 2), tabla_paginas,particiones);

						mensaje_out = crear_mensaje(TODOOK); //si falla cambiar
						enviar_mensaje(socket, mensaje_out);
						break;

					//case INIT_P:
					case EXIT_C:
						log_info(logger, "Supongo que el carpincho se murio, envienle flores a la viuda");
						eliminar_carpincho(list_get(mensaje_in, 1), tabla_paginas, particiones);
						mensaje_out = crear_mensaje(TODOOK);
						enviar_mensaje(socket, mensaje_out);
						break;

					case SET_PAGE:
						log_info(logger, "Recibi un carpincho con paginas");
						recibir_carpincho(list_get(mensaje_in, 1), list_get(mensaje_in, 2), list_get(mensaje_in, 3), tabla_paginas, particiones);
						mensaje_out = crear_mensaje(TODOOK);
						enviar_mensaje(socket, mensaje_out);
						break;

					case GET_PAGE:
						log_info(logger, "Me pidieron una pagina");
						obtener_pagina(list_get(mensaje_in, 1), list_get(mensaje_in, 2), tabla_paginas, particiones);
						mensaje_out = crear_mensaje(TODOOK);
						enviar_mensaje(socket, mensaje_out);
						break;

					case SUSPEND:
						log_info(logger, "Mensaje 2");
						break;

					default:
						log_warning(logger, "No entendi el mensaje");
						break;
				}
				liberar_mensaje_in(mensaje_in);;
			}
		}
	log_info(logger, "Finalizando SWAmP");
}


void* algoritmo_de_particiones(void** particiones, int* espacio_disponible){ //retorna la particion con mas espacio disponible
	int aux;
	int size = espacio_disponible[0];
	for(int i = 0; i < cantidad_archivos; i++){

		if(size < espacio_disponible[i]){
			size = espacio_disponible[i];
		}
	}
	for(aux = 0; aux < cantidad_archivos && (size != espacio_disponible[aux]); aux++);
	return particiones[aux];
}

char itoc(int numero){
	return numero + '0';
}
int ctoi(char caracter){
	return caracter - '0';
}

