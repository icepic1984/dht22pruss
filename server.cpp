#include "server.hpp"
#include "bbbdht22.hpp"

Connection::ConnectionPtr_t Connection::create(boost::asio::io_service& io_service)
{
	return ConnectionPtr_t(new Connection(io_service));
}

tcp::socket& Connection::socket()
{
	return socket_;
}

void Connection::start(const std::string& message)
{
	boost::asio::async_write(socket_, 
							 boost::asio::buffer(message),
							 boost::bind(&Connection::handle_write,
										 shared_from_this(),
										 boost::asio::placeholders::error,
										 boost::asio::placeholders::bytes_transferred));
}

Connection::Connection(boost::asio::io_service& io_service)
	: socket_(io_service)
{
}

void Connection::handle_write(const boost::system::error_code& /*error*/,
							  size_t /*bytes_transferred*/)
{
}

Server::Server(const DHT22Ptr_t& model,boost::asio::io_service& io_service)
	: acceptor_(io_service, tcp::endpoint(tcp::v4(), 666)),
	  model_(model)
{
	start_accept();
}

void Server::start_accept()
{
	Connection::ConnectionPtr_t new_connection =
	   Connection::create(acceptor_.get_io_service());

	acceptor_.async_accept(new_connection->socket(),
						   boost::bind(&Server::handle_accept, this, new_connection,
											 boost::asio::placeholders::error));
}

void Server::handle_accept(Connection::ConnectionPtr_t new_connection,
						   const boost::system::error_code& error) 
{
	if (!error)
	{
		new_connection->start(model_->message());
	}
	
	start_accept();
}
