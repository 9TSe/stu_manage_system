#include "TcpServer.h"

TcpServer::TcpServer()
{
	m_fd = socket(AF_INET, SOCK_STREAM, 0);
}

TcpServer::~TcpServer()
{
	close(m_fd);
}

int TcpServer::Set_Listen(unsigned short port)
{
	//��ip �� port
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port); //���
	saddr.sin_addr.s_addr = INADDR_ANY; //���
	int ret = bind(m_fd, (sockaddr*)&saddr, sizeof(saddr));

	if (ret == -1)
	{
		perror("bind");
		return -1;
	}
	std::cout << "sucessfully bind to ip: " << inet_ntoa(saddr.sin_addr) << std::endl
		<< "port: " << port << std::endl;

	//���ü���
	ret = listen(m_fd, 128);
	{
		perror("listen");
		return -1;
	}
	std::cout << "set listen sucessfully" << std::endl;
	return 0;
}

TcpSocket* TcpServer::Accept_Connect(sockaddr_in* addr)
{
	if (addr == nullptr)
	{
		perror("accept(addr) is nullptr");
		return nullptr;
	}

	socklen_t addrlen = sizeof(sockaddr_in);
	int cfd = accept(m_fd, (sockaddr*)addr, &addrlen);
	if (cfd == -1)
	{
		perror("accept");
		return nullptr;
	}
	std::cout << "connect with client sucessfully" << std::endl;
	return new TcpSocket(cfd);
}
