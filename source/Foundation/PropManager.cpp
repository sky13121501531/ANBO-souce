#include <iostream>
#include "PropManager.h"
#include "StrUtil.h"
#include "../Foundation/OSFunction.h"
#include "../DBAccess/DBManager.h"
long PropManager::AlarmID=time(0);

PropManager::PropManager(void)
{
	InitiateBaseInfo();
	//InitInfoFromDB();
	InitiateDeviceInfo();
}

PropManager::~PropManager(void)
{
	std::map<std::string,sCenterInfo*>::iterator iter = centerInfoMap.begin();
	for (; iter!=centerInfoMap.end(); ++iter)
	{
		delete iter->second;
		iter->second = NULL;
	}

	centerInfoMap.clear();

	if (mConfig != NULL)
	{
		delete mConfig;
		mConfig = NULL;
	}
}

void PropManager::InitiateBaseInfo()
{
	mConfig=new PropConfig("c:/vscttb/properties.xml");
	if (!mConfig->isInitiated())
	{
		OSFunction::ExitProcess("系统初始化(配置文件)失败");
	}
	
	//数据库
	mDbType=mConfig->GetNodeAttribute("db","dbsource","type");
	if (mDbType.length()==0)
		mDbType="";

	mDbIp=mConfig->GetNodeAttribute("db","dbsource","ip");
	if (mDbIp.length()==0)
		mDbIp="127.0.0.1";

	mDbPort=mConfig->GetNodeAttribute("db","dbsource","port");
	if (mDbPort.length()==0)
		mDbPort="";

	mDbUsername=mConfig->GetNodeAttribute("db","dbsource","username");
	if (mDbUsername.length()==0)
		mDbUsername="";

	mDbPwd=mConfig->GetNodeAttribute("db","dbsource","pwd");

	mDbName=mConfig->GetNodeAttribute("db","dbsource","dbname");
	if (mDbName.length()==0)
		mDbName="vsdvb";

	//CAS SMS信息
	CASIPAddr=mConfig->GetNodeAttribute("CASSMS","CAS","IP");
	if (CASIPAddr.length()==0)
		CASIPAddr="127.0.0.1";
	CASFilePath=mConfig->GetNodeAttribute("CASSMS","CAS","path");
	if (CASFilePath.length()==0)
		CASFilePath="/";
	SMSRequestURL=mConfig->GetNodeAttribute("CASSMS","SMS","URL");
	if(SMSRequestURL.length()==0)
		SMSRequestURL="";
	SMSIPAddr=mConfig->GetNodeAttribute("CASSMS","SMS","IP");
	if (SMSIPAddr.length()==0)
		SMSIPAddr="127.0.0.1";
	CASPort=StrUtil::Str2Int(mConfig->GetNodeAttribute("CASSMS","CAS","port"));
	SMSPort=StrUtil::Str2Int(mConfig->GetNodeAttribute("CASSMS","SMS","port"));
	ShareDir=mConfig->GetNodeAttribute("CASSMS","share","directory");
	if(ShareDir.length()==0)
		ShareDir="c:/vscttb/cassms";
	HttpPath=mConfig->GetNodeAttribute("CASSMS","share","httppath");
	if(HttpPath.length()==0)
		HttpPath="/cassms/";
	//********************
	m_alarmrectvdeciceid=mConfig->GetNodeAttribute("alarmrec","recinfo","tvdeciceid");
	if(m_alarmrectvdeciceid.length()==0)
	{
		m_alarmrectvdeciceid="8";
	}
	m_alarmrecradiodeciceid=mConfig->GetNodeAttribute("alarmrec","recinfo","radiodeciceid");
	if(m_alarmrecradiodeciceid.length()==0)
	{
		m_alarmrecradiodeciceid="16";
	}
	m_alarmrecexpiredays=mConfig->GetNodeAttribute("alarmrec","recinfo","expiredays");
	if(m_alarmrecexpiredays.length()==0)
	{
		m_alarmrecexpiredays="2";
	}
	m_alarmrecreclength=mConfig->GetNodeAttribute("alarmrec","recinfo","reclength");
	if(m_alarmrecreclength.length()==0)
	{
		m_alarmrecreclength="30";
	}
	m_radiodevalarmunittime=mConfig->GetNodeAttribute("radiodevalarm","paraminfo","unittime");
	if(m_radiodevalarmunittime.length()==0)
	{
		m_radiodevalarmunittime="5";
	}
	m_radiodevalarmrate=mConfig->GetNodeAttribute("radiodevalarm","paraminfo","alarmrate");
	if(m_radiodevalarmrate.length()==0)
	{
		m_radiodevalarmrate="0.9";
	}
	m_radiodevalarmunalarmrate=mConfig->GetNodeAttribute("radiodevalarm","paraminfo","unalarmrate");
	if(m_radiodevalarmunalarmrate.length()==0)
	{
		m_radiodevalarmunalarmrate="0.1";
	}
	mNewFrAlarmErrNewFreq=mConfig->GetNodeAttribute("NewFreqAlarm","param","errnewfreq");//同时用于频道扫描结果的剔除
	mNewFrAlarmOkNewFreq=mConfig->GetNodeAttribute("NewFreqAlarm","param","oknewfreq");//同时用于频道扫描结果的添加 程序直接添加不判断是否存在 如果已存在将会有两个相同结果
	mScanlimitlevel=mConfig->GetNodeAttribute("NewFreqAlarm","param","scanlimitlevel");
	if(mScanlimitlevel.size()<=0)
	{
		mScanlimitlevel = "18";
	}
	mScandifflowval=mConfig->GetNodeAttribute("NewFreqAlarm","param","scandifflowval");
	if(mScandifflowval.size()<=0)
	{
		mScandifflowval = "10";
	}
	//********************
	mVirtualDevInfo.insert(make_pair(CTTB, 1001));
	mVirtualDevInfo.insert(make_pair(DVBC, 1003));
	mVirtualDevInfo.insert(make_pair(ATV, 1005));
	mVirtualDevInfo.insert(make_pair(DVBS, 1007));
	mVirtualDevInfo.insert(make_pair(CTV, 1008));
	mVirtualDevInfo.insert(make_pair(THREED, 1009));
}


