#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


void recv_msg(int sockfd, char * msg)
{
    memset(msg, 0, 4);
    int n = read(sockfd, msg, 3);
    
    if (n < 0 || n != 3)
     herror("herror server.");

    printf(" Recibir mensaje: %s\n", msg);
   }

int recv_int(int sockfd)
{
    int msg = 0;
    int n = read(sockfd, &msg, sizeof(int));
    
    if (n < 0 || n != sizeof(int)) 
        herror("herror leyendo  de server socket");
    
    printf(" Recibir : %d\n", msg);
    
    return msg;
}

void write_server_int(int sockfd, int msg)
{
    int n = write(sockfd, &msg, sizeof(int));
    if (n < 0)
        herror("herror escribiendo en server socket");
    
    printf(" escribiendo en server: %d\n", msg);
    }

void herror(const char *msg)
{
    perror(msg);
    printf("juagdor desconectado.\nfin del juego.\n");
    
    exit(0);
}
int connect_en_server(char * hostname, int portno)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
 
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
    if (sockfd < 0) 
        herror("herror abriendo el server.");
	
    server = gethostbyname(hostname);
	
    if (server == NULL) {
        fprintf(stderr,"herror\n");
        exit(0);
    }
	
	memset(&serv_addr, 0, sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
    memmove(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno); 

   if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        herror("herror conectando en server");

    printf("conectado en server.\n");
     return sockfd;
}

void hacer_tablero(char tablero[][3])
{
    printf(" %c | %c | %c \n", tablero[0][0], tablero[0][1], tablero[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", tablero[1][0], tablero[1][1], tablero[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", tablero[2][0], tablero[2][1], tablero[2][2]);
}

void take_turn(int sockfd)
{
    char buffer[10];
    
    while (1) { 
        printf("Escribe del 0-8 para mover o 9 para jugador ");
	    fgets(buffer, 10, stdin);
	    int move = buffer[0] - '0';
        if (move <= 9 && move >= 0){
            printf("\n");
             write_server_int(sockfd, move);   
            break;
        } 
        else
            printf("\nNo valido.\n");
    }
}

void get_update(int sockfd, char tablero[][3])
{
    
    int player_id = recv_int(sockfd);
    int move = recv_int(sockfd);
    tablero[move/3][move%3] = player_id ? 'X' : 'O';    
}


int main(int argc, char *argv[])
{
      if (argc < 3) {
       fprintf(stderr,"Uso %s Puerto\n", argv[0]);
       exit(0);
    }

     int sockfd = connect_en_server(argv[1], atoi(argv[2]));

   
    int id = recv_int(sockfd);


    printf(" Cliente ID: %d\n", id);


    char msg[4];
    char tablero[3][3] = { {' ', ' ', ' '}, 
                         {' ', ' ', ' '}, 
                         {' ', ' ', ' '} };

    printf("Tic-Tac-ene\n------------\n");

    do {
        recv_msg(sockfd, msg);
        if (!strcmp(msg, "HLD"))
            printf("Esperando el segundo jugador...\n");
    } while ( strcmp(msg, "SRT") );

    /* The game has begun. */
    printf("Juego listo!\n");
    printf("Tu eres %c's\n", id ? 'X' : 'O');

    hacer_tablero(tablero);

    while(1) {
        recv_msg(sockfd, msg);

        if (!strcmp(msg, "TRN")) { 
	        printf("Tu movimiento...\n");
	        take_turn(sockfd);
        }
        else if (!strcmp(msg, "INV")) { 
            printf("Esa posisicion ya fue seleccionada.\n"); 
        }
        else if (!strcmp(msg, "CNT")) { 
            int num_players = recv_int(sockfd);
            printf("Actualmente hay %d jugadores activos.\n", num_players); 
        }
        else if (!strcmp(msg, "UPD")) { 
            get_update(sockfd, tablero);
            hacer_tablero(tablero);
        }
        else if (!strcmp(msg, "WAT")) { 
            printf("Esperando al otro jugador...\n");
        }
        else if (!strcmp(msg, "WIN")) { 
            printf("Haz ganado\n");
            break;
        }
        else if (!strcmp(msg, "LSE")) { 
            printf("Haz perdido.\n");
            break;
        }
        else if (!strcmp(msg, "DRW")) { 
            printf("Dibujando.\n");
            break;
        }
        else 
            herror("herror.");
    }
    
    printf("fin del juego.\n");
    close(sockfd);
    return 0;
}
