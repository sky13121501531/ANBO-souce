
#include <string>
#include "../Foundation/PropManager.h"
#include "../Communications/SysMsgSender.h"
#include "MySqlDBConnection.h"
#include "SqlServerDBConnection.h"
#include "./SqliteDBConnection.h"
#include "DBManager.h"

using namespace std;

DBManager::DBManager(void)
{
	string sqltype = PROPMANAGER::instance()->GetDbType();
	if (sqltype == "mysql")//mysql数据库，初始化mysql数据库连接
	{
		DBTask = new MySqlDBConnection();
		DBRecord=new MySqlDBConnection();
		DBChannel=new MySqlDBConnection();
		DBOther=new MySqlDBConnection();
		DBAlarmParam=new MySqlDBConnection();
	}
	else if(sqltype=="mssql")//sqlserver，初始化sqlserver数据库
	{
		DBTask = new SqlServerDBConnection();
		DBRecord=new SqlServerDBConnection();
		DBChannel=new SqlServerDBConnection();
		DBOther=new SqlServerDBConnection();
		DBAlarmParam=new SqlServerDBConnection();
	}
	else if(sqltype=="sqlite")//sqlite，初始化sqlite数据库
	{
		DBTask = new SqliteDBConnection();
		DBRecord=new SqliteDBConnection();
		DBChannel=new SqliteDBConnection();
		DBOther=new SqliteDBConnection();
		DBAlarmParam=new SqliteDBConnection();
	}
	else
	{
		//...
	}
	InitConnect();
}

DBManager::~DBManager(void)
{
	if (DBTask)
	{
		delete DBTask;
		DBTask=NULL;
	}
	if (DBChannel)
	{
		delete DBChannel;
		DBChannel=NULL;
	}
	if (DBRecord)
	{
		delete DBRecord;
		DBRecord=NULL;
	}
	if (DBOther)
	{
		delete DBOther;
		DBOther=NULL;
	}
	if (DBAlarmParam)
	{
		delete DBAlarmParam;
		DBAlarmParam = NULL;
	}
}
bool DBManager::InitConnect(void)
{
	return DBTask->InitConnect()&&DBRecord->InitConnect()&&DBOther->InitConnect()&&DBChannel->InitConnect()&&DBAlarmParam->InitConnect();
}
bool DBManager::DisConnect(void)
{
	DBTask->DisConnect();
	DBChannel->DisConnect();
	DBRecord->DisConnect();
	DBOther->DisConnect();
	DBAlarmParam->DisConnect();
	return true;
}
bool DBManager::RetryConnect( DBConnection* db )
{
	int count=0;
	bool ret=false;
	while(count<3)
	{
		string msg = string("数据库连接断开，尝试重新连接失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
		
		ret=db->InitConnect();
		if (ret)
		{
			msg = string("数据库连接断开，尝试重新连接成功");
			SYSMSGSENDER::instance()->SendMsg(msg);

			break;
		}
		count++;
	}
	return ret;
}
bool DBManager::AddXmlTask(eDVBType type,std::string xml)
{
	if (!DBTask->IsConnected())
	{
		RetryConnect(DBTask);	
	}
	return DBTask->AddXmlTask(type,xml);
}

bool DBManager::DeleteTask( eDVBType type,std::string xml )
{
	if (!DBTask->IsConnected())
	{
		RetryConnect(DBTask);
	}
	return DBTask->DeleteTask(type,xml);
}

bool DBManager::QueryTask( eDVBType type,std::vector<std::string>& vecXml )
{
	if (!DBTask->IsConnected())
	{
		RetryConnect(DBTask);
	}
	return DBTask->QueryTask(type,vecXml);
}

bool DBManager::QueryTask(std::vector<std::string>& vecXml )
{
	if (!DBTask->IsConnected())
	{
		RetryConnect(DBTask);
	}
	return DBTask->QueryTask(vecXml);
}

bool DBManager:: AddChannel(eDVBType type,std::string channelid,std::string xml)
{
	if (!DBChannel->IsConnected())
	{
		RetryConnect(DBChannel);
	}
	return DBChannel->AddChannel(type,channelid,xml);
}

bool DBManager::DeleteChannel(eDVBType type,std::string channelid)
{
	if (!DBChannel->IsConnected())
	{
		RetryConnect(DBChannel);
	}
	return DBChannel->DeleteChannel(type,channelid);
}

bool DBManager::DeleteChannelByType(eDVBType type)
{
	if (!DBChannel->IsConnected())
	{
		RetryConnect(DBChannel);
	}
	return DBChannel->DeleteChannelByType(type);
}

bool DBManager::QueryChannel(eDVBType type,std::string channelid,std::string& xml)
{
	if (!DBChannel->IsConnected())
	{
		RetryConnect(DBChannel);
	}
	return DBChannel->QueryChannel(type,channelid,xml);
}
bool  DBManager::QueryTypeChannel(eDVBType type,std::vector<std::string>&vecXml)
{
	if (!DBChannel->IsConnected())
	{
		RetryConnect(DBChannel);
	}
	return DBChannel->QueryTypeChannel(type,vecXml);
}
bool DBManager::QueryAllChannel( std::vector<std::string>& vecXml )
{
	if (!DBChannel->IsConnected())
	{
		RetryConnect(DBChannel);
	}
	return DBChannel->QueryAllChannel(vecXml);
}

bool DBManager::AddRecord( sRecordInfo record )
{
	if (!DBRecord->IsConnected())
	{
		RetryConnect(DBRecord);			
	}
	return DBRecord->AddRecord(record);
}

bool DBManager::QueryRecordByTaskid( eDVBType type,std::string taskId,std::vector<sRecordInfo>& vecRecordInfo,string StartTime,string EndTime)
{
	if (!DBRecord->IsConnected())
	{
		RetryConnect(DBRecord);
	}
	return DBRecord->QueryRecordByTaskid(type,taskId,vecRecordInfo,StartTime,EndTime);
}

bool DBManager::QueryRecordByChannelID(eDVBType type,std::string taskid,std::string ChannelID,std::vector<sRecordInfo>& vecRecordInfo,string StartTime,string EndTime)
{
	if (!DBRecord->IsConnected())
	{
		RetryConnect(DBRecord);
	}
	return DBRecord->QueryRecordByChannelID(type,taskid,ChannelID,vecRecordInfo,StartTime,EndTime);
}
bool DBManager::QueryRecordByDeviceID(eDVBType type,std::string taskid,std::string DeviceID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime,std::string EndTime)
{
	if (!DBRecord->IsConnected())
	{
		RetryConnect(DBRecord);
	}
	return DBRecord->QueryRecordByDeviceID(type,taskid,DeviceID,vecRecordInfo,StartTime,EndTime);
}
bool DBManager::DeleteRecord( std::string filename )
{
	if (!DBRecord->IsConnected())
	{
		RetryConnect(DBRecord);
	}
	return DBRecord->DeleteRecord(filename);
}

bool DBManager::QueryRecordByDays( unsigned int days,std::vector<std::string>& vecFilename )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryRecordByDays(days,vecFilename);
}