bool PropManager::InitInfoFromDB()
{
	string centerfreq,symbolrate,qam;
	sSysConfigParam SysConfig;
	vector<sDvbConfigParam> VecConfig;
	DBMANAGER::instance()->QuerySystemConfig(UNKNOWN,SysConfig);//查询系统配置

	//服务器配置 IP，端口
	mHttpVideoIP = SysConfig.VideoHttpServerIp;
	if(mHttpVideoIP.length() == 0)
	mHttpVideoIP = "127.0.0.1";

	mHttpVideoPort = SysConfig.VideoHttpPort;
	if(mHttpVideoPort.length() == 0)
		mHttpVideoPort = "5050";
	
	mVideoMaxnum = SysConfig.VideoHttpMaxnum;
	if(mVideoMaxnum.length() == 0)
		mVideoMaxnum = "10";

	mRtspVideoIP = SysConfig.VideoRtspServerIp;
	if(mRtspVideoIP.length() == 0)
		mRtspVideoIP = "127.0.0.1";

	mRtspVideoPort = SysConfig.VideoRtspPort;
	if(mRtspVideoPort.length() == 0)
		mRtspVideoPort = "554";

	mQualityIp = SysConfig.QualityServerIp;
	if(mQualityIp.length() == 0)
		mQualityIp = "127.0.0.1";
	
	mQualityPort = SysConfig.QualityPort;
	if (mQualityPort.length() == 0)
		mQualityPort = "5051";
	
	mQualityMaxnum = SysConfig.QualityMaxnum;
	if (mQualityMaxnum.length() == 0)
		mQualityMaxnum = "10";

	mRecordQualityIp = SysConfig.RecordQualityIp;
	if (mRecordQualityIp.length() ==0)
		mRecordQualityIp = "127.0.0.1";


	mRecordQualityPort = SysConfig.RecordQualityPort;
	if (mRecordQualityPort.length() ==0)
		mRecordQualityPort = "5052";

	mRecordQualityMaxnum = SysConfig.RecordQualityMaxnum;
	if (mRecordQualityMaxnum.length()==0)
		mRecordQualityMaxnum="10";

	mDeviceIp = SysConfig.DeviceServerIp;
	if (mDeviceIp.length() == 0)
		mDeviceIp = "127.0.0.1";

	mDevicePort = SysConfig.DevicePort;
	if (mDevicePort.length() == 0)
		mDevicePort = "5052";

	mDeviceMaxnum = SysConfig.DeviceMaxnum;
	if (mDeviceMaxnum.length() == 0)
		mDeviceMaxnum = "10";

	mXmlServerIP = SysConfig.XmlServerIp;
	if (mXmlServerIP.length() == 0)
		mXmlServerIP = "127.0.0.1";

	mXmlServerPort = SysConfig.XmlPort;
	if (mXmlServerPort.length() == 0)
		mXmlServerPort = "5053";


	mLogIp = SysConfig.LogServerIp;
	if (mLogIp.length() == 0)
		mLogIp = "127.0.0.1";

	mLogPort = SysConfig.LogPort;
	if (mLogPort.length() == 0)
		mLogPort = "5054";

	mLogMaxnum = SysConfig.LogMaxnum;
	if (mLogMaxnum.length() == 0)
		mLogMaxnum = "10";

	mHttpServerIP = SysConfig.HttpServerIp;
	if(mHttpServerIP.length() == 0)
		mHttpServerIP = "127.0.0.1";

	mHttpServerport = SysConfig.HttpPort;
	if(mHttpServerport.length() == 0)
		mHttpServerport = "80";

	mFtpServerIP = SysConfig.FtpServerIp;
	if(mFtpServerIP.length() == 0)
		mFtpServerIP = "127.0.0.1";

	mFtpServerPort = SysConfig.FtpPort;
	if(mFtpServerPort.length() == 0)
		mFtpServerPort = "25";

	//日志信息
	mLogPath = SysConfig.Log_Path;
	if (mLogPath.length() == 0)
		mLogPath = "c:/vscttb/logs";

	mLogExpire = SysConfig.Log_Expire;
	if (mLogExpire.length() == 0)
		mLogExpire = "7";

	mLogType = SysConfig.Log_Type;
	if (mLogType.length() == 0)
		mLogType = "debug";

	mLogOutputFile = SysConfig.Log_outputToFile;
	if (mLogOutputFile.length() == 0)
		mLogOutputFile="";

	mLogAnalyser = SysConfig.LogPathType_Analyser;
	if (mLogAnalyser.length() == 0)
		mLogAnalyser = "analyser";

	mLogVideo  = SysConfig.LogPathType_Video;
	if (mLogVideo.length() == 0)
		mLogVideo = "video";

	mLogRecord = SysConfig.LogPathType_Record;
	if (mLogRecord.length() == 0)
		mLogRecord = "record";

	mLogDevice = SysConfig.LogPathType_Device;
	if (mLogDevice.length() == 0)
		mLogDevice = "device";

	mLogOther = SysConfig.LogPathType_Other;
	if (mLogOther.length() == 0)
		mLogOther = "other";

	mLogDefault = SysConfig.LogPathType_Default;
	if (mLogDefault.length() == 0)
		mLogDefault = "default";


	//录像存储路径
	string temploclist,tempsharelist,devicelist;
	temploclist = SysConfig.Record_FileLocation;
	if (temploclist.length() == 0)
		temploclist = "d:/tsrecord/";

	tempsharelist = SysConfig.Record_FileSharePath;
	if (tempsharelist.length() == 0)
		tempsharelist = "/dtsrecord/";

	SepLocShareAndInit(temploclist, tempsharelist, mRecLocToShare, mRecFileLocVec);

	devicelist = SysConfig.Record_DeviceId;
	if(devicelist.length() == 0)
		devicelist = "1-10";

	SeparateRecordDir(devicelist);

	mRecordPeriod = SysConfig.Record_Period;
	if(mRecordPeriod.length() == 0)
		mRecordPeriod = "5";

	mMaxAvailableSize = SysConfig.Record_MaxAvailableSize;
	if (mMaxAvailableSize.length() == 0)
		mMaxAvailableSize = "2";

	mDBCleanInterval = SysConfig.Record_DBCleanInterval;
	if (mDBCleanInterval.length() == 0)
		mDBCleanInterval = "30";

	mSystemCleanTime = SysConfig.Record_SystemCleanTime;
	if(mSystemCleanTime.length() == 0)
		mSystemCleanTime = "00:00:00";

	mStoreType = SysConfig.Record_StoreType;
	if (mStoreType == "")
		mStoreType = "0";


	xmlSendTimes = StrUtil::Str2Long(SysConfig.XmlSendTime);
	if(xmlSendTimes == 0)
		xmlSendTimes = 3;
	xmlOvertime = StrUtil::Str2Long(SysConfig.XmlOverTime);
	if(xmlOvertime == 0)
		xmlOvertime = 3;


	mTempFileLocation = SysConfig.TempFileLocation;
	if (mTempFileLocation.length() == 0)
		mTempFileLocation = "c:/vscttb/temp/";

	mTempSharePath	  = SysConfig.TempSharePath;
	if (mTempSharePath.length() == 0)
		mTempSharePath = "/vstemp/";

	mTempFileExpire   = SysConfig.TempFileExpireTime;
	if (mTempFileExpire.length() == 0)
		mTempFileExpire = "1";

	mRecLocToShare.insert(make_pair(mTempFileLocation,mTempSharePath));

	DeviceManagetype = SysConfig.DeviceSchedulerTask_Type;
	if(DeviceManagetype.length() == 0)
		DeviceManagetype = "week";

	DeviceManageweekday = SysConfig.DeviceSchedulerTask_WeekDay;
	if(DeviceManageweekday.length() == 0)
		DeviceManageweekday = "2";

	DeviceManagesingleday = SysConfig.DeviceSchedulerTask_Date;
	if(DeviceManagesingleday.length() == 0)
		DeviceManagesingleday ="";

	DeviceManagetime = SysConfig.DeviceSchedulerTask_Time;
	if(DeviceManagetime.length() == 0)
		DeviceManagetime = "00:00:00";

	PsisiManagetype = SysConfig.PsisiSchedulerTask_Type;
	if(PsisiManagetype.length() == 0)
		PsisiManagetype = "week";

	PsisiManageweekday = SysConfig.PsisiSchedulerTask_WeekDay;
	if(PsisiManageweekday.length() == 0)
		PsisiManageweekday = "2";
	PsisiManagesingleday = SysConfig.PsisiSchedulerTask_Date;
	if(PsisiManagesingleday.length() == 0)
		PsisiManagesingleday ="";

	PsisiManagetime = SysConfig.PsisiSchedulerTask_Time;
	if(PsisiManagetime.length() == 0)
		PsisiManagetime = "00:00:00";

	
	DBMANAGER::instance()->QueryDvbConfig(DVBC,VecConfig);
	DBMANAGER::instance()->QueryDvbConfig(CTTB,VecConfig);
	DBMANAGER::instance()->QueryDvbConfig(ATV,VecConfig);
	DBMANAGER::instance()->QueryDvbConfig(CTV,VecConfig);
	DBMANAGER::instance()->QueryDvbConfig(AM,VecConfig);
	DBMANAGER::instance()->QueryDvbConfig(RADIO,VecConfig);
	DBMANAGER::instance()->QueryDvbConfig(DVBS,VecConfig);
	DBMANAGER::instance()->QueryDvbConfig(THREED,VecConfig);
	DBMANAGER::instance()->SetAlarmLevelThreshold(ATV,string("12323431"),string("100"));
	for(int i=0;i<VecConfig.size();i++)
	{
		sFreqScanInfo freqscaninfo;

		freqscaninfo.CenterFreq = VecConfig[i].CenterFreq;
		freqscaninfo.QAM = VecConfig[i].Qam;
		freqscaninfo.SymbolRate = VecConfig[i].Symbolrate;

		mFreqScanInfoMap.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),freqscaninfo));

		mScanFile.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),VecConfig[i].ChannelScanType));

		mRealTimeFromRecord.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),VecConfig[i].RealTimeFromRecord));
		OSDInfo osdinfo;

		osdinfo.FontSize = VecConfig[i].Osd_fontsize;
		osdinfo.Info = VecConfig[i].Osd_infoosd;
		osdinfo.InfoX = VecConfig[i].Osd_infoosdx;
		osdinfo.InfoY = VecConfig[i].Osd_infoosdy;
		osdinfo.TimeType = VecConfig[i].Osd_timeosdtype;
		osdinfo.TimeX = VecConfig[i].Osd_timeosdx;
		osdinfo.TimeY = VecConfig[i].Osd_timeosdy;
		osdinfo.ProgramX = VecConfig[i].Osd_programx;
		osdinfo.ProgramY = VecConfig[i].Osd_programy;
		mOSDInfoMap.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),osdinfo));

		mXmlTablePath.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),VecConfig[i].TablePath));
		mTableSharePath.insert( make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),VecConfig[i].SharePath) );

		VideoParam  HDRecordparam;
		VideoParam  SDRecordparam;
		VideoParam  HDRealStreamparam;
		VideoParam  SDRealStreamparam;

		HDRecordparam.width = VecConfig[i].HDRecordWidth;
		HDRecordparam.height = VecConfig[i].HDRecordHeight;
		HDRecordparam.bps   =  VecConfig[i].HDRecordBps;

		HDRealStreamparam.width = VecConfig[i].HDRealStreamWidth;
		HDRealStreamparam.height = VecConfig[i].HDRealStreamHeight;
		HDRealStreamparam.bps    = VecConfig[i].HDRealStreamBps;

		SDRecordparam.width  = VecConfig[i].SDRecordWidth;
		SDRecordparam.height = VecConfig[i].SDRecordHeight;
		SDRecordparam.bps    = VecConfig[i].SDRecordBps;

		SDRealStreamparam.width = VecConfig[i].SDRealStreamWidth;
		SDRealStreamparam.height = VecConfig[i].SDRealStreamHeight;
		SDRealStreamparam.bps    = VecConfig[i].SDRealStreamBps;

		mHDRecordVideoPram.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),HDRecordparam));
		mSDRecordVideoPram.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),SDRecordparam));
		mHDRealStreamVideoPram.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),HDRealStreamparam));
		mSDRealStreamVideoPram.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),SDRealStreamparam));
		
		UpAlarmInfo alarminfo;
	
		alarminfo.type = VecConfig[i].AlarmType;
		alarminfo.interval = VecConfig[i].AlarmInterval;
		alarminfo.onceday = VecConfig[i].OnceDay;
		alarminfo.alarmurl = VecConfig[i].AlarmServer;
		alarminfo.runplanrecord = VecConfig[i].RunplanRecord;
		mUpAlarmInfo.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),alarminfo));

		sVideoProcotol VideoProtocol;
		
		VideoProtocol.streamprotocol = VecConfig[i].VideoStreamProtocol;
		VideoProtocol.fileprotocol = VecConfig[i].VideoFileProtocol;
		VideoProtocol.fileurltype = VecConfig[i].VideoFileUrlType;
		VideoProtocol.fileoffset = VecConfig[i].VideoFileOffSet;
		mVideoProtocolMap.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type), VideoProtocol));				

		sRecordParamInfo tempRecParam;

		tempRecParam.recordexpire = VecConfig[i].RecordParam_recordexpire;
		tempRecParam.alarmrecordexpire = VecConfig[i].RecordParam_alarmrecordexpire;
		tempRecParam.alarmheadoffset = VecConfig[i].RecordParam_alarmheadoffset;
		tempRecParam.alarmtailoffset = VecConfig[i].RecordParam_alarmtailoffset;
		tempRecParam.alarmrecordmode = VecConfig[i].RecordParam_alarmrecordmode;

		mRecParamInfo.insert( make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),tempRecParam) );

		sRecordDownInfo recdowninfo;
		
		recdowninfo.protocol = VecConfig[i].RecordDown_protocol;
		recdowninfo.urltype = VecConfig[i].RecordDown_urltype;
		recdowninfo.offset = VecConfig[i].RecordDown_offset;

		mRecordDownMap.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),recdowninfo));


		mDefDstCode.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),VecConfig[i].GeneralDestCode));
		mDefSrcCode.insert(make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type),VecConfig[i].GeneralSrcCode));
	
		//获取异态录像的相关存储信息(开关、文件个数)
		sAlarmRecordStorageCfg AlarmRecCfgInfo;
		AlarmRecCfgInfo._switch = VecConfig[i].AlarmStorage_switch;
		AlarmRecCfgInfo._filenum = VecConfig[i].AlarmStorage_filenum;
		mAlarmRecStorageCfgMap.insert( make_pair(OSFunction::GetEnumDVBType(VecConfig[i].type), AlarmRecCfgInfo) );
	}

	return true;
}
bool PropManager::InitiateDeviceInfo()
{
	XmlParser mXmlProp;
	std::string nodePath;
	std::string nodeList[3] = {"device","center","task"}; 
	mXmlProp.LoadFromFile("c:/vscttb/properties.xml");

	for (int j = 0;j<3;j++)
	{
		nodePath="properties/"+nodeList[j];
		pXMLNODE node=mXmlProp.GetNodeFromPath((char *)nodePath.c_str());
		if(node==NULL)
			return false;
		pXMLNODELIST nodeList=mXmlProp.GetNodeList(node);
		for (int i=0;i<nodeList->Size();i++)
		{
			pXMLNODE nodeNext=mXmlProp.GetNextNode(nodeList);
			switch ( j )
			{
			case 0:           //获取device节点相关信息
				{
					int deviceID;
					string strdeviceid,devicetype,BaseIP,IP,cmdport,tstype, tsip, tsport,cmdprotocol,tsprotocol,tunerid,logindex,recordlist;
					mXmlProp.GetAttrNode(nodeNext,"index",strdeviceid);
					mXmlProp.GetAttrNode(nodeNext,"logindex",logindex);
					mXmlProp.GetAttrNode(nodeNext,"devicetype",devicetype);
					mXmlProp.GetAttrNode(nodeNext,"IP",IP);
					mXmlProp.GetAttrNode(nodeNext,"baseIP",BaseIP);
					mXmlProp.GetAttrNode(nodeNext,"cmdport",cmdport);
					mXmlProp.GetAttrNode(nodeNext,"tstype",tstype);
					mXmlProp.GetAttrNode(nodeNext,"tsip",tsip);
					mXmlProp.GetAttrNode(nodeNext,"tsport",tsport);
					mXmlProp.GetAttrNode(nodeNext,"cmdprotocol",cmdprotocol);
					mXmlProp.GetAttrNode(nodeNext,"tsprotocol",tsprotocol);
					mXmlProp.GetAttrNode(nodeNext,"tunerid",tunerid);
					mXmlProp.GetAttrNode(nodeNext,"recordlist",recordlist);
                    mXmlProp.GetAttrNode(nodeNext,"isSpectrum",IsSpecTrumFlag);
                    mXmlProp.GetAttrNode(nodeNext,"SpectrumRetPlus",SpecTrumRet);
					mXmlProp.GetAttrNode(nodeNext,"TVAudioPower",TVAUDIOPower);
					mXmlProp.GetAttrNode(nodeNext,"FMAudioPower",FMAUDIOPower);
					mXmlProp.GetAttrNode(nodeNext,"SignalThreshold",mSignalThreshold);
					std::vector<int> VecIndex,VecLogindex,VecRecordList,VecTsportList,VecTuneridList;
					SeparateStrVec(strdeviceid, VecIndex);
					SeparateStrVec(logindex, VecLogindex);
					SeparateStrVec(recordlist,VecRecordList);
					SeparateStrVec(tsport,VecTsportList);
					SeparateStrVec(tunerid,VecTuneridList);

					sDeviceInfo addressinfo;			//获得设备结构体信息									
					addressinfo.devicetype=devicetype;
					addressinfo.IP=IP;
					addressinfo.cmdport=cmdport;				
					addressinfo.cmdprotocol=cmdprotocol;
					addressinfo.tsprotocol = tsprotocol;					
					addressinfo.baseIP = BaseIP;
					addressinfo.tstype = tstype;
					addressinfo.tsip = tsip;
					addressinfo.tsport = "";
					for(int i=0;i<VecIndex.size();i++)
					{
						deviceID = VecIndex[i];	
						if (VecLogindex.size() > 0)
						{
							addressinfo.logindex = StrUtil::Int2Str(VecLogindex[i]) ;
						}
 						if (VecTsportList.size() > 0)
 						{
 							addressinfo.tsport = StrUtil::Int2Str(VecTsportList[i]) ;
 						}
						if (VecTuneridList.size() > 0)
						{
							addressinfo.tunerid = StrUtil::Int2Str(VecTuneridList[i]) ;
						}
						if (VecRecordList.size() > 0)
						{
							addressinfo.recordlist = StrUtil::Int2Str(VecRecordList[i]) ;
						}							
						mDeviceInfo[deviceID] = addressinfo;
					}
					for(int k=0;k<VecRecordList.size();k++)
					{
						RecordList.push_back(VecRecordList[k]);
					}
                    if(IsSpecTrumFlag=="")
                    {
                        mMonitorDevInfo.push_back(addressinfo);
                    }
					break;
				}
			case 1:          //获取监测中心相关信息 
				{
					string code,url,type;

					mXmlProp.GetAttrNode(nodeNext,string("type"),type);
					mXmlProp.GetAttrNode(nodeNext,string("url"),url);
					mXmlProp.GetAttrNode(nodeNext,string("srccode"),code);

					sCenterInfo *pCenterInfo = new sCenterInfo;
					pCenterInfo->centerUrl = url;
					pCenterInfo->centerType = type;

					centerInfoMap.insert(make_pair(code,pCenterInfo));
					break;
				}
			case 2:           //获取task节点相关信息
				{
					string tasktype,devicelist,deviceshare;

					mXmlProp.GetAttrNode(nodeNext,"tasktype",tasktype);
					mXmlProp.GetAttrNode(nodeNext,"devicelist",devicelist);
					mXmlProp.GetAttrNode(nodeNext,"deviceshare",deviceshare);

					std::list<int> deviceidlist;
					SeparateStr(devicelist,deviceidlist);
					DeleteWrongDeviceID(deviceidlist);
					if (deviceidlist.empty() == false)
						mTaskInfo.insert(make_pair(tasktype,deviceidlist));
					if(deviceshare=="yes")
						mTaskShare.insert(make_pair(tasktype,true));
					else
						mTaskShare.insert(make_pair(tasktype,false));

					if(tasktype=="DvbLevelTask")
					{
						for (std::list<int>::iterator pList=deviceidlist.begin();pList!=deviceidlist.end();pList++)
						{
							int port=8888;
							QualityCard temp;
							enumDVBType type = UNKNOWN;
							GetDeviceIP((*pList),temp.IP);
							GetDeviceCmdPort((*pList),port);
							GetDeviceType((*pList),type);
							temp.Port=StrUtil::Int2Str(port);
							mQualityCard.insert(make_pair(type,temp));
						}
						
					}
					break;
				}

			default:
				break;
			}
		}
	}

// 	if (RecordList.empty() == false)
// 	{
// 		mTaskInfo.erase(string("AutoRecord"));
// 		mTaskInfo.insert(make_pair(string("AutoRecord"),RecordList));
// 		mTaskInfo.erase(string("TaskRecord"));
// 		mTaskInfo.insert(make_pair(string("TaskRecord"),RecordList));
// 	}
	return true;
}
//日志
eLogType PropManager::GetLogType( void )
{
	if (mLogType=="debug")
		return LOG_EVENT_DEBUG;
	else if (mLogType=="warnning")
		return LOG_EVENT_WARNNING;
	else if (mLogType=="error")
		return LOG_EVENT_ERROR;
	else
		return LOG_EVENT_DEBUG;
}

