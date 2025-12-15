#include "tp4.h"

void viderBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    T_Index index = {NULL, 0, 0};
    int choix;
    char filename[100];
    char motRecherche[100];
    int res;

    do {
        printf("\n--- MENU ABR ---\n");
        printf("1. Charger fichier\n");
        printf("2. Infos Index\n");
        printf("3. Afficher Index (Format Arbre)\n");
        printf("4. Rechercher mot\n");
        printf("5. Afficher occurences (Contexte)\n");
        printf("6. Reconstruire texte\n");
        printf("7. Quitter\n");
        printf("Choix : ");
        
        if (scanf("%d", &choix) != 1) {
            viderBuffer(); continue;
        }
        viderBuffer();

        switch (choix) {
            case 1:
                printf("Fichier : ");
                scanf("%s", filename);
                if (index.racine) libererIndex(&index);
                res = indexerFichier(&index, filename);
                printf("%d mots charges.\n", res);
                break;
            case 2:
                printf("Mots distincts: %d\nTotal mots: %d\n", index.nbMotsDistincts, index.nbMotsTotal);
                break;
            case 3:
                afficherIndex(index);
                break;
            case 4:
                printf("Mot : ");
                scanf("%s", motRecherche);
                T_Noeud* n = rechercherMot(index, motRecherche);
                if (n) printf("Trouve ! %d fois.\n", n->nbOccurences);
                else printf("Inconnu.\n");
                break;
            case 5:
                printf("Mot : ");
                scanf("%s", motRecherche);
                afficherOccurencesMot(index, motRecherche);
                break;
            case 6:
                printf("Sortie : ");
                scanf("%s", filename);
                construireTexte(index, filename);
                break;
            case 7:
                libererIndex(&index);
                break;
        }
    } while (choix != 7);

    return 0;
}
