#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <math.h>
#define PI 3.14159265359

#define BUFSZ 1024

void usage(int argc, char **argv) 
{
    printf("usage: %s <ipv4|ipv6> <server port>\n", argv[0]);
    printf("example: %s ipv4 50501\n", argv[0]);
    exit(EXIT_FAILURE);
}

//função para calcular as distâncias entre o motorista e o cliente
double haversine(double lat1, double lon1, double lat2, double lon2)
{
    //distância entre latitudes e longitudes
    double dLat = (lat2 - lat1) * PI / 180.0;
    double dLon = (lon2 - lon1) * PI / 180.0;
 
    //convertendo para radianos
    lat1 = (lat1) * PI / 180.0;
    lat2 = (lat2) * PI / 180.0;
 
    //aplicando fórmula
    double a = pow(sin(dLat / 2), 2) + (pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2));
    double rad = 6371;
    double c = 2 * asin(sqrt(a));

    double dist_em_m = (rad * c)*1000;

    return dist_em_m;
}

typedef struct {
    double latitude;
    double longitude;
} Coordinate;

int main(int argc, char **argv)
{
    // Criando coordenada do servidor
    Coordinate coordServ = {-19.9227,-43.9451};

    if(argc < 3) //se foram passados menos parâmetros do que o necessário
    { 
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    //utiliza dos parâmetros passados na linha de comando para analisar o endereço
    //e preencher a estrutura sockaddr_storage do servidor
    if(0 != server_sockaddr_init(argv[1], argv[2], &storage))
    {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0); //cria um socket para ponto de comunicação
    if(s == -1)
    {
        logexit("socket");
    }

    int enable = 1;
    //permitir que o servidor reutilize a porta
    if(0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
    {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(0 != bind(s, addr, sizeof(storage))) //Associa um endereço IP + Porta ao socket
    {
        logexit("bind");
    }

    if(0 != listen(s, 100)) //Define um socket como passivo e fica aguardando uma conexão
    {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ); //converte o endereço em uma string com a versão, endereço e porta

    while (1)
    {
        printf("Aguardando solicitação\n");

        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen); //Aceita uma conexão com o cliente em um socket passivo
        if(csock == -1)
        {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ); //converte o endereço em uma string com a versão, endereço e porta

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);

        //receber se o cliente solicitou ou não a corrida
        size_t count = recv(csock, buf, BUFSZ - 1, 0); //recebe mensagem do cliente

        //avisar que recebeu a decisão
        char ok[10];
        sprintf(ok, "1 - ok");
        count = send(csock, ok, strlen(ok) + 1, 0); //envia mensagem para o cliente
        if(count != strlen(ok) + 1)
        {
            logexit("send");
        }

        if (buf[0] == '0') //cliente apertou em sair
        {
            close(csock); //fecha o socket que está conectado ao cliente
        }
        else if(buf[0] == '1') //cliente solicitou corrida
        {
            //receber coordenadas do cliente e calcular distância inicial
            Coordinate coordCli;
            count = recv(csock, &coordCli, sizeof(Coordinate), 0);
            double distancia_inicial = 0;
            distancia_inicial = haversine(coordServ.latitude, coordServ.longitude, coordCli.latitude, coordCli.longitude);

            //dar opçao do motorista(servidor) aceitar ou não a corrida
            printf("Corrida disponível:\n0 - Recusar\n1 - Aceitar\n");
            fgets(buf, BUFSZ-1, stdin);

            //se o servidor recusar corrida
            if(buf[0] == '0')
            {
                sprintf(buf, "Não foi encontrado um motorista.\n");
                count = send(csock, buf, strlen(buf) + 1, 0); //envia mensagem
                if(count != strlen(buf) + 1)
                {
                    logexit("send");
                }
            }

            //se o servidor aceitar a corrida
            if(buf[0] == '1')
            {
                //avisar o cliente que o servidor aceitou a corrida
                sprintf(buf, "1\n");
                count = send(csock, buf, strlen(buf) + 1, 0); //envia mensagem
                if(count != strlen(buf) + 1)
                {
                    logexit("send");
                }

                double distancia_atual = distancia_inicial;
                sleep(1);
                while (distancia_atual > 0)
                {
                    //envia mensagens em loop para o cliente, 
                    //atualizando da distância em que o motorista se encontra
                    sprintf(buf, "Motorista a %.0fm.\n", distancia_atual);
                    count = send(csock, buf, strlen(buf) + 1, 0);
                    if(count != strlen(buf) + 1)
                    {
                        logexit("send");
                    }
                    distancia_atual = distancia_atual - 400.0;
                    memset(buf, 0, BUFSZ); //zera o buffer
                    sleep(2); //espera dois segundos(2000ms) para continuar
                }

                sprintf(buf, "O motorista chegou!\n");
                count = send(csock, buf, strlen(buf) + 1, 0); //envia mensagem
                if(count != strlen(buf) + 1)
                {
                    logexit("send");                    
                }
                puts(buf); //printa mensagem final
                close(csock); //fecha o socket que está conectado ao cliente
            }
        }
    }
    exit(EXIT_SUCCESS); //encerra o programa
}