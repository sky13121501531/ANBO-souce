#ifndef _TYPE_DEF_H_
#define _TYPE_DEF_H_
#pragma once
///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：TypeDef.h
// 创建者：gaoxd
// 创建时间：2009-05-20
// 内容描述：自定义类型定义，建议多处使用的自定义类型统一在此处定义
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <list>
#include <vector>
#include <map>
#include "ace/SOCK_Acceptor.h"
#include "TimeUtil.h"
class XMLTask;


//定时任务时间段分类
typedef enum eDVBType 
{
	UNKNOWN	= 0,
	CTTB	= 1,
	CMMB	= 2,
	DVBC	= 3,
	RADIO	= 4,
	ATV		= 5,
	AM		= 6,
	DVBS    = 7,
	CTV     = 8,
	THREED  = 9

} enumDVBType;
/*
//定时任务时间段分类
typedef enum eFrequence 
{
	ONCE = 0,
	DAILY,
	WEEKLY,
	MONTHLY
} enumFrequence;
*/
//任务状态类型定义
typedef enum eTaskStatus 
{
	WAITING,			//等待（生成任务时状态）
	READY,				//就绪，等待被调度
	RUNNING,			//运行中
	FINISHED,			//单次运行结束
	EXPIRED				//任务过期（单次任务结束后为过期；定时任务最后一次执行完毕后为过期）
} enumTaskStatus;

//任务设置/运行结果状态值
typedef enum eTaskRetStatus 
{
	RUN_SUCCESS				= 0,			//运行成功
	RUN_FAILED				= 1,			//运行失败，无描述
	NOFILE_EXIST		    = 2,            //文件不存在
	CHANNELID_UNAVAIABLE	= 3,			//频道非法
	DEVICEID_UNAVAIABLE		= 4,			//通道非法
	NODEVICEIDMATCH			= 5,			//没有通道匹配该任务
	PREFERENTIALTASK_USE	= 6,			//存在高优先级任务占用
	DATABASEERROR			= 7,			//操作数据库错误
	DEVICEERROR				= 8,			//操作硬件错误
	WAITFORSET				= 100,			//等待设置
	SET_SUCCESS				= 101,			//设置成功
} enumTaskRetStatus ;


//日志类型定义
typedef enum eLogType              
{
	LOG_EVENT_DEBUG = 0,		//调试信息
	LOG_EVENT_WARNNING = 1,		//警告信息
	LOG_EVENT_ERROR = 2			//错误信息
}enumLogType;

//日志输出类型
typedef enum eLogOutPut
{
	LOG_OUTPUT_UNDEFINE = 0,	//未定义输出类型
	LOG_OUTPUT_SCREEN	= 1,	//输出到屏幕
	LOG_OUTPUT_FILE		= 2,	//输出到文件
	LOG_OUTPUT_BOTH		= 3		//输出到屏幕和文件
}enumLogOutPut;

//模块类型定义
typedef enum eModuleType              
{
	NONE			= 0,		//无类型，属于错误
	RECORD			= 1,		//录像
	VIDEO			= 2,		//视频，包含实时视频，视频轮播，录像视频查看
	DATAANALYSER	= 3,		//指标测量，PSI/SI分析以及其他码流分析
	ALARM			= 4,		//报警		
	OTHER			= 5,		//设置型任务
	DEVICE			= 6			//硬件层日志
}enumModuleType;

//报警类型
typedef enum eAlarmType
{
	ALARM_NONE			      = 0,	//未定义报警类型
	ALARM_ENVIRONMENT	      = 1,	//环境报警类型
	ALARM_FREQ			      = 2,	//射频报警类型
	ALARM_PROGRAM		      = 3,	//节目报警类型
	ALARM_TR101_290		      = 4,	//TR101_290报警类型
	ALARM_RECORDSCHEDULER     = 5,  //录像冲突报警
    ALARM_EQUIPMENT           = 6   //调频卡报警
}enumAlarmType;

//系统消息类型定义
typedef enum eSysMsgType              
{
	VS_MSG_SYSTINFO = 0,		//系统普通信息
	VS_MSG_PROALARM = 1,		//节目异态/报警信息
	VS_MSG_SYSALARM = 2			//系统报警信息
}enumSysMsgType;


