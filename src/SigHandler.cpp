#include "../headers/SigHandler.hpp"

volatile sig_atomic_t SigHandler::_interrupted = 0;

// Static member definition
EpollSocketServer* SigHandler::_server = NULL;  // Define the static member

void SigHandler::setup(EpollSocketServer *server) {
    struct sigaction sa;
    sa.sa_handler = SigHandler::handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    _server = server;
    // Set up signal handlers for SIGINT, SIGTERM, and SIGHUP

    sigaction(SIGINT, &sa, NULL);   // Ctrl+C
    sigaction(SIGTERM, &sa, NULL);  // kill <pid>
    sigaction(SIGHUP, &sa, NULL);   // terminal closed
}

void SigHandler::handleSignal(int signum) {
    if (signum == SIGINT || signum == SIGTERM || signum == SIGHUP) {
        std::cout << "\n[Signal] Received signal: " << signum << ", initiating graceful shutdown..." << _server << std::endl;
        if (_server) {
            _server->stopServer();
        }
        _interrupted = 1;
    }
}

bool SigHandler::wasInterrupted() {
    return _interrupted;
}

void SigHandler::reset() {
    _interrupted = 0;
}