eLogOutPut PropManager::GetOutputFile( void )
{
	if (mLogOutputFile=="screen")
		return LOG_OUTPUT_SCREEN;
	else if (mLogOutputFile=="file")
		return LOG_OUTPUT_FILE;
	else if (mLogOutputFile=="both")
		return LOG_OUTPUT_BOTH;
	else
		return LOG_OUTPUT_UNDEFINE;
}

//硬件相关信息查询
bool PropManager::GetAllDeviceList( std::list<int>& devicedlist )
{
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		devicedlist.push_back((*pDeviceinfo).first);
	}
	return true;
}
bool PropManager::GetDVBDeviceList(eDVBType dvbtype,std::list<int>& devicedlist)
{
	string devicetype = OSFunction::GetStrDVBType(dvbtype);

	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if (devicetype == pDeviceinfo->second.devicetype)
		{
			devicedlist.push_back((*pDeviceinfo).first);
		}
	}
	return true;
}

bool PropManager::GetTaskDeviceList( string taskType,eDVBType dvbtype,std::list<int>& taskdeviceidlist )
{
	bool ret=true;
	string devicetype = OSFunction::GetStrDVBType(dvbtype);

	for (std::map<std::string,std::list<int> >::iterator pTaskinfo=mTaskInfo.begin();pTaskinfo!=mTaskInfo.end();pTaskinfo++)
	{ 
		if ((*pTaskinfo).first == taskType)
		{
			std::list<int>::iterator ptr = (*pTaskinfo).second.begin();
			if ((*pTaskinfo).second.size()>1)
			{
				for (;ptr!=(*pTaskinfo).second.end();ptr++)
				{
					std::map<int,sDeviceInfo>::iterator in_ptr = mDeviceInfo.begin();
					for (;in_ptr!=mDeviceInfo.end();in_ptr++)
					{
						if (( (*ptr) ==(*in_ptr).first )&&(devicetype ==(*in_ptr).second.devicetype))   //通道号相同且devicetype相同
						{
							taskdeviceidlist.push_back(*ptr);
						}
					}
				}
			}
			else if ((*pTaskinfo).second.size() == 1)
			{
				std::map<int,sDeviceInfo>::iterator in_ptr = mDeviceInfo.begin();
				for (;in_ptr!=mDeviceInfo.end();in_ptr++)
				{
					if (( (*ptr) ==(*in_ptr).first )&&(devicetype ==(*in_ptr).second.devicetype))
					{
						taskdeviceidlist.push_back(*ptr);
					}
				}

			}
		}
	}
	return ret;
}

