///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����TaskFactory.cpp
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-05-25
// �������������񹤳�����
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
		
		std::string RootName = pXP.GetRootNodeName();//���ڵ�����
		pXP.GetAttrNode(pXP.GetRootNode(),"Type",Type);
		pXP.GetAttrNode(pXP.GetRootNode(),"DVBType",DVBType);
		if (RootName == "msg" || RootName == "Msg")
		{
			pXP.GetAttrNode( pXP.GetRootNode(),"TaskType",TaskName);
		}
		else 
		{
			string msg = string("XML�ļ���ʽ�����޷������������XML�������£�") + strStandardXML;
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
		string msg = string("XML�ļ���ʽ�����޷������������XML�������£�") + strStandardXML;
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

		return NULL;
	}

	if (!TaskName.empty() && TaskName.size() > 0)
	{
		if (TaskName == "StreamRealtimeQuery")								//ʵʱ��Ƶ
			pXMLTask = new StreamRealtimeQueryTask(strStandardXML);
		else if (TaskName == "StreamRoundQuery")							//��Ƶ�ֲ�
			pXMLTask = new StreamRealtimeRoundQueryTask(strStandardXML);
		else if (TaskName == "ChannelScanQuery"&&(DVBType=="DVBC"||DVBType=="CTTB"||DVBType=="DVBS"||DVBType=="THREED"))							//Ƶ��ɨ��
			pXMLTask = new ChannelScanQueryTask(strStandardXML);
		else if (TaskName == "ChannelScanQuery"&&(DVBType=="ATV"||DVBType=="RADIO"||DVBType=="AM"||DVBType=="CTV"))				//Ƶ��ɨ��
			pXMLTask = new ATVChannelScanQueryTask(strStandardXML);
		else if (TaskName == "ChannelSet")									//Ƶ������
			pXMLTask = new ChannelSetTask(strStandardXML);
		else if(TaskName == "RecordTaskSet")								//��������\�Զ�¼��
			pXMLTask = new RecordSetTask(strStandardXML);	
		else if(TaskName == "RecordTask")									//����¼������
			pXMLTask = new RecordTask(strStandardXML);
		else if (TaskName == "RecordQuery")									//�Զ�\����¼���ѯ\��������
			pXMLTask = new RecordQueryTask(strStandardXML);	
		else if (TaskName == "RoundRecordSet")								//����¼����������
			pXMLTask = new RoundRecordSetTask(strStandardXML);
		else if (TaskName == "RoundRecord")									//����¼������
			pXMLTask = new RoundRecordTask(strStandardXML);
		else if (TaskName == "ClientInfoQuery")								//��Ƶ�û���ѯ
			pXMLTask=new ClientInfoQueryTask(strStandardXML);
		else if (TaskName == "QualityRealtimeQuery")				        //ָ��ʵʱ��ѯ
			pXMLTask = new QualityRealtimeQueryTask(strStandardXML);
		else if (TaskName == "QualityTaskSet")								//ָ����������
			pXMLTask=new QualitySetTask(strStandardXML);
		else if(TaskName == "QualityTask")						    		//ָ������
			pXMLTask = new QualityTask(strStandardXML);
		else if (TaskName == "QualityQuery")								//ָ������ѯ
			pXMLTask = new QualityQueryTask(strStandardXML);
		else if (TaskName == "AlarmParamSet")								//������������
			pXMLTask=new AlarmParamSetTask(strStandardXML);
		else if (TaskName == "DeviceStatusQuery")			                //ͨ��״̬��ѯ
			pXMLTask=new DeviceStatusQueryTask(strStandardXML);
		else if (TaskName == "RunPlanSet")									//����ͼ����
		    pXMLTask = new RunPlanSetTask(strStandardXML);
		else if (TaskName == "TableQuery")									//���ѯ
			pXMLTask = new TableQueryTask(strStandardXML); 
		else if (TaskName == "EnvironmentQuery")							//���л���ָ���ѯ
			pXMLTask = new EnvironmentQueryTask(strStandardXML);            
		else if (TaskName == "AutoAnalysisTimeSet")							//����ҵ�����ʱ������
		    pXMLTask = new AutoAnalysisTimeSetTask(strStandardXML);         
		else if (TaskName == "QualityCompensationSet")						//ָ�겹������
		    pXMLTask = new QualityCompensationSetTask(strStandardXML);     
		else if (TaskName == "SpectrumScanQuery")							//Ƶ��ɨ��
	        pXMLTask = new SpectrumScanTask(strStandardXML);                
		else if (TaskName == "ManualRecordSet")								//�ֶ�¼������
		    pXMLTask = new ManualRecordSetTask(strStandardXML);             
		else if (TaskName == "EPGQuery")									//EPG���ӽ�Ŀ��
		    pXMLTask = new EPGQuery(strStandardXML);                        
		else if (TaskName == "MHPQuery")									//MHP��ѯ
		    pXMLTask = new MHPQueryTask(strStandardXML); 
		else if(TaskName == "AlarmThresholdSet")                            //��������
			pXMLTask=new AlarmParamSetTask(strStandardXML);
		else if(TaskName == "AlarmTypeSet")                            //������ʽ
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
		else if(TaskName == "ClearAlarmState")									//����״̬������� 
			pXMLTask = new CleanAlarmStateTask(strStandardXML);
		else if(TaskName == "MatrixQuery")										//�����л����� 
			pXMLTask = new MatrixQueryTask(strStandardXML);
		else if(TaskName == "ChangeQAMQuery")									//QAM�������� 
			pXMLTask = new ChangeQAMQueryTask(strStandardXML);
		else if(TaskName == "LoopAlaInf")										//ѭ�б����������� 
			pXMLTask = new LoopAlarmInfoTask(strStandardXML);
		else if(TaskName == "RecordParamSet")									//��Ƶת��¼��Ĭ�ϲ������� 
			pXMLTask = new RecordParamSetTask(strStandardXML);
		else if(TaskName == "AlarmQuerySet")									//������ѯ 
			pXMLTask = new AlarmQueryTask(strStandardXML);
		else if(TaskName == "RoundRecordTask")
			pXMLTask = new RoundRecordTask(strStandardXML);
		else if(TaskName == "DeviceManage")
			pXMLTask = new DeviceManageTask(strStandardXML);
		else if (TaskName == "InfoCheckRequest")								//������Ϣ��ѯ����(�°���)
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

