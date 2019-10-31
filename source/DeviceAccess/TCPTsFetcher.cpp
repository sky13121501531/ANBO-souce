
#include "TCPTsFetcher.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/AppLog.h"
#include "ace/OS.h"
#include "ace/Synch.h"
#include "./DeviceAccessMgr.h"
#include "../Communications/TsSender.h"
#include "DeviceAccess.h"
#include "../Foundation/XmlParser.h"
#include "../Communications/TSServer.h"
#include "../Communications/SysMsgSender.h"

ACE_Thread_Mutex TCPTsFetcher::TaskRoundMutex;

TCPTsFetcher::TCPTsFetcher(int deviceid) : TsFetcher(deviceid)
{
	TsSendTaskPointer = NULL;//视频任务
	TsSendRoundTaskPointer = NULL;//轮播任务
	RecordTaskPointerVec.clear();
	bFlag = false;
	bSendSwtich=false;
	DeviceTaskXml = OSFunction::CreateTaskTypeXml(DeviceId);
	CurDeviceXml = "";
	URL = PROPMANAGER::instance()->GetTCPTSUrl(deviceid);;

	const int PKGSIZE = 188;
	int value =PKGSIZE*5000;	
	TaskNum=0;
	DeviceSock.set_option(SOL_SOCKET,SO_SNDBUF,(char*)&value,sizeof(value));
	DeviceSock.set_option(SOL_SOCKET,SO_RCVBUF,(char*)&value,sizeof(value));
	//
	m_bReboot = true;
}

TCPTsFetcher::~TCPTsFetcher()
{

}
int TCPTsFetcher::Start()
{
	open(0);
	return 0;
}

int TCPTsFetcher::open(void*)
{
	bFlag = true;
	NewPackageHead = false;
	ReSendSwitch = false;

	this->activate();

	return 0;
}

