
#include "StreamRealtimeRoundQueryTask.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "TranslateXMLForDevice.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/TimeUtil.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../DeviceAccess/TsFetcherMgr.h"
#include "../Foundation/AppLog.h"
#include "ace/Synch.h"
#include "Scheduler.h"
#include "../Communications/TsSenderMgr.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include <vector>
#include <iostream>
extern bool g_realstreamround;
using namespace std;

StreamRealtimeRoundQueryTask::StreamRealtimeRoundQueryTask() : DeviceRelatedTask()
{

}

StreamRealtimeRoundQueryTask::StreamRealtimeRoundQueryTask(std::string strXML) : DeviceRelatedTask(strXML)
{	
	StopSignal = false;
	RelatedTask = NULL;

	bOneDevice=false;
	bSendToIdleDev=false;

	TaskID = "MainTask";//����Ϊ������
	
	DecivcXMLVec=TranslateXMLForDevice::TranslateRoundStreamXML(strXML);//���͸�Ӳ�����������
	ChannelNum=DecivcXMLVec.size();	//�ֲ���Ƶ����
    ChannelIndex=0;				//������

	
	//�ж��ֲ������Ƿ��зǷ�Ƶ��
	XmlParser psr;
	psr.Set_xml(strXML);
	pXMLNODE node=psr.GetNodeFromPath("Msg/StreamRoundQuery");
	std::string roundtime;
	psr.GetAttrNode(node,"RoundTime",roundtime);

	RoundTime=TimeUtil::StrToSecondTime(roundtime);//�ֲ�ʱ��

	pXMLNODELIST nodelist=psr.GetNodeList(node);

	for (int i=0;i<nodelist->Size();i++)
	{
		node=psr.GetNextNode(nodelist);
		string Freq,OrgNetID,TsID,ServiceID,VideoPID,AudioPID;
		psr.GetAttrNode(node,"Freq",Freq);
		psr.GetAttrNode(node,"OrgNetID",OrgNetID);
		psr.GetAttrNode(node,"TsID",TsID);
		psr.GetAttrNode(node,"ServiceID",ServiceID);
		psr.GetAttrNode(node,"VideoPID",VideoPID);
		psr.GetAttrNode(node,"AudioPID",AudioPID);
		if (DVBType==ATV||RADIO==DVBType||AM==DVBType||CTV==DVBType)
		{
			ChannelID=Freq;
		}
		else
			CHANNELINFOMGR::instance()->GetChannelID(DVBType,OrgNetID,TsID,ServiceID,VideoPID,AudioPID,"",ChannelID);

		if (ChannelID =="")
		{
			bRun = false;
			break;
		}
	}
		
	//���õ���
	TaskScheduler = new Scheduler();
	if (TaskScheduler != NULL)
		TaskScheduler->SetRunTime(TimeUtil::GetCurDateTime());
}

StreamRealtimeRoundQueryTask::~StreamRealtimeRoundQueryTask()
{

}

