/*
 * tp4.c
 * Implémentation avec Optimisations Algorithmiques Critiques
 */

#include "tp4.h"

/* --- FONCTIONS UTILITAIRES --- */

// Complexité : O(1)
void initIndex(T_Index* index) {
    index->racine = NULL;
    index->nbMotsDistincts = 0;
    index->nbMotsTotal = 0;
    index->nbPhrasesCapacite = 100; 
    index->tabPhrases = (char**)calloc(index->nbPhrasesCapacite, sizeof(char*));
}

// Complexité : O(1)
T_Position* creerPosition(int ligne, int ordre, int phrase) {
    T_Position* newP = (T_Position*)malloc(sizeof(T_Position));
    if (newP) {
        newP->numeroLigne = ligne;
        newP->numeroPhrase = phrase;
        newP->ordre = ordre;
        newP->suivant = NULL;
    }
    return newP;
}

// Complexité : O(1)
T_Noeud* creerNoeud(char* mot) {
    T_Noeud* newN = (T_Noeud*)malloc(sizeof(T_Noeud));
    if (newN) {
        newN->mot = strdup(mot);
        newN->nbOccurences = 0;
        newN->listePositions = NULL;
        newN->dernierePosition = NULL; // Init pointeur queue
        newN->filsGauche = NULL;
        newN->filsDroit = NULL;
    }
    return newN;
}

// Complexité : O(1) amorti (realloc est rare)
void verifierCapacitePhrases(T_Index* index, int numPhrase) {
    if (numPhrase >= index->nbPhrasesCapacite) {
        int oldCap = index->nbPhrasesCapacite;
        int newCap = (numPhrase * 2) + 50;
        index->tabPhrases = (char**)realloc(index->tabPhrases, newCap * sizeof(char*));
        // Mise à NULL des nouvelles cases pour sécurité
        for (int i = oldCap; i < newCap; i++) index->tabPhrases[i] = NULL;
        index->nbPhrasesCapacite = newCap;
    }
}

/* --- FONCTIONS PRINCIPALES --- */

// Complexité : O(k) 
// (Utilisée uniquement en secours si l'ordre n'est pas respecté)
T_Position* ajouterPosition(T_Position* listeP, int ligne, int ordre, int phrase) {
    T_Position* newP = creerPosition(ligne, ordre, phrase);
    if (!newP) return listeP;

    if (listeP == NULL || 
        (ligne < listeP->numeroLigne) || 
        (ligne == listeP->numeroLigne && ordre < listeP->ordre)) {
        newP->suivant = listeP;
        return newP;
    }

    T_Position* courant = listeP;
    while (courant->suivant != NULL) {
        if (ligne < courant->suivant->numeroLigne || 
           (ligne == courant->suivant->numeroLigne && ordre < courant->suivant->ordre)) {
            break;
        }
        courant = courant->suivant;
    }
    
    newP->suivant = courant->suivant;
    courant->suivant = newP;
    return listeP;
}

// Complexité : O(h)
// h = hauteur de l'arbre.
// L'insertion de la position est O(1) grâce au pointeur 'dernierePosition'.
int ajouterOccurence(T_Index* index, char* mot, int ligne, int ordre, int phrase) {
    if (!index) return 0;
    if (index->tabPhrases == NULL) initIndex(index);

    T_Noeud** courant = &(index->racine);
    T_Noeud* noeudTrouve = NULL;

    // 1. Recherche dans l'ABR : O(h)
    while (*courant != NULL) {
        int cmp = strcasecmp(mot, (*courant)->mot);
        if (cmp == 0) {
            noeudTrouve = *courant;
            break;
        } else if (cmp < 0) courant = &((*courant)->filsGauche);
        else courant = &((*courant)->filsDroit);
    }

    if (noeudTrouve == NULL) {
        *courant = creerNoeud(mot);
        if (!*courant) return 0;
        noeudTrouve = *courant;
        index->nbMotsDistincts++;
    }

    // 2. Ajout Position : O(1) via Queue
    T_Position* newP = creerPosition(ligne, ordre, phrase);
    
    if (noeudTrouve->listePositions == NULL) {
        noeudTrouve->listePositions = newP;
        noeudTrouve->dernierePosition = newP;
    } 
    else if (noeudTrouve->dernierePosition != NULL) {
        // Branchement direct à la fin sans parcours
        noeudTrouve->dernierePosition->suivant = newP;
        noeudTrouve->dernierePosition = newP;
    } 
    else {
        // Cas fallback (rare)
        noeudTrouve->listePositions = ajouterPosition(noeudTrouve->listePositions, ligne, ordre, phrase);
    }

    noeudTrouve->nbOccurences++;
    index->nbMotsTotal++;
    return 1;
}

