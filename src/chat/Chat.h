#ifndef CHAT_H
#define CHAT_H

#include <string>
#include <mutex>
#include <vector>
#include <atomic>
#include <thread>

class Chat {
public:
    Chat(const std::string &userPseudo, bool botMode, bool manuelMode);

    // Lance la boucle principale :
    // - Se connecte au serveur
    // - Envoie le pseudo
    // - Crée le thread de réception
    // - Lit sur stdin et envoie les messages
    void run();

    // Vérification du pseudo
    static bool verifierPseudo(const std::string &pseudo);

    // Affiche une erreur et quitte
    static void afficherErreurEtQuitter(const std::string &message, int code);

private:
    std::string pseudo_utilisateur;
    bool option_bot;
    bool option_manuel;

    int sockfd;

    std::thread threadReception;

    std::mutex affichageMutex; // protège l'affichage
    std::vector<char> bufferMessages; // stockage des messages en mode manuel

    std::atomic<bool> connexionEtablie; // true si la connexion est établie

    // Connexion au serveur
    void seConnecterAuServeur();
    void configurerSignaux();

    void envoyerPseudoAuServeur();

    // Envoie un message au serveur
    void envoyerMessageAuServeur(const std::string &destinataire, const std::string &message);

    // Thread secondaire lisant les messages depuis le serveur
    void threadLireDepuisServeur();

    // Affichage des messages
    void afficherMessageRecu(const std::string &expediteur, const std::string &message);
    void afficherMessageEnvoye(const std::string &destinataire, const std::string &message);

    // Mode manuel
    void ajouterMessageManuel(const std::string &expediteur, const std::string &message);
    void afficherMessagesEnAttente();

    // Gère SIGINT
    void gererSigintAvantConnexion();
    void gererSigintApresConnexion();

    // Fonction utilitaire pour vérifier le format et longueur des messages
    bool analyserMessageStdin(const std::string &line, std::string &dest, std::string &contenu);
};

#endif
