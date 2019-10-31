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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]表查询任务执行 !\n",DeviceID));
	bRun = true;

	SetRunning();
	
	std::string strFileName;
	std::string STD,Freq,SymbolRate;
    XmlParser parser(strStandardXML.c_str());
	pXMLNODE tableNode = parser.GetNodeFromPath("Msg/TableQuery/Table");
    parser.GetAttrNode( tableNode,"STD",STD );						//获取STD属性值
	parser.GetAttrNode( tableNode,"Freq",Freq );					//获取Freq属性值
	parser.GetAttrNode( tableNode,"SymbolRate",SymbolRate );		//获取SymbolRate属性值

	PSISIMGR::instance()->Init(DVBType,GetObjectName());
	PSISIMGR::instance()->CreateTableXML(DVBType);

	//获取TABLE的路径
	OSFunction::GetFileName(DVBType,GetObjectName(),strFileName);
	RetValue = RUN_SUCCESS;

	SendXML(TranslateUpXML::TranslateTableQuery(this,strFileName)); //发送到前端
	
	SetFinised();

	bRun = false;
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]表查询任务停止 !\n",DeviceID));
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
	return "表查询任务";
}