//设备信息定义
typedef struct
{
	std::string devicetype; // 设别类型
	std::string IP; //ip地址
	std::string baseIP; //ip地址
	std::string cmdport;//命令端口号
	std::string tstype;//ts流类型，组播或单播
	std::string tsip;//ts流ip
	std::string tsport;//数据端口号
	std::string cmdprotocol;//命令协议
	std::string tsprotocol;//数据协议
	std::string tunerid;//tuner通道号
	std::string logindex;//逻辑通道
	std::string recordlist;//录像通道
}sDeviceInfo;

typedef std::map<std::string,std::list<int> > TaskInfoMap;

typedef struct
{
	int DevicID;		//设备ID
	int Statuc; //设备状态     可取值：0 -正常状态  1 -无板卡  2 - 有任务运行
	std::string Desc;	//设备描述   
}sDeviceStatus;

//视频用户信息
typedef struct VEDIOUSERINFO
{
	ACE_SOCK_Stream client;	//用户Socket
	eDVBType DvbType;
	int fail_num;			//发送错误次数
	std::string DeviceID;	//用户使用的通道号
	std::string Name;		//名称
	std::string URL;		//音视频地址
	std::string IP;			//地址
	std::string Port;		//端口
	std::string Priority;	//优先级
	std::string CenterCode;		//中心号
	std::string MsgID;
	std::string Freq;  //频点信息

	VEDIOUSERINFO()
	{
		DvbType = UNKNOWN;
		fail_num = 0;
		DeviceID = "";
		Name = "";
		IP = "";
		Port = "";
		URL = "";
		Priority = "";
		CenterCode = "";
		MsgID = "";
		Freq="";
	};
	VEDIOUSERINFO&	operator =(const VEDIOUSERINFO& User)
	{
		client= User.client;
		DvbType = User.DvbType;
		fail_num = User.fail_num;
		DeviceID = User.DeviceID;
		Name = User.Name;
		IP = User.IP;
		Port = User.Port;
		URL = User.URL;
		Priority = User.Priority;
		CenterCode=User.CenterCode;
		MsgID = User.MsgID;
		Freq = User.Freq;
		return *this;
	};
	bool	operator ==(const VEDIOUSERINFO& User)
	{
		return client.get_handle()==User.client.get_handle()&&DeviceID==User.DeviceID&&Name==User.Name&& \
				IP==User.IP&&Port==User.Port&&Priority==User.Priority&& CenterCode==User.CenterCode && MsgID==User.MsgID && Freq==User.Freq;
	};

}sVedioUserInfo;

//频道扫描信息定义
typedef struct CHANNELINFO{
	//关键频道信息
	std::string ChannelID;
	std::string STD;
	std::string Freq; 
	std::string OrgNetID; 
	std::string TsID;
	std::string PmtPID;  //wz_101123
	//频道信息
	std::string SymbolRate; 
	std::string QAM;
	std::string HeaderMode;
	std::string CarrierNumber;
	std::string EncodeEfficiency;
	std::string InterwovenMode;
	std::string DoublePilot; 
	std::string PnPhase;
	std::string ChannelCode;
	std::string ProgramName;
	std::string ServiceID;
	std::string VideoPID;
	std::string AudioPID;
	std::string HDTV;
	std::string Encryption;
	std::string PcrPID;  //wz_101123

public:
	CHANNELINFO(const CHANNELINFO& other) //copy construction
	{
		ChannelID = other.ChannelID;
		STD=other.STD;
		Freq=other.Freq;
		OrgNetID=other.OrgNetID;
		TsID=other.TsID;
		PmtPID = other.PmtPID;  //wz_101123

		SymbolRate=other.SymbolRate;
		QAM=other.QAM;
		HeaderMode=other.HeaderMode;
		CarrierNumber=other.CarrierNumber;
		EncodeEfficiency=other.EncodeEfficiency;
		InterwovenMode=other.InterwovenMode;
		DoublePilot=other.DoublePilot;
		PnPhase=other.PnPhase;
		ChannelCode=other.ChannelCode;
		ProgramName = other.ProgramName;
		ServiceID = other.ServiceID;
		VideoPID = other.VideoPID;
		AudioPID = other.AudioPID;
		HDTV=other.HDTV;
		Encryption = other.Encryption;
		PcrPID = other.PcrPID;  //wz_101123
	};
   CHANNELINFO()
   {

   };
   bool	operator ==(const CHANNELINFO& other)
   {
	   return STD==other.STD&&Freq==other.Freq&&OrgNetID==other.OrgNetID&&TsID==other.TsID
		   &&ServiceID==other.ServiceID&&VideoPID==other.VideoPID&&AudioPID==other.AudioPID//;
		   &&PmtPID==other.PmtPID&&PcrPID==other.PcrPID;  //wz_101123
   };
   bool	operator >(const CHANNELINFO& other)
   {
	   return true;
   };
   friend  bool	 operator <(const CHANNELINFO& other1,const CHANNELINFO& other2)
   {
	   return other1.Freq<other2.Freq;
   };
   bool	operator <(const CHANNELINFO& other)
   {
	   return Freq<other.Freq;
   };
   bool	operator ()(const CHANNELINFO& other)
   {
	   return true;
   };
   CHANNELINFO&	operator =(const CHANNELINFO& other)
   {
	   ChannelID = other.ChannelID;
	   STD=other.STD;
	   Freq=other.Freq;
	   OrgNetID=other.OrgNetID;
	   TsID=other.TsID;
	   PmtPID = other.PmtPID;  //wz_101123

	   SymbolRate=other.SymbolRate;
	   QAM=other.QAM;
	   HeaderMode=other.HeaderMode;
	   CarrierNumber=other.CarrierNumber;
	   EncodeEfficiency=other.EncodeEfficiency;
	   InterwovenMode=other.InterwovenMode;
	   DoublePilot=other.DoublePilot;
	   PnPhase=other.PnPhase;
	   if (other.ChannelCode!="")
	   {
		   ChannelCode=other.ChannelCode;
	   }
	   if (other.ProgramName!="")
	   {
		   ProgramName = other.ProgramName;
	   }
	   ServiceID = other.ServiceID;
	   VideoPID = other.VideoPID;
	   AudioPID = other.AudioPID;
	   HDTV=other.HDTV;
	   Encryption = other.Encryption;
	   PcrPID = other.PcrPID;  //wz_101123

	   return *this;
   };

}sChannelInfo;

