#ifndef __MYSQLDBCONNECTION__
#define __MYSQLDBCONNECTION__

///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：MySqlDBConnection.h
// 创建者：gaoxd
// 创建时间：2009-05-27
// 内容描述：MySql数据库操作类
///////////////////////////////////////////////////////////////////////////////////////////

#include "DBConnection.h"

class MySql;

class MySqlDBConnection : public DBConnection
{
public:    
    MySqlDBConnection(void);
    ~MySqlDBConnection(void);
public:
	bool InitConnect(void);
	bool DisConnect(void);
	bool IsConnected(void);
public:
	bool AddXmlTask(eDVBType type,std::string xml);//添加xml任务
	bool DeleteTask(eDVBType type,std::string xml);//删除xml任务
	bool QueryTask(eDVBType type,std::vector<std::string>& vecXml);//查询任务
	bool QueryTask(std::vector<std::string>& vecXml);//查询任务
	//频道扫描数据库操作
	bool AddChannel(eDVBType type,std::string channelid,std::string xml);//添加频道信息
	bool DeleteChannel(eDVBType type,std::string channelid);//删除频道信息
	bool DeleteChannelByType(eDVBType type);//删除此类型的频道信息
	bool QueryChannel(eDVBType type,std::string channelid,std::string& xml);//查询指定类型和频道id的频道信息
	bool QueryAllChannel(std::vector<std::string>&vecXml);//查询所有频道信息
	bool QueryTypeChannel(eDVBType type,std::vector<std::string>&vecXml);//查询指定类型的频道信息
	bool QueryChannelID(eDVBType type,std::vector<std::string>&vecChannelID);//查询指定类型的频道id
	//录像信息数据库操作
	bool AddRecord(sRecordInfo record);//添加录像信息
	//依据监测类型和任务id查找文件
	bool QueryRecordByTaskid(eDVBType type,std::string taskId,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");
	//依据监测类型和ts流信息查找录像文件
	bool QueryRecordByChannelID(eDVBType type,std::string taskID,std::string channelID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");
	//依据监测类型和通道信息查找录像文件
	bool QueryRecordByDeviceID(eDVBType type,std::string taskid,std::string DeviceID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");
	bool UpdateFileEndTime(std::string filename,std::string endtime);
	//磁盘管理使用接口
	bool DeleteRecord(std::string filename);//依据文件名删除录像信息
	bool QueryRecordByDays(unsigned int days,std::vector<std::string>& vecFilename);//依据过期时间获得录像信息
	bool QueryRecordTop(unsigned int count,std::vector<std::string>& vecFilename);//查询前几条录像记录
	bool QueryRecordExpire(std::vector<std::string>& vecFilename);//查询过期记录
	bool QueryRecordTopInPath(unsigned int count, std::string recPath, std::vector<std::string>& vecFilename);	//获取recPath路径下count个最老的录像文件	add by jidushan 11.05.04

	//报警参数
	bool UpdateAlarmParam(sAlarmParam& alarmparam);//更新报警参数
	bool QueryAlarmParam(eDVBType type,std::vector<sAlarmParam>& AlarmParamVec);//查询报警参数
	bool QueryAlarmParam(std::vector<sAlarmParam>& AlarmParamVec);//查询报警参数
	//报警结果
	bool AddAlarmInfo(sCheckParam& alarminfo,std::string mode);
	bool UpdateAlarmInfo(eDVBType dvbtype,std::string Alarmid,string mode,string unAlarmTime);
	bool QueryAlarmInfo(string mode,std::vector<sCheckParam>& vecAlarmInfo,bool bRadioNosignal);
	bool QueryFreqAlarmID(eDVBType dvbtype,string deviceid,string freq,string typeID,string& alarmid);
	bool QueryProgramAlarmID(eDVBType dvbtype,string deviceid,string channelid,string typeID,string& alarmid);

	bool UpdateAlarmRecordExpireTime(eDVBType dvbtype,std::string Alarmid);
	//运行图参数
	bool AddRunPlanParam(sRunPlanParam runPlanParam);//添加运行图参数
	bool QueryRunPlanParam(eDVBType type,std::vector<sRunPlanParam>& runPlanParam);//查询指定类型的运行图参数
	bool QueryAllRunPlanParam(std::vector<sRunPlanParam>& runPlanParam);//查询所有的运行图参数
	bool DeleteRunPlanParam(eDVBType type,std::string channelid);//删除指定类型的运行图参数
	bool UpdateRunPlanParam(eDVBType dvbtype,std::string channelid,std::vector<sRunPlanParam> runPlanParam);//更新运行图参数
	//指标信息
	bool AddQualityInfo(eDVBType dvbtype,std::string freq,std::string taskid,eQualityInfo quality,std::string checktime);//添加指标信息
	bool QueryQualityInfo(eDVBType dvbtype,std::string freq,std::string taskid,std::vector<eQualityInfo>& vecQuality,std::string startdatetime,std::string enddatetime);//查询指标信息
	//频谱信息
	bool AddSpectrumInfo(eDVBType dvbtype,std::string freq,std::string taskid,eSpecInfo spec,std::string checktime);//添加频谱信息
	bool QuerySpecInfo(eDVBType dvbtype,std::string taskid,std::vector<eSpecInfo>& vecSpec,std::string startdatetime,std::string enddatetime);//查询频谱信息
	//MsgID
	bool GetMsgID(std::string &MsgID);//获得MsgID
	bool UpdateMsgID(std::string MsgID);//更新MsgID
	//指标补偿
	bool QueryCompensationValu(std::vector<sQualityCompensation>& qualityCmpVec);//查询指标补偿值
	bool UpdateCompensationValu(sQualityCompensation& qualityCmp);//查询指标补偿值
	bool AddCompensationValu(sQualityCompensation& qualityCmp);//添加指标补偿值
	bool DelCompensationValu(sQualityCompensation& qualityCmp);//删除指标补偿值

	//wz_101229
	//临时文件入库相关函数
	bool AddTempFile(string filename, string createtime, string expiredays);
	bool QueryTempFilesExpire( std::vector<std::string>& vecFilename );
	bool DeleteTempFile(std::string filename);
	bool QueryTempFileByDays(unsigned int days,std::vector<std::string>& vecFilename);
	bool QueryTempFileTop(unsigned int count,std::vector<std::string>& vecFilename);
	//wz_101129
	//报警查询 2011-03-19
	bool AlarmInfoQuery(vector<sCheckParam> &vecAlarm);
	bool AlarmStateClear(string Freq);
private:
	MySql* DBCon;
};

#endif
