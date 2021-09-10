#include "blocks.h"

bool crear_superBloque(){
	log_info(logger, "Verificando existencia SuperBloque");

	bool estado_superbloque = false;
	char* DIR_superBloque = obtener_directorio("/superBloque.ims");
	FILE* existe = fopen(DIR_superBloque,"r");
	int bitmap_size = roundUp(blocks_amount,8);

	if(existe != NULL) {
		log_info(logger, "SuperBloque ya existe");
		fclose(existe);
	}
	else
		log_info(logger, "SuperBloque no existe, creandolo.");

	log_info(logger, "el tamaño del bloque es %d y la cant de bloques es %d",block_size,blocks_amount);
	log_info(logger, "el directorio del superBloque es %s",DIR_superBloque);

	int fp = open(DIR_superBloque, O_CREAT | O_RDWR, 0664);

	if(fp == -1)
		log_error(logger, "No se pudo abrir/generar el archivo");
	else {
		ftruncate(fp,sizeof(uint32_t)*2+bitmap_size);

		void* superBloque = mmap(NULL, sizeof(uint32_t)*2 + bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);

		if (superBloque == MAP_FAILED)
			log_error(logger, "Error al mapear el SuperBloque");
		else {
			estado_superbloque = true;
			bitmap = bitarray_create_with_mode((char*) superBloque+sizeof(int)+sizeof(int), bitmap_size, MSB_FIRST);

			void* prueba = malloc(4);

			memcpy(prueba,&block_size,sizeof(uint32_t));
			memcpy(superBloque,prueba,sizeof(uint32_t));
			memcpy(prueba,&blocks_amount,sizeof(uint32_t));
			memcpy(superBloque+sizeof(uint32_t),prueba,sizeof(uint32_t));

			msync(bitmap->bitarray,bitmap_size,MS_SYNC);
			msync(superBloque, sizeof(uint32_t)*2 + bitmap_size, MS_SYNC);

			log_info(logger, "se escribio el archivo");
			log_info(logger, "SuperBloque Generado");

			free(prueba);
		}

		close(fp);
	}
	free(DIR_superBloque);
	return estado_superbloque;
}

bool crear_blocks(){
	log_info(logger, "Verificando existencia Blocks");

	bool estado_bloques = false;
	char* DIR_blocks = obtener_directorio("/Blocks.ims");
	int size = block_size * blocks_amount;
	FILE* existe= fopen(DIR_blocks,"r");

	if(existe != NULL) {
		log_info(logger, "Blocks ya existe");
		fclose(existe);
	} else
		log_info(logger, "Blocks no existe, creandolo");

	int fp = open(DIR_blocks, O_CREAT | O_RDWR, 0666);

	if (fp == -1)
		log_error(logger, "No se pudo abrir/generar el archivo");
	else {
		ftruncate(fp, size);

		blocks = mmap(NULL, size, PROT_READ | PROT_WRITE,MAP_SHARED, fp, 0);

		if(blocks == MAP_FAILED)
			log_error(logger, "Error al mapear Blocks");
		else {
			log_info(logger, "Blocks Generado");
			estado_bloques = true;
		}

		close(fp);
	}

	free(DIR_blocks);
	return estado_bloques;
}

void* uso_blocks() {
	int size = block_size * blocks_amount;
	blocks_copy = malloc(size);

	while(1){
		sleep(config_get_int_value(config, "TIEMPO_SINCRONIZACION"));
		//sleep(9999);

		pthread_mutex_lock(&actualizar_blocks);
			memcpy(blocks,blocks_copy,size);
		pthread_mutex_unlock(&actualizar_blocks);

		msync(blocks,(size), MS_SYNC);
		//log_info(logger, "Se sincronizo el blocks");
	}
	free(blocks_copy);
}

