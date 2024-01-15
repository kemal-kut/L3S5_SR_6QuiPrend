#include "GestionJeu.h"

//gestion des signaux
void handleSignal(int signum)
{
    receivedSignal = signum;
    //fermeture du socket serveur
    close(sock);
    //affichage d'un msg 
    printf("\nSignal reçu !\n");
    //le programme quitte proprement
    quitter();
    exit(0);
}

//initialisation du serveur
void initServ()
{
    //à l'initialisation le nb de joueurs = 0
    nbJoueurs = 0;

    //création socket type tcp
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Erreur : création socket");
        exit(1);
    }

    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = inet_addr(SERVEUR_IP);

    //liaison du socket à l'adresse et au port
    int liaison = bind(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (liaison == -1)
    {
        perror("Erreur : liaison");
        exit(1);
    }

    //allocation dynamique du tableau joueurs, ici nbJoueurs = 0
    joueurs = malloc(nbJoueurs * sizeof(Joueur));

    //affichage msg serveur démarre
    printf("Démarrage du serveur...\n");

    //attente de 3 sec (pour bien comprendre le programme lors de l'exécution)
    sleep(3);
}

//accepter connexion d'un client
void accepterJoueur()
{
    listen(sock, 1);

    struct sockaddr_in cliAddr;
    socklen_t cliAddrLen = sizeof(cliAddr);

    //accepte la connexion et crée un socket pour la communication avec le client
    int client = accept(sock, (struct sockaddr*)&cliAddr, &cliAddrLen);
    if (client == -1)
    {
        perror("Erreur : connexion client");
        exit(1);
    }

    //nouvelle connexion donc incrémentation du nombre de joueurs
    //et réallocation du tableau de joueurs
    nbJoueurs++;
    joueurs = realloc(joueurs, nbJoueurs * sizeof(Joueur));

    //stockage des infos du nouveau joueur dans le tableau
    joueurs[nbJoueurs - 1].socket = client;
    joueurs[nbJoueurs - 1].nbCartes = 0;

    //récéption du pseudo
    char msg[512];
    memset(msg, 0, sizeof(msg));
    recv(joueurs[nbJoueurs - 1].socket, msg, sizeof(msg), 0);
    strcpy(joueurs[nbJoueurs - 1].pseudo, msg);

    //creerFichier(nbJoueurs - 1);

    //affiche msg nouvelle connexion
    printf("%s s'est connecté !\n", joueurs[nbJoueurs - 1].pseudo);

    //pause de 2 sec
    sleep(2);
}

//envoyer un msg à un joueur spécifié
void envoyerMsg(int numJoueur, char *message)
{
    char msg[512]; //buffer pour stocker le msg

    //Envoi de l'ordre d'affichage
    snprintf(msg, sizeof(msg), "afficher");
    send(joueurs[numJoueur].socket, msg, sizeof(msg), 0);

    //Envoi du message
    snprintf(msg, sizeof(msg), "%s", message);
    send(joueurs[numJoueur].socket, msg, sizeof(msg), 0);
}

//recevoir un msg d'un joueur spécifié
void recevoirMsg(int joueur, char *msg)
{
    char message[512]; //buffer pour stocker le msg 
    //envoi de l'ordre de saisie
    memset(message, 0, sizeof(message));
    strcat(message, "saisir");
    send(joueurs[joueur].socket, message, 512 * sizeof(char), 0);

    //réception du message
    memset(message, 0, sizeof(message));
    recv(joueurs[joueur].socket, message, sizeof(message), 0);

    strcpy(msg, message); //copie du msg reçu dans la variable globale msg
}

//crée un fichier pour le joueur avec le script shell pour enregistrer sa connexion
void creerFichier(int joueur)
{
    char nomFichier[512]; //buffer pour stocker le nom
    strcpy(nomFichier, joueurs[joueur].pseudo); //copie du pseudo dans le buffer
    char cmd[100];//buffer pour stocker la commande shell
    snprintf(cmd, sizeof(cmd), "./../code/fichierJoueur.sh %s", nomFichier);//construction de la commande shell

    if (system(cmd) == -1)
    {
        perror("Erreur lors de la création du fichier");
        exit(1);
    }
}

