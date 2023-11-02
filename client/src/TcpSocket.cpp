#include "TcpSocket.h"

TcpSocket::TcpSocket() //һ�����ڿͻ���,��ͨ������ļ����������кͷ�����������
{
	m_fd = socket(AF_INET,SOCK_STREAM,0);
}

TcpSocket::TcpSocket(int socket) //һ�����ڷ�����,ֱ��������׽���ͨ��
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
	saddr.sin_port = htons(port); //�˿�ת��Ϊ�����ֽ���
	inet_pton(AF_INET, ip.data(), &saddr.sin_addr.s_addr); //ip��ַת��Ϊ���(�����ֽ���)
	int ret = connect(m_fd, (sockaddr*)&saddr, sizeof(saddr)); //����,m_fd,ip port�����saddr

	if (ret == -1)
	{
		perror("connect");
		return -1;
	}
	std::cout << "connect with server sucessfully" << std::endl;
	return ret; //�����ļ�������
}

int TcpSocket::written(const char* msg, int size) //��Ϊ�����,ǰ�ĸ��ֽڳ�ʼ����Ͳ��ù���
{
	int readplace = 0; //��ʼ��ȡ��λ��
	int remain = size; //ʣ���ȡ�Ĵ�С
	const char* buf = msg; //��ʱ�����,�Ա����
	while(remain > 0) //ֻ�л���ʣ��Ĵ�С�ͼ�����
	{
		if((readplace = write(m_fd, buf, remain)) > 0) //��buf����������m_fd,write����ֵΪд����ֽ�
		{
			remain -= readplace;
			buf += readplace;
		}
		else if(readplace == -1) //���writeд�����ͻ᷵��-1
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
	memcpy(buf,&flagsize,4); //ǰ��λ�����ַ�����С
	memcpy(buf+4,msg.data(), msg.size()); //�����ճ�װ�ַ��� erroû�з���

	int ret = written(buf, msg.size() + 4); //ret����ֵΪһ���ַ����Ĵ�С
	delete[] buf;
	return ret;
}



std::string TcpSocket::Recv_Msg()
{
	int len = 0;
	readn((char*)&len, 4); //��ȡ������ǰ�ĸ��ֽ�����,�õ��������ݴ�С
	len = ntohl(len);
	std::cout << "thesize of recive msg is: " << len << std::endl;

	char* buf = new char[len + 1]; //+1��Ż���
	int ret = readn(buf, len); //��ȡ����,����buf��
	buf[len] = '\0';//�ֶ���ӽ�����
	std::string returnstr(buf); //����ֵ
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