// Complexité Globale : O(N * h + M)
// N = nombre de mots (coût ABR)
// h = hauteur arbre
// M = nombre total de caractères (coût construction phrases)
// Note : La construction est O(M) Linéaire grâce au remplacement de strcat par l'arithmétique de pointeurs.
int indexerFichier(T_Index* index, char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;
    if (index->tabPhrases == NULL) initIndex(index);

    char motBuffer[256];
    
    // Buffer dynamique pour la phrase complète
    int tailleMax = 1024;
    int lenPhrase = 0; // "Curseur" de position dans la phrase
    char* phraseBuffer = (char*)malloc(tailleMax * sizeof(char));
    phraseBuffer[0] = '\0';

    int bufIndex = 0;
    char c;
    int ligne = 1, ordre = 1, phrase = 1, motsLus = 0;

    while ((c = fgetc(f)) != EOF) {
        if (isalpha(c)) {
            motBuffer[bufIndex++] = c;
        } else {
            if (bufIndex > 0) {
                motBuffer[bufIndex] = '\0';
                
                // A. Insertion ABR : O(h)
                ajouterOccurence(index, motBuffer, ligne, ordre, phrase);
                motsLus++;

                // B. Construction Phrase Optimisée : O(longueur_mot)
                int lenMot = strlen(motBuffer);
                
                // Redimensionnement si nécessaire
                if (lenPhrase + lenMot + 5 >= tailleMax) {
                    tailleMax *= 2;
                    phraseBuffer = (char*)realloc(phraseBuffer, tailleMax);
                }
                
                // Ajout d'espace (Arithmétique de pointeur, pas de strcat)
                if (lenPhrase > 0) {
                    phraseBuffer[lenPhrase] = ' ';
                    lenPhrase++;
                }
                
                // Copie directe du mot à la bonne adresse (O(lenMot))
                strcpy(phraseBuffer + lenPhrase, motBuffer);
                lenPhrase += lenMot;
                
                // Null-terminate temporaire
                phraseBuffer[lenPhrase] = '\0';

                ordre++;
                bufIndex = 0;
            }

            if (c == '\n') { ligne++; ordre = 1; }
            
            // Detection fin de phrase
            if (c == '.') {
                if (lenPhrase + 2 >= tailleMax) {
                     phraseBuffer = (char*)realloc(phraseBuffer, tailleMax + 2);
                }
                // Ajout point
                phraseBuffer[lenPhrase] = '.';
                phraseBuffer[lenPhrase + 1] = '\0';
                
                // Sauvegarde O(1) dans le Cache
                verifierCapacitePhrases(index, phrase);
                index->tabPhrases[phrase] = strdup(phraseBuffer);
                
                // Reset curseurs
                lenPhrase = 0;
                phraseBuffer[0] = '\0';
                phrase++;
            }
        }
    }
    
    // Dernier mot si pas de ponctuation finale
    if (bufIndex > 0) {
        motBuffer[bufIndex] = '\0';
        ajouterOccurence(index, motBuffer, ligne, ordre, phrase);
        motsLus++;
    }

    free(phraseBuffer);
    fclose(f);
    return motsLus;
}