//retirer les cartes distribuées du tableau de cartes
void retirerCartesDistribuees()
{
    //allocation d'un tableau temporaire pour trier les cartes non distribuées
    Carte *tab = malloc(nbCartesTotal * sizeof(Carte)); 
    int cptCartes = 0; //cpt de cartes non distribuées

    //parcours du tableau de cartes initial
    for (int i = 0; i < nbCartesTotal; i++)
    {
        if (cartes[i].valNum != 0) //Les cartes avec une valeur différente de 0 sont considérées comme non distribuées
        {
            //les cartes != 0 sont les cartes non distribuées
            //stockage des cartes non distribuées dans le tableau temporaire
            tab[cptCartes].valNum = cartes[i].valNum;
            tab[cptCartes].valTete = cartes[i].valTete;
            cptCartes++;
        }
    }
    //on vide notre tableau initial
    free(cartes);

    nbCartesTotal = cptCartes; //MAJ du nombre de total de cartes
    cartes = malloc(nbCartesTotal * sizeof(Carte)); //allocation d'un nouveau tableau de cartes avec la nouvelle taille

    //Remise des cartes non distribuées dans le tableau initial    
    for (int i = 0; i < nbCartesTotal; i++)
    {
        cartes[i].valNum = tab[i].valNum;
        cartes[i].valTete = tab[i].valTete;
    }
    free(tab); //plus besoin de ce tab : Libération de la mémoire du tableau temporaire
}

//même principe que retirerCartesDistribuees mais cette fois c'est pour les cartes du joueur (pour les cartes qu'il a posé (joué))
void retirerCartesJoueur(int numJoueur)
{
    //Vérification que le numéro de joueur est valide
    if (numJoueur < 0 || numJoueur >= nbJoueurs)
    {
        fprintf(stderr, "Erreur : Le numéro de joueur est invalide.\n");
        return;
    }

    Carte *tab = malloc(joueurs[numJoueur].nbCartes * sizeof(Carte));

    int cpt = 0;

    for (int i = 0; i < joueurs[numJoueur].nbCartes; i++)
    {
        if (joueurs[numJoueur].main[i].valNum != 0 && joueurs[numJoueur].main[i].valTete != 0)
        {
            tab[cpt].valNum = joueurs[numJoueur].main[i].valNum;
            tab[cpt].valTete = joueurs[numJoueur].main[i].valTete;
            cpt++;
        }
    }

    //Déplacement des cartes dans la main pour remplir le trou
    for (int i = 0; i < cpt; i++)
    {
        joueurs[numJoueur].main[i].valNum = tab[i].valNum;
        joueurs[numJoueur].main[i].valTete = tab[i].valTete;
    }

    //Réduction de la taille de la main
    joueurs[numJoueur].nbCartes = cpt;
    joueurs[numJoueur].main = realloc(joueurs[numJoueur].main, joueurs[numJoueur].nbCartes * sizeof(Carte));

    // Mise à zéro de la dernière carte (évitons de laisser des valeurs résiduelles)
    if (cpt < joueurs[numJoueur].nbCartes)
    {
        joueurs[numJoueur].main[joueurs[numJoueur].nbCartes].valNum = 0;
        joueurs[numJoueur].main[joueurs[numJoueur].nbCartes].valTete = 0;
    }

    majScore(numJoueur);

    free(tab);
}

