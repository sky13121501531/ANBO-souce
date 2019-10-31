#ifndef _PROPMANAGER_H_
#define _PROPMANAGER_H_

#include <string>
#include <list>
#include <map>
#include "ace/Singleton.h"
#include "PropConfig.h"
#include "TypeDef.h"


class PropManager
{
public:
	PropManager(void);
	~PropManager(void);

	//ʵʱ��Ƶ�������
	std::string GetHttpVideoIp(void)		{return mHttpVideoIP;};
	std::string GetHttpVideoPort(void)		{return mHttpVideoPort;};
	std::string GetVideoMaxnum(void)	{return mVideoMaxnum;};

	std::string GetRtspVideoIp(void)		{return mRtspVideoIP;};
	std::string GetRtspVideoPort(void)		{return mRtspVideoPort;};

	//ʵʱָ���������
	std::string GetQualityIp(void)		{return mQualityIp;};
	std::string GetQualityPort(void)	{return mQualityPort;};
	std::string GetQualityMaxnum(void)	{return mQualityMaxnum;};

	//¼��ָ���������
	std::string GetRecordQualityIp(void)		{return mRecordQualityIp;};
	std::string GetRecordQualityPort(void)	{return mRecordQualityPort;};
	std::string GetRecordQualityMaxnum(void)	{return mRecordQualityMaxnum;};

	//�忨�����������
	std::string GetDeviceIp(void)		{return mDeviceIp;};
	std::string GetDevicePort(void)		{return mDevicePort;};
	std::string GetDeviceMaxnum(void)	{return mDeviceMaxnum;};

	//xml���շ����������
	std::string GetXmlServerIP(void)	{return mXmlServerIP;};
	std::string GetXmlServerPort(void)	{return mXmlServerPort;};

	//��־���շ����������
	std::string GetLogIP(void)          {return mLogIp;}
	std::string GetLogPort(void)        {return mLogPort;}
	std::string GetLogMaxnum(void)      {return mLogMaxnum;}

	//���ñ����ϱ���Ϣ 
	bool SetUpAlarmInfo(eDVBType dvbtype,UpAlarmInfo alarminfo);
	//��ȡ�����ϱ���Ϣ
	bool GetUpAlarmInfo (eDVBType dvbtype,UpAlarmInfo& alarminfo);
	

	//��־�������
	std::string GetLogPath(void)		{return mLogPath;};
	std::string GetLogExpire(void)		{return mLogExpire;};
	std::string GetLogAnalyser(void)	{return mLogAnalyser;};
	std::string GetLogVideo(void)		{return mLogVideo;};
	std::string GetLogRecord(void)		{return mLogRecord;};
	std::string GetLogOther(void)		{return mLogOther;};
	std::string GetLogDevice(void)      {return mLogDevice;}
	std::string GetLogDefault(void)		{return mLogDefault;};
	eLogType    GetLogType(void);
	eLogOutPut  GetOutputFile(void);

	//���ݿ��������
	std::string GetDbType(void)			{return mDbType;};
	std::string GetDbIp(void)			{return mDbIp;};
	std::string GetDbPort(void)			{return mDbPort;};
	std::string GetDbUsername(void)		{return mDbUsername;};
	std::string GetDbPwd(void)			{return mDbPwd;};
	std::string GetDbName(void)			{return mDbName;};

	//¼���������
	std::string GetRecordPeriod(void)		{return mRecordPeriod;};
	std::string GetMaxAvailableSize(void)	{return mMaxAvailableSize;};
	std::string GetDBCleanInterval(void)	{return mDBCleanInterval;};
	std::string GetSystemCleanTime(void)	{return mSystemCleanTime;};
	
	void		GetRecFileLocVec(std::vector<string>& vecLoc)	{vecLoc = mRecFileLocVec;};		//��ȡ����¼���ļ�·��������
	std::string GetSharePathByLoc(std::string loc);										//ͨ��¼���ļ�·����ȡ
	bool		GetSharePathByLoc(std::string loc, std::string& sharepath);					//ͨ��¼���ļ�·����ȡ

	//std::string GetAlarmRecordPeriod(void)	{return  mAlarmRecordPeriod; }	//��ȡ��̬¼��ʱ����(ͳһʹ����ͨ¼���ļ���period,�˽ӿ���ʱע�͵�)

