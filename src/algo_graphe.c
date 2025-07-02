#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "algo_graphe.h"

void pagerank(float matrice[NOEUDS_MAX][NOEUDS_MAX], float scores[NOEUDS_MAX], int nb_noeuds) {
    float nouveaux_scores[NOEUDS_MAX];
    float facteur_amortissement = 0.95;
    float seuil_convergence = 0.0001;

    // Normaliser la matrice (chaque ligne doit sommer à 1)
    for (int i = 0; i < nb_noeuds; i++) {
        float somme_ligne = 0;
        for (int j = 0; j < nb_noeuds; j++) {
            somme_ligne += matrice[i][j];
        }
        if (somme_ligne > 0) {
            for (int j = 0; j < nb_noeuds; j++) {
                matrice[i][j] /= somme_ligne;
            }
        }
    }

    // Algorithme PageRank
    for (int iter = 0; iter < ITERATIONS_MAX; iter++) {
        float diff_max = 0;

        for (int i = 0; i < nb_noeuds; i++) {
            nouveaux_scores[i] = 0;
            for (int j = 0; j < nb_noeuds; j++) {
                nouveaux_scores[i] += scores[j] * matrice[j][i];
            }
            nouveaux_scores[i] = facteur_amortissement * nouveaux_scores[i] + 
                               (1 - facteur_amortissement) / nb_noeuds;
            
            float diff = fabs(nouveaux_scores[i] - scores[i]);
            if (diff > diff_max) diff_max = diff;
        }
        
        // Copier les nouveaux scores
        for (int i = 0; i < nb_noeuds; i++) {
            scores[i] = nouveaux_scores[i];
        }
        
        // Vérifier la convergence
        if (diff_max < seuil_convergence) {
            printf("PageRank convergé après %d itérations\n", iter + 1);
            break;
        }
    }
}

void construire_graphe_bipartite(const Donnees *donnees, float matrice[NOEUDS_MAX][NOEUDS_MAX], int *nb_noeuds) {
    // Réinitialiser la matrice
    for (int i = 0; i < NOEUDS_MAX; i++) {
        for (int j = 0; j < NOEUDS_MAX; j++) {
            matrice[i][j] = 0;
        }
    }

    int max_noeud = 0;

    // Créer des connexions basées sur les notes
    for (int n = 0; n < donnees->nb_notations; n++) {
        int noeud_utilisateur = donnees->notations[n].id_utilisateur;
        int noeud_item = donnees->notations[n].id_item + 1000;
        float note = donnees->notations[n].note;

        // Pondérer les connexions par la note
        float poids = note / 5.0; // Normaliser entre 0 et 1
        
        // Connexion bidirectionnelle utilisateur <-> item
        if (noeud_utilisateur < NOEUDS_MAX && noeud_item < NOEUDS_MAX) {
            matrice[noeud_utilisateur][noeud_item] = poids;
            matrice[noeud_item][noeud_utilisateur] = poids;
            
            if (noeud_utilisateur > max_noeud) max_noeud = noeud_utilisateur;
            if (noeud_item > max_noeud) max_noeud = noeud_item;
        }
    }

    // Ajouter des connexions entre utilisateurs similaires
    for (int u1 = 1; u1 <= donnees->nb_utilisateurs && u1 < NOEUDS_MAX; u1++) {
        for (int u2 = u1 + 1; u2 <= donnees->nb_utilisateurs && u2 < NOEUDS_MAX; u2++) {
            // Calculer similarité simple basée sur les items en commun
            int items_communs = 0;
            float diff_totale = 0;

            for (int i = 0; i < donnees->nb_notations; i++) {
                if (donnees->notations[i].id_utilisateur == u1) {
                    int item = donnees->notations[i].id_item;
                    float note1 = donnees->notations[i].note;
                    
                    for (int j = 0; j < donnees->nb_notations; j++) {
                        if (donnees->notations[j].id_utilisateur == u2 && 
                            donnees->notations[j].id_item == item) {
                            float note2 = donnees->notations[j].note;
                            diff_totale += fabs(note1 - note2);
                            items_communs++;
                            break;
                        }
                    }
                }
            }
            
            if (items_communs > 0) {
                float similarite = 1.0 / (1.0 + diff_totale / items_communs);
                if (similarite > 0.5) { // Seuil de similarité
                    matrice[u1][u2] = similarite * 0.3; // Poids plus faible
                    matrice[u2][u1] = similarite * 0.3;
                }
            }
        }
    }

    *nb_noeuds = max_noeud + 1;
}

