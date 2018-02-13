#include <iostream>

#include "include/server.h"

Server::Server(boost::asio::io_context& io_context, short port) :
    socket_(io_context, udp::endpoint(udp::v4(), port))
{
}

Server::~Server()
{
    close();
}

void Server::start_server()
{
    receive_messages();
}

void Server::stop_server()
{
    close();
}

void Server::receive_messages()
{
    socket_.async_receive_from(
                boost::asio::buffer(buffer_, BUF_SIZE), sender_endpoint_,
                [this](boost::system::error_code error, std::size_t bytes_received)
    {
        if (!error && bytes_received > 0)
        {
            std::string buffer = buffer_;

            // Connection request.
            {
                std::string::size_type found = buffer.find(CONNECT_REQ);
                if (found != std::string::npos)
                {
                    // Get nickname from connection request.
                    std::string nickname = buffer.substr(found + CONNECT_REQ.size(),
                                                         bytes_received - CONNECT_REQ.size());

                    std::cout << "Connection from " << sender_endpoint_ << std::endl;
                    handle_connection(sender_endpoint_, nickname);
                }
            }

            // Disconnection request.
            {
                std::string::size_type found = buffer.find(DISCONNECT_REQ);
                if (found != std::string::npos)
                {
                    std::cout << "Disconnection from " << sender_endpoint_ << std::endl;
                    handle_disconnection(sender_endpoint_);
                }
            }

            // Message request.
            {
                std::string::size_type found = buffer.find(MESSAGE_REQ);
                if (found != std::string::npos)
                {
                    std::cout << "Message from " << sender_endpoint_ << std::endl;
                    handle_message(sender_endpoint_, bytes_received);
                }
            }

            std::cout << "'" << std::string(buffer_, bytes_received) << "'" << std::endl;
        }

        receive_messages();
    });
}

void Server::handle_connection(const udp::endpoint& sender_endpoint, const std::string& nickname)
{
    if (users_.find(sender_endpoint) == users_.end())
    {
        users_[sender_endpoint] = nickname;
        broadcast_connection(nickname);
    }
}

void Server::handle_disconnection(const udp::endpoint& sender_endpoint)
{
    if (users_.find(sender_endpoint) != users_.end())
    {
        broadcast_disconnection(users_[sender_endpoint]);
        users_.erase(sender_endpoint);
    }
}

void Server::handle_message(const udp::endpoint& sender_endpoint, std::size_t length)
{
    auto it = users_.find(sender_endpoint);
    if (it != users_.end())
    {
        std::string nickname = it->second;

        // Prepare message in format: <nickname> : <message>.
        std::string buffer = std::string(buffer_ + MESSAGE_REQ.size(), length - MESSAGE_REQ.size());
        std::string message = nickname + " : " + buffer;

        broadcast_message(message);
    }
}

void Server::broadcast_connection(const std::string& nickname)
{
    std::string message = "Server: " + nickname + " has joined.";

    for (auto& user : users_)
    {
        socket_.async_send_to(
                    boost::asio::buffer(message.c_str(), message.size()), user.first,
                    [this](boost::system::error_code /*error*/, std::size_t /*bytes_sent*/){});
    }
}

void Server::broadcast_disconnection(const std::string& nickname)
{
    std::string message = "Server: " + nickname + " has left.";

    for (auto& user : users_)
    {
        socket_.async_send_to(
                    boost::asio::buffer(message.c_str(), message.size()), user.first,
                    [this](boost::system::error_code /*error*/, std::size_t /*bytes_sent*/){});
    }
}

void Server::broadcast_message(const std::string& message)
{
    for (auto& user : users_)
    {
        socket_.async_send_to(
                    boost::asio::buffer(message.c_str(), message.size()), user.first,
                    [this, message, user](boost::system::error_code /*error*/, std::size_t /*bytes_sent*/)
        {
            std::cout << "Message: '" << message << "' broadcasted to: " << user.first << std::endl;
        });
    }
}

void Server::close()
{
    socket_.close();
}
