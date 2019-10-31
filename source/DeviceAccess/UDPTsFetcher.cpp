
#include "UDPTsFetcher.h"
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
//#include "./CardType.h"

ACE_Thread_Mutex UDPTsFetcher::TaskRoundMutex;

UDPTsFetcher::UDPTsFetcher(int deviceid) : TsFetcher(deviceid)
{
	//string TsIp = PROPMANAGER::instance()->GetDeviceIp();
	/*TsIp = PROPMANAGER::instance()->GetHttpServerIP();
	TsIp = PROPMANAGER::instance()->GetDbIp();
	TsIp = PROPMANAGER::instance()->GetFtpServerIP();
	TsIp = PROPMANAGER::instance()->GetHttpVideoIp();
	TsIp = PROPMANAGER::instance()->GetRtspVideoIp();
	TsIp = PROPMANAGER::instance()->GetHttpServerIP();
	TsIp = PROPMANAGER::instance()->GetQualityIp();
	TsIp = PROPMANAGER::instance()->GetRecordQualityIp();
	TsIp = PROPMANAGER::instance()->GetXmlServerIP();
	TsIp = PROPMANAGER::instance()->GetLogIP();*/
	/*int tsport=0;
	PROPMANAGER::instance()->GetDeviceTsPort(deviceid,tsport);

	local_addr.set_address(TsIp.c_str(),(int)TsIp.length());
	local_addr.set_port_number(tsport);

	int rtn = DeviceSock.open(local_addr);

	if (rtn != 0)	//UDP端口失败
	{
		string info = string("音视频网络问题，请检查[") + TsIp + string(":")+ StrUtil::Int2Str(tsport) + string("]");
		OSFunction::ExitProcess(info);
	}
	*/
// 	string TsIp = "238.0.0.100";
// 	int tsport = 11000 + DeviceId;
// 	local_addr.set_address(TsIp.c_str(),(int)TsIp.length());
// 	local_addr.set_port_number(tsport);
// 
// 	int rtn = DeviceSock.open(local_addr);
// 
// 	if (rtn != 0)	//UDP端口失败
// 	{
// 		string info = string("音视频网络问题，请检查[") + TsIp + string(":")+ StrUtil::Int2Str(tsport) + string("]");
// 		OSFunction::ExitProcess(info);
// 	}

	TsSendTaskPointer = NULL;			//视频任务
	TsSendRoundTaskPointer = NULL;		//轮播任务
	RecordTaskPointerVec.clear();
	bFlag = false;
	bSendSwtich=false;
	bSendOrder=false;
	DeviceTaskXml = OSFunction::CreateTaskTypeXml(DeviceId);
	CurDeviceXml = "";
	TaskNum=0;
	
	const int PKGSIZE = 188;
	int value =PKGSIZE*5000;	
	
// 	DeviceSock.set_option(SOL_SOCKET,SO_SNDBUF,(char*)&value,sizeof(value));
// 	DeviceSock.set_option(SOL_SOCKET,SO_RCVBUF,(char*)&value,sizeof(value));

}

UDPTsFetcher::~UDPTsFetcher()
{

}
int UDPTsFetcher::Start()
{
	open(0);
	return 0;
}

int UDPTsFetcher::open(void*)
{
	bFlag = true;
	NewPackageHead = false;
	ReSendSwitch = false;

	//设置接收队列缓冲区大小
	msg_queue()->high_water_mark(188*10000);
	msg_queue()->low_water_mark(188*1000);
	

	this->activate();

	return 0;
}

