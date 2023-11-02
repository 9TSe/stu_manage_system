#include "my_sql.h"
#include "TcpServer.h"
#include "thread_pool.h"

struct SocketPag {
	TcpSocket* msg_tcp;
	TcpServer* listen_tcp;
	sockaddr_in addr;
};

void Working(void* args, MYSQL* mysql) //线程池任务函数
{
	SocketPag* pkg = static_cast<SocketPag*>(args);
	//打印客户端基本信息
	char ip[32];
	std::cout << "client ip: " << inet_ntop(AF_INET, &pkg->addr.sin_addr.s_addr, ip, sizeof(ip)) << std::endl;
	std::cout << "client port: " << ntohs(pkg->addr.sin_port) << std::endl;

	//获取转换过来的Sql指令
	while (1)
	{
		std::ostringstream result;
		std::string sql = pkg->msg_tcp->Recv_Msg(); //先确定是不是一波一波的,不是
		if (!sql.empty())
		{
			Act_Sql(mysql, sql, result); //执行sql语句
			pkg->msg_tcp->Send_Msg(result.str()); //将sql执行结果发送给client
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
	MYSQL* mysql = Connect_MySQL(); //连接MySQL

	TcpServer server;
	server.Set_Listen(10000); //绑定并且设置监听

	std::threadpool threadpool(32); //实例化线程池

	while (true)
	{
		SocketPag* pag = new SocketPag();
		//将建立连接后的信息传递给pag->addr,在以此参数构造Tcpsocket
		TcpSocket* Msgret = server.Accept_Connect(&pag->addr);
		if (Msgret == nullptr) //如果为空有两种情况
		{
			perror("main accept");
			continue;
		}
		pag->listen_tcp = &server; //将即将进入子线程的任务包完整一下
		pag->msg_tcp = Msgret; //而客户端传来的信息就在Msgret中

		std::future<void> work = threadpool.commit(Working, pag, mysql);
	}
}
