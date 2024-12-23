#include <stdio.h>
#include <stdlib.h>
#include  <stdbool.h>
#include  <string.h>
#define Total_block 100

typedef struct {
    char name[20];
    float price;
    int id;
}produit;

typedef struct {
    produit enregisrement[3];
    int nb_enregistrement;
    int next;
}Bloc;

typedef struct {
    FILE *Ms;
    char T[sizeof(Bloc)*Total_block];
    int nm_bloc;
    bool occupied;
}Ms;
typedef struct {
    char Nom_du_fichier[30];
    int Taille_du_fichier;
    int nb_enregistrement;
    int adresse_firstblock;
    int org_globale;
    int org_interne;
}mt;

typedef struct {
    bool occupied;
    int nm_bloc;
}allocation;


void initialiserMs(Ms *ms, const char *nomFichier) {
    // Vérifie si le pointeur Ms est valide
    if (ms == NULL) {
        fprintf(stderr, "Erreur : Le pointeur Ms est NULL.\n");
        return;
    }

    // Initialisation du fichier
    ms->Ms = fopen(nomFichier, "wb+"); // Ouvre le fichier en lecture/écriture binaire
    if (ms->Ms == NULL) {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s.\n", nomFichier);
        exit(EXIT_FAILURE);
    }

    // Initialisation des blocs en mémoire
    memset(ms->T, 0, sizeof(ms->T)); // Initialise tous les blocs à 0

    // Initialisation des autres champs
    ms->nm_bloc = 0; // Aucun bloc utilisé au départ
    ms->occupied = false; // Indique que la mémoire secondaire est vide

    printf("Initialisation de la structure Ms terminée.\n");
}


void modifierTableAllocation(Ms *ms, int blocIndex, bool occupe) {
    // Vérifie si le pointeur Ms est valide
    if (ms == NULL) {
        fprintf(stderr, "Erreur : Le pointeur Ms est NULL.\n");
        return;
    }

    // Vérifie si l'indice du bloc est valide
    if (blocIndex < 0 || blocIndex >= Total_block) {
        fprintf(stderr, "Erreur : Indice de bloc invalide (%d).\n", blocIndex);
        return;
    }

    // La table d'allocation est stockée dans les premiers Total_block octets de T
    char *tableAllocation = ms->T;

    // Met à jour l'état du bloc dans la table d'allocation
    tableAllocation[blocIndex] = occupe ? 1 : 0;

    // Marque le fichier comme modifié
    ms->occupied = true;

    printf("Bloc %d %s dans la table d'allocation.\n", blocIndex, occupe ? "occupé" : "libéré");
}

