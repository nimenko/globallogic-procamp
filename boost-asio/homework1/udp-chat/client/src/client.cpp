#include <iostream>

#include "include/client.h"

Client::Client(boost::asio::io_context& io_context) :
    socket_(io_context, udp::endpoint(udp::v4(), 0)),
    resolver_(io_context),
    input_(io_context)
{
    input_.assign(STDIN_FILENO);
}

Client::~Client()
{
    close();
}

void Client::connect_to_server(const std::string& hostname, const std::string& port, const std::string& nickname)
{
    udp::resolver::results_type endpoints = resolver_.resolve(udp::v4(), hostname, port);
    nickname_ = nickname;

    if (!endpoints.empty())
    {
        server_endpoint_ = *endpoints.begin();

        // Send log-in (connection) message.
        send_connection_request();
        is_connected_ = true;

        read_input();

        receive_messages();
    }
}

void Client::disconnect_from_server()
{
    if (is_connected_)
    {
        // Send log-out (disconnection) message.
        send_disconnection_request();
        is_connected_ = false;
    }
}

void Client::send_message(const std::string& message)
{
    std::string final_message = MESSAGE_REQ + message;

    socket_.async_send_to(boost::asio::buffer(final_message.c_str(), final_message.size()), server_endpoint_,
                          [this](boost::system::error_code /*error*/, std::size_t /*bytes_sent*/){});
}

void Client::receive_messages()
{
    socket_.async_receive_from(
                boost::asio::buffer(buffer_, BUF_SIZE), server_endpoint_,
                [this](boost::system::error_code error, std::size_t bytes_received)
    {
        if (!error && bytes_received > 0)
        {
            std::string message = std::string(buffer_, bytes_received);
            std::cout << message << std::endl;
        }

        receive_messages();
    });
}

void Client::send_connection_request()
{
    std::string request = CONNECT_REQ + nickname_;
    socket_.async_send_to(boost::asio::buffer(request.c_str(), request.size()), server_endpoint_,
                          [this](boost::system::error_code /*error*/, std::size_t /*bytes_sent*/){});
}

void Client::send_disconnection_request()
{
    std::string request = DISCONNECT_REQ;
    socket_.async_send_to(boost::asio::buffer(request.c_str(), request.size()), server_endpoint_,
                          [this](boost::system::error_code /*error*/, std::size_t /*bytes_sent*/){});
}

void Client::read_input()
{
    boost::asio::async_read_until(input_, input_buffer_, '\n',
                                  [this](boost::system::error_code error, std::size_t bytes_received)
    {
        if (!error && bytes_received > 0)
        {
            boost::asio::streambuf::const_buffers_type buf = input_buffer_.data();
            std::string message(boost::asio::buffers_begin(buf),
                                boost::asio::buffers_begin(buf) + bytes_received - 1);  // Without '\n'.
            send_message(message);
            input_buffer_.consume(bytes_received);
        }

        read_input();
    });
}

void Client::close()
{
    socket_.close();
    input_.close();
}
