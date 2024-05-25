// ConnPoolConsoleApplication.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include"MySqlConn.h"
#include"ConnectionPool.h"
void op1(int begin, int end)
{
	for (int i = begin; i < end; i++)
	{
        MySqlConn conn;
        conn.connect("root", "123456", "test1", "localhost");
        char sql[1024] = { 0 };
        sprintf(sql, "insert into person values(%d,23,'123','321')", i);
        //std::string sql = "insert into person values(100,23,'123','321')";
        conn.update(sql);
	}
}
void op2(ConnectionPool* pool,int begin, int end)
{
    for (int i = begin; i < end; ++i)
    {
        std::shared_ptr<MySqlConn> conn = pool->getSqlConn();
        char sql[1024] = { 0 };
        sprintf(sql, "insert into person values(%d,23,'123','321')", i);
        conn->update(sql);
    }
}
void test1()
{
#if 0
    //single thread without pool 15754ms
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    op1(0, 5000);
    auto end = std::chrono::steady_clock::now();
    auto length = end - begin;
    std::cout << "single thread without pool time: " << length.count() << " ns,"
        << length.count() / 1000000 << " ms." << std::endl;
#else
    //single thread with pool 3175ms
    ConnectionPool* pool = ConnectionPool::getConnectPool();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    op2(pool,0, 5000);
    auto end = std::chrono::steady_clock::now();
    auto length = end - begin;
    std::cout << "single thread with pool time: " << length.count() << " ns,"
        << length.count() / 1000000 << " ms." << std::endl;
#endif
}
void test2()
{
#if 0
    //3309ms
    MySqlConn conn;
    conn.connect("root", "123456", "test1", "localhost");
    auto begin = std::chrono::steady_clock::now();
    std::thread t1(op1, 0, 1000);
    std::thread t2(op1, 1000, 2000);
    std::thread t3(op1, 2000, 3000);
    std::thread t4(op1, 3000, 4000);
    std::thread t5(op1, 4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    auto end = std::chrono::steady_clock::now();
    auto length = end - begin;
    std::cout << "multi thread without pool time: " << length.count() << " ns,"
        << length.count() / 1000000 << " ms." << std::endl;
#else
    //1050ms
    ConnectionPool* pool = ConnectionPool::getConnectPool();
    auto begin = std::chrono::steady_clock::now();
    std::thread t1(op2,pool,0, 1000);
    std::thread t2(op2,pool,1000, 2000);
    std::thread t3(op2,pool,2000, 3000);
    std::thread t4(op2,pool,3000, 4000);
    std::thread t5(op2,pool,4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    auto end = std::chrono::steady_clock::now();
    auto length = end - begin;
    std::cout << "multi thread with pool time: " << length.count() << " ns,"
        << length.count() / 1000000 << " ms." << std::endl;
#endif
}
int query()
{
    MySqlConn conn;
    conn.connect("root", "123456", "test1", "localhost");
    std::string sql = "insert into person values(100,23,'123','321')";
    /*bool flag = conn.update(sql);
    std::cout << "flag value: " << flag << std::endl;*/
    sql = "select * from person";
    conn.query(sql);
    while (conn.next())
    {
        std::cout << conn.value(0) << ","
            << conn.value(1) << ","
            << conn.value(2) << ","
            << conn.value(3) << std::endl;
    }
    return 0;
}
int main()
{
    test2();
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