void viderMs(Ms *ms) {
    // Vérifie si le pointeur Ms est valide
    if (ms == NULL) {
        fprintf(stderr, "Erreur : Le pointeur Ms est NULL.\n");
        return;
    }

    // Remplit la mémoire secondaire avec des zéros
    memset(ms->T, 0, sizeof(ms->T));

    // Réinitialise les champs
    ms->nm_bloc = 0;
    ms->occupied = false;

    printf("Mémoire secondaire vidée avec succès.\n");
}
void chargerDepuisFichier(Ms *ms, const char *nom_fichier) {
    if (ms == NULL || nom_fichier == NULL) {
        fprintf(stderr, "Erreur : Pointeur invalide.\n");
        return;
    }

    FILE *f = fopen(nom_fichier, "a+");
    if (f == NULL) {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s.\n", nom_fichier);
        return;
    }

    Bloc buffer; // Buffer pour stocker un bloc d'enregistrements
    buffer.nb_enregistrement = 0;

    produit temp;
    while (fscanf(f, "%19s %2s %9s", temp.name, temp.price, temp.id) == 3) {
        // Ajouter l'enregistrement au buffer
        buffer.enregisrement[buffer.nb_enregistrement++] = temp;

        // Si le buffer est plein, on le sauvegarde dans la mémoire secondaire
        if (buffer.nb_enregistrement == 5) {
            if (ms->nm_bloc >= Total_block) {
                fprintf(stderr, "Erreur : Pas assez d'espace dans la mémoire secondaire.\n");
                fclose(f);
                return;
            }

            memcpy(ms->T + ms->nm_bloc * sizeof(Bloc), &buffer, sizeof(Bloc));
            modifierTableAllocation(ms, ms->nm_bloc, true);
            ms->nm_bloc++;

            buffer.nb_enregistrement = 0; // Réinitialiser le buffer
        }
    }

    // Sauvegarder le reste du buffer (si non vide)
    if (buffer.nb_enregistrement> 0) {
        if (ms->nm_bloc >= Total_block) {
            fprintf(stderr, "Erreur : Pas assez d'espace dans la mémoire secondaire.\n");
            fclose(f);
            return;
        }

        memcpy(ms->T + ms->nm_bloc * sizeof(Bloc), &buffer, sizeof(Bloc));
        modifierTableAllocation(ms, ms->nm_bloc, true);
        ms->nm_bloc++;
    }

    fclose(f);
    printf("Fichier %s chargé dans la mémoire secondaire.\n", nom_fichier);
}
void chargerDansBuffer(Ms *ms) {
    Bloc buffer;
    produit temp;
    buffer.nb_enregistrement = 0;  // Réinitialiser le nombre d'enregistrements

    while (buffer.nb_enregistrement < 3) {
        printf("Entrez le nom du produit : ");
        scanf("%20s", &temp.name); // Saisie du nom du produit


        printf("Entrez le prix du produit : ");
        scanf("%f", &temp.price); // Saisie du prix

        printf("Entrez l'ID du produit : ");
        scanf("%d", &temp.id); // Saisie de l'ID

        // Ajouter le produit au buffer
        buffer.enregisrement[buffer.nb_enregistrement++] = temp;
    }
        // Si le buffer est plein, le sauvegarder dans la mémoire secondaire
        if (buffer.nb_enregistrement == 3) {
            if (ms->nm_bloc >= Total_block) {
                fprintf(stderr, "Erreur : Pas assez d'espace dans la mémoire secondaire.\n");
                return;
            }

            // Sauvegarder le buffer dans la mémoire secondaire
            memcpy(ms->T + ms->nm_bloc * sizeof(Bloc), &buffer, sizeof(Bloc));
            modifierTableAllocation(ms, ms->nm_bloc, true); // Marquer le bloc comme occupé
            ms->nm_bloc++; // Incrémenter le nombre de blocs

            // Réinitialiser le buffer
            buffer.nb_enregistrement = 0;
        }


    // Sauvegarder le reste du buffer (s'il reste des produits)
    if (buffer.nb_enregistrement > 0) {
        if (ms->nm_bloc >= Total_block) {
            fprintf(stderr, "Erreur : Pas assez d'espace dans la mémoire secondaire.\n");
            return;
        }
        memcpy(ms->T + ms->nm_bloc * sizeof(Bloc), &buffer, sizeof(Bloc));
        modifierTableAllocation(ms, ms->nm_bloc, true); // Marquer le bloc comme occupé
        ms->nm_bloc++; // Incrémenter le nombre de blocs
    }

    printf("Les données ont été chargées dans la mémoire secondaire.\n");
}
void chargerMs(Ms *ms, const char *nomFichier) {
    if (ms == NULL || nomFichier == NULL) {
        fprintf(stderr, "Erreur : Pointeur invalide.\n");
        return;
    }

    // Ouvre le fichier binaire en lecture
    ms->Ms = fopen(nomFichier, "a+");  // Ouvre le fichier en mode lecture binaire
    if (ms->Ms == NULL) {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s.\n", nomFichier);
        return;
    }

    // Réinitialiser l'état de la mémoire secondaire
    memset(ms->T, 0, sizeof(ms->T));  // Remplir les blocs avec des zéros
    ms->nm_bloc = 0;  // Aucun bloc utilisé au début
    ms->occupied = false;  // Initialiser l'occupation à faux

    Bloc buffer;  // Buffer pour lire un bloc
    int blocIndex = 0;  // Indice pour le bloc actuel
    while (fread(&buffer, sizeof(Bloc), 1, ms->Ms) == 1) {
        // Si le bloc est valide, le charger dans la mémoire secondaire
        if (buffer.nb_enregistrement > 0) {
            memcpy(ms->T + ms->nm_bloc * sizeof(Bloc), &buffer, sizeof(Bloc));

            // Mettre à jour la table d'allocation
            modifierTableAllocation(ms, ms->nm_bloc, true);

            ms->nm_bloc++;  // Incrémenter le compteur de blocs utilisés
        }
    }

    fclose(ms->Ms);  // Fermer le fichier
    printf("Mémoire secondaire chargée depuis le fichier %s.\n", nomFichier);
}
void afficherElementsMs(Ms *ms) {
    printf("Affichage des éléments dans la mémoire secondaire :\n");

    // Parcourir tous les blocs dans la mémoire secondaire
    for (int i = 0; i < ms->nm_bloc; i++) {
        // Lire le bloc courant depuis la mémoire secondaire
        Bloc bloc;
        memcpy(&bloc, ms->T + i * sizeof(Bloc), sizeof(Bloc));

        printf("Bloc %d:\n", i + 1);

        // Parcourir les enregistrements dans le bloc
        for (int j = 0; j < bloc.nb_enregistrement; j++) {
            produit temp = bloc.enregisrement[j]; // Accéder à l'enregistrement j

            // Afficher les détails du produit
            printf("  Produit %d:\n", j + 1);
            printf("    Nom   : %s\n", temp.name);
            printf("    Prix  : %f\n", temp.price); // Utilisez temp.price[0] pour afficher le prix
            printf("    ID    : %d\n", temp.id);    // Utilisez temp.id[0] pour afficher l'ID
        }
    }
}

    #include <stdbool.h>
