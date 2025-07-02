#ifndef ALGO_GRAPHE_H
#define ALGO_GRAPHE_H
#include "gestion_donnees.h"

#define NOEUDS_MAX 2000
#define ITERATIONS_MAX 100

void recommandation_graphe(int id_utilisateur, int nb_recommandations, const Donnees *donnees, char *reponse);
void pagerank(float matrice[NOEUDS_MAX][NOEUDS_MAX], float scores[NOEUDS_MAX], int nb_noeuds);

#endif