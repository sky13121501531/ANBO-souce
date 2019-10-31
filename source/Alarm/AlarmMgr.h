#pragma once

///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：AlarmMgr.h
// 创建者：gaoxd
// 创建时间：2009-09-08
// 内容描述：报警模块管理类
///////////////////////////////////////////////////////////////////////////////////////////
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include <string>
#include <vector>
#include "../Foundation/TypeDef.h"
#include <ace/Task.h>
class AlarmSender;
class ReSendAlarm;
class alarmrecinfo
{
public:
	string freq;
	long entime;
};
class AlarmMgr:public ACE_Task<ACE_MT_SYNCH>
{
public:
	AlarmMgr(void);
	~AlarmMgr(void);

public:
	bool Start();
	virtual int open(void*);
	virtual int Stop();
	virtual int svc();

public:
	bool CheckAlarm(sCheckParam& checkparam,bool IsConvert);		//对外提供相同接口，使用结构传输参数，便于将来适应报警类型的变化
	bool ClearAlarm(string Freq);					//清除报警状态
	void SendAlarm(std::string alarmxml);
	string GetAlarmTypeID(string headstr);
	bool GetAlarmTypeIDEx(string headstr,vector<int>& alarmtypelist);
	bool GetAlarmParm(const sCheckParam& checkparam,sAlarmParam& alarmparam);	//获取报警参数
	//构造报警或解除报警xml
	bool CreateAlarmXML( const sCheckParam& checkparam,std::string mode,std::string reason,std::string& alarmxml);
	//**************************************
	void AddAlarmRec(string Freq,int iType);//iType=1 tv iType=2 radio
	bool IsAlarmrecExist(string Freq);
	//**************************************
protected:	//适应不同的报警逻辑
	bool CheckEnvironmenAlarmParam(const sCheckParam& checkparam,sAlarmParam& alarmparam,std::string& alarmxml);
	bool CheckFreqAlarmParam(sCheckParam& checkparam,sAlarmParam& alarmparam,std::string& alarmxml);//用于电视指标报警
	bool CheckRadioFreqAlarmParam(vector<sCheckParam>& vecParam);//用于广播指标报警,广播指标有持续时间要求
	int CheckETRAlarm(time_t curTime,vector<sCheckParam>& vecParam);					//根据当前时间判断ETR290是否需要报警
	int CheckProgramAlarm(time_t curTime,vector<sCheckParam>& vecParam,time_t& alarmtime);

protected:

	std::string CreateFreqAlarmXML(const sCheckParam& checkparam,std::string alarmvalue,std::string alarmid);
	std::string CreateProgramAlarmXML(const sCheckParam& checkparam,std::string alarmvalue,std::string alarmid);
	std::string CreateEnvironmentAlarmXML(const sCheckParam& checkparam,std::string alarmvalue);
    std::string CreateEQUIPMENTAlarmXML(const sCheckParam& checkparam,std::string alarmvalue,std::string alarmid);
    std::string CreateEQUIPMENTAlarmXML(const sCheckParam& checkparam,std::string &alarmxml);
protected:
	bool IsAlarmed(std::string typeId);				//判断某个typeid是否已经报过警
	bool ReleaseAlarm(std::string typeId);			//解除typeid的报警
	bool AddAlarm(std::string typeId);				//添加typeid的报警
	bool AddAlarmID(std::string type,int alarmid);
	bool GetAlarmID(std::string type,int& alarmid); 

	string GetAlarmPriority(eDVBType dvbtype,string type);
	
protected:

private:
	std::map<std::string,bool> AlarmSended;//记录报警是否发送
private:
	bool bFlag;
	std::string AlarmID;
	AlarmSender* pAlarmSender;
	ReSendAlarm * pAlarmReSender;
	std::vector<std::string> vecTypeID;//保存报警id
	std::vector<SeperateTypeID> vecSeperateTypeID;
	std::map<std::string,std::vector<sCheckParam> > ETRAlarm;
	std::map<std::string,std::vector<sCheckParam> > ProgramAlarm;
	std::map<std::string,std::vector<sCheckParam> > RadioQualityAlarm;
	std::map<std::string, int > mapAlarmID;//报警和解除报警使用同一个alarmid

	std::vector<sAlarmPriority> vecAlarmPriority;
	ACE_Thread_Mutex ETRMutex;
	ACE_Thread_Mutex ProgramMutex;
	ACE_Thread_Mutex RadioQualityAlarmMutex;
	ACE_Thread_Mutex MutexTypeID;
	ACE_Thread_Mutex MutexAlarmID;
	//***********报警录制相关*****************
	ACE_Thread_Mutex MutexAlarmRecTime;
	string m_tvdeciceid;//录制通道,不能做其他录制
	string m_radiodeciceid;//录制通道,不能做其他录制
	string m_expiredays;//过期天数,单位为天
	string m_reclength;//单位为秒
	long m_lsttime;//记录下次任务可用开始时间
	std::vector<alarmrecinfo> AlarmRec;
	//广播调制度报警
	int m_radiodevalarmunittime;//广播调制度,单元测试时间 单位为秒
	float m_radiodevalarmrate;//单元测试时间,报警不正常率
	float m_radiodevalarmunalarmrate;//单元测试时间,解除报警正常率
	//**************************************
};
typedef ACE_Singleton<AlarmMgr,ACE_Mutex>  ALARMMGR;
