#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int jugadornum= 0;
pthread_mutex_t mutexcont;

void error(const char *msj)
{
    perror(msj);
    pthread_exit(NULL);
}


void escribir_cliente_int(int cli_sockfd, int msj)
{
    int n = write(cli_sockfd, &msj, sizeof(int));
    if (n < 0)
        error(" Error escribiendo");
}



void escribir_clientes_int(int * cli_sockfd, int msj)
{
    escribir_cliente_int(cli_sockfd[0], msj);
    escribir_cliente_int(cli_sockfd[1], msj);
}

int Escuchar(int puerto)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR abriendo socket de escucha.");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;	
    serv_addr.sin_addr.s_addr = INADDR_ANY;	
    serv_addr.sin_port = htons(puerto);		

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR construyendo socket de escucha.");

 
    printf(" Escuchas.\n");    
   

    return sockfd;
}
int recv_int(int cli_sockfd)
{
    int msj = 0;
    int n = read(cli_sockfd, &msj, sizeof(int));
    
    if (n < 0 || n != sizeof(int))  return -1;

    printf("Recibiendo int: %d\n", msj);
    
    return msj;
}

void write_client_msj(int cli_sockfd, char * msj)
{
    int n = write(cli_sockfd, msj, strlen(msj));
    if (n < 0)
        error("ERROR Escribiendo msj en el socket del cliente");
}

void escribir_cliente_msj(int * cli_sockfd, char * msj)
{
    write_client_msj(cli_sockfd[0], msj);
    write_client_msj(cli_sockfd[1], msj);
}


