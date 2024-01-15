#include <stdbool.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "GestionJeu.h"

char pseudo[25];
char msg[512];
char pseudoRobot[25][20];
int receivedSignalRobot = 0;

int robotSocket;
bool robotCo;

void handleSignalRobot(int signum)
{
    receivedSignalRobot = signum;
    printf("Signal reçu !\n");
    close(robotSocket);
    robotCo = false;
    exit(0);
}

void pseudoAleatoire()
{
    srand((unsigned int)time(NULL));
    strcpy(pseudo, pseudoRobot[rand() % 25]);
}

//réponse aléatoire (LE ROBOT N'EST PAS INTELLIGENT)
int reponseAleatoire()
{
    int rep = rand() % 10 + 1;
    return rep;
}

void initRobot()
{
    robotCo = false;
    
    robotSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (robotSocket == -1)
    {
        perror("Erreur : création socket");
        exit(1);
    }

    struct sockaddr_in robotAddr;
    robotAddr.sin_family = AF_INET;
    robotAddr.sin_port = htons(PORT);

    //Convertir l'adresse IP de format texte en format binaire
    if (inet_pton(AF_INET, SERVEUR_IP, &robotAddr.sin_addr) <= 0) {
        perror("Adresse invalide");
        exit(EXIT_FAILURE);
    }

    while (!robotCo)
    {
        if (connect(robotSocket, (struct sockaddr*)&robotAddr, sizeof(robotAddr)) == -1)
        {
            perror("Erreur : connexion client");
            sleep(2);
        } else robotCo = true;
    }

    memset(msg, 0, sizeof(msg));
    //choix aléatoire du pseudo
    pseudoAleatoire();
    printf("Pseudo choisi au hasard : %s\n", pseudo);
    send(robotSocket, &pseudo, 25*sizeof(char), 0);

    printf("%s, est connecté au serveur !\n", pseudo);
    robotCo = true;
}

int main()
{
    //initialisation des pseudos de robot
    strcpy(pseudoRobot[0], "ronaldo_bot");
    strcpy(pseudoRobot[1], "messi_bot");
    strcpy(pseudoRobot[2], "neymar_bot");
    strcpy(pseudoRobot[3], "mbappe_bot");
    strcpy(pseudoRobot[4], "haaland_bot");
    strcpy(pseudoRobot[6], "ozil_bot");
    strcpy(pseudoRobot[7], "mahrez_bot");
    strcpy(pseudoRobot[8], "benzema_bot");
    strcpy(pseudoRobot[5], "modric_bot");
    strcpy(pseudoRobot[9], "salah_bot");
    strcpy(pseudoRobot[10], "kane_bot");
    strcpy(pseudoRobot[11], "lewandowski_bot");
    strcpy(pseudoRobot[12], "debruyne_bot");
    strcpy(pseudoRobot[13], "griezmann_bot");
    strcpy(pseudoRobot[14], "aguero_bot");
    strcpy(pseudoRobot[15], "vardy_bot");
    strcpy(pseudoRobot[16], "kroos_bot");
    strcpy(pseudoRobot[17], "ramos_bot");
    strcpy(pseudoRobot[18], "sterling_bot");
    strcpy(pseudoRobot[19], "alaba_bot");
    strcpy(pseudoRobot[20], "courtois_bot");
    strcpy(pseudoRobot[21], "suarez_bot");
    strcpy(pseudoRobot[22], "firmino_bot");
    strcpy(pseudoRobot[23], "silva_bot");
    strcpy(pseudoRobot[24], "casemiro_bot");    

    initRobot();

    signal(SIGINT, handleSignalRobot);
    signal(SIGTERM, handleSignalRobot);

    while (robotCo)
    {
        //en attente d'ordre

        memset(msg, 0, sizeof(msg));
        recv(robotSocket, msg, sizeof(msg), 0);

        if (strcmp(msg, "afficher") == 0)
        {
            memset(msg, 0, 512);
            recv(robotSocket, msg, 512, 0);
            printf("%s\n", msg);
        } 
        else if (strcmp(msg, "saisir") == 0)
        {
            memset(msg, 0, 512);
            int rep = reponseAleatoire();
            snprintf(msg, 521, "%d", rep);
            send(robotSocket, msg, 512 * sizeof(char), 0);
            printf("\n%s a répondu : %d\n", pseudo, rep);
        }
        else if (strcmp(msg, "quitter") == 0)
        {
            robotSocket = false;
            close(robotSocket);
            exit(0);
        }
    }

    close(robotSocket);

    return 0;
}