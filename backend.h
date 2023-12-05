

#ifndef SOBAY_BACKEND_H
#define SOBAY_BACKEND_H
#endif //SOBAY_BACKEND_H
#include "users_lib.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include "sys/stat.h"
#include "fcntl.h"
#include "pthread.h"
#include <ctype.h>
#include "time.h"

void licat(char *categoria) ;

void listar() ;



void lival(int preco) ;

void buy(int id, int valor) ;
void printItems();

int LerItems();
char* sell(char *nome,char *cate,char *preco,char *precocj,char *duracao,char *vendedor,char *comprador);

void updateItem(int line_number, char *word, int num);