///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：TranslateUpXML.cpp
// 创建者：jiangcheng
// 创建时间：2009-06-04
// 内容描述：将原子任务执行结果的XML转化为应用系统所需的XML
///////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996)
#include "TranslateUpXML.h"
#include "TranslateUpXMLForCTTB.h"
#include "TranslateUpXMLForDVBC.h"
#include "TranslateUpXMLForRADIO.h"
#include "TranslateUpXMLForATV.h"
#include "TranslateUpXMLForAM.h"
#include "TranslateUpXMLForDVBS.h"
#include "TranslateUpXMLForCTV.h"
#include "TranslateUpXMLForTHREED.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/PropManager.h"
#include "../Task//StreamRealtimeQueryTask.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Foundation/PropManager.h"
#include "../DBAccess/DBManager.h"
//返回实时视频XML
std::string TranslateUpXML::TranslateStreamRealtimeQuery(const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateStreamRealtimeQuery(task);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateStreamRealtimeQuery(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateStreamRealtimeQuery(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateStreamRealtimeQuery(task);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateStreamRealtimeQuery(task);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateStreamRealtimeQuery(task);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateStreamRealtimeQuery(task);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateStreamRealtimeQuery(task);
		}
	default:
		return "";
	}
	return "";
}
//返回轮播查询XML
std::string TranslateUpXML::TranslateStreamRoundQuery(const XMLTask* task,std::string deviceID)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateStreamRoundQuery(task,deviceID);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateStreamRoundQuery(task,deviceID);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateStreamRoundQuery(task,deviceID);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateStreamRoundQuery(task,deviceID);
		}
	case ATV:
		{

			return TranslateUpXMLForATV::TranslateStreamRound(task,deviceID);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateStreamRound(task,deviceID);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateStreamRound(task,deviceID);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateStreamRound(task,deviceID);
		}
	default:
		return "";
	}
	return "";
}
//返回频道扫描XML
std::string TranslateUpXML::TranslateChannelScan(const XMLTask* task,const std::string& strRtnXML)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateChannelScan(task,strRtnXML);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateChannelScan(task,strRtnXML);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateChannelScan(task,strRtnXML);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateChannelScan(task,strRtnXML);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateChannelScanQuery(task,strRtnXML);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateChannelScanQuery(task,strRtnXML);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateChannelScanQuery(task,strRtnXML);
		}

	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateChannelScanQuery(task,strRtnXML);
		}
	default:
		return "";
	}
	return "";
}
//返回频道设置xml
std::string TranslateUpXML::TranslateChannelSet( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateChannelSet(task);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateChannelSet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateChannelSet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateChannelSet(task);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateChannelSet(task);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateChannelSet(task);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateChannelSet(task);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateChannelSet(task);
		}
	default:
		return "";
	}
	return "";
}

//返回视频用户查询XML
std::string TranslateUpXML::TranslateClientInfoQuery(const XMLTask* task,std::vector<sVedioUserInfo>& vecClient)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateClientInfoQuery(task,vecClient);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateClientInfoQuery(task,vecClient);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateClientInfoQuery(task,vecClient);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateClientInfoQuery(task,vecClient);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateClientInfoQuery(task,vecClient);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateClientInfoQuery(task,vecClient);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateClientInfoQuery(task,vecClient);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateClientInfoQuery(task,vecClient);
		}
	default:
		return "";
	}
	return "";
}
//返回表查询XML
std::string TranslateUpXML::TranslateTableQuery(const XMLTask *task, std::string strFileName)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateTableQuery(task,strFileName);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateTableQuery(task,strFileName);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateTableQuery(task,strFileName);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateTableQuery(task,strFileName);
		}
	default:
		return "";
	}
	return "";

}
//返回录像设置XML
std::string TranslateUpXML::TranslateRecord(const XMLTask* task,const std::vector<XMLTask*>& resultVec)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateRecord(task,resultVec);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateRecord(task,resultVec);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateRecord(task,resultVec);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateRecord(task,resultVec);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateRecord(task,resultVec);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateRecord(task,resultVec);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateRecord(task,resultVec);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateRecord(task,resultVec);
		}
	default:
		return "";
	}
	return "";
}

//返回偱切录像设置XML
std::string TranslateUpXML::TranslateRoundRecord(const XMLTask* task,const std::vector<XMLTask*>& resultVec)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateRoundRecord(task,resultVec);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateRoundRecord(task,resultVec);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateRoundRecord(task,resultVec);
		}
	default:
		return "";
	}
	return "";
}

