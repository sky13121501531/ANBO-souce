///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：TaskFactory.cpp
// 创建者：jiangcheng
// 创建时间：2009-05-25
// 内容描述：任务工厂基类
///////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996)
#include "TaskFactory.h"
#include "XMLTask.h"
#include "./StreamRealtimeQueryTask.h"
#include "./ChannelScanQueryTask.h"
#include "./RecordSetTask.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/XmlParser.h"
#include "./RecordQueryTask.h"
#include "./StreamRealtimeRoundQueryTask.h"
#include "./ClientInfoQueryTask.h"
#include "./QualitySetTask.h"
#include "./QualityRealtimeQueryTask.h"
#include "./ChannelSetTask.h"
#include "./AlarmParamSetTask.h"
#include "./DeviceStatusQueryTask.h"
#include "./RunPlanSetTask.h"
#include "./QualityQueryTask.h"
#include "./RecordTask.h"
#include "./RoundRecordSetTask.h"
#include "./RoundRecordTask.h"
#include "./QualityTask.h"
#include "./TableQueryTask.h"
#include "./EnvironmentQueryTask.h"
#include "./AutoAnalysisTimeSetTask.h"
#include "./QualityCompensationSetTask.h"
#include "./SpectrumScanTask.h"
#include "./ManualRecordSetTask.h"
#include "./EPGQueryTask.h"
#include "./MHPQueryTask.h"
#include "./AlarmParamSetTask.h"
#include "./SMSICCardQueryTask.h"
#include "./SMSCAProductQuery.h"
#include "./SMSEntitlementQuery.h"
#include "./SMSProdPurchaseQueryTask.h"
#include "./CASEntitlementQueryTask.h"
#include "./CASICCardQueryTask.h"
#include "./CASProductListQueryTask.h"
#include "./CASStaticsQueryTask.h"
#include "./OSDSetTask.h"
#include "./AudioParamSetTask.h"
#include "./RecordCapabilityQueryTask.h"
#include "./HDDefAudioParamSetTask.h"
#include "./StreamRealtimeRateTask.h"
#include "./AlarmReportSetTask.h"
#include "./ATVChannelScanQueryTask.h"
#include "./SpectrumSetTask.h"
#include "./SpectrumTask.h"
#include "./SpectrumQueryTask.h"
#include "./StopClientTask.h"
#include "./ManualTaskSchedulerQuery.h"
#include "./TaskSchedulerQuery.h"
#include "./CleanAlarmStateTask.h"
#include "./MatrixQueryTask.h"
#include "./ChangeQAMQueryTask.h"
#include "./LoopAlarmInfoTask.h"
#include "./RecordParamSetTask.h"
#include "./AlarmQueryTask.h"
#include "./DeviceManageTask.h"
#include "./InfoCheckRequestTask.h"
#include "./RunPlanCheckTask.h"
#include "./AlarmExpireTask.h"
#include "./DeviceInfoQuery.h"
#include "./ServerStatusSetTask.h"
#include "./MultiVideoQueryTask.h"
#include "../Communications/SysMsgSender.h"

TaskFactory::TaskFactory()
{

}

TaskFactory::~TaskFactory()
{

}

