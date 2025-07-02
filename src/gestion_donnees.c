#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gestion_donnees.h"

int charger_donnees(const char *nom_fichier, Donnees *donnees) {
    FILE *fichier = fopen(nom_fichier, "r");
    if (fichier == NULL) {
        printf("Fichier %s non trouvé. Création de données d'exemple...\n", nom_fichier);
        creer_donnees_exemple(donnees);
        return 0;
    }
    
    donnees->nb_notations = 0;
    donnees->nb_utilisateurs = 0;
    donnees->nb_items = 0;
    
    char ligne[256];
    int max_utilisateur = 0, max_item = 0;
    
    if (fgets(ligne, sizeof(ligne), fichier) != NULL) {
        if (strpbrk(ligne, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") != NULL) {
            // Ignorer la ligne d'en-tête
        } else {
            fseek(fichier, 0, SEEK_SET);
        }
    }
    
    while (fgets(ligne, sizeof(ligne), fichier) != NULL && donnees->nb_notations < NOTATIONS_MAX) {
        int id_utilisateur, id_item;
        float note;
        
        if (sscanf(ligne, "%d,%d,%f", &id_utilisateur, &id_item, &note) == 3 ||
            sscanf(ligne, "%d;%d;%f", &id_utilisateur, &id_item, &note) == 3 ||
            sscanf(ligne, "%d %d %f", &id_utilisateur, &id_item, &note) == 3) {
            
            if (id_utilisateur > 0 && id_item > 0 && note >= 0.0 && note <= 5.0) {
                donnees->notations[donnees->nb_notations].id_utilisateur = id_utilisateur;
                donnees->notations[donnees->nb_notations].id_item = id_item;
                donnees->notations[donnees->nb_notations].note = note;
                donnees->nb_notations++;
                
                if (id_utilisateur > max_utilisateur) max_utilisateur = id_utilisateur;
                if (id_item > max_item) max_item = id_item;
            }
        }
    }
    
    fclose(fichier);
    
    donnees->nb_utilisateurs = max_utilisateur;
    donnees->nb_items = max_item;
    
    printf("Données chargées: %d notations, %d utilisateurs, %d items\n",
           donnees->nb_notations, donnees->nb_utilisateurs, donnees->nb_items);
    
    return 0;
}

void creer_donnees_exemple(Donnees *donnees) {
    int donnees_exemple[][3] = {
        {1, 101, 5}, {1, 102, 3}, {1, 103, 4}, {1, 104, 2},
        {2, 101, 4}, {2, 102, 5}, {2, 105, 3}, {2, 106, 4},
        {3, 103, 5}, {3, 104, 4}, {3, 105, 2}, {3, 107, 5},
        {4, 101, 3}, {4, 106, 4}, {4, 107, 3}, {4, 108, 5},
        {5, 102, 4}, {5, 103, 3}, {5, 108, 4}, {5, 109, 2}
    };
    
    int nb_donnees = sizeof(donnees_exemple) / sizeof(donnees_exemple[0]);
    
    donnees->nb_notations = nb_donnees;
    donnees->nb_utilisateurs = 5;
    donnees->nb_items = 9;
    
    for (int i = 0; i < nb_donnees; i++) {
        donnees->notations[i].id_utilisateur = donnees_exemple[i][0];
        donnees->notations[i].id_item = donnees_exemple[i][1];
        donnees->notations[i].note = donnees_exemple[i][2];
    }
}

void calculer_moyennes(Donnees *donnees) {
    int compteurs[UTILISATEURS_MAX] = {0};
    
    for (int i = 0; i < UTILISATEURS_MAX; i++) {
        donnees->moyennes_utilisateurs[i] = 0.0;
    }
    
    for (int i = 0; i < donnees->nb_notations; i++) {
        int id_user = donnees->notations[i].id_utilisateur;
        if (id_user > 0 && id_user < UTILISATEURS_MAX) {
            donnees->moyennes_utilisateurs[id_user] += donnees->notations[i].note;
            compteurs[id_user]++;
        }
    }
    
    for (int i = 0; i < UTILISATEURS_MAX; i++) {
        if (compteurs[i] > 0) {
            donnees->moyennes_utilisateurs[i] /= compteurs[i];
        }
    }
}

void afficher_statistiques(const Donnees *donnees) {
    printf("\n=== STATISTIQUES DES DONNÉES ===\n");
    printf("Nombre total de notations: %d\n", donnees->nb_notations);
    printf("Nombre d'utilisateurs: %d\n", donnees->nb_utilisateurs);
    printf("Nombre d'items: %d\n", donnees->nb_items);
    
    float somme = 0, min_note = 5.0, max_note = 0.0;
    for (int i = 0; i < donnees->nb_notations; i++) {
        float note = donnees->notations[i].note;
        somme += note;
        if (note < min_note) min_note = note;
        if (note > max_note) max_note = note;
    }
    
    float moyenne = somme / donnees->nb_notations;
    printf("Note moyenne: %.2f\n", moyenne);
    printf("Note minimale: %.1f\n", min_note);
    printf("Note maximale: %.1f\n", max_note);
    printf("Densité: %.2f%%\n", (100.0 * donnees->nb_notations) / (donnees->nb_utilisateurs * donnees->nb_items));
    printf("================================\n\n");
}