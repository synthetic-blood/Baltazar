#include<iostream>
#include<asio/io_context.hpp>
#include<asio/ip/tcp.hpp>
#include<asio/read.hpp>
#include<asio/write.hpp>
#include<asio/dispatch.hpp>
#include<asio/thread_pool.hpp>
#include<fstream>
#include<filesystem>
#include<string>
#include<array>
#include<thread>
#include<memory>
#include"Baltazar.hpp"
using asio::ip::tcp;
asio::io_context ioctx;
asio::thread_pool tpctx(4);
asio::ip::tcp::acceptor Acceptor(ioctx, tcp::endpoint(tcp::v4(), 80));
void SendResponse(const asio::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<tcp::socket> Socket,std::shared_ptr<std::vector<char>> ResponseBuffer) {
	if (bytes_transferred == 0) {
		Socket->close();
	}
}
void ReadRequest(const asio::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<tcp::socket> Socket, std::shared_ptr<std::array<char, 1024>> RequestBuffer) {
	if (!ec) {
		Baltazar::RequestMap RequestValues = Baltazar::GetRequestProperties(std::string(RequestBuffer->data(), bytes_transferred));
		//std::cout.write(RequestBuffer->data(), bytes_transferred);
		std::shared_ptr<std::vector<char>>  Response(new std::vector<char>(Baltazar::ComposeResponse(RequestValues)));
		//Send
		asio::async_write(*Socket, asio::buffer(*Response, Response->size()), std::bind(SendResponse, std::placeholders::_1, std::placeholders::_2, Socket, Response));
		Socket->async_read_some(asio::buffer(*RequestBuffer), std::bind(ReadRequest, std::placeholders::_1, std::placeholders::_2, Socket, RequestBuffer));
	}
}
void AcceptConnection(const asio::error_code& ec, std::shared_ptr<tcp::socket> Socket) {
	//Accept Connection
	std::shared_ptr<tcp::socket> NewSocket(new tcp::socket(ioctx));
	Acceptor.async_accept(*NewSocket, std::bind(AcceptConnection, std::placeholders::_1, NewSocket));
	//Accept New
	//Async start sub processes
	std::shared_ptr<std::array<char, 1024>> RequestBuffer(new std::array<char, 1024>);
	Socket->async_read_some(asio::buffer(*RequestBuffer), std::bind(ReadRequest, std::placeholders::_1, std::placeholders::_2, Socket, RequestBuffer));
}

int main() {
	Baltazar::LoadMimeTypes();
	std::vector<std::thread> threads;
	std::shared_ptr<tcp::socket> Socket(new tcp::socket(ioctx));
	Acceptor.async_accept(*Socket, std::bind(AcceptConnection, std::placeholders::_1, Socket));
	unsigned int max = std::thread::hardware_concurrency() * 2;
	for (int i = 0; i < max; i++) {
		threads.emplace_back([i]() {
			while (true) {
				std::cout << i << std::endl;
				ioctx.run_one();
			}
		});
	}
	for (std::thread& th : threads) {
		if (th.joinable()) {
			th.join();
		}
	}
	while(!ioctx.stopped()){}
	return 0;
}
