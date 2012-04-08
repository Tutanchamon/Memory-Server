/******************************************
 * Filename : multi.c
 * Purpose  : handles data from up to three sockets and relays to others
 *            could easily be expanded
 * Author   : Simon Amor (simon@foobar.co.uk)
 *
 * Warning  : This code is not fully functional - it's also not my usual
 *            coding style. Converting this to a set of functions is left
 *            as an exercise to the reader ;-)
 ***/

/*
 * WARNING: THIS CODE IS ONLY PARTIALLY FUNCTIONAL
 */

#include <stdio.h>
/* for EXIT_FAILURE and EXIT_SUCCESS */
#include <string.h>
#include <stdlib.h>

#define TRUE   1
#define FALSE  0
#define MAX_BUF 10240

/* network functions */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* FD_SET, FD_ISSET, FD_ZERO macros */
#include <sys/time.h>
#include <errno.h>
// include'y do wysylania plikow
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>

/* metoda z cwiczen od Gogolewskiego */
void ObsluzPolaczenie(int gn)
{
    char sciezka[512];
    long dl_pliku, wyslano, wyslano_razem, przeczytano;
    struct stat fileinfo;
    FILE* plik;
    unsigned char bufor[1024];

   /* memset(sciezka, 0, 512);
    if (recv(gn, sciezka, 512, 0) <= 0)
    {
        printf("Potomny: blad przy odczycie sciezki\n");
        return;
    }*/
    strcpy(sciezka, "a.gif");

    printf("Potomny: klient chce plik %s\n", sciezka);

    if (stat(sciezka, &fileinfo) < 0)
    {
        printf("Potomny: nie moge pobrac informacji o pliku\n");
        return;
    }

    if (fileinfo.st_size == 0)
    {
        printf("Potomny: rozmiar pliku 0\n");
        return;
    }

    printf("Potomny: dlugosc pliku: %d\n", fileinfo.st_size);

    dl_pliku = htonl((long) fileinfo.st_size);

   /* if (send(gn, &dl_pliku, sizeof(long), 0) != sizeof(long))
    {
        printf("Potomny: blad przy wysylaniu wielkosci pliku\n");
        return;
    }*/

    dl_pliku = fileinfo.st_size;
    wyslano_razem = 0;
    plik = fopen(sciezka, "rb");
    if (plik == NULL)
    {
        printf("Potomny: blad przy otwarciu pliku\n");
        return;
    }

    while (wyslano_razem < dl_pliku)
    {
        przeczytano = fread(bufor, 1, 1024, plik);
        wyslano = send(gn, bufor, przeczytano, 0);
        if (przeczytano != wyslano)
            break;
        wyslano_razem += wyslano;
        printf("Potomny: wyslano %d bajtow\n", wyslano_razem);
    }

    if (wyslano_razem == dl_pliku)
        printf("Potomny: plik wyslany poprawnie\n");
    else
        printf("Potomny: blad przy wysylaniu pliku\n");
    fclose(plik);
    return;
}
/*********************************/


int main()
{
  int opt=TRUE;
  int master_socket;
  struct sockaddr_in address;
  int addrlen;

  int new_socket;

  int client_socket[3];
  int max_clients=3;

  int activity, loop, loop2, valread;
  char buffer[10240];    /* data buffer of 1K */

  char filename[10240];
    struct stat stat_buf;      /* argument to fstat */
    int rc;
    int offset = 0;
    int i;

    // te z polibudy
    int fd;
	int count_r, count_w;
	char* bufptr;
	char buf[MAX_BUF];


  fd_set readfds;

  char *message="Data-relay v0.1 (C)1996 Simon Amor <simon@foobar.co.uk>\n\r";

/* initialise all client_socket[] to 0 so not checked */
  for (loop=0; loop < max_clients; loop++) {
    client_socket[loop] = 0;
  }

/* create the master socket and check it worked */
  if ((master_socket = socket(AF_INET,SOCK_STREAM,0))==0) {
/* if socket failed then display error and exit */
    perror("Create master_socket");
    exit(EXIT_FAILURE);
  }

/* set master socket to allow multiple connections */
  if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,
                 (char *)&opt, sizeof(opt))<0) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

/* type of socket created */
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
/* 7000 is the port to use for connections */
  address.sin_port = htons(7000);
/* bind the socket to port 7000 */
  if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
/* if bind failed then display error message and exit */
    perror("bind");
    exit(EXIT_FAILURE);
  }