int UDPTsFetcher::svc()
{
	eDVBType dvbtype = UNKNOWN;
	PROPMANAGER::instance()->GetDeviceType(DeviceId,dvbtype);


	std::string TsIp;
	int tsport;
	PROPMANAGER::instance()->GetDeviceTsIP(DeviceId, TsIp);
	PROPMANAGER::instance()->GetDeviceTsPort(DeviceId, tsport);

// 	string deviceIP = "";
// 	PROPMANAGER::instance()->GetDeviceIP(DeviceId, deviceIP);
// 	string TsIp = "238.0.0." + deviceIP.substr(deviceIP.rfind(".") + 1);
// 	unsigned int tsport=11000+DeviceId;


	string mulitcase = TsIp + string(":")+ StrUtil::Int2Str(tsport);
	ACE_INET_Addr multicast_addr_(mulitcase.c_str()); 

//	DeviceSock.join(multicast_addr_,1,(PROPMANAGER::instance()->GetDeviceIp()).c_str());
	
	DeviceSock.join(multicast_addr_);
	DeviceSock.set_option(IP_MULTICAST_TTL, 5);
	int nSocketBuffSize = 1024*1024;
	ACE_SOCK* sk= &DeviceSock;
	sk->set_option(SOL_SOCKET, SO_RCVBUF, &nSocketBuffSize, sizeof(int));
////////////

	int ReadSize	= 188*7;	//每次获得数据的大小
	int BufLen		= 188*70;	//每次发送或者写文件的块大小 
	unsigned char* RcvBuf	= new unsigned char[ReadSize];		//接收BUF
	unsigned char* TsBuf	= new unsigned char[BufLen];		//缓冲BUF
	memset(RcvBuf,0,ReadSize);
	memset(TsBuf,0,BufLen);
	
	int PacketLen = 0;
	ACE_Time_Value PutOutTime(ACE_OS::gettimeofday()+ACE_Time_Value(1));
	ACE_Time_Value RecvTimeOut(5);

	int NoDataCount = 0;

	int RebootCardCount = 0;
	time_t mRebootTime = time(0);		//初始化重启时间
	int nTimeValue = 60*60*8;			//8个小时重启一次

	time_t mReSendDeviceXMLTime = time(0);	//重发指令时间（同步时间，且保证节目正确）
	int nReSendDeviceXMLInterval = 86400;	//每天重发
	int TunerId = -1;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,TunerId);

	while(bFlag)
	{
		if (DeviceSock.get_handle()==NULL||CurDeviceXml==""||CurDeviceXml.empty()||TaskNum<=0)
		{
			//即使没有任务也接收数据，防止udp缓冲区被占满
			int RecvLen = DeviceSock.recv(RcvBuf,ReadSize,remote_addr,0,&RecvTimeOut);	//接收数据
			
			if(PacketLen!= 0)
			{
				memset(TsBuf,0,BufLen);
				PacketLen= 0;
			};
			OSFunction::Sleep(0,5);
			FlushHistoryDatum();						
			continue;
		}
		
		if (CurDeviceXml.empty() == false && bSendOrder == false)
		{
			if (TsSendTaskPointer != NULL)
			{
				TsSendTaskPointer->SetSendSwitch(false);
			}

			FlushHistoryDatum();

			SendTsDeviceXml();														//重发任务指令
			if (bSendOrder == false)
				continue;
			time_t flushtime = time(0)-1;	
			ACE_Time_Value sflushtime(ACE_OS::gettimeofday());
			ACE_Time_Value eflushtime(ACE_OS::gettimeofday()+ACE_Time_Value(1));
			//while(flushtime<time(0))
			while((eflushtime-sflushtime).msec()>0)
			{//消耗2秒历史数据
				ACE_Time_Value nowaittime (ACE_Time_Value::zero);
				if(DeviceSock.recv(RcvBuf,ReadSize,remote_addr,0,&nowaittime)<=0)	//消耗历史网络数据
					break;
				sflushtime = ACE_OS::gettimeofday();
			}
			if (TsSendTaskPointer != NULL)
			{
				TsSendTaskPointer->SetSendSwitch(true);
			}
		}

		//每隔固定时间重发强制重发任务指令
		if (time(0)-mReSendDeviceXMLTime >= nReSendDeviceXMLInterval && CurDeviceXml.empty() == false)
		{
			SendTsDeviceXml();
			mReSendDeviceXMLTime = time(0);
			std::string info = std::string("通道[") + StrUtil::Int2Str(DeviceId) + std::string("]强制重发任务指令");
			SYSMSGSENDER::instance()->SendMsg(info);
		}
		if(ReSendSwitch)
		{
			SendTsDeviceXml();
			ReSendSwitch = false;
		}

		memset(RcvBuf,0,ReadSize);
		int RecvLen = DeviceSock.recv(RcvBuf,ReadSize,remote_addr,0,&RecvTimeOut);	//接收数据
//		std::cout << "DeviceID  " << DeviceId << "recive : " << RecvLen << std::endl;
		if (RecvLen <= 0)
		{
			if(++NoDataCount >= 5000)
			{
				std::string info = std::string("通道[") + StrUtil::Int2Str(DeviceId) + std::string("]重发任务指令");
				SYSMSGSENDER::instance()->SendMsg(info);

				SendTsDeviceXml();													//重发任务指令
				NoDataCount = 0;
			}
			//达到时间阈值以及计数阈值时发送一次重启命令(注意时间判断与计数递增的顺序，防止RebootCardCount溢出)
			//RebootCardCount计数是为了保证运行到某个时刻无法获取数据时，连续超过100次才重启板卡，防止出现超时后偶尔接收不到即重启板卡
			//if (TunerId == 0 && time(0)-mRebootTime >= nTimeValue && ++RebootCardCount >= 100)
			//{
			//	bool ret = RebootCard();
			//	int RebootNum = 5;
			//	while(ret==false && --RebootNum)	//每隔8个小时重启，如果失败重试5次，如果依然失败等待下次运行周期
			//	{
			//		ret = RebootCard(); 
			//	}

			//	OSFunction::Sleep(100);
			//	RebootCardCount = 0;
			//	mRebootTime = time(0);
			//	bSendOrder = false;
			//}
			continue;
		}

		NoDataCount = 0;
		RebootCardCount = 0;

		if (BufLen-PacketLen > RecvLen)
		{
			memcpy(TsBuf+PacketLen,RcvBuf,RecvLen);	
			PacketLen += RecvLen;
			continue;
		}

		//第一次接收到数据包，处理包头，保证能够播放和录像文件能够播放
		unsigned char* pPackage = NULL;
		/*if (NewPackageHead == true)
		{
			int beginPos = FindBegin(TsBuf,PacketLen);
			PacketLen -= beginPos;
			pPackage = TsBuf + beginPos;
			NewPackageHead = false;
		}
		else*/
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
					mb->copy((char*)pPackage,PacketLen);
					if(-1 == TsSendRoundTaskPointer->putq(mb,&PutOutTime))
					{
						mb->release();
						TsSendRoundTaskPointer->msg_queue()->flush();
					//	ACE_DEBUG ((LM_DEBUG,"(%T | %t)\n 通道[%d] 放弃数据-Round\n",DeviceId));
					}
				}
			}
			catch(ACE_bad_alloc)
			{
				if (mb != NULL)
					mb->release();
				ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d] UDPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
			}
			catch (...)
			{
				if (mb != NULL)
					mb->release();
				ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d] UDPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
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
					mb->copy((char*)pPackage,PacketLen);
					if(-1 == TsSendTaskPointer->putq(mb,&PutOutTime))
					{
						mb->release();
						TsSendTaskPointer->msg_queue()->flush();
					//	ACE_DEBUG ((LM_DEBUG,"(%T | %t)\n通道[%d] 放弃数据-TsSend\n",DeviceId));
					}
				}
			}
			catch(ACE_bad_alloc)
			{
				if (mb != NULL)
					mb->release();
				ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d] UDPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
			}
			catch(...)
			{
				if (mb != NULL)
					mb->release();
				ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]UDPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
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
						mb->copy((char*)pPackage,PacketLen);
						if(-1 == (*ptr)->putq(mb,&PutOutTime))
						{
							mb->release();
							(*ptr)->msg_queue()->flush();
						//	ACE_DEBUG ((LM_DEBUG,"(%T | %t)\n通道[%d] 放弃数据-RecordTask\n",DeviceId));
						}
						//加入系统时间
						//if(dvbtype==THREED)
							PutSysTime(*ptr);
					}
				}
				catch(ACE_bad_alloc)
				{
					if (mb != NULL)
						mb->release();
					ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d] UDPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
				}
				catch(...)
				{
					if (mb != NULL)
						mb->release();
					ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]UDPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
				}
			}
		}
		TaskPointerMutex.release();

		memset(TsBuf,0,BufLen);
		PacketLen = 0;

		//将本次未处理数据拷贝进缓存
		memcpy(TsBuf+PacketLen,RcvBuf,RecvLen);	//拷贝至内存
		PacketLen += RecvLen;
	}
	delete RcvBuf;
	delete TsBuf;
	DeviceSock.close();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]停止获取TS流\n",DeviceId));
	bFlag = false;
	return 0;
}

