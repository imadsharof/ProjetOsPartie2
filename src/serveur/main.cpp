#include <iostream>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cctype>

using namespace std;

static vector<int> client_sockets;
static vector<string> noms;
static mutex client_mutex;
static bool running = true;
static int serveur_fd;

static const int port_par_defaut = 1234;
static const int backlog = 5;

void handle_sigint(int) {
    cerr << "\nSIGINT reçu, fermeture du serveur..." << endl;
    running = false;

    {
        lock_guard<mutex> lock(client_mutex);
        for (int client_socket : client_sockets) {
            close(client_socket);
        }
        client_sockets.clear();
        noms.clear();
    }

    close(serveur_fd);
    exit(0);
}

static int trouverIndexPseudo(const string &pseudo) {
    for (size_t i = 0; i < noms.size(); i++) {
        if (noms[i] == pseudo) {
            return (int)i;
        }
    }
    return -1;
}

static void envoyerAuClient(int index, const char* buffer, int len_message) {
    if (index < 0 || (size_t)index >= client_sockets.size()) return;
    ssize_t w = write(client_sockets[index], buffer, len_message);
    (void)w; // On peut vérifier si w < 0 pour erreur
}

static void envoyerErreurAuClient(int index, const string &dest) {
    string msg = "SERVEUR:Cette personne (" + dest + ") n'est pas connectée.\n";
    envoyerAuClient(index, msg.c_str(), (int)msg.size());
}

static void traiterMessage(int emetteur_index, const char* buffer, int len_message) {
    string line(buffer, len_message);

    // Supprimer les \r et \n finaux
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
        line.pop_back();
    }

    size_t pos = line.find(' ');
    if (pos == string::npos) {
        // Pas d'espace => format invalide, on ignore
        cerr << "Message invalide reçu de " << noms[emetteur_index] << ": \"" << line << "\"" << endl;
        return; 
    }
    string dest = line.substr(0, pos);
    string contenu = line.substr(pos+1);

    if (contenu.size() > 1024) {
        cerr << "Message trop long, déconnexion du client " << noms[emetteur_index] << endl;
        close(client_sockets[emetteur_index]);
        {
            lock_guard<mutex> lock(client_mutex);
            client_sockets.erase(client_sockets.begin() + emetteur_index);
            noms.erase(noms.begin() + emetteur_index);
        }
        return;
    }

    lock_guard<mutex> lock(client_mutex);
    int dest_index = trouverIndexPseudo(dest);
    if (dest_index == -1) {
        cerr << "Destinataire " << dest << " non connecté." << endl;
        envoyerErreurAuClient(emetteur_index, dest);
    } else {
        string expediteur = noms[emetteur_index];
        string msgFinal = expediteur + ":" + contenu + "\n";
        cerr << "Envoi de \"" << msgFinal << "\" à " << dest << " depuis " << expediteur << endl;
        envoyerAuClient(dest_index, msgFinal.c_str(), (int)msgFinal.size());
    }
}

static void* client_handler(void* arg) {
    int client_socket = *((int*)arg);
    delete (int*)arg;

    int mon_index = -1;
    {
        lock_guard<mutex> lock(client_mutex);
        for (size_t i = 0; i < client_sockets.size(); i++) {
            if (client_sockets[i] == client_socket) {
                mon_index = (int)i;
                break;
            }
        }
    }

    if (mon_index == -1) {
        cerr << "Erreur: client introuvable après connexion." << endl;
        close(client_socket);
        pthread_exit(NULL);
    }

    cerr << "Le client " << noms[mon_index] << " est prêt à envoyer des messages." << endl;

    char buffer[4096];
    while (running) {
        ssize_t len_message_recu = read(client_socket, buffer, sizeof(buffer));
        if (len_message_recu <= 0) {
            cerr << "Client " << noms[mon_index] << " déconnecté." << endl;
            close(client_socket);
            {
                lock_guard<mutex> lock(client_mutex);
                client_sockets.erase(client_sockets.begin() + mon_index);
                noms.erase(noms.begin() + mon_index);
            }
            break;
        }
        traiterMessage(mon_index, buffer, (int)len_message_recu);
    }

    pthread_exit(NULL);
}

int main() {
    int port = port_par_defaut;
    if (const char* env_port = getenv("PORT_SERVEUR")) {
        int p = atoi(env_port);
        if (p > 0 && p <= 65535) {
            port = p;
        }
    }

    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, SIG_IGN);

    serveur_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serveur_fd < 0) {
        perror("Problème avec la création du socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(serveur_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(serveur_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in adresse;
    memset(&adresse, 0, sizeof(adresse));
    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = INADDR_ANY;
    adresse.sin_port = htons(port);
    socklen_t adresse_len = sizeof(adresse);

    if (bind(serveur_fd, (struct sockaddr*)&adresse, sizeof(adresse)) == -1) {
        perror("Erreur lors de la liaison du socket");
        close(serveur_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(serveur_fd, backlog) == -1) {
        perror("Erreur lors de l'écoute");
        close(serveur_fd);
        exit(EXIT_FAILURE);
    }

    cerr << "Serveur en écoute sur le port " << port << endl;

    while (running) {
        int client_socket = accept(serveur_fd, (struct sockaddr*)&adresse, &adresse_len);
        if (client_socket == -1) {
            if (!running) break; 
            continue;
        }

        char nom[31]; 
        memset(nom, 0, sizeof(nom));
        ssize_t r = read(client_socket, nom, 30);
        if (r <= 0) {
            cerr << "Aucun pseudo reçu, fermeture du client." << endl;
            close(client_socket);
            continue;
        }

        // Enlever \n, \r
        {
            char* nl = strchr(nom, '\n');
            if (nl) *nl = '\0';
            char* cr = strchr(nom, '\r');
            if (cr) *cr = '\0';
        }

        string pseudo = nom;
        if (pseudo.empty()) {
            cerr << "Pseudo vide, fermeture du client." << endl;
            close(client_socket);
            continue;
        }

        {
            lock_guard<mutex> lock(client_mutex);
            client_sockets.push_back(client_socket);
            noms.push_back(pseudo);
        }

        cerr << "Client " << pseudo << " connecté." << endl;

        int* sock_ptr = new int(client_socket);
        pthread_t client_thread;
        if (pthread_create(&client_thread, nullptr, client_handler, sock_ptr) != 0) {
            perror("Erreur lors de la création du thread client");
            close(client_socket);
            {
                lock_guard<mutex> lock(client_mutex);
                auto it_s = find(client_sockets.begin(), client_sockets.end(), client_socket);
                if (it_s != client_sockets.end()) {
                    size_t idx = it_s - client_sockets.begin();
                    client_sockets.erase(it_s);
                    noms.erase(noms.begin() + idx);
                }
            }
            delete sock_ptr;
        } else {
            pthread_detach(client_thread);
        }
    }

    close(serveur_fd);
    return 0;
}
