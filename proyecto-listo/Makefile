SRC_DIR = src
INCLUDE_DIR = include
LIB_DIR = lib
OBJ_DIR = obj

#Es mandatorio usar este CFLAG para evitar el uso de VLAs en el proyecto
CFLAGS = -Werror=vla -c

DEPS =  $(INCLUDE_DIR)/csapp.h $(INCLUDE_DIR)/common.h  
LIB = -lcbor -lpthread

.PHONY: all
all: chat_server chat_client

chat_server:  $(OBJ_DIR)/chat_server.o
	gcc -o chat_server $(OBJ_DIR)/chat_server.o $(LIB)
	
chat_client: $(OBJ_DIR)/chat_client.o
	gcc -o chat_client $(OBJ_DIR)/chat_client.o $(LIB)

$(OBJ_DIR)/chat_server.o: $(DEPS) $(SRC_DIR)/chat_server.c 
	gcc $(CFLAGS) $(SRC_DIR)/chat_server.c -o $@

$(OBJ_DIR)/chat_client.o: $(DEPS) $(SRC_DIR)/chat_client.c
	gcc $(CFLAGS) $(SRC_DIR)/chat_client.c -o $@

.PHONY: clean
clean:
	rm chat_client chat_server $(OBJ_DIR)/*.o
