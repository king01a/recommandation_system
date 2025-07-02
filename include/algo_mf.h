#ifndef ALGO_MF_H
#define ALGO_MF_H
#include "gestion_donnees.h"

#define NB_FACTEURS 10  // Définition ajoutée ici

void recommandation_mf(int id_utilisateur, int nb_recommandations, const Donnees *donnees, char *reponse);
void factorisation_matricielle(const Donnees *donnees, float matrice_utilisateurs[][NB_FACTEURS], float matrice_items[][NB_FACTEURS]);

#endif
