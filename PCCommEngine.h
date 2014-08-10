/*
 * PCCommEngine.h
 *
 *  Created on: Dec 19, 2012
 *      Author: xltyt
 */

#ifndef PCCOMMENGINE_H_
#define PCCOMMENGINE_H_
#include<pthread.h>
#include<termios.h>
#include <semaphore.h>
#define MAX_PS_COUNT    5
//#define MAX_RECV_COUNT   8
#define MAX_RECV_LENGTH  64*8
#define KEY               64

#define SEM_NAME "mysem"
#define OPEN_FLAG O_RDWR|O_CREAT
#define OPEN_MODE 00777
#define INIT_V    1
using namespace std;


//
typedef struct ps_info
{
    pid_t           m_pid;                                      //进程ID
    char            m_Recv[MAX_RECV_LENGTH];  //进程接受队列
    int             m_hCommFile;
 //   int            m_WaitCount;  //标志在等待的消息数量 决定接受线程接受的数据是否保留
 //   int             m_RecvIndex;                            //接受队列正在接受的消息号
  //  bool            m_bCanRead[MAX_RECV_COUNT];                            //是否可读
}ps_info;
typedef struct
{

    pthread_t       m_pThreadRead;
    termios	        m_dConfig;//串口配置变量
    ps_info         m_vecPsInfo[MAX_PS_COUNT];//进程信息队列
    int             m_PsCount;//当前使用串口的进程数
}Comm;


extern "C"{

bool AddPid(Comm * p_map,int fd);
bool openComm(const char * filename);
typedef bool openComm_t(const char*);
int OpenCommInternal(const char * filename,Comm* p_map);
bool DeletePid(Comm * p_map);
int GetIndexFromPid(Comm*);
bool Config(Comm* p_map,int fd);
bool closeComm();
typedef int closeComm_t();
bool writeComm(const void* lpBuffer,int len);
typedef bool writeComm_t(const void*,int);
int readComm(char* lpBuffer);
typedef int readComm_t(char*);

}
#endif /* PCCOMMENGINE_H_ */