XMLTask* TaskFactory::CreateTask(std::string strStandardXML)
{
	XMLTask *pXMLTask = NULL;
	string TaskName;

	if (strStandardXML.empty()) return NULL;
	ValidateXML(strStandardXML);
	std::string Type;
	std::string DVBType;
	try
	{		
		XmlParser pXP((const char *)strStandardXML.c_str());
		
		std::string RootName = pXP.GetRootNodeName();//根节点名字
		pXP.GetAttrNode(pXP.GetRootNode(),"Type",Type);
		pXP.GetAttrNode(pXP.GetRootNode(),"DVBType",DVBType);
		if (RootName == "msg" || RootName == "Msg")
		{
			pXP.GetAttrNode( pXP.GetRootNode(),"TaskType",TaskName);
		}
		else 
		{
			string msg = string("XML文件格式错误，无法创建任务对象。XML内容如下：") + strStandardXML;
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
			return NULL;
		}
		if (Type!=""&&Type=="SMSDown")
		{
			pXMLNODE childNode=pXP.GetNodeFirstChild(pXP.GetRootNode());
			std::string name=pXP.GetNodeName(childNode);
			if (name=="ICCardQuery")
			{
				pXMLTask=new SMSICCardQueryTask(strStandardXML);
			}
			else if (name=="CAProductQuery")
			{
				pXMLTask=new SMSCAProductQuery(strStandardXML);
			}
			else if (name=="EntitlementQuery")
			{
				pXMLTask=new SMSEntitlementQuery(strStandardXML);
			}
			else if (name=="ProdPurchaseQuery")
			{
				pXMLTask=new SMSProdPurchaseQueryTask(strStandardXML);
			}
			return pXMLTask;
		}
		else if (Type!=""&&Type=="CASDown")
		{
			pXMLNODE childNode=pXP.GetNodeFirstChild(pXP.GetRootNode());
			std::string name=pXP.GetNodeName(childNode);
			if (name=="ICCardQuery")
			{
				pXMLTask=new CASICCardQueryTask(strStandardXML);
			}
			else if (name=="ProductListQuery")
			{
				pXMLTask=new CASProductListQueryTask(strStandardXML);
			}
			else if (name=="EntitlementQuery")
			{
				pXMLTask=new CASEntitlementQueryTask(strStandardXML);
			}
			else if (name=="StaticsQuery")
			{
				pXMLTask=new CASStaticsQueryTask(strStandardXML);
			}
			return pXMLTask;
		}
	}
	catch(...)
	{
		string msg = string("XML文件格式错误，无法创建任务对象。XML内容如下：") + strStandardXML;
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

		return NULL;
	}

	if (!TaskName.empty() && TaskName.size() > 0)
	{
		if (TaskName == "StreamRealtimeQuery")								//实时视频
			pXMLTask = new StreamRealtimeQueryTask(strStandardXML);
		else if (TaskName == "StreamRoundQuery")							//视频轮播
			pXMLTask = new StreamRealtimeRoundQueryTask(strStandardXML);
		else if (TaskName == "ChannelScanQuery"&&(DVBType=="DVBC"||DVBType=="CTTB"||DVBType=="DVBS"||DVBType=="THREED"))							//频道扫描
			pXMLTask = new ChannelScanQueryTask(strStandardXML);
		else if (TaskName == "ChannelScanQuery"&&(DVBType=="ATV"||DVBType=="RADIO"||DVBType=="AM"||DVBType=="CTV"))				//频道扫描
			pXMLTask = new ATVChannelScanQueryTask(strStandardXML);
		else if (TaskName == "ChannelSet")									//频道设置
			pXMLTask = new ChannelSetTask(strStandardXML);
		else if(TaskName == "RecordTaskSet")								//设置任务\自动录像
			pXMLTask = new RecordSetTask(strStandardXML);	
		else if(TaskName == "RecordTask")									//设置录像任务
			pXMLTask = new RecordTask(strStandardXML);
		else if (TaskName == "RecordQuery")									//自动\任务录像查询\下载任务
			pXMLTask = new RecordQueryTask(strStandardXML);	
		else if (TaskName == "RoundRecordSet")								//偱切录像设置任务
			pXMLTask = new RoundRecordSetTask(strStandardXML);
		else if (TaskName == "RoundRecord")									//偱切录像任务
			pXMLTask = new RoundRecordTask(strStandardXML);
		else if (TaskName == "ClientInfoQuery")								//视频用户查询
			pXMLTask=new ClientInfoQueryTask(strStandardXML);
		else if (TaskName == "QualityRealtimeQuery")				        //指标实时查询
			pXMLTask = new QualityRealtimeQueryTask(strStandardXML);
		else if (TaskName == "QualityTaskSet")								//指标任务设置
			pXMLTask=new QualitySetTask(strStandardXML);
		else if(TaskName == "QualityTask")						    		//指标任务
			pXMLTask = new QualityTask(strStandardXML);
		else if (TaskName == "QualityQuery")								//指标结果查询
			pXMLTask = new QualityQueryTask(strStandardXML);
		else if (TaskName == "AlarmParamSet")								//报警参数设置
			pXMLTask=new AlarmParamSetTask(strStandardXML);
		else if (TaskName == "DeviceStatusQuery")			                //通道状态查询
			pXMLTask=new DeviceStatusQueryTask(strStandardXML);
		else if (TaskName == "RunPlanSet")									//运行图设置
		    pXMLTask = new RunPlanSetTask(strStandardXML);
		else if (TaskName == "TableQuery")									//表查询
			pXMLTask = new TableQueryTask(strStandardXML); 
		else if (TaskName == "EnvironmentQuery")							//运行环境指标查询
			pXMLTask = new EnvironmentQueryTask(strStandardXML);            
		else if (TaskName == "AutoAnalysisTimeSet")							//数据业务分析时间设置
		    pXMLTask = new AutoAnalysisTimeSetTask(strStandardXML);         
		else if (TaskName == "QualityCompensationSet")						//指标补偿设置
		    pXMLTask = new QualityCompensationSetTask(strStandardXML);     
		else if (TaskName == "SpectrumScanQuery")							//频谱扫描
	        pXMLTask = new SpectrumScanTask(strStandardXML);                
		else if (TaskName == "ManualRecordSet")								//手动录制设置
		    pXMLTask = new ManualRecordSetTask(strStandardXML);             
		else if (TaskName == "EPGQuery")									//EPG电子节目单
		    pXMLTask = new EPGQuery(strStandardXML);                        
		else if (TaskName == "MHPQuery")									//MHP查询
		    pXMLTask = new MHPQueryTask(strStandardXML); 
		else if(TaskName == "AlarmThresholdSet")                            //报警门限
			pXMLTask=new AlarmParamSetTask(strStandardXML);
		else if(TaskName == "AlarmTypeSet")                            //报警方式
			pXMLTask=new AlarmReportSetTask(strStandardXML);
		else if (TaskName=="OSDSet")
			pXMLTask=new OSDSetTask(strStandardXML);
		else if(TaskName == "NVRSteamRateSet")
			pXMLTask = new StreamRealtimeRateTask(strStandardXML);
		else if(TaskName == "RecordCapabilityQuery")
			pXMLTask = new RecordCapabilityQueryTask(strStandardXML);
		else if(TaskName == "HDDefAudioParamSet")
			pXMLTask =new HDDefAudioParamSetTask(strStandardXML);
		else if(TaskName == "AudioParamSet")
			pXMLTask = new AudioParamSetTask(strStandardXML);
		else if(TaskName == "SpectrumTaskSet")
			pXMLTask = new SpectrumSetTask(strStandardXML);
		else if(TaskName == "SpectrumTask")
			pXMLTask = new SpectrumTask(strStandardXML);
		else if(TaskName == "SpectrumQuery")
			pXMLTask = new SpectrumQueryTask(strStandardXML);
		else if(TaskName == "StopClient")
			pXMLTask = new StopClientTask(strStandardXML);
		else if(TaskName == "TaskSchedulerQuery")
			pXMLTask = new TaskSchedulerQuery(strStandardXML);
		else if(TaskName == "ManualTaskSchedulerQuery")
			pXMLTask = new ManualTaskSchedulerQuery(strStandardXML);
		else if(TaskName == "ClearAlarmState")									//报警状态清除任务 
			pXMLTask = new CleanAlarmStateTask(strStandardXML);
		else if(TaskName == "MatrixQuery")										//矩阵切换任务 
			pXMLTask = new MatrixQueryTask(strStandardXML);
		else if(TaskName == "ChangeQAMQuery")									//QAM设置任务 
			pXMLTask = new ChangeQAMQueryTask(strStandardXML);
		else if(TaskName == "LoopAlaInf")										//循切报警设置任务 
			pXMLTask = new LoopAlarmInfoTask(strStandardXML);
		else if(TaskName == "RecordParamSet")									//视频转码录像默认参数设置 
			pXMLTask = new RecordParamSetTask(strStandardXML);
		else if(TaskName == "AlarmQuerySet")									//报警查询 
			pXMLTask = new AlarmQueryTask(strStandardXML);
		else if(TaskName == "RoundRecordTask")
			pXMLTask = new RoundRecordTask(strStandardXML);
		else if(TaskName == "DeviceManage")
			pXMLTask = new DeviceManageTask(strStandardXML);
		else if (TaskName == "InfoCheckRequest")								//配置信息查询任务(新安播)
			pXMLTask = new InfoCheckRequestTask(strStandardXML);
		else if (TaskName == "RunPlanCheckRequest")
			pXMLTask = new RunPlanCheckTask(strStandardXML);
		else if(TaskName == "TaskAndAlarmExpireRequest")
			pXMLTask = new AlarmExpireTask(strStandardXML);
		else if(TaskName == "DeviceInfo")
			pXMLTask = new DeviceInfoQuery(strStandardXML);
		else if(TaskName == "ServerStatusSet")
			pXMLTask = new ServerStatusSetTask(strStandardXML);
		else if(TaskName == "MultiVideoQueryTask")
			pXMLTask = new MultiVideoQueryTask(strStandardXML);
		else
			return NULL;
	}

	return pXMLTask;
}
bool TaskFactory::ValidateXML(string& xml)
{
	size_t pos = xml.find("<!--",0);
	while (pos >=0 && pos < xml.length())
	{
		size_t pos2 = xml.find("-->",pos);
		xml.erase(xml.begin()+pos, xml.begin()+pos2+3);
		pos = xml.find("<!--");
	}
	return true;
}

