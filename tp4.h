#ifndef TP4_H
#define TP4_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ----- Structure Position -----
typedef struct Position {
    int numeroLigne;         // ligne du mot
    int ordre;               // ordre dans la ligne
    int numeroPhrase;        // numéro de phrase
    struct Position *suivant;
} T_Position;

// ----- Structure Noeud (ABR) -----
typedef struct Noeud {
    char *mot;                    // mot stocké
    int nbOccurences;             // nombre total d'occurrences du mot
    T_Position *listePositions;   // liste chainée des positions
    struct Noeud *filsGauche;     // fils gauche
    struct Noeud *filsDroit;      // fils droit
} T_Noeud;

// ----- Structure Index -----
typedef struct Index {
    T_Noeud *racine;       // racine de l'arbre binaire de recherche
    int nbMotsDistincts;   // nombre de mots différents
    int nbMotsTotal;       // nombre total d'occurrences
} T_Index;



// ---  Fonctions de base ---

// 1. Ajouter une position dans une liste triée
T_Position *ajouterPosition(T_Position *listeP, int ligne, int ordre, int phrase);

// 2. Ajouter une occurrence dans l'index (insertion ABR)
int ajouterOccurence(T_Index *index, char *mot, int ligne, int ordre, int phrase);

// 3. Indexer un fichier texte
int indexerFichier(T_Index *index, char *filename);

// 4. Afficher l'index en ordre alphabétique
void afficherIndex(T_Index index);

// 5. Rechercher un mot dans l'index
T_Noeud* rechercherMot(T_Index index, char *mot);

// 6. Afficher toutes les phrases contenant un mot
void afficherOccurencesMot(T_Index index, char *mot);

// 7. Construire le texte à partir de l'index
void construireTexte(T_Index index, char *filename);

// --- Fonctions utilitaires ---

// Pour initialiser un index
void initialiserIndex(T_Index *index);

// Libération de tout l'arbre
void libererIndex(T_Index *index);

// Comparaison de mots insensible à la casse
int comparerMots(const char *m1, const char *m2);


#endif