void recommandation_graphe(int id_utilisateur, int nb_recommandations, const Donnees *donnees, char *reponse) {
    if (id_utilisateur <= 0 || id_utilisateur >= UTILISATEURS_MAX) {
        sprintf(reponse, "ERREUR: ID utilisateur invalide (%d)", id_utilisateur);
        return;
    }

    printf("PageRank: Construction du graphe pour l'utilisateur %d...\n", id_utilisateur);

    static float matrice[NOEUDS_MAX][NOEUDS_MAX];
    static float scores[NOEUDS_MAX];
    static int nb_noeuds = 0;
    static int graphe_construit = 0;

    // Construire le graphe une seule fois
    if (!graphe_construit) {
        construire_graphe_bipartite(donnees, matrice, &nb_noeuds);
        
        // Initialiser les scores uniformément
        for (int i = 0; i < nb_noeuds; i++) {
            scores[i] = 1.0 / nb_noeuds;
        }
        
        // Calculer PageRank
        pagerank(matrice, scores, nb_noeuds);
        graphe_construit = 1;
    }

    typedef struct {
        int id_item;
        float score;
    } RecommandationGraphe;

    RecommandationGraphe recommandations[ITEMS_MAX];
    int nb_candidats = 0;

    // Collecter les scores des items (nœuds > 1000)
    for (int noeud = 1001; noeud < nb_noeuds && noeud < NOEUDS_MAX; noeud++) {
        int id_item = noeud - 1000;

        // Vérifier que l'utilisateur n'a pas déjà noté cet item
        int deja_note = 0;
        for (int i = 0; i < donnees->nb_notations; i++) {
            if (donnees->notations[i].id_utilisateur == id_utilisateur && 
                donnees->notations[i].id_item == id_item) {
                deja_note = 1;
                break;
            }
        }
        
        if (!deja_note && scores[noeud] > 0) {
            // Pondérer par la connexion avec l'utilisateur
            float score_final = scores[noeud];
            if (id_utilisateur < NOEUDS_MAX) {
                score_final *= (1.0 + matrice[id_utilisateur][noeud] * 2.0);
            }
            
            recommandations[nb_candidats].id_item = id_item;
            recommandations[nb_candidats].score = score_final;
            nb_candidats++;
            
            if (nb_candidats >= ITEMS_MAX) break;
        }
    }

    // Trier par score décroissant
    for (int i = 0; i < nb_candidats - 1; i++) {
        for (int j = i + 1; j < nb_candidats; j++) {
            if (recommandations[i].score < recommandations[j].score) {
                RecommandationGraphe temp = recommandations[i];
                recommandations[i] = recommandations[j];
                recommandations[j] = temp;
            }
        }
    }

    // Limiter le nombre de recommandations
    int nb_final = (nb_recommandations < nb_candidats) ? nb_recommandations : nb_candidats;

    if (nb_final == 0) {
        sprintf(reponse, "GRAPHE: Aucune recommandation trouvée pour l'utilisateur %d", id_utilisateur);
        return;
    }

    // Formater la réponse
    sprintf(reponse, "GRAPHE: Recommandations pour utilisateur %d: ", id_utilisateur);
    for (int i = 0; i < nb_final; i++) {
        char temp[50];
        sprintf(temp, "Item %d (score: %.3f)", recommandations[i].id_item, recommandations[i].score);
        if (i < nb_final - 1) {
            strcat(temp, ", ");
        }
        strcat(reponse, temp);
    }
}
