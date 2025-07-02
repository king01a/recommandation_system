# Système de Recommandation Avancé  
Un système client-serveur implémentant KNN, Factorisation Matricielle et PageRank  

---

## Table des matières  
1. [Objectif du Projet]  
2. [Architecture du Projet]  
3. [Prérequis & Installation]  
4. [Utilisation]  
5. [Structure des Fichiers]  
6. [Format des Données]  
7. [Algorithmes Implémentés] 
8. [Compilation avec Make]  
9. [Exécution] 
10. [FAQ & Dépannage]  
11. [Licence] 

---

## Objectif du Projet  
Ce projet est un système de recommandation intelligent qui permet de suggérer des items (films, livres, produits...) à des utilisateurs en utilisant trois algorithmes différents :  

- **K-Nearest Neighbors (KNN)** : Basé sur la similarité entre utilisateurs  
- **Factorisation Matricielle (MF)** : Décomposition matricielle pour prédire les notes  
- **PageRank sur Graphe** : Algorithme de classement inspiré de Google  

L'architecture client-serveur permet une interaction flexible et scalable.  

---

## Architecture du Projet  

```
recommandation_system/
├── src/                    # Code source
│   ├── client.c           # Programme client
│   ├── serveur.c          # Programme serveur
│   ├── algo_knn.c         # Algorithme KNN
│   ├── algo_mf.c          # Algorithme MF
│   ├── algo_graphe.c      # Algorithme PageRank
│   └── gestion_donnees.c  # Gestion des données
├── include/               # Fichiers d'en-tête
│   ├── algo_knn.h
│   ├── algo_mf.h
│   ├── algo_graphe.h
│   └── gestion_donnees.h
├── donnees/               # Fichiers de données
│   └── notations.txt      # Fichier d'entrée
├── build/                 # Fichiers objets (générés)
├── bin/                   # Exécutables (générés)
│   ├── client            # Exécutable client
│   └── serveur           # Exécutable serveur
└── Makefile              # Automatisation de la compilation
```

---

## Prérequis & Installation  
### Prérequis  
- **Système d'exploitation** : Linux (testé sur Ubuntu 20.04+)  
- **Compilateur** : GCC (version 9.4.0 ou supérieure recommandée)  
- **Outils** : Make  
- **Bibliothèques** : math.h, pthread (incluses dans glibc)  

### Installation  
1. Clonez le dépôt :  
   
   git clone https://github.com/king01a/recommandation_system.git  
   cd recommandation_system  
   

2. Compilez le projet :
   
   make all  # Compile tous les composants
   
   
   Cette commande :
   - Crée automatiquement les répertoires `build/` et `bin/`
   - Génère les fichiers objets dans `build/`
   - Produit les exécutables dans `bin/`

---

## Utilisation

### 1. Lancer le serveur

./bin/serveur


**Fonctionnement :**
- Charge automatiquement les données depuis `donnees/notations.txt`
- Crée des données d'exemple si le fichier est absent
- Écoute sur le port 8080 (modifiable dans `serveur.c`)

### 2. Lancer le client

./bin/client


**Commandes disponibles :**

> 1 KNN 5       # 5 recommandations pour l'utilisateur 1 via KNN  
> 2 MF 3        # 3 recommandations via Factorisation Matricielle  
> 3 GRAPHE 10   # 10 recommandations via PageRank  
> help          # Affiche l'aide  
> quit          # Quitte le programme  


---

## Structure des Fichiers

Le projet utilise une organisation claire des fichiers :

- **src/** : Contient tous les fichiers sources (.c)
- **include/** : Contient tous les fichiers d'en-tête (.h)
- **build/** : Généré automatiquement, contient les fichiers objets
- **bin/** : Généré automatiquement, contient les exécutables
- **donnees/** : Contient les fichiers de données d'entrée

---

## Format des Données

Le système accepte deux formats de fichier d'entrée (`donnees/notations.txt`) :

**Format CSV (recommandé) :**

utilisateur_id,item_id,note  
1,101,5  
1,102,3  
2,101,4  

**Format Espace/Tabulation :**

1 101 5  
1 102 3  
2 101 4  


**Contraintes :**
- Les IDs doivent être des entiers positifs
- Les notes doivent être entre 0.0 et 5.0

---

## Algorithmes Implémentés

### 1. K-Nearest Neighbors (KNN)
**Paramètres configurables :**
- `K_VOISINS` : Nombre de voisins (défaut: 5)
- `SEUIL_SIMILARITE` : Similarité minimale (défaut: 0.0)

**Fichiers concernés :** `src/algo_knn.c`, `include/algo_knn.h`

### 2. Factorisation Matricielle (MF)
**Paramètres configurables :**
- `NB_FACTEURS` : Dimensions latentes (défaut: 10)
- `TAUX_APPRENTISSAGE` : Vitesse d'apprentissage (défaut: 0.01)

**Fichiers concernés :** `src/algo_mf.c`, `include/algo_mf.h`

### 3. PageRank sur Graphe
**Paramètres configurables :**
- `FACTEUR_AMORTISSEMENT` : Probabilité de suivi (défaut: 0.85)
- `ITERATIONS_MAX` : Itérations maximum (défaut: 100)

**Fichiers concernés :** `src/algo_graphe.c`, `include/algo_graphe.h`

---

## Compilation avec Make

### Commandes disponibles

| Commande          |		 Action 		 |
|---------------------------|------------------------------------------------|
| `make` ou `make all` | Compile tout le projet (serveur + client) |
| `make clean` 	     | Supprime tous les fichiers générés (build/ et bin/) |

### Détails de compilation

Le Makefile utilise les options suivantes :
- **Compilateur** : GCC
- **Flags** : `-Wall -Wextra -I./include` (warnings + inclusion des headers)
- **Liens** : `-lm -lpthread` (bibliothèques math et pthread)

**Dépendances de compilation :**
- Le serveur dépend de : `serveur.o`, `algo_knn.o`, `algo_mf.o`, `algo_graphe.o`, `gestion_donnees.o`
- Le client dépend uniquement de : `client.o`

---

## Exécution

### Démarrage complet
1. **Terminal 1** (Serveur) :

   make all
   ./bin/serveur

2. **Terminal 2** (Client) :
   
   ./bin/client
   

### Exemple d'utilisation

# Compilation
make all

# Terminal 1
./bin/serveur
[INFO] Serveur démarré sur le port 8080...

# Terminal 2  
./bin/client
> 1 KNN 5
[Recommandations KNN pour utilisateur 1...]
> quit


---

## FAQ & Dépannage

### Problèmes fréquents

**1. Erreur de port :**

bind: Address already in use

**Solution :**

sudo lsof -i :8080  # Trouvez le processus utilisant le port
kill -9 <PID>       # Terminez le processus


**2. Fichier de données introuvable :**
Le serveur génère automatiquement des données d'exemple si `donnees/notations.txt` n'existe pas.

**3. Erreurs de compilation :**
Vérifiez que vous avez :
- GCC installé (`gcc --version`)
- Les droits d'écriture dans le répertoire
- Make installé (`make --version`)

### Personnalisation

**Pour modifier :**
- **Le port** : Éditez `PORT` dans `serveur.c`
- **Les algorithmes** : Modifiez les paramètres dans les fichiers `.h` correspondants
- **Les flags de compilation** : Modifiez `CFLAGS` dans le Makefile

### Nettoyage

make clean  # Supprime build/ et bin/


---

## Licence

Ce projet est distribué sous licence libre.
