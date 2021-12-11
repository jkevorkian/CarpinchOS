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

	remove("/home/utnso/SWAmP_files/swap1.bin");
	remove("/home/utnso/SWAmP_files/swap2.bin");
	rmdir("/home/utnso/SWAmP_files");
	char* punto_montaje = "/home/utnso/SWAmP_files/";
	mkdir(punto_montaje,0755);

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

	int server_fd = crear_conexion_servidor(ip_swamp,puerto , 1);

	if((server_fd < 0)) {
		log_info(logger, "Fallo de conexion");
		close(server_fd);
		log_destroy(logger);
		return ERROR_CONEXION;
	}
	data_socket(server_fd, logger);
	log_info(logger, "Servidor listo");

	int socket_memoria = esperar_cliente(server_fd);
	data_socket(socket_memoria, logger);

	log_info(logger, "Conexion establecida con la memoria");
	int i = 0;
	while(1) {

			t_list* mensaje_in = recibir_mensaje(socket_memoria);
			t_mensaje* mensaje_out;

			if (!validar_mensaje(mensaje_in, logger)) {
				log_error(logger, "Cliente desconectado");
				//salir_proceso = true;
			} else {
				usleep(config_get_int_value(config, "RETARDO_SWAP"));
				switch((int)list_get(mensaje_in, 0)) {

					case NEW_PAGE:
						log_info(logger, "Reservando marcos");
						int reserva = reservar_marcos((int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), tabla_paginas,particiones);
						if(reserva < 0){
							mensaje_out = crear_mensaje(NO_MEMORY);
						}
						else{
							mensaje_out = crear_mensaje(TODOOK);
						}
						enviar_mensaje(socket_memoria, mensaje_out);
						break;

					case SET_PAGE:
						log_info(logger, "Memoria me pidio actualizar la pagina % del carpincho %d", (uint32_t)list_get(mensaje_in, 2), (uint32_t)list_get(mensaje_in, 1));
						char* buffer;
						buffer = malloc(pagina_size);
						memcpy(buffer, (void *)list_get(mensaje_in, 3), pagina_size);
						loggear_pagina(logger, buffer);
						int aux = recibir_carpincho((int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), buffer, tabla_paginas, particiones);
						//printf("El contenido de la pagina es %s \n",buffer);
						if(aux < 0){
							mensaje_out = crear_mensaje(NO_MEMORY);
						}
						else{
							mensaje_out = crear_mensaje(TODOOK);
						}
						enviar_mensaje(socket_memoria, mensaje_out);
						free(buffer);
						break;

					case GET_PAGE:
						log_info(logger, "Memoria me pidio obtener la pagina %d del carpincho %d", (uint32_t)list_get(mensaje_in, 2), (uint32_t)list_get(mensaje_in, 1));
						//char* mensaje = string_new();
						//string_append(&mensaje,obtener_pagina((int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), tabla_paginas, particiones));
						//strcpy(mensaje,obtener_pagina((int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), tabla_paginas, particiones));
						char* buffer_aux;
						buffer_aux = malloc(pagina_size);
						memcpy(buffer_aux, obtener_pagina((int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), tabla_paginas, particiones),pagina_size);
						loggear_pagina(logger, buffer_aux);

						//printf("El contenido de la pagina es %s \n",buffer_aux);
						//mensaje_out = crear_mensaje(DATA_CHAR);
						mensaje_out = crear_mensaje(DATA_PAGE);
						agregar_a_mensaje(mensaje_out, "%sd",pagina_size,buffer_aux);
						log_info(logger, "Agregue mensaje");
						//agregar_a_mensaje(mensaje_out, "%s",buffer_aux);
						enviar_mensaje(socket_memoria, mensaje_out);
						log_info(logger, "Envie mensaje");
						//free(mensaje);
						free(buffer_aux);
						break;

					case RM_PAGE:	// 14
						log_info(logger, "Memoria me pidio eliminar una pagina del carpincho %d", (int)list_get(mensaje_in, 1));
						int ultima_pagina = ultima_pagina_carpincho((int)list_get(mensaje_in, 1), tabla_paginas);
						//log_info(logger, "Memoria me pidio eliminar la pagina %d del carpincho %d",(int)list_get(mensaje_in, 2), (int)list_get(mensaje_in, 1));
						eliminar_pagina(tabla_paginas, (int)list_get(mensaje_in, 1), ultima_pagina, particiones);
						mensaje_out = crear_mensaje(TODOOK);
						enviar_mensaje(socket_memoria, mensaje_out);
						break;

					case EXIT_C:
						log_info(logger, "Supongo que el carpincho se murio, envienle flores a la viuda");
						eliminar_carpincho((int)list_get(mensaje_in, 1), tabla_paginas, particiones);
						//mensaje_out = crear_mensaje(TODOOK);
						//enviar_mensaje(socket, mensaje_out);
						break;

					default:
						log_warning(logger, "No entendi el mensaje");
						break;
				}
				liberar_mensaje_in(mensaje_in);
				i++;
			}
			imprimir_tabla(tabla_paginas);
		}
	log_info(logger, "Finalizando SWAmP");
	free(particiones);
	free(espacio_disponible);
}


void* algoritmo_de_particiones(void** particiones, int* espacio_disponible){ //retorna la particion con mas espacio disponible
	int aux;
	int size = espacio_disponible[0];
	printf("El espacio en la particion 0 es %d\n",size);
	for(int i = 0; i < cantidad_archivos; i++){
		printf("El espacio en la particion %d es %d\n",i,espacio_disponible[i]);
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

void loggear_pagina(t_log *logger, void *pagina) {
	uint8_t byte;
	for(int i = 0; i < 32; i++) {
		memcpy(&byte, pagina + i, 1);
		printf("%3d|", byte);

		div_t barra_n = div(i + 1, 4);
		if(i > 0 && !barra_n.rem)
			printf("\n");
	}
}
