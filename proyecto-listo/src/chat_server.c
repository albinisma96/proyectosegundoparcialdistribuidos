#include "../include/common.h"
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#define SBUFSIZE 16

void service_client(int connfd, chat_client_t *np);

// Global variables
int server_sockfd = 0, client_sockfd = 0;
chat_client_t *root, *now;

/*bool decode_file_request(unsigned char *buf, ssize_t length, chat_client_t *req);*/
void client_handler(void *p_client);
void daemonize(const char *cmd);
void send_to_all_clients(chat_client_t *np, char tmp_buffer[]);
const char DIRECTORIO[] = "archivos/";
chat_client_t *root, *now;

sbuf_t sbuf;
//int open_listenfd(char *port);
void catch_ctrl_c_and_exit(int sig);

int main(int argc, char **argv)
{
	
	bool dflag = false;  // Opcion para activar el daemon
	socklen_t clientlen=sizeof(struct sockaddr_in);
	int listenfd, connfd, opt;
	//unsigned int clientlen;
	struct sockaddr_in clientaddr;
	struct hostent *hp;
	char *haddrp, *port;
	struct stat st = {0};
	pthread_t tid;

	while ((opt = getopt (argc, argv, "d:h")) != -1){
		switch(opt)
		{
			case 'd':
				dflag = true;
				break;
		
			case 'h':
				printf("chat_server distributes encrypted chat messages between connected clients.\n\n");
				printf("Usage:\n %s [-d] <port> \n", argv[0]);
				printf(" %s -h\n", argv[0]);
				printf("Options:\n");
				printf(" -h\t\t\tHelp, show this screen.\n");
				printf(" -d\t\t\tDaemon mode.\n");
				
				return 0;
			case '?':
			default:
				//fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
				return -1;
		}
	}
	
	
	//Ignora SIGPIPE, la cual es enviada durante una desconección remota
	signal(SIGPIPE, SIG_IGN);
	
	if(dflag){
		port=argv[2];
		daemonize("server");
	}
	else if(!dflag){
	port=argv[1];
	}

	/*if (argc != 2) {
		fprintf(stderr, "uso: %s <port>\n", argv[0]);
		exit(-1);
	}*/
	//port = argv[1];
	sbuf_init(&sbuf,SBUFSIZE);

	signal(SIGINT, catch_ctrl_c_and_exit);

    // Create socket
    server_sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (server_sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }

    // Socket information
    struct sockaddr_in server_info, client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(atoi(port));

    // Bind and Listen
    bind(server_sockfd, (struct sockaddr *)&server_info, s_addrlen);
    listen(server_sockfd, 5);

    // Print Server IP
    getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Start Server on: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));

    // Initial linked list for clients
    root = newNode(server_sockfd, inet_ntoa(server_info.sin_addr));
    now = root;

    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);

        // Print Client IP
        getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        printf("Client %s:%d come in.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

        // Append linked list for clients
        chat_client_t *c = newNode(client_sockfd, inet_ntoa(client_info.sin_addr));
        c->prev = now;
        now->link = c;
        now = c;

        pthread_t id;
        if (pthread_create(&id, NULL, (void *)client_handler, (void *)c) != 0) {
            perror("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
	
	
}

void catch_ctrl_c_and_exit(int sig) {
    chat_client_t *tmp;
    while (root != NULL) {
        printf("\nClose socketfd: %d\n", root->data);
        close(root->data); // close all socket include server_sockfd
        tmp = root;
        root = root->link;
        free(tmp);
    }
    printf("Bye\n");
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(chat_client_t *np, char tmp_buffer[]) {
    chat_client_t *tmp = root->link;
    while (tmp != NULL) {
        if (np->data != tmp->data) { // all clients except itself.
            printf("Send to sockfd %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, BLOCK_SIZE, 0);
        }
        tmp = tmp->link;
    }
}

void client_handler(void *p_client) {
    int leave_flag = 0;
    char nickname[MAX_USERNAME] = {};
    char recv_buffer[TEXT_SIZE] = {};
    char send_buffer[BLOCK_SIZE] = {};
    chat_client_t *np = (chat_client_t *)p_client;

    // Naming
    if (recv(np->data, nickname, MAX_USERNAME, 0) <= 0 || strlen(nickname) >= MAX_USERNAME-1) {
        printf("didn't input name.\n");
        leave_flag = 1;
    } else {
        strncpy(np->name, nickname, MAX_USERNAME);
        printf("%s(%d) join the chatroom.\n", np->name, np->data);
        sprintf(send_buffer, "%s join the chatroom.", np->name);
        send_to_all_clients(np, send_buffer);
    }

    // Conversation
    while (1) {
        if (leave_flag) {
            break;
        }
        int receive = recv(np->data, recv_buffer, TEXT_SIZE, 0);
        if (receive > 0) {
            if (strlen(recv_buffer) == 0) {
                continue;
            }
            sprintf(send_buffer, "%s：%s ", np->name, recv_buffer);
        } else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
            printf("%s(%d) leave the chatroom.\n", np->name, np->data);
            sprintf(send_buffer, "%s leave the chatroom.", np->name);
            leave_flag = 1;
        } else {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        send_to_all_clients(np, send_buffer);
    }

    // Remove Node
    close(np->data);
    if (np == now) { // remove an edge node
        now = np->prev;
        now->link = NULL;
    } else { // remove a middle node
        np->prev->link = np->link;
        np->link->prev = np->prev;
    }
    free(np);
}

void daemonize(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    /*
     * Clear file creation mask.
     */
    umask(0);

    /*
     * Get maximum number of file descriptors.
     */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		printf("%s: can't get file limit", cmd);
        //err_quit("%s: can't get file limit", cmd);

    /*
     * Become a session leader to lose controlling TTY.
     */
    if ((pid = fork()) < 0)
	
        printf("%s: can't fork", cmd);
    else if (pid != 0) /* parent */
        exit(0);
    setsid();

    /*
     * Close all open file descriptors.
     */
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++)
        close(i);

    /*
     * Attach file descriptors 0, 1, and 2 to /dev/null.
     */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    /*
     * Initialize the log file.
     */
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
          fd0, fd1, fd2);
        exit(1);
    }
}

//Funcion que ayuda a crear hilos para los clientes

/*
 * decode_file_request - Desempaca el contenido CBOR en recv_buf y lo almacena en req.
 *		TEXT_SIZE es el número de bytes a usar en recv_buf.
 *     
 *     Retorna false si hubo error.  
 */
/*bool decode_file_request(unsigned char *buf, ssize_t length, chat_client_t *req)
{
	cbor_item_t* item = NULL;
	struct cbor_load_result result;
	struct cbor_pair* pair;
	bool success = false;
	ssize_t str_size,n;

	item = cbor_load(buf, length, &result);

	if(item){
		if (result.error.code == CBOR_ERR_NONE)
			if(cbor_isa_map(item)){
				if((n = cbor_map_size(item)) <= 0)
					return false;

				pair = cbor_map_handle(item);
				memset(req, 0, sizeof(chat_client_t));
				for(int i=0; i < n; i++){
					if(cbor_isa_uint(pair[i].key)) {
						enum FIELD key = *(pair[i].key->data);
						switch(key) {
							case USER:
								if(!cbor_isa_uint(pair[i].value)) return false;
								req->action = *(pair[i].value->data);
								break;
							case MESSAGE:
								if(!cbor_isa_string(pair[i].value)) return false;
								str_size = pair[i].value->metadata.string_metadata.length;
								memcpy(req->filename, pair[i].value->data, str_size);
								break;
							case SIZE:
								if(!cbor_isa_uint(pair[i].value)) return false;
								memcpy(&req->size, pair[i].value->data, sizeof(size_t));
								break;
							default:
								fprintf(stderr,"Unknown field %d, ignoring.\n",key);
						}
					}else
						return false;
				}

				success = true;
			}
		// Deallocate the result 
		cbor_decref(&item);
	}

	return success;
}*/

void sbuf_init(sbuf_t *sp, int n)
{
	sp->buf = calloc(n, sizeof(int));
	sp->n = n;
	/* Buffer holds max of n items */
	sp->front = sp->rear = 0;
	/* Empty buffer iff front == rear */
	sem_init(&sp->mutex, 0, 1);
	/* Binary semaphore for locking */
	sem_init(&sp->slots, 0, n);
	/* Initially, buf has n empty slots */
	sem_init(&sp->items, 0, 0);
	/* Initially, buf has zero data items */
	}

/* Clean up buffer sp */
void sbuf_deinit(sbuf_t *sp)
{
	free(sp->buf);
}

/* Insert item onto the rear of shared buffer sp */
void sbuf_insert(sbuf_t *sp, int item)
{
	sem_wait(&sp->slots);
	/* Wait for available slot */
	sem_wait(&sp->mutex);
	/* Lock the buffer */
	sp->buf[(++sp->rear)%(sp->n)] = item;
	/* Insert the item */
	sem_post(&sp->mutex);
	/* Unlock the buffer */
	sem_post(&sp->items);
	/* Announce available item */
}

/* Remove and return the first item from buffer sp */
int sbuf_remove(sbuf_t *sp)
{
	int item;
	sem_wait(&sp->items);
	/* Wait for available item */
	sem_wait(&sp->mutex);
	/* Lock the buffer */
	item = sp->buf[(++sp->front)%(sp->n)]; /* Remove the item */
	sem_post(&sp->mutex);
	/* Unlock the buffer */
	sem_post(&sp->slots);
	/* Announce available slot */
	return item;
}
