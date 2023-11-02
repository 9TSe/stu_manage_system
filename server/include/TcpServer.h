#include "TcpSocket.h"

class TcpServer
{
public:
	TcpServer();
	~TcpServer();
	int Set_Listen(unsigned short port);
	TcpSocket* Accept_Connect(sockaddr_in* addr = nullptr);

private:
	int m_fd;
};