bool PropManager::GetDeviceAddress( int deviceid,string& ip,int& cmdport,int &tsport,string &cmdprotocol,string &tsprotocol )
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			ip=(*pDeviceinfo).second.IP;
			//ip=(*pDeviceinfo).second.baseIP;
			cmdport=StrUtil::Str2Int((*pDeviceinfo).second.cmdport);
			tsport =StrUtil::Str2Int((*pDeviceinfo).second.tsport);
			cmdprotocol = (*pDeviceinfo).second.cmdprotocol;
			tsprotocol = (*pDeviceinfo).second.tsprotocol;
			break;
		}
	}
	return ret;
}

bool PropManager::GetDeviceIP(int deviceid,string& ip)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			ip=(*pDeviceinfo).second.IP;
			break;
		}
	}
	return ret;
}

bool PropManager::GetDeviceIPandPORT(int deviceid,string& ip,string& port)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			ip=(*pDeviceinfo).second.IP;
			port = (*pDeviceinfo).second.cmdport;
			break;
		}
	}
	return ret;
}

bool PropManager::GetDeviceCmdPort(int deviceid,int& cmdport)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			cmdport=StrUtil::Str2Int((*pDeviceinfo).second.cmdport);
			break;
		}
	}
	return ret;
}
bool PropManager::GetDeviceCmdProtocol(int deviceid,std::string& cmdprotocol)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			cmdprotocol =(*pDeviceinfo).second.cmdprotocol;
			break;
		}
	}
	return ret;
}