//返回指标任务设置xml
std::string TranslateUpXML::TranslateQualitySet( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateQualitySet(task);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateQualitySet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateQualitySet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateQualitySet(task);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateQualitySet(task);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateQualitySet(task);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateQualitySet(task);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateQualitySet(task);
		}
	default:
		return "";
	}
	return "";
}
//返回指标查询XML
std::string TranslateUpXML::TranslateQualityQuery( const XMLTask* task,std::map< string,vector<eQualityInfo> > FreqQualityInfo)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateQualityQuery(task,FreqQualityInfo);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateQualityQuery(task,FreqQualityInfo);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateQualityQuery(task,FreqQualityInfo);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateQualityQuery(task,FreqQualityInfo);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateQualityQuery(task,FreqQualityInfo);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateQualityQuery(task,FreqQualityInfo);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateQualityQuery(task,FreqQualityInfo);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateQualityQuery(task,FreqQualityInfo);
		}
	default:
		return "";
	}
	return "";
}
//返回报警参数设置XML
std::string TranslateUpXML::TranslateAlarmParamSet( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateAlarmParamSet(task);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateAlarmParamSet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateAlarmParamSet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateAlarmParamSet(task);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateAlarmParamSet(task);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateAlarmParamSet(task);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateAlarmParamSet(task);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateAlarmParamSet(task);
		}
	default:
		return "";
	}
	return "";
}
//返回通道状态查询XML
std::string TranslateUpXML::TranslateDeviceStatusQuery(const XMLTask* task,std::vector<sDeviceStatus> & VecDeviceStatus)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateDeviceStatusQuery(task,VecDeviceStatus);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateDeviceStatusQuery(task,VecDeviceStatus);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateDeviceStatusQuery(task,VecDeviceStatus);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateDeviceStatusQuery(task,VecDeviceStatus);
		}
	default:
		return "";
	}
	return "";
}

//返回运行图设置XML
std::string TranslateUpXML::TranslateRunPlanSet(const XMLTask* task)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateRunPlanSet(task);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateRunPlanSet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateRunPlanSet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateRunPlanSet(task);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateRunPlanSet(task);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateRunPlanSet(task);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateRunPlanSet(task);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateRunPlanSet(task);
		}
	default:
		return "";
	}
	return "";
}

//返回自动录像音视频查询XML
std::string TranslateUpXML::TranslateAutoRecordStream( const XMLTask* task,std::string URL,std::string startDateTime,std::string endDateTime )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateAutoRecordStream(task,URL,startDateTime,endDateTime);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateAutoRecordStream(task,URL,startDateTime,endDateTime);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateAutoRecordStream(task,URL,startDateTime,endDateTime);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateAutoRecordStream(task,URL,startDateTime,endDateTime);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateAutoRecordStream(task,URL,startDateTime,endDateTime);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateAutoRecordStream(task,URL,startDateTime,endDateTime);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateAutoRecordStream(task,URL,startDateTime,endDateTime);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateAutoRecordStream(task,URL,startDateTime,endDateTime);
		}
	default:
		return "";
	}
	return "";
}

//返回自动录像节目文件下载XML
std::string TranslateUpXML::TranslateAutoRecordFile( const XMLTask* task,std::string URL,std::string startDateTime,std::string endDateTime)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateAutoRecordFile(task,URL,startDateTime,endDateTime);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateAutoRecordFile(task,URL,startDateTime,endDateTime);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateAutoRecordFile(task,URL,startDateTime,endDateTime);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateAutoRecordFile(task,URL,startDateTime,endDateTime);
		}
	default:
		return "";
	}
	return "";
}

//返回任务录像音视频查询XML
std::string TranslateUpXML::TranslateTaskRecordStream( const XMLTask* task,std::string URL,std::string startDateTime,std::string endDateTime )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateTaskRecordStream(task,URL,startDateTime,endDateTime);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateTaskRecordStream(task,URL,startDateTime,endDateTime);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateTaskRecordStream(task,URL,startDateTime,endDateTime);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateTaskRecordStream(task,URL,startDateTime,endDateTime);
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateTaskRecordStream(task,URL,startDateTime,endDateTime);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateTaskRecordStream(task,URL,startDateTime,endDateTime);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateTaskRecordStream(task,URL,startDateTime,endDateTime);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateTaskRecordStream(task,URL,startDateTime,endDateTime);
		}
	default:
		return "";
	}
	return "";
}

