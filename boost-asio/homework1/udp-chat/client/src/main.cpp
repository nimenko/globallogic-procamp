#include <iostream>
#include <thread>

#include <include/client.h>

int main(int argc, char** argv)
{
    try
    {
        if (argc != 4)
        {
            std::cerr << "Usage: client <hostname> <port> <nickname>" << std::endl;
            return 1;
        }

        boost::asio::io_context io_context;

        std::string hostname = std::string(argv[1]);
        std::string port = std::string(argv[2]);
        std::string nickname = std::string(argv[3]);

        Client client(io_context);
        client.connect_to_server(hostname, port, nickname);

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
