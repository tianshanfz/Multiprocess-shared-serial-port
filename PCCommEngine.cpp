#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/shm.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include"PCCommEngine.h"

extern "C" bool  RecvDataINternal(int n_PsIndex, char cpRXBuffer[MAX_RECV_LENGTH],int len )
{
  //  int n_RecvIndex=p_map->m_vecPsInfo[n_PsIndex].m_RecvIndex;
     //strcat(p_map->m_vecPsInfo[n_PsIndex].m_Recv,cpRXBuffer);

  /*  if(cpRXBuffer[len-1]=='#'||cpRXBuffer[len-1]=='\n'||cpRXBuffer[len-1]=='\0')//结束符
    {
         p_map->m_vecPsInfo[n_PsIndex].m_CanRead[n_RecvIndex]=true;

         p_map->m_vecPsInfo[n_PsIndex].m_RecvIndex++;  //接受到完整数据
        printf("recv count:%d   data:%s(thread)\n",n_RecvIndex,p_map->m_vecPsInfo[n_PsIndex].m_Recv[n_RecvIndex]);

    }*/


    return true;
}
extern "C" void* ThreadReadProc(void* data)
{
	Comm* p_map=(Comm*)data;

	while(1)
	{
	     int n_PsIndex=p_map->m_PsCount-1;
	    	int fd=p_map->m_vecPsInfo[0].m_hCommFile;
	    	int len=0;
            ioctl(fd,FIONREAD,&len);//获取缓冲区数据数
            if(len>0)
            {
                 char cpRXBuffer[MAX_RECV_LENGTH];
                 usleep(5000);  //保证数据读完整
                len=read(fd,cpRXBuffer,MAX_RECV_LENGTH);
                    sem_t *sem = sem_open(SEM_NAME, 0);
                    sem_wait(sem);
                    strcat(p_map->m_vecPsInfo[n_PsIndex].m_Recv,cpRXBuffer);
                    sem_post(sem);
                    printf("recv  data:%s(thread)\n",p_map->m_vecPsInfo[n_PsIndex].m_Recv);

            }

            usleep(7000);
	}
	return 0;
}
extern "C" bool Config(Comm* p_map,int fd)
{
    p_map->m_dConfig.c_cflag|=CS8;			//! Byte of the Data.
    p_map->m_dConfig.c_cflag&=~CSTOPB;	//! Use one bit for stopbit.
    p_map->m_dConfig.c_cflag&=~PARENB;	//! No parity bit
    p_map->m_dConfig.c_cflag&=~(OPOST);
    p_map->m_dConfig.c_cflag |= CLOCAL |CREAD;
//	m_dConfig.c_cc[VTIME]=100;
//	m_dConfig.c_cc[VMIN]=0;
    p_map->m_dConfig.c_lflag&=~(ICANON|ISIG|ECHO|ECHOE);
    p_map->m_dConfig.c_lflag&=~(INLCR|ICRNL|IGNCR);
    p_map->m_dConfig.c_oflag&=~(ONLCR|OCRNL);
    cfsetispeed(&p_map->m_dConfig,B57600);
    cfsetispeed(&p_map->m_dConfig,B57600);
    tcflush(fd,TCIOFLUSH);
    int status=tcsetattr(fd,TCSANOW,&p_map->m_dConfig);
    if(status!=0)
    {
        return false;
    }
    tcflush(fd,TCIOFLUSH);
    return true;
}
extern "C" bool AddPid(Comm * p_map,int fd)
{
    if(GetIndexFromPid(p_map)>=0)  //表示原队列中已存在该进程
    {
        return false;
    }
    if(p_map->m_PsCount>=MAX_PS_COUNT)
    {
        return false;
    }
     sem_t *sem = sem_open(SEM_NAME, 0);
    sem_wait(sem);
    p_map->m_vecPsInfo[p_map->m_PsCount].m_pid=getpid();
     p_map->m_vecPsInfo[p_map->m_PsCount++].m_hCommFile=fd;
     sem_post(sem);
    return true;
}
extern "C" bool DeletePid(Comm * p_map)
{
    int n_index=GetIndexFromPid(p_map);
    if(n_index==-1)  //原队列中不存在该进程
    {
         printf("ERROR:Delete failed\n");
        return false;
    }
    else
    {
         sem_t *sem = sem_open(SEM_NAME, 0);
     sem_wait(sem);
        p_map->m_PsCount--;

        for(int j=n_index;j<p_map->m_PsCount;j++)
        {
            p_map->m_vecPsInfo[j]=p_map->m_vecPsInfo[j+1];
        }
         sem_post(sem);
        return true;
    }
}
extern "C" int OpenCommInternal(const char * filename,Comm* m_map)
{
    char szRealPort[256]="/dev/";
    strcat(szRealPort, filename);

    int fd=open(szRealPort,O_RDWR|O_NOCTTY|O_NDELAY);

    if(fd == -1)
    {
         printf("ERROR :open failed\n");
        return false;
    }
    if(!Config(m_map,fd))
    {
        printf("ERROR :config failed\n");
        close( fd);
        return false;
    }

	 printf("SUCCESS :com open !!\n");
    return fd;
}
extern "C" bool openComm(const char * filename)
{
    Comm *p_map;
    int shm_id=shmget(KEY,4096,IPC_CREAT|0666);
    p_map=( Comm *)shmat(shm_id,NULL, 0);
    int i_fd=0;
    i_fd= OpenCommInternal(filename,p_map);
    if(i_fd==false)
    {
        return false;
    }
    if(p_map->m_PsCount==0)//第一个串口打开者
    {
        sem_t *sem = sem_open(SEM_NAME, OPEN_FLAG, OPEN_MODE, INIT_V);
        if(pthread_create(&p_map->m_pThreadRead,NULL,ThreadReadProc,(void*)p_map) )
        {
            printf("ERROR :ThreadRead created failed\n");
            return false;
        }

    }
     return AddPid(p_map,i_fd);
  /*
    m_bConnected = TRUE;*/
}
extern "C" bool closeComm()
{
    Comm *p_map;
    int shm_id=shmget(KEY,4096,IPC_CREAT|0666);
    p_map=( Comm *)shmat(shm_id,NULL,0);
    bool ret=DeletePid(p_map);
    //关闭串口
    int n_PsIndex=GetIndexFromPid(p_map);
     int fd=p_map->m_vecPsInfo[n_PsIndex].m_hCommFile;
    close(fd);
    if(p_map->m_PsCount==0)  //最后一个进程
    {

        pthread_cancel(p_map->m_pThreadRead);
        memset(p_map,0,4096);
       sem_t *sem = sem_open(SEM_NAME, 0);
         //删掉在系统创建的信号量
        sem_unlink(SEM_NAME);
	        //彻底销毁打开的信号量
       sem_close(sem);
    }
    if(shmdt(p_map) == -1)
    {
        printf("ERROR :shmdt failed\n");
        return false;
    }
    return ret;
}

