#include "TcpSocket.h"

TcpSocket::TcpSocket() //一般用于客户端,并通过这个文件描述符进行和服务器的连接
{
	m_fd = socket(AF_INET,SOCK_STREAM,0);
}

TcpSocket::TcpSocket(int socket) //一般用于服务器,直接用这个套接字通信
{
	m_fd = socket;
}

TcpSocket::~TcpSocket()
{
	if(m_fd > 0)
		close(m_fd);
}

int TcpSocket::Connect_Host(std::string ip, unsigned short port)
{
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port); //端口转换为网络字节序
	inet_pton(AF_INET, ip.data(), &saddr.sin_addr.s_addr); //ip地址转换为大端(网络字节序)
	int ret = connect(m_fd, (sockaddr*)&saddr, sizeof(saddr)); //连接,m_fd,ip port存放在saddr

	if (ret == -1)
	{
		perror("connect");
		return -1;
	}
	std::cout << "connect with server sucessfully" << std::endl;
	return ret; //返回文件描述符
}

int TcpSocket::written(const char* msg, int size) //作为发射端,前四个字节初始化后就不用管了
{
	int readplace = 0; //开始读取的位置
	int remain = size; //剩余读取的大小
	const char* buf = msg; //临时存放流,以便输出
	while(remain > 0) //只有还有剩余的大小就继续读
	{
		if((readplace = write(m_fd, buf, remain)) > 0) //将buf中内容输入m_fd,write返回值为写入的字节
		{
			remain -= readplace;
			buf += readplace;
		}
		else if(readplace == -1) //如果write写入错误就会返回-1
		{
			return -1;
		}
	}
	return 0;
}
int TcpSocket::Send_Msg(std::string msg)
{
	char* buf = new char[msg.size() + 4];
	int flagsize = htonl(msg.size());
	memcpy(buf,&flagsize,4); //前四位设置字符串大小
	memcpy(buf+4,msg.data(), msg.size()); //后面照常装字符串 erro没有发完

	int ret = written(buf, msg.size() + 4); //ret返回值为一个字符串的大小
	delete[] buf;
	return ret;
}



std::string TcpSocket::Recv_Msg()
{
	int len = 0;
	readn((char*)&len, 4); //读取传来的前四个字节数据,得到数据内容大小
	len = ntohl(len);
	std::cout << "thesize of recive msg is: " << len << std::endl;

	char* buf = new char[len + 1]; //+1存放换行
	int ret = readn(buf, len); //读取数据,放置buf内
	buf[len] = '\0';//手动添加结束符
	std::string returnstr(buf); //返回值
	delete[] buf;
	return returnstr;
}

int TcpSocket::readn(char* msg, int size)
{
	int readplace = 0;
	int remain = size; 
	char* buf = msg;
	while (remain > 0)
	{
		if ((readplace = read(m_fd, buf, size)) > 0)
		{
			remain -= readplace;
			buf += readplace;
		}
		else if (readplace == -1)
		{
			return -1;
		}
	}
	return 0;
}




