#pragma once

#include <stdlib.h>
#include <arpa/inet.h>

void logexit(const char *msg);
//mostra mensagem de erro e encerra o programa

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage);
/*
analisa uma string de endereço e uma string de porta,
e preenche a estrutura sockaddr_storage com as informações apropriadas,
sejam elas para um endereço IPv4 ou IPv6
*/

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);
/*
converte uma estrutura sockaddr (que pode representar um endereço IPv4 ou IPv6)
em uma string legível que inclui a versão do protocolo (IPv4 ou IPv6),
o endereço IP e o número da porta
*/

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage);
/*
responsável por inicializar a estrutura sockaddr_storage com as informações necessárias
para configurar um socket de servidor, seja ele utilizando o protocolo IPv4 ou IPv6. 
*/