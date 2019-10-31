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

	//实时视频相关配置
	std::string GetHttpVideoIp(void)		{return mHttpVideoIP;};
	std::string GetHttpVideoPort(void)		{return mHttpVideoPort;};
	std::string GetVideoMaxnum(void)	{return mVideoMaxnum;};

	std::string GetRtspVideoIp(void)		{return mRtspVideoIP;};
	std::string GetRtspVideoPort(void)		{return mRtspVideoPort;};

	//实时指标相关配置
	std::string GetQualityIp(void)		{return mQualityIp;};
	std::string GetQualityPort(void)	{return mQualityPort;};
	std::string GetQualityMaxnum(void)	{return mQualityMaxnum;};

	//录像指标相关配置
	std::string GetRecordQualityIp(void)		{return mRecordQualityIp;};
	std::string GetRecordQualityPort(void)	{return mRecordQualityPort;};
	std::string GetRecordQualityMaxnum(void)	{return mRecordQualityMaxnum;};

	//板卡服务相关配置
	std::string GetDeviceIp(void)		{return mDeviceIp;};
	std::string GetDevicePort(void)		{return mDevicePort;};
	std::string GetDeviceMaxnum(void)	{return mDeviceMaxnum;};

	//xml接收服务相关配置
	std::string GetXmlServerIP(void)	{return mXmlServerIP;};
	std::string GetXmlServerPort(void)	{return mXmlServerPort;};

	//日志接收服务相关配置
	std::string GetLogIP(void)          {return mLogIp;}
	std::string GetLogPort(void)        {return mLogPort;}
	std::string GetLogMaxnum(void)      {return mLogMaxnum;}

	//设置报警上报信息 
	bool SetUpAlarmInfo(eDVBType dvbtype,UpAlarmInfo alarminfo);
	//获取报警上报信息
	bool GetUpAlarmInfo (eDVBType dvbtype,UpAlarmInfo& alarminfo);
	

	//日志相关配置
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

	//数据库相关配置
	std::string GetDbType(void)			{return mDbType;};
	std::string GetDbIp(void)			{return mDbIp;};
	std::string GetDbPort(void)			{return mDbPort;};
	std::string GetDbUsername(void)		{return mDbUsername;};
	std::string GetDbPwd(void)			{return mDbPwd;};
	std::string GetDbName(void)			{return mDbName;};

	//录像相关配置
	std::string GetRecordPeriod(void)		{return mRecordPeriod;};
	std::string GetMaxAvailableSize(void)	{return mMaxAvailableSize;};
	std::string GetDBCleanInterval(void)	{return mDBCleanInterval;};
	std::string GetSystemCleanTime(void)	{return mSystemCleanTime;};
	
	void		GetRecFileLocVec(std::vector<string>& vecLoc)	{vecLoc = mRecFileLocVec;};		//获取保存录像文件路径的容器
	std::string GetSharePathByLoc(std::string loc);										//通过录像文件路径获取
	bool		GetSharePathByLoc(std::string loc, std::string& sharepath);					//通过录像文件路径获取

	//std::string GetAlarmRecordPeriod(void)	{return  mAlarmRecordPeriod; }	//获取异态录像时间间隔(统一使用普通录像文件的period,此接口暂时注释掉)

	//异态录像	
	std::string GetAlarmHeadOffset(enumDVBType eDvbtype);		//获取异态录像文件头偏移秒数
	std::string GetAlarmTailOffset(enumDVBType eDvbtype);		//获取异态录像文件尾偏移秒数
	std::string GetAlarmRecordMode(enumDVBType eDvbtype);		//获取异态录像mode
	std::string GetAlarmRecordExpire(enumDVBType eDvbtype);		//获取异态录像过期天数
	std::string GetRecordExpiredays(enumDVBType eDvbtype);		//获取普通录像过期天数

	//录像文件服务器相关
	std::string GetHttpServerIP(void)		{return mHttpServerIP;};
	std::string GetHttpServerPort(void)		{return mHttpServerport;};
	std::string GetFtpServerIP(void)		{return mFtpServerIP;};
	std::string GetFtpServerPort(void)		{return mFtpServerPort;};	

	//获得默认的DstCode和SrcCode
	std::string GetDefDstCode(eDVBType dvbtype) {return mDefDstCode[dvbtype];};
	std::string GetDefSrcCode(eDVBType dvbtype) {return mDefSrcCode[dvbtype];};
    std::string GetIsSpectrumFlag(void)	{return IsSpecTrumFlag;};//SpecTrumRet
    std::string GetSpectrumLitRet(void)	{return SpecTrumRet;};
    int GetMonitorDevNum() {return mMonitorDevInfo.size();};
    bool GetDevMonitorInfo(int index,sDeviceInfo& DeviceInfo);
	//任务属性
	bool IsShareTask(std::string taskname);
	//设备管理相关配置
	bool GetAllDeviceList(std::list<int>& devicedlist);										//获得全部设备ID
	bool GetDVBDeviceList(eDVBType dvbtype,std::list<int>& devicedlist);					//获得某一类型设备ID
	bool GetTaskDeviceList(string taskType,eDVBType dvbtype,std::list<int>& devicedlist);	//获得完成该任务的通道组
	bool GetDeviceAddress(int deviceid,string& ip,int& cmdport,int &tsport,string &cmdprotocol,string &tsprotocol);								//获得设备IP、Port
	
	bool GetDeviceIP(int deviceid,string& ip);	//设备IP
	bool GetDeviceIPandPORT(int deviceid,string& ip,string& port);//设备IP和端口
	
	bool GetDeviceCmdPort(int deviceid,int& cmdport);								//设备命令端口 
	bool GetDeviceCmdProtocol(int deviceid,std::string& cmdprotocol);				//设备命令协议
	
	bool GetDeviceIndex(string & deviceid,string logindex,string devicetype);
	bool GetDeviceIndex(std::list<int>& devicedlist,string baseip);
	bool GetDeviceTunerID(int deviceid,int& tunerid);
	bool GetDeviceLogIndex(int deviceid, int& logindex);
	bool GetDeviceTsPort(int deviceid,int& tsport);									//设备音视频端口 
	bool GetDeviceTsProtocol(int deviceid,std::string& tsprotocol);					//设备音视频协议

	bool GetDeviceID(string ip,string tunerid,int & deviceid);                      //通过IP和TunerID获得DeviceID
	bool GetDeviceIDByIP(string ip,std::list<int> & devicedlist);                   //通过IP获得DeviceID列表
	bool IsDeviceAvaiable(int deviceid,string tasktype,eDVBType dvbtype);			//判断特定监测类型的任务对于给定的通道号是否合法
	bool IsDeviceAvaiable(int deviceid);											//判断通道号是否合法
	bool IsRecordDeviceID(int deviceid);											//判断是否是录像通道
	bool GetDeviceType(int deviceid,eDVBType& dvbtype);                             //根据通道号获得监测类型
	bool GetQualityCardInfo(eDVBType dvbtype,std::string &ip,int &port);            //获取指标卡IP 端口信息

	//中心、分中心相关配置
	std::string GetUrl(const std::string& srcCode);

	//上行XML发送相关配置
	long GetXmlSendTimes(void)	{return xmlSendTimes; };
	long GetXmlOvertime(void)	{return xmlOvertime; };

	//获取XMl路径
	bool GetXmlPath(std::map<eDVBType,std::string>& xmlpathmap){xmlpathmap = mXmlPath;return true;};
	bool GetXmlPath(eDVBType dvbtype,std::string& xmlpath){xmlpath = mXmlPath[dvbtype];return true;};

	//获取tablepath路径
	bool GetXmlTablePath(std::map<eDVBType,std::string>& xmlpathmap){xmlpathmap = mXmlTablePath;return true;};
	bool GetXmlTablePath(eDVBType dvbtype,std::string& xmlpath){xmlpath = mXmlTablePath[dvbtype];return true;};
	//add by jidushan 11.03.31 
	bool GetTableSharePath(eDVBType dvbtype, std::string& sharepath){sharepath = mTableSharePath[dvbtype];return true;};		//根据dvbtype获取表共享路径			
	bool GetTableSharePath(std::map<eDVBType,std::string>& tablesharemap){tablesharemap = mTableSharePath;return true;};		//获取共享路径map
	std::string GetTableSharePath(eDVBType dvbtype, bool ret);		//获取string类型共享路径

	//获取频率扫描信息
	bool GetFreqScanInfo(std::map<eDVBType,sFreqScanInfo> FreqScaninfoMap){FreqScaninfoMap = mFreqScanInfoMap;return true;};
	bool GetFreqScanInfo(eDVBType dvbtype,sFreqScanInfo& FreqScanSet){FreqScanSet = mFreqScanInfoMap[dvbtype];return true;};

	bool GetPSISIInfo(eDVBType type,std::string& valu);
	bool SetPSISIInfo(eDVBType type,std::string text);
	//
	std::string GetNewFrAlarmErrNewFreq(void)	{return mNewFrAlarmErrNewFreq;};
	std::string GetNewFrAlarmOkNewFreq(void)	{return mNewFrAlarmOkNewFreq;};
	std::string GetScanlimitlevel(void)	{return mScanlimitlevel;};
	std::string GetScandifflowval(void)	{return mScandifflowval;};

	//CAS SMS信息
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
	//获得AlarmID
	bool GetAlarmID(long& alarmid);
	
	//轮播虚拟通道相关 wz_0217
	bool GetVirDevList( std::list<int>& devicelist );							//(不分监测类型)获取轮播的所有虚拟通道号
	bool GetVirDeviceId(eDVBType type, int& deviceid);							//根据type获取虚拟通道号
	bool GetVirDevType(int deviceid, eDVBType& type);							//根据deviceid获取监测类型
	bool IsRoundChannel(int deviceid);											//判断是否轮播的虚拟通道
		
	//获取视频服务协议 wz_110309
	//bool GetVideoProtocol(eDVBType dvbtype,std::string& Protocol);
	bool GetStreamProtocol(eDVBType dvbtype,std::string& Protocol);			//获取流传送协议
	bool GetFileProtocol(eDVBType dvbtype,std::string& Protocol);			//获取文件传送协议
	bool GetFileUrlType(eDVBType dvbtype,std::string& urltype);				//获取文件URL类型			
	bool GetFileOffset(eDVBType dvbtype,std::string& offset);				//获取文件偏移类型
	
	//文件下载相关 wz_110309
	bool GetRecDownProtocol(eDVBType dvbtype,std::string& Protocol);			//获取文件下载传送协议
	bool GetRecDownUrlType(eDVBType dvbtype,std::string& urltype);				//获取文件下载URL类型			
	bool GetRecDownOffset(eDVBType dvbtype,std::string& offset);				//获取文件下载偏移类型

	//临时文件相关	add by jidushan 11.03.31
	std::string GetTempFileLoac()	{return mTempFileLocation;};	//获取临时文件存储路径
	std::string GetTempSharePath()	{return mTempSharePath;};		//获取临时文件共享目录
	std::string GetTempFileExpire()	{return mTempFileExpire;};		//获取临时文件过期时间

	std::string GetDeviceManagetype()     {return DeviceManagetype;};    //获取板卡管理时间配置
	std::string GetDeviceManageweekday()  {return DeviceManageweekday;};
	std::string GetDeviceManagesingleday() {return DeviceManagesingleday;};
	std::string GetDeviceManagetime()      {return DeviceManagetime;};


	std::string GetPsisiManagetype()     {return PsisiManagetype;};    //定时进行PSISI扫描
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
	void GetAlarmRecStorageCfg(enumDVBType eDvbtype, sAlarmRecordStorageCfg& info)	{ info=mAlarmRecStorageCfgMap[eDvbtype]; }		//获取异态录像存储的配置信息

	bool SetTempFileExpire(std::string expire);						//设置临时文件过期时间
	bool IsScanFile(eDVBType dvbtype){return (mScanFile[dvbtype]=="1");};      //是否通过本地文件完成频道扫描
	string GetScanType(eDVBType dvbtype){return mScanFile[dvbtype];};          //获取频道扫描类型
	bool IsRealTimeFromRecord(eDVBType dvbtype){return (mRealTimeFromRecord[dvbtype]=="1");};//是否从录像通道获取实时视频

	std::string GetStoreType();		//获取录像文件存储方式：分散还是顺序		add by jidushan 11.05.04
	std::string GetRecPathByDevId(int deviceid);	//通过通道号获取录像文件存放路径  add by jidushan 11.05.04
	bool InitInfoFromDB();          //从数据库中获取配置信息
	void GetHDRecordVideoParam(eDVBType dvbtype,VideoParam &param){param=mHDRecordVideoPram[dvbtype];};//获取相关类型码率，分辨率信息
	void GetHDRealStreamVideoParam(eDVBType dvbtype,VideoParam &param){param=mHDRealStreamVideoPram[dvbtype];};
	void GetSDRecordVideoParam(eDVBType dvbtype,VideoParam &param){param=mSDRecordVideoPram[dvbtype];};
	void GetSDRealStreamVideoParam(eDVBType dvbtype,VideoParam &param){param=mSDRealStreamVideoPram[dvbtype];};
	//
	std::string GetTCPTSUrl(int deviceid);
	bool GetDeviceTsIP(int deviceid, std::string& strTsIP);
	bool GetDeviceTsPort(int deviceid,std::string& strtsport);
	bool SeparateStrVec( string src,vector<int>& lis );
