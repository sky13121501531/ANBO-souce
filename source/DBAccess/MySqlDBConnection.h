#ifndef __MYSQLDBCONNECTION__
#define __MYSQLDBCONNECTION__

///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����MySqlDBConnection.h
// �����ߣ�gaoxd
// ����ʱ�䣺2009-05-27
// ����������MySql���ݿ������
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
	bool AddXmlTask(eDVBType type,std::string xml);//���xml����
	bool DeleteTask(eDVBType type,std::string xml);//ɾ��xml����
	bool QueryTask(eDVBType type,std::vector<std::string>& vecXml);//��ѯ����
	bool QueryTask(std::vector<std::string>& vecXml);//��ѯ����
	//Ƶ��ɨ�����ݿ����
	bool AddChannel(eDVBType type,std::string channelid,std::string xml);//���Ƶ����Ϣ
	bool DeleteChannel(eDVBType type,std::string channelid);//ɾ��Ƶ����Ϣ
	bool DeleteChannelByType(eDVBType type);//ɾ�������͵�Ƶ����Ϣ
	bool QueryChannel(eDVBType type,std::string channelid,std::string& xml);//��ѯָ�����ͺ�Ƶ��id��Ƶ����Ϣ
	bool QueryAllChannel(std::vector<std::string>&vecXml);//��ѯ����Ƶ����Ϣ
	bool QueryTypeChannel(eDVBType type,std::vector<std::string>&vecXml);//��ѯָ�����͵�Ƶ����Ϣ
	bool QueryChannelID(eDVBType type,std::vector<std::string>&vecChannelID);//��ѯָ�����͵�Ƶ��id
	//¼����Ϣ���ݿ����
	bool AddRecord(sRecordInfo record);//���¼����Ϣ
	//���ݼ�����ͺ�����id�����ļ�
	bool QueryRecordByTaskid(eDVBType type,std::string taskId,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");
	//���ݼ�����ͺ�ts����Ϣ����¼���ļ�
	bool QueryRecordByChannelID(eDVBType type,std::string taskID,std::string channelID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");
	//���ݼ�����ͺ�ͨ����Ϣ����¼���ļ�
	bool QueryRecordByDeviceID(eDVBType type,std::string taskid,std::string DeviceID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime="",std::string EndTime="");
	bool UpdateFileEndTime(std::string filename,std::string endtime);
	//���̹���ʹ�ýӿ�
	bool DeleteRecord(std::string filename);//�����ļ���ɾ��¼����Ϣ
	bool QueryRecordByDays(unsigned int days,std::vector<std::string>& vecFilename);//���ݹ���ʱ����¼����Ϣ
	bool QueryRecordTop(unsigned int count,std::vector<std::string>& vecFilename);//��ѯǰ����¼���¼
	bool QueryRecordExpire(std::vector<std::string>& vecFilename);//��ѯ���ڼ�¼
	bool QueryRecordTopInPath(unsigned int count, std::string recPath, std::vector<std::string>& vecFilename);	//��ȡrecPath·����count�����ϵ�¼���ļ�	add by jidushan 11.05.04

	//��������
	bool UpdateAlarmParam(sAlarmParam& alarmparam);//���±�������
	bool QueryAlarmParam(eDVBType type,std::vector<sAlarmParam>& AlarmParamVec);//��ѯ��������
	bool QueryAlarmParam(std::vector<sAlarmParam>& AlarmParamVec);//��ѯ��������
	//�������
	bool AddAlarmInfo(sCheckParam& alarminfo,std::string mode);
	bool UpdateAlarmInfo(eDVBType dvbtype,std::string Alarmid,string mode,string unAlarmTime);
	bool QueryAlarmInfo(string mode,std::vector<sCheckParam>& vecAlarmInfo,bool bRadioNosignal);
	bool QueryFreqAlarmID(eDVBType dvbtype,string deviceid,string freq,string typeID,string& alarmid);
	bool QueryProgramAlarmID(eDVBType dvbtype,string deviceid,string channelid,string typeID,string& alarmid);

	bool UpdateAlarmRecordExpireTime(eDVBType dvbtype,std::string Alarmid);
	//����ͼ����
	bool AddRunPlanParam(sRunPlanParam runPlanParam);//�������ͼ����
	bool QueryRunPlanParam(eDVBType type,std::vector<sRunPlanParam>& runPlanParam);//��ѯָ�����͵�����ͼ����
	bool QueryAllRunPlanParam(std::vector<sRunPlanParam>& runPlanParam);//��ѯ���е�����ͼ����
	bool DeleteRunPlanParam(eDVBType type,std::string channelid);//ɾ��ָ�����͵�����ͼ����
	bool UpdateRunPlanParam(eDVBType dvbtype,std::string channelid,std::vector<sRunPlanParam> runPlanParam);//��������ͼ����
	//ָ����Ϣ
	bool AddQualityInfo(eDVBType dvbtype,std::string freq,std::string taskid,eQualityInfo quality,std::string checktime);//���ָ����Ϣ
	bool QueryQualityInfo(eDVBType dvbtype,std::string freq,std::string taskid,std::vector<eQualityInfo>& vecQuality,std::string startdatetime,std::string enddatetime);//��ѯָ����Ϣ
	//Ƶ����Ϣ
	bool AddSpectrumInfo(eDVBType dvbtype,std::string freq,std::string taskid,eSpecInfo spec,std::string checktime);//���Ƶ����Ϣ
	bool QuerySpecInfo(eDVBType dvbtype,std::string taskid,std::vector<eSpecInfo>& vecSpec,std::string startdatetime,std::string enddatetime);//��ѯƵ����Ϣ
	//MsgID
	bool GetMsgID(std::string &MsgID);//���MsgID
	bool UpdateMsgID(std::string MsgID);//����MsgID
	//ָ�겹��
	bool QueryCompensationValu(std::vector<sQualityCompensation>& qualityCmpVec);//��ѯָ�겹��ֵ
	bool UpdateCompensationValu(sQualityCompensation& qualityCmp);//��ѯָ�겹��ֵ
	bool AddCompensationValu(sQualityCompensation& qualityCmp);//���ָ�겹��ֵ
	bool DelCompensationValu(sQualityCompensation& qualityCmp);//ɾ��ָ�겹��ֵ

	//wz_101229
	//��ʱ�ļ������غ���
	bool AddTempFile(string filename, string createtime, string expiredays);
	bool QueryTempFilesExpire( std::vector<std::string>& vecFilename );
	bool DeleteTempFile(std::string filename);
	bool QueryTempFileByDays(unsigned int days,std::vector<std::string>& vecFilename);
	bool QueryTempFileTop(unsigned int count,std::vector<std::string>& vecFilename);
	//wz_101129
	//������ѯ 2011-03-19
	bool AlarmInfoQuery(vector<sCheckParam> &vecAlarm);
	bool AlarmStateClear(string Freq);
private:
	MySql* DBCon;
};

#endif
