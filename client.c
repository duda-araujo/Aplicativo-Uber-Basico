#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#include <sys/types.h>
#include <sys/socket.h>

void usage(int argc, char **argv) 
{
    printf("usage: %s <ipv4|ipv6> <server IP> <server port>\n", argv[0]);
    printf("example: %s ipv4 127.0.0.1 50501\n", argv[0]);
    exit(EXIT_FAILURE);
}

#define BUFSZ 1024

typedef struct {
    double latitude;
    double longitude;
} Coordinate;

int main(int argc, char **argv)
{
    // Criando coordenada do cliente
    Coordinate coordCli = {-19.9218, -43.9366};

    if(argc < 4) //se foram passados menos parâmetros do que o necessário
    { 
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    //utiliza dos parâmetros passados na linha de comando para analisar o endereço
    //e preencher a estrutura sockaddr_storage
    if(0 != addrparse(argv[2], argv[3], &storage))
    {
        usage(argc, argv);
    }

    int aux_conectado = 1;
    while (aux_conectado)
    {
        int s;
        s = socket(storage.ss_family, SOCK_STREAM, 0); //cria um socket para ponto de comunicação
        if(s == -1)
        {
            logexit("socket");
        }

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        printf("0 - Sair\n1 - Solicitar Corrida\n");
        fgets(buf, BUFSZ-1, stdin);

        if(buf[0] == '0') //cliente aperta em sair
        {
            aux_conectado = 0;
            break;
        }
        else if(buf[0] == '1') //conectar e solicitar corrida
        {
            struct sockaddr *addr = (struct sockaddr *)(&storage);
            if(0 != connect(s, addr, sizeof(storage))) //inicia a conexão ao socket criado
            {
                logexit("connect");
            }

            char addrstr[BUFSZ];
            addrtostr(addr, addrstr, BUFSZ); //converte o endereço em uma string com a versão, endereço e porta

            //avisar para o servidor que o cliente solicitou corrida
            int count = send(s, buf, strlen(buf)+1, 0); //envia mensagem para o outro socket
            if(count != strlen(buf)+1)
            {
                logexit("send");
            }

            //receber um ok de pode enviar outra mensagem
            count = recv(s, buf, BUFSZ, 0); //recebe mensagem do outro socket
            if(count == 0)
            {
                break; //Termina conexão
            }

            if(buf[0] == '1') //se servidor ok
            {
                //enviar coordenadas do cliente para o servidor
                count = send(s, &coordCli, sizeof(Coordinate)+1, 0);
                if(count != sizeof(Coordinate)+1){
                    logexit("send");
                }
                
                memset(buf, 0, BUFSZ);
                char primeira_mensagem = '1';

                while (1)
                {
                    //recebendo mensagens do servidor em loop
                    count = recv(s, buf, BUFSZ, 0);
                    if(count == 0)
                    {
                        break;
                    }

                    if (primeira_mensagem == '1')
                    {
                        if(buf[0] != '1') //servidor recusou a corrida
                        {
                            puts(buf); //mostrar que não encontrou motorista
                            close(s); 
                            break;
                        }
                        //se o servidor aceitar a corrida, não precisa printar o 1
                        primeira_mensagem = '0';
                    }
                    else
                    {
                        //printando mensagens recebidas do servidor depois de aceitar
                        puts(buf);
                        
                        //se chegar a mensagem final
                        if(buf[0] == 'O')
                        {
                            aux_conectado = 0; //corrida efetuada, podemos sair do loop e fechar a conexão
                            break;
                        }
                    }
                }
            }
            close(s); //fecha o socket
        }
    }   
}