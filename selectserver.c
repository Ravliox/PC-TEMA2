#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//structura in care ne vom tine informatiile despre clienti
typedef struct {
	char fname[12];
	char lname[12];
	int card_no;
	int pin;
	char secret_pass[16];
	float sold;
	int login;
	int counter;
	int socket;
} client;

#define MAX_CLIENTS	5
#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(1);
}

// functia cu care vom identifica userul ce detine numarul de card dat
int find_card (client* client_list, int user_count, int number){
	for (int i = 0; i < user_count; i++){
		if (client_list[i].card_no == number){
			return i;
		}
	}
	return -1;
}
// functia cu care vom identifica userul ce tocmai a introdus pinul
int find_pin (client* client_list, int user_count, int pin){
	for (int i = 0; i < user_count; i++){
		if (client_list[i].pin == pin){
			return i;
		}
	}
	return -1;
}

// functia ce identifica ce user avem pe socketul curent
int find_socket (client* client_list, int user_count, int socket){
	for (int i = 0; i < user_count; i++){
		if (client_list[i].socket == socket){
			return i;
		}
	}
	return -1;
} 

int main(int argc, char *argv[])
{
     int sockfd, SOCK_UTP, newsockfd, portno, clilen;
     char buffer[BUFLEN];
	 char send_buffer[BUFLEN];
     struct sockaddr_in serv_addr, cli_addr;
     int n, i, j, user_count, k;
	 
	 //deschidem fisierul cu useri
	 FILE* users_data_file = fopen (argv[2], "r");
	 char* line = malloc (sizeof(char) * 100);
	 fgets (line, 100, users_data_file);	// citim numarul de useri
	 k = 0;

	 user_count = atoi (line);
	 client* client_list = malloc (sizeof(client) * user_count);	//alocam vectorul de clienti

	 while (k < user_count){
		fgets (line, 100, users_data_file);
		// transpune tot textul intr-un dublu array
		char** data = malloc (sizeof (char*) * 6);
		for (int p = 0; p < 6; p++){
			data[p] = malloc (sizeof(char) * 20);
		}
		int p = 0;
		char* token = strtok (line, " ");
		while (token != NULL){
			strcpy (data[p], token);
			p++;
			token = strtok (NULL, " ");
		}

		// prelucreaza informatia din text in structura si adaug-o la vector
		strcpy (client_list[k].fname, data[0]);
		strcpy (client_list[k].lname, data[1]);
		client_list[k].card_no = atoi (data[2]);
		client_list[k].pin = atoi (data[3]);
		strcpy (client_list[k].secret_pass, data[4]);
		client_list[k].sold = atof (data[5]);
		client_list[k].login = 0;
		client_list[k].counter = 1;
		client_list[k].socket = -1;
		k++; 
	 }

	 
     fd_set read_fds;	//multimea de citire folosita in select()
     fd_set tmp_fds;	//multime folosita temporar 
     int fdmax;		//valoare maxima file descriptor din multimea read_fds

     if (argc < 2) {
         fprintf(stderr,"Usage : %s port\n", argv[0]);
         exit(1);
     }

     //golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
     FD_ZERO(&read_fds);
     FD_ZERO(&tmp_fds);
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
	 SOCK_UTP = socket (AF_INET, SOCK_DGRAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     
     portno = atoi(argv[1]);

     memset((char *) &serv_addr, 0, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
     serv_addr.sin_port = htons(portno);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
              error("ERROR on binding");
	 if (bind(SOCK_UTP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
              error("ERROR on binding");
     
     listen(sockfd, MAX_CLIENTS);

     //adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
     FD_SET(sockfd, &read_fds);
     fdmax = sockfd;
     // main loop
	 while (1) {
		tmp_fds = read_fds; 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("ERROR in select");
	
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
			
				if (i == sockfd) {
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR in accept");
					} 
					else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}
					
				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
							printf("selectserver: socket %d hung up\n", i);
						} else {
							error("ERROR in recv");
						}
						close(i); 
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care 
					} 
					
					else { //recv intoarce >0
						memset(send_buffer, 0 , BUFLEN);
						char* tokens = strtok (buffer, " ");
						// citim primul cuvant
						if (strcmp (tokens, "login") == 0){
							
							tokens = strtok (NULL, " ");	//luam numarul de card
							int nr_card = atoi(tokens);
							tokens = strtok (NULL, " ");	//luam pinul
							int pin = atoi (tokens);
							printf ("PIN: %d\n", pin);
							int find = find_card (client_list, user_count, nr_card);	// gaseste cardul
							if (find < 0){												//daca nu il gasesti
								sprintf (send_buffer, "%s", "ATM> -4 : Numar card inexistent");
								send (i, send_buffer, strlen(send_buffer), 0);
							}
							else if (client_list[find].login == 0){						//daca il gasesti
																						//verifica daca exista ses. deschisa
								int pin_find = find_pin (client_list, user_count, pin);
								if (pin_find < 0){										// verifica pinul
									if (client_list[find].counter == 3){
										client_list[find].login = 2;
										sprintf (send_buffer, "%s", "ATM> -5 : Card Blocat");		//blocheaza cardul
										send (i, send_buffer, strlen(send_buffer), 0);
									}
									else {
										sprintf (send_buffer, "%s", "ATM> -3 : Pin incorect");
										send (i, send_buffer, strlen(send_buffer), 0);
										client_list[find].counter++;
									}
								}
								else {				// succes
									sprintf (send_buffer, "ATM> Welcome, %s %s!", client_list[find].fname, client_list[find].lname);
									send (i, send_buffer, strlen(send_buffer), 0);
									client_list[find].login = 1;
									client_list[find].socket = i;
								}
							} 
							else if (client_list[find].login == 1){					//daca sesiunea e deschisa
								sprintf (send_buffer, "%s", "ATM> -2 : Sesiune deja deschisa");
								send (i, send_buffer, strlen (send_buffer), 0);
							}
							else if (client_list[find].login == 2){					//daca cardul e blocat
								sprintf (send_buffer, "%s", "ATM> -5 : Card blocat");
								send (i, send_buffer, strlen (send_buffer), 0);
							}
						}
						else if (strcmp (tokens, "logout\n") == 0){					// pentru logout
							int socket_find = find_socket (client_list, user_count, i);
							if (socket_find >= 0){									// daca este autentificat deja
								client_list[socket_find].socket = -1;
								client_list[socket_find].login = 0;
								sprintf (send_buffer, "%s", "ATM> Deconectare de la bancomat!");
								send (i, send_buffer, strlen (send_buffer), 0);
							}
							else {													// daca nu
								printf ("%d %d \n", socket_find, client_list[socket_find].socket);
								sprintf (send_buffer, "%s", "ATM> -1 : Clientul nu este autentificat");
								send (i, send_buffer, strlen (send_buffer), 0);
							}
						}
						else if (strcmp (tokens, "listsold\n") == 0){				// pentru sold
							int socket_find = find_socket (client_list, user_count, i);	//identifica userul
							if (socket_find >= 0){
								sprintf (send_buffer, "ATM> %.2f", client_list[socket_find].sold);
								send (i, send_buffer, strlen (send_buffer), 0);
							}
							else{
								sprintf (send_buffer, "%s", "ATM> -1 : Clientul nu este autentificat");
								send (i, send_buffer, strlen (send_buffer), 0);
							}
						}
						else if (strcmp (tokens, "getmoney") == 0){					//pentru getmoney
							int socket_find = find_socket (client_list, user_count, i);
							if (socket_find >= 0){
								tokens = strtok (NULL, " ");
								int sum = atoi (tokens);
								if (sum % 10 != 0){									//verificam conditiile
									sprintf (send_buffer, "%s", "ATM> -9 : Suma nu este multiplu de 10");
									send (i, send_buffer, strlen (send_buffer), 0);
								}
								else if (sum > client_list[socket_find].sold){
									sprintf (send_buffer, "%s", "ATM> -8 : Fonduri insuficiente");
									send (i, send_buffer, strlen (send_buffer), 0);
								}
								else {
									client_list[socket_find].sold -= sum;
									sprintf (send_buffer, "ATM> Suma %d retrasa cu succes", sum);
									send (i, send_buffer, strlen (send_buffer), 0); 
								}
							}
							else{													//daca nu este autentificat
								sprintf (send_buffer, "%s", "ATM> -1 : Clientul nu este autentificat");
								send (i, send_buffer, strlen (send_buffer), 0);
							}
						}
						else if (strcmp (tokens, "putmoney") == 0){					//pentru putmoney
							int socket_find = find_socket (client_list, user_count, i);
							if (socket_find >= 0){
								tokens = strtok (NULL, " ");
								float sum = atof (tokens);
								client_list[socket_find].sold += sum;
								sprintf (send_buffer, "%s", "ATM> Suma depusa cu succes");
								send (i, send_buffer, strlen (send_buffer), 0);
							}
							else{
								sprintf (send_buffer, "%s", "ATM> -1 : Clientul nu este autentificat");
								send (i, send_buffer, strlen (send_buffer), 0);
							}
						}
						else if (strcmp (tokens, "Quitting") == 0){				//pentru quit
							int socket_find = find_socket (client_list, user_count, i);
							if (socket_find >= 0){								// daca e logat 
								client_list[socket_find].socket = -1;			//delogheaza-l
								client_list[socket_find].login = 0;
							}
							printf("Client disconnected", i);					//sterge-l din lista
							close(i); 
							FD_CLR(i, &read_fds);	

						}
						else if (strcmp (tokens, "unlock") == 0){				//pentru unlock
							tokens = strtok (NULL, " ");
							int card_to_unlock = atoi (tokens);
							
							int unlock_find = find_card (client_list, user_count, card_to_unlock);		//gaseste cardul
							printf ("%d %d\n", card_to_unlock, unlock_find);
							sprintf (send_buffer, "%s", "UNLOCK> Trimite parola secreta");				//cere parola
							send (i, send_buffer, strlen (send_buffer), 0);
							memset(buffer, 0 , BUFLEN);
							memset(send_buffer, 0 , BUFLEN);
							recv(i, buffer, sizeof(buffer), 0);											//primeste parola
							tokens = strtok (buffer, "\n");
							if ( strcmp (buffer, client_list[unlock_find].secret_pass) != 0 ){			//esuare
								sprintf (send_buffer, "%s", "UNLOCK > Deblocare esuata");
								send (i, send_buffer, strlen (send_buffer), 0);
							}
							else {
								sprintf (send_buffer, "%s", "UNLOCK > Client deblocat");				//succes
								client_list[unlock_find].login = 0;
								send (i, send_buffer, strlen (send_buffer), 0);
							}
						}
					}					
				}
			} 
		}
	}



     close(sockfd);
   
     return 0; 
}


