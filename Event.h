/*
 * Event.h
 *
 *  Created on: Dec 22, 2012
 *      Author: xltyt
 */

#ifndef EVENT_H_
#define EVENT_H_

#include<pthread.h>
#define EVENT_WAIT_TIMEOUT -1
#define EVENT_SUCCESS 	  0
#define INFINITE          -1
class CEventCond
{
public:
    CEventCond();
    virtual ~CEventCond();

public:
    /*
     * 发送事件
     */
    virtual int SetEvent();
    virtual int ResetEvent();
    /*
     * 等待事件
     */
    virtual int WaitEvent(int mSec=INFINITE);

private:
    // 事件变量
    pthread_cond_t   m_pCond;
    // 互斥变量
    pthread_mutex_t  m_pMutex;
    // 发送状态
    int     m_nSendStatus;
};


#endif /* EVENT_H_ */
