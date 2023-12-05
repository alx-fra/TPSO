#include "backend.h"
#include "structs.h"

#define TAM_PALAVRA 100
#define BUFFER_SIZE 100
extern int counter;
struct utilizador usersOn[20];
struct item itemsLeilao[30];
int nitems = 0;
int id = 0;
time_t elapsed_time;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void delete_last_char(char *file_name) {

    FILE *fptr = fopen(file_name, "r+");
    if (fptr == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }


    fseek(fptr, 0, SEEK_END);


    long int file_size = ftell(fptr);


    if (file_size == 0) {
        perror("Error: cannot delete character from empty file");
        fclose(fptr);
        exit(EXIT_FAILURE);
    }


    fseek(fptr, -1, SEEK_END);


    fputc(' ', fptr);


    fclose(fptr);
}

void deleteEmptyLines(char *fileName) {
    FILE *fpRead = fopen(fileName, "r");

    if (fpRead == NULL) {
        printf("Error opening file\n");
        exit(1);
    }

    FILE *fpWrite = fopen("temp.txt", "w");

    if (fpWrite == NULL) {
        printf("Error opening temporary file\n");
        exit(1);
    }

    char buffer[1000];


    while (fgets(buffer, 1000, fpRead) != NULL) {
        if (strcmp(buffer, "\n") != 0) {
            fputs(buffer, fpWrite);
        }
    }

    fclose(fpRead);
    fclose(fpWrite);

    remove(fileName);

    rename("temp.txt", fileName);
}

void *checkitems(void *arg) {

    while (1) {

        for (int i = 0; i <= nitems - 1; i++) {

            if (itemsLeilao[i].duracao > 0) {

                if (elapsed_time >= itemsLeilao[i].duracao) {
                    if (strcmp(itemsLeilao[i].usernameCompra, "-") != 0 &&
                        strcmp(itemsLeilao[i].usernameCompra, itemsLeilao[i].usernameVende) != 0) {
                        updateItem(itemsLeilao[i].id, itemsLeilao[i].usernameCompra, 7);
                        updateUserBalance(itemsLeilao[i].usernameCompra,
                                          getUserBalance(itemsLeilao[i].usernameCompra) - itemsLeilao[i].valor);
                        saveUsersFile("utilizadores_dados.txt");
                    }
                }
            }
        }
    }
}

void *timer_thread(void *arg) {
    time_t start_time = time(NULL);

    while (1) {
        elapsed_time = time(NULL) - start_time;
        sleep(1);
    }
}

void updateItem(int line_number, char *word, int num) {
    FILE *file = fopen("itens_leilao.txt", "r+");

    char line[BUFFER_SIZE];

    int current_line = 1;

    long line_start = 0;
    while (fgets(line, BUFFER_SIZE, file)) {
        if (current_line == line_number) {
            char line_copy[BUFFER_SIZE];
            strcpy(line_copy, line);

            char *words[8];
            char *current_word = strtok(line_copy, " ");
            int i = 0;
            while (current_word) {
                words[i] = current_word;
                current_word = strtok(NULL, " ");
                i++;
            }

            char *copy = malloc(strlen(word) + 1);
            if (copy == NULL) {
                printf("Error: malloc failed\n");
                exit(1);
            }
            strcpy(copy, word);

            words[num - 1] = copy;

            char new_line[BUFFER_SIZE];
            char spaces[BUFFER_SIZE];
            char bfr[BUFFER_SIZE];
            char ch;
            strcpy(new_line, words[0]);
            for (int i = 1; i < 8; i++) {
                strcat(new_line, " ");
                strcat(new_line, words[i]);
            }


            strcpy(spaces, "");
            strcpy(bfr, "");

            /*for (int i = 0; i < strlen(word) - 4; i++) {
                strcat(spaces, "x");
            }*/


            fseek(file, line_start + strlen(line), SEEK_SET);
            do {
                ch = fgetc(file);
                sprintf(bfr, "%s%c", bfr, ch);
            } while (ch != EOF);
            bfr[strlen(bfr) - 1] = '\0';
            fseek(file, line_start + strlen(line), SEEK_SET);
            fputs(spaces, file);

            strcat(new_line, "\n");
            strcat(new_line, bfr);
            fseek(file, line_start, SEEK_SET);
            fputs(new_line, file);


            free(copy);
        }

        line_start = ftell(file);
        current_line++;

    }


    fclose(file);
    deleteEmptyLines("itens_leilao.txt");
    LerItems();
}


