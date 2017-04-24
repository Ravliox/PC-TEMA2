#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h> 

#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    /* opening the log file */
    char id_name[30] = "client-";
    char pid_string[15];
    int pid = getpid();     // pid of the process
    sprintf (pid_string, "%d", pid);
    strcat (id_name, pid_string);
    strcat (id_name, ".log");
    
    FILE* log_file = fopen (id_name, "w");

    char buffer[BUFLEN];
    if (argc < 3) {
       fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
       exit(0);
    }  
    
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addr.sin_addr);
    
    
    if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");    
    
    while(1){
  		//citesc de la tastatura
    	memset(buffer, 0 , BUFLEN);
    	fgets(buffer, BUFLEN-1, stdin);

        if (strcmp (buffer, "quit\n") == 0){
            memset (buffer, 0, BUFLEN);
            sprintf (buffer, "%s", "Quitting");
            send(sockfd,buffer,strlen(buffer), 0);
            break;
        }

    	//trimit mesaj la server
    	n = send(sockfd,buffer,strlen(buffer), 0);
    	if (n < 0) 
        	 error("ERROR writing to socket");
        memset (buffer, 0, BUFLEN);
        n = recv (sockfd, buffer, sizeof(buffer), 0);
        fprintf (log_file, "ATM> %s\n", buffer);
        printf ("ATM> %s\n", buffer);
    }

    fclose (log_file);
    return 0;
}


