#pragma once
#include<queue>
#include"MySqlConn.h"
#include<string>
#include<mutex>
#include<condition_variable>
#include<nlohmann/json.hpp>
#include<fstream>
#include<thread>
class ConnectionPool
{
	using string = std::string;
	using json = nlohmann::json;
public:
	static ConnectionPool* getConnectPool();
	ConnectionPool(const ConnectionPool& obj) = delete;
	ConnectionPool& operator=(const ConnectionPool& obj) = delete;
	//取出可用连接
	std::shared_ptr<MySqlConn> getSqlConn();
	~ConnectionPool();
private:
	ConnectionPool();
	void produceConn();
	void recycleConn();
	bool parseJsonFile();
	void addConn();
	string m_ip;
	string m_user;
	string m_pwd;
	string m_dbName;
	unsigned short m_port;
	int m_minSize;
	int m_maxSize;
	int m_timeout;
	int m_maxIdleTime;//最大空闲时长
	std::queue<MySqlConn*> m_connectionQ;
	std::mutex m_mutexQ;
	std::condition_variable m_cond;
};

