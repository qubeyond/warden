#include "services/cli/cli_service.hpp"
#include <csignal>
#include <atomic>

void signal_handler(int) {
    exit(0);
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    warden::services::CliService cli;
    return cli.run(argc, argv);
}