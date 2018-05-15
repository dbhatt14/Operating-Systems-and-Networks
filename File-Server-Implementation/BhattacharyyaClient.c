// Client Socket application
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CHAR_PER_LINE 512

int main(int argc, char const *argv[])
{
    struct sockaddr_in client_addr;
    int socket_id = 0;
	int valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[MAX_CHAR_PER_LINE] = {'\0'};
	
	if(2 != argc)
	{
		printf("\nImproper argument\n");
		exit(0);
	}
	
	int port = atoi(argv[1]);
	printf("Port number %d\n", port);
	
    if (0 > (socket_id = socket(AF_INET, SOCK_STREAM, 0)))
    {
        perror("\n Socket creation error \n");
        exit(0);
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
      
    // Convert IPv4 and IPv6 addresses
    if(0 >= inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)) 
    {
        perror("\nInvalid client_addr/ client_addr not supported \n");
        return -1;
    }
  
    if (0 > connect(socket_id, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    {
        printf("\nConnection Failed, Port number not used for data transmission \n");
        return -1;
    }
	else
	{
		printf("\nConnection to server established\n");
	}
	
	char filename[520],outputfilename[520] = "out_";
	printf("\nEnter file name: ");
	scanf("%s",filename);
    send(socket_id , filename , strlen(filename) , 0 );
	strcat(outputfilename,filename);
	printf("\nWaiting for data reception\n");
	FILE *fptr = fopen(outputfilename, "w");
	while(0 < (read( socket_id , buffer, MAX_CHAR_PER_LINE)))
	{
    	fputs(buffer,fptr);
		printf("\n%s",buffer);
		memset(buffer,'\0',512);
	}
	printf("\n%s",buffer);
	
	rewind(fptr);
	fseek(fptr, 0L, SEEK_END);
	int sz = ftell(fptr);
	if(0 == sz)
	{
		printf("\nNo data received\n");
		char cmd[100];
		sprintf(cmd, "rm -rf %s", outputfilename);
		(void) system(cmd);
		printf("\nFile does not exist on server\n");
	}
	else
	{
		printf("\nFile received\n");
		printf("\nLocal copy file name : %s\n",outputfilename);
	}
	fclose(fptr);
	printf("\nDisconnecting from server\n");
	printf("\nDisconnected\n");
    return 0;
}