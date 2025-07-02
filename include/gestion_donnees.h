#ifndef GESTION_DONNEES_H
#define GESTION_DONNEES_H

#define UTILISATEURS_MAX 1000
#define ITEMS_MAX 1000
#define NOTATIONS_MAX 100000

typedef struct {
    int id_utilisateur;
    int id_item;
    float note;
} Notation;

typedef struct {
    Notation notations[NOTATIONS_MAX];
    int nb_notations;
    float moyennes_utilisateurs[UTILISATEURS_MAX];
    int nb_utilisateurs;
    int nb_items;
} Donnees;

// Prototypes
int charger_donnees(const char *nom_fichier, Donnees *donnees);
void calculer_moyennes(Donnees *donnees);
void afficher_statistiques(const Donnees *donnees);
void creer_donnees_exemple(Donnees *donnees);  // Ajout du prototype manquant

#endif