#include <string.h>

// Structure pour représenter le résultat de la recherche
typedef struct {
    int bloc;     // Numéro du bloc
    int position; // Position dans le bloc
    bool trouve;  // Indique si l'enregistrement est trouvé
} ResultatRecherche;

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



;

// Fonction de recherche utilisant un buffer
ResultatRecherche rechercherParIDAvecBuffer(Ms *ms, int idRecherche, bool globaleChainee, bool interneTriee) {
    ResultatRecherche resultat = {-1, -1, false}; // Initialisation du résultat

    // Création d'un buffer pour contenir plusieurs blocs
    Bloc buffer[3];

    int blocCourant = 0;

    while (blocCourant < ms->nm_bloc) {
        // Charger les blocs dans le buffer
        int blocsCharges = 0;
        for (int i = 0; i < 3 && blocCourant < ms->nm_bloc; i++, blocCourant++) {
            memcpy(&buffer[i], ms->T + blocCourant * sizeof(Bloc), sizeof(Bloc));
            blocsCharges++;
        }

        // Recherche dans les blocs du buffer
        for (int i = 0; i < blocsCharges; i++) {
            Bloc *bloc = &buffer[i];

            if (interneTriee) {
                // Recherche binaire dans un bloc trié
                int gauche = 0, droite = bloc->nb_enregistrement - 1;
                while (gauche <= droite) {
                    int milieu = (gauche + droite) / 2;
                    if (bloc->enregisrement[milieu].id == idRecherche) {
                        resultat.bloc = blocCourant - blocsCharges + i; // Bloc correspondant
                        resultat.position = milieu;                     // Position dans le bloc
                        resultat.trouve = true;
                        return resultat;
                    } else if (bloc->enregisrement[milieu].id < idRecherche) {
                        gauche = milieu + 1;
                    } else {
                        droite = milieu - 1;
                    }
                }
            } else {
                // Recherche linéaire dans un bloc non trié
                for (int j = 0; j < bloc->nb_enregistrement; j++) {
                    if (bloc->enregisrement[j].id == idRecherche) {
                        resultat.bloc = blocCourant - blocsCharges + i; // Bloc correspondant
                        resultat.position = j;                          // Position dans le bloc
                        resultat.trouve = true;
                        return resultat;
                    }
                }
            }
        }
    }

    // Si aucun résultat n'a été trouvé
    printf("Enregistrement avec ID %d introuvable.\n", idRecherche);
    return resultat;
}