//initialisation d'une nouvelle manche
void initManche()
{
    //initialisation des cartes
    cartes = malloc(104 * sizeof(Carte));
    if (cartes == NULL) {
        perror("Erreur : allocation de mémoire pour les cartes");
        exit(1);
    }
    nbCartesTotal = 0;

    for (int i = 1; i < 105; i++)
    {
        cartes[i - 1].valNum = i;
        cartes[i - 1].valTete = (rand() % 7) + 1; //tête de boeuf au hasard entre 1 et 7
        nbCartesTotal++;
    }

    int x; //valeur au hasard (pour tirer une carte aléatoirement)

    //initialisation du plateau
    for (int i = 0; i < 4; i++)
    {
        x = rand() % nbCartesTotal;
        while (cartes[x].valNum == 0 && cartes[x].valTete == 0) x = rand() % nbCartesTotal;
        
        for (int j = 0; j < 6; j++)
        {
            plateau[i][j].valNum = 0;
            plateau[i][j].valTete = 0;

            if (cartes[x].valNum != 0 && cartes[x].valTete != 0)
            {
                plateau[i][0].valNum = cartes[x].valNum;
                plateau[i][0].valTete = cartes[x].valTete;
            }
        }
        cartes[x].valNum = 0;
        cartes[x].valTete = 0;
        retirerCartesDistribuees();
    }

    //initialisation des cartes des joueurs
    for (int i = 0; i < nbJoueurs; i++)
    {
        joueurs[i].nbCartes = 10;
        joueurs[i].main = malloc(joueurs[i].nbCartes * sizeof(Carte));

        for (int j = 0; j < 10; j++)
        {
            x = rand() % nbCartesTotal;
            while (cartes[x].valNum == 0 && cartes[x].valTete == 0) x = rand() % 104;
            joueurs[i].main[j].valNum = 0;
            joueurs[i].main[j].valTete = 0;

            if (cartes[x].valNum != 0 && cartes[x].valTete != 0)
            {
                joueurs[i].main[j].valNum = cartes[x].valNum;
                joueurs[i].main[j].valTete = cartes[x].valTete;
            }
            cartes[x].valNum = 0;
            cartes[x].valTete = 0;
            retirerCartesDistribuees();
        }
        majScore(i);
    }
    retirerCartesDistribuees(); //pas obligé d'appeler une 3ème fois la méthode ici
}

//envoi du plateau aux joueurs
void afficherPlateau()
{
    for (int i = 0; i < nbJoueurs; i++) envoyerMsg(i, "\n PLATEAU DE JEU \n");
    //Parcours des lignes du plateau
    for (int i = 0; i < 4; i++)
    {
        //Construction du message pour la ligne
        char msg[512];
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), " %d - ", i + 1);

        //Parcours des colonnes du plateau
        for (int j = 0; j < 6; j++)
        {
            //Vérification si la carte est présente sur le plateau
            if (plateau[i][j].valNum > 0 && plateau[i][j].valTete > 0)
            {
                char strCarte[50];
                snprintf(strCarte, sizeof(strCarte), "<%d - %d> ", plateau[i][j].valNum, plateau[i][j].valTete);
                strcat(msg, strCarte);
            }
        }

        //Envoi du message de la ligne à tous les joueurs
        for (int k = 0; k < nbJoueurs; k++)
        {
            envoyerMsg(k, msg);
        }
    }
}

//envoi du paquet de carte au joueur spécifié (à qui c'est le tour de jouer) 
void afficherCartesJ(int joueur)
{
    envoyerMsg(joueur, "\n Vos cartes :");
    char msg[512];
    memset(msg, 0, sizeof(msg));

    for (int i = 0; i < joueurs[joueur].nbCartes; i++)
    {
        if (joueurs[joueur].main[i].valNum != 0 && joueurs[joueur].main[i].valTete != 0)
        {
            char strCarte[50];
            snprintf(strCarte, sizeof(strCarte), " %d.[%d - %d]", i + 1, joueurs[joueur].main[i].valNum, joueurs[joueur].main[i].valTete);
            strcat(msg, strCarte);
        }
    }
    envoyerMsg(joueur, msg);
}

//envoi du score au joueur à c'est le tour de jouer 
void afficherScore(int joueur)
{
    char msg[512];
    memset(msg, 0, sizeof(msg));
    snprintf(msg, sizeof(msg), "\n Votre score : %d ", joueurs[joueur].score);
    envoyerMsg(joueur, msg);
}