int criaFPROMOTERS() {

    //variavel de ambiente com o nome do ficheiro dos nomes dos promotores
    if (setenv("FPROMOTERS", "./promotores_nomes.txt", 1) == -1) {
        fprintf(stderr, "\nErro ao guardar nome como variavel de ambiente\n");
        return 1;
    }

    return 0;
}

int criaFUSERS() {
    //variavel de ambiente com o nome do ficheiro dos nomes dos utilizadores
    if (setenv("FUSERS", "utilizadores_dados.txt", 1) == -1) {
        fprintf(stderr, "\nErro ao guardar nome como variavel de ambiente\n");
        return 1;
    }

    return 0;
}

int criaFITEMS() {
    //variavel de ambiente com o nome do ficheiro dos nomes dos utilizadores
    if (setenv("FITEMS", "itens_leilao.txt", 1) == -1) {
        fprintf(stderr, "\nErro ao guardar nome como variavel de ambiente\n");
        return 1;
    }

    return 0;
}

void *exePromoters(void *arg) {
    char nomePromotor[TAM];
    char *nomeFicheiroNomesProm = getenv("FPROMOTERS");
    //obter nome do primeiro promotor
    setbuf(stdout, NULL);
    FILE *txt = fopen(nomeFicheiroNomesProm, "r");
    if (txt == NULL) {
        perror("Error in opening file");
        pthread_exit(NULL);
    }
    fscanf(txt, "%[^\n]", nomePromotor);
    fclose(txt);


    //executar o programa dos promotores
    int canal[2];
    setbuf(stdout, NULL);
    //criar pipe
    if (pipe(canal) < 0) {
        printf("\nErro na criacao do pipe anonimo\n");
        pthread_exit(NULL);
    }

    int estado;
    pid_t child_id;
    child_id = fork();
    if (child_id == -1) {
        fprintf(stderr, "\nErro no Fork\n");
    }
    if (child_id == 0) // child
    {
        close(canal[0]); // child nao le
        dup2(canal[1], 1); // redireciona stdout

        execl(nomePromotor, nomePromotor, NULL);

        fprintf(stderr, "\nExec falhou\n");
    } else {
        close(canal[1]); // pai nao le

        char bufLer[1];
        while (read(canal[0], bufLer, 1) > 0) {
            write(1, bufLer, 1); // 1 -> stdout
        }

        close(canal[0]);
        wait(&estado);
        if (WIFEXITED(estado)) {
            printf("\nPAI: O filho terminou com %d\n", WEXITSTATUS(estado));
        }
    }
    pthread_exit(NULL);
}

int LerAtualizarUsers() {

    int validacao = 0;
    char *nameFileUsers = getenv("FUSERS");
    int saldo;

    //leitura dos utilizadores2
    //int loadUsersFile(char * pathname);
    //printf("\nTeste:%s\n",nameFileUsers);
    validacao = loadUsersFile(nameFileUsers);
    if (validacao == -1) {
        printf("\nErro na leitura dos utilizadores\n");
        return 1;
    }


    int p = 0;
    int numUsersOn = 0;
    for (int h = 0; h < 20; h++) {
        if (strcmp(usersOn[h].username, " ") != 0) {
            numUsersOn++;
        }
    }
    printf("\nLidos com sucesso, %d utilizadores\n", numUsersOn);
    while (p < numUsersOn) {
        //confirmar se usersOn está valido
        validacao = isUserValid(usersOn[p].username, usersOn[p].password);
        if (validacao == 1) {
            printf("\nUtilizador %s valido\n", usersOn[p].username);
            saldo = getUserBalance(usersOn[p].username);
            saldo--;
            usersOn[p].saldo = saldo;
            printf("Saldo pos atualizacao: %i\n", saldo);
            updateUserBalance(usersOn[p].username, saldo);
        } else if (validacao == 0) {
            printf("\nUtilizador %s invalido ou com password errada\n", usersOn[p].username);
        } else if (validacao == -1) {
            printf("\nErro ao confirmar validade do ulitizador\n");
            return 1;
        }
        p++;
    }

    //Atualização dos utilizadores
    //int saveUsersFile(char * filename);
    validacao = saveUsersFile(nameFileUsers);
    if (validacao == -1) {
        printf("\nErro na gravacao dos utilizadores\n");
        return 1;
    } else if (validacao == 0)
        printf("\nDados de utilizadores gravados com sucesso\n");

    return 0;
}

