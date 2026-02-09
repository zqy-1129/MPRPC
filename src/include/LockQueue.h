#pragma once

#include <queue>
#include <mutex>
#include <condition_variable> 

/**
 *  模板本身不会被编译成机器码（因为T是未知类型）；
 *  只有当你使用add<int>、add<double>时，编译器才会根据模板定义，生成对应类型的add函数代码；
 *  如果模板定义在.cpp中，调用模板的main.cpp编译时，编译器看不到模板定义 → 无法生成实例化代码；
 *  而模板所在的.cpp因为没有看到实例化请求 → 也不会生成代码 → 最终链接失败。
 */
// 异步日志队列
template<typename T>
class LockQueue
{
public:
    // 多个worker线程都会进行日志写
    void Push(const T &data)
    {   
        // 根据其生命周期自动解锁
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        // 唤醒对应的一个线程
        m_condvariable.notify_one();
    }

    T Pop()
    {   
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // 日志为空，线程进入wait状态
            m_condvariable.wait(lock);
        }
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
};