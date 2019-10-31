
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

	if (rtn != 0)	//UDP�˿�ʧ��
	{
		string info = string("����Ƶ�������⣬����[") + TsIp + string(":")+ StrUtil::Int2Str(tsport) + string("]");
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
// 	if (rtn != 0)	//UDP�˿�ʧ��
// 	{
// 		string info = string("����Ƶ�������⣬����[") + TsIp + string(":")+ StrUtil::Int2Str(tsport) + string("]");
// 		OSFunction::ExitProcess(info);
// 	}

	TsSendTaskPointer = NULL;			//��Ƶ����
	TsSendRoundTaskPointer = NULL;		//�ֲ�����
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

	//���ý��ն��л�������С
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

	int ReadSize	= 188*7;	//ÿ�λ�����ݵĴ�С
	int BufLen		= 188*70;	//ÿ�η��ͻ���д�ļ��Ŀ��С 
	unsigned char* RcvBuf	= new unsigned char[ReadSize];		//����BUF
	unsigned char* TsBuf	= new unsigned char[BufLen];		//����BUF
	memset(RcvBuf,0,ReadSize);
	memset(TsBuf,0,BufLen);
	
	int PacketLen = 0;
	ACE_Time_Value PutOutTime(ACE_OS::gettimeofday()+ACE_Time_Value(1));
	ACE_Time_Value RecvTimeOut(5);

	int NoDataCount = 0;

	int RebootCardCount = 0;
	time_t mRebootTime = time(0);		//��ʼ������ʱ��
	int nTimeValue = 60*60*8;			//8��Сʱ����һ��

	time_t mReSendDeviceXMLTime = time(0);	//�ط�ָ��ʱ�䣨ͬ��ʱ�䣬�ұ�֤��Ŀ��ȷ��
	int nReSendDeviceXMLInterval = 86400;	//ÿ���ط�
	int TunerId = -1;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,TunerId);

	while(bFlag)
	{
		if (DeviceSock.get_handle()==NULL||CurDeviceXml==""||CurDeviceXml.empty()||TaskNum<=0)
		{
			//��ʹû������Ҳ�������ݣ���ֹudp��������ռ��
			int RecvLen = DeviceSock.recv(RcvBuf,ReadSize,remote_addr,0,&RecvTimeOut);	//��������
			
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

			SendTsDeviceXml();														//�ط�����ָ��
			if (bSendOrder == false)
				continue;
			time_t flushtime = time(0)-1;	
			ACE_Time_Value sflushtime(ACE_OS::gettimeofday());
			ACE_Time_Value eflushtime(ACE_OS::gettimeofday()+ACE_Time_Value(1));
			//while(flushtime<time(0))
			while((eflushtime-sflushtime).msec()>0)
			{//����2����ʷ����
				ACE_Time_Value nowaittime (ACE_Time_Value::zero);
				if(DeviceSock.recv(RcvBuf,ReadSize,remote_addr,0,&nowaittime)<=0)	//������ʷ��������
					break;
				sflushtime = ACE_OS::gettimeofday();
			}
			if (TsSendTaskPointer != NULL)
			{
				TsSendTaskPointer->SetSendSwitch(true);
			}
		}

		//ÿ���̶�ʱ���ط�ǿ���ط�����ָ��
		if (time(0)-mReSendDeviceXMLTime >= nReSendDeviceXMLInterval && CurDeviceXml.empty() == false)
		{
			SendTsDeviceXml();
			mReSendDeviceXMLTime = time(0);
			std::string info = std::string("ͨ��[") + StrUtil::Int2Str(DeviceId) + std::string("]ǿ���ط�����ָ��");
			SYSMSGSENDER::instance()->SendMsg(info);
		}
		if(ReSendSwitch)
		{
			SendTsDeviceXml();
			ReSendSwitch = false;
		}

		memset(RcvBuf,0,ReadSize);
		int RecvLen = DeviceSock.recv(RcvBuf,ReadSize,remote_addr,0,&RecvTimeOut);	//��������
//		std::cout << "DeviceID  " << DeviceId << "recive : " << RecvLen << std::endl;
		if (RecvLen <= 0)
		{
			if(++NoDataCount >= 5000)
			{
				std::string info = std::string("ͨ��[") + StrUtil::Int2Str(DeviceId) + std::string("]�ط�����ָ��");
				SYSMSGSENDER::instance()->SendMsg(info);

				SendTsDeviceXml();													//�ط�����ָ��
				NoDataCount = 0;
			}
			//�ﵽʱ����ֵ�Լ�������ֵʱ����һ����������(ע��ʱ���ж������������˳�򣬷�ֹRebootCardCount���)
			//RebootCardCount������Ϊ�˱�֤���е�ĳ��ʱ���޷���ȡ����ʱ����������100�β������忨����ֹ���ֳ�ʱ��ż�����ղ����������忨
			//if (TunerId == 0 && time(0)-mRebootTime >= nTimeValue && ++RebootCardCount >= 100)
			//{
			//	bool ret = RebootCard();
			//	int RebootNum = 5;
			//	while(ret==false && --RebootNum)	//ÿ��8��Сʱ���������ʧ������5�Σ������Ȼʧ�ܵȴ��´���������
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

		//��һ�ν��յ����ݰ���������ͷ����֤�ܹ����ź�¼���ļ��ܹ�����
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

		//��Ƶ�ֲ�����ָ�벻Ϊ�գ��򽫻�����ݷ�����Ƶ�����̵߳����ݶ�����
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
					//	ACE_DEBUG ((LM_DEBUG,"(%T | %t)\n ͨ��[%d] ��������-Round\n",DeviceId));
					}
				}
			}
			catch(ACE_bad_alloc)
			{
				if (mb != NULL)
					mb->release();
				ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d] UDPTsFetcherʹ��ACE_Message_Block����\n",DeviceId));
			}
			catch (...)
			{
				if (mb != NULL)
					mb->release();
				ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d] UDPTsFetcherʹ��ACE_Message_Block����\n",DeviceId));
			}
		}
		TaskRoundMutex.release();

		//��Ƶ����ָ�벻Ϊ�գ��򽫻�����ݷ�����Ƶ�����̵߳����ݶ�����
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
					//	ACE_DEBUG ((LM_DEBUG,"(%T | %t)\nͨ��[%d] ��������-TsSend\n",DeviceId));
					}
				}
			}
			catch(ACE_bad_alloc)
			{
				if (mb != NULL)
					mb->release();
				ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d] UDPTsFetcherʹ��ACE_Message_Block����\n",DeviceId));
			}
			catch(...)
			{
				if (mb != NULL)
					mb->release();
				ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]UDPTsFetcherʹ��ACE_Message_Block����\n",DeviceId));
			}
		}
		//��Ƶ¼������ָ�벻Ϊ�գ��򽫻�����ݷ�����Ƶ�����̵߳����ݶ�����
		std::vector<ACE_Task<ACE_MT_SYNCH>*>::iterator ptr = RecordTaskPointerVec.begin();
		for(;ptr!=RecordTaskPointerVec.end();++ptr)
		{
			if ((*ptr) != NULL)//¼������ָ�벻Ϊ�գ��򽫻�����ݷ���¼����������ݶ�����
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
						//	ACE_DEBUG ((LM_DEBUG,"(%T | %t)\nͨ��[%d] ��������-RecordTask\n",DeviceId));
						}
						//����ϵͳʱ��
						//if(dvbtype==THREED)
							PutSysTime(*ptr);
					}
				}
				catch(ACE_bad_alloc)
				{
					if (mb != NULL)
						mb->release();
					ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d] UDPTsFetcherʹ��ACE_Message_Block����\n",DeviceId));
				}
				catch(...)
				{
					if (mb != NULL)
						mb->release();
					ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]UDPTsFetcherʹ��ACE_Message_Block����\n",DeviceId));
				}
			}
		}
		TaskPointerMutex.release();

		memset(TsBuf,0,BufLen);
		PacketLen = 0;

		//������δ�������ݿ���������
		memcpy(TsBuf+PacketLen,RcvBuf,RecvLen);	//�������ڴ�
		PacketLen += RecvLen;
	}
	delete RcvBuf;
	delete TsBuf;
	DeviceSock.close();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]ֹͣ��ȡTS��\n",DeviceId));
	bFlag = false;
	return 0;
}