void sumar_caracteres(char caracter_llenado, int cantidad_caracteres) {
	char* DIR_metadata;

	if(caracter_llenado == 'O')
		DIR_metadata = obtener_directorio("/Files/Oxigeno.ims");
	else if(caracter_llenado == 'C')
		DIR_metadata = obtener_directorio("/Files/Comida.ims");
	else
		DIR_metadata = obtener_directorio("/Files/Basura.ims");

	crear_metadata(DIR_metadata, caracter_llenado);

	t_config* metadata = config_create(DIR_metadata);
	free(DIR_metadata);

	int size_original = config_get_int_value(metadata,"SIZE");
	int bloques_max;
	int cantidad_original_bloques = config_get_int_value(metadata,"BLOCK_COUNT");

	char** bloques_anteriores = config_get_array_value(metadata,"BLOCKS");

	int size_final = size_original + cantidad_caracteres;

	if(size_original % block_size != 0) {
		log_info(logger, "Se encontro lugar en el ultimo bloque (sumar)");
		int aux = cantidad_caracteres;
		cantidad_caracteres = escribir_caracter_en_bloque(caracter_llenado, cantidad_caracteres, bloques_anteriores[cantidad_original_bloques-1], size_original);
		size_original += aux-cantidad_caracteres;
	}

	bloques_max = roundUp(size_final,block_size); //con los caracteres nuevos me fijo cuantos bloques ocuparia

	int bloques_faltantes = bloques_max - cantidad_original_bloques;

	if(bloques_faltantes > 0){//si ocupo mas bloques de los que tengo asignados, debo agregar mas bloques
		char* bloques_a_agregar= string_new();

		pthread_mutex_lock(&actualizar_bitmap);
			char* bloque_libre = proximo_bloque_libre();
		pthread_mutex_unlock(&actualizar_bitmap);

		string_append(&bloques_a_agregar,bloque_libre);//agrego el proximo bloque disponible que va a almacenar los caracteres
		cantidad_caracteres = escribir_caracter_en_bloque(caracter_llenado, cantidad_caracteres, bloque_libre, size_original);

		free(bloque_libre);
		bloques_faltantes--;

		for(int i = 0; i < bloques_faltantes; i++) { //si siguen faltando agregar bloques los agrego
			pthread_mutex_lock(&actualizar_bitmap);
				bloque_libre = proximo_bloque_libre();
			pthread_mutex_unlock(&actualizar_bitmap);

			string_append(&bloques_a_agregar,",");
			string_append(&bloques_a_agregar,bloque_libre);

			cantidad_caracteres = escribir_caracter_en_bloque(caracter_llenado, cantidad_caracteres, bloque_libre, size_original);
			free(bloque_libre);
		}
		log_info(logger, "Los bloques a agregar son: %s\n", bloques_a_agregar);

		//agregar los caracteres al bloque
		//junto los bloques viejos con los nuevos y lo guardo en la metadata

		char* bloques_totales = string_new();

		string_append(&bloques_totales, "[");

		if(cantidad_original_bloques > 0) {
			string_append(&bloques_totales,bloques_anteriores[0]);

			for(int i = 1; i < cantidad_original_bloques; i++) {
				string_append(&bloques_totales, ",");
				string_append(&bloques_totales, bloques_anteriores[i]);
			}
		}

		char* size_final_str = string_itoa(size_final);
		config_set_value(metadata, "SIZE", size_final_str);
		liberar_split(bloques_anteriores);
		free(size_final_str);

		if(cantidad_original_bloques == 0)
			string_append(&bloques_totales,bloques_a_agregar);
		else {
			string_append(&bloques_totales,",");
			string_append(&bloques_totales,bloques_a_agregar);
		}

		string_append(&bloques_totales,"]");

		//log_info(logger, "Los bloques totales son: %s",bloques_totales);
		char* bloques_max_str = string_itoa(bloques_max);

		char* MD5_nuevo = crear_MD5(bloques_totales, caracter_llenado, bloques_max);

		config_set_value(metadata,"MD5_ARCHIVO",MD5_nuevo);
		//free(MD5_nuevo);

		config_set_value(metadata,"BLOCKS",bloques_totales);
		config_set_value(metadata,"BLOCK_COUNT",bloques_max_str);//Actualizo la cantidad de bloques
		free(bloques_max_str);

		free(bloques_totales);
		free(bloques_a_agregar);
	}
	else {
		char* size_final_str = string_itoa(size_final);
		config_set_value(metadata,"SIZE",size_final_str);
		free(size_final_str);
	}
	//actualizo el MD5
	//char* MD5_nuevo = crear_MD5(caracter_llenado,size_final);
	//config_set_value(metadata,"MD5_ARCHIVO",MD5_nuevo);

	//free(MD5_nuevo);

	config_save(metadata);
	config_destroy(metadata);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void quitar_caracteres(char caracter_llenado, int cantidad_caracteres){
	char* DIR_metadata;

	if(caracter_llenado=='O')
		DIR_metadata = obtener_directorio("/Files/Oxigeno.ims");
	else if(caracter_llenado=='C')
		DIR_metadata = obtener_directorio("/Files/Comida.ims");
	else {
		DIR_metadata = obtener_directorio("/Files/Basura.ims");

		log_info(logger, "Buscando archivo de basura ya existentes");
		FILE* metadata = fopen(DIR_metadata,"rb");

		if(metadata != NULL){
			log_info(logger, "Archivo existente encontrado");

			t_config* metadata = config_create(DIR_metadata);

			int cantidad_caracteres=config_get_int_value(metadata,"SIZE");
			int cantidad_original_bloques=config_get_int_value(metadata,"BLOCK_COUNT");
			char** bloques_originales=config_get_array_value(metadata,"BLOCKS");
			int temp;
			for(int i=0;i<cantidad_original_bloques;i++){
				temp=atoi(bloques_originales[i]);
				cantidad_caracteres=borrar_caracter_en_bloque(caracter_llenado,cantidad_caracteres,bloques_originales[i],cantidad_caracteres);
				pthread_mutex_lock(&actualizar_bitmap);
					bitarray_clean_bit(bitmap,atoi(bloques_originales[i]));
				pthread_mutex_unlock(&actualizar_bitmap);
			}
			if(cantidad_caracteres>0){
				char* cant_caracteres_str = string_itoa(temp-1);
				borrar_caracter_en_bloque(caracter_llenado,cantidad_caracteres,cant_caracteres_str,config_get_int_value(metadata,"SIZE"));
				free(cant_caracteres_str);
			}

			liberar_split(bloques_originales);
			log_info(logger, "Se tiro la basura (se elimino el archivo)");
			remove(DIR_metadata);
			free(DIR_metadata);
			return;
		}
		else{
			log_info(logger, "No existe el archivo de basura");
			free(DIR_metadata);
			return;
		}

	}
	crear_metadata(DIR_metadata, caracter_llenado);
	t_config* metadata=config_create(DIR_metadata);

	free(DIR_metadata);

	int size_original=config_get_int_value(metadata,"SIZE");
	if((size_original-cantidad_caracteres)<0){
		log_info(logger, "Se intentaron quitar mas caracteres de los existentes");
		cantidad_caracteres=size_original;
	}
	int size_final=MAX(0,(size_original-cantidad_caracteres));
	int bloques_reducidos;
	int cantidad_original_bloques=config_get_int_value(metadata,"BLOCK_COUNT");
	bloques_reducidos=roundUp(size_final,block_size); //con los caracteres nuevos me fijo cuantos bloques ocuparia
	int bloques_sobrantes=cantidad_original_bloques-bloques_reducidos;

	if(bloques_sobrantes>0){//si hay mas bloques de los que necesito, debo sacar bloques}
		char** bloques_originales = config_get_array_value(metadata,"BLOCKS");
		char* bloques_a_dejar = string_new();
		string_append(&bloques_a_dejar,"[");
		log_info(logger, "BLoques reducidos = %d",bloques_reducidos);

		if(bloques_reducidos != 0) {
			for(int i=0;i<bloques_reducidos;i++){
				string_append(&bloques_a_dejar,bloques_originales[i]);
				if(i+1==bloques_reducidos){;
					string_append(&bloques_a_dejar,"]");
				}
				else
					string_append(&bloques_a_dejar,",");
			}
		}else
			string_append(&bloques_a_dejar,"]");

		int temp;
		for(int i=bloques_reducidos;i<cantidad_original_bloques;i++){
			temp=atoi(bloques_originales[i]);
			int aux=cantidad_caracteres;
			cantidad_caracteres=borrar_caracter_en_bloque(caracter_llenado,cantidad_caracteres,bloques_originales[i],size_original);
			size_original-=aux-cantidad_caracteres;
			pthread_mutex_lock(&actualizar_bitmap);
				bitarray_clean_bit(bitmap,atoi(bloques_originales[i]));
			pthread_mutex_unlock(&actualizar_bitmap);
		}
		if(cantidad_caracteres>0){
			char* cant_caracteres_str = string_itoa(temp-1);
			borrar_caracter_en_bloque(caracter_llenado,cantidad_caracteres,string_itoa(temp-1),size_original);
			free(cant_caracteres_str);
		}
		log_info(logger, "los bloques a dejar son %s",bloques_a_dejar);
				//agregar los caracteres al bloque

		char* MD5_nuevo=crear_MD5(bloques_a_dejar, caracter_llenado, bloques_reducidos);
		config_set_value(metadata,"MD5_ARCHIVO",MD5_nuevo);

		liberar_split(bloques_originales);
		config_set_value(metadata,"BLOCK_COUNT",string_itoa(bloques_reducidos));//Actualizo la cantidad de bloques
		config_set_value(metadata,"BLOCKS",bloques_a_dejar);
		free(bloques_a_dejar);
	}
	char* size_final_str = string_itoa(size_final);
	config_set_value(metadata,"SIZE",size_final_str);
	free(size_final_str);
	//char* MD5_nuevo=crear_MD5(caracter_llenado,size_final);
	//config_set_value(metadata,"MD5_ARCHIVO",MD5_nuevo);
	//free(MD5_nuevo);
	config_save(metadata);
	config_destroy(metadata);

}

char* proximo_bloque_libre(){// busca el prox libre y lo pone en 1
	int bitmap_size = blocks_amount / 8;

	for(int i = 0; i < blocks_amount; i++) {
		if(!bitarray_test_bit(bitmap, i)){
			bitarray_set_bit(bitmap, i);
			msync(bitmap->bitarray, bitmap_size, MS_SYNC);
			char* bloqueLibre = string_itoa(i);
			return bloqueLibre;
		}
	}

	log_info(logger, "Todos los bloques estan ocupados");
	return NULL;
}

int escribir_caracter_en_bloque(char caracter_llenado, int cantidad_caracteres, char* bloque_libre, int size){
	int bloqueALlenar = atoi(bloque_libre);
	int caract_escritos = 0;

	for(int caracteresEnBloque = size % block_size; caracteresEnBloque < block_size && cantidad_caracteres > 0; caracteresEnBloque++) {
		int desplazamiento = (bloqueALlenar * block_size) + caracteresEnBloque;
		char* agregar = string_from_format("%c",caracter_llenado);

		pthread_mutex_lock(&actualizar_blocks);
			memcpy(blocks_copy+desplazamiento, agregar, sizeof(char));
		pthread_mutex_unlock(&actualizar_blocks);

		free(agregar);

		caract_escritos++;
		cantidad_caracteres--;
	}

	//printf("se escribieron %d\'%c\'\n",caract_escritos,caracter_llenado);

	if(cantidad_caracteres==0)
		log_info(logger, "Se completo la escritura de caracteres");

	return cantidad_caracteres;
}

int borrar_caracter_en_bloque(char caracter_llenado,int cantidad_caracteres,char* bloque_libre,int size){
	int bloqueALlenar=atoi(bloque_libre);
	int caracteresEnBloque;
	int caract_borrados=0;
	if(size%block_size==0){
		caracteresEnBloque=8;
	}
	else
		caracteresEnBloque=size%block_size;

	while(caracteresEnBloque>0 && cantidad_caracteres>0){
				int desplazamiento=bloqueALlenar*block_size+caracteresEnBloque;
		pthread_mutex_lock(&actualizar_blocks);
				memcpy(blocks_copy+desplazamiento,"\0",sizeof(char));
		pthread_mutex_unlock(&actualizar_blocks);
				caract_borrados++;
				cantidad_caracteres--;
		caracteresEnBloque--;
	}
	printf("se borraron %d\'%c\'\n",caract_borrados,caracter_llenado);
	if(cantidad_caracteres==0){
		log_info(logger, "Se completo el borrado de caracteres");
	}
	else
		log_info(logger, "Bloque vacio");
	return cantidad_caracteres;
}

