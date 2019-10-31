
#include "SpectrumQueryTask.h"
#include "TranslateDownXML.h"
#include "ace/Log_Msg.h"
#include "../Foundation/TimeUtil.h"
#include "../DBAccess/DBManager.h"
#include "../Foundation/StrUtil.h"
#include "TranslateUpXML.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/OSFunction.h"

SpectrumQueryTask::SpectrumQueryTask() : DeviceIndependentTask()
{

}

SpectrumQueryTask::SpectrumQueryTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

SpectrumQueryTask::~SpectrumQueryTask()
{
}

void SpectrumQueryTask::Run()
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]频谱查询任务执行 !\n",DeviceID));
	SetRunning();
	XmlParser psr;
	psr.Set_xml(strStandardXML);
	RetValue = RUN_SUCCESS;
	std::string taskid,equcode,startdatetime,enddatetime,reporttime;
	std::string STD;
	pXMLNODE childnode=psr.GetNodeFromPath("Msg/SpectrumQuery");
	try
	{
		psr.GetAttrNode(childnode,string("EquCode"),equcode);        //获取Quality节点的ReportTime属性
		psr.GetAttrNode(childnode,string("TaskID"),taskid);                //获取Quality节点的TaskID属性
		psr.GetAttrNode(childnode,string("StartDateTime"),startdatetime);  //获取Quality节点的StartDateTime属性
		psr.GetAttrNode(childnode,string("EndDateTime"),enddatetime);      //获取Quality节点的EndDateTime属性

	}
	catch(...)
	{
		string error = "获取SpectrumQuery节点属性失败";
		APPLOG::instance()->WriteLog( OTHER,LOG_EVENT_ERROR,error,LOG_OUTPUT_BOTH );
		SetFinised();
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]频谱查询任务停止 !\n",DeviceID));
	}
	vector<eSpecInfo> vecSpecInfo;
	//查询指标测量结果信息
	if( DBMANAGER::instance()->QuerySpecInfo(DVBType,taskid,vecSpecInfo,startdatetime,enddatetime) ==false )
	{
		RetValue = DATABASEERROR;
	}
	
	string sxml = TranslateUpXML::TranslateSpectrumQuery(this,vecSpecInfo);
	SendXML(sxml);
	// cout<<"\n这是发送的频谱数据<<<<<<<<<<<<<<<"<<sxml<<"<<<<<<<<<<<<<<<\n";

	
	//将结果发送到前端系统
	// OSFunction::Sleep(3); //等待发送结束
	
	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]频谱查询任务停止 !\n",DeviceID));
}

std::string SpectrumQueryTask::GetTaskName()
{
	return "频谱查询任务";
}

std::string SpectrumQueryTask::GetObjectName()
{
	return std::string("QualityQueryTask");
}