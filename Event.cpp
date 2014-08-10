

/*
 * File:   EventCond.cpp
 * Author: root
 *
 * Created on 2010年4月27日, 上午8:13
 */

#include "Event.h"


CEventCond::CEventCond()
{
    // gthread must be initialized.

	pthread_mutex_init( &m_pMutex , NULL);

	pthread_cond_init( &m_pCond , NULL);
    m_nSendStatus = 0;
}

CEventCond::~CEventCond()
{
        pthread_mutex_destroy(&m_pMutex);
        pthread_cond_destroy(&m_pCond);

    m_nSendStatus = 0;
}

/*
 * 函数作用：发送事件
 * 函数参数：
 * 函数返回：
 *  int 成功：0  失败：－1
 */
int CEventCond::SetEvent()
{
	pthread_mutex_lock(&m_pMutex);
    pthread_cond_signal(&m_pCond);
    pthread_mutex_unlock(&m_pMutex);

    m_nSendStatus = 1;

    return 0;
}
int CEventCond::ResetEvent()
{
    pthread_mutex_destroy(&m_pMutex);
	pthread_cond_destroy(&m_pCond);
	pthread_mutex_init( &m_pMutex , NULL);
	pthread_cond_init( &m_pCond , NULL);

    m_nSendStatus = 0;

    return 0;
}

/*
 * 函数作用：等待事件
 * 函数参数：
 *  nWaitTime ： 等待超时时间（秒）
 * 函数返回：
 *  SUCCESS ： 成功
 *  WAIT_TIMEOUT： 等待超时
 *
 */
int CEventCond::WaitEvent(int mSec)// 毫秒
{
	int nRet = EVENT_WAIT_TIMEOUT;
    if ( m_nSendStatus == 1  )
    {
        return EVENT_SUCCESS;
    }
	pthread_mutex_lock(&m_pMutex);
	if(mSec==INFINITE)
	{
		pthread_cond_wait(&m_pCond,&m_pMutex);
		nRet=EVENT_SUCCESS;
	}
    // lock
    else
    {
        // 超时等待时间
        // 要使用绝对时间，不然等待直接返回超时（time(NULL) + nWaitTime）
        timespec t;
        	 t.tv_sec = time(NULL)+mSec/1000;
        	 t.tv_nsec = mSec%1000*1000000;
        if ( pthread_cond_timedwait(&m_pCond,&m_pMutex, &t)==EVENT_SUCCESS )
        {
            nRet = EVENT_SUCCESS;
        }

    }
    // unlock
    pthread_mutex_unlock(&m_pMutex);
    return nRet;
}

