#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <string>

class ClientSocket {
public:
    static int connectToServer(const std::string &ip, int port);
};

#endif

