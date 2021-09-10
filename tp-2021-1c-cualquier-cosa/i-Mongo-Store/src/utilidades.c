#include "utilidades.h"

/*
char* crear_MD5(char caracter_llenado, int cantidad_caracteres) {
	log_info(logger, "Generando MD5");
	char caracter_string[2] = {caracter_llenado , '\0'};

	char* caracteres = string_new();
	string_append(&caracteres,"echo ");

	for(int i=0; i<cantidad_caracteres; i++)
		string_append(&caracteres,caracter_string);

	string_append(&caracteres, " | md5sum > archivoMD5_");
	string_append(&caracteres,caracter_string);

	int md5_result= system(caracteres);

	free(caracteres);

	if(md5_result >= 0) { //> o >= ?
		char* md5;
		char temp[12]="archivoMD5_";
		strcat(temp,caracter_string);//genero el archivo MD5 para cada recurso
		FILE* file = fopen(temp,"r");

		if(file == NULL)
			return NULL;

		fseek(file, 0, SEEK_END);
		long int size = 32;
		rewind(file);

		md5 = calloc(size + 1, 1);

		fread(md5,1,size,file);
		fclose(file);
		return md5;

	}
	else
		log_info(logger, "Fallo al calcular el MD5");

	return("ERROR DE MD5");
}*/

char* crear_MD5(char* bloques_str, char caracter_llenado, int cantidad_bloques) {
	log_info(logger, "Generando MD5");
    int largo = string_length(bloques_str);

    char temp[largo-2];

    for(int i=1;i<largo-1;i++)
        temp[i-1]=bloques_str[i];

    temp[largo-2]='\0';

    //free(bloques_str);
    bloques_str = string_duplicate(temp);

    log_info(logger, "Bloques_str: %s", bloques_str);

    char** bloques=string_split(bloques_str,",");

	char caracter_string[2] = {caracter_llenado , '\0'};
	char* caracteres = string_new();
	string_append(&caracteres,"echo ");

	for(int i=0;i<cantidad_bloques;i++)
		string_append(&caracteres,bloques[i]);

	string_append(&caracteres, " | md5sum > archivoMD5_");
	string_append(&caracteres, caracter_string);

	int md5_result= system(caracteres);

	free(caracteres);

	if(md5_result >= 0) { //> o >= ?
		char* md5;
		char temp[12]="archivoMD5_";
		strcat(temp,caracter_string);//genero el archivo MD5 para cada recurso
		FILE* file = fopen(temp,"r");

		if(file == NULL)
			return NULL;

		fseek(file, 0, SEEK_END);
		long int size = 32;
		rewind(file);

		md5 = calloc(size + 1, 1);
		//free(caracter_string);
		fread(md5,1,size,file);
		fclose(file);
		return md5;

	}
	else
		log_info(logger, "Fallo al calcular el MD5");

	return "ERROR DE MD5";
}

tripulante* obtener_tripulante(int id_trip, int id_patota) {
	for(int i = 0; i < list_size(lista_tripulantes); i++) {
		tripulante* trip = (tripulante*)list_get(lista_tripulantes, i);
		if(trip->id_trip == id_trip && trip->id_patota == id_patota)
			return trip;
	}

	log_error(logger, "No se encontro a ese tripulante");
	return NULL;
}

int roundUp(int a, int b){
	int R = a/b;
	int r = a % b;

	if(r > 0)
		R++;

	return R;
}

char* juntar_posiciones(int x, int y){
	char* posicion = string_new();
	char* x_str = string_itoa(x);
	char* y_str = string_itoa(y);

	string_append(&posicion, x_str);
	string_append(&posicion, "|");
	string_append(&posicion, y_str);

	free(x_str);
	free(y_str);

	return posicion;
}

void generar_directorio(char* nombre) {
	char *directorio = obtener_directorio(nombre);

	mkdir(directorio,0755);
	log_info(logger, "Se genero el directorio:  %s",directorio);
	free(directorio);
}

void imprimir_bitmap(t_bitarray* bitmap) {
	char* bits = string_new();

	for(int i = 0; i<blocks_amount ;i++){
		if(bitarray_test_bit(bitmap, i))
			string_append(&bits, "1 ");
		else
			string_append(&bits, "0 ");
	}

	log_info(logger, "Bitmap: %s", bits);
	free(bits);
}

char* obtener_directorio(char* nombre) {
	char* DIR_nombre = string_new();

	string_append(&DIR_nombre,punto_montaje);
	string_append(&DIR_nombre,nombre);

	return DIR_nombre;
}

void liberar_split(char** split) {
	int i = 0;

	while(split[i] != NULL) {
		free(split[i]);
		i++;
	}

	free(split);
}

void calcularBloquesUsadosRecursos(t_bitarray*bitmap_copy,char* DIR_metadata){
	FILE* existe = fopen(DIR_metadata,"r");
	printf("aqui toy 4.1\n");
	if(existe != NULL) {
		t_config* metadata=config_create(DIR_metadata);
		char** bloques=config_get_array_value(metadata,"BLOCKS");
		int block_count=config_get_int_value(metadata,"BLOCK_COUNT");
		int bitmap_size = roundUp(blocks_amount,8);
		printf("aqui toy 4.2\n");
		for(int i = 0; i<block_count; i++){
			printf("aqui toy 4.3\n");
			bitarray_set_bit(bitmap_copy, atoi(bloques[i]));
			//msync(bitmap->bitarray, bitmap_size, MS_SYNC);
			printf("aqui toy 4.4\n");
		}
		printf("aqui toy 4.5\n");
		config_destroy(metadata);
		liberar_split(bloques);
		fclose(existe);
	}
}

void calcularBloquesUsadosBitacoras(t_bitarray*bitmap_copy,char* DIR_metadata){
		t_config* metadata=config_create(DIR_metadata);
		char** bloques=config_get_array_value(metadata,"BLOCKS");
		int size=config_get_int_value(metadata,"SIZE");
		int block_count=roundUp(size,block_size);
		int bitmap_size = roundUp(blocks_amount,8);

		for(int i = 0; i<block_count; i++){
			bitarray_set_bit(bitmap_copy, atoi(bloques[i]));
			//msync(bitmap->bitarray, bitmap_size, MS_SYNC);
		}
		config_destroy(metadata);
		liberar_split(bloques);

}