typedef std::map<enumDVBType,std::vector<sChannelInfo>> ChannelInfoMap;

//录像信息
typedef struct{
	enumDVBType dvbtype;
	std::string channelID;
	std::string taskid;
	std::string filename;
	std::string starttime;
	std::string endtime;
	std::string expiredays;
}sRecordInfo;
//自动录像文件信息
typedef struct
{
	std::string url;
	std::string starttime;
	std::string endtime;

}sRecordFileInfo;
//中心信息
typedef struct  
{
	std::string centerType;
	std::string centerUrl;
}sCenterInfo;

//报警参数
typedef struct AlarmParam{
	enumDVBType DVBType;			//监测类型
	enumAlarmType AlarmType;		//报警类型
	std::string TypeID;				//报警ID
	std::string TypeDesc;			//报警描述
	std::string STD;				//发射标准
	std::string Freq;				//频率
	std::string SymbolRate;			//符号率
	std::string ChannelID;			//频道ID
	std::string Duration;			//持续时间
	std::string Num;				//发生次数
	std::string TimeInterval;		//时间间隔
	std::string DownThreshold;		//下限
	std::string UpThreshold;		//上限
	std::string Switch;				//开关  1：开 0 ： 关
	std::string DeviceID;           //设备通道号
	
	AlarmParam& operator = (const AlarmParam param)
	{
		DVBType=param.DVBType;
		AlarmType=param.AlarmType;
		TypeID=param.TypeID;
		if (param.TypeDesc!="")
		{
			TypeDesc=param.TypeDesc;
		}
		if(param.STD!="")
		{
			STD=param.STD;
		}
		if (param.TypeDesc!="")
		{
			TypeDesc=param.TypeDesc;
		}
		if(param.Freq!="")
		{
			Freq=param.Freq;
		}
		if (param.SymbolRate!="")
		{
			SymbolRate=param.SymbolRate;
		}
		if(param.ChannelID!="")
		{
			ChannelID=param.ChannelID;
		}
		if (param.Duration!="")
		{
			Duration=param.Duration;
		}
		if(param.Num!="")
		{
			Num=param.Num;
		}
		if (param.TimeInterval!="")
		{
			TimeInterval=param.TimeInterval;
		}
		if(param.DownThreshold!="")
		{
			DownThreshold=param.DownThreshold;
		}
		if (param.UpThreshold!="")
		{
			UpThreshold=param.UpThreshold;
		}
		if(param.Switch!="")
		{
			Switch=param.Switch;
		}
		if(param.DeviceID!="")
		{
			DeviceID=param.DeviceID;
		}

	return *this;
	}
}sAlarmParam;

