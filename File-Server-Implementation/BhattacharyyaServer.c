//Server socket application

//header files
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENT_ALLOWED_SERVED 5
#define MAX_CHAR_PER_LINE 512

// global data
int server_id;
struct sockaddr_in serv_addr;
int addrlen = sizeof(serv_addr);
int g_client_served = 0;

//thread
void* Connection_handler (void* client_sock);
void service_handler(int);

//main function

int main(int argc, char const *argv[])
{
	pthread_t l_thread_id[MAX_CLIENT_ALLOWED_SERVED];
	if(2 != argc)
	{
		printf("\nImproper argument\n");
		exit(0);
	}

	int port = atoi(argv[1]);
	printf("Port number %d\n", port);
	
    // Creating socket file descriptor
	//AF_INET (IPv4 protocol) , AF_INET6 (IPv6 protocol)
	//SOCK_STREAM: TCP(reliable, connection oriented)      SOCK_DGRAM: UDP(unreliable, connectionless)
	// 0 for IP(Internet Protocol)
    if (0 == (server_id = socket(AF_INET, SOCK_STREAM, 0)))
    {
        perror("socket failed");
        exit(0);
    }
	else
	{
		printf("\nServer running\n");
	}
      
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons( port );
      
    // Attaching socket to the port
    if (0 > bind(server_id, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    {
        perror("bind failed");
		printf("Incorrect port number used to data transmission\n");
        exit(0);
    }
	else
	{
		printf("\nAttached to the Server socket sucessfully\n");
		
		for(int i = 0; i < MAX_CLIENT_ALLOWED_SERVED; i++)
		{
				pthread_create(&l_thread_id[i], NULL, &Connection_handler, NULL);
				pthread_join(l_thread_id[i], NULL);
		}
	}
    return 0;
}

//Thread function
void* Connection_handler(void* client_sock)
{
	int client_id;
	//Listen to the incoming connection
	if (0 > listen(server_id, 3))
	{
		perror("listen error");
		exit(0);
	}

	if (0 > (client_id = accept(server_id, (struct sockaddr *)&serv_addr,(socklen_t*)&addrlen)))
	{
		perror("accept failure");
		exit(0);
	}
	else
	{
		printf("\nConnection established\n");
		service_handler(client_id);
	}
}

//handle file operation
void service_handler(int client_id)
{
	int valread;
	char buffer[512] = {0};
	
    valread = read( client_id , buffer, 512);
    printf("Filename received from client: %s\n",buffer);
	printf("Searching file on server: %s\n",buffer);
	
	FILE *fp = fopen(buffer, "r");
	if(NULL == fp)
	{
		printf("\nFile %s does not exist\n ",buffer);
	}
	else
	{
		printf("\nFile %s found on server\n ",buffer);
		char r_buffer[MAX_CHAR_PER_LINE] = {'\0'};

		while(!feof(fp))
		{
			fgets(r_buffer,MAX_CHAR_PER_LINE,fp);
			send(client_id , r_buffer , strlen(r_buffer) , 0 );
			memset(r_buffer, '\0', MAX_CHAR_PER_LINE);
		}
		printf("\nFile transferred....\nClosing client connection\n");
		fclose(fp);
	}
	close(client_id);
	if(MAX_CLIENT_ALLOWED_SERVED > ++g_client_served)
	{
		printf("\nServer running\n");
	}
	else
	{
		printf("\nMaximum client served\n");
		printf("\nServer shutting down\n");
		close(server_id);
	}
}