bool PropManager::GetDeviceTsIP(int deviceid,std::string& strTsIP)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			strTsIP = (*pDeviceinfo).second.tsip;
			break;
		}
	}
	return ret;
}

bool PropManager::GetDeviceTsPort(int deviceid,std::string& strtsport)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			strtsport = (*pDeviceinfo).second.tsport;
			break;
		}
	}
	return ret;
}


bool PropManager::GetDeviceTsPort(int deviceid,int& tsport)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			tsport =StrUtil::Str2Int((*pDeviceinfo).second.tsport);
			break;
		}
	}
	return ret;
}
bool PropManager::GetDeviceTunerID(int deviceid,int& tunerid)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			tunerid =StrUtil::Str2Int((*pDeviceinfo).second.tunerid);
			break;
		}
	}
	return ret;//DLL中做逻辑转换,上层设备ID和调频ID一致2012-5-23 hjw
	//tunerid = deviceid;
	//return true;
}
bool PropManager::GetDeviceLogIndex(int deviceid, int& logindex)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			logindex =StrUtil::Str2Int((*pDeviceinfo).second.logindex);
			break;
		}
	}
	return ret;
}
bool PropManager::GetDeviceIndex(string & deviceid,string logindex,string devicetype)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).second.logindex==logindex &&(*pDeviceinfo).second.devicetype==devicetype)
		{
			ret=true;
			deviceid=StrUtil::Int2Str((*pDeviceinfo).first);
			break;
		}
	}
	return ret;
}
bool PropManager::GetDeviceIndex(std::list<int>& devicedlist,string baseip)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).second.baseIP==baseip)
		{
			ret=true;
			devicedlist.push_back((*pDeviceinfo).first);
		}
	}
	return ret;
}

bool PropManager::GetDeviceTsProtocol(int deviceid,std::string& tsprotocol)
{
	bool ret=false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=true;
			tsprotocol = (*pDeviceinfo).second.tsprotocol;
			break;
		}
	}
	return ret;
}

bool PropManager::GetDeviceID(string ip,string tunerid,int & deviceid)
{
	bool ret = false;
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).second.IP==ip && (*pDeviceinfo).second.tunerid == tunerid)
		{
			ret=true;
			deviceid=(*pDeviceinfo).first;
			break;
		}
	}
	return ret;

}

bool PropManager::GetDeviceIDByIP(string ip,std::list<int> & devicedlist)
{
	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if (ip == pDeviceinfo->second.IP)
		{
			devicedlist.push_back((*pDeviceinfo).first);
		}
	}
	return true;
}

bool PropManager::IsDeviceAvaiable( int deviceid,string tasktype,eDVBType dvbtype )
{
	bool ret=false;
	string devicetype = OSFunction::GetStrDVBType(dvbtype);

	for (std::map<int,sDeviceInfo>::iterator pDeviceinfo=mDeviceInfo.begin();pDeviceinfo!=mDeviceInfo.end();pDeviceinfo++)
	{
		if ((*pDeviceinfo).first==deviceid)
		{
			ret=((*pDeviceinfo).second.devicetype==devicetype)&&(CheckDeviceInList(deviceid,mTaskInfo[tasktype]));
		}
	}
	return ret;
}
bool PropManager::IsDeviceAvaiable(int deviceid)
{ 
	std::map<int,sDeviceInfo>::iterator ptr = mDeviceInfo.begin();
	for (;ptr!=mDeviceInfo.end();++ptr)
	{
		if (deviceid == (*ptr).first)
		{
			return true;
		}
	}
	return false;
}
//根据通道号获得监测类型
bool PropManager::GetDeviceType(int deviceid,eDVBType& dvbtype)
{
	map<int,sDeviceInfo>::iterator ptr = mDeviceInfo.begin();
	for (;ptr!= mDeviceInfo.end();ptr++)
	{
		if ((*ptr).first == deviceid)
		{ 
			dvbtype = OSFunction::GetEnumDVBType((*ptr).second.devicetype);
			break;
		}
	}
	return true;
}
//中心URL查询
std::string PropManager::GetUrl(const std::string& srcCode)
{
	std::map<std::string,sCenterInfo*>::iterator iter = centerInfoMap.find(srcCode);

	if (iter != centerInfoMap.end())
		return iter->second->centerUrl;
	else
		return "";
}

