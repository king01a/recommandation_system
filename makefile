# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -I./include
LDFLAGS = -lm -lpthread

# Dossiers
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Fichiers sources
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
EXEC_SERVEUR = $(BIN_DIR)/serveur
EXEC_CLIENT = $(BIN_DIR)/client

# Cible principale
all: prepare $(EXEC_SERVEUR) $(EXEC_CLIENT)

prepare:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Règle pour le serveur
$(EXEC_SERVEUR): $(BUILD_DIR)/serveur.o $(BUILD_DIR)/algo_knn.o $(BUILD_DIR)/algo_mf.o $(BUILD_DIR)/algo_graphe.o $(BUILD_DIR)/gestion_donnees.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Règle pour le client
$(EXEC_CLIENT): $(BUILD_DIR)/client.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Règle générique pour les fichiers objets
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all prepare clean