// Complexité : O(N) (Parcours infixe complet)
void afficherIndexRecursif(T_Noeud* noeud, char* lastChar) {
    if (!noeud) return;
    afficherIndexRecursif(noeud->filsGauche, lastChar);

    char currentChar = toupper(noeud->mot[0]);
    if (currentChar != *lastChar) {
        if (*lastChar != '\0') printf("\n");
        printf("%c\n", currentChar);
        *lastChar = currentChar;
    }

    printf("|-- %s\n", noeud->mot);
    T_Position* p = noeud->listePositions;
    while (p) {
        printf("|---- (l:%d, o:%d, p:%d)\n", p->numeroLigne, p->ordre, p->numeroPhrase);
        p = p->suivant;
    }
    printf("|\n");

    afficherIndexRecursif(noeud->filsDroit, lastChar);
}

// Complexité : O(N)
void afficherIndex(T_Index index) {
    if (index.racine == NULL) {
        printf("Index vide.\n");
        return;
    }
    char lastChar = '\0';
    afficherIndexRecursif(index.racine, &lastChar);
}

// Complexité : O(h)
T_Noeud* rechercherMot(T_Index index, char* mot) {
    T_Noeud* courant = index.racine;
    while (courant != NULL) {
        int cmp = strcasecmp(mot, courant->mot);
        if (cmp == 0) return courant;
        if (cmp < 0) courant = courant->filsGauche;
        else courant = courant->filsDroit;
    }
    return NULL;
}

// Complexité : O(h + k)
// h = recherche du noeud
// k = nombre d'occurrences
// L'accès à la phrase est O(1) grâce au cache 'tabPhrases'.
void afficherOccurencesMot(T_Index index, char* mot) {
    T_Noeud* noeud = rechercherMot(index, mot);
    if (!noeud) {
        printf("Mot '%s' non trouve.\n", mot);
        return;
    }

    printf("Mot = \"%s\"\nOccurences = %d\n", noeud->mot, noeud->nbOccurences);
    
    T_Position* p = noeud->listePositions;
    while (p) {
        printf("| Ligne %d, Phrase %d : ", p->numeroLigne, p->numeroPhrase);
        
        // Optimisation : Affichage direct depuis le cache
        if (p->numeroPhrase < index.nbPhrasesCapacite && index.tabPhrases[p->numeroPhrase] != NULL) {
            printf("%s", index.tabPhrases[p->numeroPhrase]);
        }
        printf("\n");
        p = p->suivant;
    }
}

// Complexité : O(P)
// P = Nombre de phrases.
// Reconstruction immédiate grâce au cache pré-construit.
void construireTexte(T_Index index, char* filename) {
    if (index.nbMotsTotal == 0) return;
    FILE* f = fopen(filename, "w");
    if (!f) return;

    for (int i = 1; i < index.nbPhrasesCapacite; i++) {
        if (index.tabPhrases[i] != NULL) {
            fprintf(f, "%s\n", index.tabPhrases[i]);
        }
    }
    fclose(f);
    printf("Texte reconstruit dans %s.\n", filename);
}

/* --- LIBERATION --- */

// Complexité : O(N)
void libererNoeud(T_Noeud* noeud) {
    if (noeud != NULL) {
        libererNoeud(noeud->filsGauche);
        libererNoeud(noeud->filsDroit);
        
        T_Position* p = noeud->listePositions;
        while (p != NULL) {
            T_Position* temp = p;
            p = p->suivant;
            free(temp);
        }
        free(noeud->mot);
        free(noeud);
    }
}

// Complexité : O(P + N)
void libererIndex(T_Index* index) {
    // Libération du cache de phrases
    if (index->tabPhrases) {
        for (int i = 0; i < index->nbPhrasesCapacite; i++) {
            if (index->tabPhrases[i]) free(index->tabPhrases[i]);
        }
        free(index->tabPhrases);
        index->tabPhrases = NULL;
    }
    // Libération de l'ABR
    libererNoeud(index->racine);
    index->racine = NULL;
    index->nbMotsDistincts = 0;
    index->nbMotsTotal = 0;
}