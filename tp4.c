#include "tp4.h"
#include <strings.h> // Pour strcasecmp

/* --- Fonctions Utilitaires --- */

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

/* --- Fonctions Principales --- */

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

// Helper pour afficherIndex (Formatage Image 1)
void afficherIndexRecursif(T_Noeud* noeud, char* lastChar) {
    if (!noeud) return;

    afficherIndexRecursif(noeud->filsGauche, lastChar);

    char currentChar = toupper(noeud->mot[0]);
    
    // Si la lettre change (ex: on passe de A à B), on affiche l'en-tête
    if (currentChar != *lastChar) {
        if (*lastChar != '\0') printf("\n"); 
        printf("%c\n", currentChar);
        *lastChar = currentChar;
    }

    // Formatage précis : Barre verticale, tirets, mot
    printf("|-- %s\n", noeud->mot);
    
    T_Position* p = noeud->listePositions;
    while (p) {
        // Formatage précis : Barre verticale + 4 tirets + position
        // Exemple image : (1:2, o:16, p:8) -> On suppose l=Ligne, o=Ordre, p=Phrase
        // L'image utilise "l:1", "o:2"...
        printf("|---- (l:%d, o:%d, p:%d)\n", p->numeroLigne, p->ordre, p->numeroPhrase);
        p = p->suivant;
    }
    
    // Petite ligne de séparation verticale vide entre les mots d'une même lettre
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

/* --- STRUCTURES INTERMEDIAIRES POUR Q6 & Q7 --- */

typedef struct MotDansPhrase {
    char* mot;
    int ordre;
    struct MotDansPhrase* suivant;
} T_MotDansPhrase;

typedef struct {
    T_MotDansPhrase** phrases;
    int capacite;
} T_MapPhrases;

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

// Q6 : Affichage contextuel (Formatage Image 2)
void afficherOccurencesMot(T_Index index, char* mot) {
    T_Noeud* noeud = rechercherMot(index, mot);
    if (!noeud) {
        printf("Mot '%s' non trouve.\n", mot);
        return;
    }

    printf("Mot = \"%s\"\nOccurences = %d\n", noeud->mot, noeud->nbOccurences);
    
    // Reconstruction de tout le texte pour avoir les phrases complètes
    T_MapPhrases map;
    initMap(&map, 100);
    remplirMapPhrases(index.racine, &map);

    T_Position* p = noeud->listePositions;
    while (p) {
        // Formatage exact de l'image 2 : "| Ligne X, mot Y : Phrase..."
        printf("| Ligne %d, mot %d : ", p->numeroLigne, p->ordre);
        
        if (p->numeroPhrase < map.capacite && map.phrases[p->numeroPhrase]) {
            T_MotDansPhrase* m = map.phrases[p->numeroPhrase];
            while (m) {
                printf("%s", m->mot);
                if (m->suivant) printf(" ");
                m = m->suivant;
            }
            printf(".\n");
        }
        p = p->suivant;
    }
    libererMap(&map);
}

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
