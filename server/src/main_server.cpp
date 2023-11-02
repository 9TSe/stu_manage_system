#include "my_sql.h"
#include "TcpServer.h"
#include "thread_pool.h"

struct SocketPag {
	TcpSocket* msg_tcp;
	TcpServer* listen_tcp;
	sockaddr_in addr;
};

void Working(void* args, MYSQL* mysql) //�̳߳�������
{
	SocketPag* pkg = static_cast<SocketPag*>(args);
	//��ӡ�ͻ��˻�����Ϣ
	char ip[32];
	std::cout << "client ip: " << inet_ntop(AF_INET, &pkg->addr.sin_addr.s_addr, ip, sizeof(ip)) << std::endl;
	std::cout << "client port: " << ntohs(pkg->addr.sin_port) << std::endl;

	//��ȡת��������Sqlָ��
	while (1)
	{
		std::ostringstream result;
		std::string sql = pkg->msg_tcp->Recv_Msg(); //��ȷ���ǲ���һ��һ����,����
		if (!sql.empty())
		{
			Act_Sql(mysql, sql, result); //ִ��sql���
			pkg->msg_tcp->Send_Msg(result.str()); //��sqlִ�н�����͸�client
		}
		else
		{
			break;
		}
		result.clear();
	}

	delete pkg->listen_tcp;
	delete pkg;
	return;
}

int main()
{
	MYSQL* mysql = Connect_MySQL(); //����MySQL

	TcpServer server;
	server.Set_Listen(10000); //�󶨲������ü���

	std::threadpool threadpool(32); //ʵ�����̳߳�

	while (true)
	{
		SocketPag* pag = new SocketPag();
		//���������Ӻ����Ϣ���ݸ�pag->addr,���Դ˲�������Tcpsocket
		TcpSocket* Msgret = server.Accept_Connect(&pag->addr);
		if (Msgret == nullptr) //���Ϊ�����������
		{
			perror("main accept");
			continue;
		}
		pag->listen_tcp = &server; //�������������̵߳����������һ��
		pag->msg_tcp = Msgret; //���ͻ��˴�������Ϣ����Msgret��

		std::future<void> work = threadpool.commit(Working, pag, mysql);
	}
}