bool UDPTsFetcher::SetTsSendTask(TsSender* task)
{
	if (task == NULL)
		return false;

	TaskPointerMutex.acquire();
	TsSendTaskPointer = task;		//设置视频任务的指针
	TaskPointerMutex.release();
	return true;
}

bool UDPTsFetcher::SetTsRoundTask(TsSender* task)
{
	if (task == NULL)
		return false;

	TaskRoundMutex.acquire();
	TsSendRoundTaskPointer = task;//设置视频录播任务的指针
	TaskRoundMutex.release();
	return true;
}

bool UDPTsFetcher::SetRecordTask( ACE_Task<ACE_MT_SYNCH>* task )
{
	if(task==NULL)
		return false;
	TaskPointerMutex.acquire();
	RecordTaskPointerVec.push_back(task);
	TaskPointerMutex.release();
	return true;
}

int UDPTsFetcher::Stop()
{
	bFlag = false;
	this->wait();

	return 0;
}
void UDPTsFetcher::StopHistoryTask()
{
	TaskMutex.acquire();
	CurDeviceXml = "";
	bSendOrder = false;
//	OSFunction::Sleep(1);
	TaskMutex.release();
	return;
}
bool UDPTsFetcher::SetTsDeviceXml( std::string devicexml )
{
	bool ret = false;
	
	if(devicexml.empty())
		return false;	

	if(devicexml==CurDeviceXml)
		return true;
	
	TaskMutex.acquire();
	CurDeviceXml = devicexml;
	bSendOrder = false;
	TaskMutex.release();

	return true;
}