private:
	void InitiateBaseInfo();		//获取基本信息
	bool InitiateDeviceInfo();       //将任务信息、通道信息、监测中心信息、XML路径信息读入内存

private:
	bool DeleteWrongDeviceID(std::list<int>& deviceIDList);		        //删除任务通道组的通道号中不合理的通道号
	bool CheckDeviceInList(int deviceid,std::list<int> deviceList);		//判断通道号deviceid是否存在于该任务的通道组中
	bool SeparateStr(string src,std::list<int>& lis);		            //将src依据分号分割成不同的字符串，放入到lis中
	//分解字符串srcloc和srcshare中的路径，存储到map和vector中	add by jidushan 11.03.31
	bool SepLocShareAndInit(string srcloc, string srcshare, map<string ,string>& loctoshare, vector<string>& vecLoc);
	bool SeparateRecordDir( string src );
private:

	std::string DeviceManagetype;//板卡管理时间设置
	std::string DeviceManageweekday;
	std::string DeviceManagesingleday;
	std::string DeviceManagetime;
	
	std::string PsisiManagetype;//板卡管理时间设置
	std::string PsisiManageweekday;
	std::string PsisiManagesingleday;
	std::string PsisiManagetime;

	PropConfig* mConfig;
	//系统
	std::string mHttpVideoIP;		//视频Http IP
	std::string mHttpVideoPort;		//视频Http监听端口
	std::string mVideoMaxnum;		//最多连接数

	std::string mRtspVideoIP;		//视频Rtsp IP
	std::string mRtspVideoPort;		//视频Rtsp监听端口

	std::string mQualityIp;			//指标IP
	std::string mQualityPort;		//指标监听端口
	std::string mQualityMaxnum;		//指标最多连接数

	std::string mRecordQualityIp;			//指标IP
	std::string mRecordQualityPort;		//指标监听端口
	std::string mRecordQualityMaxnum;		//指标最多连接数

	std::string mDeviceIp;			//板卡服务IP
	std::string mDevicePort;		//板卡服务端口
	std::string mDeviceMaxnum;		//最多连接数

	std::string mXmlServerIP;		//xml接收IP
	std::string mXmlServerPort;		//xml接收Port

	std::string mLogIp;             //日志服务IP
	std::string mLogPort;           //日志服务监听端口
	std::string mLogMaxnum;         //日志服务最多连接数

	std::string mLogPath;			//日志路径
	std::string mLogExpire;			//日志过期天数
	std::string mLogType;			//日志类型
	std::string mLogOutputFile;		//
	std::string mLogAnalyser;		//指标日志文件夹名
	std::string mLogVideo;			//视频日志文件夹名
	std::string mLogRecord;			//录像任务日志文件夹名
	std::string mLogDevice;         //设备日志文件夹名
	std::string mLogOther;			//其他日志文件夹名
	std::string mLogDefault;		//默认的日志文件夹名
	std::string mDbType;			//数据库类型
	std::string mDbIp;				//数据库地址
	std::string mDbPort;			//数据库端口
	std::string mDbUsername;		//数据库用户名
	std::string mDbPwd;				//数据库密码
	std::string mDbName;			//数据库名称
	//
	string mNewFrAlarmErrNewFreq;	//过滤新频频点 误报的新频频点
	string mNewFrAlarmOkNewFreq;	//添加新频频点 漏报的新频频点
	string mScanlimitlevel;	//频道扫描电平门限配置
	string mScandifflowval;	//频道扫描底噪差值配置

	//录像文件信息
	std::map<std::string, std::string> mRecLocToShare;		//录像文件路径与共享路径关联map
	std::vector<std::string> mRecFileLocVec;			//录像文件存储路径列表

	std::string mRecordPeriod;		//录像时间间隔
	//std::string mRecordExpireDays;	//录像文件过期天数
	std::string mMaxAvailableSize;	//磁盘最小空间GB
	std::string mDBCleanInterval;	//数据库清理时间间隔
	std::string mSystemCleanTime;	//系统清理时间

	//std::string mAlarmRecordPeriod;	//异态录像时间间隔
	//std::string mAlarmHeadOffset;	//异态录像文件头偏移秒数
	//std::string mAlarmTailOffset;	//异态录像文件尾偏移秒数
	//std::string mAlarmRecordExpire;	//异态录像过期天数

	std::map<enumDVBType, sRecordParamInfo> mRecParamInfo;	//录像参数信息map
	//录像文件服务器相关
	std::string mHttpServerIP;       //http服务器IP
	std::string mHttpServerport;     //http服务器端口
	std::string mFtpServerIP;        //ftp服务器IP
	std::string mFtpServerPort;      //ftp服务器端口

	//不同类型默认dstcode和srccode
	std::map<eDVBType,std::string> mDefDstCode;
	std::map<eDVBType,std::string> mDefSrcCode;

	//设备任务信息
	std::map<int,sDeviceInfo> mDeviceInfo;
	std::map<std::string,std::list<int> > mTaskInfo;
	std::map<std::string,bool> mTaskShare;

	//其他
	long xmlSendTimes;         //xml文件发送次数
	long xmlOvertime;          //xml文件发送超时时间[单位:秒]

	std::map<std::string,sCenterInfo*> centerInfoMap;	//中心信息
	std::map<eDVBType,std::string> mXmlPath;			//命令接收路径
	std::map<eDVBType,std::string>mXmlTablePath;        //表信息接收路径
	//add by jidushan 11.03.31
	std::map<eDVBType,std::string>mTableSharePath;		//表共享路径map

	//wz_110309
	std::map<eDVBType, sVideoProcotol>mVideoProtocolMap;	//视频服务协议map
	std::map<eDVBType, sRecordDownInfo>mRecordDownMap;		//文件下载信息map

	//psisi第一次完成标识
	std::string mDVBCPsiSiValue;
	std::string mCTTBPsiSiValue;

	std::map<eDVBType,QualityCard> mQualityCard;//指标卡信息

	//CAS SMS地址信息
	std::string ShareDir;//本地共享casms文件的目录
	std::string HttpPath;//Http服务器目录别名
	std::string CASIPAddr;
	std::string CASFilePath;
	std::string SMSIPAddr;
	std::string SMSRequestURL;
	int CASPort;
	int SMSPort;

	//****************************************************************************
	string m_alarmrectvdeciceid;//报警录制通道,不能做其他录制
	string m_alarmrecradiodeciceid;//报警录制通道,不能做其他录制
	string m_alarmrecexpiredays;//报警录制文件过期天数,单位为天
	string m_alarmrecreclength;//报警录制文件长 单位为秒
	string m_radiodevalarmunittime;//广播调制度,单元测试时间 单位为秒
	string m_radiodevalarmrate;//单元测试时间,报警不正常率
	string m_radiodevalarmunalarmrate;//单元测试时间,解除报警正常率
	//****************************************************************************


	std::map<eDVBType,std::string> mRealTimeFromRecord; //实时视频是否从录像通道获取 0：否，1：是
	std::list<int> RecordList; //保存录像通道
	//频率扫描信息
	std::map<eDVBType,sFreqScanInfo> mFreqScanInfoMap;
	std::map<eDVBType,OSDInfo> mOSDInfoMap;
	//不同监测类型的协议
	//AlarmID
	static long AlarmID;
	
	std::map< enumDVBType, int > mVirtualDevInfo;	//轮播虚拟通道信息  wz_0217
	// 报警主动上报
	std::map<enumDVBType,UpAlarmInfo> mUpAlarmInfo;
	std::map<enumDVBType,std::string> mScanFile;
	//临时文件保存相关	add by jidushan 11.03.31
	std::string mTempFileLocation;		//临时文件保存地址
	std::string mTempSharePath;			//临时文件共享路径
	std::string mTempFileExpire;		//临时文件过期时间

	std::string mStoreType;			//录像文件存储类型：分散还是顺序		add by jidushan 11.05.04
	std::map<std::string, std::list<int>> mDeviceIdToPath;		//通道号与录像文件存放path关联map  add by jidushan 11.05.04

	std::map<enumDVBType,VideoParam> mHDRealStreamVideoPram;
	std::map<enumDVBType,VideoParam> mHDRecordVideoPram;
	std::map<enumDVBType,VideoParam> mSDRealStreamVideoPram;
	std::map<enumDVBType,VideoParam> mSDRecordVideoPram;
	
	std::map<enumDVBType, sAlarmRecordStorageCfg> mAlarmRecStorageCfgMap;
	std::vector<sDeviceInfo> mMonitorDevInfo;

    std::string IsSpecTrumFlag;//是否通过中间件给频谱仪发指令；  0,1,2:本机直接通信仪器   其它通过中间件
    std::string SpecTrumRet;
};
typedef ACE_Singleton <PropManager, ACE_Thread_Mutex>  PROPMANAGER;
#endif