int LerItems() {
    int status;
    char *nomeFicheiroItems = getenv("FITEMS");

    //printf("\nTesteNome:%s\n",nomeFicheiroItems);
    int i = 0, l = 0;
    FILE *txt = fopen(nomeFicheiroItems, "r");
    if (txt == NULL) {
        perror("Error in opening file");
        return 1;
    }
    char ch;
    while ((ch = fgetc(txt)) != EOF) {
        if (ch == '\n') {
            l++;
        }
    }
    nitems = l + 1;
    fclose(txt);
    txt = fopen(nomeFicheiroItems, "r");

    while (status != EOF && i < nitems) {
        status = fscanf(txt, "%i %s %s %i %i %i %s %s\n",
                        &itemsLeilao[i].id,
                        itemsLeilao[i].nome,
                        itemsLeilao[i].categoria,
                        &itemsLeilao[i].valor,
                        &itemsLeilao[i].valorCompreJa,
                        &itemsLeilao[i].duracao,
                        itemsLeilao[i].usernameVende,
                        itemsLeilao[i].usernameCompra);


        i++;
    }

    fclose(txt);
    //deleteEmptyLines(nomeFicheiroItems);
    return 0;
}

int kick(char *username) {
    int userid = -1;
    char clientrd[10], clientwr[10];
    for (int i = 0; i < 20; i++) {
        if (strcmp(username, usersOn[i].username) == 0) {
            userid = usersOn[i].id;
            strcpy(usersOn[i].username, " ");
            break;
        }
    }
    if (userid == -1) {
        printf("usuario nao conectado\n");
    } else {
        sprintf(clientrd, "%drd", userid);
        sprintf(clientwr, "%drdwr", userid);
        unlink(clientrd);
        unlink(clientwr);
    }
}

void printusers() {
    printf("Utilizadores:\n");
    int i = 0;
    while (strcmp(usersOn[i].username, " ") != 0) {
        printf("%s com o id %d\n", usersOn[i].username, usersOn[i].id);
        i++;
    }
}

void prom() {
    printf("Lista de Promotores\n");
}

void reprom() {
    printf("Promotores Atualizados!\n");
}


void cancel(char *string) {
    printf("Promotor %s cancelado\n", string);
}

int validacao(char **array, int tam) {
    int command = 0;
    if (strcmp(array[0], "sell") == 0) {
        if (tam != 6) {
            command = -1;
        } else {
            int length;
            for (int i = 3; i <= 5; i++) {
                length = strlen(array[i]);
                for (int j = 0; j < length; ++j) {
                    if (!isdigit(array[i][j])) {
                        command = -1;
                    }
                }
            }
            if (command != -1) {
                command = 1;
            }
        }
    } else if (strcmp(array[0], "list") == 0) {
        if (tam != 1) {
            command = -1;
        } else {
            command = 2;
        }
    } else if (strcmp(array[0], "licat") == 0) {
        if (tam != 2) {
            command = -1;
        } else {
            command = 3;
        }
    } else if (strcmp(array[0], "lisel") == 0) {
        if (tam != 2) {
            command = -1;
        } else {
            command = 4;
        }
    } else if (strcmp(array[0], "lival") == 0) {
        if (tam != 2) {
            command = -1;
        } else {
            command = 5;
        }
    } else if (strcmp(array[0], "litime") == 0) {
        if (tam != 2) {
            command = -1;
        } else {
            command = 6;
        }
    } else if (strcmp(array[0], "time") == 0) {
        if (tam != 1) {
            command = -1;
        } else {
            command = 7;
        }
    } else if (strcmp(array[0], "buy") == 0) {
        if (tam != 3) {
            command = -1;
        } else {
            command = 8;
        }
    } else if (strcmp(array[0], "cash") == 0) {
        if (tam != 1) {
            command = -1;
        } else {
            command = 9;
        }
    } else if (strcmp(array[0], "add") == 0) {
        if (tam != 2) {
            command = -1;
        } else {
            command = 10;
        }
    } else if (strcmp(array[0], "exit") == 0) {
        if (tam != 1) {
            command = 1;
        } else {
            command = -2;
        }
    } else {
        command = -1;
    }
    return command;
}

