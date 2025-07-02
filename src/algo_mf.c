#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "algo_mf.h"

#define TAUX_APPRENTISSAGE 0.01
#define REGULARISATION 0.01
#define ITERATIONS_MAX 100

void initialiser_matrice(float matrice[][NB_FACTEURS], int nb_lignes) {
    for (int i = 0; i < nb_lignes; i++) {
        for (int j = 0; j < NB_FACTEURS; j++) {
            matrice[i][j] = ((float)rand() / RAND_MAX - 0.5) * 0.1;
        }
    }
}

void factorisation_matricielle(const Donnees *donnees, float matrice_utilisateurs[][NB_FACTEURS], float matrice_items[][NB_FACTEURS]) {

if (donnees->nb_utilisateurs > UTILISATEURS_MAX || donnees->nb_items > ITEMS_MAX) {
        printf("ERREUR: Taille données dépasse capacité\n");
        exit(EXIT_FAILURE);
    }
    srand(42);
    initialiser_matrice(matrice_utilisateurs, UTILISATEURS_MAX);
    initialiser_matrice(matrice_items, ITEMS_MAX);
    
    for (int iter = 0; iter < ITERATIONS_MAX; iter++) {
        float erreur_totale = 0;
        
        for (int n = 0; n < donnees->nb_notations; n++) {
            int u = donnees->notations[n].id_utilisateur;
            int i = donnees->notations[n].id_item;
            float note_reelle = donnees->notations[n].note;
            
            float prediction = 0;
            for (int f = 0; f < NB_FACTEURS; f++) {
                prediction += matrice_utilisateurs[u][f] * matrice_items[i][f];
            }
            
            float erreur = note_reelle - prediction;
            erreur_totale += erreur * erreur;
            
            for (int f = 0; f < NB_FACTEURS; f++) {
                float temp_u = matrice_utilisateurs[u][f];
                float temp_i = matrice_items[i][f];
                
                matrice_utilisateurs[u][f] += TAUX_APPRENTISSAGE * (erreur * temp_i - REGULARISATION * temp_u);
                matrice_items[i][f] += TAUX_APPRENTISSAGE * (erreur * temp_u - REGULARISATION * temp_i);
            }
        }
        
        if (iter % 20 == 0) {
            printf("MF - Itération %d, Erreur: %.4f\n", iter, sqrt(erreur_totale / donnees->nb_notations));
        }
    }
}

float predire_note_mf(int id_utilisateur, int id_item, float matrice_utilisateurs[][NB_FACTEURS], float matrice_items[][NB_FACTEURS]) {
    float prediction = 0;
    for (int f = 0; f < NB_FACTEURS; f++) {
        prediction += matrice_utilisateurs[id_utilisateur][f] * matrice_items[id_item][f];
    }
    
    if (prediction < 0) prediction = 0;
    if (prediction > 5) prediction = 5;
    
    return prediction;
}

void recommandation_mf(int id_utilisateur, int nb_recommandations, const Donnees *donnees, char *reponse) {
    if (id_utilisateur <= 0 || id_utilisateur >= UTILISATEURS_MAX) {
        sprintf(reponse, "ERREUR: ID utilisateur invalide (%d)", id_utilisateur);
        return;
    }
    
    printf("MF: Calcul des facteurs pour l'utilisateur %d...\n", id_utilisateur);
    
    static float matrice_utilisateurs[UTILISATEURS_MAX][NB_FACTEURS];
    static float matrice_items[ITEMS_MAX][NB_FACTEURS];
    static int factorisation_calculee = 0;
    
    if (!factorisation_calculee) {
        factorisation_matricielle(donnees, matrice_utilisateurs, matrice_items);
        factorisation_calculee = 1;
    }
    
    typedef struct {
        int id_item;
        float note_predite;
    } RecommandationMF;
    
    RecommandationMF recommandations[ITEMS_MAX];
    int nb_candidats = 0;
    
    for (int item = 1; item <= donnees->nb_items + 100; item++) {
        int deja_note = 0;
        for (int i = 0; i < donnees->nb_notations; i++) {
            if (donnees->notations[i].id_utilisateur == id_utilisateur &&
                donnees->notations[i].id_item == item) {
                deja_note = 1;
                break;
            }
        }
        
        if (!deja_note) {
            int item_existe = 0;
            for (int i = 0; i < donnees->nb_notations; i++) {
                if (donnees->notations[i].id_item == item) {
                    item_existe = 1;
                    break;
                }
            }
            
            if (item_existe) {
                float note_pred = predire_note_mf(id_utilisateur, item, matrice_utilisateurs, matrice_items);
                if (note_pred > 2.5) {
                    recommandations[nb_candidats].id_item = item;
                    recommandations[nb_candidats].note_predite = note_pred;
                    nb_candidats++;
                    
                    if (nb_candidats >= ITEMS_MAX) break;
                }
            }
        }
    }
    
    for (int i = 0; i < nb_candidats - 1; i++) {
        for (int j = i + 1; j < nb_candidats; j++) {
            if (recommandations[i].note_predite < recommandations[j].note_predite) {
                RecommandationMF temp = recommandations[i];
                recommandations[i] = recommandations[j];
                recommandations[j] = temp;
            }
        }
    }
    
    int nb_final = (nb_recommandations < nb_candidats) ? nb_recommandations : nb_candidats;
    
    if (nb_final == 0) {
        sprintf(reponse, "MF: Aucune recommandation trouvée pour l'utilisateur %d", id_utilisateur);
        return;
    }
    
    sprintf(reponse, "MF: Recommandations pour utilisateur %d: ", id_utilisateur);
    for (int i = 0; i < nb_final; i++) {
        char temp[50];
        sprintf(temp, "Item %d (note: %.1f)", recommandations[i].id_item, recommandations[i].note_predite);
        if (i < nb_final - 1) {
            strcat(temp, ", ");
        }
        strcat(reponse, temp);
    }
}