/* try to specify maximum of 3 pending connections for the master socket */
  if (listen(master_socket, 3)<0) {
/* if listen failed then display error and exit */
    perror("listen");
    exit(EXIT_FAILURE);
  }

  while (1==1) {
    FD_ZERO(&readfds);
/* reason we say max_clients+3 is stdin,stdout,stderr take up the first
 * couple of descriptors and we might as well allow a couple of extra.
 * If your program opens files at all you will want to allow enough extra.
 * Another option is to specify the maximum your operating system allows.
 */

/* setup which sockets to listen on */
    FD_SET(master_socket, &readfds);
    for (loop=0; loop<max_clients; loop++) {
      if (client_socket[loop] > 0) {
        FD_SET(client_socket[loop], &readfds);
      }
    }

/* wait for connection, forever if we have to */
    activity=select(max_clients+3, &readfds, NULL, NULL, NULL);

    if ((activity < 0) && (errno!=EINTR)) {
/* there was an error with select() */
    }
    if (FD_ISSET(master_socket, &readfds)) {
/* Open the new socket as 'new_socket' */
      addrlen=sizeof(address);
      if ((new_socket = accept(master_socket, (struct sockaddr *)&address, &addrlen))<0)
      {
/* if accept failed to return a socket descriptor, display error and exit */
        perror("accept");
        exit(EXIT_FAILURE);
      }
/* inform user of socket number - used in send and receive commands */
      printf("New socket is fd %d\n",new_socket);

/* transmit message to new connection */
/*      if (send(new_socket, message, strlen(message), 0) != strlen(message)) {
/* if send failed to send all the message, display error and exit */
     /*   perror("send");
      }*/
     // else printf("\nWiadomosc wyslana");

     // puts("Welcome message sent successfully");

/* add new socket to list of sockets */
        ObsluzPolaczenie(new_socket);
      for (loop=0; loop<max_clients; loop++) {
        if (client_socket[loop] == 0) {
          client_socket[loop] = new_socket;
          printf("Adding to list of sockets as %d\n", loop);
          loop = max_clients;
        }
      }
    }
    for (loop=0; loop<max_clients; loop++) {
        printf("Idzie petla z loop\n");
      if (FD_ISSET(client_socket[loop], &readfds)) {
          // dodane ode mnie
        if ((valread = recv(client_socket[loop], buffer, 10240, 0)) < 0) { // bylo read zamiast recv
          printf("Wywolal sie fragment, ktorego nie czaje\n");
          close(client_socket[loop]);
          client_socket[loop] = 0;
        } else {
/* set the terminating NULL byte on the end of the data read */

          buffer[valread] = '\0';


          // dodane do obslugi plikow
            // ale jednak i tak coś nie dziala
            // wiec do zakomentowania
          /*strcpy(filename, buffer);
          printf("\nFilename to: %s\n", filename);
          filename[valread] = '\0';
          if (filename[strlen(filename)-1] == '\n')
            filename[strlen(filename)-1] = '\0';
          if (filename[strlen(filename)-1] == '\r')
            filename[strlen(filename)-1] = '\0';

            /* open the file to be sent */

            /*fd = open(filename, O_RDONLY);
            if (fd == -1) {
              fprintf(stderr, "unable to open '%s': %s\n", filename, strerror(errno));
              exit(1);
            }

            /* get the size of the file to be sent */
           // fstat(fd, &stat_buf);







          // to bylo dodane do osblugi plikow
            printf("\nWchodze do loop2");
          for (loop2=0; loop2<max_clients; loop2++) {
/* note, flags for send() are normally 0, see man page for details */
            printf("Tera idzie po loop2\n");
          //  send(client_socket[loop2], buffer, strlen(buffer), 0);   ----- chwilowo usuniete do testow z plikami
          /**********************************************/
          /* wrzucone zamiast senda by wyslac plik      */

          /* copy file using sendfile
            offset = 0;
            rc = sendfile (client_socket[loop2], fd, (off_t*) &offset, stat_buf.st_size);
            if (rc == -1) {
              fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
              exit(1);
            }
            if (rc != stat_buf.st_size) {
              fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",
                      rc,
                      (int)stat_buf.st_size);
              exit(1);
            }



          /*********************************************/



          }
          printf("Idzie memset\n");
          memset(buffer, 0, strlen(buffer));
          strcpy(buffer,"");
          /* close descriptor for file that was sent */
            close(fd);



        }
/*
 * use read() to read from the socket into a buffer using something
 * similar to the following:
 *
 * If the read didn't cause an error then send buffer to all
 * client sockets in the array - use for loop and send() just
 * as if you were sending it to one connection
 *
 * important point to note is that if the connection is char-by-char
 * the person will not be able to send a complete string and you will
 * need to use buffers for each connection, checking for overflows and
 * new line characters (\n or \r)
 */
      }
    }
  }

/* normally you should close the sockets here before program exits */
/* however, in this example the while() loop prevents getting here */
}

