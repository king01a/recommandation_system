
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "algo_knn.h"

typedef struct {
    int id_utilisateur;
    float similarite;
} VoisinSimilaire;

int comparer_voisins(const void *a, const void *b) {
    VoisinSimilaire *va = (VoisinSimilaire *)a;
    VoisinSimilaire *vb = (VoisinSimilaire *)b;
    
    if (va->similarite < vb->similarite) return 1;
    if (va->similarite > vb->similarite) return -1;
    return 0;
}

float calculer_similarite_pearson(int utilisateur1, int utilisateur2, const Donnees *donnees) {
    if (utilisateur1 == utilisateur2) return 1.0;
    
    float somme1 = 0, somme2 = 0, somme1_carres = 0, somme2_carres = 0, somme_produits = 0;
    int nb_commun = 0;
    
    for (int i = 0; i < donnees->nb_notations; i++) {
        if (donnees->notations[i].id_utilisateur == utilisateur1) {
            int item_commun = donnees->notations[i].id_item;
            float note1 = donnees->notations[i].note;
            
            for (int j = 0; j < donnees->nb_notations; j++) {
                if (donnees->notations[j].id_utilisateur == utilisateur2 && 
                    donnees->notations[j].id_item == item_commun) {
                    
                    float note2 = donnees->notations[j].note;
                    float ecart1 = note1 - donnees->moyennes_utilisateurs[utilisateur1];
                    float ecart2 = note2 - donnees->moyennes_utilisateurs[utilisateur2];
                    
                    somme1 += ecart1;
                    somme2 += ecart2;
                    somme1_carres += ecart1 * ecart1;
                    somme2_carres += ecart2 * ecart2;
                    somme_produits += ecart1 * ecart2;
                    nb_commun++;
                    break;
                }
            }
        }
    }
    
    if (nb_commun < 2) return 0.0;
    
    float numerateur = somme_produits - (somme1 * somme2 / nb_commun);
    float denominateur = sqrt((somme1_carres - (somme1 * somme1 / nb_commun)) *
                        (somme2_carres - (somme2 * somme2 / nb_commun)));
    
    if (denominateur == 0) return 0.0;
    
    return numerateur / denominateur;
}
int utilisateur_a_note_item(int id_utilisateur, int id_item, const Donnees *donnees) {
    for (int i = 0; i < donnees->nb_notations; i++) {
        if (donnees->notations[i].id_utilisateur == id_utilisateur &&
            donnees->notations[i].id_item == id_item) {
            return 1;
        }
    }
    return 0;
}

float predire_note_knn(int id_utilisateur, int id_item, const Donnees *donnees, int k) {
    VoisinSimilaire voisins[UTILISATEURS_MAX];
    int nb_voisins = 0;
    
    for (int i = 0; i < donnees->nb_notations; i++) {
        if (donnees->notations[i].id_item == id_item &&
            donnees->notations[i].id_utilisateur != id_utilisateur) {
            
            int autre_utilisateur = donnees->notations[i].id_utilisateur;
            int deja_present = 0;
            for (int j = 0; j < nb_voisins; j++) {
                if (voisins[j].id_utilisateur == autre_utilisateur) {
                    deja_present = 1;
                    break;
                }
            }
            
            if (!deja_present) {
                float sim = calculer_similarite_pearson(id_utilisateur, autre_utilisateur, donnees);
                if (sim > 0) {
                    voisins[nb_voisins].id_utilisateur = autre_utilisateur;
                    voisins[nb_voisins].similarite = sim;
                    nb_voisins++;
                }
            }
        }
    }
    
    if (nb_voisins == 0) {
        return donnees->moyennes_utilisateurs[id_utilisateur];
    }
    
    qsort(voisins, nb_voisins, sizeof(VoisinSimilaire), comparer_voisins);
    
    int k_effectif = (k < nb_voisins) ? k : nb_voisins;
    float somme_ponderee = 0.0;
    float somme_poids = 0.0;
    
    for (int i = 0; i < k_effectif; i++) {
        int voisin = voisins[i].id_utilisateur;
        float poids = voisins[i].similarite;
        
        for (int j = 0; j < donnees->nb_notations; j++) {
            if (donnees->notations[j].id_utilisateur == voisin && 
                donnees->notations[j].id_item == id_item) {
                
                somme_ponderee += poids * donnees->notations[j].note;
                somme_poids += poids;
                break;
            }
        }
    }
    
    if (somme_poids > 0) {
        return somme_ponderee / somme_poids;
    }
    
    return donnees->moyennes_utilisateurs[id_utilisateur];
}

void recommandation_knn(int id_utilisateur, int nb_recommandations, const Donnees *donnees, char *reponse) {
    if (id_utilisateur <= 0 || id_utilisateur >= UTILISATEURS_MAX) {
        sprintf(reponse, "ERREUR: ID utilisateur invalide (%d)", id_utilisateur);
        return;
    }
    
    typedef struct {
        int id_item;
        float note_predite;
    } Recommandation;
    
    Recommandation recommandations[ITEMS_MAX];
    int nb_candidats = 0;
    int k = 5;
    
    for (int item = 1; item <= donnees->nb_items + 100; item++) {
        if (!utilisateur_a_note_item(id_utilisateur, item, donnees)) {
            int item_existe = 0;
            for (int i = 0; i < donnees->nb_notations; i++) {
                if (donnees->notations[i].id_item == item) {
                    item_existe = 1;
                    break;
                }
            }
            
            if (item_existe) {
                float note_pred = predire_note_knn(id_utilisateur, item, donnees, k);
                if (note_pred > 0) {
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
                Recommandation temp = recommandations[i];
                recommandations[i] = recommandations[j];
                recommandations[j] = temp;
            }
        }
    }
    
    int nb_final = (nb_recommandations < nb_candidats) ? nb_recommandations : nb_candidats;
    
    if (nb_final == 0) {
        sprintf(reponse, "KNN: Aucune recommandation trouvée pour l'utilisateur %d", id_utilisateur);
        return;
    }
    
    sprintf(reponse, "KNN: Recommandations pour utilisateur %d: ", id_utilisateur);
    for (int i = 0; i < nb_final; i++) {
        char temp[50];
        sprintf(temp, "Item %d (note: %.1f)", recommandations[i].id_item, recommandations[i].note_predite);
        if (i < nb_final - 1) {
            strcat(temp, ", ");
        }
        strcat(reponse, temp);
    }
}