typedef std::vector<sAlarmParam> AlarmParamVec;
typedef std::map<std::string,std::vector<sAlarmParam>> CodeAlarmParamMap;

//检查报警需要的参数
typedef struct CheckParam{
	enumDVBType DVBType;			//监测类型
	enumAlarmType AlarmType;		//报警类型
	std::string TypeID;				//报警ID
	std::string TypeDesc;			//报警描述
	std::string STD;				//发射标准
	std::string Freq;				//频率
	std::string SymbolRate;			//符号率
	std::string ChannelID;			//频道ID
	std::string TypedValue;			//测量值
	time_t CheckTime;
	std::string AlarmID;
	std::string DeviceID;           //设备通道号
	std::string mode;
	std::string ServiceID;
	std::string VideoID;
	std::string AudioID;
	std::string StrCheckTime;
}sCheckParam;
//报警上报方式
struct  AlarmInfo
{
	enumDVBType DVBType;
	enumAlarmType AlarmType;
	std::string STD;				//发射标准
	std::string Freq;				//频率
	std::string SymbolRate;			//符号率
	std::string ChannelID;			//频道ID
	std::string TypeID;				//报警ID
	std::string TypeDesc;			//报警描述
	std::string AlarmValue;			//报警值
	std::string CheckTime;
};
//报警结果信息
struct AlarmResult
{
	enumDVBType DVBType;			//监测类型
	enumAlarmType AlarmType;		//报警类型
	std::string TypeID;				//报警ID
	std::string TypeDesc;			//报警描述
	std::string STD;				//发射标准
	std::string Freq;				//频率
	std::string SymbolRate;			//符号率
	std::string ChannelID;			//频道ID
	std::string TypedValue;			//报警值
	std::string AlarmTime;			//发生报警时间
};
typedef struct  RunPlanParam
{
	enumDVBType dvbtype;			//监测类型
	std::string ChannelID;			//频道ID
	int Type;						//时间段类型 0：单次；1：多次
	std::string Month;				//月份
	std::string DayOfMonth;			//月份中的天数
	std::string DayOfWeek;			//星期
	std::string StartTime;			//开始时间
	std::string EndTime;			//结束时间
	std::string ValidStartDateTime;	//有效起始时间
	std::string ValidEndDateTime;	//有效结束时间
	std::string StartDateTime;		//开始时间
	std::string EndDateTime;		//结束时间
}sRunPlanParam;

typedef std::vector<sRunPlanParam> RunPlanParamVec;
typedef std::map< std::string, RunPlanParamVec > RunPlanParamMap;

typedef struct QualityInfo{
	std::string type;
	std::string valu;
	std::string desc;
	std::string STD;
	std::string checktime;
	std::string freq;
}eQualityInfo;

typedef struct SpecInfo
{
	std::string type;
	std::string valu;
	std::string desc;
	std::string STD;
	std::string checktime;
	std::string freq;
	std::string status;
}eSpecInfo;

