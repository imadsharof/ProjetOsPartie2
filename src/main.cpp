// main.cpp
// Ce fichier constitue le point d'entrée du programme de chat.
// Il initialise les paramètres fournis par l'utilisateur et lance la session de chat

#include "Chat.h"
#include <iostream>

int main(int argc, char *argv[]) {
    // Vérification du nombre minimal de paramètres (au moins 3 sont nécessaires : programme + deux pseudonymes).
    if (argc < 3) {
        // Affiche un message d'erreur et quitte le programme avec le code de retour 1.
        Chat::afficherErreurEtQuitter("Utilisation : start pseudo_utilisateur pseudo_destinataire [--bot] [--manuel]", 1);
    }

    // Récupération des pseudonymes de l'utilisateur et du destinataire depuis les arguments.
    std::string pseudo_utilisateur = argv[1];
    std::string pseudo_destinataire = argv[2];

    // Initialisation des options
    bool option_bot = false;    // Indique si le mode "bot" est activé
    bool option_manuel = false; // Indique si le mode "manuel" est activé

    // Parcours des arguments supplémentaires pour vérifier les options --bot et --manuel
    if (argc > 3) {
        // Si le troisième argument est --bot, active le mode bot
        if (std::string(argv[3]) == "--bot") option_bot = true;
        // Si le troisième argument est --manuel, active le mode manuel
        else if (std::string(argv[3]) == "--manuel") option_manuel = true;
    }
    if (argc > 4) {
        // Si le quatrième argument est --bot, active le mode bot
        if (std::string(argv[4]) == "--bot") option_bot = true;
        // Si le quatrième argument est --manuel, active le mode manuel
        else if (std::string(argv[4]) == "--manuel") option_manuel = true;
    }

    // Vérification de la validité des pseudonymes
    // Les pseudonymes ne doivent pas dépasser 30 caractères ni contenir de caractères interdits.
    if (!Chat::verifierPseudo(pseudo_utilisateur) || !Chat::verifierPseudo(pseudo_destinataire)) {
        return 1; // Quitte le programme avec un code d'erreur si un pseudonyme est invalide
    }

    // Création d'une instance de la classe Chat pour gérer la session de chat avec les options spécifiées
    Chat chat(pseudo_utilisateur, pseudo_destinataire, option_bot, option_manuel);

    // Lancement de la session de chat
    chat.run();  // Exécute la méthode principale de chat qui gère la boucle de discussion

    return 0; // Indique que le programme s'est terminé avec succès
}
