#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>

int main (int argc, char* argv[]) {
    int op1, op2, somma, prodotto;
    int pid_server, pid1, pid2;
    int sock1, sock2;
    struct sockaddr_in server1, server2, client;
    fd_set sock_fdset;
    int length;
    int msgsock1, msgsock2;
    char line[256], answer[256];

    pid_server = getpid();
    printf("SERVER: server pid: %d\n", pid_server);

    // Creazione della socket STREAM 1 e STREAM 2

    sock1 = socket(AF_INET, SOCK_STREAM, 0);

    // gestione dell'eventuale errore nella creazione della socket 1
    if (sock1 < 0) {
        perror("opening socket 1");
        exit(1);
    }

    sock2 = socket(AF_INET, SOCK_STREAM, 0);

    if (sock2 < 0) {
        perror("opening socket 2");
        exit(1);
    }

    // accettiamo le connessioni da ogni client
    server1.sin_family = AF_INET;
    server1.sin_addr.s_addr = INADDR_ANY;
    server1.sin_port = htons(1520);

    int on = 1;

    /*
    if (setsockopt(sock1, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on)) < 0) {
        perror("setsockopt reuseaddr sock1");
    }
    */

    if (bind(sock1, (struct sockaddr *)&server1, sizeof server1) < 0) {
        perror("binding stream socket1");
        exit(1);
    }

    server2.sin_family = AF_INET;
    server2.sin_addr.s_addr = INADDR_ANY;
    server2.sin_port = htons(1521);

    if (bind(sock2, (struct sockaddr *)&server2, sizeof server2)) {
        perror("binding stream socket2");
        exit(1);
    }

    length = sizeof server1;

    if (getsockname(sock1, (struct sockaddr*)&server1, &length) < 0) {
        perror("getting socket name");
        exit(1);
    }
    printf("SOCKET 1 port %d\n", ntohs(server1.sin_port));

    length = sizeof server2;

    if (getsockname(sock2, (struct sockaddr*)&server2, &length) < 0) {
        perror("getting socket name");
        exit(1);
    }
    printf("SOCKET 2 port %d\n", ntohs(server2.sin_port));

    listen(sock1, 4);
    listen(sock2, 4);

    do {
        /*
         * espressione tipica dei programmi che usano select, quindi per controllare più socket
         * fd_zero inizializza sock_fdset, le altre due opzioni inseriscono prima
         * una sock e poi l'altra
         */
        FD_ZERO(&sock_fdset);
        FD_SET(sock1, &sock_fdset);
        FD_SET(sock2, &sock_fdset);

        int n;
        do {
            n = select(sock2 + 1, &sock_fdset, NULL, NULL, NULL);
        }
        while (n == -1 && errno == EINTR);

        length = sizeof client;
        if (FD_ISSET(sock1, &sock_fdset)) {
            msgsock1 = accept(sock1, (struct sockaddr*)&client, &length);
            if ((pid1 == fork()) == 0) {
                close(sock1);
                read(msgsock1, line, sizeof(line));
                sscanf(line, "%d%d", &op1, &op2);
                somma = op1 + op2;

                // aggiungere funzione log active

                sprintf(answer, "%d\n", somma);

                write(msgsock1, answer, strlen(answer) + 1);
                close(msgsock1);
                exit(0);
            }
            else {
                close(msgsock1);
             }
        }
        if (FD_ISSET(sock2, &sock_fdset)) {
            msgsock2 = accept(sock2, (struct sockaddr*)&client, &length);

            if ((pid2 == fork()) == 0) {
                close(sock2);
                read(msgsock2, line, sizeof line);
                sscanf(line, "%d%d", &op1, &op2);
                prodotto = op1 * op2;
                sprintf(answer, "%d\n", prodotto);

                // inserire funzione log

                write(msgsock2, answer, strlen(answer) + 1);
                close(msgsock2);

                exit(0);
            }
            else {
                close(msgsock2);
            }
        }
    } while (1);
}
