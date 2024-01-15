#!/bin/bash

nomJoueur=$1 #pseudo du joueur en argument

#fichier + son chemin 
chemin="../logs/joueurs"
fichier="$chemin/$nomJoueur.txt"

#petite fonction pour enregistrer la date et l'heure de connexion dans le fichier
enregister_connexion(){

    echo "Connexion : $(date +"%d-%m-%Y à %H-%M-%S")" >> "$fichier"
}

#vérification : création du répertoire s'il existe pas (pas obligé)
mkdir -p "$chemin"

#vérification si le fichier du joueur existe déjà ou pas
if [ ! -e "$fichier" ]; then
    touch "$fichier"
fi

#appel de la fonction pour enregistrer la connexion dans le fichier
enregister_connexion

#à la fin de la partie
if [ "$2" == "FIN_PARTIE" ]; then
    #Ajout du score final
    echo "SCORE_FINAL : $3" >> "$fichier"

    #Ajout du résultat de la partie (Gagné ou Perdu)
    if [ "$3" -ge 50 ]; then
        echo "RESULTAT : Perdu" >> "$fichier"
    else
        echo "RESULTAT : Gagné" >> "$fichier"
    fi
fi
