
#include "MHPQueryTask.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/XmlParser.h"
#include <vector>
#include <iostream>
#include "ace/Task.h"
#include "../DBAccess/DBManager.h"
#include "../Foundation/TimeUtil.h"
using namespace std;

MHPQueryTask::MHPQueryTask() : DeviceIndependentTask()
{

}

MHPQueryTask::MHPQueryTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

MHPQueryTask::~MHPQueryTask()
{

}
void MHPQueryTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]MHP��ѯ����ִ�� !\n",DeviceID));
	bRun = true;
	SetRunning();
	XmlParser parser;
	parser.Set_xml(strStandardXML);
	SetRetValue(RUN_SUCCESS);
	std::string ftp,userName,passWord;
	SendXML(TranslateUpXML::TranslateMHPQuery(this,ftp,userName,passWord));
	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]MHP��ѯ����ֹͣ !\n",DeviceID));
}
string MHPQueryTask::GetTaskName()
{
	return "MHP��ѯ����";
}
std::string MHPQueryTask::GetObjectName()
{
	return std::string("MHPQueryTask");
}