//返回任务录像节目文件下载XML
std::string TranslateUpXML::TranslateTaskRecordFile( const XMLTask* task,std::string URL,std::string startDateTime,std::string endDateTime )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateTaskRecordFile(task,URL,startDateTime, endDateTime);
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateTaskRecordFile(task,URL, startDateTime,endDateTime);
			break;
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateTaskRecordFile(task,URL, startDateTime,endDateTime);
			break;
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateTaskRecordFile(task,URL, startDateTime,endDateTime);
			break;
		}
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateTaskRecordFile(task,URL,startDateTime,endDateTime);
			break;
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateTaskRecordFile(task,URL,startDateTime,endDateTime);
			break;
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateTaskRecordFile(task,URL,startDateTime,endDateTime);
			break;
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateTaskRecordFile(task,URL,startDateTime,endDateTime);
			break;
		}
	default:
		return "";
	}
	return "";
}
//EPG查询XML
std::string TranslateUpXML::TranslateEPGQuery(const XMLTask*task,std::string URL)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateEPGQuery(task,URL);
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateEPGQuery(task,URL);
			break;
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateEPGQuery(task,URL);
			break;
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateEPGQuery(task,URL);
			break;
		}

	default:
		return "";
	}
	return "";
}

std::string TranslateUpXML::TranslateAlarmReport( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateAlarmReport(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateAlarmReport(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateAlarmReport(task);
		}
	default:
		return "";
	}
	return "";
}

//wz_110107
std::string TranslateUpXML::TranslateCleanAlarmStateTask(const XMLTask* task)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateCleanAlarmStateTask(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateCleanAlarmStateTask(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateCleanAlarmStateTask(task);
		}
	default:
		return "";
	}
	return "";
}

//wz_110107
std::string TranslateUpXML::TranslateMatrixQuery( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateMatrixQuery(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateMatrixQuery(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateMatrixQuery(task);
		}
	default:
		return "";
	}
	return "";
}

//wz_110107
std::string TranslateUpXML::TranslateChangeQAMQuery( const XMLTask* task )
{
 if (task == NULL)
	 return "";
 switch(task->GetDVBType())
 {
 case CTTB:
	 {
		 break;
	 }
 case DVBC:
	 {
		 return TranslateUpXMLForDVBC::TranslateChangeQAMQuery(task);
	 }
 case DVBS:
	 {
		 return TranslateUpXMLForDVBS::TranslateChangeQAMQuery(task);
	 }
 case THREED:
	 {
		 return TranslateUpXMLForTHREED::TranslateChangeQAMQuery(task);
	 }
 default:
	 return "";
 }
 return "";
}

//wz_110107
std::string TranslateUpXML::TranslateLoopAlarmInfo( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateLoopAlarmInfo(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateLoopAlarmInfo(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateLoopAlarmInfo(task);
		}
	default:
		return "";
	}
	return "";
}

//wz_110107
std::string TranslateUpXML::TranslateRecordParamSet( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateRecordParamSet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateRecordParamSet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateRecordParamSet(task);
		}
	default:
		return "";
	}
	return "";
}