void obtenerClientes(int lis_sockfd, int * cli_sockfd)
{
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
   
    printf(" Escuchando a los clientes...\n");


    int num_conn = 0;
    while(num_conn < 2)
    {
	    listen(lis_sockfd, 253 - jugadornum);
        
        memset(&cli_addr, 0, sizeof(cli_addr));

        clilen = sizeof(cli_addr);
	
        cli_sockfd[num_conn] = accept(lis_sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
        if (cli_sockfd[num_conn] < 0)
            error("ERROR en la coneccion con el cliente.");

        printf("Conexion aceptada del cliente %d\n", num_conn);
    
        
        write(cli_sockfd[num_conn], &num_conn, sizeof(int));
        
       
        printf("Cliente %d .\n", num_conn); 
   
        
        pthread_mutex_lock(&mutexcont);
        jugadornum++;
        printf("Numero de jugadores %d.\n", jugadornum);
        pthread_mutex_unlock(&mutexcont);

        if (num_conn == 0) {
            write_client_msj(cli_sockfd[0],"HLD");
            
         
            printf(" Esperando Cliente.\n");
            
        }

        num_conn++;
    }
}

int get_jugador_mov(int cli_sockfd)
{

    printf(" Obteniendo jugador mov...\n");
   
    
    write_client_msj(cli_sockfd, "TRN");

    return recv_int(cli_sockfd);
}

int check_mov(char tablero[][3], int mov, int jugador_id)
{
    if ((mov == 9) || (tablero[mov/3][mov%3] == ' ')) { 
        
      
        printf(" jugador %d's mov no valido.\n", jugador_id);

        
        return 1;
   }
   else {    
    
       printf(" jugador %d's mov no valido.\n", jugador_id);
 
    
       return 0;
   }
}

void actual_tablero(char tablero[][3], int mov, int jugador_id)
{
    tablero[mov/3][mov%3] = jugador_id ? 'X' : 'O';
    
  
    printf("tablero actualizado.\n");

}

void hacer_tablero(char tablero[][3])
{
    printf(" %c | %c | %c \n", tablero[0][0], tablero[0][1], tablero[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", tablero[1][0], tablero[1][1], tablero[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", tablero[2][0], tablero[2][1], tablero[2][2]);
}

void send_update(int * cli_sockfd, int mov, int jugador_id)
{
   
    printf("Enviando actualizacion...\n");

    
    escribir_cliente_msj(cli_sockfd, "UPD");

    escribir_clientes_int(cli_sockfd, jugador_id);
    
    escribir_clientes_int(cli_sockfd, mov);
    

    printf(" actualizacion.\n");

}

void send_jugadornum(int cli_sockfd)
{
    write_client_msj(cli_sockfd, "CNT");
    escribir_cliente_int(cli_sockfd, jugadornum);


    printf(" jugador enviado\n");

}

int check_tablero(char tablero[][3], int ultimo_mov)
{

    printf(" Checando ganador...\n");

   
    int fila = ultimo_mov/3;
    int col = ultimo_mov%3;

    if ( tablero[fila][0] == tablero[fila][1] && tablero[fila][1] == tablero[fila][2] ) { 
   
        printf(" Ganado por fila %d.\n", fila);
     
        return 1;
    }
    else if ( tablero[0][col] == tablero[1][col] && tablero[1][col] == tablero[2][col] ) { 

        printf(" Ganado por columna %d.\n", col);
     
        return 1;
    }
    else if (!(ultimo_mov % 2)) { if ( (ultimo_mov == 0 || ultimo_mov == 4 || ultimo_mov == 8) && (tablero[1][1] == tablero[0][0] && tablero[1][1] == tablero[2][2]) ) 
        { 
  
            printf(" Ganado por diagonal.\n");
         
            return 1;
        }
        if ( (ultimo_mov == 2 || ultimo_mov == 4 || ultimo_mov == 6) && (tablero[1][1] == tablero[0][2] && tablero[1][1] == tablero[2][0]) ) 
            { 
       
            printf(" Ganado por diagonal.\n");
 
            return 1;
        }
    }


    printf(" No hubo ganador.\n");

    
    return 0;
}

void *ComJuego(void *thread_data) 
{
    int *cli_sockfd = (int*)thread_data; 
    char tablero[3][3] = { {' ', ' ', ' '},  
                         {' ', ' ', ' '}, 
                         {' ', ' ', ' '} };

    printf("Juego listo!\n");
    
       escribir_cliente_msj(cli_sockfd, "SRT");


    printf(" Mensaje de Inicio enviado.\n");


    hacer_tablero(tablero);
    
    int prev_jugador_turn = 1;
    int jugador_turn = 0;
    int JuegoFin = 0;
    int turn_count = 0;
    while(!JuegoFin) {
        
        if (prev_jugador_turn != jugador_turn)
            write_client_msj(cli_sockfd[(jugador_turn + 1) % 2], "WAT");

        int valid = 0;
        int mov = 0;
        while(!valid) {             mov = get_jugador_mov(cli_sockfd[jugador_turn]);
            if (mov == -1) break; 
            printf("jugador %d Selecciono la pocicion %d\n", jugador_turn, mov);
                
            valid = check_mov(tablero, mov, jugador_turn);
            if (!valid) { 
                printf("mov no valido....\n");
                write_client_msj(cli_sockfd[jugador_turn], "INV");
            }
        }

	    if (mov == -1) { 
            printf("jugador desconectado.\n");
            break;
        }
        else if (mov == 9) {
            prev_jugador_turn = jugador_turn;
            send_jugadornum(cli_sockfd[jugador_turn]);
        }
        else {
                    actual_tablero(tablero, mov, jugador_turn);
            send_update( cli_sockfd, mov, jugador_turn );
                
         
            hacer_tablero(tablero);

                        JuegoFin = check_tablero(tablero, mov);
            
            if (JuegoFin == 1) {
                write_client_msj(cli_sockfd[jugador_turn], "WIN");
                write_client_msj(cli_sockfd[(jugador_turn + 1) % 2], "LSE");
                printf("jugador %d gano.\n", jugador_turn);
            }
            else if (turn_count == 8) {                printf("hacer.\n");
                escribir_cliente_msj(cli_sockfd, "DRW");
                JuegoFin = 1;
            }

            prev_jugador_turn = jugador_turn;
            jugador_turn = (jugador_turn + 1) % 2;
            turn_count++;
        }
    }

    printf("juego terminado.\n");

	close(cli_sockfd[0]);
    close(cli_sockfd[1]);

    pthread_mutex_lock(&mutexcont);
    jugadornum--;
    printf("Numero de jugadores %d.", jugadornum);
    jugadornum--;
    printf("Numero de jugadores %d.", jugadornum);
    pthread_mutex_unlock(&mutexcont);
    
    free(cli_sockfd);

    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{   
    if (argc < 2) {
        fprintf(stderr,"ERROR, no ingreso puerto\n");
        exit(1);
    }
    
    int lis_sockfd = Escuchar(atoi(argv[1])); 
    pthread_mutex_init(&mutexcont, NULL);

    while (1) {
        if (jugadornum<= 252) {   
            int *cli_sockfd = (int*)malloc(2*sizeof(int)); 
            memset(cli_sockfd, 0, 2*sizeof(int));
            
            obtenerClientes(lis_sockfd, cli_sockfd);
            
    
            printf(" Empezando nuevo juego...\n");
  

            pthread_t thread;int result = pthread_create(&thread, NULL, ComJuego, (void *)cli_sockfd);
            if (result){
                printf("Creacion de hilo fallo: %d\n", result);
                exit(-1);
            }
            
            printf(" Nuevo juego listo.\n");
            }
    }

    close(lis_sockfd);

    pthread_mutex_destroy(&mutexcont);
pthread_exit(NULL);
}