	//��̬¼��	
	std::string GetAlarmHeadOffset(enumDVBType eDvbtype);		//��ȡ��̬¼���ļ�ͷƫ������
	std::string GetAlarmTailOffset(enumDVBType eDvbtype);		//��ȡ��̬¼���ļ�βƫ������
	std::string GetAlarmRecordMode(enumDVBType eDvbtype);		//��ȡ��̬¼��mode
	std::string GetAlarmRecordExpire(enumDVBType eDvbtype);		//��ȡ��̬¼���������
	std::string GetRecordExpiredays(enumDVBType eDvbtype);		//��ȡ��ͨ¼���������

	//¼���ļ����������
	std::string GetHttpServerIP(void)		{return mHttpServerIP;};
	std::string GetHttpServerPort(void)		{return mHttpServerport;};
	std::string GetFtpServerIP(void)		{return mFtpServerIP;};
	std::string GetFtpServerPort(void)		{return mFtpServerPort;};	

	//���Ĭ�ϵ�DstCode��SrcCode
	std::string GetDefDstCode(eDVBType dvbtype) {return mDefDstCode[dvbtype];};
	std::string GetDefSrcCode(eDVBType dvbtype) {return mDefSrcCode[dvbtype];};
    std::string GetIsSpectrumFlag(void)	{return IsSpecTrumFlag;};//SpecTrumRet
    std::string GetSpectrumLitRet(void)	{return SpecTrumRet;};
    int GetMonitorDevNum() {return mMonitorDevInfo.size();};
    bool GetDevMonitorInfo(int index,sDeviceInfo& DeviceInfo);
	//��������
	bool IsShareTask(std::string taskname);
	//�豸�����������
	bool GetAllDeviceList(std::list<int>& devicedlist);										//���ȫ���豸ID
	bool GetDVBDeviceList(eDVBType dvbtype,std::list<int>& devicedlist);					//���ĳһ�����豸ID
	bool GetTaskDeviceList(string taskType,eDVBType dvbtype,std::list<int>& devicedlist);	//�����ɸ������ͨ����
	bool GetDeviceAddress(int deviceid,string& ip,int& cmdport,int &tsport,string &cmdprotocol,string &tsprotocol);								//����豸IP��Port
	
	bool GetDeviceIP(int deviceid,string& ip);	//�豸IP
	bool GetDeviceIPandPORT(int deviceid,string& ip,string& port);//�豸IP�Ͷ˿�
	
	bool GetDeviceCmdPort(int deviceid,int& cmdport);								//�豸����˿� 
	bool GetDeviceCmdProtocol(int deviceid,std::string& cmdprotocol);				//�豸����Э��
	
	bool GetDeviceIndex(string & deviceid,string logindex,string devicetype);
	bool GetDeviceIndex(std::list<int>& devicedlist,string baseip);
	bool GetDeviceTunerID(int deviceid,int& tunerid);
	bool GetDeviceLogIndex(int deviceid, int& logindex);
	bool GetDeviceTsPort(int deviceid,int& tsport);									//�豸����Ƶ�˿� 
	bool GetDeviceTsProtocol(int deviceid,std::string& tsprotocol);					//�豸����ƵЭ��

	bool GetDeviceID(string ip,string tunerid,int & deviceid);                      //ͨ��IP��TunerID���DeviceID
	bool GetDeviceIDByIP(string ip,std::list<int> & devicedlist);                   //ͨ��IP���DeviceID�б�
	bool IsDeviceAvaiable(int deviceid,string tasktype,eDVBType dvbtype);			//�ж��ض�������͵�������ڸ�����ͨ�����Ƿ�Ϸ�
	bool IsDeviceAvaiable(int deviceid);											//�ж�ͨ�����Ƿ�Ϸ�
	bool IsRecordDeviceID(int deviceid);											//�ж��Ƿ���¼��ͨ��
	bool GetDeviceType(int deviceid,eDVBType& dvbtype);                             //����ͨ���Ż�ü������
	bool GetQualityCardInfo(eDVBType dvbtype,std::string &ip,int &port);            //��ȡָ�꿨IP �˿���Ϣ

	//���ġ��������������
	std::string GetUrl(const std::string& srcCode);

	//����XML�����������
	long GetXmlSendTimes(void)	{return xmlSendTimes; };
	long GetXmlOvertime(void)	{return xmlOvertime; };

	//��ȡXMl·��
	bool GetXmlPath(std::map<eDVBType,std::string>& xmlpathmap){xmlpathmap = mXmlPath;return true;};
	bool GetXmlPath(eDVBType dvbtype,std::string& xmlpath){xmlpath = mXmlPath[dvbtype];return true;};