//envoi de la demande de chosir la carte à poser au joueur à c'est le tour de jouer
int demandePoserCarte(int joueur) {
    int carte;

    do {
        char msg[200];
        memset(msg, 0, sizeof(msg));
        envoyerMsg(joueur, "\n SAISIR N° CARTE À JOUER\n");

        //Recevoir le message (la carte à jouer entre 1 et le nb total de cartes qu'il a)
        recevoirMsg(joueur, msg);
        
        carte = (int)strtol(msg, NULL, 10);

        //si l'entier est entre 1 et nb total de cartes du joueur
        if (carte < 1 || carte > joueurs[joueur].nbCartes) {
            fprintf(stderr, "Erreur : Le numéro de carte doit être entre 1 et votre nb total de cartes.\n");
        }
    } while (carte < 1 || carte > joueurs[joueur].nbCartes);

    return carte;
}

//envoi de la demande de chosir la rangée au joueur à c'est le tour de jouer
int demandeRangee(int joueur) {
    int rangee;

    do {
        char msg[200];
        memset(msg, 0, sizeof(msg));
        envoyerMsg(joueur, "\n SAISIR N° RANGÉE \n");

        //Recevoir le message
        recevoirMsg(joueur, msg);
        
        rangee = (int)strtol(msg, NULL, 10);

        //Vérifier que l'entier est entre 1 et 4
        if (rangee < 1 || rangee > 4) {
            fprintf(stderr, "Erreur : Le numéro de rangée doit être entre 1 et 4.\n");
        }
    } while (rangee < 1 || rangee > 4); //tant que la rangée n'est pas entre 1 et 4 (il y'a que 4 rangées dans le plateau de jeu)

    return rangee;
}

