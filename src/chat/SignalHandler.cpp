#include "SignalHandler.h"
#include <signal.h>
#include <atomic>
#include <iostream>

static std::atomic<bool> g_sigintReceived(false);
static std::atomic<bool> g_connectionEstablished(false);

static void sigintHandler(int) {
    g_sigintReceived = true;
}

void SignalHandler::setupSignalHandlers() {
    struct sigaction sa;
    sa.sa_handler = sigintHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    signal(SIGPIPE, SIG_IGN);
}

bool SignalHandler::sigintBeforeConnect() {
    return g_sigintReceived && !g_connectionEstablished;
}

bool SignalHandler::sigintAfterConnect() {
    return g_sigintReceived && g_connectionEstablished;
}

void SignalHandler::clearSigintFlag() {
    g_sigintReceived = false;
}