bool DBManager::QueryRecordTop(unsigned int count,std::vector<std::string>& vecFilename)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryRecordTop(count,vecFilename);
}

bool DBManager::QueryRecordExpire( std::vector<std::string>& vecFilename )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryRecordExpire(vecFilename);
}
bool DBManager::DeleteSpecDataExpire()
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->DeleteSpecDataExpire();
}
bool DBManager::DeleteQualityDataExpire()
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->DeleteQualityDataExpire();
}

bool DBManager::QueryChannelID( eDVBType type,std::vector<std::string>&vecChannelID )
{
	if (!DBChannel->IsConnected())
	{
		RetryConnect(DBChannel);
	}
	return DBChannel->QueryChannelID(type,vecChannelID);
}

bool DBManager::UpdateAlarmParam(sAlarmParam& alarmparam)
{
	if (!DBAlarmParam->IsConnected())
	{
		RetryConnect(DBAlarmParam);
	}
	bool ret = DBAlarmParam->UpdateAlarmParam(alarmparam);
	return ret;
}
bool DBManager::QueryAlarmParam(eDVBType type,std::vector<sAlarmParam>& AlarmParamVec)
{
	if (!DBAlarmParam->IsConnected())
	{
		RetryConnect(DBAlarmParam);
	}
	return DBAlarmParam->QueryAlarmParam(type,AlarmParamVec);
}

bool DBManager::QueryAlarmParam(std::vector<sAlarmParam>& AlarmParamVec)
{
	if (!DBAlarmParam->IsConnected())
	{
		RetryConnect(DBAlarmParam);
	}
	return DBAlarmParam->QueryAlarmParam(AlarmParamVec);
}

bool DBManager::QueryRunPlanParam( eDVBType type,std::vector<sRunPlanParam>& runPlanParam )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryRunPlanParam(type,runPlanParam);
}

bool DBManager::QueryAllRunPlanParam( std::vector<sRunPlanParam>& runPlanParam )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryAllRunPlanParam(runPlanParam);
}

bool DBManager::UpdateRunPlanParam(eDVBType dvbtype,string channelid,std::vector< sRunPlanParam> runPlanParam )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->UpdateRunPlanParam(dvbtype,channelid,runPlanParam);
}

bool DBManager::AddQualityInfo( eDVBType dvbtype,std::string freq,std::string taskid,eQualityInfo quality,std::string checktime)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->AddQualityInfo(dvbtype,freq,taskid,quality,checktime);
}

bool DBManager::QueryQualityInfo( eDVBType dvbtype,std::string freq,std::string taskid,std::vector<eQualityInfo>& vecQuality,std::string startdatetime,std::string enddatetime )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryQualityInfo(dvbtype,freq,taskid,vecQuality,startdatetime,enddatetime);
}

bool DBManager::GetMsgID( std::string &MsgID )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->GetMsgID(MsgID);
	return true;
}