//其他函数
bool PropManager::CheckDeviceInList( int deviceid,std::list<int> deviceList )
{
	for (std::list<int>::iterator pList=deviceList.begin();pList!=deviceList.end();pList++)
	{
		if ((*pList)==deviceid)
		{
			return true;
		}
	}
	return false;
}

bool PropManager::DeleteWrongDeviceID(std::list<int>& deviceIDList)     //从任务的通道组中去掉不合理的deviceid
{
	std::list<int>::iterator ptr = deviceIDList.begin();
	std::map<int ,sDeviceInfo>::iterator in_ptr;
	if (deviceIDList.size()>0)
	{  
		for (;ptr!=deviceIDList.end();)
		{ 
			in_ptr=mDeviceInfo.begin();
			for (;in_ptr!= mDeviceInfo.end();in_ptr++)
			{
				if ((*ptr) ==(*in_ptr).first)
				{
					break;
				}
			}
			if (in_ptr == mDeviceInfo.end())
			{
				ptr=deviceIDList.erase(ptr);
			}
			else
			{
				ptr++;
			}
		}
	}
	return true;
}

bool PropManager::SeparateStr( string src,list<int>& lis )//src要求：单独一个字符串没有分号；多个字符串之间以分号隔开且最后一个字符串后没有分号；
{
	char * context;
	char * id = strtok_s(const_cast<char*>(src.c_str()),";",&context);
	while(id!= NULL)
	{
		lis.push_back(atoi(id));
		id = strtok_s(NULL,";",&context);
	}
	return true;
}

bool PropManager::SeparateStrVec( string src,vector<int>& lis )//src要求：单独一个字符串没有分号；多个字符串之间以分号隔开且最后一个字符串后没有分号；
{
	char * context;
	char * id = strtok_s(const_cast<char*>(src.c_str()),";",&context);
	while(id!= NULL)
	{
		lis.push_back(atoi(id));
		id = strtok_s(NULL,";",&context);
	}
	return true;
}

bool PropManager::SeparateRecordDir( string src )//src要求：单独一个字符串没有分号；多个字符串之间以分号隔开且最后一个字符串后没有分号；
{

	vector<string> vec;
	char * context;
	char * id = strtok_s(const_cast<char*>(src.c_str()),";",&context);
	while(id!= NULL)
	{
		vec.push_back(id);
		id = strtok_s(NULL,";",&context);
	}

	for(int i=0;i<mRecFileLocVec.size();i++)
	{
		list<int> temp_list;
		char * str;
		char *head = strtok_s(const_cast<char*>(vec[i].c_str()),"-",&str);
		char *tail = strtok_s(NULL,"-",&str);
		for(int k=atoi(head);k<=atoi(tail);k++)
		{
			temp_list.push_back(k);
		}
		mDeviceIdToPath.insert( make_pair(mRecFileLocVec[i], temp_list) );
	}
	return true;
}


//add by jidushan 11.03.31
bool PropManager::SepLocShareAndInit(string srcloc, string srcshare, map<string ,string>& loctoshare,vector<string>& vecLoc)
{
	char* contextloc;
	char* contextshare;
	char* idloc = strtok_s(const_cast<char*>(srcloc.c_str()),";",&contextloc);
	char* idshare = strtok_s(const_cast<char*>(srcshare.c_str()),";",&contextshare);
	
	string loc,share;
	while (idloc != NULL && idshare != NULL)
	{
		loc = idloc;
		share = idshare;
		loctoshare.insert(make_pair(loc, share));
		vecLoc.push_back(loc);
		idloc = strtok_s(NULL, ";", &contextloc);
		idshare = strtok_s(NULL, ";", &contextshare);
	}

	return true;
}
bool PropManager::IsShareTask( std::string taskname )
{
	bool ret=false;

	std::map<std::string, bool>::iterator ptr=mTaskShare.find(taskname);
	if (ptr != mTaskShare.end())
	{
		ret = ptr->second;
	}
	return ret;
}

bool PropManager::SetPSISIInfo(eDVBType type, std::string text )
{
	if (type==CTTB)
	{
		mConfig->SetNodeText("psisi","cttb",text.c_str());
		mCTTBPsiSiValue=text;
	}
	else if(type==DVBC)
	{
		mConfig->SetNodeText("psisi","dvbc",text.c_str());
		mDVBCPsiSiValue=text;
	}
	else
		return false;
	return true;
}

bool PropManager::GetPSISIInfo( eDVBType type,std::string& valu )
{
	if(type==CTTB)
		valu=mCTTBPsiSiValue;
	else if(type==DVBC)
		valu=mDVBCPsiSiValue;
	else
		return false;
	return true;
}

bool PropManager::GetCASIP( std::string& CASIP )
{
	CASIP=CASIPAddr;
	return true;
}

bool PropManager::GetCASPort( int& port )
{
	port=CASPort;
	return true;
}

bool PropManager::GetSMSIP( std::string& SMSIP )
{
	SMSIP=SMSIPAddr;
	return true;
}

bool PropManager::GetSMSPort( int& port )
{
	port=SMSPort;
	return true;
}

bool PropManager::GetCASFilePath( std::string& path )
{
	path=CASFilePath;
	return true;
}

bool PropManager::GetSMSURL( std::string& url )
{
	url=SMSRequestURL;
	return true;
}

bool PropManager::GetShareDir( std::string& dir )
{
	dir=ShareDir;
	return true;
}
bool PropManager::GetHttpPath(std::string &path)
{
	path=HttpPath;
	return true;
}
bool PropManager::GetOSDInfo(eDVBType dvbtype,std::string hdtv,OSDInfo& info )
{
	info=mOSDInfoMap[dvbtype];
	OSFunction::SetOSDTimeMode(StrUtil::Str2Int(info.TimeType),info);

	if (hdtv == "1")
	{
		info.FontSize = StrUtil::Int2Str(24);
		info.InfoX = StrUtil::Int2Str(800);
		info.InfoY = StrUtil::Int2Str(480);
		info.ProgramX = StrUtil::Int2Str(20);
		info.ProgramY = StrUtil::Int2Str(20);
		info.TimeX = StrUtil::Int2Str(450);
		info.TimeY = StrUtil::Int2Str(20);
	}

	return true;
}

