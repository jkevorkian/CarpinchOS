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
			//escribir_archivo(temp,16,"Hola Juan Carlos");
			//leer_archivo(temp,15);
			//vaciar_archivo(temp,7);
			//leer_archivo(temp,20);
			particiones[aux] = temp;
			espacio_disponible[aux] = particion_size;
			cargar_particion_en_tabla(tabla_paginas, aux);
		file_amount--;
		aux++;
	}
	/* PRUEBAS
	carpincho_existente(tabla_paginas,0);
	cargar_pagina_en_tabla(tabla_paginas,1,0,5);
	cargar_pagina_en_tabla(tabla_paginas,0,1,9);
	cargar_pagina_en_tabla(tabla_paginas,1,0,7);
	cargar_pagina_en_tabla(tabla_paginas,1,0,8);
	cargar_pagina_en_tabla(tabla_paginas,0,1,6);
	cargar_pagina_en_tabla(tabla_paginas,0,1,1);
	cargar_pagina_en_tabla(tabla_paginas,0,1,2);
	imprimir_tabla(tabla_paginas);
	printf("El tamanio disponible en 0 es %d \n",espacio_disponible[0]);
	printf("El tamanio disponible en 1 es %d  \n",espacio_disponible[1]);


	carpincho_existente(tabla_paginas,0);
	carpincho_existente(tabla_paginas,1);

imprimir_tabla(tabla_paginas);

	leer_archivo(particiones[0],15);
	leer_archivo(particiones[1],11);

	printf("El tamanio disponible en 1 es %d \n",espacio_disponible[0]);
	printf("El tamanio disponible en 2 es %d  \n",espacio_disponible[1]);

	guardar_pagina_en_memoria("Hola Juan Carlos, soy el espiritu, como estas chupa pija 12345678",0,particiones[0]);
	leer_pagina(particiones[0],0);

	int bla;
	bla = carpincho_tiene_marcos_disponibles(tabla_paginas, 0);
	bla = carpincho_tiene_marcos_disponibles(tabla_paginas, 1);

	char* msg = "Hola Juan Carlos, soy el espiritu, como estas chupa pija ";
	char* msg2 = "0123456789121416182022242628303234363840424446485052545658606264";
	char* msg3 = "1234567891113151719212325272931333537394143454749515355575961o64";
	char* msg4 = "1234567891113151719212325272931333537394143454749515355575961o65";
*/
	char* aa = "soy 0                                                     soy 0";
	char* bb = "soy 1                                                     soy 1";
	char* cc = "soy 2                                                     soy 2";
	char* dd = "soy 3                                                     soy 3";
	char* ee = "soy 4                                                     soy 4";

	recibir_carpincho(3,1,dd, tabla_paginas, particiones);
	recibir_carpincho(3,2,dd, tabla_paginas, particiones);
	recibir_carpincho(1,4,bb, tabla_paginas, particiones);
	recibir_carpincho(0,2,aa, tabla_paginas, particiones);
	recibir_carpincho(0,3,aa, tabla_paginas, particiones);
	recibir_carpincho(4,2,ee, tabla_paginas, particiones);
	recibir_carpincho(4,3,ee, tabla_paginas, particiones);
	recibir_carpincho(2,7,cc, tabla_paginas, particiones);
	imprimir_tabla(tabla_paginas);

/*	leer_pagina(particiones[0],6);
	printf("El tamanio disponible en 0 es %d \n",espacio_disponible[0]);
	recibir_carpincho(5,7,msg2, tabla_paginas, particiones);
	imprimir_tabla(tabla_paginas);
	leer_pagina(particiones[0],6);
	leer_pagina(particiones[0],5);

	printf("El tamanio disponible en 0 es %d \n",espacio_disponible[0]);
	printf("El tamanio disponible en 1 es %d  \n",espacio_disponible[1]);
*/
	leer_pagina(particiones[1],0);
		leer_pagina(particiones[1],1);
		leer_pagina(particiones[1],2);
		leer_pagina(particiones[1],3);

	eliminar_carpincho(0, tabla_paginas, particiones);
	imprimir_tabla(tabla_paginas);
	printf("El tamanio disponible en 1 es %d  \n",espacio_disponible[1]);
	leer_pagina(particiones[1],0);
			leer_pagina(particiones[1],1);
			leer_pagina(particiones[1],2);
			leer_pagina(particiones[1],3);

	eliminar_carpincho(3, tabla_paginas, particiones);
	imprimir_tabla(tabla_paginas);
	printf("El tamanio disponible en 1 es %d  \n",espacio_disponible[1]);
	leer_pagina(particiones[0],0);
	leer_pagina(particiones[0],1);
	leer_pagina(particiones[0],2);
	leer_pagina(particiones[0],3);

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

void* algoritmo_de_particiones(void** particiones, int* espacio_disponible){ //retorna la particion con mas espacio disponible
	int aux;
	int size = espacio_disponible[0];
	printf("el size es %d\n",size);
	for(int i = 0; i < cantidad_archivos; i++){

		if(size < espacio_disponible[i]){
			size = espacio_disponible[i];
			printf("el size es %d\n",size);
		}
	}
	for(aux = 0; aux < cantidad_archivos && (size != espacio_disponible[aux]); aux++);
	return particiones[aux];
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