bool UDPTsFetcher::SetTsSendTask(TsSender* task)
{
	if (task == NULL)
		return false;

	TaskPointerMutex.acquire();
	TsSendTaskPointer = task;		//������Ƶ�����ָ��
	TaskPointerMutex.release();
	return true;
}

bool UDPTsFetcher::SetTsRoundTask(TsSender* task)
{
	if (task == NULL)
		return false;

	TaskRoundMutex.acquire();
	TsSendRoundTaskPointer = task;//������Ƶ¼�������ָ��
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
		//std::string info = std::string("ͨ��[") + StrUtil::Int2Str(DeviceId) + std::string("]����ָ��Ϊ��");
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
// 	tmpparser.SetAttrNode( "TunerID",StrUtil::Int2Str(TunerID),TsNode );//����ָ����TunerID�Լ�ϵͳʱ��
// 	tmpparser.SetAttrNode( "SystemClock",(long)time(0),TsNode );
// 
// 	tmpparser.GetAttrNode(TsNode,"PmtPID",PmtPID);
// 	tmpparser.GetAttrNode(TsNode,"PcrPID",PcrPID);
// 	tmpparser.GetAttrNode(TsNode,"VideoPID",VideoPID);
// 	tmpparser.GetAttrNode(TsNode,"AudioPID",AudioPID);
// 
// 	std::string msg = string("ͨ��[") + StrUtil::Int2Str(DeviceId)+ string("]ָ��:") + \
// 						string("PMT[") + PmtPID + string("] PCR[") + PcrPID + string("] VPID[") + \
// 						VideoPID + string("] APID[") + AudioPID + string("]");
// 
// 	SYSMSGSENDER::instance()->SendMsg(msg);*/
// 	
// 	tmpparser.SaveToString(TmpDeviceXml);
// 
	if (TmpDeviceXml.empty())
	{
		std::string info = std::string("ͨ��[") + StrUtil::Int2Str(DeviceId) + std::string("]����XML��������");
		SYSMSGSENDER::instance()->SendMsg(info,UNKNOWN,VS_MSG_SYSALARM,DEVICE);
		OSFunction::Sleep(1);
		bSendOrder = false;
		return false;
	}
	
	DEVICEACCESSMGR::instance()->SendTaskMsg(DeviceId,TmpDeviceXml,retXML);

	TaskMutex.release();
	
	if (retXML == "")
	{
		std::string info = std::string("ͨ��[") + StrUtil::Int2Str(DeviceId) + std::string("]��ȡ������Ϣʧ��,����xmlΪ��");
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
		std::string info = std::string("ͨ��[") + StrUtil::Int2Str(DeviceId) + std::string("]����ָ��ʧ��:") + retXML;
		cout<<info<<endl;
		cout<<"����ָ��Ϊ��"<<TmpDeviceXml<<endl;//
		//	//SYSMSGSENDER::instance()->SendMsg(info,UNKNOWN,VS_MSG_SYSALARM,DEVICE);
		OSFunction::Sleep(10);
		bSendOrder = false;
		return false;
	}

	std::string rtninfo = std::string("ͨ��[") + StrUtil::Int2Str(DeviceId) + std::string("����ָ��ɹ�");
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
		std::string info = std::string("ͨ��[") + StrUtil::Int2Str(DeviceId) + std::string("]��ȡ������Ϣʧ��--UDP");
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
		std::string info = std::string("ͨ��[") + StrUtil::Int2Str(DeviceId) + std::string("]������������ָ��ʧ��");
		SYSMSGSENDER::instance()->SendMsg(info,UNKNOWN,VS_MSG_SYSALARM,DEVICE);

		return false;
	}

	std::string info = std::string("ͨ��[") + StrUtil::Int2Str(DeviceId) + std::string("]������������ָ��ɹ�");
	SYSMSGSENDER::instance()->SendMsg(info);

	return true;
}

