#include "Chat.h"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "chat pseudo_utilisateur [--bot] [--manuel]" << std::endl;
        return 1;
    }

    std::string pseudo_utilisateur = argv[1];
    bool option_bot = false;
    bool option_manuel = false;

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--bot") option_bot = true;
        else if (arg == "--manuel") option_manuel = true;
    }

    if (!Chat::verifierPseudo(pseudo_utilisateur)) {
        return 1;
    }

    Chat chat(pseudo_utilisateur, option_bot, option_manuel);
    chat.run();

    return 0;
}