std::string TranslateUpXML::TranslateAlarmQuery( const XMLTask* task,std::vector<sCheckParam> vecAlarmResult )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateAlarmQuery(task,vecAlarmResult);
			break;
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateAlarmQuery(task,vecAlarmResult);
			break;
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateAlarmQuery(task,vecAlarmResult);
			break;
		}
	default:
		return "";
	}
	return "";
}
//实时指标XML
std::string TranslateUpXML::TranslateQualityRealtimeQuery(const XMLTask* task,std::string strXml)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateQualityRealtimeQuery(task,strXml);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateQualityRealtimeQuery(task,strXml);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateQualityRealtimeQuery(task,strXml);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateQualityRealtimeQuery(task,strXml);
		}
	default:
		return "";
	}
	return "";
}
//运行环境XML
std::string TranslateUpXML::TranslateEnvironmentQuery(const XMLTask* task,EnvMapInfo& EnvMap,std::vector<sCheckParam>& paramVec)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateEnvironmentQuery(task, EnvMap, paramVec);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateEnvironmentQuery(task, EnvMap, paramVec);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateEnvironmentQuery(task, EnvMap, paramVec);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateEnvironmentQuery(task, EnvMap, paramVec);
		}
	default:
		return "";
	}
	return "";
}
//MHP查询XML
std::string TranslateUpXML::TranslateMHPQuery(const XMLTask *task, std::string ftp, std::string userName, std::string passWord)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateMHPQuery(task, ftp,userName,passWord);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateMHPQuery(task, ftp,userName,passWord);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateMHPQuery(task, ftp,userName,passWord);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateMHPQuery(task, ftp,userName,passWord);
		}
	default:
		return "";
	}
	return "";
}
//指标补偿设置XML
std::string TranslateUpXML::TranslateQualityCompensationSet(const XMLTask *task)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateQualityCompensationSet(task);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateQualityCompensationSet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateQualityCompensationSet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateQualityCompensationSet(task);
		}
	default:
		return "";
	}
	return "";
}
//手动录像主动上报
std::string TranslateUpXML::TranslateManualRecordReport(const XMLTask *task, const std::string& strStandXML,std::string URL )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateManualRecordReport(task,strStandXML,URL);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateManualRecordReport(task,strStandXML,URL);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateManualRecordReport(task,strStandXML,URL);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateManualRecordReport(task,strStandXML,URL);
		}
	default:
		return "";
	}
	return "";
}
//数据业务分析时间设置XML
std::string TranslateUpXML::TranslateAutoAnalysisTimeSet(const XMLTask* task)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateAutoAnalysisTimeSet(task);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateAutoAnalysisTimeSet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateAutoAnalysisTimeSet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateAutoAnalysisTimeSet(task);
		}
	default:
		return "";
	}
	return "";
}

std::string TranslateUpXML::TranslateManualRecordSet( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			return TranslateUpXMLForCTTB::TranslateManualRecordSet(task);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateManualRecordSet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateManualRecordSet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateManualRecordSet(task);
		}
	default:
		return "";
	}
	return "";
}

std::string TranslateUpXML::TranslateOSDSet( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateOSDSet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateOSDSet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateOSDSet(task);
		}

	default:
		return "";
	}
	return "";
}

std::string TranslateUpXML::TranslateStreamRealtimeRate(const XMLTask* task)
{
	if(task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateStreamRealtimeRate(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateStreamRealtimeRate(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateStreamRealtimeRate(task);
		}
	default:
		return "";
	}
	return "";
}
std::string TranslateUpXML::TranslateRecordCapabilityQuery(const XMLTask* task)
{
	if(task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateRecordCapabilityQuery(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateRecordCapabilityQuery(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateRecordCapabilityQuery(task);
		}
	default:
		return "";
	}
	return "";
}
std::string TranslateUpXML::TranslateHDDefAudioParamSet(const XMLTask* task)
{
	if(task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateHDDefAudioParamSet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateHDDefAudioParamSet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateHDDefAudioParamSet(task);
		}
	default:
		return "";
	}
	return "";
}
std::string TranslateUpXML::TranslateAudioParamSet(const XMLTask* task)
{
	if(task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case CTTB:
		{
			break;
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateAudioParamSet(task);
		}
	case DVBS:
		{
			return TranslateUpXMLForDVBS::TranslateAudioParamSet(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateAudioParamSet(task);
		}
	default:
		return "";
	}
	return "";
}

//返回频谱任务设置xml
std::string TranslateUpXML::TranslateSpectrumSet( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateSpectrumSet(task);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateSpectrumSet(task);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateSpectrumSet(task);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateSpectrumSet(task);
		}
	default:
		return "";
	}
	return "";
}
std::string TranslateUpXML::TranslateSpectrumQuery( const XMLTask* task,std::vector<eSpecInfo> vecSpecInfo)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateSpectrumQuery(task,vecSpecInfo);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateSpectrumQuery(task,vecSpecInfo);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateSpectrumQuery(task,vecSpecInfo);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateSpectrumQuery(task,vecSpecInfo);
		}
	default:
		return "";
	}
	return "";
}

std::string TranslateUpXML::TranslateStopClient( const XMLTask* task )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateStopClient(task);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateStopClient(task);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateStopClient(task);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateStopClient(task);
		}
	case DVBC:
		{
			return TranslateUpXMLForDVBC::TranslateStopClient(task);
		}
	case THREED:
		{
			return TranslateUpXMLForTHREED::TranslateStopClient(task);
		}
	default:
		return "";
	}
	return "";
}

