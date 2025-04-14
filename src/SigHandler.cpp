#include "../headers/SigHandler.hpp"

volatile sig_atomic_t SigHandler::_interrupted = 0;

// Static member definition
EpollSocketServer* SigHandler::_server = NULL;  // Define the static member
Controller* SigHandler::_controller = NULL;  // Define the static member

void SigHandler::setup(EpollSocketServer *server, Controller *controller)
{
    struct sigaction sa;
    sa.sa_handler = SigHandler::handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    _server = server;
    _controller = controller;
    // Set up signal handlers for SIGINT, SIGTERM, and SIGHUP

    sigaction(SIGINT, &sa, NULL);   // Ctrl+C
    sigaction(SIGTERM, &sa, NULL);  // kill <pid>
    sigaction(SIGHUP, &sa, NULL);   // terminal closed
}

void SigHandler::handleSignal(int signum)
{
    if (signum == SIGINT || signum == SIGTERM || signum == SIGHUP)
    {
        std::cout << "\n[Signal] Received signal: " << signum << ", initiating graceful shutdown..." << std::endl;
        if (_server)
        {
            _server->stopServer();
        }
        if (_controller)
        {
            _controller->clearData();
        }
        _interrupted = 1;
    }
    exit(0);
}

bool SigHandler::wasInterrupted()
{
    return _interrupted;
}

void SigHandler::reset()
{
    _interrupted = 0;
}
