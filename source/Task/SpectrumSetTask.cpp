

#include "SpectrumSetTask.h"
#include "TranslateDownXML.h"
#include "TranslateDownXMLForCTTB.h"
#include "TranslateUpXML.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/XmlParser.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../DBAccess/DBManager.h"
#include "./SpectrumTask.h"
#include <vector>
#include <iostream>
#include "ace/Task.h"

using namespace std;

SpectrumSetTask::SpectrumSetTask() : DeviceIndependentTask()
{

}

SpectrumSetTask::SpectrumSetTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

SpectrumSetTask::~SpectrumSetTask()
{

}

void SpectrumSetTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]设置频谱任务执行 !\n",DeviceID));
	bRun = true;
	SetRunning();

	XmlParser parser;
	std::string action;
	parser.Set_xml(strStandardXML);
	//解析任务，判断是设置还是删除
	pXMLNODE node=parser.GetNodeFromPath("Msg/SpectrumTaskSet");
	parser.GetAttrNode(node,"TaskID",TaskID);
	parser.GetAttrNode(node,"Action",action);
	if (action=="Set")//任务设置
	{
		std::vector<std::string> strVecTask;
		TranslateDownXML::TranslateSpectrumTask(strStandardXML,strVecTask);
		for (size_t i=0; i!= strVecTask.size();++i)
		{
			SpectrumTask* spectrumtask = new SpectrumTask(strVecTask[i]);//新的指标任务
			//添加新任务
			if( BUSINESSLAYOUTMGR::instance()->AddTask(spectrumtask) == true )
			{
				RetValue = RUN_SUCCESS;
			}
			else
			{
				RetValue = RUN_FAILED;
			}
		}
	}
	else if (action=="Del")//任务删除
	{
		if( BUSINESSLAYOUTMGR::instance()->DelTask(TaskID) == true )
			RetValue = RUN_SUCCESS;
		else
			RetValue = RUN_FAILED;
	}

	SendXML(TranslateUpXML::TranslateSpectrumSet(this));

	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]设置频谱任务停止 !\n",DeviceID));
}
string SpectrumSetTask::GetTaskName()
{
	return "设置频谱任务";
}
std::string SpectrumSetTask::GetObjectName()
{
	return std::string("SpectrumSetTask");
}
