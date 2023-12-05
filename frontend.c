#include "frontend.h"

size_t bufsize = 50;;

int main(int argc, char const *argv[]) {
    struct stat state;
    struct utilizador user;
    struct mensagem msg;
    if (argc != 3) {
        printf("Sintaxe invalida, tente novamente\n");
    } else {
        if ((access(signalrd, F_OK)) == -1) {
            printf("Serviço temporariamente indisponível, tente novamente mais tarde\n");
            exit(10);
        }


        int wr = open(signalrd, O_RDWR);
        int rd = open(signalwr, O_RDWR);
        strcpy(user.username, argv[1]);
        char pipename[TAM];
        write(wr, &user.username, sizeof(user.username));
        read(rd, &pipename, sizeof(pipename));
        close(wr);
        close(rd);
        char rdpipe[TAM];
        char wrpipe[TAM];
        strcat(pipename, "rd");
        strcpy(rdpipe, pipename);
        strcat(pipename, "wr");
        strcpy(wrpipe, pipename);
        mkfifo(rdpipe, 0666);
        mkfifo(wrpipe, 0666);
        rd = open(rdpipe, O_RDWR);
        wr = open(wrpipe, O_RDWR);

        if (rd < 0 || wr < 0) {
            printf("Erro ao abrir pipe\n");

            unlink(rdpipe);
            unlink(wrpipe);
            exit(10);
        }

        strcpy(user.password, argv[2]);
        write(wr, &user, sizeof(user));
        read(rd, &msg, sizeof(msg));
        if (msg.valor <= 0) {
            printf("%s\n\n", msg.texto);

            close(rd);
            close(wr);
            unlink(rdpipe);
            unlink(wrpipe);
            exit(10);
        }
        printf("%s\n\n", msg.texto);

        printf("Bem vindo ao SOBay.\n");
        char string[256];

        int invalid, tam;
        do {

            printf("Que acao deseja realizar?\n");
            invalid = 0;
            fgets(string, sizeof(string), stdin);
            if ((access(rdpipe, F_OK)) == -1) {
                printf("Voce foi expulso\n");
                close(rd);
                close(wr);
                unlink(rdpipe);
                unlink(wrpipe);
                exit(10);
            }
            if ((access(signalrd, F_OK)) == -1) {
                printf("Serviço em manutenção, volte mais tarde\n");
                close(rd);
                close(wr);
                unlink(rdpipe);
                unlink(wrpipe);
                exit(10);
            }
            strcpy(msg.texto, string);
            msg.valor = 0;
            write(wr, &msg, sizeof(msg));
            read(rd, &msg, sizeof(msg));
            printf("%s\n", msg.texto);


        } while (msg.valor != -2);
        close(rd);
        close(wr);
        unlink(rdpipe);
        unlink(wrpipe);
        exit(10);
    }

    return 0;
}