bool UDPTsFetcher::FlushHistoryDatum()
{
	TaskRoundMutex.acquire();
	//����ֲ�����������
	if (TsSendRoundTaskPointer != NULL)
	{
		TsSendRoundTaskPointer->msg_queue()->flush();
	}
	TaskRoundMutex.release();

	TaskPointerMutex.acquire();
	//��շ��Ͷ���������
	if (TsSendTaskPointer != NULL)
	{
		TsSendTaskPointer->msg_queue()->flush();
	}
	//���¼�����������
	std::vector<ACE_Task<ACE_MT_SYNCH>*>::iterator ptr = RecordTaskPointerVec.begin();
	for(;ptr!=RecordTaskPointerVec.end();++ptr)
	{
		if ((*ptr) != NULL)//¼������ָ�벻Ϊ�գ��򽫻�����ݷ���¼����������ݶ�����
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
		NewPackageHead = true;	//�л��ֲ�����ָ���ʱ����ѽ����ͷ����
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

	//�����ݼ���ϵͳʱ��
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
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d] UDPTsFetcherʹ��ACE_Message_Block����\n",DeviceId));
	}
	catch(...)
	{
		if (TimeMb != NULL)
			TimeMb->release();
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]UDPTsFetcherʹ��ACE_Message_Block����\n",DeviceId));
	}
	return;
}