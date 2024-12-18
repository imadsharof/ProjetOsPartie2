#include "Chat.h"
#include "ClientSocket.h"
#include "SignalHandler.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <thread>
#include <sstream>

static bool pseudoValide(const std::string &p) {
    if (p.size() > 30) return false;
    if (p == "." || p == "..") return false;
    if (p.find('/')!=std::string::npos || p.find('-')!=std::string::npos ||
        p.find('[')!=std::string::npos || p.find(']')!=std::string::npos) {
        return false;
    }
    return true;
}

Chat::Chat(const std::string &userPseudo, bool botMode, bool manuelMode)
    : pseudo_utilisateur(userPseudo), option_bot(botMode), option_manuel(manuelMode), sockfd(-1), connexionEtablie(false) {
}

bool Chat::verifierPseudo(const std::string &pseudo) {
    if (!pseudoValide(pseudo)) {
        afficherErreurEtQuitter("Pseudo invalide (longueur>30, '.' '..' ou caractères /-[ ])", 3);
        return false;
    }
    return true;
}

void Chat::afficherErreurEtQuitter(const std::string &message, int code) {
    std::cerr << message << std::endl;
    std::exit(code);
}

void Chat::seConnecterAuServeur() {
    std::string ip = "127.0.0.1";
    int port = 1234;

    if(const char* env_port = std::getenv("PORT_SERVEUR")) {
        int p = std::atoi(env_port);
        if(p > 0 && p <= 65535) {
            port = p;
        }
    }
    if(const char* env_ip = std::getenv("IP_SERVEUR")) {
        // Vérif simple de l'IP
        // On suppose qu'elle est valide
        ip = env_ip;
    }

    sockfd = ClientSocket::connectToServer(ip, port);
    if (sockfd < 0) {
        afficherErreurEtQuitter("Impossible de se connecter au serveur.", 4);
    }

    connexionEtablie = true;
}

void Chat::envoyerPseudoAuServeur() {
    // Envoyer le pseudo avec un '\n'
    std::string p = pseudo_utilisateur + "\n";
    if (write(sockfd, p.c_str(), p.size()) < 0) {
        afficherErreurEtQuitter("Erreur d'envoi du pseudo au serveur.", 5);
    }
}

void Chat::configurerSignaux() {
    SignalHandler::setupSignalHandlers();
}

void Chat::run() {
    configurerSignaux();

    // Avant connexion, si SIGINT survient, on termine avec code 4
    if (SignalHandler::sigintBeforeConnect()) {
        std::exit(4);
    }

    seConnecterAuServeur();

    if (SignalHandler::sigintBeforeConnect()) {
        // Si SIGINT juste avant qu'on considère connexion établie
        close(sockfd);
        std::exit(4);
    }

    envoyerPseudoAuServeur();

    // Lancer le thread de réception
    threadReception = std::thread(&Chat::threadLireDepuisServeur, this);

    // Boucle principale : lire stdin et envoyer messages
    std::string line;
    while (true) {
        if(!std::getline(std::cin, line)) {
            // stdin fermé -> terminaison normale
            break;
        }

        // Analyser message
        std::string dest, contenu;
        if (!analyserMessageStdin(line, dest, contenu)) {
            // Format invalide, ignorer
            continue;
        }

        // Vérifier longueur contenu
        if (contenu.size() > 1024) {
            // Le serveur va déconnecter, mais on évite d'envoyer
            std::cerr << "Message trop long, ignoré." << std::endl;
            continue;
        }

        envoyerMessageAuServeur(dest, contenu);

        // Afficher le message envoyé (si pas bot)
        if(!option_bot) {
            std::lock_guard<std::mutex> lock(affichageMutex);
            std::cout << "[\x1B[4m" << pseudo_utilisateur << "\x1B[0m] " << contenu << std::endl;
        }

        // Si manuel, on affiche les messages en attente après envoi
        if (option_manuel) {
            afficherMessagesEnAttente();
        }

        // Gérer SIGINT en mode manuel
        if (SignalHandler::sigintAfterConnect() && option_manuel) {
            afficherMessagesEnAttente();
            SignalHandler::clearSigintFlag();
        }
    }

    // Fin normale (stdin fermé)
    close(sockfd);
    if (threadReception.joinable()) {
        threadReception.join();
    }
    std::exit(0);
}

