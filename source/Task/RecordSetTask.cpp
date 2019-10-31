///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：RecordSetTask.cpp
// 创建者：jiangcheng
// 创建时间：2009-06-01
// 内容描述：自动录像设置任务类
///////////////////////////////////////////////////////////////////////////////////////////
#include "RecordSetTask.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "RecordTask.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../DBAccess/DBManager.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include <vector>
#include <iostream>
#include "ace/Task.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
using namespace std;

RecordSetTask::RecordSetTask() : DeviceIndependentTask()
{

}

RecordSetTask::RecordSetTask(std::string strXML) : DeviceIndependentTask(strXML)
{
	
}

RecordSetTask::~RecordSetTask()
{

}
void RecordSetTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]设置录像任务执行 !\n",DeviceID));
	bRun = true;
	SetRunning();

	std::string Action;
	std::vector<std::string> TaskXMLVec;
	std::vector<XMLTask*> vecTask;
	RetValue = RUN_SUCCESS;
	TranslateDownXML::TranslateRecordTask(strStandardXML,TaskXMLVec);
	//特殊处理模拟监测自动录像设置逻辑
	//所有自动录像逻辑同一，设置任务删除所有历史自动录像		-- Modify by gxd 2010-11-02
	if ((GetDVBType() == ATV || GetDVBType() == RADIO || GetDVBType() == AM ||GetDVBType() == CTV) && strStandardXML.find("AutoRecord")!=string::npos)
	{
		BUSINESSLAYOUTMGR::instance()->DelRecInfo(GetDVBType(),"0");	//删除所有其他自动录像,仅仅删除了执行队列
		BUSINESSLAYOUTMGR::instance()->DelRecInfo(AM, "0");	//删除AM所有其他自动录像,仅仅删除了执行队列
		DBMANAGER::instance()->DeleteTask(GetDVBType(),string("Record TaskID=\"0\""));
		OSFunction::Sleep(1);
	}
	for (size_t i=0; i< TaskXMLVec.size();++i)
	{
		XmlParser psr;
		int deviceid = -1;
		psr.Set_xml(TaskXMLVec[i]);
		pXMLNODE recordNode;
		if(TaskXMLVec[i].find("AutoRecord")!=string::npos)
		{
			recordNode=psr.GetNodeFromPath("Msg/AutoRecord/Record");
		}
		else
		{
			recordNode=psr.GetNodeFromPath("Msg/TaskRecord/Record");
		}
		psr.GetAttrNode(recordNode,"Action",Action);
		psr.GetAttrNode(recordNode,"TaskID",TaskID);
	 	psr.GetAttrNode(recordNode,"DeviceID",deviceid);
		std::string strTmpFreq;
		psr.GetAttrNode(recordNode,"Freq",strTmpFreq);
		/*2018-10-15
		 *
		 *山西铜川 没有AM接口但是需要AM的业务 
		 *
		 *这里只是简单的判断频点的大小，小于某一值就假设为AM业务
		 */
// 		if(StrUtil::Str2Float(strTmpFreq) < 30.0)
// 		{
// 			pXMLNODE rootNode = psr.GetNodeFromPath( "Msg" );
// 			psr.SetAttrNode("DVBType", std::string("AM"), rootNode);
// 			psr.SaveToString(TaskXMLVec[i]);
// 		}
 		//if ((GetDVBType()==ATV) && (deviceid !=-1))
		//{
		//	deviceid = deviceid+1;
		//}
		//if ((GetDVBType()==RADIO) && (deviceid !=-1))
		//{
		//	deviceid = deviceid+9;
		//}

		if(Action=="Set"||Action=="set")
		{
			RecordTask* recordtask = new RecordTask(TaskXMLVec[i]);
			BUSINESSLAYOUTMGR::instance()->AddRecTask(recordtask);
			
			vecTask.push_back(recordtask); //
			// OSFunction::Sleep(0,500);
		 }
		else if(Action=="Del"||Action=="del")
		{	
			if(deviceid != 0 && !BUSINESSLAYOUTMGR::instance()->DelTask(deviceid,TaskID))
			{
				RetValue=DATABASEERROR;
			}
			else if(deviceid == 0 && !BUSINESSLAYOUTMGR::instance()->DelTask(TaskID))
			{
				RetValue=DATABASEERROR;
			}
			RetValue=RUN_SUCCESS;
		}
	}
	SendXML(TranslateUpXML::TranslateRecord(this,vecTask));//发送到前端
	OSFunction::Sleep(1);
	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]设置录像任务停止 !\n",DeviceID));
}

std::string RecordSetTask::GetObjectName()
{
	return std::string("RecordSetTask");
}
string RecordSetTask::GetTaskName()
{
	return "设置录像任务";
}
