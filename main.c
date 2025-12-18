/*
 * main.c
 * Point d'entrée du programme
 */

#include "tp4.h"

void viderBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    T_Index index;
    initIndex(&index); // Initialisation indispensable

    int choix;
    char filename[100];
    char motRecherche[100];
    int res;

    do {
        printf("\n--- MENU ABR (Optimisation Maximale) ---\n");
        printf("1. Charger fichier\n");
        printf("2. Infos Index\n");
        printf("3. Afficher Index\n");
        printf("4. Rechercher mot\n");
        printf("5. Afficher occurences (Opti O(h+k))\n");
        printf("6. Reconstruire texte (Opti O(P))\n");
        printf("7. Quitter\n");
        printf("Choix : ");
        
        if (scanf("%d", &choix) != 1) { viderBuffer(); continue; }
        viderBuffer();

        switch (choix) {
            case 1:
                printf("Nom du fichier : ");
                scanf("%s", filename);
                // Libération propre avant rechargement
                if (index.racine) { libererIndex(&index); initIndex(&index); }
                res = indexerFichier(&index, filename);
                printf("%d mots indexes.\n", res);
                break;
            case 2:
                printf("Mots distincts : %d\n", index.nbMotsDistincts);
                printf("Mots total     : %d\n", index.nbMotsTotal);
                printf("Phrases cached : %d slots\n", index.nbPhrasesCapacite);
                break;
            case 3:
                afficherIndex(index);
                break;
            case 4:
                printf("Mot a chercher : ");
                scanf("%s", motRecherche);
                T_Noeud* n = rechercherMot(index, motRecherche);
                if (n) printf("Trouve ! Occurences : %d\n", n->nbOccurences);
                else printf("Mot inconnu.\n");
                break;
            case 5:
                printf("Mot a analyser : ");
                scanf("%s", motRecherche);
                afficherOccurencesMot(index, motRecherche);
                break;
            case 6:
                printf("Fichier de sortie : ");
                scanf("%s", filename);
                construireTexte(index, filename);
                break;
            case 7:
                printf("Fermeture et liberation memoire...\n");
                libererIndex(&index);
                break;
            default:
                printf("Choix invalide.\n");
        }
    } while (choix != 7);

    return 0;
}