#ifndef SIGHANDLER_HPP
#define SIGHANDLER_HPP

#include <csignal>
#include <iostream>
#include "EpollSocketServer.hpp"
#include "Controller.hpp"

class SigHandler {
public:
    static void setup(EpollSocketServer *server, Controller *controller);

    static bool wasInterrupted();
    static void reset();
    static EpollSocketServer* _server;
    static Controller* _controller;


private:
    static void handleSignal(int signum);
    // sig_atomic_t / special int type defined to be safe to access inside signal handlers. 
    // Guarantees that reads and writes to it are atomic
    static volatile sig_atomic_t _interrupted;
};

#endif // SIGHANDLER_HPP
