#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#define PORT 6666
#define SERVEUR_IP "127.0.0.1"

typedef struct
{
    int valNum;
    int valTete;
} Carte;

typedef struct 
{
    char pseudo[512];
    int socket;
    Carte *main;
    int nbCartes;
    int score;
} Joueur;

int sock; //socket du serveur

//variable pour indiquer si le signal a été reçu
int receivedSignal = 0;
void handleSignal(int signum);

Joueur *joueurs;
int nbJoueurs; //nb de joueurs dans la partie

Carte *cartes;
int nbCartesTotal; //nb de cartes dans la pioche

Carte plateau[4][6]; //Plateau de jeu

//connexion
void initServ();
void accepterJoueur();

//communication
void envoyerMsg(int joueur, char *msg);
void recevoirMsg(int joueur, char *msg);

//jeu
void menu();
void initManche(); //initialisation 

void redonnerCartes(int joueur); //quand le joueur n'a plus de cartes

void retirerCartesDistribuees(); //retirer carte distribué de la pioche
void retirerCartesJoueur(int joueur); //retirer carte jouée de la main du joueur

void afficherPlateau();
void afficherCartesJ(int joueur);
void afficherScore(int joueur);

int demandePoserCarte(int joueur);
int demandeRangee(int joueur);

void poserCarte(int joueur);
void majScore(int joueur);

void jouerPartie();

void quitter();

//méthode pour appeler mes scripts shell
void creerFichier(int joueur);
void fichierScore();