bool DBManager::UpdateMsgID( std::string MsgID )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->UpdateMsgID(MsgID);
	return true;
}

bool DBManager::QueryCompensationValu(std::vector<sQualityCompensation>& qualityCmpVec)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->QueryCompensationValu(qualityCmpVec);
	return true;
}

bool DBManager::UpdateCompensationValu( sQualityCompensation& qualityCmp )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->UpdateCompensationValu(qualityCmp);
	return true;
}


bool DBManager::AddAlarmInfo( sCheckParam& alarminfo,std::string mode )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->AddAlarmInfo(alarminfo,mode);
	return true;
}


bool DBManager::QueryAlarmInfo( string mode,std::vector<sCheckParam>& vecAlarmInfo,bool bRadioNosignal)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->QueryAlarmInfo(mode,vecAlarmInfo,bRadioNosignal);
	return true;
}

bool DBManager::UpdateAlarmInfo( eDVBType dvbtype,std::string Alarmid,string mode,string unAlarmTime)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->UpdateAlarmInfo(dvbtype,Alarmid,mode,unAlarmTime);
	return true;
}

bool DBManager::QueryFreqAlarmID( eDVBType dvbtype,string deviceid,string freq,string typeID,string& alarmid )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->QueryFreqAlarmID(dvbtype,deviceid,freq,typeID,alarmid);
	return true;
}

bool DBManager::QueryProgramAlarmID(eDVBType dvbtype,string deviceid,string channelid,string typeID,string& alarmid)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->QueryProgramAlarmID(dvbtype,deviceid,channelid,typeID,alarmid);
	return true;
}

bool DBManager::UpdateFileEndTime( std::string filename,std::string endtime )
{

	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->UpdateFileEndTime(filename,endtime);
	return true;
}

bool DBManager::AddSpectrumInfo(enumDVBType dvbtype,std::string freq,std::string taskid,eSpecInfo spec,std::string checktime)
{

	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->AddSpectrumInfo(dvbtype,freq,taskid,spec,checktime);
}
bool DBManager::QuerySpecInfo(eDVBType dvbtype,std::string taskid,std::vector<eSpecInfo>& vecQuality,std::string startdatetime,std::string enddatetime)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QuerySpecInfo(dvbtype,taskid,vecQuality,startdatetime,enddatetime);
}

//wz_101229 
bool DBManager::AddTempFile(string filename, string createtime, string expiredays)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	DBOther->AddTempFile(filename, createtime, expiredays);
	return true;
}

//wz_101229
bool DBManager::QueryTempFileExpire(std::vector<std::string>& vecFilename)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryTempFilesExpire(vecFilename);
}

//wz_101229
bool DBManager::QueryTempFileTop(unsigned int count,std::vector<std::string>& vecFilename)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryTempFileTop(count,vecFilename);
}

//wz_101229
bool DBManager::QueryTempFileByDays( unsigned int days,std::vector<std::string>& vecFilename )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryTempFileByDays(days,vecFilename);
}

//wz_101229
bool DBManager::DeleteTempFile( std::string filename )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->DeleteTempFile(filename);
}

bool DBManager::AlarmInfoQuery( vector<sCheckParam> &vecAlarm )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->AlarmInfoQuery(vecAlarm);
}
bool DBManager::AlarmStateClear(string Freq)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->AlarmStateClear(Freq);
}

bool DBManager::UpdateAlarmRecordExpireTime(eDVBType dvbtype,std::string Alarmid)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->UpdateAlarmRecordExpireTime(dvbtype,Alarmid);
}


bool DBManager::QueryRecordTopInPath(unsigned int count, std::string recPath, std::vector<std::string>& vecFilename)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryRecordTopInPath(count, recPath, vecFilename);
}


bool DBManager::QueryFileConfig(eDVBType dvbtype,std::string NodeName,std::string& NodeValue)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryFileConfig(dvbtype, NodeName, NodeValue);
}

bool DBManager::QuerySystemConfig(eDVBType dvbtype, sSysConfigParam &config)
{

	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QuerySystemConfig(dvbtype, config);
}
bool DBManager::QueryDvbConfig(eDVBType type,vector<sDvbConfigParam> &VecConfig)
{

	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryDvbConfig(type, VecConfig);
}

bool DBManager::QueryAlarmLevelThreshold( vector<ThresHold> &VecThresHold)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->QueryAlarmLevelThreshold(VecThresHold);
}

bool DBManager::SetAlarmLevelThreshold( eDVBType dvbtype,string freq,string alarmthreshold)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}
	return DBOther->SetAlarmLevelThreshold(dvbtype,freq,alarmthreshold);
}

bool DBManager::QueryAlarmPriority(std::vector<sAlarmPriority> &vecAlarmPriority)
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}	
	return DBOther->QueryAlarmPriority(vecAlarmPriority);
}

bool DBManager::QueryScheduleAlarmInfo( string taskid )
{
	if (!DBOther->IsConnected())
	{
		RetryConnect(DBOther);
	}	
	return DBOther->QueryScheduleAlarmInfo(taskid);
}