bool UDPTsFetcher::SendTsDeviceXml()
{
	std::string retXML,TmpDeviceXml;
	std::string PmtPID,PcrPID,VideoPID,AudioPID;

	if (CurDeviceXml.empty())
	{
		//std::string info = std::string("通道[") + StrUtil::Int2Str(DeviceId) + std::string("]任务指令为空");
		//SYSMSGSENDER::instance()->SendMsg(info,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}

	TaskMutex.acquire();
	TmpDeviceXml = CurDeviceXml;

// 	TaskMutex.acquire();
// 	XmlParser tmpparser(CurDeviceXml.c_str());
// 	TaskMutex.release();
// 
// 	pXMLNODE TsNode = tmpparser.GetNodeFromPath( "Msg/TSQuery/TS" );
// 
// 	tmpparser.SetAttrNode( "TunerID",StrUtil::Int2Str(TunerID),TsNode );//重设指令中TunerID以及系统时间
// 	tmpparser.SetAttrNode( "SystemClock",(long)time(0),TsNode );
// 
// 	tmpparser.GetAttrNode(TsNode,"PmtPID",PmtPID);
// 	tmpparser.GetAttrNode(TsNode,"PcrPID",PcrPID);
// 	tmpparser.GetAttrNode(TsNode,"VideoPID",VideoPID);
// 	tmpparser.GetAttrNode(TsNode,"AudioPID",AudioPID);
// 
// 	std::string msg = string("通道[") + StrUtil::Int2Str(DeviceId)+ string("]指令:") + \
// 						string("PMT[") + PmtPID + string("] PCR[") + PcrPID + string("] VPID[") + \
// 						VideoPID + string("] APID[") + AudioPID + string("]");
// 
// 	SYSMSGSENDER::instance()->SendMsg(msg);*/
// 	
// 	tmpparser.SaveToString(TmpDeviceXml);
// 
	if (TmpDeviceXml.empty())
	{
		std::string info = std::string("通道[") + StrUtil::Int2Str(DeviceId) + std::string("]命令XML处理出错");
		SYSMSGSENDER::instance()->SendMsg(info,UNKNOWN,VS_MSG_SYSALARM,DEVICE);
		OSFunction::Sleep(1);
		bSendOrder = false;
		return false;
	}
	
	DEVICEACCESSMGR::instance()->SendTaskMsg(DeviceId,TmpDeviceXml,retXML);

	TaskMutex.release();
	
	if (retXML == "")
	{
		std::string info = std::string("通道[") + StrUtil::Int2Str(DeviceId) + std::string("]获取返回信息失败,返回xml为空");
		SYSMSGSENDER::instance()->SendMsg(info,UNKNOWN,VS_MSG_SYSALARM,DEVICE);
		OSFunction::Sleep(1);
		bSendOrder = false;
		return false;
	}
	
	string retvalue;
	XmlParser parser(retXML.c_str());
	pXMLNODE returtnNode = parser.GetNodeFromPath("Msg/Return");
	parser.GetAttrNode(returtnNode,"Value",retvalue);

	if (retvalue != "0")
	{
		std::string info = std::string("通道[") + StrUtil::Int2Str(DeviceId) + std::string("]任务指令失败:") + retXML;
		cout<<info<<endl;
		cout<<"任务指令为："<<TmpDeviceXml<<endl;//
		//	//SYSMSGSENDER::instance()->SendMsg(info,UNKNOWN,VS_MSG_SYSALARM,DEVICE);
		OSFunction::Sleep(10);
		bSendOrder = false;
		return false;
	}

	std::string rtninfo = std::string("通道[") + StrUtil::Int2Str(DeviceId) + std::string("任务指令成功");
	SYSMSGSENDER::instance()->SendMsg(rtninfo);

	bSendOrder = true;
	return true;
}

bool UDPTsFetcher::RebootCard()
{
	
	std::string retXML;
	std::string CardRebootXML = OSFunction::CreateCardRebootXml(DeviceId);
	DEVICEACCESSMGR::instance()->SendTaskMsg(DeviceId,CardRebootXML,retXML);

	if (retXML == "")
	{
		std::string info = std::string("通道[") + StrUtil::Int2Str(DeviceId) + std::string("]获取返回信息失败--UDP");
		SYSMSGSENDER::instance()->SendMsg(info,UNKNOWN,VS_MSG_SYSALARM,DEVICE);
		
		bSendOrder = false;
		return false;
	}

	string retvalue;
	XmlParser parser(retXML.c_str());
	pXMLNODE returtnNode = parser.GetNodeFromPath("Msg/Return");
	parser.GetAttrNode(returtnNode,"Value",retvalue);

	if (retvalue != "0")
	{
		std::string info = std::string("通道[") + StrUtil::Int2Str(DeviceId) + std::string("]发送主动重启指令失败");
		SYSMSGSENDER::instance()->SendMsg(info,UNKNOWN,VS_MSG_SYSALARM,DEVICE);

		return false;
	}

	std::string info = std::string("通道[") + StrUtil::Int2Str(DeviceId) + std::string("]发送主动重启指令成功");
	SYSMSGSENDER::instance()->SendMsg(info);

	return true;
}

bool UDPTsFetcher::FlushHistoryDatum()
{
	TaskRoundMutex.acquire();
	//清空轮播队列中数据
	if (TsSendRoundTaskPointer != NULL)
	{
		TsSendRoundTaskPointer->msg_queue()->flush();
	}
	TaskRoundMutex.release();

	TaskPointerMutex.acquire();
	//清空发送队列中数据
	if (TsSendTaskPointer != NULL)
	{
		TsSendTaskPointer->msg_queue()->flush();
	}
	//清空录像队列中数据
	std::vector<ACE_Task<ACE_MT_SYNCH>*>::iterator ptr = RecordTaskPointerVec.begin();
	for(;ptr!=RecordTaskPointerVec.end();++ptr)
	{
		if ((*ptr) != NULL)//录像任务指针不为空，则将获得数据放入录像任务的数据队列中
		{
			(*ptr)->msg_queue()->flush();
		}
	}
	TaskPointerMutex.release();
	return true;
}

bool UDPTsFetcher::DelRecordTask( ACE_Task<ACE_MT_SYNCH>* task )
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
void UDPTsFetcher::SetReSendSwitch(bool sendswitch)
{
	ReSendSwitch = sendswitch;
}
void UDPTsFetcher::SetSendSwitch( bool sendswitch )
{
	TaskRoundMutex.acquire();
	bSendSwtich=sendswitch;
	TaskRoundMutex.release();

	if (sendswitch == true)
	{
		NewPackageHead = true;	//切换轮播任务指令的时候需呀做包头处理
	}
}

void UDPTsFetcher::IncreaseTaskNum()
{
	TaskNumMutex.acquire();
	TaskNum++;
	TaskNumMutex.release();
}

void UDPTsFetcher::DecreaseTaskNum()
{
	TaskNumMutex.acquire();
	TaskNum--;
	TaskNumMutex.release();
}

void UDPTsFetcher::TimeConvert(time_t curtime,unsigned char *timebuf)
{
	timebuf[0]=(curtime>>56);
	timebuf[1]=(curtime>>48)&(0x00ff);
	timebuf[2]=(curtime>>40)&(0x0000ff);
	timebuf[3]=(curtime>>32)&(0x000000ff);
	timebuf[4]=(curtime>>24)&(0x00000000ff);
	timebuf[5]=(curtime>>16)&(0x0000000000ff);
	timebuf[6]=(curtime>>8)&(0x000000000000ff);
	timebuf[7]=(curtime)&(0x00000000000000ff);
}
void UDPTsFetcher::PutSysTime(ACE_Task<ACE_MT_SYNCH>* task)
{
	if (task == NULL)
		return;

	//给数据加入系统时间
	unsigned char Timebuf[188]={0};
	Timebuf[0]=0x47;
	Timebuf[2]=0x1A;

	TimeConvert(time(0),&Timebuf[4]);

	ACE_Message_Block *TimeMb = NULL;
	try
	{
		TimeMb = new ACE_Message_Block(188);
		if (TimeMb != NULL)
		{
			TimeMb->copy((char*)Timebuf,188);
		}
		ACE_Time_Value PutOutTime(ACE_OS::gettimeofday()+ACE_Time_Value(1));
		task->putq(TimeMb,&PutOutTime);
	}
	catch(ACE_bad_alloc)
	{
		if (TimeMb != NULL)
			TimeMb->release();
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d] UDPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
	}
	catch(...)
	{
		if (TimeMb != NULL)
			TimeMb->release();
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]UDPTsFetcher使用ACE_Message_Block出错\n",DeviceId));
	}
	return;
}