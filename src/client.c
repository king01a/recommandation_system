#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define TAILLE_TAMPON 2048

void afficher_aide() {
    printf("\n=== CLIENT DE RECOMMANDATIONS ===\n");
    printf("Format de requête: ID_UTILISATEUR ALGORITHME NB_RECOMMANDATIONS\n");
    printf("Algorithmes disponibles:\n");
    printf("  - KNN: K-Nearest Neighbors (basé sur similarité utilisateurs)\n");
    printf("  - MF: Factorisation Matricielle (prédiction de notes)\n");
    printf("  - GRAPHE: PageRank sur graphe utilisateurs-items\n");
    printf("Exemples:\n");
    printf("  1 KNN 5        -> 5 recommandations KNN pour utilisateur 1\n");
    printf("  2 MF 3         -> 3 recommandations MF pour utilisateur 2\n");
    printf("  3 GRAPHE 10    -> 10 recommandations PageRank pour utilisateur 3\n");
    printf("Tapez 'quit' pour quitter\n");
    printf("================================\n\n");
}

int main() {
int socket_client;
struct sockaddr_in adresse_serveur;
char tampon[TAILLE_TAMPON];

printf("=== CLIENT SYSTÈME DE RECOMMANDATIONS ===\n");

while (1) {
    // Création du socket client
    if ((socket_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("ERREUR: Échec création socket client\n");
        continue;
    }
    
    // Configuration de l'adresse du serveur
    memset(&adresse_serveur, 0, sizeof(adresse_serveur));
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(PORT);
    
    // Convertir adresse IP
    if (inet_pton(AF_INET, "127.0.0.1", &adresse_serveur.sin_addr) <= 0) {
        printf("ERREUR: Adresse IP invalide\n");
        close(socket_client);
        continue;
    }
    
    // Connexion au serveur
    if (connect(socket_client, (struct sockaddr *)&adresse_serveur, 
               sizeof(adresse_serveur)) < 0) {
        printf("ERREUR: Connexion au serveur échouée. Le serveur est-il démarré?\n");
        close(socket_client);
        
        printf("Voulez-vous réessayer? (o/n): ");
        fgets(tampon, sizeof(tampon), stdin);
        if (tampon[0] == 'n' || tampon[0] == 'N') {
            break;
        }
        continue;
    }
    
    printf("Connecté au serveur!\n");
    afficher_aide();
    
    // Boucle de communication
    while (1) {
        printf("Entrez votre requête: ");
        if (fgets(tampon, TAILLE_TAMPON, stdin) == NULL) {
            break;
        }
        
        // Retirer le saut de ligne
        tampon[strcspn(tampon, "\n")] = 0;
        
        // Commandes spéciales
        if (strcmp(tampon, "quit") == 0 || strcmp(tampon, "exit") == 0) {
            printf("Au revoir!\n");
            close(socket_client);
            exit(0);
        }
        
        if (strcmp(tampon, "help") == 0 || strcmp(tampon, "aide") == 0) {
            afficher_aide();
            continue;
        }
        
        if (strlen(tampon) == 0) {
            continue;
        }
        
        // Validation basique de la requête
        int id_user, nb_reco;
        char algo[20];
        if (sscanf(tampon, "%d %19s %d", &id_user, algo, &nb_reco) != 3) {
            printf("Format incorrect. Utilisez: ID_UTILISATEUR ALGO NB_RECOMMANDATIONS\n");
            continue;
        }
        
        // Envoyer requête
        ssize_t octets_envoyes = send(socket_client, tampon, strlen(tampon), 0);
        if (octets_envoyes < 0) {
            printf("ERREUR: Échec envoi requête\n");
            break;
        }
        
        printf("Requête envoyée: %s\n", tampon);
        printf("Traitement en cours...\n");
        
        // Recevoir réponse
        memset(tampon, 0, TAILLE_TAMPON);
        ssize_t octets_recus = recv(socket_client, tampon, TAILLE_TAMPON - 1, 0);
        if (octets_recus <= 0) {
            printf("ERREUR: Échec réception réponse\n");
            break;
        }
        
        tampon[octets_recus] = '\0';
        printf("\n=== RÉPONSE DU SERVEUR ===\n");
        printf("%s\n", tampon);
        printf("========================\n\n");
    }
    
    close(socket_client);
    break;
}

return 0;

}
