#include "MySqlConn.h"

MySqlConn::MySqlConn()
{
	m_conn = mysql_init(nullptr);
	mysql_set_character_set(m_conn, "utf8");
}

MySqlConn::~MySqlConn()
{
	if (m_conn != nullptr)
	{
		mysql_close(m_conn);
	}
	freeResult();
}

bool MySqlConn::connect(string user, string pwd, string dbName, string ip, unsigned short port)
{
	MYSQL* ptr = mysql_real_connect(m_conn, ip.c_str(), user.c_str(), pwd.c_str(), 
		dbName.c_str(), port, nullptr, 0);

	return ptr!=nullptr;
}

bool MySqlConn::update(string sql)
{
	if (mysql_query(m_conn, sql.c_str()))
	{
		return false;
	}
	return true;
}

bool MySqlConn::query(string sql)
{
	freeResult();
	if (mysql_query(m_conn, sql.c_str()))
	{
		return false;
	}
	m_result = mysql_store_result(m_conn);
	return true;
}

bool MySqlConn::next()
{
	if (m_result != nullptr)
	{
		m_row = mysql_fetch_row(m_result);
		if(m_row != nullptr)
			return true;
	}
	return false;
}

std::string MySqlConn::value(int index)
{
	int rowNum = mysql_num_fields(m_result);
	if (index >= rowNum || index < 0)
	{
		return string();
	}
	char* val = m_row[index];
	//mysql_fetch_lengths返回的当前行中每个字段的长度。
	unsigned long length = mysql_fetch_lengths(m_result)[index];
	return string(val,length);
}

bool MySqlConn::transaction()
{

	return mysql_autocommit(m_conn,false);
}

bool MySqlConn::commit()
{
	return mysql_commit(m_conn);
}

bool MySqlConn::roolback()
{
	return mysql_rollback(m_conn);
}

void MySqlConn::refreshAliveTime()
{
	m_alivetime = std::chrono::steady_clock::now();
}

long long MySqlConn::getAliveTime()
{
	std::chrono::nanoseconds res = 
		std::chrono::steady_clock::now() - m_alivetime;
	//纳秒转毫秒
	std::chrono::microseconds millsec =
		std::chrono::duration_cast<std::chrono::microseconds>(res);
	return millsec.count();
}

void MySqlConn::freeResult()
{
	if (m_result)
	{
		mysql_free_result(m_result);
		m_result = nullptr;
	}
}