bool PropManager::SetOSDInfo(eDVBType dvbtype,OSDInfo info)
{
	OSDInfo  temposd;
	temposd=mOSDInfoMap[dvbtype];
	if(info.FontSize!="")
		temposd.FontSize=info.FontSize;
	if(info.InfoX!="")
		temposd.InfoX=info.InfoX;
	if(info.InfoY!="")
		temposd.InfoY=info.InfoY;
	if(info.ProgramX!="")
		temposd.ProgramX=info.ProgramX;
	if(info.ProgramY!="")
		temposd.ProgramY=info.ProgramY;
	if(info.TimeType!="")
		temposd.TimeType=info.TimeType;
	if(info.TimeX!="")
		temposd.TimeX=info.TimeX;
	if(info.TimeY!="")
		temposd.TimeY=info.TimeY;

	temposd.Info=info.Info;
	mOSDInfoMap[dvbtype]=temposd;
	string type;
	if(dvbtype==DVBC)
	{
		type="DVBC";
		if (info.TimeType == "0" || info.TimeType == "1" || info.TimeType == "3")
		{
			mOSDInfoMap[dvbtype].TimeType = "6";
		}
		else if (info.TimeType == "2")
		{
			mOSDInfoMap[dvbtype].TimeType = "8";
		}
	}
	else if(dvbtype==CTTB)
		type="CTTB";
	else if(dvbtype==ATV)
		type="ATV";
	else if(dvbtype==CTV)
		type="CTV";
	else if(dvbtype==DVBS)
	{
		type="DVBS";
		if (info.TimeType == "0" || info.TimeType == "1" || info.TimeType == "3")
		{
			mOSDInfoMap[dvbtype].TimeType = "6";
		}
		else if (info.TimeType == "2")
		{
			mOSDInfoMap[dvbtype].TimeType = "8";
		}
	}
	if(dvbtype==THREED)
	{
		type="THREED";
		if (info.TimeType == "0" || info.TimeType == "1" || info.TimeType == "3")
		{
			mOSDInfoMap[dvbtype].TimeType = "6";
		}
		else if (info.TimeType == "2")
		{
			mOSDInfoMap[dvbtype].TimeType = "8";
		}
	}

	mConfig->SetNodeAttribute(type,"osd","infoosd",mOSDInfoMap[dvbtype].Info.c_str());
	mConfig->SetNodeAttribute(type,"osd","infoosdx",mOSDInfoMap[dvbtype].InfoX.c_str());
	mConfig->SetNodeAttribute(type,"osd","infoosdy",mOSDInfoMap[dvbtype].InfoY.c_str());
	mConfig->SetNodeAttribute(type,"osd","timeosdtype",mOSDInfoMap[dvbtype].TimeType.c_str());
	mConfig->SetNodeAttribute(type,"osd","timeosdx",mOSDInfoMap[dvbtype].TimeX.c_str());
	mConfig->SetNodeAttribute(type,"osd","timeosdy",mOSDInfoMap[dvbtype].TimeY.c_str());
	return true;
}

bool PropManager::GetAlarmID( long& alarmid )
{
	alarmid=++AlarmID;
	return true;
}

//wz_0217
bool PropManager::GetVirDeviceId(eDVBType type, int& deviceid)
{
	std::map<eDVBType, int>::iterator iter = mVirtualDevInfo.begin();
	for (;iter != mVirtualDevInfo.end();++iter)
	{
		if (type == iter->first)
		{
			deviceid = iter->second;
			return true;
		}
	}

	return false;
}
//wz_0217
//wz_0217
bool PropManager::GetVirDevList( std::list<int>& devicelist )
{
	std::map<eDVBType,int>::iterator iter = mVirtualDevInfo.begin();
	for (;iter != mVirtualDevInfo.end();++iter)
	{
		devicelist.push_back(iter->second);
	}
	return true;
}
//wz_0217
//wz_0217
//deviceid为虚拟通道，返回true，为实际通道，返回false
bool PropManager::GetVirDevType(int deviceid, eDVBType& type)
{
	std::map<eDVBType,int>::iterator iter = mVirtualDevInfo.begin();
	for (;iter != mVirtualDevInfo.end();++iter)
	{
		if (deviceid == iter->second)
		{
			type = iter->first;
			return true;
		}
	}
	return false;
}
//wz_0217
//wz_0217
bool PropManager::IsRoundChannel(int deviceid)
{
	std::map<eDVBType,int>::iterator iter = mVirtualDevInfo.begin();
	for (;iter != mVirtualDevInfo.end();++iter)
	{
		if (deviceid == iter->second)
		{
			return true;
		}
	}
	return false;
}
//wz_0217

bool PropManager::GetQualityCardInfo(eDVBType dvbtype,std::string &ip,int &port)
{
	std::map<eDVBType,QualityCard>::iterator iter=mQualityCard.begin();
	for(;iter != mQualityCard.end();iter++)
	{
		if(dvbtype == iter->first)
		{
			ip=iter->second.IP;
			port=StrUtil::Str2Int(iter->second.Port);
			return true;
		}
	}
	return false;
}

//wz_110309
bool PropManager::GetStreamProtocol(eDVBType dvbtype,std::string& Protocol)
{
	//map中可能没有dvbtype
	if (mVideoProtocolMap.find(dvbtype) == mVideoProtocolMap.end())
	{
		string strDvbType = OSFunction::GetStrDVBType(dvbtype);
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)获取[%s]流传送协议类型失败!\n",strDvbType.c_str()));
		return false;
	}
	Protocol = mVideoProtocolMap[dvbtype].streamprotocol;
	return true;
}

//wz_110309
bool PropManager::GetFileProtocol(eDVBType dvbtype,std::string& Protocol)
{
	//map中可能没有dvbtype
	if (mVideoProtocolMap.find(dvbtype) == mVideoProtocolMap.end())
	{
		string strDvbType = OSFunction::GetStrDVBType(dvbtype);
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)获取[%s]文件传送协议类型失败!\n",strDvbType.c_str()));
		return false;
	}
	Protocol = mVideoProtocolMap[dvbtype].fileprotocol;
	return true;
}

//wz_110309
bool PropManager::GetFileUrlType(eDVBType dvbtype,std::string& urltype)
{
	//map中可能没有dvbtype
	if (mVideoProtocolMap.find(dvbtype) == mVideoProtocolMap.end())
	{
		string strDvbType = OSFunction::GetStrDVBType(dvbtype);
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)获取[%s]文件urltype失败!\n",strDvbType.c_str()));
		return false;
	}
	urltype = mVideoProtocolMap[dvbtype].fileurltype;
	return true;
}

//wz_110309
bool PropManager::GetFileOffset(eDVBType dvbtype,std::string& offset)
{
	//map中可能没有dvbtype
	if (mVideoProtocolMap.find(dvbtype) == mVideoProtocolMap.end())
	{
		string strDvbType = OSFunction::GetStrDVBType(dvbtype);
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)获取[%s] : offset失败!\n",strDvbType.c_str()));
		return false;
	}
	offset = mVideoProtocolMap[dvbtype].fileoffset;
	return true;
}

//wz_110309
bool PropManager::GetRecDownProtocol(eDVBType dvbtype,std::string& Protocol)
{
	//map中可能没有dvbtype
	if (mRecordDownMap.find(dvbtype) == mRecordDownMap.end())
	{
		string strDvbType = OSFunction::GetStrDVBType(dvbtype);
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)获取[%s]文件下载协议类型失败!\n",strDvbType.c_str()));
		return false;
	}
	Protocol = mRecordDownMap[dvbtype].protocol;
	return true;
}