void *atende(void *arg) {
    char pipeid[10];
    sprintf(pipeid, "%d", *(int *) arg);
    char rdpipe[TAM];
    char wrpipe[TAM];
    strcat(pipeid, "rd");
    strcpy(rdpipe, pipeid);
    strcat(pipeid, "wr");
    strcpy(wrpipe, pipeid);

    int wr = open(rdpipe, O_RDWR);
    int rd = open(wrpipe, O_RDWR);

    if (wr < 0 || rd < 0) {
        perror("Error opening pipe");
        pthread_exit(NULL);
    }
    struct mensagem msg;
    struct utilizador user;
    read(rd, &user, sizeof(user));
    for (int i = 0; i < 20; ++i) {
        if (strcmp(usersOn[i].username, user.username) == 0) {
            strcpy(msg.texto, "UM USUARIO COM AS MESMAS CREDENCIAIS JA SE ENCONTRA LOGADO!");
            msg.valor = -2;
            write(wr, &msg, sizeof(msg));
            strcpy(usersOn[i].username, " ");
            pthread_exit(NULL);
        }
    }
    usersOn[id] = user;

    user.id = *((int *) arg) + 1;
    printf("A atender cliente %d, com o username %s\n", user.id, user.username);
    switch (isUserValid(user.username, user.password)) {
        case -1:
            strcpy(msg.texto, "ERRO AO VALIDAR USUARIO");
            msg.valor = -1;
            write(wr, &msg, sizeof(msg));
            strcpy(usersOn[id].username, " ");

            pthread_exit(NULL);
            break;
        case 0:
            strcpy(msg.texto, "USERNAME/PASSWORD INVALIDA");
            msg.valor = 0;
            write(wr, &msg, sizeof(msg));
            strcpy(usersOn[id].username, " ");
            pthread_exit(NULL);
            break;
        case 1:
            strcpy(msg.texto, "LOGIN EFETUADO COM SUCESSO");
            msg.valor = 1;
            write(wr, &msg, sizeof(msg));
            break;
    }
    int check;
    int tam = 0;
    char *array[7];
    user.saldo = getUserBalance(user.username);
    int x;

    do {

        ssize_t n = read(rd, &msg, sizeof(msg));
        if (n < 0) {
            perror("erro");
            exit(EXIT_FAILURE);
        }
        char str[TAM];
        strcpy(str, msg.texto);
        strtok(msg.texto, "\n");
        array[0] = strtok(msg.texto, " ");
        for (int i = 1; i < sizeof(array) / sizeof(array[0]); i++) {
            array[i] = strtok(NULL, " ");
        }
        for (int i = 0; array[i] != NULL; ++i) {
            tam = i + 1;
        }
        LerItems();


        switch (validacao(array, tam)) {
            case -2://exit
                strcpy(usersOn[id].username, " ");
                msg.valor = -2;
                strcpy(msg.texto, "Obrigado pela sua visita!\n");
                write(wr, &msg, sizeof(msg));
                saveUsersFile("utilizadores_dados.txt");
                pthread_exit(EXIT_SUCCESS);
                break;
            case -1://comando invalido
                strcpy(msg.texto, "comando invalido, tente novamente\n");
                break;
            case 1://sell
                strcpy(msg.texto, sell(array[1], array[2], array[3], array[4], array[5], user.username, "-\n"));
                break;
            case 2://list
                for (int i = 0; i < nitems; ++i) {
                    sprintf(str,
                            "%s\nId:%i\n\nNome:%s\nCategoria:%s\nValor:%i\nValorCompreJa:%i\nDuracao:%i\nUsernameVende:%s\nUsernameCompra:%s\n",
                            str,
                            itemsLeilao[i].id,
                            itemsLeilao[i].nome,
                            itemsLeilao[i].categoria,
                            itemsLeilao[i].valor,
                            itemsLeilao[i].valorCompreJa,
                            itemsLeilao[i].duracao,
                            itemsLeilao[i].usernameVende,
                            itemsLeilao[i].usernameCompra);
                }
                strcpy(msg.texto, str);

                break;
            case 3://licat

                for (int i = 0; i < nitems; ++i) {
                    if (strcmp(itemsLeilao[i].categoria, array[1]) == 0) {
                        sprintf(str,
                                "%s\nId:%i\n\nNome:%s\nCategoria:%s\nValor:%i\nValorCompreJa:%i\nDuracao:%i\nUsernameVende:%s\nUsernameCompra:%s\n",
                                str,
                                itemsLeilao[i].id,
                                itemsLeilao[i].nome,
                                itemsLeilao[i].categoria,
                                itemsLeilao[i].valor,
                                itemsLeilao[i].valorCompreJa,
                                itemsLeilao[i].duracao,
                                itemsLeilao[i].usernameVende,
                                itemsLeilao[i].usernameCompra);
                        strcpy(array[1], itemsLeilao[i].categoria);
                    }
                }
                strcpy(msg.texto, str);
                break;
            case 4://lisel

                for (int i = 0; i < nitems; ++i) {
                    if (strcmp(itemsLeilao[i].usernameVende, array[1]) == 0) {
                        sprintf(str,
                                "%s\nId:%i\n\nNome:%s\nCategoria:%s\nValor:%i\nValorCompreJa:%i\nDuracao:%i\nUsernameVende:%s\nUsernameCompra:%s\n",
                                str,
                                itemsLeilao[i].id,
                                itemsLeilao[i].nome,
                                itemsLeilao[i].categoria,
                                itemsLeilao[i].valor,
                                itemsLeilao[i].valorCompreJa,
                                itemsLeilao[i].duracao,
                                itemsLeilao[i].usernameVende,
                                itemsLeilao[i].usernameCompra);
                        strcpy(array[1], itemsLeilao[i].usernameVende);
                    }
                }
                strcpy(msg.texto, str);
                break;
            case 5://lival


                for (int i = 0; i < nitems; ++i) {
                    if (itemsLeilao[i].valor <= atoi(array[1])) {
                        sprintf(str,
                                "%s\nId:%i\n\nNome:%s\nCategoria:%s\nValor:%i\nValorCompreJa:%i\nDuracao:%i\nUsernameVende:%s\nUsernameCompra:%s\n",
                                str,
                                itemsLeilao[i].id,
                                itemsLeilao[i].nome,
                                itemsLeilao[i].categoria,
                                itemsLeilao[i].valor,
                                itemsLeilao[i].valorCompreJa,
                                itemsLeilao[i].duracao,
                                itemsLeilao[i].usernameVende,
                                itemsLeilao[i].usernameCompra);
                    }
                }
                strcpy(msg.texto, str);
                break;
            case 6://litime


                for (int i = 0; i < nitems; ++i) {
                    if (itemsLeilao[i].duracao <= atoi(array[1])) {
                        sprintf(str,
                                "%s\nId:%i\n\nNome:%s\nCategoria:%s\nValor:%i\nValorCompreJa:%i\nDuracao:%i\nUsernameVende:%s\nUsernameCompra:%s\n",
                                str,
                                itemsLeilao[i].id,
                                itemsLeilao[i].nome,
                                itemsLeilao[i].categoria,
                                itemsLeilao[i].valor,
                                itemsLeilao[i].valorCompreJa,
                                itemsLeilao[i].duracao,
                                itemsLeilao[i].usernameVende,
                                itemsLeilao[i].usernameCompra);
                    }
                }
                strcpy(msg.texto, str);
                break;
            case 7:
                sprintf(msg.texto, "tempo: %d", elapsed_time);
                break;
            case 8:


                if (strcmp(user.username, itemsLeilao[atoi(array[1]) - 1].usernameVende) == 0) {
                    strcpy(msg.texto, "Este item já te pertence\n");
                } else if (atoi(array[2]) > user.saldo) {
                    strcpy(msg.texto, "Saldo Insuficiente :(\n");

                } else if (strcmp(itemsLeilao[atoi(array[1]) - 1].usernameCompra,
                                  itemsLeilao[atoi(array[1]) - 1].usernameVende) == 0 ||
                           itemsLeilao[atoi(array[1]) - 1].duracao < elapsed_time) {
                    strcpy(msg.texto, "Este item nao esta a venda:(\n");

                } else {
                    if (atoi(array[2]) <= itemsLeilao[atoi(array[1]) - 1].valor &&
                        strcmp(itemsLeilao[atoi(array[1]) - 1].usernameCompra,
                               "-") != 0) {
                        strcpy(msg.texto, "Licitacao muito baixa\n");
                    } else if (atoi(array[2]) >= itemsLeilao[atoi(array[1]) - 1].valorCompreJa) {
                        updateItem(atoi(array[1]), user.username, 7);
                        updateItem(atoi(array[1]), user.username, 8);
                        strcpy(msg.texto, "Item adquirido\n");
                        x = updateUserBalance(user.username, user.saldo - atoi(array[2]));
                        printf("x:%d\n", x);
                        sprintf(msg.texto, "Saldo atualizado com sucesso\nNovo saldo:%d",
                                getUserBalance(user.username));

                    } else {
                        updateItem(atoi(array[1]), array[2], 4);
                        updateItem(atoi(array[1]), user.username, 8);
                        strcpy(msg.texto, "Licitacao feita\n");

                    }

                }

                break;
            case 9://cash
                sprintf(msg.texto, "O seu saldo:%d \n", getUserBalance(user.username));
                break;
            case 10://add

                x = updateUserBalance(user.username, user.saldo + atoi(array[1]));
                sprintf(msg.texto, "Saldo atualizado com sucesso\nNovo saldo:%d", getUserBalance(user.username));
                break;

        };
        saveUsersFile("utilizadores_dados.txt");
        write(wr, &msg, sizeof(msg));
        strcpy(msg.texto, " ");
        strcpy(str, " ");

    } while (1);
}


