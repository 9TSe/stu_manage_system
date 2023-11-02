#include<string>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<iostream>
#include<stdio.h>
#include<string.h>

class TcpSocket
{
public:
	TcpSocket();
	TcpSocket(int socket);
	~TcpSocket();
	int Connect_Host(std::string ip, unsigned short port);
	int Send_Msg(std::string msg);
	std::string Recv_Msg();

private:
	int readn(char* msg, int size);
	int written(const char* msg, int size);

private:
	int m_fd; //通信的套接字
};