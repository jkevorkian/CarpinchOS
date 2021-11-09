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
	t_particion particiones[cantidad_archivos];
	int aux = 0;
	while(file_amount > 0){
		log_info(logger, "Generando Particiones");
		printf("El Path del archivo es %s \n",file_locations[file_amount-1]);
		//particiones[aux] = crear_particion((file_locations[file_amount-1]),aux);
			char*temp = crear_archivo(file_locations[file_amount-1]);
			//escribir_archivo(temp,16,"Hola Juan Carlos");
			//leer_archivo(temp,15);
			//vaciar_archivo(temp,7);
			//leer_archivo(temp,20);
		file_amount--;
		aux++;
	}
	log_info(logger, "Particiones creadas correctamente");

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

t_particion crear_particion(char* DIR_archivo, int numero_particion){
	log_info(logger, "Creando Particion %d...",numero_particion);
	char* particion_new = crear_archivo(DIR_archivo);
	char tabla_new[paginas_por_particion];
	t_particion new;
	crear_tabla_particion(tabla_new);
	new.id = numero_particion,
	new.particion = particion_new,
	new.tabla_paginas = tabla_new;
	log_info(logger, "Particion creada");
	return new;
}


void crear_tabla_marcos(char* tabla_global){ //va a guardar el estado (ocupados o desocupados) de los marcos a nivel global
	//char tabla_global[cantidad_total_paginas];
	for(int i = 0; i < cantidad_total_paginas; i++){
		tabla_global[i] = '0'; //inician todas los marcos desocupados (en 0)
	}
}
void crear_tabla_particion(char* tabla_particion){ //guarda los numeros de pagina guardadas en cada marco de cada archivo, se crea 1 por archivo
	for(int i = 0; i < cantidad_total_paginas; i++){
			tabla_particion[i] = '\0'; //inician todas los marcos desocupados
			//printf("%d\n",i);
		}
}
void ocupar_marco(char* tabla_global, char* tabla_particion,int numero_marco, int numero_pagina){
	tabla_particion[numero_marco] = numero_pagina;
	tabla_global[numero_marco] = '1';
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
