///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����HTTPDeviceAccess.h
// �����ߣ�gaoxd
// ����ʱ�䣺2010-07-06
// ����������Ӳ���豸�����࣬����TCPЭ��
///////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "ace/Synch.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Connector.h"
#include "ace/INET_Addr.h"
#include <string>
#include <vector>
#include "CardType.h"
#include "../Foundation/TypeDef.h"
#include "DeviceAccess.h"


using namespace std;
class TCPDeviceAccess : public DeviceAccess
{
public:
	TCPDeviceAccess(int deviceid,std::string strIP,int nPort);
	virtual ~TCPDeviceAccess(void);
private:
	TCPDeviceAccess(void);

public:
	bool SendTaskMsg(const std::string& strCmdMsg,std::string& strRetMsg);
	bool GetTsReceiveHandle(const std::string& strAddress,ACE_SOCK_Stream& streamHandle){return false;};
	bool CheckDeviceIDLock();					//�жϸ�ͨ���Ƿ�����
//	bool GetRadioTunerQulity(VSTuner_Quality_Result_Obj& radiotunQ); //��ȡ�㲥�ŵ�ָ�����

protected:
	bool ConnectToServer();													//���ӷ�����
	bool SendCmdToServer(MSG_TYPE msg_type,void* info,int infolen);			//�������ݲ��ж��Ƿ�ɹ�
	bool SendCmdToServer(void* indata,int indatalen,void* outdata,int outdatalen);												
	bool SendCmdToServerEx(void* indata,int indatalen,string& outdata);												
	
	bool IsInList(int ifreqkz,std::vector<int>& vecFreq);

	/**	�����������ܽӿ�
	*/
private:
	virtual bool GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg) { return true ; }
	virtual bool GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg){ return true ; }
	virtual bool GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg){ return true ; }
	virtual bool GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg){ return true ; }



	/**	��������ָ��ӿ�
	*/
protected:
	virtual int  SendCmdForChannelScan(MSG_TYPE msg_type,void* info,ChannelScanRetMessage_Handle pRetObj, int infolen)	{return -1 ; }	//����Ƶ��ɨ��
	virtual int  SendCmdForSpecScan(MSG_TYPE msg_type,void* info, RadioSpecRetMessage_Handle pRetObj,int infolen)	{return -1 ; }
	virtual int  SendCmdComForQuality(MSG_TYPE msg_type,void* info,RadioQuaRetMessage_Handle pRetObj,int infolen)	{return -1 ; }
public:
	int  SendCmdToTVCom(MSG_TYPE msg_type,void* info,int infolen);
	int  GetTVQulity(int ifreq,TVQuality &TVQua,int worktype=0);//worktype=0Ϊ�����ϱ�,worktype=1Ϊ�����ϱ�,���ص�ƽ
	int  GetRadioQulity(int ifreq,RadioQuaRetMessage_Obj &rqr,int worktype=0);//worktype=0Ϊ�����ϱ�,worktype=1Ϊ�����ϱ�,���ص�ƽ
	bool Spectrumscan(int startfreq,int endfreq,int scanstep,int measurewide,vector<SpectrumInfo> &vecSpectrumValue,int  worktype=0,int type = 0);//worktype=0Ϊ�����ϱ�,worktype=1Ϊ�����ϱ�;type = 0��������type = 1����׼Ƶ��
	bool Channelscan(int chan,int startfreq,int endfreq,int scanstep,SpecialRadioRetMessage_Obj& ChanScanRet,int type = 0);//type = 0��������type = 1����׼Ƶ��
	/**	��ȡָ��ӿ�(ֻ�ṩ�㲥��AM)
	*/
public:
	virtual bool GetQuality(float fFreq, RadioQuaRetMessage_Obj &rqr)	{ return true ; }
	bool defaultSpectRum(std::vector<int>& freqVec){return true;};
	//virtual bool GetRadioQuality(float fFreq, RadioQuaRetMessage_Obj &rqr)	{ return true ; }
	//virtual bool GetAMQuality(float fFreq, RadioQuaRetMessage_Obj &rqr)	{ return true ; }


	/**	��Ƶ
	*/
private:
	virtual bool TurnFreq(TunerConfig_Obj& tunerinfo) { return true ; }
protected:
	bool SetOSD(int chan,const SubtitleConfig_Obj& osdinfo);	//����OSD
	bool SetSysTime(int chan);

private:
	bool InitCard();						//��ʼ���忨

	bool StartTranscode();					//��ʼת��
	bool StopTranscode();					//ֹͣת��<�����������>

	bool SetVideoAlertInfo( AlertConfig_Obj& alertinfo );		//������Ƶ��̬����
	bool SetAudioAlertInfo( AlertConfig_Obj& alertinfo );		//������Ƶ��̬����
	

protected:
	int GetAudioIndex(int audiorate);		//��ȡ��Ƶ���ʱ��
protected:
	ACE_SOCK_Connector connector;
	ACE_SOCK_Stream stream;
	ACE_INET_Addr server;

	ACE_Thread_Mutex SendtodevMutex;

	time_t m_tChanFixKeepTime;
	bool m_bIsFreqLock;
	int mChannelID;

	VSTuner_Quality_Result_Obj m_curTunerQuality;
	time_t t_m_curTunerQuality;
};
