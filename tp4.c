#include "tp4.h"
#include <strings.h> // Pour strcasecmp

/* --- STRUCTURES INTERNES (Helper pour Q6 & Q7) --- */

// Structure pour stocker un mot avec son ordre dans une phrase
typedef struct MotDansPhrase {
    char* mot;
    int ordre;
    struct MotDansPhrase* suivant;
} T_MotDansPhrase;

// Structure pour stocker toutes les phrases (utilisée par construireTexte)
typedef struct {
    T_MotDansPhrase** phrases;
    int capacite;
} T_MapPhrases;

/* --- FONCTIONS UTILITAIRES --- */

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

T_Noeud* creerNoeud(char* mot) {
    T_Noeud* newN = (T_Noeud*)malloc(sizeof(T_Noeud));
    if (newN) {
        newN->mot = strdup(mot);
        newN->nbOccurences = 0;
        newN->listePositions = NULL;
        newN->filsGauche = NULL;
        newN->filsDroit = NULL;
    }
    return newN;
}

int comparerMots(char* m1, char* m2) {
    return strcasecmp(m1, m2);
}

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

void libererIndex(T_Index* index) {
    libererNoeud(index->racine);
    index->racine = NULL;
    index->nbMotsDistincts = 0;
    index->nbMotsTotal = 0;
}

/* --- FONCTIONS PRINCIPALES --- */

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

int ajouterOccurence(T_Index* index, char* mot, int ligne, int ordre, int phrase) {
    if (!index) return 0;
    T_Noeud** courant = &(index->racine);

    while (*courant != NULL) {
        int cmp = comparerMots(mot, (*courant)->mot);
        if (cmp == 0) {
            (*courant)->listePositions = ajouterPosition((*courant)->listePositions, ligne, ordre, phrase);
            (*courant)->nbOccurences++;
            index->nbMotsTotal++;
            return 1;
        } else if (cmp < 0) {
            courant = &((*courant)->filsGauche);
        } else {
            courant = &((*courant)->filsDroit);
        }
    }

    *courant = creerNoeud(mot);
    if (*courant == NULL) return 0;
    (*courant)->listePositions = ajouterPosition(NULL, ligne, ordre, phrase);
    (*courant)->nbOccurences = 1;
    index->nbMotsTotal++;
    index->nbMotsDistincts++;
    return 1;
}

int indexerFichier(T_Index* index, char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    char buffer[256];
    int bufIndex = 0;
    char c;
    int ligne = 1, ordre = 1, phrase = 1, motsLus = 0;

    if (index->racine == NULL) {
        index->nbMotsDistincts = 0;
        index->nbMotsTotal = 0;
    }

    while ((c = fgetc(f)) != EOF) {
        if (isalpha(c)) {
            buffer[bufIndex++] = c;
        } else {
            if (bufIndex > 0) {
                buffer[bufIndex] = '\0';
                ajouterOccurence(index, buffer, ligne, ordre, phrase);
                motsLus++;
                ordre++;
                bufIndex = 0;
            }
            if (c == '\n') { ligne++; ordre = 1; }
            else if (c == '.') { phrase++; }
        }
    }
    if (bufIndex > 0) {
        buffer[bufIndex] = '\0';
        ajouterOccurence(index, buffer, ligne, ordre, phrase);
        motsLus++;
    }

    fclose(f);
    return motsLus;
}

// Helper pour afficherIndex
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

void afficherIndex(T_Index index) {
    if (index.racine == NULL) {
        printf("Index vide.\n");
        return;
    }
    char lastChar = '\0';
    afficherIndexRecursif(index.racine, &lastChar);
}

T_Noeud* rechercherMot(T_Index index, char* mot) {
    T_Noeud* courant = index.racine;
    while (courant != NULL) {
        int cmp = comparerMots(mot, courant->mot);
        if (cmp == 0) return courant;
        if (cmp < 0) courant = courant->filsGauche;
        else courant = courant->filsDroit;
    }
    return NULL;
}

/* --- HELPERS POUR RECONSTRUCTION DE PHRASES --- */

// Libérer une liste de mots
void libererListeMots(T_MotDansPhrase* liste) {
    while (liste) {
        T_MotDansPhrase* temp = liste;
        liste = liste->suivant;
        free(temp);
    }
}

// Insérer un mot trié par ordre dans la liste
void insererMotTrie(T_MotDansPhrase** liste, char* mot, int ordre) {
    T_MotDansPhrase* nouveau = (T_MotDansPhrase*)malloc(sizeof(T_MotDansPhrase));
    if (!nouveau) return;
    
    nouveau->mot = mot;
    nouveau->ordre = ordre;
    nouveau->suivant = NULL;

    if (*liste == NULL || (*liste)->ordre > ordre) {
        nouveau->suivant = *liste;
        *liste = nouveau;
        return;
    }

    T_MotDansPhrase* courant = *liste;
    while (courant->suivant != NULL && courant->suivant->ordre < ordre) {
        courant = courant->suivant;
    }
    
    nouveau->suivant = courant->suivant;
    courant->suivant = nouveau;
}

