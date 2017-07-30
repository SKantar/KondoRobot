/*
 * Server hosted at RM Board
 * Version 0.1  2016-05-21 Sladjan Kantar (School of Computing)
 *
 *
 *		main.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "kondo.h"

#define DEFAULT_FILENAME "SpcaPict.jpg"
#define BUFFER_SIZE 256
#define MAX_MESSAGE_LENGTH 256

pthread_mutex_t lock;

/**
 * Function sends file content to client socket
 *
 * @param client_sock - Client Socket
 * @param file_path - full or relative path to file
 * @return 1 if everything is ok, or -1 if we have some problem
 */

int send_file(int client_sock, char *file_path)
{
    char buffer[BUFFER_SIZE];
    long file_len;
    
    system("spcacat -N 1 -f jpg -s 640x480 -g -o");
    
    /** Open File with path 'file_path' **/
    FILE *file = fopen(file_path, "rb");

    /** If we can't open file with specified path **/
    if (!file)
    {
        printf("Unable to open file '%s'", file_path);
        return -1;
    }

    /** Get File Length **/
    fseek(file, 0, SEEK_END);
    file_len = ftell(file);
    fseek(file, 0, SEEK_SET);

    /** Send File Length to Client **/
    write(client_sock , &file_len , sizeof(file_len));

    /** Send File in chunks (Chunk size: 256B) **/
    while (fread(buffer, 1, sizeof(buffer), file) > 0)
    {
        write(client_sock , buffer , sizeof(buffer));
        memset(buffer, 0, BUFFER_SIZE);
    }
    /** Close File **/
    fclose(file);

    return 1;
}


/**
 * Function execute requested command on server side and send response to client socket
 * @param client_sock - Client Socket
 * @param command - Content of command we need to execute
 */
void execute_command(int client_sock, char *command)
{
    /** Lock Function for another Clients **/
    pthread_mutex_lock(&lock);

    /** Execute Command **/
    if(strcmp(command, "GET_PICTURE") == 0)
    {
        /** Take picture, then send content to client **/
        send_file(client_sock, DEFAULT_FILENAME);
    }
    else if(strcmp(command, "FORWARD") == 0)
    {
        /** Make an forward step **/
        step_forward();
    }
    else if(strcmp(command, "BACKWARD") == 0)
    {
        /** Make an backward step **/
        step_backward();
    }
    else if(strcmp(command, "LEFT") == 0)
    {
        /** Turn to the left **/
        rotate_left();
    }
    else if(strcmp(command, "RIGHT") == 0)
    {
        /** Turn to the right **/
        rotate_right();
    }
    else if(strcmp(command, "NEGATIVE") == 0)
    {
        /** Denial of head **/
        denial();
    }
    else if(strcmp(command, "BOW") == 0)
    {
        /** Make one's bow**/
        bow();
    }

    write(client_sock , command , strlen(command));

    /** Unlock Function for another Clients **/
    pthread_mutex_unlock(&lock);
}


/**
 * Function that handle connection for each client
 */
void *connection_handler(void *socket_desc)
{
    /** Get the socket descriptor **/
    int sock = *(int*)socket_desc;

    ssize_t read_size;
    char client_message[MAX_MESSAGE_LENGTH];

    /** Receive a message from client **/
    while( (read_size = recv(sock , client_message , MAX_MESSAGE_LENGTH , 0)) > 0 )
    {
        printf("%s\n", client_message);
        /** execute command **/
        execute_command(sock, client_message);
        memset(client_message, 0, MAX_MESSAGE_LENGTH);
    }

    /** if we can't read any data **/
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    /** Free the socket pointer **/
    free(socket_desc);

    return 0;
}

/**
 * Main Function
 **/
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;

    /** Initialize a Mutex **/
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("Mutex init failed\n");
        return 1;
    }

    /** Create Socket **/
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
        printf("Could not create socket\n");
    puts("Socket created");

    /** Prepare the sockaddr_in structure **/
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    /** Bind **/
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    /** Listen **/
    listen(socket_desc , 3);

    /** Accept and incoming connection **/
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        /** try to create a thread **/
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        puts("Handler assigned");
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    /** Free the mutex **/
    pthread_mutex_destroy(&lock);

    return 0;
}