extern "C" bool writeComm(const void* lpBuffer,int len)
{
    Comm *p_map;
    int shm_id=shmget(KEY,4096,IPC_CREAT|0666);
    p_map=( Comm *)shmat(shm_id,NULL,0);
    int n_PsIndex=GetIndexFromPid(p_map);
    int fd=p_map->m_vecPsInfo[n_PsIndex].m_hCommFile;
    int i_Ret = write(fd, lpBuffer,len);
    printf("%d",fd);
    if(i_Ret==-1)
    {
        printf("ERROR: write error\n");
        return false;
    }
 //   int ps_index=GetIndexFromPid(p_map);
  //  p_map->m_vecPsInfo[ps_index].m_WaitCount++;//标志该进程要接受数据
  //    printf("SUCCESS: write success data:%s,ret:%d\n",(char*)lpBuffer,i_Ret);
    return true;
}
extern "C" int GetIndexFromPid(Comm* p_map)
{
    int pid=getpid();
     for(int i=0;i<p_map->m_PsCount;i++)
     {
        if(pid==p_map->m_vecPsInfo[i].m_pid)
        {
            return i;
        }
     }
     return -1;
}
extern "C" int readComm(char* lpBuffer)
{
    Comm *p_map;
    int shm_id=shmget(KEY,4096,IPC_CREAT|0666);
    p_map=( Comm *)shmat(shm_id,NULL,0);

    int n_PsIndex=GetIndexFromPid(p_map);
   // int n_RecvIndex=p_map->m_vecPsInfo[n_PsIndex].m_RecvIndex;
    int ret=0;
    if(p_map->m_vecPsInfo[n_PsIndex].m_Recv[0]==0)//没有接受到数据
    {

    }
    else
    {

        printf("recv  data:%s\n",p_map->m_vecPsInfo[n_PsIndex].m_Recv);
    //    p_map->m_vecPsInfo[n_PsIndex].m_RecvIndex--;
    sem_t *sem = sem_open(SEM_NAME, 0);
     sem_wait(sem);
        strcpy(lpBuffer,p_map->m_vecPsInfo[n_PsIndex].m_Recv);//将接受队列尾的消息弹出

     //   p_map->m_vecPsInfo[n_PsIndex].m_WaitCount--;
    //    if(p_map->m_vecPsInfo[n_PsIndex].m_WaitCount<=0)
    //    {
            memset(p_map->m_vecPsInfo[n_PsIndex].m_Recv,0,MAX_RECV_LENGTH);
           sem_post(sem);

            ret= strlen(lpBuffer);
     //   }
    }
    return ret;
}