void Chat::threadLireDepuisServeur() {
    char buffer[2048];
    while(true) {
        ssize_t n = read(sockfd, buffer, sizeof(buffer)-1);
        if(n <= 0) {
            // Connexion rompue
            break;
        }
        buffer[n] = '\0';
        std::string data(buffer);
        // Format: "EXPEDITEUR:message" ou "SERVEUR:Cette personne (dest) n'est pas connectée."
        size_t pos = data.find(':');
        if(pos == std::string::npos) {
            // Message invalide, ignorer
            continue;
        }
        std::string expediteur = data.substr(0,pos);
        std::string contenu = data.substr(pos+1);

        if (option_manuel) {
            // Émettre bip
            std::cout << '\a' << std::flush;
            ajouterMessageManuel(expediteur, contenu);
        } else {
            afficherMessageRecu(expediteur, contenu);
        }

        // Si tampon > 4096 en manuel, afficher
        if (option_manuel && bufferMessages.size() > 4096) {
            afficherMessagesEnAttente();
        }
    }
    // Connexion interrompue
    std::exit(0);
}

void Chat::afficherMessageRecu(const std::string &expediteur, const std::string &message) {
    std::lock_guard<std::mutex> lock(affichageMutex);
    if (option_bot) {
        std::cout << "[" << expediteur << "] " << message << std::endl;
    } else {
        std::cout << "[\x1B[4m" << expediteur << "\x1B[0m] " << message << std::endl;
    }
}

void Chat::ajouterMessageManuel(const std::string &expediteur, const std::string &message) {
    std::lock_guard<std::mutex> lock(affichageMutex);
    std::string msg;
    if (option_bot) {
        msg = "[" + expediteur + "] " + message + "\n";
    } else {
        msg = "[\x1B[4m" + expediteur + "\x1B[0m] " + message + "\n";
    }
    for (char c : msg) {
        bufferMessages.push_back(c);
    }
}

void Chat::afficherMessagesEnAttente() {
    std::lock_guard<std::mutex> lock(affichageMutex);
    for (char c : bufferMessages) {
        std::cout << c;
    }
    bufferMessages.clear();
    std::cout.flush();
}

bool Chat::analyserMessageStdin(const std::string &line, std::string &dest, std::string &contenu) {
    std::istringstream iss(line);
    if(!(iss >> dest)) {
        return false;
    }
    std::string reste;
    std::getline(iss, reste);
    if (reste.empty()) {
        return false;
    }
    // Enlever l'espace initial
    if(!reste.empty() && reste[0]==' ') reste.erase(0,1);
    contenu = reste;
    return true;
}

void Chat::envoyerMessageAuServeur(const std::string &destinataire, const std::string &message) {
    // Le serveur attend "DESTINATAIRE MESSAGE"
    std::string fullmsg = destinataire + " " + message + "\n";
    if (write(sockfd, fullmsg.c_str(), fullmsg.size()) < 0) {
        std::cerr << "Erreur d'envoi du message au serveur." << std::endl;
    }
}

void Chat::gererSigintAvantConnexion() {
    // Avant connexion, si SIGINT -> sortie code 4
    if (SignalHandler::sigintBeforeConnect()) {
        std::exit(4);
    }
}

void Chat::gererSigintApresConnexion() {
    if (option_manuel && SignalHandler::sigintAfterConnect()) {
        afficherMessagesEnAttente();
        SignalHandler::clearSigintFlag();
    }
}
