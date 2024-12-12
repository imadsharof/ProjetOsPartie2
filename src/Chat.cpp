// Chat.cpp
// Ce fichier contient l'implémentation de la classe Chat, qui gère les opérations principales
// pour l'envoi et la réception de messages dans un programme de chat simple.

#include "Chat.h"
#include <iostream>

// Constructeur de la classe Chat : initialise les pseudonymes de l'utilisateur et du destinataire, 
// ainsi que les options pour le mode bot et manuel.
Chat::Chat(const std::string &userPseudo, const std::string &destPseudo, bool botMode, bool manuelMode)
    : pseudo_utilisateur(userPseudo), pseudo_destinataire(destPseudo), option_bot(botMode), option_manuel(manuelMode) {}

// Méthode principale pour exécuter la session de chat.
// Elle lit les messages de l'utilisateur dans une boucle jusqu'à ce que 'exit' soit entré.
void Chat::run() {
    std::string message; // Variable pour stocker les messages envoyés
    while (true) {
        // Invite l'utilisateur à entrer un message
        std::cout << "Entrez un message (ou 'exit' pour quitter) : ";
        std::getline(std::cin, message); // Lit le message de l'entrée standard

        // Si l'utilisateur entre 'exit', on quitte la boucle et termine le chat
        if (message == "exit") break;

        // Envoie le message en l'affichant
        envoyerMessage(message);
        // Simule la réception d'un message de la part du destinataire
        recevoirMessage();
    }
}

// Méthode statique pour vérifier si un pseudonyme est valide.
// Elle vérifie la longueur maximale de 30 caractères et des caractères interdits.
bool Chat::verifierPseudo(const std::string &pseudo) {
    // Vérifie la longueur du pseudonyme
    if (pseudo.length() > 30) {
        afficherErreurEtQuitter("Erreur : Le pseudonyme doit contenir au maximum 30 caractères.", 2);
        return false;
    }
    // Vérifie les caractères interdits et les noms spéciaux
    if (pseudo == "." || pseudo == ".." || pseudo.find('/') != std::string::npos || 
        pseudo.find('-') != std::string::npos || pseudo.find('[') != std::string::npos || pseudo.find(']') != std::string::npos) {
        afficherErreurEtQuitter("Erreur : Les pseudonymes ne peuvent pas contenir les caractères / - [ ] ou être '.' ou '..'.", 3);
        return false;
    }
    return true; // Retourne vrai si le pseudonyme est valide
}

// Méthode statique pour afficher un message d'erreur et quitter le programme avec un code spécifique.
void Chat::afficherErreurEtQuitter(const std::string &message, int code) {
    std::cerr << message << std::endl; // Affiche le message d'erreur sur stderr
    exit(code); // Quitte le programme avec le code fourni
}

// Méthode pour envoyer un message, affichant le pseudonyme de l'utilisateur.
// Si le mode bot est activé, il n'y a pas de soulignement.
void Chat::envoyerMessage(const std::string &message) {
    if (!option_bot) {
        // Affiche le pseudonyme souligné pour différencier l'utilisateur
        std::cout << "[\x1B[4m" << pseudo_utilisateur << "\x1B[0m] " << message << std::endl;
    }
}

// Méthode pour simuler la réception d'un message de la part du destinataire.
// Si le mode manuel est activé, le message n'est pas affiché automatiquement.
void Chat::recevoirMessage() {
    std::string message_recu = "Réponse de " + pseudo_destinataire; // Message simulé

    // Si le mode manuel n'est pas activé, on affiche le message reçu
    if (!option_manuel) {
        if (option_bot) {
            // Mode bot : pas de soulignement du pseudonyme du destinataire
            std::cout << "[" << pseudo_destinataire << "] " << message_recu << std::endl;
        } else {
            // Pseudonyme souligné pour le destinataire
            std::cout << "[\x1B[4m" << pseudo_destinataire << "\x1B[0m] " << message_recu << std::endl;
        }
    }
}
