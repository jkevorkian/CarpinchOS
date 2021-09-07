/*
 *  sockets.h
 *
 *  Created on: 21 jun. 2021
 *  Author: cualquier-cosa
 */

#ifndef _SOCKETS_H_
#define _SOCKETS_H_

#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>   // sprintf
#include <arpa/inet.h>
#include<commons/log.h>
#include<commons/collections/list.h>

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

#endif /* _SOCKETS_H_ */