//CPU信息
typedef struct
{
	DWORD   dwUnknown1;
	ULONG   uKeMaximumIncrement;
	ULONG   uPageSize;
	ULONG   uMmNumberOfPhysicalPages;
	ULONG   uMmLowestPhysicalPage;
	ULONG   uMmHighestPhysicalPage;
	ULONG   uAllocationGranularity;
	PVOID   pLowestUserAddress;
	PVOID   pMmHighestUserAddress;
	ULONG   uKeActiveProcessors;
	BYTE    bKeNumberProcessors;
	BYTE    bUnknown2;
	WORD    wUnknown3;
} SYSTEM_BASIC_INFORMATION;
typedef struct
{
	LARGE_INTEGER   liIdleTime;
	DWORD           dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;
typedef struct
{
	LARGE_INTEGER liKeBootTime;
	LARGE_INTEGER liKeSystemTime;
	LARGE_INTEGER liExpTimeZoneBias;
	ULONG         uCurrentTimeZoneId;
	DWORD         dwReserved;
} SYSTEM_TIME_INFORMATION;

typedef struct task
{
//	XMLTask* pTask;

	eDVBType dvbtype;
	eTaskStatus status;

	std::string msgid;
	std::string setdatetime;	//任务下发日期
	int deviceid;
	time_t startdatetime;		//开始时间日期
	time_t enddatetime;			//开始时间日期
	long cycleinterval;			//周期间隔

	std::string taskid;
	std::string taskname;
	std::string freq;
	std::string channelid;
	std::string priority;
	std::string runpriority;
} sTaskInfo;

typedef struct qualityCompenstaion
{
	eDVBType dvbtype;
	std::string deviceid;
	std::string type;
	std::string valu;
} sQualityCompensation;
//环境信息
struct EnvInfo
{
	std::string Type;
	std::string Desc;
	EnvInfo()
	{

	};
	bool operator ==(const EnvInfo& other) const
	{
		return Type==other.Type&&Desc==other.Desc;
	};
	bool operator >(const EnvInfo& other) const
	{
		return Type > other.Type;
	};
	friend  bool operator > (const EnvInfo& other1,const EnvInfo& other2)
	{
		return other1.Type>other2.Type;
	};
	friend  bool operator < (const EnvInfo& other1,const EnvInfo& other2)
	{ 
		return other1.Type<other2.Type;
	};
	bool operator ()(const EnvInfo& other) const
	{
		return true;
	};
};
typedef std::map<EnvInfo,std::string>  EnvMapInfo;


struct sFreqScanInfo{
	std::string CenterFreq;
	std::string SymbolRate;
	std::string QAM;
};

struct OSDInfo{
	std::string FontSize;
	std::string Info;
	std::string InfoX;
	std::string InfoY;
	std::string TimeType;
	std::string TimeX;
	std::string TimeY;
	std::string ProgramX;
	std::string ProgramY;
};

struct SpectrumInfo
{
	float freq;
	float level;
};


struct SpectrumInfoEx
{
	float freq;
	float level;
	float usn;
	float wam;
	float offset;
};

struct QualityCard
{
	std::string IP;
	std::string Port;
};

//wz_110309
//配置文件videoprotocol节点信息结构
typedef struct _sVideoProtocol
{
	std::string streamprotocol;
	std::string fileprotocol;
	std::string fileurltype;
	std::string	fileoffset;
} sVideoProcotol, *psVideoProtocol;

//wz_110309
//recorddown节点信息结构
typedef struct _sRecordDownInfo
{
	std::string protocol;
	std::string urltype;
	std::string offset;
} sRecordDownInfo, *pRecordDownInfo;

//报警上报结构
struct UpAlarmInfo
{ 
	std::string type;		//0：查询；1：主动上报
	std::string interval;	//上报时间间隔 单位：秒
	std::string onceday;    //1:隔天报警;0 不做隔天报警
	std::string alarmurl;	//报警上报地址
	std::string runplanrecord; // 运行图期间是否保存录像 0：保存 1 不保存
};
//录像文件参数结构
typedef struct _sRecordParamInfo
{ 
	std::string recordexpire;		//录像过期天数
	std::string alarmrecordexpire;	//异态录像过期天数
	std::string alarmheadoffset;	//异态录像头偏移长度
	std::string alarmtailoffset;	//异态录像尾偏移长度
	std::string alarmrecordmode;	//异态录像存储模式
} sRecordParamInfo, psRecordParamInfo;

//定时检查类型定义  week,day,single;
typedef enum eCheckType              
{
	PERWEEK				= 0,		//每星期
	PERDAY				= 1,		//每天
	PERSINGLE			= 2,		//单次
	NOTHING             = 3         //不做处理
}enumCheckType;

typedef struct SignalCheck
{
	enumDVBType  dvbtype;
	std::string DeviceID;
	std::string Freq;
	std::string ChannelID;
	int  level;
	SignalCheck ()
	{
		level=100;
	}
	SignalCheck& operator = (const SignalCheck other)
	{
		dvbtype=other.dvbtype;
		DeviceID=other.DeviceID;
		Freq=other.Freq;
		ChannelID=other.ChannelID;
		level = other.level;
		return *this;
	}
	bool operator ==(const SignalCheck& other)
	{
		return dvbtype==other.dvbtype&&DeviceID==other.DeviceID&&Freq==other.Freq&&ChannelID==other.ChannelID;
	}

} sSignalCheck;

typedef struct SystemConfigureParameter
{
	string SystemIp;
	string VideoHttpServerIp;
	string VideoHttpPort;
	string VideoHttpMaxnum;
	string VideoRtspServerIp;
	string VideoRtspPort;
	string VideoRtspMaxnum;
	string QualityServerIp;
	string QualityPort;
	string QualityMaxnum;
	string DeviceServerIp;
	string DevicePort;
	string DeviceMaxnum;
	string LogServerIp;
	string LogPort;
	string LogMaxnum;
	string XmlServerIp;
	string XmlPort;
	string HttpServerIp;
	string HttpPort;
	string FtpServerIp;
	string FtpPort;
	string RecordQualityIp;
	string RecordQualityPort;
	string RecordQualityMaxnum;
	string Record_FileLocation;
	string Record_FileSharePath;
	string Record_DeviceId;
	string Record_Period;
	string Record_MaxAvailableSize;
	string Record_DBCleanInterval;
	string Record_SystemCleanTime;
	string Record_StoreType;
	string Log_Path;
	string Log_Expire;
	string Log_Type;
	string Log_outputToFile;
	string LogPathType_Analyser;
	string LogPathType_Record;
	string LogPathType_Video;
	string LogPathType_Device;
	string LogPathType_Other;
	string LogPathType_Default;
	string XmlSendTime;
	string XmlOverTime;
	string TempFileLocation;
	string TempSharePath;
	string TempFileExpireTime;
	string DeviceSchedulerTask_Type;
	string DeviceSchedulerTask_WeekDay;
	string DeviceSchedulerTask_Date;
	string DeviceSchedulerTask_Time;
	string PsisiSchedulerTask_Type;
	string PsisiSchedulerTask_WeekDay;
	string PsisiSchedulerTask_Date;
	string PsisiSchedulerTask_Time;
}sSysConfigParam;

typedef struct DvbConfigureParameter
{
	string type;
	string CenterFreq;
	string Symbolrate;
	string Qam;
	string ChannelScanType;
	string Osd_fontsize;
	string Osd_infoosd;
	string Osd_infoosdx;
	string Osd_infoosdy;
	string Osd_timeosdtype;
	string Osd_timeosdx;
	string Osd_timeosdy;
	string Osd_programx;
	string Osd_programy;
	string TablePath;
	string SharePath;
	string AlarmType;
	string AlarmInterval;
	string OnceDay;
	string RunplanRecord;
	string AlarmServer;
	string VideoStreamProtocol;
	string VideoFileProtocol;
	string VideoFileUrlType;
	string VideoFileOffSet;
	string RecordParam_recordexpire;
	string RecordParam_alarmrecordexpire;
	string RecordParam_alarmheadoffset;
	string RecordParam_alarmtailoffset;
	string RecordParam_alarmrecordmode;
	string RecordDown_protocol;
	string RecordDown_urltype;
	string RecordDown_offset;
	string GeneralDestCode;
	string GeneralSrcCode;
	string RealTimeFromRecord;

	string HDRealStreamWidth;
	string HDRealStreamHeight;
	string HDRealStreamBps;
	string SDRealStreamWidth;
	string SDRealStreamHeight;
	string SDRealStreamBps;

	string HDRecordWidth;
	string HDRecordHeight;
	string HDRecordBps;
	string SDRecordWidth;
	string SDRecordHeight;
	string SDRecordBps;
	
	string AlarmStorage_switch;		//异态录像存储开关	wz
	string AlarmStorage_filenum;	//异态录像存储文件个数	wz
}sDvbConfigParam;
//模拟无载波报警电平门限
typedef struct
{
	eDVBType dvbtype;
	string freq;
	string downthreshod;
}ThresHold;

//分别率，码率
typedef struct
{
	string width;
	string height;
	string bps;
}VideoParam;

typedef struct  
{
	std::string _switch;
	std::string _filenum;
} sAlarmRecordStorageCfg;
typedef struct AlarmPriority
{
	eDVBType  dvbtype;
	string    type;
	string    priority;
}sAlarmPriority;
struct SeperateTypeID
{
	string   head;
	string   type;
};
typedef struct AlarmTypePriority
{
	string UnLock;
	string Static;
	string Black;
	string NoAudio;
	string NoVideo;
	string Slice;
}sAlarmTypePriority;
typedef struct TVSPECTRUMPARM
{
	int SpecType;
	int startFreq;
	int endFreq;
	int stepFreq;
	string SpecVBW;
	string SpecRBW;
}SpectrumParm;

typedef struct ALARMRESENDINFO
{
    string sucFailmode;//0 失败 1 成功
    string alarmid;
    string mode;
    string url;
    string alarmxml;
}AlarmResendInfo;
#endif
