// Chat.h
#ifndef CHAT_H
#define CHAT_H

#include <string>

class Chat {
public:
    Chat(const std::string &userPseudo, const std::string &destPseudo, bool botMode, bool manuelMode);
    void run(); // Méthode pour exécuter le chat
    static bool verifierPseudo(const std::string &pseudo);
    static void afficherErreurEtQuitter(const std::string &message, int code);

private:
    void envoyerMessage(const std::string &message);
    void recevoirMessage();
    std::string pseudo_utilisateur;
    std::string pseudo_destinataire;
    bool option_bot;
    bool option_manuel;
};

#endif // CHAT_H
