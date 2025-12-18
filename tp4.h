/*
 * tp4.h
 * Version : GOD TIER FINAL
 * Optimisations : Queue d'insertion + Cache de phrases + Arithmétique Pointeurs
 */

#ifndef TP4_H
#define TP4_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h> // Pour strcasecmp

/* --- Structures --- */

typedef struct Position {
    int numeroLigne;
    int ordre;
    int numeroPhrase;
    struct Position* suivant;
} T_Position;

typedef struct Noeud {
    char* mot;
    int nbOccurences;
    T_Position* listePositions;
    
    // Optimisation 1 : Insertion en O(1)
    T_Position* dernierePosition; 
    
    struct Noeud* filsGauche;
    struct Noeud* filsDroit;
} T_Noeud;

typedef struct Index {
    T_Noeud* racine;
    int nbMotsDistincts;
    int nbMotsTotal;

    // Optimisation 2 : Cache de phrases complètes (Accès O(1))
    char** tabPhrases; 
    int nbPhrasesCapacite;
} T_Index;

/* --- Prototypes --- */

void initIndex(T_Index* index);
T_Position* ajouterPosition(T_Position* listeP, int ligne, int ordre, int phrase);
int ajouterOccurence(T_Index* index, char* mot, int ligne, int ordre, int phrase);
int indexerFichier(T_Index* index, char* filename);
void afficherIndex(T_Index index);
T_Noeud* rechercherMot(T_Index index, char* mot);
void afficherOccurencesMot(T_Index index, char* mot);
void construireTexte(T_Index index, char* filename);
void libererIndex(T_Index* index);

#endif