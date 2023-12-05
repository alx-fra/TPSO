//
// Created by charles on 16-11-2022.
//

#ifndef SOBAY_STRUCTS_H
#define SOBAY_STRUCTS_H
#define TAM 1024
#define TAMMSG 2046
#endif //SOBAY_STRUCTS_H

#include "users_lib.h"

char *signalrd = "signalrd";
char *signalwr = "signalwr";
struct utilizador{
    char username[TAM],password[TAM];
    int saldo,id;
};


struct item{
    int id;
    char nome[TAM];
    char categoria[TAM];
    int valor;
    int valorCompreJa;
    int duracao;
    char usernameVende[TAM];
    char usernameCompra[TAM];
};
struct promotor{
    char categoria[20];
    int desconto,duracao;

};

struct mensagem{
    char texto[TAMMSG];
    int valor;
};