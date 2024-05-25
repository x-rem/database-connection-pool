#include "ConnectionPool.h"

ConnectionPool* ConnectionPool::getConnectPool()
{
    static ConnectionPool pool;
    return &pool;
}

bool ConnectionPool::parseJsonFile()
{

    std::ifstream ifs("dbconf.json");
    if (!ifs.is_open()) {
        std::cerr << "Failed to open JSON file." << std::endl;
        return false;
    }
    try {
        json j;
        ifs >> j;
        m_ip = j["ip"].get<std::string>();
        m_user = j["username"].get<std::string>();
        m_pwd = j["password"].get<std::string>();
        m_dbName = j["dbName"].get<std::string>();
        m_port = j["port"].get<unsigned short>();
        m_minSize = j["minSize"].get<int>();
        m_maxSize = j["maxSize"].get<int>();
        m_timeout = j["timeout"].get<int>();
        m_maxIdleTime = j["maxIdleTime"].get<int>();
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }
}

void ConnectionPool::addConn()
{
    MySqlConn* conn = new MySqlConn;
    conn->connect(m_user, m_pwd, m_dbName, m_ip, m_port);
    conn->refreshAliveTime();
    m_connectionQ.push(conn);
}

std::shared_ptr<MySqlConn> ConnectionPool::getSqlConn()
{
    std::unique_lock<std::mutex> locker(m_mutexQ);
    // 当连接池为空时
    //阻塞线程，等待新的可用连接
    while (m_connectionQ.empty())
    {
        if (std::cv_status::timeout ==
            m_cond.wait_for(locker, std::chrono::milliseconds(m_timeout)))
        {
            if (m_connectionQ.empty())
            {
                continue;
            }
        }
    }
    //当使用完该连接后，会调用删除器，执行lambda表达式内的代码
    //此时会将连接归还
    std::shared_ptr<MySqlConn> 
        connptr(m_connectionQ.front(), 
            [this](MySqlConn* conn) {
                m_mutexQ.lock();
                conn->refreshAliveTime();
                m_connectionQ.push(conn);
                m_mutexQ.unlock();
             });
    m_connectionQ.pop();
    m_cond.notify_all();//唤醒使用了同一变量的线程
    return connptr;
}

ConnectionPool::~ConnectionPool()
{
    while (!m_connectionQ.empty())
    {
        MySqlConn* conn = m_connectionQ.front();
        m_connectionQ.pop();
        delete conn;
    }
}

ConnectionPool::ConnectionPool()
{
    if (!parseJsonFile())
        return;
    for (int i = 0; i < m_minSize; i++)
        addConn();
    std::thread producer(&ConnectionPool::produceConn,this);
    std::thread recycler(&ConnectionPool::recycleConn, this);
    producer.detach();
    recycler.detach();
}


void ConnectionPool::produceConn()
{
    while (true)
    {
        //unique_lock构造时锁定互斥锁
        //当条件变量满足时，wait() 方法会重新锁定互斥锁
        //当队列可用数量满足可用最小连接后，不再创建连接，此时阻塞
        std::unique_lock<std::mutex> locker(m_mutexQ);
        while (m_connectionQ.size() >= m_minSize && m_connectionQ.size() < m_maxSize)
        {
            //让线程释放互斥锁并等待条件变量 m_cond 的通知
            m_cond.wait(locker);
        }
        addConn();
        m_cond.notify_all();//唤醒使用了同一变量的线程
    }
}

void ConnectionPool::recycleConn()
{
    //如果当前连接池中的连接数量超过了最小容量
    //判断最初的连接是否空闲，若是空闲则回收资源
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::lock_guard<std::mutex> locker(m_mutexQ);
        while (m_connectionQ.size() > m_minSize)
        {
            MySqlConn* conn = m_connectionQ.front();
            if (conn->getAliveTime() >= m_maxIdleTime)
            {
                m_connectionQ.pop();
                delete conn;
            }
            else
            {
                break;
            }
        }
    }
}
