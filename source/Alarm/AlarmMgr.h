#pragma once

///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����AlarmMgr.h
// �����ߣ�gaoxd
// ����ʱ�䣺2009-09-08
// ��������������ģ�������
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
	bool CheckAlarm(sCheckParam& checkparam,bool IsConvert);		//�����ṩ��ͬ�ӿڣ�ʹ�ýṹ������������ڽ�����Ӧ�������͵ı仯
	bool ClearAlarm(string Freq);					//�������״̬
	void SendAlarm(std::string alarmxml);
	string GetAlarmTypeID(string headstr);
	bool GetAlarmTypeIDEx(string headstr,vector<int>& alarmtypelist);
	bool GetAlarmParm(const sCheckParam& checkparam,sAlarmParam& alarmparam);	//��ȡ��������
	//���챨����������xml
	bool CreateAlarmXML( const sCheckParam& checkparam,std::string mode,std::string reason,std::string& alarmxml);
	//**************************************
	void AddAlarmRec(string Freq,int iType);//iType=1 tv iType=2 radio
	bool IsAlarmrecExist(string Freq);
	//**************************************
protected:	//��Ӧ��ͬ�ı����߼�
	bool CheckEnvironmenAlarmParam(const sCheckParam& checkparam,sAlarmParam& alarmparam,std::string& alarmxml);
	bool CheckFreqAlarmParam(sCheckParam& checkparam,sAlarmParam& alarmparam,std::string& alarmxml);//���ڵ���ָ�걨��
	bool CheckRadioFreqAlarmParam(vector<sCheckParam>& vecParam);//���ڹ㲥ָ�걨��,�㲥ָ���г���ʱ��Ҫ��
	int CheckETRAlarm(time_t curTime,vector<sCheckParam>& vecParam);					//���ݵ�ǰʱ���ж�ETR290�Ƿ���Ҫ����
	int CheckProgramAlarm(time_t curTime,vector<sCheckParam>& vecParam,time_t& alarmtime);

protected:

	std::string CreateFreqAlarmXML(const sCheckParam& checkparam,std::string alarmvalue,std::string alarmid);
	std::string CreateProgramAlarmXML(const sCheckParam& checkparam,std::string alarmvalue,std::string alarmid);
	std::string CreateEnvironmentAlarmXML(const sCheckParam& checkparam,std::string alarmvalue);
    std::string CreateEQUIPMENTAlarmXML(const sCheckParam& checkparam,std::string alarmvalue,std::string alarmid);
    std::string CreateEQUIPMENTAlarmXML(const sCheckParam& checkparam,std::string &alarmxml);
protected:
	bool IsAlarmed(std::string typeId);				//�ж�ĳ��typeid�Ƿ��Ѿ�������
	bool ReleaseAlarm(std::string typeId);			//���typeid�ı���
	bool AddAlarm(std::string typeId);				//���typeid�ı���
	bool AddAlarmID(std::string type,int alarmid);
	bool GetAlarmID(std::string type,int& alarmid); 

	string GetAlarmPriority(eDVBType dvbtype,string type);
	
protected:

private:
	std::map<std::string,bool> AlarmSended;//��¼�����Ƿ���
private:
	bool bFlag;
	std::string AlarmID;
	AlarmSender* pAlarmSender;
	ReSendAlarm * pAlarmReSender;
	std::vector<std::string> vecTypeID;//���汨��id
	std::vector<SeperateTypeID> vecSeperateTypeID;
	std::map<std::string,std::vector<sCheckParam> > ETRAlarm;
	std::map<std::string,std::vector<sCheckParam> > ProgramAlarm;
	std::map<std::string,std::vector<sCheckParam> > RadioQualityAlarm;
	std::map<std::string, int > mapAlarmID;//�����ͽ������ʹ��ͬһ��alarmid

	std::vector<sAlarmPriority> vecAlarmPriority;
	ACE_Thread_Mutex ETRMutex;
	ACE_Thread_Mutex ProgramMutex;
	ACE_Thread_Mutex RadioQualityAlarmMutex;
	ACE_Thread_Mutex MutexTypeID;
	ACE_Thread_Mutex MutexAlarmID;
	//***********����¼�����*****************
	ACE_Thread_Mutex MutexAlarmRecTime;
	string m_tvdeciceid;//¼��ͨ��,����������¼��
	string m_radiodeciceid;//¼��ͨ��,����������¼��
	string m_expiredays;//��������,��λΪ��
	string m_reclength;//��λΪ��
	long m_lsttime;//��¼�´�������ÿ�ʼʱ��
	std::vector<alarmrecinfo> AlarmRec;
	//�㲥���ƶȱ���
	int m_radiodevalarmunittime;//�㲥���ƶ�,��Ԫ����ʱ�� ��λΪ��
	float m_radiodevalarmrate;//��Ԫ����ʱ��,������������
	float m_radiodevalarmunalarmrate;//��Ԫ����ʱ��,�������������
	//**************************************
};
typedef ACE_Singleton<AlarmMgr,ACE_Mutex>  ALARMMGR;