	//��ȡtablepath·��
	bool GetXmlTablePath(std::map<eDVBType,std::string>& xmlpathmap){xmlpathmap = mXmlTablePath;return true;};
	bool GetXmlTablePath(eDVBType dvbtype,std::string& xmlpath){xmlpath = mXmlTablePath[dvbtype];return true;};
	//add by jidushan 11.03.31 
	bool GetTableSharePath(eDVBType dvbtype, std::string& sharepath){sharepath = mTableSharePath[dvbtype];return true;};		//����dvbtype��ȡ����·��			
	bool GetTableSharePath(std::map<eDVBType,std::string>& tablesharemap){tablesharemap = mTableSharePath;return true;};		//��ȡ����·��map
	std::string GetTableSharePath(eDVBType dvbtype, bool ret);		//��ȡstring���͹���·��

	//��ȡƵ��ɨ����Ϣ
	bool GetFreqScanInfo(std::map<eDVBType,sFreqScanInfo> FreqScaninfoMap){FreqScaninfoMap = mFreqScanInfoMap;return true;};
	bool GetFreqScanInfo(eDVBType dvbtype,sFreqScanInfo& FreqScanSet){FreqScanSet = mFreqScanInfoMap[dvbtype];return true;};

	bool GetPSISIInfo(eDVBType type,std::string& valu);
	bool SetPSISIInfo(eDVBType type,std::string text);
	//
	std::string GetNewFrAlarmErrNewFreq(void)	{return mNewFrAlarmErrNewFreq;};
	std::string GetNewFrAlarmOkNewFreq(void)	{return mNewFrAlarmOkNewFreq;};
	std::string GetScanlimitlevel(void)	{return mScanlimitlevel;};
	std::string GetScandifflowval(void)	{return mScandifflowval;};

	//CAS SMS��Ϣ
	bool GetCASIP(std::string& CASIP);
	bool GetCASFilePath(std::string& path);
	bool GetSMSIP(std::string& SMSIP);
	bool GetSMSURL(std::string& url);
	bool GetCASPort(int& port);
	bool GetSMSPort(int& port);
	bool GetShareDir(std::string& dir);
	bool GetHttpPath(std::string& path);

	bool GetOSDInfo(eDVBType dvbtype,std::string hdtv,OSDInfo& info);
	bool SetOSDInfo(eDVBType dvbtype,OSDInfo info);
	//���AlarmID
	bool GetAlarmID(long& alarmid);
	
	//�ֲ�����ͨ����� wz_0217
	bool GetVirDevList( std::list<int>& devicelist );							//(���ּ������)��ȡ�ֲ�����������ͨ����
	bool GetVirDeviceId(eDVBType type, int& deviceid);							//����type��ȡ����ͨ����
	bool GetVirDevType(int deviceid, eDVBType& type);							//����deviceid��ȡ�������
	bool IsRoundChannel(int deviceid);											//�ж��Ƿ��ֲ�������ͨ��
		
	//��ȡ��Ƶ����Э�� wz_110309
	//bool GetVideoProtocol(eDVBType dvbtype,std::string& Protocol);
	bool GetStreamProtocol(eDVBType dvbtype,std::string& Protocol);			//��ȡ������Э��
	bool GetFileProtocol(eDVBType dvbtype,std::string& Protocol);			//��ȡ�ļ�����Э��
	bool GetFileUrlType(eDVBType dvbtype,std::string& urltype);				//��ȡ�ļ�URL����			
	bool GetFileOffset(eDVBType dvbtype,std::string& offset);				//��ȡ�ļ�ƫ������
	
	//�ļ�������� wz_110309
	bool GetRecDownProtocol(eDVBType dvbtype,std::string& Protocol);			//��ȡ�ļ����ش���Э��
	bool GetRecDownUrlType(eDVBType dvbtype,std::string& urltype);				//��ȡ�ļ�����URL����			
	bool GetRecDownOffset(eDVBType dvbtype,std::string& offset);				//��ȡ�ļ�����ƫ������

	//��ʱ�ļ����	add by jidushan 11.03.31
	std::string GetTempFileLoac()	{return mTempFileLocation;};	//��ȡ��ʱ�ļ��洢·��
	std::string GetTempSharePath()	{return mTempSharePath;};		//��ȡ��ʱ�ļ�����Ŀ¼
	std::string GetTempFileExpire()	{return mTempFileExpire;};		//��ȡ��ʱ�ļ�����ʱ��

