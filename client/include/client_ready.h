#include"TcpSocket.h"
#include <fstream>
#include <unordered_map>
#include <vector>

void Connect(TcpSocket& client) //���ӷ�����
{
    int ret = client.Connect_Host("127.0.0.1", 10000);
    if (ret == -1)
        perror("client connect to host");
    return;
}


class Login {
public:
    std::string m_name;
    Login(const std::string& filename)
        : filename_(filename)
    {
        LoadDataFromFile();
    }

    bool Authenticate()
    {
        std::string username;
        std::cout << "please input your name: ";
        std::cin >> username;
        m_name = username;
        std::string password;
        std::cout << "please input your password: ";
        std::cin >> password;

        if (accounts_.find(username) != accounts_.end() && accounts_[username] == password)
        {
            std::cout << "Authentication successful for " << username << std::endl;
            return true;
        }
        else
        {
            std::cout << "Authentication failed for " << username << std::endl;
            return false;
        }
    }

    void AddAccount(const std::string& username, const std::string& password)
    {
        accounts_[username] = password;
        SaveDataToFile();
        std::cout << "Account added: " << username << std::endl;
    }

    bool DeleteAccount(const std::string& username)
    {
        auto it = accounts_.find(username);
        if (it != accounts_.end())
        {
            accounts_.erase(it);
            SaveDataToFile();
            std::cout << "Account deleted: " << username << std::endl;
            return true;
        }
        else
        {
            std::cout << "Account not found: " << username << std::endl;
            return false;
        }
    }

    bool ModifyPassword(const std::string& username, const std::string& newPassword)
    {
        auto it = accounts_.find(username);
        if (it != accounts_.end())
        {
            it->second = newPassword;
            SaveDataToFile();
            std::cout << "Password for account " << username << " modified." << std::endl;
            return true;
        }
        else
        {
            std::cout << "Account not found: " << username << std::endl;
            return false;
        }
    }

private:
    std::string filename_;
    std::unordered_map<std::string, std::string> accounts_;

    void LoadDataFromFile()  //��ȡ�ļ�����
    {
        std::ifstream file(filename_, std::ios::binary); //�����ƶ�ȡ
        if (file.is_open())
        {
            while (true)
            {
                std::string username, password;
                if (ReadStringFromBinaryFile(file, username) && ReadStringFromBinaryFile(file, password))
                {
                    accounts_[username] = password;
                }
                else
                {
                    break;
                }
            }
            file.close();
        }
    }

    void SaveDataToFile() {
        std::ofstream file(filename_, std::ios::binary);
        if (file.is_open())
        {
            for (const auto& entry : accounts_)
            {
                WriteStringToBinaryFile(file, entry.first);
                WriteStringToBinaryFile(file, entry.second);
            }
            file.close();
        }
    }

    bool ReadStringFromBinaryFile(std::ifstream& file, std::string& str)
    {
        uint32_t strLength = 0;
        if (file.read(reinterpret_cast<char*>(&strLength), sizeof(uint32_t)))
        {
            str.resize(strLength);
            if (file.read(&str[0], strLength))
            {
                return true;
            }
        }
        return false;
    }

    void WriteStringToBinaryFile(std::ofstream& file, const std::string& str)
    {
        uint32_t strLength = static_cast<uint32_t>(str.size());
        file.write(reinterpret_cast<const char*>(&strLength), sizeof(uint32_t));
        file.write(str.c_str(), strLength);
    }
};



enum class chose
{
    add,
    del,
    select,
    modify
};
int Chose_Function() //ѡ����ɾ���
{
    std::cout << "******************************************************" << std::endl;
    std::cout << "**************    input num to use    ****************" << std::endl;
    std::cout << "*****    0.add    ************    1.delete    ********" << std::endl;
    std::cout << "*****    2.select ************    3.modify    ********" << std::endl;
    std::cout << "******************************************************" << std::endl;
    int input;
    std::cin >> input;
    return input;
}

