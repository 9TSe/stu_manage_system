#include "client_ready.h"

int main()
{

	Login user("account.txt");
	//user.AddAccount("542213430101", "9tse");

	while (!user.Authenticate());


	TcpSocket client;
	Connect(client);	//���ӷ�����

	while (1)
	{
		switch (Chose_Function())
		{
		case (int)chose::add:
			Add_Action(client, user);
			break;
		case (int)chose::del:
			Del_Action(client, user);
			break;
		case (int)chose::select:
			Sel_Action(client, user);
			break;
		case (int)chose::modify:
			Mod_Action(client, user);
			break;
		default:
			std::cout<<"chose error"<<std::endl;
		}
	}


	return 0;
}
