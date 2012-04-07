// PROSTY KLIENT do chatu

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#define IP(H) *((unsigned long*) (H)->h_addr_list[0])

int main(){

int gniazdo;
char adres[512];
int port;
char buforod[1024];
char buforzap[1024];
int odebrano;
struct sockaddr_in adr;
struct hostent *h;

long dl_pliku, odebrano_razem;

printf("Podaj adres IP lub nazwe hosta serwera: ");
scanf("%s", adres);
h = gethostbyname(adres);
if (h == NULL){
    printf("Zly adres host");
    return 1;
}
printf("Podaj numer portu: ");
scanf("%d", &port);




gniazdo = socket(PF_INET, SOCK_STREAM, 0);

adr.sin_family = AF_INET;
adr.sin_port = htons(port);
adr.sin_addr = *(struct in_addr*) h->h_addr;

printf("Lacze sie z %s:%d\n", inet_ntoa(adr.sin_addr), port);

if (connect(gniazdo, (struct sockaddr*) &adr, sizeof(adr)) <0 ){
    printf("Nawiazanie polaczenia nie udalo sie");
    close(gniazdo);
    return 2;
}

printf("Polaczenie nawiazane\n");

while(1){
    if (odebrano = recv(gniazdo, buforod, 1024, 0) < 0){
        printf("Blad przy odbiorze!\n");
    }
    else {
        printf("Wiadomosc od: %s: %s\n", inet_ntoa(adr.sin_addr), buforod);
        memset(buforod, 0, strlen(buforod));
    }


   // printf("Podaj tresc komunikatu: ");
   // scanf("%s", buforzap);
   // if (send(gniazdo, buforzap, strlen(buforzap), 0) < 0){
   //     printf("Wyslanie nie powiodlo sie\n");
    //}

}

close(gniazdo);
return 0;



}
