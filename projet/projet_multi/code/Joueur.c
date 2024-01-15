#include "GestionJeu.h"

int clientSocket;
bool clientCo;
char msg[512];

int receivedSignalClient = 0;

void handleSignalClient(int signum)
{
    receivedSignalClient = signum;
    printf("Signal reçu !\n");
    close(clientSocket);
    clientCo = false;
    exit(0);
}

void initClient()
{
    clientCo = false;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        perror("Erreur : création socket");
        exit(1);
    }

    struct sockaddr_in cliAddr;
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_port = htons(PORT);

    //Convertir l'adresse IP de format texte en format binaire
    if (inet_pton(AF_INET, SERVEUR_IP, &cliAddr.sin_addr) <= 0) {
        perror("Adresse invalide");
        exit(EXIT_FAILURE);
    }

    while (!clientCo)
    {
        if (connect(clientSocket, (struct sockaddr*)&cliAddr, sizeof(cliAddr)) == -1)
        {
            perror("Erreur : connexion client");
            sleep(2);
        } else clientCo = true;
    }

    //envoi du pseudo
    printf("Pseudo : ");
    memset(msg, 0, sizeof(msg));
    scanf("%s", msg);
    send(clientSocket, msg, 25 * sizeof(char), 0);

    printf("%s, vous êtes connecté au serveur !\n", msg);
    clientCo = true;
}

int main()
{
    initClient();

    signal(SIGINT, handleSignalClient);  //Ctrl+C
    signal(SIGTERM, handleSignalClient); //Signal de terminaison

    while (clientCo)
    {  
        //printf("En attente d'ordre...\n");
        //en attente d'ordre
        memset(msg, 0, sizeof(msg));
        recv(clientSocket, msg, sizeof(msg), 0);
        
        if (strcmp(msg, "afficher") == 0)
        {
            memset(msg, 0, 512);
            recv(clientSocket, msg, 512, 0);
            printf("%s\n", msg);
        } 
        else if (strcmp(msg, "saisir") == 0)
        {
            memset(msg, 0, 512);
            scanf("%s", msg);
            send(clientSocket, msg, 512 * sizeof(char), 0);
        }
        else if (strcmp(msg, "quitter") == 0)
        {
            clientCo = false;
            close(clientSocket);
        }
    }

    close (clientSocket);
    
    return 0;
}