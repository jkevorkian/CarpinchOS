#include "hilos_cpu.h"

void iniciar_hilos_cpu() {
	hilos_cpu = list_create();

	for(int i=0; i<grado_multiprocesamiento; i++) {
		pthread_t *hilo_cpu = malloc(sizeof(pthread_t));
		pthread_create(hilo_cpu, NULL, cpu, NULL);
		list_add(hilos_cpu, hilo_cpu);
	}
}

void* cpu() {
	while(1) {
		sem_wait(&carpinchos_running);
		carpincho* carp = quitar_running();

		char *tiempo_inicio = temporal_get_string_time("%H:%M:%S:%MS"); // "12:51:59:331"

		bool seguir = true;

		//se ejecutan las tareas para cada carpincho

		while(seguir) {
			t_list* mensaje_in = recibir_mensaje(carp->socket_mateLib);

			if (!validar_mensaje(mensaje_in, logger)) {
				log_error(logger, "Carpincho desconectado");
				seguir = false;
			} else {
				t_mensaje* mensaje_out;
				switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
					case MEM_ALLOC:
					case MEM_FREE:
					case MEM_READ:
					case MEM_WRITE:
						mensaje_out = crear_mensaje((int)list_get(mensaje_in, 0)); //creo el mismo mensaje que recibi
						agregar_a_mensaje(mensaje_out, "%d", (int)list_get(mensaje_in, 1)); //le agrego el parametro recibido
						enviar_mensaje(carp->socket_memoria, mensaje_out);

						liberar_mensaje_out(mensaje_out);

						t_list* mensaje_mateLib = recibir_mensaje(carp->socket_memoria); //espero la respuesta de la ram

						mensaje_out = crear_mensaje((int)list_get(mensaje_mateLib, 0)); //tal cual llega genero el mismo mensaje para enviarselo a mateLib
						if((int)list_get(mensaje_mateLib, 0) == DATA) 					//si es un MEM_READ la memoria me devuelve el mensaje DATA con un parametro, asi que en ese caso tengo que agregarlo
							agregar_a_mensaje(mensaje_out, "%s", (int)list_get(mensaje_mateLib, 1));
						enviar_mensaje(carp->socket_mateLib, mensaje_out);

						liberar_mensaje_out(mensaje_out);
						liberar_mensaje_in(mensaje_mateLib);
						break;
					case SEM_INIT:
						break;
					case SEM_WAIT:
						break;
					case SEM_POST:
						break;
					case SEM_DESTROY:
						break;
					case CALL_IO:
						break;
					case MATE_CLOSE:
						break;
				}
				liberar_mensaje_in(mensaje_in);
			}
		}
		char *tiempo_fin = temporal_get_string_time("%H:%M:%S:%MS");

		carp->rafaga_real_anterior = obtener_rafaga_real(tiempo_inicio, tiempo_fin);
		carp->estimacion_proxima_rafaga = obtener_estimacion_proxima_rafaga(carp->rafaga_real_anterior, carp->estimacion_proxima_rafaga);
	}
}

int obtener_rafaga_real(char *tiempo_i, char *tiempo_f) { //El tiempo tiene el formato "12:51:59:331" Hora:Minuto:Segundo:MiliSegundo
	char** tiempo_inicio = string_split(tiempo_i, ":");
	char** tiempo_fin = string_split(tiempo_f, ":");
	int tiempo_rafaga[4];

	for(int i=0; i<4; i++)
		tiempo_rafaga[i] = atoi(tiempo_fin[i]) - atoi(tiempo_inicio[i]);

	liberar_split(tiempo_inicio);
	liberar_split(tiempo_fin);

	return (((tiempo_rafaga[0]*60 + tiempo_rafaga[1])*60 + tiempo_rafaga[2]))*1000 + tiempo_rafaga[3];
}

double obtener_estimacion_proxima_rafaga(int rafaga_real, int estimacion) {
	return alfa*rafaga_real + (1-alfa)*estimacion;
}

void liberar_split(char** split) {
	int i = 0;

	while(split[i] != NULL) {
		free(split[i]);
		i++;
	}

	free(split);
}



