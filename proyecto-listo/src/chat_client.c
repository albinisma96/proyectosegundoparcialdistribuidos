#include "../include/common.h"
//

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char nickname[MAX_USERNAME] = {};
void send_msg_handler();
void recv_msg_handler();
void catch_ctrl_c_and_exit(int sig);


//Buffer para request serializado en formato CBOR
//unsigned char request_cbor_b[MAXSIZE];

//ssize_t encode_file_request(request_t req, unsigned char *request_b);

//void process_request(int clientfd, request_t req);


//Manejo básico de señales
void catch_signals(int signum){
	switch(signum){
		case SIGPIPE:
			fprintf(stderr, "Error de conexión: Socket ha sido cerrado remotamente.\n");
			break;
		case SIGABRT:
			fprintf(stderr, "Error fatal: SIGABRT capturado.\n");
			break;
	}
	
	exit(-1);
}

int main(int argc, char **argv)
{	
	signal(SIGINT, catch_ctrl_c_and_exit);
	bool uflag = false;
	bool pflag = false;
	int clientfd,opt;
	char *port, *host,*ip;
	
	//char console_buf[MAXLINE];
	struct stat sbuf = {0};
	chat_client_t req = {0};

	signal(SIGPIPE, catch_signals);
	signal(SIGABRT, catch_signals);

	while ((opt = getopt (argc, argv, "up:h")) != -1){
		switch(opt)
		{
			case 'u':
				uflag = true;
				break;
			case 'p':
				pflag = true;
				break;
			case 'h':
				printf("chat_client connects to a remote chat_server service, allows the user to send messages to the chat group and displays chat messages from other clients.\n\n");
				printf("Usage:\n %s -u <user> -p <password> <ip> <port> \n", argv[0]);
				printf(" %s -h\n", argv[0]);
				printf("Options:\n");
				printf(" -h\t\t\tHelp, show this screen.\n");
				printf(" -u\t\t\tSpecify the username.\n");
				printf(" -p\t\t\tSpecify the password.\n");
				return 0;
			case '?':
			default:
				//fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
				return -1;
		}
	}

	if(uflag){
		host=argv[2];
		//daemonize("server");
	}
	if(pflag){
		ip=argv[5];
		port=argv[6];
//		daemonize("server");
	}

	/*if (argc != 3) {
		fprintf(stderr, "uso: %s <host> <port>\n", argv[0]);
		exit(-1);
	}*/
signal(SIGINT, catch_ctrl_c_and_exit);

    // Naming
   // printf("Please enter your name: ");
    //if (fgets(nickname, MAX_USERNAME, host) != NULL) {
        str_trim_lf(nickname, MAX_USERNAME);
    //}
    if (strlen(nickname) >= MAX_USERNAME-1) {
        printf("\nName must be more than one and less than thirty characters.\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1) {
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
    server_info.sin_addr.s_addr = inet_addr(ip);
    server_info.sin_port = htons(atoi(port));

    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *)&server_info, s_addrlen);
    if (err == -1) {
        printf("Connection to Server error!\n");
        exit(EXIT_FAILURE);
    }
    
    // Names
    getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
    getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

    send(sockfd, nickname, MAX_USERNAME, 0);

    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if(flag) {
            printf("\nBye\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}

/*
 * process_request - Procesa el request req con en la comunicación con el servidor.
 */
/*void process_request(int clientfd, chat_client_t req)
{
	size_t filesize = 0;
	ssize_t length = encode_file_request(req, request_cbor_b);
	
	if(length > 0){
		//Envia tamaño del request
		if(send(clientfd, &length, sizeof(ssize_t), 0) <= 0)
			connection_error(clientfd);

		//Envia request
		if(send(clientfd, request_cbor_b, length, 0) <= 0)
			connection_error(clientfd);

		}
}*/

/*
 * parse_command - Llena el struct req a partir de la consola: console_buf
 *		Formato esperado:
 *		GET <filename>
 *		PUT <filename>
 *		LIST
 *     
 *     Retorna false si hubo error de parsing.  
 */

/*
 * encode_file_request - Serializa el request req al buffer buf
 *		usando formato CBOR.
 *     
 *     Retorna el número de bytes usados en el buffer buf -1 si hubo error.  
 */
    
    
    // Names
    /*getsockname(clientfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
    getpeername(clientfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));
*/
    
void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void recv_msg_handler() {
    char receiveMessage[BLOCK_SIZE] = {};
    while (1) {
        int receive = recv(sockfd, receiveMessage, BLOCK_SIZE, 0);
        if (receive > 0) {
            printf("\r%s\n", receiveMessage);
            str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        } else { 
            // -1 
        }
    }
}

void send_msg_handler() {
    char message[TEXT_SIZE] = {};
    while (1) {
        str_overwrite_stdout();
        while (fgets(message, TEXT_SIZE, stdin) != NULL) {
            str_trim_lf(message, TEXT_SIZE);
            if (strlen(message) == 0) {
                str_overwrite_stdout();
            } else {
                break;
            }
        }
        send(sockfd, message, TEXT_SIZE, 0);
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }
    catch_ctrl_c_and_exit(2);
}


/*ssize_t encode_file_request(request_t req, unsigned char *buf)
{
	ssize_t length = -1;

	if(buf){
		// Crear un CBOR map con espacio para tres parejas key - value 
		cbor_item_t* root = cbor_new_definite_map(5);

		// Agregar parejas key - value a root 
	  	cbor_map_add(root,
	        (struct cbor_pair){
	           .key = cbor_move(cbor_build_uint8(ACTION)),
	           .value = cbor_move(cbor_build_uint8(req.action))});
	  	cbor_map_add(root,
	       (struct cbor_pair){
	           .key = cbor_move(cbor_build_uint8(FILENAME)),
	           .value = cbor_move(cbor_build_string(req.filename))});
	  	cbor_map_add(root,
	       (struct cbor_pair){
	           .key = cbor_move(cbor_build_uint8(SIZE)),
	           .value = cbor_move(cbor_build_uint64(req.size))});


	  	length = cbor_serialize(root, buf, MAXSIZE);
	  	cbor_decref(&root);
	}

	return length;
}*/