//wz_110309
bool PropManager::GetRecDownUrlType(eDVBType dvbtype,std::string& urltype)
{
	//map中可能没有dvbtype
	if (mRecordDownMap.find(dvbtype) == mRecordDownMap.end())
	{
		string strDvbType = OSFunction::GetStrDVBType(dvbtype);
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)获取[%s]文件下载urltype失败!\n",strDvbType.c_str()));
		return false;
	}
	urltype = mRecordDownMap[dvbtype].urltype;
	return true;
}

//wz_110309
bool PropManager::GetRecDownOffset(eDVBType dvbtype,std::string& offset)		
{
	//map中可能没有dvbtype
	if (mRecordDownMap.find(dvbtype) == mRecordDownMap.end())
	{
		string strDvbType = OSFunction::GetStrDVBType(dvbtype);
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)获取[%s]文件下载offset失败!\n",strDvbType.c_str()));
		return false;
	}
	offset = mRecordDownMap[dvbtype].offset;
	return true;
}

bool PropManager::SetUpAlarmInfo(eDVBType dvbtype,UpAlarmInfo alarminfo)
{
	mUpAlarmInfo[dvbtype] = alarminfo;
	string type;
	if(dvbtype==DVBC)
		type="DVBC";
	else if(dvbtype==CTTB)
		type="CTTB";
	else if(dvbtype==ATV)
		type="ATV";
	else if(dvbtype==CTV)
		type="CTV";
	else if(dvbtype==DVBS)
		type="DVBS";
	else if(dvbtype==THREED)
		type="THREED";

	mConfig->SetNodeAttribute(type,"alarm","type",alarminfo.type.c_str());
	mConfig->SetNodeAttribute(type,"alarm","interval",alarminfo.interval.c_str());
	mConfig->SetNodeAttribute(type,"alarm","alarmserver",alarminfo.alarmurl.c_str());
	return true;

}

bool PropManager::GetUpAlarmInfo (eDVBType dvbtype,UpAlarmInfo& alarminfo)
{
	alarminfo=mUpAlarmInfo[dvbtype];
	return true;
}


std::string PropManager::GetTableSharePath(eDVBType dvbtype, bool ret)
{
	if (mTableSharePath.find(dvbtype) == mTableSharePath.end())
	{
		ret = false;
		return "";
	}
	ret = true;
	return mTableSharePath[dvbtype];
}

bool PropManager::SetTempFileExpire(std::string expire)
{
	mTempFileExpire = expire;
	bool ret = mConfig->SetNodeAttribute("other", "temppath", "fileexpire", expire.c_str());
	return ret;
}

std::string PropManager::GetSharePathByLoc(std::string loc)
{
	string temppath = "";
	map<string, string>::iterator iter = mRecLocToShare.find(loc);
	if (iter != mRecLocToShare.end())
	{
		temppath = iter->second;
	}
	return temppath;
}


bool PropManager::GetSharePathByLoc(std::string loc, std::string& sharepath)
{
	bool isAddPath = false;
	std::string tempname = loc;
	size_t pos =0;
	do{ 
		pos = tempname.find_last_of("/");
		if (pos ==std::string::npos)
		{
			return false;
		}
		std::string dirpath = tempname.substr(0,pos+1);
		map<string, string>::iterator iter = mRecLocToShare.find(dirpath);
		if (iter == mRecLocToShare.end())
		{
			tempname = tempname.substr(0,pos);
			isAddPath = true;
			continue;
		}
		else
		{
			sharepath = iter->second;
			
			if(isAddPath)
			{
				sharepath.append(loc.substr(pos+1,loc.length()));
			}
			return true;
			break;
		}
	
	}while(pos>2);

 	return false;
// 	map<string, string>::iterator iter = mRecLocToShare.find(loc);
// 	if (iter == mRecLocToShare.end())
// 	{
// 		return false;
// 	}
// 		sharepath = iter->second;
// 		return true;
}


std::string PropManager::GetRecPathByDevId(int deviceid)	//通过通道号获取录像文件存放路径
{
	map<string, list<int>>::iterator mapIter = mDeviceIdToPath.begin();
	for (;mapIter!=mDeviceIdToPath.end();++mapIter)
	{
		if ( !(mapIter->second).empty() )
		{
			list<int>::iterator listIter = (mapIter->second).begin();
			for (;listIter!=(mapIter->second).end();++listIter)
			{
				if (deviceid == *listIter)
				{
					return mapIter->first;
				}
			}
		}
		else
			continue;
	}

	return "";
}


std::string PropManager::GetStoreType()
{
	return mStoreType;
}

	
//获取异态录像文件头偏移秒数
std::string PropManager::GetAlarmHeadOffset(enumDVBType eDvbtype)	
{ 
	if (mRecParamInfo.find(eDvbtype)==mRecParamInfo.end())
		return string("20");	//默认值为20

	return mRecParamInfo[eDvbtype].alarmheadoffset; 
}		
//获取异态录像文件尾偏移秒数
std::string PropManager::GetAlarmTailOffset(enumDVBType eDvbtype)	
{
	if (mRecParamInfo.find(eDvbtype)==mRecParamInfo.end())
		return string("20");	//默认值为20

	return mRecParamInfo[eDvbtype].alarmtailoffset;
}	
	
//获取异态录像过期天数
std::string PropManager::GetAlarmRecordExpire(enumDVBType eDvbtype)	
{
	if (mRecParamInfo.find(eDvbtype)==mRecParamInfo.end())
		return string("30");	//默认值为30天

	return mRecParamInfo[eDvbtype].alarmrecordexpire; 
}	

//获取普通录像过期天数
std::string PropManager::GetRecordExpiredays(enumDVBType eDvbtype)
{
	if (mRecParamInfo.find(eDvbtype)==mRecParamInfo.end())
		return string("7");		//默认值为7天

	return mRecParamInfo[eDvbtype].recordexpire; 
}
//获取异态录像mode
std::string PropManager::GetAlarmRecordMode(enumDVBType eDvbtype)	
{
	if (mRecParamInfo.find(eDvbtype)==mRecParamInfo.end())
		return string("all");	//默认值为all

	return mRecParamInfo[eDvbtype].alarmrecordmode; 
}

std::string PropManager::GetTCPTSUrl(int deviceid)
{
	string url="http://";
	string strip;
	GetDeviceIP(deviceid,strip);
	string strtsport;
	GetDeviceTsPort(deviceid,strtsport);
	url=url+strip+":";
	url=url+strtsport;
	return url;
}

bool PropManager::IsRecordDeviceID(int deviceid)
{
	bool ret = false;;
	std::map<std::string, std::list<int>>::iterator autoIter = mTaskInfo.find("AutoRecord");

	if(autoIter == mTaskInfo.end())
		return ret;
	else
	{
		std::list<int>::iterator iter = autoIter->second.begin();
		for(; iter != autoIter->second.end(); iter++)
			if(*iter == deviceid)
			{
				ret = true;
				break;
			}
	}
	return ret;
}
bool PropManager::GetDevMonitorInfo(int index,sDeviceInfo& DeviceInfo)
{
    bool ret = false;
    if(index<mMonitorDevInfo.size())
    {
        DeviceInfo = mMonitorDevInfo[index];
        ret = true;
    }
    return ret;
}