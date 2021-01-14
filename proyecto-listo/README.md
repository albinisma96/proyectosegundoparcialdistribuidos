# Proyecto: Chat encriptado
Aplicación cliente - servidor grupo de chat encriptado usando serialización CBOR y encriptación AES256.

## Uso
El programa *chat_servidor* tiene el siguiente uso:
```
./chat_server –h
chat_server distributes encrypted chat messages between connected clients.

Usage:
  chat_server [-d] <port>
  chat_server -h

Options:
  -h             Help, show this screen.
  -d             Daemon mode.
```
Ejemplo, para escuchar en el puerto 8080 en modo daemon:
```
./chat_server –d 8080
```

El programa *chat_cliente* tiene el siguiente uso:
```
./chat_client –h
chat_client connects to a remote chat_server service, allows the user to send messages to the chat group and displays chat messages from other clients.

Usage:
  chat_client –u <user> -p <password> <ip> <port>
  chat_client -h

Options:
  -h             Help, show this screen.
  -u             Specify the username.
  -p             Specify the password.
```
Ejemplo, para conectarse a un servicio de chat en la IP 192.168.100 en el puerto 8080 con usuario *maria* y clave *secret*:
```
./chat_client -u maria -p secret 192.168.100 8080
```
## Archivos esqueleto
Se proveen tres archivos plantilla/esqueleto para empezar el proyecto:
* **include/common.h**: Contiene deficiones globales y estructuras comunes en todo el proyecto.
* **Makefile**: Contiene esqueleto básico del Makefile a usar.
* **README.md**: Este archivo, modificarlo y agregar información descriptiva del proyecto (modo de uso, compilación, dependencias, etc.)