//méthode pour poser une carte
void poserCarte(int joueur)
{
    //demande de la carte et de la rangée
    int carte = demandePoserCarte(joueur);
    int rangee = demandeRangee(joueur);

    int cpt = 0;//cpt de cartes déjà présentes dans la rangée

    bool isOk = false; //booléen pour dire qu'on a posé la carte ou pas

    while (cpt < 5 && !isOk) //tant que le nombre de cartes déjà posées dans la rangée est < 5
    {   
        //si la case qu'on parcourt ne possède pas de carte
        if (plateau[rangee-1][cpt].valNum == 0)
        {   
            //si la carte posée dans la case juste avant a une valeur supérieure à la carte que le joueur veut poser
            if (plateau[rangee - 1][cpt - 1].valNum > joueurs[joueur].main[carte - 1].valNum)
            {
                //on récupère toutes les cartes de la rangée

                //Augmenter la taille de la main du joueur de 6 cartes (on augmente de 6 juste pour s'assurer)
                joueurs[joueur].nbCartes += 6;
                joueurs[joueur].main = realloc(joueurs[joueur].main, joueurs[joueur].nbCartes * sizeof(Carte));

                //Stocker la carte à placer
                Carte tempo;
                tempo.valNum = joueurs[joueur].main[carte - 1].valNum;
                tempo.valTete = joueurs[joueur].main[carte - 1].valTete;

                //Mettre la carte jouée à zéro (on l'enlève du paquet de joueur on l'a placé dans une carte temporaire)
                joueurs[joueur].main[carte - 1].valNum = 0;
                joueurs[joueur].main[carte - 1].valTete = 0;

                //Copier les cartes du plateau dans la main du joueur
                for (int i = 0; i < 6; i++)
                {
                    //Copier les cartes à la fin de la main du joueur
                    joueurs[joueur].main[joueurs[joueur].nbCartes - 6 + i].valNum = plateau[rangee - 1][i].valNum;
                    joueurs[joueur].main[joueurs[joueur].nbCartes - 6 + i].valTete = plateau[rangee - 1][i].valTete;

                    //Réinitialiser le plateau
                    plateau[rangee - 1][i].valNum = 0;
                    plateau[rangee - 1][i].valTete = 0;
                }

                //Mettre à jour la taille de la main du joueur
                joueurs[joueur].nbCartes -= 1;

                //Placer la carte stockée à la première position du plateau
                plateau[rangee - 1][0].valNum = tempo.valNum;
                plateau[rangee - 1][0].valTete = tempo.valTete;
                
                printf("%s récupère toutes les cartes de la rangée n°%d.\n", joueurs[joueur].pseudo, rangee);
                
                //Mettre à jour la main du joueur
                retirerCartesJoueur(joueur);
            } else
            {
                //On place la carte à la dernière case dispo
                plateau[rangee - 1][cpt].valNum = joueurs[joueur].main[carte - 1].valNum;
                plateau[rangee - 1][cpt].valTete = joueurs[joueur].main[carte - 1].valTete;
                printf("%s a placé la carte [%d - %d] dans la rangée n°%d.\n", joueurs[joueur].pseudo, joueurs[joueur].main[carte - 1].valNum, joueurs[joueur].main[carte - 1].valTete, rangee);
                
                //on met la carte à 0 dans le paquet du joueur pour pouvoir l'enlever avec la méthode retirerCartesJoueurs
                joueurs[joueur].main[carte - 1].valNum = 0;
                joueurs[joueur].main[carte - 1].valTete = 0;
                retirerCartesJoueur(joueur); 
                majScore(joueur); //MAJ du score (on somme toutes les têtes de boeufs dans le paquet du joueur)
            }
            isOk = !isOk; //on a placé la carte
        } 
        else
        {
            cpt++; //on incrémente si il y'a une carte déja posée dans cette case
        }
    }

    if (cpt >= 5) //si la rangée possède déjà 5 cartes donc la carte que le joueur veut poser est la 6ème
    //d'après les règles du jeu dans cette situation le joueur récupère toutes les cartes de la rangée et place sa carte à la première case de la rangée
    {
        //Augmenter la taille de la main du joueur de 6 cartes
        envoyerMsg(joueur, "La rangée comporte 5 cartes, vous allez récupérer toutes les cartes de la rangée !");
        joueurs[joueur].nbCartes += 6;
        joueurs[joueur].main = realloc(joueurs[joueur].main, joueurs[joueur].nbCartes * sizeof(Carte));

        for (int i = 0; i < 6; i++)
        {
            //on récup toutes les cartes de la rangée
            joueurs[joueur].main[joueurs[joueur].nbCartes - 6 + i].valNum = plateau[rangee - 1][i].valNum;
            joueurs[joueur].main[joueurs[joueur].nbCartes - 6 + i].valTete = plateau[rangee - 1][i].valTete;

            plateau[rangee - 1][i].valNum = 0;
            plateau[rangee - 1][i].valTete = 0;
        }

        printf("%s a récupéré les 5 cartes de la rangée n°%d.\n", joueurs[joueur].pseudo, rangee);
        //placer la carte à la première case
        plateau[rangee - 1][0].valNum = joueurs[joueur].main[carte - 1].valNum;
        plateau[rangee - 1][0].valTete = joueurs[joueur].main[carte - 1].valTete;
        joueurs[joueur].main[carte - 1].valNum = 0;
        joueurs[joueur].main[carte - 1].valTete = 0;
        retirerCartesJoueur(joueur);
        majScore(joueur);
    }
}

//méthode au cas où le joueur n'a plus de cartes à poser
void redonnerCartes(int joueur)
{
    printf("%s n'a plus de cartes, redistribution de 10 cartes...", joueurs[joueur].pseudo);
    free(joueurs[joueur].main);
    joueurs[joueur].nbCartes = 10;
    joueurs[joueur].main = malloc(joueurs[joueur].nbCartes * sizeof(Carte));

    for (int i = 0; i < 10; i++)
    {
        int x = rand() % nbCartesTotal;
        while (cartes[x].valNum == 0 && cartes[x].valTete == 0) x = rand() % 104;
        joueurs[joueur].main[i].valNum = 0;
        joueurs[joueur].main[i].valTete = 0;

        if (cartes[x].valNum != 0 && cartes[x].valTete != 0)
        {
            joueurs[joueur].main[i].valNum = cartes[x].valNum;
            joueurs[joueur].main[i].valTete = cartes[x].valTete;
        }
        cartes[x].valNum = 0;
        cartes[x].valTete = 0;
        retirerCartesDistribuees();
    }
    majScore(joueur);
}

