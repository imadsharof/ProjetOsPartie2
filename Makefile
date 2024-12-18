# Définition des répertoires et des fichiers
SRCDIR  := ./src
BINDIR  := ./
CHATDIR := $(SRCDIR)/chat
SERVDIR := $(SRCDIR)/serveur

EXE     := $(BINDIR)start  # le client
SERV_EXE := $(BINDIR)serveur-chat

# Compilation avec g++
CC      := g++
CFLAGS  := -Wall -Wextra -O3 -std=c++20 -g

# Sources pour le client
CHAT_SOURCES := $(CHATDIR)/main.cpp $(CHATDIR)/Chat.cpp $(CHATDIR)/ClientSocket.cpp $(CHATDIR)/SignalHandler.cpp
CHAT_OBJECTS := $(CHAT_SOURCES:.cpp=.o)

# Sources pour le serveur
SERV_SOURCES := $(SERVDIR)/main.cpp
SERV_OBJECTS := $(SERV_SOURCES:.cpp=.o)

.PHONY: all clean

all: $(EXE) $(SERV_EXE)

$(EXE): $(CHAT_OBJECTS)
	$(CC) $(CFLAGS) $(CHAT_OBJECTS) -o $(EXE)

$(SERV_EXE): $(SERV_OBJECTS)
	$(CC) $(CFLAGS) $(SERV_OBJECTS) -o $(SERV_EXE)

$(CHATDIR)/%.o: $(CHATDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVDIR)/%.o: $(SERVDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CHAT_OBJECTS) $(SERV_OBJECTS) $(EXE) $(SERV_EXE)

