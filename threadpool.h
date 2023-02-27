#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <queue>
#include <list>
#include "locker.h"
#include <exception>
#include <cstdio>

// 线程池类 模版类
template< typename T>
class threadpool{

public:
    threadpool(int thread_number = 8, int max_requests = 100000);
    
    ~threadpool();
    
    bool append(T* request);
    
    void run();

private:
    static void* worker(void* arg);

private:
    int m_thread_number;
    
    pthread_t* m_threads;
    
    int m_max_requests;
    
    std::list<T*> m_workqueue;
    locker m_queuelocker;
    
    sem m_queuestat;
    
    bool m_stop;
};


template< typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) : 
    m_thread_number(thread_number), m_max_requests(max_requests),
    m_stop(false), m_threads(NULL){
     
    if((thread_number <= 0) || (max_requests <= 0)){
        throw std::exception();
    }

    m_threads = new pthread_t[m_thread_number];
    if(!m_threads){
        throw std::exception();
    }

    // 创建thread_number个线程，并设置为线程脱离
    for(int i = 0; i < thread_number; i += 1){
        printf("create the %d thread\n", i);

        if(pthread_create(m_threads + i, NULL, worker, this) != 0){
            delete[] m_threads;
            throw std::exception();
        }

        if(pthread_detach(m_threads[i])){
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template<typename T>
threadpool<T>::~threadpool(){
    delete [] m_threads;
    m_stop = true;
}


template<typename T>
bool threadpool<T>::append(T* request){
    m_queuelocker.lock();
    if(m_workqueue.size() > m_max_requests){
        m_queuelocker.unlock();
        return false;
    }

    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}


template<typename T>
void* threadpool<T>::worker(void* arg){
    threadpool* pool = (threadpool*) arg;
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run(){
    while(!m_stop){
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty()){
            m_queuelocker.unlock();
            continue;
        }

        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();

        if(!request){
            continue;
        }

        request->process();

    }
}



#endif


