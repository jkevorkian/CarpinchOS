/*
 *  sockets.h
 *
 *  Created on: 21 jun. 2021
 *  Author: cualquier-cosa
 */

#ifndef _SOCKETS_H_
#define _SOCKETS_H_

#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>   // sprintf
#include <arpa/inet.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdarg.h>

typedef enum {
	INIT_P,		SND_PO,		ER_SOC,		ER_RCV,		MEM_ALLOC,
	MEM_FREE,	MEM_READ,	MEM_WRITE
} protocolo_msj;

typedef struct {
	int tamanio;
	void* contenido;
} t_buffer;

typedef struct {
	protocolo_msj op_code;
	t_buffer* buffer;
} t_mensaje;

/**
* @NAME: crear_conexion_cliente
* @DESC: Crea un socket cliente que se conectará a un socket servidor que se ubicará en la ip y puerto definidos por parámetro.
* @OUTP: Devuelve el socket del cliente
*/
int crear_conexion_cliente(char* ip, char* puerto);

/**
* @NAME: crear_conexion_servidor
* @DESC: Crea un servicio activo en el puerto e ip definidas por parámetro
* @OUTP: Devuelve el socket del servicio
* @NOTA: Si el puerto es el 0, se generará un puerto aleatorio libre
*/
int crear_conexion_servidor(char *ip, int puerto, int cola_escucha);

/**
* @NAME: esperar_cliente
* @DESC: Toma el primer socket que esté en la cola de escucha y establece la conexión
* @OUTP: Devuelve el socket del ciente que se conectó
*/
int esperar_cliente(int socket);

/**
* @NAME: puerto_desde_socket
* @DESC: Toma un socket y devuelve el puerto en el que está activo
* @OUTP: Devuelve un puerto
*/
int puerto_desde_socket(int socket);

/**
* @NAME: validar_socket
* @DESC: Recibe la salida de un crear_conexion_* e informa si hubo errores
* @OUTP: Devuelve true si el socket es válido
*/
bool validar_socket(int socket, t_log* logger);

/**
* @NAME: data_socket
* @DESC: Muestra en pantalla la información del socket recibido por parámetro
*/
void data_socket(int socket, t_log* logger);

// Funciones para manejo de mensajes
/**
* @NAME: crear_mensaje
* @DESC: Crea una estructura t_mensaje con un protocolo definido por el parametro
* @OUTP: Devuelve un puntero a la estructura t_mensaje creada
*/
t_mensaje* crear_mensaje(protocolo_msj cod_op);

/**
* @NAME: agregar_parametro_a_mensaje
* @DESC: Permite agregar una serie de parametros a un mensaje
* @NOTA: Se debe explicitar qué parametros van a pasarse
*/
void agregar_a_mensaje(t_mensaje* mensaje, char* formato, ...);

/**
* @NAME: enviar_mensaje
* @DESC: Envía un mensaje a través de un socket
*/
void enviar_mensaje(int socket, t_mensaje* mensaje);

/**
* @NAME: recibir_mensaje
* @DESC: Espera un mensaje desde un socket y cuando lo recibe devuelve una lista con los
* 		parámetros del mismo.
* @OUTP: Devuelve una lista de parametros que corresponden con el protocolo y los parametros
* 		definidos por el emisor antes de que haya enviado el mensaje.
* @NOTA: Si no hay conexion, CREO que devuelve NULL
*/
t_list* recibir_mensaje(int socket);

/**
* @NAME: liberar_mensaje_out
* @DESC: Libera la memoria correspondiente a un mensaje de salida
*/
void liberar_mensaje_out(t_mensaje* mensaje);

/**
* @NAME: liberar_mensaje_in
* @DESC: Libera la memoria correspondiente a un mensaje de entrada
*/
void liberar_mensaje_in(t_list* mensaje);

/**
* @NAME: validar_mensaje
* @DESC: Recibe la salida de un recibir_mensaje e informa si hubo errores
* @OUTP: Devuelve true si el mensaje es válido
*/
bool validar_mensaje(t_list* mensaje_in, void* logger);

#endif /* _SOCKETS_H_ */