int main() {
    Ms ms;

    // Initialiser la mémoire secondaire
    initialiserMs(&ms, "ms.txt");

    // Charger des produits dans le buffer
    chargerDansBuffer(&ms);

    // Afficher les éléments de la mémoire secondaire
    afficherElementsMs(&ms);

    // Rechercher un enregistrement par ID
    int idRecherche;
    printf("Entrez l'ID à rechercher : ");
    scanf("%d", &idRecherche);

    // Effectuer la recherche avec buffer
    ResultatRecherche resultat = rechercherParIDAvecBuffer(&ms, idRecherche, false, false);

    // Afficher le résultat
    if (resultat.trouve) {
        printf("Enregistrement trouvé dans le bloc %d, position %d.\n", resultat.bloc, resultat.position);
    } else {
        printf("Enregistrement non trouvé.\n");
    }

    return 0;
}





/*
void initialiserLaMs( Ms *Ms ){

    printf("Initialiser la Ms ...\n");

    for(int i=0; i<sizeof(bloc)*Total_block ;i++){
        Ms->T[i]='\0';
    }
    for(int i=0; i<Total_block ;i++){
        Ms->nm_bloc=i;
    }

}


 void MajTabledallocation(Ms *Ms ) {
    printf("MajTabledallocation ...\n");
    allocation T[Total_block-1];
    bloc buffer;

    for(int j=1; j<Total_block ;j++) {
        fseek(Ms, sizeof(buffer), SEEK_SET+1);
        for(int i = j*sizeof(bloc); i< (j+1)*sizeof(bloc) ;i++) {
            if(Ms->T[i]!='\0') {
                T[j].occupied=true;
                break;
            }

        }
        T[j].nm_bloc = j;
        fwrite(&buffer, sizeof(buffer), 1, Ms->T[0]);
    }



/*
int main(){
    Ms Ms;

    initialiserLaMs(&Ms);

    produit p;
    strcpy(p.age, "25");
    strcpy(p.id, "100");
    strcpy(p.name,"Ahmed");

    memcpy(&Ms.T[0], &p, sizeof(produit));


    for(int i=0; i<sizeof(bloc) ;i++){
        memcpy(&Ms.T[i], &p, sizeof(produit));
    }

    for(int i=0; i<5 ;i++){
        printf("Ms->T[%d] = %c\n",i,Ms.T[i]);
    }




    return 0;
}int main() {
    Ms Ms;

    initialiserLaMs(&Ms);
    produit p;

    for (int i = 0; i < sizeof(bloc); i++) {
        printf("Ms->T[%d] = %c\n", i, Ms.T[i]);
    }


    for(int i=0; i<5 ;i++) {

        printf("Entrez le nom : ");
        scanf(" %20s", p.name);


        printf("Entrez l'âge : ");
        scanf(" %3s", p.age);


        printf("Entrez l'ID : ");
        scanf(" %10s", p.id);


        memcpy(&Ms.T[i * sizeof(produit)], &p, sizeof(produit));
    }


    for (int i = 0; i < sizeof(bloc); i++) {
        printf("Ms->T[%d] = %c\n", i, Ms.T[i]);
    }


    return 0;
}


*/
