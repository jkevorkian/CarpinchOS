#include "mateLib.h"


char* leer_consola() {
	char* buffer = readline(">");
	if(buffer)
		add_history(buffer);

	return buffer;
}

typedef struct mate_struct{

}mate_struct;

int mate_init(mate_instance* lib_ref,char* config){
	lib_ref->group_info = malloc(sizeof(mate_struct));
	config = config_create(config);
	if(lib_ref->group_info == NULL){
		return -1;
	}else return 0;
}

int mate_close(mate_instance* lib_ref)
{
  free(lib_ref->group_info);
  return 0;
}

int main() {
	t_log *logger = log_create("mateLib.log", "MATELIB", 1, LOG_LEVEL_INFO);

	while(1) {
		char* buffer_consola = leer_consola();
		t_list *instancias = list_create();
		int id_inst = 0;

		if(!strcasecmp(buffer_consola, "0" )) {
			int socket_auxiliar = crear_conexion_cliente(ip_kernel, puerto_kernel); //son un define en el mateLib.h

			if(validar_socket(socket_auxiliar, logger)) {
				log_info(logger, "Socket auxiliar funcionando");

				t_list *mensaje_in = recibir_mensaje(socket_auxiliar);

				if((int)list_get(mensaje_in, 0) == SEND_PORT) {
					log_info(logger, "Puerto recibido");

					char puerto[7];
					sprintf(puerto, "%d", (int)list_get(mensaje_in, 1));

					mateLib *inst = malloc(sizeof(mateLib));
					inst->socket = crear_conexion_cliente(ip_kernel, puerto);
					inst->id = id_inst;
					list_add(instancias, inst);
					id_inst++;
				} else
					log_error(logger, "Error en la comunicacion");

				close(socket_auxiliar);
			}
		} else {
			char** input = string_split(buffer_consola, " ");
			mateLib* inst = (mateLib*)list_get(instancias, atoi(input[0])-1);

			log_info(logger, "Carpincho %d obtenido", atoi(input[0]));

			char* listaDeStrings[]={"MEM_ALLOC", "MEM_FREE", "MEM_READ", "MEM_WRITE", "SEM_INIT", "SEM_WAIT", "SEM_POST", "SEM_DESTROY", "CALL_IO", "MATE_CLOSE"};
			int valor;

			for(int i=0;i<10;i++){
				if(!strcasecmp(input[1],listaDeStrings[i])) {
					valor = i;
					break;
				}
			}

			log_info(logger, "mensaje a enviar %s (%d)", input[1], valor);

			t_mensaje* mensaje_out;

			switch(valor) {
				case 0:
					mensaje_out = crear_mensaje(MEM_ALLOC);
				case 1:
					if(valor == 1)
						mensaje_out = crear_mensaje(MEM_FREE);
				case 2:
					if(valor == 2)
						mensaje_out = crear_mensaje(MEM_READ);
				case 3:
					if(valor == 3)
						mensaje_out = crear_mensaje(MEM_WRITE);

					agregar_a_mensaje(mensaje_out, "%d", atoi(input[2]));
					enviar_mensaje(inst->socket, mensaje_out);

					liberar_mensaje_out(mensaje_out);

					t_list* mensaje_mateLib = recibir_mensaje(inst->socket);

					if((int)list_get(mensaje_mateLib, 0) == DATA)
						log_info(logger, "Mensaje Recibido DATA | %d", (int)list_get(mensaje_mateLib, 1));
					else
						log_info(logger, "Mensaje Recibido %d", (int)list_get(mensaje_mateLib, 0));

					liberar_mensaje_in(mensaje_mateLib);
					break;
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					break;
			}
		}
	}

	return 0;
}

/*
void quitar(tripulante* trip, t_list* list) {
	int index = 0;
	bool continuar = true;

	while(continuar) {
		tripulante* nuevo_tripulante = (tripulante*)list_get(list, index);

		if(nuevo_tripulante->id_trip == trip->id_trip && nuevo_tripulante->id_patota == trip->id_patota) {
			continuar = false;
			list_remove(list, index);
		}
		index++;
	}
}

char* listaDeStrings[]={"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
		int valor;

		for(int i=0;i<10;i++){
			if(!strcasecmp(buffer_consola,listaDeStrings[i])) {
				valor = i;
				break;
			}
		}
*/
