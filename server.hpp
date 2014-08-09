#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <memory>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class DHT22;
typedef std::shared_ptr<DHT22> DHT22Ptr_t;

class Connection : public std::enable_shared_from_this<Connection> {
public:
   typedef std::shared_ptr<Connection> ConnectionPtr_t;

   static ConnectionPtr_t create(boost::asio::io_service& io_service);
   tcp::socket& socket();
   void start(const std::string& message);
   
private:
   Connection(boost::asio::io_service& io_service);

   void handle_write(const boost::system::error_code& /*error*/,
					 size_t /*bytes_transferred*/);

   tcp::socket socket_;
};

class Server
{
public:
   Server(const DHT22Ptr_t& model, boost::asio::io_service& io_service);
   
private:
   void start_accept();

   void handle_accept(Connection::ConnectionPtr_t new_connection,
					  const boost::system::error_code& error);
   tcp::acceptor acceptor_;
   DHT22Ptr_t model_;
   
};

#endif 
