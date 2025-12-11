# include "tp4.h"

T_Position *ajouterPosition(T_Position *listeP, int ligne, int ordre, int phrase) {
    T_Position *nouvellePos = (T_Position *)malloc(sizeof(T_Position));
    if (!nouvellePos) {
        fprintf(stderr, "Erreur d'allocation mémoire pour une nouvelle position.\n");
        return listeP;
    }
    nouvellePos->numeroLigne = ligne;
    nouvellePos->ordre = ordre;
    nouvellePos->numeroPhrase = phrase;
    nouvellePos->suivant = NULL;

    // insertion triée 
    if (listeP == NULL || (ligne < listeP->numeroLigne)||(ligne == listeP->numeroLigne && ordre < listeP->ordre)) { 
        // si la liste est vide ou si la nouvelle position doit être insérée au début 
        nouvellePos->suivant = listeP;
        return nouvellePos;
    }

    T_Position *current = listeP;
    while (current->suivant != NULL && 
            (current->suivant->numeroLigne < ligne || 
                (current->suivant->numeroLigne == ligne && current->suivant->ordre < ordre)
            )) {
                // tant que la position suivante est avant la nouvelle position on avance 
        current = current->suivant;
    }
    nouvellePos->suivant = current->suivant;
    current->suivant = nouvellePos;

    return listeP;
}

int ajouterOccurence(T_Index *index, char *mot, int ligne, int ordre, int phrase) {
    if (!index || !mot) {
        return -1; // erreur
    }
    T_Noeud **current = &(index->racine);
    while (*current != NULL) {
        int cmp = strcasecmp(mot, (*current)->mot);// comparaison lexicographique (insensible à la casse)
        if (cmp == 0) {
            // mot trouvé
            (*current)->nbOccurences++;
            (*current)->listePositions = ajouterPosition((*current)->listePositions, ligne, ordre, phrase);
            index->nbMotsTotal++;
            return 0; // succès
        } else if (cmp < 0) {// mot avant dans l'ordre alphabétique
            current = &((*current)->filsGauche);
        } else {// mot après dans l'ordre alphabétique
            current = &((*current)->filsDroit);
        }
    }
    // mot non trouvé, insertion d'un nouveau noeud
    T_Noeud *nouveauNoeud = (T_Noeud *)malloc(sizeof(T_Noeud));
    if (!nouveauNoeud) {
        fprintf(stderr, "Erreur d'allocation mémoire pour un nouveau noeud.\n");
        return -1; // erreur
    }
    nouveauNoeud->mot = strdup(mot);// dupliquer la chaîne
    nouveauNoeud->nbOccurences = 1;
    nouveauNoeud->listePositions = NULL;
    nouveauNoeud->listePositions = ajouterPosition(nouveauNoeud->listePositions, ligne, ordre, phrase);
    nouveauNoeud->filsGauche = NULL;
    nouveauNoeud->filsDroit = NULL; 
    *current = nouveauNoeud;
    index->nbMotsDistincts++;
    index->nbMotsTotal++;

    return 0;
}

int indexerFichier(T_Index *index, char *filename) {
    if (!index || !filename) {
        return -1; // erreur
    }
    FILE *file = fopen(filename, "r");// ouvrir le fichier en lecture
    if (!file) {
        fprintf(stderr, "Erreur d'ouverture du fichier %s.\n", filename);
        return -1; // erreur
    }

    char ligne[1024];// tampon pour lire les lignes
    int numeroLigne = 0;
    int numeroPhrase = 1;

    while (fgets(ligne, sizeof(ligne), file)) {// lire chaque ligne
        numeroLigne++;
        // on decoupe la ligne en mots sachant que les séparateurs sont les espaces ou les poit
        char *token = strtok(ligne, " \t\n.");
        int ordre = 0;
        while (token) {
            ordre++;
            char mot[256];
            strncpy(mot, token, sizeof(mot));
            mot[sizeof(mot) - 1] = '\0'; // assurer la terminaison
            toString(mot);

            // Ajouter l'occurrence dans l'index
            ajouterOccurence(index, mot, numeroLigne, ordre, numeroPhrase);

            // Vérifier si le token contient une fin de phrase
            if (strchr(token, '.')) {
                numeroPhrase++;
            }

            token = strtok(NULL, " \t\n.");
        }
    }

    fclose(file);
    return index->nbMotsTotal; // succès
}

