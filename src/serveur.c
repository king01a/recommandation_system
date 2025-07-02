#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "algo_knn.h"
#include "algo_mf.h"
#include "algo_graphe.h"
#include "gestion_donnees.h"

#define PORT 8080
#define CLIENTS_MAX 10
#define TAILLE_TAMPON 2048

typedef struct {
    int socket;
    const Donnees *donnees;
    int numero_client;
} ClientThread;

volatile int serveur_actif = 1;

void gestionnaire_signal(int signal) {
    printf("\nSignal %d reçu. Arrêt du serveur...\n", signal);
    serveur_actif = 0;
}

void *gerer_client(void *arg) {
    ClientThread *ct = (ClientThread *)arg;
    int num_client = ct->numero_client; // Sauvegarde avant free
    char tampon_reception[TAILLE_TAMPON] = {0};
    char tampon_reponse[TAILLE_TAMPON] = {0};

    printf("[Client %d] Connexion établie\n", num_client);

    ssize_t octets_lus = recv(ct->socket, tampon_reception, TAILLE_TAMPON - 1, 0);
    if (octets_lus <= 0) {
        printf("[Client %d] Erreur lecture ou connexion fermée\n", num_client);
        close(ct->socket);
        free(ct);
        return NULL;
    }

    tampon_reception[octets_lus] = '\0';
    printf("[Client %d] Requête reçue: %s\n", num_client, tampon_reception);

    int id_utilisateur, nb_recommandations;
    char algo[20];
    int elements_lus = sscanf(tampon_reception, "%d %19s %d", &id_utilisateur, algo, &nb_recommandations);

    if (elements_lus != 3) {
        strcpy(tampon_reponse, "ERREUR: Format incorrect. Utilisez: ID_UTILISATEUR ALGO NB_RECOMMANDATIONS");
    }
    else if (nb_recommandations <= 0 || nb_recommandations > 50) {
        strcpy(tampon_reponse, "ERREUR: Nombre de recommandations doit être entre 1 et 50");
    }
    else {
        if (strcmp(algo, "KNN") == 0) {
            recommandation_knn(id_utilisateur, nb_recommandations, ct->donnees, tampon_reponse);
        }
        else if (strcmp(algo, "MF") == 0) {
            recommandation_mf(id_utilisateur, nb_recommandations, ct->donnees, tampon_reponse);
        }
        else if (strcmp(algo, "GRAPHE") == 0) {
            recommandation_graphe(id_utilisateur, nb_recommandations, ct->donnees, tampon_reponse);
        }
        else {
            sprintf(tampon_reponse, "ERREUR: Algorithme '%s' non reconnu. Utilisez: KNN, MF, ou GRAPHE", algo);
        }
    }

    ssize_t octets_envoyes = send(ct->socket, tampon_reponse, strlen(tampon_reponse), 0);
    if (octets_envoyes < 0) {
        printf("[Client %d] Erreur envoi réponse\n", num_client);
    } else {
        printf("[Client %d] Réponse envoyée (%zd octets)\n", num_client, octets_envoyes);
    }

    close(ct->socket);
    free(ct);
    printf("[Client %d] Connexion fermée\n", num_client);
    return NULL;
}

int main() {
    int socket_serveur, socket_client;
    struct sockaddr_in adresse;
    socklen_t longueur_adresse = sizeof(adresse);
    int compteur_clients = 0;

    signal(SIGINT, gestionnaire_signal);
    signal(SIGTERM, gestionnaire_signal);

    printf("=== SERVEUR DE RECOMMANDATIONS ===\n");
    printf("Chargement des données...\n");

    Donnees donnees;
    if (charger_donnees("donnees/notations.txt", &donnees) != 0) {
        fprintf(stderr, "ERREUR: Impossible de charger les données\n");
        exit(EXIT_FAILURE);
    }

    calculer_moyennes(&donnees);
    afficher_statistiques(&donnees);

    if ((socket_serveur = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("ERREUR: Échec création socket");
        exit(EXIT_FAILURE);
    }

    int option = 1;
    if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))) {
        perror("ERREUR: setsockopt");
        exit(EXIT_FAILURE);
    }

    memset(&adresse, 0, sizeof(adresse));
    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = INADDR_ANY;
    adresse.sin_port = htons(PORT);

    if (bind(socket_serveur, (struct sockaddr *)&adresse, sizeof(adresse)) < 0) {
        perror("ERREUR: Échec liaison socket");
        close(socket_serveur);
        exit(EXIT_FAILURE);
    }

    if (listen(socket_serveur, CLIENTS_MAX) < 0) {
        perror("ERREUR: Échec écoute");
        close(socket_serveur);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);
    printf("Appuyez sur Ctrl+C pour arrêter le serveur\n\n");

    while (serveur_actif) {
        fd_set set_lecture;
        struct timeval timeout;
        
        FD_ZERO(&set_lecture);
        FD_SET(socket_serveur, &set_lecture);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activite = select(socket_serveur + 1, &set_lecture, NULL, NULL, &timeout);
        
        if (activite < 0 && errno != EINTR) {
            perror("ERREUR: select");
            break;
        }
        
        if (activite > 0 && FD_ISSET(socket_serveur, &set_lecture)) {
            socket_client = accept(socket_serveur, (struct sockaddr *)&adresse, &longueur_adresse);
            
            if (socket_client < 0) {
                if (errno != EINTR) {
                    perror("ERREUR: Échec acceptation connexion");
                }
                continue;
            }
            
            pthread_t thread_id;
            ClientThread *ct = malloc(sizeof(ClientThread));
            if (ct == NULL) {
                fprintf(stderr, "ERREUR: Allocation mémoire échouée\n");
                close(socket_client);
                continue;
            }
            
            ct->socket = socket_client;
            ct->donnees = &donnees;
            ct->numero_client = ++compteur_clients;
            
            if (pthread_create(&thread_id, NULL, gerer_client, (void*)ct) < 0) {
                perror("ERREUR: Échec création thread");
                close(socket_client);
                free(ct);
                continue;
            }
            
            pthread_detach(thread_id);
        }
    }

    printf("\nFermeture du serveur...\n");
    close(socket_serveur);
    return 0;
}
