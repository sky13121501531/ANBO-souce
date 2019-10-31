#include "TableQueryTask.h"
#include "ace/Task.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "../Foundation/OSFunction.h"
#include "../BusinessProcess/PSISIParser.h"
#include "../BusinessProcess/PSISIMgr.h"

TableQueryTask::TableQueryTask(void)
{
}

TableQueryTask::TableQueryTask( std::string strXML ) : DeviceIndependentTask(strXML)
{
}
TableQueryTask::~TableQueryTask(void)
{
}

void TableQueryTask::Run( void )
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]���ѯ����ִ�� !\n",DeviceID));
	bRun = true;

	SetRunning();
	
	std::string strFileName;
	std::string STD,Freq,SymbolRate;
    XmlParser parser(strStandardXML.c_str());
	pXMLNODE tableNode = parser.GetNodeFromPath("Msg/TableQuery/Table");
    parser.GetAttrNode( tableNode,"STD",STD );						//��ȡSTD����ֵ
	parser.GetAttrNode( tableNode,"Freq",Freq );					//��ȡFreq����ֵ
	parser.GetAttrNode( tableNode,"SymbolRate",SymbolRate );		//��ȡSymbolRate����ֵ

	PSISIMGR::instance()->Init(DVBType,GetObjectName());
	PSISIMGR::instance()->CreateTableXML(DVBType);

	//��ȡTABLE��·��
	OSFunction::GetFileName(DVBType,GetObjectName(),strFileName);
	RetValue = RUN_SUCCESS;

	SendXML(TranslateUpXML::TranslateTableQuery(this,strFileName)); //���͵�ǰ��
	
	SetFinised();

	bRun = false;
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]���ѯ����ֹͣ !\n",DeviceID));
}
bool TableQueryTask::TableQuery(const std::string STD, const std::string Freq, const std::string SymbolRate, std::string &strFileName)
{
	strFileName = "Table.rar";
	return true;
}

std::string TableQueryTask::GetObjectName()
{
	return"TableQueryTask";
}

std::string TableQueryTask::GetTaskName()
{
	return "���ѯ����";
}