// Fonction auxiliaire pour afficher récursivement (parcours in-ordre)
void afficherIndexAux(T_Noeud *noeud) {
    if (noeud == NULL) {
        return;
    }
    
    // Parcours in-ordre : gauche, racine, droit (ordre alphabétique)
    afficherIndexAux(noeud->filsGauche);
    
    // Afficher le mot
    printf("%c\n", noeud->mot[0]); // première lettre du mot
    printf("|-- %s\n", noeud->mot);
    
    // Afficher toutes les positions du mot
    T_Position *pos = noeud->listePositions;
    while (pos != NULL) {
        printf("|---- (l:%d, o:%d, p:%d)\n", pos->numeroLigne, pos->ordre, pos->numeroPhrase);
        pos = pos->suivant;
    }
    printf("|\n");
    
    afficherIndexAux(noeud->filsDroit);
}

void afficherIndex(T_Index index) {
    if (index.racine == NULL) {
        printf("L'index est vide.\n");
        return;
    }
    
    afficherIndexAux(index.racine);
}

T_Noeud* rechercherMot(T_Index index, char *mot) {
    T_Noeud *current = index.racine;
    while (current != NULL) {
        int cmp = strcasecmp(mot, current->mot);
        if (cmp == 0) {
            return current; // mot trouvé
        } else if (cmp < 0) {
            current = current->filsGauche; // chercher à gauche
        } else {
            current = current->filsDroit; // chercher à droite
        }
    }
    return NULL; // mot non trouvé
}

void afficherOccurrencesMot(T_Index index, char *mot, char *filename) {
    if (!mot || !filename) {
        return;
    }
    
    // Rechercher le mot dans l'index
    T_Noeud *noeud = rechercherMot(index, mot);
    if (noeud == NULL) {
        printf("Le mot \"%s\" n'a pas été trouvé dans l'index.\n", mot);
        return;
    }
    
    // Afficher le mot et le nombre d'occurrences
    printf("Mot = \"%s\"\n", noeud->mot);
    printf("Occurrences = %d\n", noeud->nbOccurences);
    
    // Relire le fichier pour afficher les phrases
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erreur d'ouverture du fichier %s.\n", filename);
        return;
    }
    
    char ligne[1024];
    int numeroLigne = 0;
    
    // Parcourir le fichier et reconstituer les phrases 
    //todo: optimiser cette partie
    while (fgets(ligne, sizeof(ligne), file)) {
        numeroLigne++;
        
        // Vérifier si cette ligne contient une occurrence du mot
        T_Position *pos = noeud->listePositions;
        int trouve = 0;
        while (pos != NULL) {
            if (pos->numeroLigne == numeroLigne) {
                trouve = 1;
                break;
            }
            pos = pos->suivant;
        }
        
        if (trouve) {
            // Afficher chaque occurrence de cette ligne
            pos = noeud->listePositions;
            while (pos != NULL) {
                if (pos->numeroLigne == numeroLigne) {
                    printf("| Ligne %d, mot %d : %s\n", pos->numeroLigne, pos->ordre, ligne);
                }
                pos = pos->suivant;
            }
        }
    }
    
    fclose(file);
}
// il manque construireTexte 
// TODO 
// et le meunu

// le meunu 

void initialiserIndex(T_Index *index) {
    if (index) {
        index->racine = NULL;
        index->nbMotsDistincts = 0;
        index->nbMotsTotal = 0;
    }
}