int main(int argc, char const *argv[]) {
    loadUsersFile("utilizadores_dados.txt");
    int n = 0;
    for (int i = 0; i < 20; i++) {
        strcpy(usersOn[i].username, " ");
    }
    pthread_t thread[20];
    pthread_t timethread, checkitemsthrd, promotores;

    if (criaFPROMOTERS() == 1)
        printf("\nErro ao armazenar varivel de ambiente \"FPROMOTERS\"\n");
    if (criaFUSERS() == 1)
        printf("\nErro ao armazenar varivel de ambiente \"FUSERS\"\n");
    if (criaFITEMS() == 1)
        printf("\nErro ao armazenar varivel de ambiente \"FUSERS\"\n");


    if (access(signalrd, F_OK) != 0) {
        mkfifo(signalrd, 0666);
    }
    if (access(signalwr, F_OK) != 0) {
        mkfifo(signalwr, 0666);
    }
    setbuf(stdout, NULL);
    LerItems();
    pthread_create(&timethread, NULL, timer_thread, NULL);

    pthread_create(&checkitemsthrd, NULL, checkitems, NULL);

    fd_set readset;
    printf("\nIndique qual das funcionalidades deseja testar:\n1 -> Execução do promotor\n2 -> Leitura dos utilizadores\n3 -> visualizar itens\n");
    int rd = open(signalrd, O_RDWR);
    int wr = open(signalwr, O_RDWR);

    do {
        FD_ZERO(&readset);
        FD_SET(rd, &readset);
        FD_SET(0, &readset);


        int escolha = 0;
        int result = select(rd + 1, &readset, NULL, NULL, NULL);
        if (result > 0) {
            if (FD_ISSET(rd, &readset)) {

                char strpipe[TAM];

                read(rd, &strpipe, sizeof(strpipe));
                sprintf(strpipe, "%d", n);

                write(wr, &strpipe, sizeof(strpipe));
                for (id = 0; id < 20; id++) {
                    if (strcmp(usersOn[id].username, " ") == 0) {
                        break;
                    }
                }
                int *arg = malloc(sizeof(n));
                *arg = n;
                pthread_create(&thread[n], NULL, atende, arg);
                n++;
            }

        }

        if (FD_ISSET(fileno(stdin), &readset)) {
            char string[120], *array[3];
            int tam, invalid = 0;

            fgets(string, 120, stdin);
            strtok(string, "\n");
            array[0] = strtok(string, " ");

            for (int i = 1; i < sizeof(array) / sizeof(array[0]); i++) {
                array[i] = strtok(NULL, " ");
            }
            for (int i = 0; array[i] != NULL; ++i) {
                tam = i + 1;
            }
            if (strcmp(array[0], "close") == 0) {
                if (tam != 1) {
                    invalid = 1;
                } else {
                    printf("adeus\n");
                    close(rd);
                    close(wr);
                    unlink(signalrd);
                    unlink(signalwr);

                    return 0;
                }
            } else if (strcmp(array[0], "users") == 0) {
                if (tam != 1) {
                    invalid = 1;
                } else {
                    printusers();
                }
            } else if (strcmp(array[0], "prom") == 0) {
                if (tam != 1) {
                    invalid = 1;
                } else {
                    prom();
                }
            } else if (strcmp(array[0], "reprom") == 0) {
                if (tam != 1) {
                    invalid = 1;
                } else {
                    reprom();
                }
            } else if (strcmp(array[0], "kick") == 0) {
                if (tam != 2) {
                    invalid = 1;
                } else {
                    kick(array[1]);
                }
            } else if (strcmp(array[0], "cancel") == 0) {
                if (tam != 2) {
                    invalid = 1;
                } else {
                    cancel(array[1]);

                }
            } else if (strcmp(array[0], "1") == 0) {
                pthread_create(&promotores, NULL, exePromoters, NULL);

            } else if (strcmp(array[0], "2") == 0) {
                if (LerAtualizarUsers() == 1) {
                    printf("\nErro ao ler e atualizar utilizadores\n");
                    invalid = 1;

                }
            } else if (strcmp(array[0], "3") == 0) {
                if (LerItems() == 1) {
                    printf("\nErro ao ler e atualizar itens\n");
                    invalid = 1;
                } else {
                    printItems();
                }
            } else {
                invalid = 1;
            }
            if (invalid == 1) {
                printf("comando invalido, tente novamente\n");
            }
            printf("\nIndique qual das funcionalidades deseja testar:\n1 -> Execução do promotor\n2 -> Leitura dos utilizadores\n3 -> visualizar itens\n");

        }

    } while (1);

}