int TCPTsFetcher::svc()
{
	int ReadSize	= 188*7;	//每次获得数据的大小
	int BufLen		= 188*70;	//每次发送或者写文件的块大小 

	unsigned char* RcvBuf	= new unsigned char[ReadSize];		//接收BUF
	unsigned char* TsBuf	= new unsigned char[BufLen];		//缓冲BUF

	memset(RcvBuf,0,ReadSize);
	memset(TsBuf,0,BufLen);
	
	int PacketLen = 0;
	ACE_Time_Value PutOutTime(ACE_OS::gettimeofday()+ACE_Time_Value(5));
	ACE_Time_Value RecvTimeOut(10);

	while(bFlag)
	{
		if (CurDeviceXml == ""||TaskNum<=0)
		{
			OSFunction::Sleep(0,100);
			continue;
		}

		SendTsDeviceXml();

		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]开始获取TS流\n\t\t地址<%s>!\n",DeviceId,URL.c_str()));
		
		URLChanged = false;
		int FailCount = 0;

		TaskPointerMutex.acquire();
		std::vector<ACE_Task<ACE_MT_SYNCH>*>::iterator ptr = RecordTaskPointerVec.begin();
		for(;ptr!=RecordTaskPointerVec.end();++ptr)
		{
			if ( *ptr != NULL)
			{
				(*ptr)->msg_queue()->flush();
			}
		}
		TaskPointerMutex.release();

		DeviceSock.close();
		DEVICEACCESSMGR::instance()->GetTsReceiveHandle(URL,DeviceSock);//连接url指定的服务器，保存连接socket
		time_t Resendtime = time(0);
		bool Resend = false;
		int  resendnum = 10;
		while(URLChanged == false && bFlag)
		{
			if(ReSendSwitch||(time(0)-Resendtime>30&&Resend&&resendnum>=0)||(time(0)-Resendtime>30*60&&Resend&&resendnum<-96))
			{
				string retXML;
				DEVICEACCESSMGR::instance()->SendTaskMsg(DeviceId,CurDeviceXml,retXML);
				ReSendSwitch = false;
				Resend = false;
				Resendtime = time(0);
				resendnum--;
			}

			if (URL == "" || DeviceSock.get_handle() == NULL||TaskNum<=0)
			{
				break;
			}
			//
			time_t nGetTime = TimeUtil::StrToDateTime(TimeUtil::GetCurDate() + " 03:10:00");
			if(time(0)>=nGetTime)
			{
				if(!m_bReboot)
				{
					DEVICEACCESSMGR::instance()->SetSysTime(DeviceId+50);
					m_bReboot = true;
				}
			}
			else
			{
				m_bReboot = false;
			}
			//
			memset(RcvBuf,0,ReadSize);
			int RecvLen = DeviceSock.recv(RcvBuf,ReadSize,&RecvTimeOut);//接收数据

			if (RecvLen <= 0)
			{
				//重新获取Socket
				TaskMutex.acquire();
				if (URL != "")
				{
					ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]重连服务器!\n",DeviceId));
					DeviceSock.close();
					DEVICEACCESSMGR::instance()->GetTsReceiveHandle(URL,DeviceSock);
					Resend = true;
				}
				TaskMutex.release();
				OSFunction::Sleep(0,500);
				continue;
			}
			//
			resendnum = 10;
			//
			if (BufLen-PacketLen > RecvLen)
			{
				memcpy(TsBuf+PacketLen,RcvBuf,RecvLen);	
				PacketLen += RecvLen;
				continue;
			}

			//第一次接收到数据包，处理包头，保证能够播放和录像文件能够播放
			unsigned char* pPackage = NULL;
			if(PacketLen > 0)
			{
				if (NewPackageHead == true)
				{
					int beginPos = FindBegin(TsBuf,PacketLen);
					PacketLen -= beginPos;
					pPackage = TsBuf + beginPos;
					NewPackageHead = false;
				}
				else
				{
					pPackage = TsBuf;
				}
				//视频轮播任务指针不为空，则将获得数据放入视频发送线程的数据队列中
				TaskRoundMutex.acquire();
				if (TsSendRoundTaskPointer != NULL && bSendSwtich == true)
				{
					ACE_Message_Block *mb = NULL;
					try
					{
						mb = new ACE_Message_Block(PacketLen);
						if (mb != NULL)
						{
							memcpy(mb->wr_ptr(),pPackage,PacketLen);
							mb->wr_ptr(PacketLen);

							if(-1 == TsSendRoundTaskPointer->putq(mb,&PutOutTime))
							{
								mb->release();
								TsSendRoundTaskPointer->msg_queue()->flush();
							}
						}					
					}
					catch (...)
					{
						if (mb != NULL)
							mb->release();
						ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d] TCPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
					}
				}
				TaskRoundMutex.release();

				//视频任务指针不为空，则将获得数据放入视频发送线程的数据队列中
				TaskPointerMutex.acquire();
				if (TsSendTaskPointer != NULL)
				{
					ACE_Message_Block *mb = NULL;
					try
					{
						mb = new ACE_Message_Block(PacketLen);
						if (mb != NULL)
						{
							memcpy(mb->wr_ptr(),pPackage,PacketLen);
							mb->wr_ptr(PacketLen);
							if(-1 == TsSendTaskPointer->putq(mb,&PutOutTime))
							{
								mb->release();
								TsSendTaskPointer->msg_queue()->flush();
							}
						}					
					}
					catch(...)
					{
						if (mb != NULL)
							mb->release();
						ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d] TCPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
					}
					
				}
				//视频录像任务指针不为空，则将获得数据放入视频发送线程的数据队列中
				std::vector<ACE_Task<ACE_MT_SYNCH>*>::iterator ptr = RecordTaskPointerVec.begin();
				for(;ptr!=RecordTaskPointerVec.end();++ptr)
				{
					if ((*ptr) != NULL)//录像任务指针不为空，则将获得数据放入录像任务的数据队列中
					{
						ACE_Message_Block *mb = NULL;
						try
						{
							mb = new ACE_Message_Block(PacketLen);
							if (mb != NULL)
							{
								memcpy(mb->wr_ptr(),pPackage,PacketLen);
								mb->wr_ptr(PacketLen);
								if(-1 == (*ptr)->putq(mb,&PutOutTime))
								{
									mb->release();
									(*ptr)->msg_queue()->flush();
								}
							}
						}
						catch(...)
						{
							if (mb != NULL)
								mb->release();
							ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d] TCPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
						}
					}
				}
				TaskPointerMutex.release();
			}
			memset(TsBuf,0,BufLen);
			PacketLen = 0;
			//将本次未处理数据拷贝进缓存
			if (BufLen-PacketLen > RecvLen)
			{
				memcpy(TsBuf+PacketLen,RcvBuf,RecvLen);	//拷贝至内存
				PacketLen += RecvLen;
			}
		}
		DeviceSock.close();

		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]停止获取TS流\n",DeviceId));
	}

	delete RcvBuf;
	delete TsBuf;
	bFlag = false;

	return 0;
}

