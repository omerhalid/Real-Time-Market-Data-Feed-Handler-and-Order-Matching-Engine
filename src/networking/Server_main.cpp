#include "Server.cpp"
#include <iostream>
#include <boost/asio.hpp>

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }

        // Set up the IO context and start the server
        boost::asio::io_context io_context;
        short port = std::atoi(argv[1]);
        Server server(io_context, port);
        
        // Run the server
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