//boucle du jeu
void jouerPartie()
{
    bool finPartie = false;

    int cptManches = 1; //ce compteur ne s'arrête jamais on a pas de nb de manche limite
    int cptTours = 1; //10 tours par manche

    int tourJoueur; //pour suivre à qui c'est le tour de jouer
    system("clear");
    printf("La partie est lancée !\n");

    initManche();//initialisation des cartes, des cartes du joueur et du plateau

    while (!finPartie)
    {
        //initManche();
        tourJoueur = 0;
        cptTours = 1;
        
        printf("MANCHE %d\n", cptManches);

        while (cptTours <= 10 && !finPartie) //boucle d'une manche complète
        {

            afficherPlateau(); //affichage du plateau chez tous le monde
            afficherCartesJ(tourJoueur); //envoi les cartes de son paquet au joueur à qui c'est le tour de jouer
            afficherScore(tourJoueur); //affichage de son score (somme des têtes de boeuf de son paquet)
            envoyerMsg(tourJoueur, "\n\nÀ TOI DE JOUER !");

            poserCarte(tourJoueur); 

            if (joueurs[tourJoueur].score >= 50) //j'ai mis à 50 la limite du score pour que la partie se termine plus rapidement pendant les simulations
            //mais de base la limite c'est 66
            {
                //si le joueur a un score supérieur à 66 il a perdu la partie
                finPartie = true;

                printf("%s a perdu ! \n", joueurs[tourJoueur].pseudo);

                char msg[512];

                //on notifie le joueur qu'il a perdu
                envoyerMsg(tourJoueur, "Vous avez perdu !");
                
                //Envoi de l'ordre de quitter
                snprintf(msg, sizeof(msg), "quitter");
                send(joueurs[tourJoueur].socket, msg, sizeof(msg), 0);


                //Enregistrement du résultat dans le fichier du joueur
                char cmd[100];
                snprintf(cmd, sizeof(cmd), "./../code/fichierJoueur.sh %s FIN_PARTIE %d %d %d", joueurs[tourJoueur].pseudo, joueurs[tourJoueur].score, 1, 0);

                if (system(cmd) == -1)
                {
                    perror("Erreur lors de l'exécution du script Shell");
                    exit(1);
                }
                
                //boucle pour envoyer la notification qu'ils ont gagnés, qu'ils doivent et génération du fichier résultat
                for (int i = 0; i < nbJoueurs; i++)
                {
                    if (i != tourJoueur)
                    {
                        //Enregistrement du résultat dans le fichier des autres joueurs
                        snprintf(cmd, sizeof(cmd), "./../code/fichierJoueur.sh %s FIN_PARTIE %d %d %d", joueurs[i].pseudo, joueurs[i].score, 0, 1);

                        if (system(cmd) == -1)
                        {
                            perror("Erreur lors de l'exécution du script Shell");
                            exit(1);
                        }

                        char msg[512];

                        //Envoi de l'ordre d'affichage et quitter
                        envoyerMsg(i, "Vous avez gagné");

                        snprintf(msg, sizeof(msg), "quitter");
                        send(joueurs[i].socket, msg, sizeof(msg), 0);
                    }
                }

                finPartie = true;
            }
            tourJoueur++;
            if (tourJoueur == nbJoueurs) tourJoueur = 0;
            cptTours++;
        }
        cptManches++;
    }
    system("./../code/classement.sh"); //à la fin de la partie on génére le pdf avec le classement
}

//MAJ du score du joueur spécifié
void majScore(int joueur)
{
    //somme des têtes de boeuf qui se trouve dans le paquet de carte du joueur spécifié
    joueurs[joueur].score = 0;
    for (int i = 0; i < joueurs[joueur].nbCartes; i++)
    {
        joueurs[joueur].score += joueurs[joueur].main[i].valTete;
    }
}

