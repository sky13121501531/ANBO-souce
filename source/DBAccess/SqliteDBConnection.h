


#ifndef SQLITE_DBCONNECTION_H
#define SQLITE_DBCONNECTION_H


#include "./LSql.h"
#include "./DBConnection.h"
#include "ace/Synch.h"
#include <string>


class SqliteDBConnection : public DBConnection
{
public:
	SqliteDBConnection(void);
	~SqliteDBConnection(void);
public:
	bool CreateSqliteTable();	//建立系统需要的各种表(如果表已经建立了，就跳过)
	bool InitConnect(void);		//初始化sqlserver数据库连接
	bool DisConnect(void);		//断开sqlserver数据库连接
	bool IsConnected(void);
public:
	/**	Task 表相关操作
	*/
	bool AddXmlTask(eDVBType type,std::string xml);		//添加xml任务
	bool DeleteTask(eDVBType type,std::string xml);		//删除xml任务
	bool QueryTask(eDVBType type,std::vector<std::string>& vecXml);		//查询任务
	bool QueryTask(std::vector<std::string>& vecXml);	//查询任务
	
	/** channel 表相关操作
	*/
	bool AddChannel(eDVBType type,std::string channelid,std::string xml);		//添加频道信息
	bool DeleteChannel(eDVBType type,std::string channelid);					//删除频道信息
	bool DeleteChannelByType( eDVBType type);									//删除此类型的频道
	bool QueryChannel(eDVBType type,std::string channelid,std::string& xml);	//查询指定类型和频道id的频道信息
	bool QueryAllChannel(std::vector<std::string>&vecXml);						//查询所有频道信息
	bool QueryTypeChannel(eDVBType type,std::vector<std::string>&vecXml);		//查询指定类型的频道信息
	bool QueryChannelID(eDVBType type,std::vector<std::string>&vecChannelID);	//查询指定类型的频道id
	
	/**	record 表相关操作
	*/
	bool AddRecord(sRecordInfo record);		//添加录像信息
	bool QueryRecordByTaskid(eDVBType type,std::string taskId,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");		//依据监测类型和任务id查找文件
	bool QueryRecordByChannelID(eDVBType type,std::string taskID,std::string channelID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");	//依据监测类型和ts流信息查找录像文件
	bool QueryRecordByDeviceID(eDVBType type,std::string taskid,std::string DeviceID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");	//依据监测类型和通道信息查找录像文件
	bool UpdateFileEndTime(std::string filename,std::string endtime);
	
						/* 磁盘管理使用接口 */
	bool DeleteRecord(std::string filename);//依据文件名删除录像信息
	bool QueryRecordByDays(unsigned int days,std::vector<std::string>& vecFilename);//依据过期时间获得录像信息
	bool QueryRecordTop(unsigned int count,std::vector<std::string>& vecFilename);//查询前几条录像记录
	bool QueryRecordExpire(std::vector<std::string>& vecFilename);//查询过期记录
	bool QueryRecordTopInPath(unsigned int count, std::string recPath, std::vector<std::string>& vecFilename);	//获取recPath路径下count个最老的录像文件	add by jidushan 11.05.04
	
	/** alarmparam 表相关操作
	*/
	bool UpdateAlarmParam(sAlarmParam& alarmparam);//更新报警参数
	bool QueryAlarmParam(eDVBType type,std::vector<sAlarmParam>& AlarmParamVec);//查询报警参数
	bool QueryAlarmParam(std::vector<sAlarmParam>& AlarmParamVec);//查询报警参数
	
	/**	alarminfo 表相关操作
	*/
	bool AddAlarmInfo(sCheckParam& alarminfo,std::string mode);
	bool UpdateAlarmInfo(eDVBType dvbtype,std::string Alarmid,string mode,string unAlarmTime);
	bool QueryAlarmInfo(string mode,std::vector<sCheckParam>& vecAlarmInfo,bool bRadioNosignal);
	bool QueryFreqAlarmID(eDVBType dvbtype,string deviceid,string freq,string typeID,string& alarmid);
	bool QueryProgramAlarmID(eDVBType dvbtype,string deviceid,string channelid,string typeID,string& alarmid);
	bool UpdateAlarmRecordExpireTime(eDVBType dvbtype,std::string Alarmid);
	
	bool AlarmInfoQuery(vector<sCheckParam> &vecAlarm);
	bool AlarmStateClear(string Freq);

	/**	runplanparam 表相关操作
	*/
	bool AddRunPlanParam(sRunPlanParam runPlanParam);//添加运行图参数
	bool QueryRunPlanParam(eDVBType type,std::vector<sRunPlanParam>& runPlanParam);//查询指定类型的运行图参数
	bool QueryAllRunPlanParam(std::vector<sRunPlanParam>& runPlanParam);//查询所有的运行图参数
	bool DeleteRunPlanParam(eDVBType type,std::string channelid);//删除指定类型的运行图参数
	bool UpdateRunPlanParam(eDVBType dvbtype,std::string channelid,std::vector<sRunPlanParam> runPlanParam);//更新运行图参数
	
	/**	quality 表相关操作
	*/
	bool AddQualityInfo(eDVBType dvbtype,std::string freq,std::string taskid,eQualityInfo quality,std::string checktime);//添加指标信息
	bool QueryQualityInfo(eDVBType dvbtype,std::string freq,std::string taskid,std::vector<eQualityInfo>& vecQuality,std::string startdatetime,std::string enddatetime);//查询指标信息
	
	/**	spec 表相关操作
	*/
	bool AddSpectrumInfo(eDVBType dvbtype,std::string freq,std::string taskid,eSpecInfo spec,std::string checktime);//添加频谱信息
	bool QuerySpecInfo(eDVBType dvbtype,std::string taskid,std::vector<eSpecInfo>& vecSpec,std::string startdatetime,std::string enddatetime);//查询频谱信息
	
	/**	MsgID 表相关操作
	*/
	bool GetMsgID(std::string &MsgID);//获得MsgID
	bool UpdateMsgID(std::string MsgID);//更新MsgID
	
	/**	qualitycompensation 表相关操作
	*/
	bool QueryCompensationValu(std::vector<sQualityCompensation>& qualityCmpVec);//查询指标补偿值
	bool UpdateCompensationValu(sQualityCompensation& qualityCmp);//查询指标补偿值
	bool AddCompensationValu(sQualityCompensation& qualityCmp);//添加指标补偿值
	bool DelCompensationValu(sQualityCompensation& qualityCmp);//删除指标补偿值
	
	/**	tempfile 表相关操作
	*/
	bool AddTempFile(string filename, string createtime, string expiredays);
	bool QueryTempFilesExpire( std::vector<std::string>& vecFilename );
	bool DeleteTempFile(std::string filename);
	bool QueryTempFileByDays(unsigned int expireDays,std::vector<std::string>& vecFilename);	//
	bool QueryTempFileTop(unsigned int count,std::vector<std::string>& vecFilename);

private:
	DeriveLsql* DBCon;

private:
	ACE_Thread_Mutex m_sqliteMutex; 
};






#endif