void listar() {
    printf("LISTADO!\n");
}


char *sell(char *nome, char *cate, char *preco, char *precocj, char *duracao, char *vendedor, char *comprador) {
    char *strreturn = malloc(50);
    char ch;
    char str2[TAM];
    int linhas = 1;

    FILE *fp;

    fp = fopen("itens_leilao.txt", "r");

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            linhas++;
        }
    }
    itemsLeilao[nitems].id = linhas;
    itemsLeilao[nitems].duracao = atoi(duracao);
    itemsLeilao[nitems].valor = atoi(preco);
    itemsLeilao[nitems].valorCompreJa = atoi(precocj);
    strcpy(itemsLeilao[nitems].categoria, cate);
    strcpy(itemsLeilao[nitems].nome, nome);
    strcpy(itemsLeilao[nitems].usernameCompra, "-");
    strcpy(itemsLeilao[nitems].usernameVende, vendedor);
    nitems = linhas;
    nitems++;


    sprintf(str2, "\n%d %s %s %s %s %s %s %s", nitems, nome, cate, preco, precocj, duracao, vendedor, comprador);
    fclose(fp);
    fp = fopen("itens_leilao.txt", "a");
    fprintf(fp, "%s", str2);
    fclose(fp);
    sprintf(strreturn, "O seu item foi registado com sucesso com o id %d", nitems);
    // deleteEmptyLines("itens_leilao.txt");
    delete_last_char("itens_leilao.txt");
    return strreturn;
}

void printItems() {
    for (int i = 0; i < nitems; ++i) {
        printf("\nId:%i\n\nNome:%s\nCategoria:%s\nValor:%i\nValorCompreJa:%i\nDuracao:%i\nUsernameVende:%s\nUsernameCompra:%s\n",
               itemsLeilao[i].id,
               itemsLeilao[i].nome,
               itemsLeilao[i].categoria,
               itemsLeilao[i].valor,
               itemsLeilao[i].valorCompreJa,
               itemsLeilao[i].duracao,
               itemsLeilao[i].usernameVende,
               itemsLeilao[i].usernameCompra);
    }
    printf("nitems:%d\n", nitems);
}

/*
- Leitura do ficheiro dos itens à venda. Os dados serão lidos e interpretados. Não se trata de apenas ler e
imprimir um ficheiro de texto - os valores inteiros serão armazenados e mostrados como inteiros.
*/
