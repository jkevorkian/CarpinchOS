#include "swamp.h"


int main(){
	logger = log_create("swamp.log", "SWAMP", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando SWAmP");
	config = config_create("swamp.config");
	file_locations = config_get_array_value(config, "ARCHIVOS_SWAP"); //es char**
	cantidad_archivos = config_get_int_value(config, "CANTIDAD_ARCHIVOS");
	ip_swamp = config_get_string_value(config, "IP");
	pagina_size = config_get_int_value(config, "TAMANIO_PAGINA");
	particion_size = config_get_int_value(config, "TAMANIO_SWAP");
	swamp_size = particion_size * cantidad_archivos;
	cantidad_total_paginas = (swamp_size / pagina_size);
	paginas_por_particion = (particion_size / pagina_size);

	remove("/home/utnso/SWAmP_files/swap2.bin");
	remove("/home/utnso/SWAmP_files/swap1.bin");

	int file_amount = cantidad_archivos;
	void** particiones = malloc(cantidad_archivos * sizeof(void*));
	int* espacio_disponible = malloc(cantidad_archivos * sizeof(int));
	int aux = 0;

	char tabla_paginas[3][cantidad_total_paginas];
	crear_tabla(tabla_paginas);

	while(file_amount > 0){
		log_info(logger, "Generando Particiones");
		printf("El Path del archivo es %s \n",file_locations[file_amount-1]);
			void* temp = crear_archivo(file_locations[file_amount-1]);
			escribir_archivo(temp,16,"Hola Juan Carlos");
			//leer_archivo(temp,15);
			//vaciar_archivo(temp,7);
			//leer_archivo(temp,20);
			particiones[aux] = temp;
			espacio_disponible[aux] = particion_size;
			cargar_particion_en_tabla(tabla_paginas, aux);
		file_amount--;
		aux++;
	}
	cargar_pagina_en_tabla(tabla_paginas,1,0,5);
	cargar_pagina_en_tabla(tabla_paginas,0,1,9);
	cargar_pagina_en_tabla(tabla_paginas,1,0,7);
	cargar_pagina_en_tabla(tabla_paginas,0,1,6);
	cargar_pagina_en_tabla(tabla_paginas,0,1,1);

imprimir_tabla(tabla_paginas);

	leer_archivo(particiones[0],15);
	leer_archivo(particiones[1],11);

	printf("El tamanio disponible en 1 es %d \n",espacio_disponible[0]);
	printf("El tamanio disponible en 2 es %d  \n",espacio_disponible[1]);

	guardar_pagina_en_memoria("Hola Juan Carlos, soy el espiritu, como estas chupa pija 12345678",0,particiones[0]);
	leer_pagina(particiones[0],0);


//-------------------------------------Conexiones-------------------------------------
/*	int server_fd = crear_conexion_servidor(ip_swamp, config_get_int_value(config, "PUERTO"), 1);

	//if(!validar_socket(server_fd, logger)) {
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
				switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
					//case INIT_S:
					case 1:
						log_info(logger, "Mensaje 1");
						break;

					//case INIT_P:
					case 2:
						log_info(logger, "Mensaje 2");
						break;

					default:
						log_warning(logger, "No entendi el mensaje");
						break;
				}
				liberar_mensaje_in(mensaje_in);;
			}
		}*/
	log_info(logger, "Finalizando SWAmP");
}



char itoc(int numero){
	return numero + '0';
}






/*
void crear_archivo(char* DIR_archivo){


	int file = open(DIR_archivo, O_RDWR | O_CREAT);
	if(file == -1){
		printf("No se pudo generar el archivo\n");
		return;
	}


	void* particion = mmap(NULL, particion_size , PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
	truncate(DIR_archivo, particion_size);

	rellenar_archivo(particion);
	leer_archivo(particion,10);
	escribir_archivo(particion,16,"Hola Juan Carlos");
	leer_archivo(particion,15);
	vaciar_archivo(particion,7);
	leer_archivo(particion,20);
}
void rellenar_archivo(void* particion){
	int desplazamiento = 0;
	int caracteres_por_escribir = particion_size;
	log_info(logger, "Rellenando archivo...");
	while(caracteres_por_escribir > 0){
		memcpy(particion + desplazamiento, "\0", sizeof(char));
		desplazamiento++;
		caracteres_por_escribir--;
	}
	log_info(logger, "Archivo rellenado con exito");
}
void escribir_archivo(void* particion, int largo, char* mensaje){
	log_info(logger, "Escribiendo archivo...");
	int desplazamiento = 0;
	while(largo > 0){
		memcpy(particion + desplazamiento, string_from_format("%c",mensaje[desplazamiento]), sizeof(char));
		desplazamiento++;
		largo--;
	}
	log_info(logger, "Archivo escrito");
}
void leer_archivo(void* particion,int cantidad_caracteres){
	log_info(logger, "Leyendo archivo...");
	char mensaje[cantidad_caracteres+1];
	char* p = mensaje;
	memcpy(p, particion, cantidad_caracteres+1);
	mensaje[cantidad_caracteres+1] = '\0';
	printf("el archivo contiene: %s \n",mensaje);

}
void vaciar_archivo(void* particion,int cantidad_caracteres){
	log_info(logger, "Vaciando archivo...");
	int desplazamiento = 0;
	while(cantidad_caracteres > 0){
		memcpy(particion + desplazamiento, "0", sizeof(char));
		desplazamiento++;
		cantidad_caracteres--;
	}

}
*/
