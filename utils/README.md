# Utils TP AprobadOS

## Introducción
Este directorio contiene una librería de funciones utiles para el desarrollo de cualquiera de los módulos del TP CarpinchOS.
Para poder usar la librería correctamente, se tendrá que instalar en el directorio correspondiente a las librerías de C del sistema. Esto no es un problema ya que de eso se encargar el archivo makefile.
Para hacer uso del makefile, hay que utilizar la función make en la terminal. Si uno se encuentra en el directorio de las utils/, simplemente la linea "make" funciona; si uno no se encuentra allí, puede utilizar en cambio "make -C ubicación".

Una vez instalada, se puede hacer uso de las funciones si se las incluye en la cabecera del archivo ".c" correspondiente y, además, se vincula la misma al momento de realizar la generación de los ejecutables. Esto último tampoco es un problema: de eso se encarga el makefile de cada programa.

## Comunicación entre procesos (IPC)
### Establecer conexión
Las funciones principales se refieren a la comunicación entre procesos a través del uso de "sockets". Para su uso, se detallan las siguientes funciones:
- crear_conexion_servidor(). Crea un servicio activo en el puerto e ip definidas por parámetro.
- esperar_cliente(). Toma el primer socket que esté en la cola de escucha y establece la conexión.
- crear_conexion_cliente(). Crea un socket cliente que se conectará a un socket servidor que se ubicará en la ip y puerto definidos por parámetro.
Para establecer la comunicación, solo se utilizan las funciones ya mencionadas.
El orden en que se nombraron corresponde con el orden en que deben ser llamadas.
En el servidor, primero se creará el socket (crear_conexion_cliente) y, luego se pondrá al proceso en estado bloqueado esperando que aparezca un cliente. Cuando se comunique el cliente (crear_conexion_cliente), ambos procesos estarán habilitados para comunicarse con los mensajes que se explicarán más adelante.

Nota: para verificar que las funciones de crear_conexion hayan sido exitosas, se puede hacer uso de la función validar_socket().

### Enviar y recibir mensajes
Para enviar mensajes y recibir mensajes, es necesario establecer un protocolo. Este protocolo se define con un entero que lo identifica y una serie de parametros que completan el mensaje.
El número de protocolo se determina a partir de un enum. Por otro lado, los campos consecuentes se describen a partir de una cadena de caracteres.
Por ejemplo, un protocolo puede tener el id 3 y tener la descripción "%d%d%ss".
La subcadena "%d" significa que el campo es un entero de 32 bits; "%s", que es un string, y "%ss" que es una cantidad de strings definidos por el entero anterior.
Las funciones que se utilizan para enviar mensajes son:
- crear_mensaje. Crea una estructura t_mensaje con un protocolo definido por parametro.
- agregar_a_mensaje. Permite agregar parametros a un mensaje. No necesariamente se deben agregar todos juntos.
- enviar_mensaje. Envía un mensaje a través de un socket.

Por otro lado, para recibir mensajes se utiliza:
- recibir_mensaje. Espera un mensaje desde un socket y devuelve una lista con los parámetros de este.