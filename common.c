#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

void logexit(const char *msg)
{
    perror(msg); //Imprime uma mensagem de erro
    exit(EXIT_FAILURE); //Encerra o programa indicando falha
}

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage)
{
    //Verifica se os parâmetros de entrada são nulos
    if(addrstr == NULL || portstr == NULL)
    {
        return -1;
    }

    //Converte a string da porta para um valor numérico
    uint16_t port = (uint16_t)atoi(portstr); //unsigned short
    if(port == 0)
    {
        return -1;
    }
    //Converte a porta para a ordem de bytes da rede
    port = htons(port); //host to network short

    struct in_addr inaddr4; //32-bit IP address
    //Tenta converter a string de endereço para um endereço IPv4
    if(inet_pton(AF_INET, addrstr, &inaddr4))
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; //128-bit IPv6 address
    //Tenta converter a string de endereço para um endereço IPv6
    if(inet_pton(AF_INET6, addrstr, &inaddr6))
    {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    //Retorna -1 se nenhuma das conversões foi bem-sucedida
    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize){
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = ""; //Buffer para armazenar o endereço IP como string
    uint16_t port;

    //Verifica se o endereço é IPv4
    if (addr->sa_family == AF_INET)
    {
        version = 4; //Define a versão do protocolo como IPv4
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;

        //Converte o endereço IPv4 de binário para string
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr, INET6_ADDRSTRLEN +1))
        {
            logexit("ntop");
        }
        //Converte a porta de ordem de bytes de rede para ordem de bytes do host
        port = ntohs(addr4->sin_port); // network to host short
    }

    //Verifica se o endereço é IPv6
    else if(addr->sa_family == AF_INET6) 
    {
        version = 6; //Define a versão do protocolo como IPv6
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr, INET6_ADDRSTRLEN +1))
        {
            logexit("ntop");
        }
        //Converte a porta de ordem de bytes de rede para ordem de bytes do host
        port = ntohs(addr6->sin6_port); // network to host short
    }
    //Caso o endereço fornecido não seja IPv4 nem IPv6
    else 
    {
        //Sai do programa com uma mensagem de erro
        logexit("unknown protocol family"); 
    }

    //Se o ponteiro para a string de saída não for nulo, formata a string de saída
    if(str)
    {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage)
{
    // Converte a string da porta para um valor numérico
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;  // Retorna -1 se a conversão da porta falhar
    }
    // Converte a porta da ordem de bytes do host para a ordem de bytes da rede
    port = htons(port); //host to network short

    // Inicializa a estrutura sockaddr_storage com zeros
    memset(storage, 0, sizeof(*storage));

    // Verifica se o protocolo é IPv4
    if (0 == strcmp(proto, "ipv4")) {
        // Configura a estrutura sockaddr_in para IPv4
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;        // Define a família de endereços como AF_INET (IPv4)
        addr4->sin_addr.s_addr = INADDR_ANY; // Aceita conexões de qualquer endereço IPv4
        addr4->sin_port = port;              // Define a porta
        return 0;  // Retorna 0 indicando sucesso
    }
    // Verifica se o protocolo é IPv6
    else if (0 == strcmp(proto, "ipv6")) {
        // Configura a estrutura sockaddr_in6 para IPv6
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;     // Define a família de endereços como AF_INET6 (IPv6)
        addr6->sin6_addr = in6addr_any;    // Aceita conexões de qualquer endereço IPv6
        addr6->sin6_port = port;           // Define a porta
        return 0;  // Retorna 0 indicando sucesso
    }
    // Se o protocolo não for nem IPv4 nem IPv6, retorna -1 indicando erro
    else 
    {
        return -1;
    }
}