#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

class SignalHandler {
public:
    static void setupSignalHandlers();
    static bool sigintBeforeConnect();
    static bool sigintAfterConnect();
    static void clearSigintFlag();
};

#endif