//génération du fichier scores des joueurs (on affiche dans un fichier les résultats des joueurs sans faire de classement)
void fichierScore()
{
    printf("Pour voir le score de tous les joueurs de la partie, allez voir dans le fichier scores dans le répetoire historique\n");

    FILE *scoreFile = fopen("../logs/historique/scores.txt", "w");
    if (scoreFile != NULL) {
        fprintf(scoreFile, "SCORE :\n");

        for (int i = 0; i < nbJoueurs; i++) {
            fprintf(scoreFile, "%d - %s : %d pts\n", i + 1, joueurs[i].pseudo, joueurs[i].score);
        }
        fclose(scoreFile);
    } else {
        perror("Erreur : ouverture fichier partie");
        exit(1);
    }
}

//méthode pour quitter proprement le programme
void quitter()
{
    for (int i = 0; i < nbJoueurs; i++)
    {
        envoyerMsg(i, "Au revoir !");

        //envoi de l'ordre de quitter
        char quitterMsg[512];
        snprintf(quitterMsg, sizeof(quitterMsg), "quitter");
        if (send(joueurs[i].socket, quitterMsg, sizeof(quitterMsg), 0) == -1)
        {
            perror("Erreur lors de l'envoi de l'ordre 'quitter'");
        }
        
        printf("%s s'est déconnecté.\n", joueurs[i].pseudo);

        //Fermeture socket  joueur
        if (close(joueurs[i].socket) == -1)
        {
            perror("Erreur lors de la fermeture de la socket du joueur");
        }

        //Libération de la mémoire pour le pseudo
        memset(joueurs[i].pseudo, 0, sizeof(joueurs[i].pseudo));
    }


    //Réinitialisation du nombre de joueurs à zéro
    nbJoueurs = 0;

    //Libération de la mémoire pour les cartes
    free(cartes);

    //Libération de la mémoire pour les mains des joueurs
    for (int i = 0; i < nbJoueurs; i++)
    {
        free(joueurs[i].main);
    }

    //Libération de la mémoire pour la liste des joueurs
    free(joueurs);

    printf("Au revoir !\n");
    exit(0);
}

void menu()
{
    sleep(3);
    system("clear");
    printf("\n          MENU          \n");
    printf("\n    1 - JOUER\n");
    printf("    2 - AJOUTER JOUEUR\n");
    printf("    3 - LISTE JOUEURS CONNECTÉS\n");
    printf("    4 - QUITTER\n");

    int choix;
    printf("\nChoix : ");
    scanf("%d", &choix);

    switch (choix)
    {
        default:
            printf("Erreur : choix incorrect\n");
            break;
        case 1:
            if (nbJoueurs >= 2 && nbJoueurs <= 8)
            {
                jouerPartie();
                break;
            }
            else 
            {
                system("clear");
                printf("La partie ne peut pas se lancer");
            }
            break;
        case 2:
            if (nbJoueurs < 10)
            {
                system("clear");
                printf("  AJOUT D'UN NOUVEAU JOUEUR\n\n");
                printf("  1 - Ajouter un humain\n");
                printf("  2 - Ajouter un robot\n");
                printf("\nChoix : ");

                int choix2;
                scanf("%d", &choix2);

                switch (choix2)
                {
                    default:
                        printf("Erreur : choix incorrect\n");
                        break;
                    case 1:
                        //Ajout d'un humain
                        system("clear");
                        printf("En attente de connexion de l'humain...\n");
                        accepterJoueur();
                        break;
                    case 2:
                        //Ajout d'un robot
                        system("clear");
                        printf("En attente de connexion du robot...\n");
                        accepterJoueur();
                        break;
                }
            } else printf("Le salon est plein !\n");
            break;
        case 3:
            system("clear");
            printf("  LISTE DES JOUEURS CONNECTÉS\n");
            if (nbJoueurs < 1)
            {
                printf("Le salon est vide...\n");
                break;
            }
            
            for (int i = 0; i < nbJoueurs; i++)
            {
                printf("%d - %s\n", i + 1, joueurs[i].pseudo);
            }
            break;
        case 4:
            fichierScore();
            quitter();
            break;
    }

}

int main()
{
    srand(time(NULL));
    
    initServ(); //initialisation du serveur
    
    signal(SIGINT, handleSignal);  //Ctrl+C
    signal(SIGTERM, handleSignal); //Signal de terminaison
    
    while (1)
    {
        menu();
    }

    return 0;
}
