#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <string>

#include <boost/asio.hpp>

using boost::asio::ip::udp;

class Server
{
public:
    Server(boost::asio::io_context& io_context, short port);
    ~Server();

    void start_server();
    void stop_server();

private:
    void receive_messages();

    void handle_connection(const udp::endpoint& sender_endpoint, const std::string& nickname);
    void handle_disconnection(const udp::endpoint& sender_endpoint);
    void handle_message(const udp::endpoint& sender_endpoint, std::size_t length);

    void broadcast_connection(const std::string& nickname);
    void broadcast_disconnection(const std::string& nickname);
    void broadcast_message(const std::string& message);

    void close();

    udp::socket socket_;
    udp::endpoint sender_endpoint_;

    std::map<udp::endpoint, std::string> users_;

    enum { BUF_SIZE = 1024 };
    char buffer_[BUF_SIZE];

    const std::string CONNECT_REQ = "#connect#";
    const std::string DISCONNECT_REQ = "#disconnect#";
    const std::string MESSAGE_REQ = "#msg#";
};

#endif // SERVER_H
