# Définition des répertoires et des fichiers
SRCDIR  := ./src
BINDIR  := ./
EXE     := $(BINDIR)start  # Remplace "chat" par "start"

# Ajout des fichiers du serveur
SERV_SRCDIR := $(SRCDIR)/serveur
SERV_SOURCES := $(SERV_SRCDIR)/main.cpp
SERV_OBJECTS := $(SERV_SOURCES:.cpp=.o)
SERV_EXE := $(BINDIR)serveur-chat

# Compilation avec g++
CC      := g++
CFLAGS  := -Wall -Wextra -O3 -std=c++20 -g

# Fichiers sources et objets pour le client
SOURCES := $(SRCDIR)/main.cpp $(SRCDIR)/Chat.cpp
OBJECTS := $(SOURCES:.cpp=.o)

# Cible par défaut
.PHONY: all clean

all: $(EXE) $(SERV_EXE)

$(SERV_EXE): $(SERV_OBJECTS)
	$(CC) $(CFLAGS) $(SERV_OBJECTS) -o $(SERV_EXE)

$(SERV_SRCDIR)/%.o: $(SERV_SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXE)

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJECTS) $(EXE) $(SERV_OBJECTS) $(SERV_EXE)

