#ifndef CLIENT_H
#define CLIENT_H

#include <string>

#include <boost/asio.hpp>

using boost::asio::ip::udp;

class Client
{
public:
    Client(boost::asio::io_context& io_context);
    ~Client();

    void connect_to_server(const std::string& hostname, const std::string& port, const std::string& nickname);
    void disconnect_from_server();

    void send_message(const std::string& message);

    bool is_connected() { return is_connected_; }

private:
    void send_connection_request();
    void send_disconnection_request();

    void receive_messages();

    void read_input();

    void close();

    udp::socket socket_;
    udp::endpoint server_endpoint_;
    udp::resolver resolver_;

    std::string nickname_;

    enum { BUF_SIZE = 1024 };
    char buffer_[BUF_SIZE];

    boost::asio::posix::stream_descriptor input_;
    boost::asio::streambuf input_buffer_;

    bool is_connected_;

    const std::string CONNECT_REQ = "#connect#";
    const std::string DISCONNECT_REQ = "#disconnect#";
    const std::string MESSAGE_REQ = "#msg#";
};

#endif // CLIENT_H