bool TCPTsFetcher::SetTsSendTask(TsSender* task)
{
	if (task == NULL)
		return false;

	TaskPointerMutex.acquire();
	TsSendTaskPointer = task;//设置视频任务的指针
	TaskPointerMutex.release();
	return true;
}

bool TCPTsFetcher::SetTsRoundTask(TsSender* task)
{
	if (task == NULL)
		return false;

	TaskRoundMutex.acquire();
	TsSendRoundTaskPointer = task;//设置视频录播任务的指针
	TaskRoundMutex.release();
	return true;
}

bool TCPTsFetcher::SetRecordTask( ACE_Task<ACE_MT_SYNCH>* task )
{
	if(task==NULL)
		return false;
	TaskPointerMutex.acquire();
	RecordTaskPointerVec.push_back(task);
	TaskPointerMutex.release();
	return true;
}

int TCPTsFetcher::Stop()
{
	bFlag = false;
	this->wait();

	return 0;
}

bool TCPTsFetcher::SetTsDeviceXml( std::string devicexml )
{
	bool ret = false;
	
	if(devicexml=="")
		return false;

	if(devicexml==CurDeviceXml)
		return true;
	else
	{	
		TaskMutex.acquire();
		CurDeviceXml=devicexml;
		//URL = "";			//重新获取地址 
		URLChanged=true;
		DeviceSock.close();
		TaskMutex.release();
	}
	return ret;
}

bool TCPTsFetcher::SendTsDeviceXml()
{
	TaskMutex.acquire();
	bool NoTaskXml = CurDeviceXml.empty();
	TaskMutex.release();

	if (NoTaskXml == true)
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]无任务指令\n",DeviceId));
		return true;
	}

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]发送任务指令\n",DeviceId));

//	NewPackageHead = true;	//发送新的任务指令的时候需要做包头处理

	std::string retXML;

	TaskMutex.acquire();
	DEVICEACCESSMGR::instance()->SendTaskMsg(DeviceId,CurDeviceXml,retXML);
	TaskMutex.release();

	if (retXML == "")
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]获取返回信息失败--TCP\n",DeviceId));
		return false;
	}
	
	string retvalue;
	XmlParser parser(retXML.c_str());
	pXMLNODE returtnNode = parser.GetNodeFromPath("Msg/Return");
	parser.GetAttrNode(returtnNode,"Value",retvalue);

	if (retvalue != "0")
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]获取地址失败\n",DeviceId));
		return false;
	}

	//pXMLNODE queryNode = parser.GetNodeFromPath("Msg/Stream/MediaStream");
	//parser.GetAttrNode(queryNode,"URL",URL);
	//if (URL == "")
	//{
	//	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]获取地址失败\n",DeviceId));
	//	return false;
	//}
	return true;
}

bool TCPTsFetcher::DelRecordTask( ACE_Task<ACE_MT_SYNCH>* task )
{
	TaskPointerMutex.acquire();
	std::vector<ACE_Task<ACE_MT_SYNCH>*>::iterator ptr=RecordTaskPointerVec.begin();
	for(;ptr!=RecordTaskPointerVec.end();)
	{		
		if((*ptr)==task)
		{
			delete *ptr;
			*ptr = NULL;
			ptr=RecordTaskPointerVec.erase(ptr++);
			break;
		}
		else
			ptr++;		
	}
	TaskPointerMutex.release();
	return true;
}

void TCPTsFetcher::SetSendSwitch( bool sendswitch )
{
	TaskRoundMutex.acquire();
	bSendSwtich=sendswitch;
	TaskRoundMutex.release();

	if (sendswitch == true)
	{
		NewPackageHead = true;	//切换轮播任务指令的时候需呀做包头处理
	}
}

void TCPTsFetcher::IncreaseTaskNum()
{
	TaskNumMutex.acquire();
	TaskNum++;
	TaskNumMutex.release();
}

void TCPTsFetcher::DecreaseTaskNum()
{
	TaskNumMutex.acquire();
	TaskNum--;
	TaskNumMutex.release();
}

void TCPTsFetcher::StopHistoryTask()
{
	return;
}
void TCPTsFetcher::SetReSendSwitch(bool sendswitch)
{
	ReSendSwitch = sendswitch;
	return;
}