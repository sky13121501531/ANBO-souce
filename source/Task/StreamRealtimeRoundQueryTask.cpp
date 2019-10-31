
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

	TaskID = "MainTask";//设置为主任务
	
	DecivcXMLVec=TranslateXMLForDevice::TranslateRoundStreamXML(strXML);//发送给硬件的命令队列
	ChannelNum=DecivcXMLVec.size();	//轮播的频道数
    ChannelIndex=0;				//计数器

	
	//判断轮播任务是否含有非法频道
	XmlParser psr;
	psr.Set_xml(strXML);
	pXMLNODE node=psr.GetNodeFromPath("Msg/StreamRoundQuery");
	std::string roundtime;
	psr.GetAttrNode(node,"RoundTime",roundtime);

	RoundTime=TimeUtil::StrToSecondTime(roundtime);//轮播时间

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
		
	//设置调度
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
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]实时视频轮播主任务执行 !\n",DeviceID));
	}
	else
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]实时视频轮播辅任务执行 !\n",DeviceID));
	}
	bRun = true;
	SetRunning();

	if(ChannelNum==0)
		RetValue=RUN_FAILED;

	//判断视频轮播任务设置是否成功,设置失败，直接给用户发送失败信息
	if(RetValue != SET_SUCCESS)
	{
		if(TaskID == "MainTask")
			SendXML(TranslateUpXML::TranslateStreamRoundQuery(this));

		if (TaskScheduler != NULL)
			TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));

		SetFinised();//任务停止
		

		if (TaskID == "MainTask")
			ACE_DEBUG((LM_DEBUG,"(%T | %t)实时视频轮播主任务设置失败!\n"));
		else
			ACE_DEBUG((LM_DEBUG,"(%T | %t)实时视频轮播辅任务设置失败!\n"));

		return;
	}

	if (TaskID == "MainTask")
	{
		//通道查找
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

		if (bOneDevice == false)//主任务生成辅助任务
		{
			XMLTask* pTask = new StreamRealtimeRoundQueryTask(strStandardXML);
			pTask->SetTaskID("AideTask");	//设置为辅助任务
			pTask->SetDeciveID(idleDeviceID);

			((StreamRealtimeRoundQueryTask*)(pTask))->SetRelatedTask(this);
			RelatedTask = ((StreamRealtimeRoundQueryTask*)(pTask));

			BUSINESSLAYOUTMGR::instance()->AddTask(pTask);
			if (pTask->GetRetValue() != SET_SUCCESS)		//辅助任务设置失败
			{
				RetValue = pTask->GetRetValue();
				SendXML(TranslateUpXML::TranslateStreamRoundQuery(this));

				if (TaskScheduler != NULL)
					TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));

				SetFinised();//任务停止
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
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)轮播任务获取轮播通道失败 !\n"));
	}
	//wz_0217

	if (TaskID == "MainTask" && bOneDevice == true)//单通道且轮播频道不为空
	{
		TSFETCHERMGR::instance()->SetSendSwitch(busyDeviceID,true);
		TSFETCHERMGR::instance()->IncreaseTaskNum(busyDeviceID);

		bool FirstCheck = true;
		while (bRun&&IsRunning()) 
		{
			//wz_0217
			if (TSSENDERMGR::instance()->HasClient(roundchannel) == false)//没有连接用户，任务退出
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
				if(ChannelIndex>ChannelNum-1)//到达最后一个命令，返回第一个
					ChannelIndex = 0;
				bSendToIdleDev=false;
			}
			OSFunction::Sleep(0,100);
		}
	}
	else if (TaskID == "MainTask" && bOneDevice == false)//两个通道且轮播频道不为空
	{	
	//	TSSENDERMGR::instance()->SetSendSwitch(idleDeviceID,false);

		TSFETCHERMGR::instance()->SetSendSwitch(busyDeviceID,true);

		TSFETCHERMGR::instance()->SetTsDeviceXml(busyDeviceID,DecivcXMLVec[ChannelIndex]);//先向任务通道发送第一个命令
		TSFETCHERMGR::instance()->IncreaseTaskNum(busyDeviceID);
		TSFETCHERMGR::instance()->IncreaseTaskNum(idleDeviceID);
		bool FirstCheck = true;
		while ( bRun&&IsRunning()&&StopSignal==false) 
		{
			//wz_0217
			if (TSSENDERMGR::instance()->HasClient(roundchannel) == false)//没有连接用户，任务退出
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
				if(++ChannelIndex>ChannelNum-1)//到达最后一个命令，返回第一个
					ChannelIndex = 0;

				TSFETCHERMGR::instance()->SetTsDeviceXml(idleDeviceID,DecivcXMLVec[ChannelIndex]);
				bSendToIdleDev=true;
			}
			if(time(0)-lastTime>=RoundTime)
			{
				lastTime=time(0);
				//交换空闲通道和任务通道
				deviceid=idleDeviceID;
				idleDeviceID=busyDeviceID;
				busyDeviceID=deviceid;
				//通知空闲通道停止发数据，任务通道发送数据 

				TSFETCHERMGR::instance()->SetSendSwitch(idleDeviceID,false);
				OSFunction::Sleep(0,500);
				TSFETCHERMGR::instance()->SetSendSwitch(busyDeviceID,true);
				bSendToIdleDev=false;
			}
			OSFunction::Sleep(0,100);
		}
	}
	else if(TaskID == "AideTask")		//辅助任务
	{
		while(bRun&&StopSignal==false)
		{
			OSFunction::Sleep(0,500);
		}
	}

	//任务停止后逻辑
	if (StopSignal == false && RelatedTask != NULL)	//关联任务没有停止
	{
		RelatedTask->SetStopSignal();//通知其停止
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
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]实时视频轮播主任务停止 !\n",DeviceID));
	}
	else
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]实时视频轮播辅任务停止 !\n",DeviceID));
	}
	g_realstreamround = false;
}
string StreamRealtimeRoundQueryTask::GetTaskName()
{
	if (TaskID == "MainTask")
		return std::string("实时视频轮播主任务");
	else
		return std::string("实时视频轮播辅任务");
}

std::string StreamRealtimeRoundQueryTask::GetObjectName()
{
	return std::string("StreamRealtimeRoundQueryTask");
}
