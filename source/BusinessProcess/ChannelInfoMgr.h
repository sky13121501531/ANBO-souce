///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����ChannelInfoMgr.h
// �����ߣ�gaoxd
// ����ʱ�䣺2009-06-13
// ����������Ƶ����Ϣ����
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "./TaskMonitor.h"
#include "../Foundation/TypeDef.h"
#include <string>
#include <vector>


class ChannelInfoMgr
{
public:
	ChannelInfoMgr();
	virtual ~ChannelInfoMgr();

public:
	bool UpdateChannelInfo(eDVBType type, std::string channelInfo);			//����Ƶ����Ϣ�������ڴ�����ͬʱ�������ݿ�
	bool UpdateChannelByID(eDVBType type,std::vector<std::string> channelvec);
	bool UpdateChannelInfo(eDVBType type,std::vector<sChannelInfo> info);

public:
	bool GetAudioID(enumDVBType dvbtype,const std::string orgNetID,const std::string tsID,const std::string serviceID,std::string& audioPID);
	bool GetVideoID(enumDVBType dvbtype,const std::string orgNetID,const std::string tsID,const std::string serviceID,std::string& videoPID);
	bool GetFreq(enumDVBType dvbtype,const std::string orgNetID,const std::string tsID,const std::string serviceID,std::string& freq);
	bool GetProName(enumDVBType dvbtype,const std::string orgNetID,const std::string tsID,const std::string serviceID,const std::string Freq,std::string& proname);
	bool GetChannelID(enumDVBType dvbtype,const std::string orgNetID,const std::string tsID,const std::string serviceID,const std::string videoPID,const std::string audioPID,std::string chcode,std::string& channelID);
	bool GetOrgNetID(enumDVBType dvbtype,const std::string freq,const std::string audiopid,const std::string videopid,const std::string serviceid,std::string& orgnetid);
	bool GetTsID(enumDVBType dvbtype,const std::string freq,const std::string audiopid,const std::string videopid,const std::string serviceid,std::string& tsid);
	bool GetChannelInfo(enumDVBType dvbtype,const std::string channelid,std::string& vpid,std::string& apid,std::string& serviceid,std::string& prgname);
	bool GetFreqByChannelCode(enumDVBType dvbtype,std::string code,std::string&freq);
	bool GetProNameByFreq(enumDVBType dvbtype,std::string freq,std::string&name);
	bool GetChannelCodeByFreq(enumDVBType dvbtype,std::string freq,std::string&channelcode);
	bool GetSymbolRateQAMByFreq(enumDVBType dvbtype,std::string freq,std::string& symbolrate,std::string& qam);
	bool GetPmtPidPcrPid(enumDVBType dvbtype,std::string freq,std::string serviceid,std::string& pmtpid,std::string& pcrpid);
	bool IsHDTV(enumDVBType dvbtype,const std::string orgNetID,const std::string tsID,const std::string serviceID);
	bool GetChanCodeByChanId(enumDVBType eDvbtype, const std::string chanId, std::string& chanCode);	//ͨ��chanId��ȡchanCode
public:
	bool GetChannelInfoByDvbtype(enumDVBType eDvbtype, std::vector<sChannelInfo>& vecChanInfo );	//ͨ��edvbtype��ȡƵ����Ϣ
	bool GetFreqInfoByDvbtype(enumDVBType eDvbtype, std::vector<std::string>& vecFreqInfo );
private:
	bool InitChannelInfo();		//��ʼ��Ƶ����Ϣ

private:
	bool GetChannelInfoFromReportXML(std::string reportxml);	//���ϴ�xml���Ƶ����Ϣ
	bool GetChannelInfoFromDBXML(std::string dbxml);			//�����ݿ�xml���Ƶ����Ϣ

	bool UpdateChannelInfoMap(const eDVBType& dvbtype,sChannelInfo& channelinfo);		//����Ƶ����Ϣ
	
	bool CreatDBXML(const eDVBType& dvbtype,sChannelInfo& channel,std::string& xml);	//�������ݿ��е�Ƶ����Ϣ����xml
	bool UpdateDB(eDVBType type);

private:
	ChannelInfoMap  mChannelInfoMap;
	ACE_Thread_Mutex ChannelMapMutex;

};

typedef ACE_Singleton<ChannelInfoMgr,ACE_Mutex>  CHANNELINFOMGR;
