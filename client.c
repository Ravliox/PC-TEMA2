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
    int sockfd, n, SOCK_UTP;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int last_login = -1;
    int connected = 0;

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
    SOCK_UTP = socket (AF_INET, SOCK_DGRAM, 0);
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
        fprintf (log_file, "%s", buffer);
        
        //daca trebuie sa semnalam serverului ca ne deconectam
        if (strcmp (buffer, "quit\n") == 0){
            memset (buffer, 0, BUFLEN);
            sprintf (buffer, "%s", "Quitting");
            send(sockfd,buffer,strlen(buffer), 0);
            break;
        }
        // daca cerem unlock
        else if (strcmp (buffer, "unlock\n") == 0){
            char* last_login_string = malloc (20);
            sprintf (last_login_string, " %d", last_login);
            sprintf (buffer, "%s", "unlock");
            strcat (buffer, last_login_string);
        }

    	//trimit mesaj la server
    	n = send(sockfd,buffer,strlen(buffer), 0);
        char *token = strtok (buffer, " ");         // daca ce am trimis este o cerere de login
        if (strcmp (token, "login") == 0){          // salveaza numarul cardului
            token = strtok (NULL, " ");
            last_login = atoi (token);
              
        }
    	if (n < 0) 
        	 error("ERROR writing to socket");
        memset (buffer, 0, BUFLEN);
        n = recv (sockfd, buffer, sizeof(buffer), 0);   //primeste raspuns si afiseaza
        fprintf (log_file, "%s\n\n", buffer);
        printf ("%s\n", buffer);
    }

    fclose (log_file);
    return 0;
}


