#include <string>
#include <mysql.h>
#include <iostream>
#include <iomanip> // ���ͷ�ļ���ʹ�� setw
#include <sstream> // ���ͷ�ļ���ʹ�� std::ostringstream

MYSQL* Connect_MySQL()
{
    MYSQL* mysql = mysql_init(nullptr); //����MySQL����,��ʼ��
    if (mysql == nullptr)
    {
        perror("mysql_init");
        return nullptr;
    }

    //����MySQL
    mysql = mysql_real_connect(mysql, "192.168.200.131", "root", "9tse",
        "9tse", 0, NULL, 0);
    if (mysql == nullptr)
    {
        perror("mysql connect");
        return nullptr;
    }

    mysql_set_character_set(mysql, "utf8"); //����Ϊutf8����

    std::cout << "Mysql connect successfully" << std::endl;
    return mysql;
}


void Act_Sql(MYSQL* mysql, const std::string sql, std::ostringstream& result)
{
    const char* char_sql = sql.c_str();
    int ret = mysql_query(mysql, char_sql); // ִ��SQL
    if (ret != 0) {
        perror("action mysql");
        return;
    }

    MYSQL_RES* res = mysql_store_result(mysql); // ȡ�������
    if (res == nullptr) {
        perror("mysql_store_result");
        return;
    }

    int num = mysql_num_fields(res); // ���������
    MYSQL_FIELD* fields = mysql_fetch_fields(res); // ����������

    // ��ӡ�����������ֶο�Ⱥ������
    result << std::left;
    for (int i = 0; i < num; ++i) {
        result << std::setw(12) << fields[i].name << "\t";
    }
    result << "\n";

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL) // ������
    {
        // ��ӡÿһ�в������ֶο�Ⱥ������
        for (int i = 0; i < num; ++i) {
            result << std::setw(12) << row[i] << "\t";
        }
        result << "\n";
    }

    mysql_free_result(res); // �ͷ���Դ - �����
}