// Collecter tous les mots d'une phrase donnée (parcours de l'ABR)
void collecterMotsPhrase(T_Noeud* noeud, int numPhrase, T_MotDansPhrase** phrase) {
    if (!noeud) return;
    
    collecterMotsPhrase(noeud->filsGauche, numPhrase, phrase);
    
    T_Position* p = noeud->listePositions;
    while (p) {
        if (p->numeroPhrase == numPhrase) {
            insererMotTrie(phrase, noeud->mot, p->ordre);
        }
        p = p->suivant;
    }
    
    collecterMotsPhrase(noeud->filsDroit, numPhrase, phrase);
}

// Reconstruire et afficher UNE phrase
void reconstruireEtAfficherPhrase(T_Noeud* racine, T_Position* pos) {
    T_MotDansPhrase* phrase = NULL;
    collecterMotsPhrase(racine, pos->numeroPhrase, &phrase);
    
    printf("| Ligne %d, mot %d : ", pos->numeroLigne, pos->ordre);
    
    T_MotDansPhrase* m = phrase;
    while (m) {
        printf("%s", m->mot);
        if (m->suivant) printf(" ");
        m = m->suivant;
    }
    printf(".\n");
    
    libererListeMots(phrase);
}

/* --- Q6 : AFFICHAGE OPTIMISÉ DES OCCURRENCES --- */

void afficherOccurencesMot(T_Index index, char* mot) {
    T_Noeud* noeud = rechercherMot(index, mot);
    if (!noeud) {
        printf("Mot '%s' non trouve.\n", mot);
        return;
    }

    printf("Mot = \"%s\"\nOccurences = %d\n", noeud->mot, noeud->nbOccurences);
    
    // Pour chaque occurrence, reconstruire UNIQUEMENT la phrase concernée
    T_Position* p = noeud->listePositions;
    while (p) {
        reconstruireEtAfficherPhrase(index.racine, p);
        p = p->suivant;
    }
}

/* --- HELPERS POUR Q7 (RECONSTRUCTION COMPLÈTE) --- */

void initMap(T_MapPhrases* map, int tailleEstimee) {
    map->capacite = tailleEstimee;
    map->phrases = (T_MotDansPhrase**)calloc(map->capacite, sizeof(T_MotDansPhrase*));
}

void ajouterMotDansMap(T_MapPhrases* map, int idPhrase, int ordre, char* mot) {
    if (idPhrase >= map->capacite) {
        int oldCap = map->capacite;
        int newCap = (idPhrase * 2) + 10;
        map->phrases = (T_MotDansPhrase**)realloc(map->phrases, newCap * sizeof(T_MotDansPhrase*));
        for (int i = oldCap; i < newCap; i++) map->phrases[i] = NULL;
        map->capacite = newCap;
    }
    
    T_MotDansPhrase* newM = (T_MotDansPhrase*)malloc(sizeof(T_MotDansPhrase));
    newM->mot = mot; 
    newM->ordre = ordre;
    newM->suivant = NULL;

    T_MotDansPhrase** tete = &(map->phrases[idPhrase]);
    if (*tete == NULL || (*tete)->ordre > ordre) {
        newM->suivant = *tete;
        *tete = newM;
    } else {
        T_MotDansPhrase* cur = *tete;
        while (cur->suivant != NULL && cur->suivant->ordre < ordre) {
            cur = cur->suivant;
        }
        newM->suivant = cur->suivant;
        cur->suivant = newM;
    }
}

void remplirMapPhrases(T_Noeud* noeud, T_MapPhrases* map) {
    if (!noeud) return;
    remplirMapPhrases(noeud->filsGauche, map);
    T_Position* p = noeud->listePositions;
    while (p) {
        ajouterMotDansMap(map, p->numeroPhrase, p->ordre, noeud->mot);
        p = p->suivant;
    }
    remplirMapPhrases(noeud->filsDroit, map);
}

void libererMap(T_MapPhrases* map) {
    for (int i = 0; i < map->capacite; i++) {
        T_MotDansPhrase* m = map->phrases[i];
        while (m) {
            T_MotDansPhrase* temp = m;
            m = m->suivant;
            free(temp);
        }
    }
    free(map->phrases);
}

/* --- Q7 : RECONSTRUCTION DU TEXTE --- */

void construireTexte(T_Index index, char* filename) {
    if (!index.racine) return;
    FILE* f = fopen(filename, "w");
    if (!f) return;

    T_MapPhrases map;
    initMap(&map, 100);
    remplirMapPhrases(index.racine, &map);

    for (int i = 0; i < map.capacite; i++) {
        T_MotDansPhrase* m = map.phrases[i];
        if (m) {
            while (m) {
                fprintf(f, "%s", m->mot);
                if (m->suivant) fprintf(f, " ");
                m = m->suivant;
            }
            fprintf(f, ".\n");
        }
    }
    
    libererMap(&map);
    fclose(f);
}