void StreamRealtimeRoundQueryTask::Run(void)
{
	g_realstreamround = true;
	if (TaskID == "MainTask")
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]ʵʱ��Ƶ�ֲ�������ִ�� !\n",DeviceID));
	}
	else
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]ʵʱ��Ƶ�ֲ�������ִ�� !\n",DeviceID));
	}
	bRun = true;
	SetRunning();

	if(ChannelNum==0)
		RetValue=RUN_FAILED;

	//�ж���Ƶ�ֲ����������Ƿ�ɹ�,����ʧ�ܣ�ֱ�Ӹ��û�����ʧ����Ϣ
	if(RetValue != SET_SUCCESS)
	{
		if(TaskID == "MainTask")
			SendXML(TranslateUpXML::TranslateStreamRoundQuery(this));

		if (TaskScheduler != NULL)
			TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));

		SetFinised();//����ֹͣ
		

		if (TaskID == "MainTask")
			ACE_DEBUG((LM_DEBUG,"(%T | %t)ʵʱ��Ƶ�ֲ�����������ʧ��!\n"));
		else
			ACE_DEBUG((LM_DEBUG,"(%T | %t)ʵʱ��Ƶ�ֲ�����������ʧ��!\n"));

		return;
	}

	if (TaskID == "MainTask")
	{
		//ͨ������
		std::list<int> devicelist;
		PROPMANAGER::instance()->GetTaskDeviceList(GetObjectName(),DVBType,devicelist);

		if (devicelist.size() == 1)
		{
			busyDeviceID = DeviceID;
			bOneDevice=true;
		}
		else
		{
			busyDeviceID = DeviceID;
			std::list<int>::iterator ptr = devicelist.begin();
			for (;ptr!=devicelist.end();++ptr)
			{
				if (*ptr != DeviceID)
				{
					idleDeviceID = *ptr;
					break;
				}
			}
			(ptr == devicelist.end()) ? bOneDevice=true : bOneDevice=false;		
		}

		std::string rtnXML;
		RetValue = RUN_SUCCESS;

		if (bOneDevice == false)//���������ɸ�������
		{
			XMLTask* pTask = new StreamRealtimeRoundQueryTask(strStandardXML);
			pTask->SetTaskID("AideTask");	//����Ϊ��������
			pTask->SetDeciveID(idleDeviceID);

			((StreamRealtimeRoundQueryTask*)(pTask))->SetRelatedTask(this);
			RelatedTask = ((StreamRealtimeRoundQueryTask*)(pTask));

			BUSINESSLAYOUTMGR::instance()->AddTask(pTask);
			if (pTask->GetRetValue() != SET_SUCCESS)		//������������ʧ��
			{
				RetValue = pTask->GetRetValue();
				SendXML(TranslateUpXML::TranslateStreamRoundQuery(this));

				if (TaskScheduler != NULL)
					TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));

				SetFinised();//����ֹͣ
				return;
			}
		}

		string aidedevice;
		bOneDevice == false ? aidedevice = StrUtil::Int2Str(idleDeviceID) : aidedevice = "";
		SendXML(TranslateUpXML::TranslateStreamRoundQuery(this,aidedevice));
	}

	int deviceid=0;
	time_t lastTime=time(0);
	//wz_0217
	int roundchannel = 0;
	if(false == PROPMANAGER::instance()->GetVirDeviceId(DVBType, roundchannel))
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)�ֲ������ȡ�ֲ�ͨ��ʧ�� !\n"));
	}
	//wz_0217

	if (TaskID == "MainTask" && bOneDevice == true)//��ͨ�����ֲ�Ƶ����Ϊ��
	{
		TSFETCHERMGR::instance()->SetSendSwitch(busyDeviceID,true);
		TSFETCHERMGR::instance()->IncreaseTaskNum(busyDeviceID);

		bool FirstCheck = true;
		while (bRun&&IsRunning()) 
		{
			//wz_0217
			if (TSSENDERMGR::instance()->HasClient(roundchannel) == false)//û�������û��������˳�
			{
				if (FirstCheck == true)
				{
					FirstCheck = false;
					OSFunction::Sleep(5,0);
					continue;
				}
				break;
			}
			if(!bSendToIdleDev)
			{
				TSFETCHERMGR::instance()->SetTsDeviceXml(busyDeviceID,DecivcXMLVec[ChannelIndex]);
				bSendToIdleDev=true;
			}
			if(time(0)-lastTime>=RoundTime)
			{
				lastTime=time(0);
				++ChannelIndex;
				if(ChannelIndex>ChannelNum-1)//�������һ��������ص�һ��
					ChannelIndex = 0;
				bSendToIdleDev=false;
			}
			OSFunction::Sleep(0,100);
		}
	}
	else if (TaskID == "MainTask" && bOneDevice == false)//����ͨ�����ֲ�Ƶ����Ϊ��
	{	
	//	TSSENDERMGR::instance()->SetSendSwitch(idleDeviceID,false);

		TSFETCHERMGR::instance()->SetSendSwitch(busyDeviceID,true);

		TSFETCHERMGR::instance()->SetTsDeviceXml(busyDeviceID,DecivcXMLVec[ChannelIndex]);//��������ͨ�����͵�һ������
		TSFETCHERMGR::instance()->IncreaseTaskNum(busyDeviceID);
		TSFETCHERMGR::instance()->IncreaseTaskNum(idleDeviceID);
		bool FirstCheck = true;
		while ( bRun&&IsRunning()&&StopSignal==false) 
		{
			//wz_0217
			if (TSSENDERMGR::instance()->HasClient(roundchannel) == false)//û�������û��������˳�
			{
				if (FirstCheck == true)
				{
					FirstCheck = false;
					OSFunction::Sleep(3,0);
					continue;
				}
				break;
			}
			if(!bSendToIdleDev)
			{
				if(++ChannelIndex>ChannelNum-1)//�������һ��������ص�һ��
					ChannelIndex = 0;

				TSFETCHERMGR::instance()->SetTsDeviceXml(idleDeviceID,DecivcXMLVec[ChannelIndex]);
				bSendToIdleDev=true;
			}
			if(time(0)-lastTime>=RoundTime)
			{
				lastTime=time(0);
				//��������ͨ��������ͨ��
				deviceid=idleDeviceID;
				idleDeviceID=busyDeviceID;
				busyDeviceID=deviceid;
				//֪ͨ����ͨ��ֹͣ�����ݣ�����ͨ���������� 

				TSFETCHERMGR::instance()->SetSendSwitch(idleDeviceID,false);
				OSFunction::Sleep(0,500);
				TSFETCHERMGR::instance()->SetSendSwitch(busyDeviceID,true);
				bSendToIdleDev=false;
			}
			OSFunction::Sleep(0,100);
		}
	}
	else if(TaskID == "AideTask")		//��������
	{
		while(bRun&&StopSignal==false)
		{
			OSFunction::Sleep(0,500);
		}
	}

	//����ֹͣ���߼�
	if (StopSignal == false && RelatedTask != NULL)	//��������û��ֹͣ
	{
		RelatedTask->SetStopSignal();//֪ͨ��ֹͣ
	}
	if(TaskID == "MainTask")
	{
		TSFETCHERMGR::instance()->SetSendSwitch(idleDeviceID,false);
		TSFETCHERMGR::instance()->SetSendSwitch(busyDeviceID,false);
		TSSENDERMGR::instance()->SetSendSwitch(idleDeviceID,true);
		TSSENDERMGR::instance()->SetSendSwitch(busyDeviceID,true);
	}
	
	bRun = false;

	if (TaskScheduler != NULL)
		TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));

	SetFinised();
	if(TaskID == "MainTask" && bOneDevice == true)
	{
		TSFETCHERMGR::instance()->DecreaseTaskNum(busyDeviceID);
	}
	if (TaskID == "MainTask" && bOneDevice == false)
	{
		TSFETCHERMGR::instance()->DecreaseTaskNum(busyDeviceID);
		TSFETCHERMGR::instance()->DecreaseTaskNum(idleDeviceID);
	}
	if(TaskID == "MainTask")
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]ʵʱ��Ƶ�ֲ�������ֹͣ !\n",DeviceID));
	}
	else
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]ʵʱ��Ƶ�ֲ�������ֹͣ !\n",DeviceID));
	}
	g_realstreamround = false;
}
string StreamRealtimeRoundQueryTask::GetTaskName()
{
	if (TaskID == "MainTask")
		return std::string("ʵʱ��Ƶ�ֲ�������");
	else
		return std::string("ʵʱ��Ƶ�ֲ�������");
}

std::string StreamRealtimeRoundQueryTask::GetObjectName()
{
	return std::string("StreamRealtimeRoundQueryTask");
}