	std::string GetDeviceManagetype()     {return DeviceManagetype;};    //��ȡ�忨����ʱ������
	std::string GetDeviceManageweekday()  {return DeviceManageweekday;};
	std::string GetDeviceManagesingleday() {return DeviceManagesingleday;};
	std::string GetDeviceManagetime()      {return DeviceManagetime;};


	std::string GetPsisiManagetype()     {return PsisiManagetype;};    //��ʱ����PSISIɨ��
	std::string GetPsisiManageweekday()  {return PsisiManageweekday;};
	std::string GetPsisiManagesingleday() {return PsisiManagesingleday;};
	std::string GetPsisiManagetime()      {return PsisiManagetime;};
	//****************************************************************************
	std::string GetAlarmrecTvDeciceid()     {return m_alarmrectvdeciceid;};    
	std::string GetAlarmrecRadioDeciceid()     {return m_alarmrecradiodeciceid;};    
	std::string GetAlarmrecExpiredays()  {return m_alarmrecexpiredays;};
	std::string GetAlarmrecrecLength() {return m_alarmrecreclength;};
	std::string GetRadiodevalarmUnitTime()     {return m_radiodevalarmunittime;};    
	std::string GetRadiodevalarmRate()  {return m_radiodevalarmrate;};
	std::string GetRadiodevalarmUnalarmRate() {return m_radiodevalarmunalarmrate;};
	//**************************************************************************
	void GetAlarmRecStorageCfg(enumDVBType eDvbtype, sAlarmRecordStorageCfg& info)	{ info=mAlarmRecStorageCfgMap[eDvbtype]; }		//��ȡ��̬¼��洢��������Ϣ

	bool SetTempFileExpire(std::string expire);						//������ʱ�ļ�����ʱ��
	bool IsScanFile(eDVBType dvbtype){return (mScanFile[dvbtype]=="1");};      //�Ƿ�ͨ�������ļ����Ƶ��ɨ��
	string GetScanType(eDVBType dvbtype){return mScanFile[dvbtype];};          //��ȡƵ��ɨ������
	bool IsRealTimeFromRecord(eDVBType dvbtype){return (mRealTimeFromRecord[dvbtype]=="1");};//�Ƿ��¼��ͨ����ȡʵʱ��Ƶ

	std::string GetStoreType();		//��ȡ¼���ļ��洢��ʽ����ɢ����˳��		add by jidushan 11.05.04
	std::string GetRecPathByDevId(int deviceid);	//ͨ��ͨ���Ż�ȡ¼���ļ����·��  add by jidushan 11.05.04
	bool InitInfoFromDB();          //�����ݿ��л�ȡ������Ϣ
	void GetHDRecordVideoParam(eDVBType dvbtype,VideoParam &param){param=mHDRecordVideoPram[dvbtype];};//��ȡ����������ʣ��ֱ�����Ϣ
	void GetHDRealStreamVideoParam(eDVBType dvbtype,VideoParam &param){param=mHDRealStreamVideoPram[dvbtype];};
	void GetSDRecordVideoParam(eDVBType dvbtype,VideoParam &param){param=mSDRecordVideoPram[dvbtype];};
	void GetSDRealStreamVideoParam(eDVBType dvbtype,VideoParam &param){param=mSDRealStreamVideoPram[dvbtype];};
	//
	std::string GetTCPTSUrl(int deviceid);
	bool GetDeviceTsIP(int deviceid, std::string& strTsIP);
	bool GetDeviceTsPort(int deviceid,std::string& strtsport);
	bool SeparateStrVec( string src,vector<int>& lis );
private:
	void InitiateBaseInfo();		//��ȡ������Ϣ
	bool InitiateDeviceInfo();       //��������Ϣ��ͨ����Ϣ�����������Ϣ��XML·����Ϣ�����ڴ�

private:
	bool DeleteWrongDeviceID(std::list<int>& deviceIDList);		        //ɾ������ͨ�����ͨ�����в������ͨ����
	bool CheckDeviceInList(int deviceid,std::list<int> deviceList);		//�ж�ͨ����deviceid�Ƿ�����ڸ������ͨ������
	bool SeparateStr(string src,std::list<int>& lis);		            //��src���ݷֺŷָ�ɲ�ͬ���ַ��������뵽lis��
	//�ֽ��ַ���srcloc��srcshare�е�·�����洢��map��vector��	add by jidushan 11.03.31
	bool SepLocShareAndInit(string srcloc, string srcshare, map<string ,string>& loctoshare, vector<string>& vecLoc);
	bool SeparateRecordDir( string src );
private:

