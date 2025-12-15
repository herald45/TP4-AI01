/*
 * tp4.h
 * Déclarations des structures et prototypes pour l'indexation de texte par ABR.
 */

#ifndef TP4_H
#define TP4_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --- A. Structures de données (Source: 9-27) --- */

// Structure représentant une position unique d'un mot
typedef struct Position {
    int numeroLigne;
    int ordre;
    int numeroPhrase;
    struct Position* suivant;
} T_Position;

// Structure représentant un nœud de l'ABR
typedef struct Noeud {
    char* mot;
    int nbOccurences;
    T_Position* listePositions;
    struct Noeud* filsGauche;
    struct Noeud* filsDroit;
} T_Noeud;

// Structure représentant l'Index global
typedef struct Index {
    T_Noeud* racine;
    int nbMotsDistincts;
    int nbMotsTotal;
} T_Index;

/* --- B. Prototypes des fonctions --- */

// 1. Gestion des positions
T_Position* ajouterPosition(T_Position* listeP, int ligne, int ordre, int phrase);

// 2. Gestion de l'ABR (Ajout)
int ajouterOccurence(T_Index* index, char* mot, int ligne, int ordre, int phrase);

// 3. Indexation du fichier
int indexerFichier(T_Index* index, char* filename);

// 4. Affichage de l'index (Parcours infixe)
void afficherIndex(T_Index index);

// 5. Recherche
T_Noeud* rechercherMot(T_Index index, char* mot);

// 6. Affichage des phrases contenant un mot (Optimisé)
void afficherOccurencesMot(T_Index index, char* mot);

// 7. Reconstruction du texte (Optimisé)
void construireTexte(T_Index index, char* filename);

// Utilitaires de gestion mémoire
void libererIndex(T_Index* index);

#endif
