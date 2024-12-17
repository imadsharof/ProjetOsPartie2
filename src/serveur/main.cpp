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

using namespace std;

vector<int> client_sockets;
vector<string> noms;
mutex client_mutex;
bool running = true;
int serveur_fd;

const int port_par_defaut = 1234;
const int clients_max = 1000;
const int backlog = 5;

void handle_signal(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nSIGINT reçu, fermeture du serveur..." << std::endl;
        running = false;

        // Fermer toutes les connexions clients
        std::lock_guard<std::mutex> lock(client_mutex);
        for (int client_socket : client_sockets) {
            close(client_socket);
        }
        client_sockets.clear();

        // Fermer le socket du serveur
        close(serveur_fd);
        exit(0);
    }
}

void envoyer_message(char buffer[4096], int len_message){
    string nom;
    for(int i = 0; i < len_message; i++){
        if(buffer[i] == ' '){
            break;
        }
        else{
            nom += buffer[i];
        }
    }
    for(size_t i; i < size(client_sockets); i++){
        if(nom == noms[i]){
            write(client_sockets[i], buffer, len_message);
        }
    }
}

void* read_socket(void* args){
    int* client_socket = (int*) args;
    int len_message_recu;
    char buffer[4096];
    while(true){
        while ((len_message_recu = read(*client_socket, buffer, 4096)) > 0){
            envoyer_message(buffer, len_message_recu);
        };
    }
}

int main() {
    signal(SIGINT, handle_signal);
    serveur_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(serveur_fd < 0){
        printf("Problème avec la création du socket");
        exit(0);
    }

    int opt = 1;
    setsockopt(serveur_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in adresse;
    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = INADDR_ANY;
    adresse.sin_port = htons(port_par_defaut);
    socklen_t adresse_len = sizeof(adresse);

    // Liaison du socket à l'adresse et au port
    if (bind(serveur_fd, (struct sockaddr*)&adresse, sizeof(adresse)) == -1) {
        perror("Erreur lors de la liaison du socket");
        close(serveur_fd);
        exit(EXIT_FAILURE);
    }

    // Écoute des connexions entrantes
    if (listen(serveur_fd, backlog) == -1) {
        perror("Erreur lors de l'écoute");
        close(serveur_fd);
        exit(EXIT_FAILURE);
    }
    while (running) {
        int client_socket = accept(serveur_fd, (struct sockaddr*)&adresse, &adresse_len);
        if (client_socket == -1) {
            continue;
        }

        client_sockets.push_back(client_socket);

        char nom[30];
        read(client_socket, nom, sizeof(nom) - 1);
        noms.push_back(string(nom));

        // Créer un thread pour gérer le client
        pthread_t client_thread;
        if (pthread_create(&client_thread, nullptr, read_socket, &client_socket) != 0) {
            perror("Erreur lors de la création du thread client");
            close(client_socket);
        } else {
            pthread_detach(client_thread);
        }
    }

    return 0;
}