std::string TranslateUpXML::TranslateTaskSchedulerQuery(const  XMLTask* task,std::vector<sTaskInfo> taskinfovec )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateTaskSchedulerQuery(task,taskinfovec);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateTaskSchedulerQuery(task,taskinfovec);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateTaskSchedulerQuery(task,taskinfovec);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateTaskSchedulerQuery(task,taskinfovec);
		}
	default:
		return "";
	}
	return "";
}

std::string TranslateUpXML::TranslateManualTaskSchedulerQuery( const XMLTask* task,std::vector<sTaskInfo> taskinfovec)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
		{
			return TranslateUpXMLForATV::TranslateManualTaskSchedulerQuery(task,taskinfovec);
		}
	case RADIO:
		{
			return TranslateUpXMLForRADIO::TranslateManualTaskSchedulerQuery(task,taskinfovec);
		}
	case AM:
		{
			return TranslateUpXMLForAM::TranslateManualTaskSchedulerQuery(task,taskinfovec);
		}
	case CTV:
		{
			return TranslateUpXMLForCTV::TranslateManualTaskSchedulerQuery(task,taskinfovec);
		}
	default:
		return "";
	}
	return "";
}


std::string TranslateUpXML::TranslateChannelCheck(const XMLTask* task, const std::vector<sChannelInfo>& vecChanInfo )
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
			return TranslateUpXMLForATV::TranslateChannelCheck(task,vecChanInfo);
	case RADIO:
			return TranslateUpXMLForRADIO::TranslateChannelCheck(task,vecChanInfo);
	default:
		return "";
	}
	return "";
}

std::string TranslateUpXML::TranslateRecordCheck(const XMLTask* task, const std::map<int,sTaskInfo>& mapTaskInfo)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
			return TranslateUpXMLForATV::TranslateRecordCheck(task, mapTaskInfo);
	case RADIO:
			return TranslateUpXMLForRADIO::TranslateRecordCheck(task, mapTaskInfo);
	default:
		return "";
	}
	return "";
}


std::string TranslateUpXML::TranslateAlarmParamCheck(const XMLTask* task, const CodeAlarmParamMap& mapAlarmParam)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
		return TranslateUpXMLForATV::TranslateAlarmParamCheck(task, mapAlarmParam);
	case RADIO:
		return TranslateUpXMLForRADIO::TranslateAlarmParamCheck(task, mapAlarmParam);
	default:
		return "";
	}
	return "";
}


std::string TranslateUpXML::TranslateRunPlanCheck(const XMLTask* task, std::string equCode, const RunPlanParamMap& mapRunplanParam)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
		return TranslateUpXMLForATV::TranslateRunPlanCheck(task, equCode, mapRunplanParam);
	case RADIO:
		return TranslateUpXMLForRADIO::TranslateRunPlanCheck(task, equCode, mapRunplanParam);
	default:
		return "";
	}
	return "";
}


std::string TranslateUpXML::TranslateAlarmExpireTask(const XMLTask* task)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
		return TranslateUpXMLForATV::TranslateAlarmExpireTask(task);
	case RADIO:
		return TranslateUpXMLForRADIO::TranslateAlarmExpireTask(task);
	default:
		return "";
	}
	return "";
}

std::string TranslateUpXML::TranslateDeviceInfoQuery(const XMLTask* task)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
		return TranslateUpXMLForATV::TranslateDeviceInfoQuery(task);
	default:
		return "";
	}
	return "";
}

 std::string TranslateUpXML::TranslateServerStatusSet(const XMLTask* task)
{
	if (task == NULL)
		return "";
	switch(task->GetDVBType())
	{
	case ATV:
		return TranslateUpXMLForATV::TranslateServerStatusSet(task);
	default:
		return "";
	}
	return "";
}

 std::string TranslateUpXML::TranslateTaskRecordFile(  const XMLTask* task,std::vector<sRecordFileInfo> FileInfo)
 {
	 if (task == NULL)
		 return "";
	 switch(task->GetDVBType())
	 { 
	 case ATV:
		 {
			 return TranslateUpXMLForATV::TranslateTaskRecordFile(task,FileInfo);
			 break;
		 }
	 case RADIO:
		 {
			 return TranslateUpXMLForRADIO::TranslateTaskRecordFile(task,FileInfo);
			 break;
		 }
	 default:
		 return "";
	 }
	 return "";
 }



