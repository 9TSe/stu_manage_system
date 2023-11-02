#include <string>
#include <mysql.h>
#include <iostream>
#include <iomanip> // 添加头文件以使用 setw
#include <sstream> // 添加头文件以使用 std::ostringstream

MYSQL* Connect_MySQL()
{
    MYSQL* mysql = mysql_init(nullptr); //启动MySQL服务,初始化
    if (mysql == nullptr)
    {
        perror("mysql_init");
        return nullptr;
    }

    //连接MySQL
    mysql = mysql_real_connect(mysql, "192.168.200.131", "root", "9tse",
        "9tse", 0, NULL, 0);
    if (mysql == nullptr)
    {
        perror("mysql connect");
        return nullptr;
    }

    mysql_set_character_set(mysql, "utf8"); //设置为utf8编码

    std::cout << "Mysql connect successfully" << std::endl;
    return mysql;
}


void Act_Sql(MYSQL* mysql, const std::string sql, std::ostringstream& result)
{
    const char* char_sql = sql.c_str();
    int ret = mysql_query(mysql, char_sql); // 执行SQL
    if (ret != 0) {
        perror("action mysql");
        return;
    }

    MYSQL_RES* res = mysql_store_result(mysql); // 取出结果集
    if (res == nullptr) {
        perror("mysql_store_result");
        return;
    }

    int num = mysql_num_fields(res); // 结果集列数
    MYSQL_FIELD* fields = mysql_fetch_fields(res); // 所有列名字

    // 打印列名并设置字段宽度和左对齐
    result << std::left;
    for (int i = 0; i < num; ++i) {
        result << std::setw(12) << fields[i].name << "\t";
    }
    result << "\n";

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL) // 遍历行
    {
        // 打印每一列并设置字段宽度和左对齐
        for (int i = 0; i < num; ++i) {
            result << std::setw(12) << row[i] << "\t";
        }
        result << "\n";
    }

    mysql_free_result(res); // 释放资源 - 结果集
}