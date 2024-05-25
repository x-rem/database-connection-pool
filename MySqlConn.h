#pragma once
#include<string>
#include<iostream>
#include<mysql.h>
#include<chrono>
class MySqlConn
{
	using string = std::string;
public:
	MySqlConn();
	//初始化数据库连接
	//释放数据库连接
	~MySqlConn();
	//连接数据库
	bool connect(string user, string pwd, string dbName, 
		string ip, unsigned short port = 3306);
	//更新数据库
	bool update(string sql);
	//查询数据库
	bool query(string sql);
	//遍历查询得到的结果集
	bool next();
	//得到结果集中的字段值
	string value(int index);
	//事务操作
	bool transaction();
	//提交事务
	bool commit();
	//事务回滚
	bool roolback();
	//刷新起始空闲时间点
	void refreshAliveTime();
	//计算连接存活总时长
	long long getAliveTime();
private:
	void freeResult();//回收结果集的内存
	MYSQL* m_conn = nullptr;
	MYSQL_RES* m_result = nullptr;
	MYSQL_ROW m_row= nullptr;
	std::chrono::steady_clock::time_point m_alivetime;
};