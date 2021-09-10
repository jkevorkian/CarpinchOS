#include "sabotajes.h"

void inicializar_detector_sabotajes(int socket_discord) {
	ubicaciones_sabotajes = config_get_array_value(config, "POSICIONES_SABOTAJE");
	contador_sabotajes = 0;

	signal(SIGUSR1, analizador_sabotajes);

	int socket_d = crear_conexion_servidor(IP_MONGO, 0, 1);

	t_mensaje* mensaje_out = crear_mensaje(SND_PO);
	agregar_parametro_a_mensaje(mensaje_out, (void *)puerto_desde_socket(socket_d), ENTERO);
	enviar_mensaje(socket_discord, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	socket_sabotajes = esperar_cliente(socket_d);
	log_info(logger, "Detector de sabotajes iniciado exitosamente");
}

void analizador_sabotajes(int senial) {
	char** ubicacion_dividida = string_split(ubicaciones_sabotajes[contador_sabotajes], "|");
	int pos_x = atoi(ubicacion_dividida[0]);
	int pos_y = atoi(ubicacion_dividida[1]);
	liberar_split(ubicacion_dividida);

	log_info(logger, "Sabotaje recibido en ubicacion %d|%d", pos_x, pos_y);

	log_warning(logger, "Enviando sabotaje al discordiador");
	t_mensaje* mensaje_out = crear_mensaje(SABO_P);
	agregar_parametro_a_mensaje(mensaje_out, (void*)pos_x, ENTERO);
	agregar_parametro_a_mensaje(mensaje_out, (void*)pos_y, ENTERO);
	enviar_mensaje(socket_sabotajes, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list* mensaje_in = recibir_mensaje(socket_sabotajes);

	mensaje_out = crear_mensaje(TODOOK);

	if((int)list_get(mensaje_in, 0) == SABO_I) {
		int id_trip = (int)list_get(mensaje_in, 1);
		int id_patota = (int)list_get(mensaje_in, 2);
		tripulante* trip = obtener_tripulante(id_trip, id_patota);

		log_info(logger, "El tripulante %d de la patota %d esta yendo a la ubicacion del sabotaje", id_trip, id_patota);
		inicio_sabotaje(trip->dir_bitacora);

		enviar_mensaje(socket_sabotajes, mensaje_out);
		liberar_mensaje_in(mensaje_in);

		mensaje_in = recibir_mensaje(socket_sabotajes);

		if((int)list_get(mensaje_in, 0) == SABO_F) {
			log_info(logger, "El tripulante llego a la ubicacion del sabotaje");
			fin_sabotaje(trip->dir_bitacora);
			enviar_mensaje(socket_sabotajes, mensaje_out);
			//todo RESOLVER SABOTAJE
			contador_sabotajes++;
		} else
			log_error(logger, "Se Murio");

		liberar_mensaje_in(mensaje_in);
		liberar_mensaje_out(mensaje_out);
	}else {
		log_error(logger, "No se pudo resolver el sabotaje");
		liberar_mensaje_in(mensaje_in);
		liberar_mensaje_out(mensaje_out);
	}
}

//////////////////////////////////FUNCIONES RESOLVEDORAS////////////////////////////////
/*
void arreglar_BlockCount_superBloque(){
	char* DIR_SuperBLoque=string_new();
	string_append(&DIR_SuperBLoque,obtener_directorio("/superBloque.ims"));
	int superBlock_file = open(DIR_SuperBLoque, O_CREAT | O_RDWR, 0664);

    void* superBloque = mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_SHARED, superBlock_file, 0);

    uint32_t SUP_block_size;
    memcpy(&SUP_block_size, superBloque, sizeof(uint32_t));

    int SUP_blocks_amount;
    memcpy(&SUP_blocks_amount, sizeof(uint32_t)+superBloque, sizeof(uint32_t));

    char* DIR_Bloques=string_new();
    string_append(&DIR_Bloques,obtener_directorio("/Blocks.ims"));
   struct stat sb;
   stat(DIR_Bloques,&sb);
   int tamanio_blocks=sb.st_size; //obtengo los bytes que pesa el Blocks.ims
   int real_block_amount=tamanio_blocks/SUP_block_size;

    printf("blocks amount superbloque: %d, cantidad real de bloques: %d\n",SUP_blocks_amount,real_block_amount);

    if(SUP_blocks_amount!=real_block_amount){
    	log_info(logger, "Se saboteo el BlockCount del archivo de SuperBloque. Solucionando sabotaje...");
    	 memcpy(superBloque+sizeof(int), &real_block_amount, sizeof(uint32_t));
    	 msync(superBloque, sizeof(int)*2, MS_SYNC);
    	 close(superBlock_file);
    	 free(DIR_SuperBLoque);
    	 log_info(logger, "Sabotaje solucionado");
    	 return;
    }
    else
    	log_info(logger, "El BlockCount del SuperBloque no se vio afectado por el sabotaje.");
    close(superBlock_file);
    free(DIR_Bloques);
    free(DIR_SuperBLoque);

    //liberar sb??
}

void arreglar_Bitmap_superBloque(){
	char* DIR_SuperBLoque=string_new();
	printf("aqui toy 1\n");
	string_append(&DIR_SuperBLoque,obtener_directorio("/superBloque.ims"));
	int superBlock_file = open(DIR_SuperBLoque, O_CREAT | O_RDWR, 0664);
	int bitmap_size = roundUp(blocks_amount,8);
    void* superBloque = mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_SHARED, superBlock_file, 0);
    printf("aqui toy 2\n");
    t_bitarray* bitmap_copy;
    //memcpy(&bitmap_copy, 2*(sizeof(uint32_t))+superBloque,bitmap_size);
    bitmap_copy = bitarray_create_with_mode((char*) superBloque+sizeof(int)+sizeof(int), bitmap_size, MSB_FIRST);
    printf("aqui toy 3\n");
    t_bitarray* bitmap_correcto = bitarray_create_with_mode((char*) superBloque+sizeof(int)+sizeof(int), bitmap_size, MSB_FIRST);
    printf("aqui toy 4\n");
    char*Dir_metadata=obtener_directorio("/Files/Oxigeno.ims");
    calcularBloquesUsadosRecursos(bitmap_correcto,Dir_metadata);
    free(Dir_metadata);
    Dir_metadata=obtener_directorio("/Files/Comida.ims");
    calcularBloquesUsadosRecursos(bitmap_correcto,Dir_metadata);
    free(Dir_metadata);
    Dir_metadata=obtener_directorio("/Files/Basura.ims");
	calcularBloquesUsadosRecursos(bitmap_correcto,Dir_metadata);
	free(Dir_metadata);
	printf("aqui toy 5\n");
	for(int i = 0; i < list_size(lista_tripulantes); i++) {
		printf("aqui toy 6\n");
	        tripulante* trip = (tripulante*)list_get(lista_tripulantes, i);
	        calcularBloquesUsadosRecursos(bitmap_correcto,trip->dir_bitacora);
	    }
	printf("aqui toy 7\n");
	 //imprimir_bitmap(bitmap_correcto);
	bool esElMismo=true;
	for(int i = 0; i<bitmap_size; i++){
		if(bitarray_test_bit(bitmap_copy, i) == bitarray_test_bit(bitmap_correcto, i)){
			esElMismo=false;
			break;
		}

	}
	if(!esElMismo){
		log_info(logger, "Se saboteo el SIZE del archivo de SuperBloque. Solucionando sabotaje...");
		bitmap_copy=bitmap_correcto;
		msync(bitmap_copy->bitarray,bitmap_size,MS_SYNC);
		msync(superBloque, sizeof(uint32_t)*2 + bitmap_size, MS_SYNC);

		close(superBlock_file);
		free(DIR_SuperBLoque);
		log_info(logger, "Sabotaje solucionado");
		return;
	}else
    	log_info(logger, "El Bitmap del SuperBloque no se vio afectado por el sabotaje.");
    close(superBlock_file);
    free(DIR_SuperBLoque);
    //liberar sb??
}
//-------------------------------------------------Sabotajes a FILES-----------------------------------------------------------
void arreglar_blocks_recursos(char* DIR_metadata){
	 t_config* metadata=config_create(DIR_metadata);

	 int cantidad_bloques=config_get_int_value(metadata,"BLOCK COUNT");
	 int size=config_get_int_value(metadata,"SIZE");
	 char* string_caracter_llenado=config_get_string_value(metadata,"CARACTER_LLENADO");
	 char caracter_llenado=string_caracter_llenado[0];
	 char* md5_correcto=config_get_string_value(metadata,"MD5_ARCHIVO");
	 char** bloques=config_get_array_value(metadata,"BLOCKS");

	 char*md5_encontrado=crear_MD5(bloques, 'A',cantidad_bloques);
	 int aux=0;

	 if(md5_encontrado!=md5_correcto){
		 log_info(logger, "Se saboteo el BLOCKS del archivo de recursos. Solucionando sabotaje...");
		 log_info(logger, "Restaurando archivo...");
		 int caracteres_a_agregar=size;

		 for(int i = 0; i<cantidad_bloques && caracteres_a_agregar>0;i++){
			 caracteres_a_agregar = escribir_caracter_en_bloque(caracter_llenado, caracteres_a_agregar, bloques[i], aux);
			 aux=size-caracteres_a_agregar;
		 }
		 log_info(logger, "Archivos restaurados");
		 config_set_value(metadata,"MD5_ARCHIVO",md5_correcto);
		 free(string_caracter_llenado);
		 free(md5_correcto);
		 free(md5_encontrado);
		 liberar_split(bloques);
	 }
	 else
	     log_info(logger, "El Blocks del archivo de Recursos no se vio afectado por el sabotaje.");

}
void arreglar_size_recursos(char* DIR_metadata){
	t_config* metadata=config_create(DIR_metadata);
	 char** bloques=config_get_array_value(metadata,"BLOCKS");
	 int size_encontrado=config_get_int_value(metadata,"SIZE");
	 printf("size encontrado: %d",size_encontrado);
	 int cantidad_bloques=config_get_int_value(metadata,"BLOCK_COUNT");
	 char* string_caracter_llenado=config_get_string_value(metadata,"CARACTER_LLENADO");
	 char caracter_llenado=string_caracter_llenado[0];
	 int size_correcto=(cantidad_bloques-1)*block_size; //el ultimo debo revisar cuantos caracteres tiene escritos}
	 char caracter=caracter_llenado;
	 int bloque_a_vereificar= atoi(bloques[cantidad_bloques-1]);
	 int aux=0;
	 while(caracter==caracter_llenado){
			printf("aqui toy\n");
		 int desplazamiento =bloque_a_vereificar * block_size + aux;
		 memcpy(&caracter, blocks_copy + desplazamiento, sizeof(char));
		 aux++;
	 }//aux--;
	size_correcto+=aux;
	 printf("size encontrado: %d",size_correcto);
	if(size_encontrado != size_correcto){
		 log_info(logger, "Se saboteo el SIZE del archivo de recursos. Solucionando sabotaje...");
		 char* str_size_correcto=string_itoa(size_correcto);
		 config_set_value(metadata,"SIZE",str_size_correcto);
		 config_save(metadata);
		 log_info(logger, "Sabotaje solucionado");
		 config_destroy(metadata);
		 free(str_size_correcto);
	}else
	     log_info(logger, "El Size del archivo de Recursos no se vio afectado por el sabotaje.");
	printf("aqui toy 6\n");


}
void arreglar_blockcount_recursos(char* DIR_metadata){
	t_config* metadata=config_create(DIR_metadata);
	 char** bloques=config_get_array_value(metadata,"BLOCKS");
	 int block_count_encontrado=config_get_int_value(metadata,"BLOCK_COUNT");

	 int bloques_correctos=0;
	 while(bloques[bloques_correctos] != NULL){
		 bloques_correctos++;
	 }
	 if(bloques_correctos != block_count_encontrado){
		 log_info(logger, "Se saboteo la cantidad de bloques del archivo de recursos. Solucionando sabotaje...");
		 char* str_bloques_correctos=string_itoa(bloques_correctos);
		 printf("%s",str_bloques_correctos);
		 config_set_value(metadata,"BLOCK_COUNT",str_bloques_correctos);
		 config_save(metadata);
		 log_info(logger, "Sabotaje solucionado");
		 config_destroy(metadata);
		 free(str_bloques_correctos);
	 }
	 else
	 	 log_info(logger, "El BlockCount del archivo de Recursos no se vio afectado por el sabotaje.");
}*/
