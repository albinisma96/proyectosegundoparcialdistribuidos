#ifndef __COMMON_H__
#define __COMMON_H__

#include "../include/csapp.h"
int open_listenfd(char *port);
//Tamaño fijo de un "bloque" de comunicación
#define BLOCK_SIZE 256
int open_listenfd(char *port);
//Tamaño máximo de un mensaje de texto en el chat
#define TEXT_SIZE 140

//Tamaño máximo de un nombre de usuario
#define MAX_USERNAME 32

//Enumeración de keys para su uso en CBOR
enum FIELD {MESSAGE = 1, USER, TOKEN, DESTINATION, PRESENCE};

//Estructura sugerida para representar un mensaje de chat
typedef struct chat_client {
	int data;
	char message[TEXT_SIZE];
	char user_from[MAX_USERNAME];
	char user_to[MAX_USERNAME];
	_Bool is_presence;
	char name[MAX_USERNAME];
	char token[MAX_USERNAME];
	struct chat_client* prev;
    	struct chat_client* link;
}chat_client_t;

//Estructura sugerida para representar un mensaje de login
/*typedef struct login_message {
	char user[MAX_USERNAME];
	char token[MAX_USERNAME];
}login_message_t;*/
/*typedef struct {
int *buf;
int n;
int front;
int rear;
sem_t mutex;
sem_t slots;
sem_t items;
} sbuf_t;*/

chat_client_t *newNode(int sockfd, char* ip) {
    chat_client_t *users = (chat_client_t *)malloc( sizeof(chat_client_t) );
    users->data = sockfd;
    users->link = NULL;
    users->prev = NULL;
    //strncpy(users->ip, ip, 16);
    strncpy(users->name, "NULL", 5);
    return users;
}
/*void sbuf_init(sbuf_t *sp, int n);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);*/
void str_trim_lf (char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}


#endif /* __COMMON_H__ */
