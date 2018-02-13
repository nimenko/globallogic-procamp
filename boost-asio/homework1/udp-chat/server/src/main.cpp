#include <iostream>

#include <include/server.h>

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: server <port>" << std::endl;
            return 1;
        }

        boost::asio::io_context io_context;

        Server server(io_context, std::atoi(argv[1]));
        server.start_server();

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