void Add_Action(TcpSocket& client, Login& user) //�����û�,�����Զ������˺�,ֻ��1����Ȩ��
{
    if (user.m_name != "542213430101")
    {
        std::cout << "Apologize, you do not have the authority" << std::endl;
        return;
    }
    std::string sql = "INSERT INTO stu_msg (num, name, gender, profession, class, score) VALUES (";
    std::string input;
    std::string num;
    std::vector<std::string> inputValues;
    std::cout << "Enter the following information in order (num, name, gender, profession, class, score):" << std::endl;

    for (int i = 0; i < 6; i++)
    {
        std::cin >> input;
        inputValues.push_back(input);
        if (i == 0)
            num = input;
    }
    for (const std::string& value : inputValues)
        sql += "'" + value + "',";

    // Remove the trailing ", " 
    sql = sql.substr(0, sql.length() - 1);
    sql += ")";
    //����
    client.Send_Msg(sql);

    //�����û�(Ĭ���û�����������ѧ��)
    user.AddAccount(num, num);

}


void Del_Action(TcpSocket& client, Login& user) //ɾ���˺�(login������չ)
{
    if (user.m_name != "542213430101")
    {
        std::cout << "Apologize, you do not have the authority" << std::endl;
        return;
    }

    std::cout << "input the student number to delete: ";
    std::string num;
falg:
    std::cin >> num;
    if (num == "542213430101")
    {
        std::cout << "you can't delete yourself, try again" << std::endl;
        goto falg;
    }
    std::string sql("delete from stu_msg where num = ");
    client.Send_Msg(sql + num);

    user.DeleteAccount(num);
}

void Sel_Action(TcpSocket& client, Login& user)
{
    std::cout << "select your find way" << std::endl
        << "1. select all" << std::endl
        << "2. select by more ways" << std::endl;
    int input;
    std::cin >> input;

    std::string sql("select * from stu_msg ");
    if (input == 1)
    {
        client.Send_Msg(sql);
    }
    else
    {
        std::cout<<"chose your select way by ** write ** word"<<std::endl;
        std::string selectway;
        std::cout<<"num   name  gender  profession  class  score"<<std::endl;
        std::cin >> selectway;

        std::string message;
        std::cout << "input select message"<<std::endl;
        std::cin >> message;

        std::string sqlsend = sql + "where " + selectway + "= '" + message + "'";
        std::cout << sqlsend<<std::endl;
        client.Send_Msg(sqlsend);
    }
    //sleep(1);
    std::cout << client.Recv_Msg() << std::endl;
    return;
}


void Mod_Action(TcpSocket& client, Login& user) //�޸��˺���Ϣ���Լ�����Ϣ
{
    std::cout << "which you want to modify :" << std::endl;
    std::cout << "account/table msg (1/0) :" << std::endl;
    int chose;
    std::cin >> chose;
    if (chose) //�޸��˺���Ϣ
    {
        std::string newpassword;
        std::cout << "input your new password" << std::endl;
        std::cin >> newpassword;
        user.ModifyPassword(user.m_name, newpassword);
    }
    else //�޸ı����е���Ϣ
    {
        std::string rootchosenum;
        if (user.m_name == "542213430101")
        {
            std::cout << "chose num to change" << std::endl;
            std::cin >> rootchosenum;
        }
        std::cout << "input word to change message" << std::endl;
        std::cout << "num  name  gender  profession  class  score" << std::endl;
        std::string input;
        std::cin >> input;
        std::string sql("update stu_msg set ");
        input += "='";
        sql += input;
        std::cout << "input message to change" << std::endl;
        std::cin >> input;
        input += "' where num = ";
        if (user.m_name == "542213430101")
            input += rootchosenum;
        else
            input += user.m_name;
        sql += input;
        client.Send_Msg(sql);
    }
}