	std::string DeviceManagetype;//�忨����ʱ������
	std::string DeviceManageweekday;
	std::string DeviceManagesingleday;
	std::string DeviceManagetime;
	
	std::string PsisiManagetype;//�忨����ʱ������
	std::string PsisiManageweekday;
	std::string PsisiManagesingleday;
	std::string PsisiManagetime;

	PropConfig* mConfig;
	//ϵͳ
	std::string mHttpVideoIP;		//��ƵHttp IP
	std::string mHttpVideoPort;		//��ƵHttp�����˿�
	std::string mVideoMaxnum;		//���������

	std::string mRtspVideoIP;		//��ƵRtsp IP
	std::string mRtspVideoPort;		//��ƵRtsp�����˿�

	std::string mQualityIp;			//ָ��IP
	std::string mQualityPort;		//ָ������˿�
	std::string mQualityMaxnum;		//ָ�����������

	std::string mRecordQualityIp;			//ָ��IP
	std::string mRecordQualityPort;		//ָ������˿�
	std::string mRecordQualityMaxnum;		//ָ�����������

	std::string mDeviceIp;			//�忨����IP
	std::string mDevicePort;		//�忨����˿�
	std::string mDeviceMaxnum;		//���������

	std::string mXmlServerIP;		//xml����IP
	std::string mXmlServerPort;		//xml����Port

	std::string mLogIp;             //��־����IP
	std::string mLogPort;           //��־��������˿�
	std::string mLogMaxnum;         //��־�������������

	std::string mLogPath;			//��־·��
	std::string mLogExpire;			//��־��������
	std::string mLogType;			//��־����
	std::string mLogOutputFile;		//
	std::string mLogAnalyser;		//ָ����־�ļ�����
	std::string mLogVideo;			//��Ƶ��־�ļ�����
	std::string mLogRecord;			//¼��������־�ļ�����
	std::string mLogDevice;         //�豸��־�ļ�����
	std::string mLogOther;			//������־�ļ�����
	std::string mLogDefault;		//Ĭ�ϵ���־�ļ�����
	std::string mDbType;			//���ݿ�����
	std::string mDbIp;				//���ݿ��ַ
	std::string mDbPort;			//���ݿ�˿�
	std::string mDbUsername;		//���ݿ��û���
	std::string mDbPwd;				//���ݿ�����
	std::string mDbName;			//���ݿ�����
	//
	string mNewFrAlarmErrNewFreq;	//������ƵƵ�� �󱨵���ƵƵ��
	string mNewFrAlarmOkNewFreq;	//�����ƵƵ�� ©������ƵƵ��
	string mScanlimitlevel;	//Ƶ��ɨ���ƽ��������
	string mScandifflowval;	//Ƶ��ɨ������ֵ����

	//¼���ļ���Ϣ
	std::map<std::string, std::string> mRecLocToShare;		//¼���ļ�·���빲��·������map
	std::vector<std::string> mRecFileLocVec;			//¼���ļ��洢·���б�

	std::string mRecordPeriod;		//¼��ʱ����
	//std::string mRecordExpireDays;	//¼���ļ���������
	std::string mMaxAvailableSize;	//������С�ռ�GB
	std::string mDBCleanInterval;	//���ݿ�����ʱ����
	std::string mSystemCleanTime;	//ϵͳ����ʱ��

	//std::string mAlarmRecordPeriod;	//��̬¼��ʱ����
	//std::string mAlarmHeadOffset;	//��̬¼���ļ�ͷƫ������
	//std::string mAlarmTailOffset;	//��̬¼���ļ�βƫ������
	//std::string mAlarmRecordExpire;	//��̬¼���������

	std::map<enumDVBType, sRecordParamInfo> mRecParamInfo;	//¼�������Ϣmap
	//¼���ļ����������
	std::string mHttpServerIP;       //http������IP
	std::string mHttpServerport;     //http�������˿�
	std::string mFtpServerIP;        //ftp������IP
	std::string mFtpServerPort;      //ftp�������˿�

	//��ͬ����Ĭ��dstcode��srccode
	std::map<eDVBType,std::string> mDefDstCode;
	std::map<eDVBType,std::string> mDefSrcCode;

