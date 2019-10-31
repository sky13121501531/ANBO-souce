
#ifndef __DBMANAGER_H_
#define __DBMANAGER_H_

///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：DBManager.h
// 创建者：gaoxd
// 创建时间：2009-05-27
// 内容描述：数据库操作管理类，外部通过统一接口操作数据库
///////////////////////////////////////////////////////////////////////////////////////////

#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "../Foundation/TypeDef.h"
#include "DBParamTypeDef.h"

class DBConnection;

class DBManager
{
public:    
    DBManager(void);
    virtual ~DBManager(void);
private:
	 bool InitConnect(void);
	 bool DisConnect(void);
	 bool IsConnected(void);
	 bool RetryConnect(DBConnection* db);
public:
	//任务数据库操作
	bool AddXmlTask(eDVBType type,std::string xml);
	bool DeleteTask(eDVBType type,std::string xml);
	bool QueryTask(eDVBType type,std::vector<std::string>&  vecXml);
	bool QueryTask(std::vector<std::string>&  vecXml);
	//频道扫描数据库操作
	bool AddChannel(eDVBType type,std::string channelid,std::string xml);
	bool DeleteChannel(eDVBType type,std::string channelid);
	bool DeleteChannelByType(eDVBType type);
	bool QueryChannel(eDVBType type,std::string channelid,std::string& xml);
	bool QueryTypeChannel(eDVBType type,std::vector<std::string>&vecXml);
	bool QueryAllChannel(std::vector<std::string>&vecXml);
	bool QueryChannelID(eDVBType type,std::vector<std::string>&vecChannelID);
	//录像信息数据库操作
	bool AddRecord(sRecordInfo record);
	//依据监测类型和任务ID查找文件
	bool QueryRecordByTaskid(eDVBType type,std::string taskId,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");
	//依据监测类型和频道信息查找录像文件
	bool QueryRecordByChannelID(eDVBType type,std::string taskid,std::string ChannelID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");
	//依据监测类型和通道信息查找录像文件
	bool QueryRecordByDeviceID(eDVBType type,std::string taskid,std::string DeviceID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");
	//更新录像文件的结束时间
	bool UpdateFileEndTime(std::string filename,std::string endtime);
	//磁盘管理使用接口
	bool DeleteRecord(std::string filename);//依据文件名删除录像信息
	bool QueryRecordByDays(unsigned int days,std::vector<std::string>& vecFilename);//依据过期时间删除录像信息
	bool QueryRecordTop(unsigned int count,std::vector<std::string>& vecFilename);//查询前几条录像记录
	bool QueryRecordExpire(std::vector<std::string>& vecFilename);//查询过期记录
	bool DeleteSpecDataExpire();
	bool DeleteQualityDataExpire();
	bool QueryRecordTopInPath(unsigned int count, std::string recPath, std::vector<std::string>& vecFilename);	//获取recPath路径下count个最老的录像文件	add by jidushan 11.05.04
	//报警参数
	bool UpdateAlarmParam(sAlarmParam& alarmparam);//更新报警参数
	bool QueryAlarmParam(eDVBType type,std::vector<sAlarmParam>& AlarmParamVec);//查询报警参数
	bool QueryAlarmParam(std::vector<sAlarmParam>& AlarmParamVec);
	//运行图参数
	bool QueryRunPlanParam(eDVBType dvbtype,std::vector<sRunPlanParam>& runPlanParam);
	bool QueryAllRunPlanParam(std::vector<sRunPlanParam>& runPlanParam);
	bool UpdateRunPlanParam(eDVBType dvbtype,std::string channelid,std::vector<sRunPlanParam> runPlanParam);
	//指标测量
	bool AddQualityInfo(eDVBType dvbtype,std::string freq,std::string taskid,eQualityInfo quality,std::string checktime);
	bool QueryQualityInfo(eDVBType dvbtype,std::string freq,std::string taskid,std::vector<eQualityInfo>& vecQuality,std::string startdatetime,std::string enddatetime);
	//频谱测量
	bool AddSpectrumInfo(enumDVBType dvbtype,std::string freq,std::string taskid,eSpecInfo spec,std::string checktime);
	bool QuerySpecInfo(eDVBType dvbtype,std::string taskid,std::vector<eSpecInfo>& vecQuality,std::string startdatetime,std::string enddatetime);
	//MsgID
	bool GetMsgID(std::string &MsgID);//获得MsgID
	bool UpdateMsgID(std::string MsgID);//更新MsgID
	//指标补偿
	bool QueryCompensationValu(std::vector<sQualityCompensation>& qualityCmpVec);//查询指标补偿值
	bool UpdateCompensationValu(sQualityCompensation& qualityCmp);//更新指标补偿值
	//报警信息
	bool AddAlarmInfo(sCheckParam& alarminfo,std::string mode);
	bool UpdateAlarmInfo(eDVBType dvbtype,std::string Alarmid,string mode,string unAlarmTime);
	bool QueryAlarmInfo(string mode,std::vector<sCheckParam>& vecAlarmInfo,bool bRadioNosignal);
	bool QueryFreqAlarmID(eDVBType dvbtype,string deviceid,string freq,string typeID,string& alarmid);
	bool QueryProgramAlarmID(eDVBType dvbtype,string deviceid,string channelid,string typeID,string& alarmid);
	//更新异态录像过期时间
	bool UpdateAlarmRecordExpireTime(eDVBType dvbtype,std::string Alarmid);
	//wz_101229
	//临时文件入库
	bool AddTempFile(string filename, string createtime, string expiredays); //向数据库中添加临时文件信息
	bool QueryTempFileExpire(std::vector<std::string>& vecFilename);//查询过期临时文件
	bool DeleteTempFile( std::string filename );  //从数据库中删除filename对应的临时文件信息
	bool QueryTempFileByDays(unsigned int days,std::vector<std::string>& vecFilename);  //按天数查询临时文件信息
	bool QueryTempFileTop(unsigned int count,std::vector<std::string>& vecFilename);  //按时间查询从头开始的count个临时文件信息
	//wz_101229
	//报警查询2011-03-19
	bool AlarmInfoQuery(vector<sCheckParam> &vecAlarm);
	//报警状态清除2011-03-19
	bool AlarmStateClear(string Freq);
	//配置信息查询
    bool QueryFileConfig(eDVBType dvbtype,std::string NodeName,std::string& NodeValue);


	bool QuerySystemConfig(eDVBType dvbtype, sSysConfigParam &config);
	bool QueryDvbConfig(eDVBType type, vector<sDvbConfigParam> &VecConfig);
	bool QueryAlarmLevelThreshold( vector<ThresHold> &VecThresHold);
	bool SetAlarmLevelThreshold( eDVBType dvbtype,string freq,string alarmthreshold);

	bool QueryAlarmPriority(std::vector<sAlarmPriority> &vecAlarmPriority);
	bool QueryScheduleAlarmInfo( string taskid );
private:
	DBConnection* DBTask;
	DBConnection* DBChannel;
	DBConnection* DBRecord;
	DBConnection* DBOther;
	DBConnection* DBAlarmParam;
};
typedef ACE_Singleton<DBManager,ACE_Mutex>  DBMANAGER;
#endif