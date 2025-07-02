#ifndef ALGO_KNN_H
#define ALGO_KNN_H
#include "gestion_donnees.h"

void recommandation_knn(int id_utilisateur, int nb_recommandations, const Donnees *donnees, char *reponse);
float calculer_similarite_pearson(int utilisateur1, int utilisateur2, const Donnees *donnees);

#endif