	//�豸������Ϣ
	std::map<int,sDeviceInfo> mDeviceInfo;
	std::map<std::string,std::list<int> > mTaskInfo;
	std::map<std::string,bool> mTaskShare;

	//����
	long xmlSendTimes;         //xml�ļ����ʹ���
	long xmlOvertime;          //xml�ļ����ͳ�ʱʱ��[��λ:��]

	std::map<std::string,sCenterInfo*> centerInfoMap;	//������Ϣ
	std::map<eDVBType,std::string> mXmlPath;			//�������·��
	std::map<eDVBType,std::string>mXmlTablePath;        //����Ϣ����·��
	//add by jidushan 11.03.31
	std::map<eDVBType,std::string>mTableSharePath;		//����·��map

	//wz_110309
	std::map<eDVBType, sVideoProcotol>mVideoProtocolMap;	//��Ƶ����Э��map
	std::map<eDVBType, sRecordDownInfo>mRecordDownMap;		//�ļ�������Ϣmap

	//psisi��һ����ɱ�ʶ
	std::string mDVBCPsiSiValue;
	std::string mCTTBPsiSiValue;

	std::map<eDVBType,QualityCard> mQualityCard;//ָ�꿨��Ϣ

	//CAS SMS��ַ��Ϣ
	std::string ShareDir;//���ع���casms�ļ���Ŀ¼
	std::string HttpPath;//Http������Ŀ¼����
	std::string CASIPAddr;
	std::string CASFilePath;
	std::string SMSIPAddr;
	std::string SMSRequestURL;
	int CASPort;
	int SMSPort;

	//****************************************************************************
	string m_alarmrectvdeciceid;//����¼��ͨ��,����������¼��
	string m_alarmrecradiodeciceid;//����¼��ͨ��,����������¼��
	string m_alarmrecexpiredays;//����¼���ļ���������,��λΪ��
	string m_alarmrecreclength;//����¼���ļ��� ��λΪ��
	string m_radiodevalarmunittime;//�㲥���ƶ�,��Ԫ����ʱ�� ��λΪ��
	string m_radiodevalarmrate;//��Ԫ����ʱ��,������������
	string m_radiodevalarmunalarmrate;//��Ԫ����ʱ��,�������������
	//****************************************************************************


	std::map<eDVBType,std::string> mRealTimeFromRecord; //ʵʱ��Ƶ�Ƿ��¼��ͨ����ȡ 0����1����
	std::list<int> RecordList; //����¼��ͨ��
	//Ƶ��ɨ����Ϣ
	std::map<eDVBType,sFreqScanInfo> mFreqScanInfoMap;
	std::map<eDVBType,OSDInfo> mOSDInfoMap;
	//��ͬ������͵�Э��
	//AlarmID
	static long AlarmID;
	
	std::map< enumDVBType, int > mVirtualDevInfo;	//�ֲ�����ͨ����Ϣ  wz_0217
	// ���������ϱ�
	std::map<enumDVBType,UpAlarmInfo> mUpAlarmInfo;
	std::map<enumDVBType,std::string> mScanFile;
	//��ʱ�ļ��������	add by jidushan 11.03.31
	std::string mTempFileLocation;		//��ʱ�ļ������ַ
	std::string mTempSharePath;			//��ʱ�ļ�����·��
	std::string mTempFileExpire;		//��ʱ�ļ�����ʱ��

	std::string mStoreType;			//¼���ļ��洢���ͣ���ɢ����˳��		add by jidushan 11.05.04
	std::map<std::string, std::list<int>> mDeviceIdToPath;		//ͨ������¼���ļ����path����map  add by jidushan 11.05.04

	std::map<enumDVBType,VideoParam> mHDRealStreamVideoPram;
	std::map<enumDVBType,VideoParam> mHDRecordVideoPram;
	std::map<enumDVBType,VideoParam> mSDRealStreamVideoPram;
	std::map<enumDVBType,VideoParam> mSDRecordVideoPram;
	
	std::map<enumDVBType, sAlarmRecordStorageCfg> mAlarmRecStorageCfgMap;
	std::vector<sDeviceInfo> mMonitorDevInfo;

    std::string IsSpecTrumFlag;//�Ƿ�ͨ���м����Ƶ���Ƿ�ָ�  0,1,2:����ֱ��ͨ������   ����ͨ���м��
    std::string SpecTrumRet;
};
typedef ACE_Singleton <PropManager, ACE_Thread_Mutex>  PROPMANAGER;
#endif