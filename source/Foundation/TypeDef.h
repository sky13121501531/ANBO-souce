#ifndef _TYPE_DEF_H_
#define _TYPE_DEF_H_
#pragma once
///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����TypeDef.h
// �����ߣ�gaoxd
// ����ʱ�䣺2009-05-20
// �����������Զ������Ͷ��壬����ദʹ�õ��Զ�������ͳһ�ڴ˴�����
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <list>
#include <vector>
#include <map>
#include "ace/SOCK_Acceptor.h"
#include "TimeUtil.h"
class XMLTask;


//��ʱ����ʱ��η���
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
//��ʱ����ʱ��η���
typedef enum eFrequence 
{
	ONCE = 0,
	DAILY,
	WEEKLY,
	MONTHLY
} enumFrequence;
*/
//����״̬���Ͷ���
typedef enum eTaskStatus 
{
	WAITING,			//�ȴ�����������ʱ״̬��
	READY,				//�������ȴ�������
	RUNNING,			//������
	FINISHED,			//�������н���
	EXPIRED				//������ڣ��������������Ϊ���ڣ���ʱ�������һ��ִ����Ϻ�Ϊ���ڣ�
} enumTaskStatus;

//��������/���н��״ֵ̬
typedef enum eTaskRetStatus 
{
	RUN_SUCCESS				= 0,			//���гɹ�
	RUN_FAILED				= 1,			//����ʧ�ܣ�������
	NOFILE_EXIST		    = 2,            //�ļ�������
	CHANNELID_UNAVAIABLE	= 3,			//Ƶ���Ƿ�
	DEVICEID_UNAVAIABLE		= 4,			//ͨ���Ƿ�
	NODEVICEIDMATCH			= 5,			//û��ͨ��ƥ�������
	PREFERENTIALTASK_USE	= 6,			//���ڸ����ȼ�����ռ��
	DATABASEERROR			= 7,			//�������ݿ����
	DEVICEERROR				= 8,			//����Ӳ������
	WAITFORSET				= 100,			//�ȴ�����
	SET_SUCCESS				= 101,			//���óɹ�
} enumTaskRetStatus ;


//��־���Ͷ���
typedef enum eLogType              
{
	LOG_EVENT_DEBUG = 0,		//������Ϣ
	LOG_EVENT_WARNNING = 1,		//������Ϣ
	LOG_EVENT_ERROR = 2			//������Ϣ
}enumLogType;

//��־�������
typedef enum eLogOutPut
{
	LOG_OUTPUT_UNDEFINE = 0,	//δ�����������
	LOG_OUTPUT_SCREEN	= 1,	//�������Ļ
	LOG_OUTPUT_FILE		= 2,	//������ļ�
	LOG_OUTPUT_BOTH		= 3		//�������Ļ���ļ�
}enumLogOutPut;

//ģ�����Ͷ���
typedef enum eModuleType              
{
	NONE			= 0,		//�����ͣ����ڴ���
	RECORD			= 1,		//¼��
	VIDEO			= 2,		//��Ƶ������ʵʱ��Ƶ����Ƶ�ֲ���¼����Ƶ�鿴
	DATAANALYSER	= 3,		//ָ�������PSI/SI�����Լ�������������
	ALARM			= 4,		//����		
	OTHER			= 5,		//����������
	DEVICE			= 6			//Ӳ������־
}enumModuleType;

//��������
typedef enum eAlarmType
{
	ALARM_NONE			      = 0,	//δ���屨������
	ALARM_ENVIRONMENT	      = 1,	//������������
	ALARM_FREQ			      = 2,	//��Ƶ��������
	ALARM_PROGRAM		      = 3,	//��Ŀ��������
	ALARM_TR101_290		      = 4,	//TR101_290��������
	ALARM_RECORDSCHEDULER     = 5,  //¼���ͻ����
    ALARM_EQUIPMENT           = 6   //��Ƶ������
}enumAlarmType;

//ϵͳ��Ϣ���Ͷ���
typedef enum eSysMsgType              
{
	VS_MSG_SYSTINFO = 0,		//ϵͳ��ͨ��Ϣ
	VS_MSG_PROALARM = 1,		//��Ŀ��̬/������Ϣ
	VS_MSG_SYSALARM = 2			//ϵͳ������Ϣ
}enumSysMsgType;


//�豸��Ϣ����
typedef struct
{
	std::string devicetype; // �������
	std::string IP; //ip��ַ
	std::string baseIP; //ip��ַ
	std::string cmdport;//����˿ں�
	std::string tstype;//ts�����ͣ��鲥�򵥲�
	std::string tsip;//ts��ip
	std::string tsport;//���ݶ˿ں�
	std::string cmdprotocol;//����Э��
	std::string tsprotocol;//����Э��
	std::string tunerid;//tunerͨ����
	std::string logindex;//�߼�ͨ��
	std::string recordlist;//¼��ͨ��
}sDeviceInfo;

typedef std::map<std::string,std::list<int> > TaskInfoMap;

typedef struct
{
	int DevicID;		//�豸ID
	int Statuc; //�豸״̬     ��ȡֵ��0 -����״̬  1 -�ް忨  2 - ����������
	std::string Desc;	//�豸����   
}sDeviceStatus;

//��Ƶ�û���Ϣ
typedef struct VEDIOUSERINFO
{
	ACE_SOCK_Stream client;	//�û�Socket
	eDVBType DvbType;
	int fail_num;			//���ʹ������
	std::string DeviceID;	//�û�ʹ�õ�ͨ����
	std::string Name;		//����
	std::string URL;		//����Ƶ��ַ
	std::string IP;			//��ַ
	std::string Port;		//�˿�
	std::string Priority;	//���ȼ�
	std::string CenterCode;		//���ĺ�
	std::string MsgID;
	std::string Freq;  //Ƶ����Ϣ

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

//Ƶ��ɨ����Ϣ����
typedef struct CHANNELINFO{
	//�ؼ�Ƶ����Ϣ
	std::string ChannelID;
	std::string STD;
	std::string Freq; 
	std::string OrgNetID; 
	std::string TsID;
	std::string PmtPID;  //wz_101123
	//Ƶ����Ϣ
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

//¼����Ϣ
typedef struct{
	enumDVBType dvbtype;
	std::string channelID;
	std::string taskid;
	std::string filename;
	std::string starttime;
	std::string endtime;
	std::string expiredays;
}sRecordInfo;
//�Զ�¼���ļ���Ϣ
typedef struct
{
	std::string url;
	std::string starttime;
	std::string endtime;

}sRecordFileInfo;
//������Ϣ
typedef struct  
{
	std::string centerType;
	std::string centerUrl;
}sCenterInfo;

//��������
typedef struct AlarmParam{
	enumDVBType DVBType;			//�������
	enumAlarmType AlarmType;		//��������
	std::string TypeID;				//����ID
	std::string TypeDesc;			//��������
	std::string STD;				//�����׼
	std::string Freq;				//Ƶ��
	std::string SymbolRate;			//������
	std::string ChannelID;			//Ƶ��ID
	std::string Duration;			//����ʱ��
	std::string Num;				//��������
	std::string TimeInterval;		//ʱ����
	std::string DownThreshold;		//����
	std::string UpThreshold;		//����
	std::string Switch;				//����  1���� 0 �� ��
	std::string DeviceID;           //�豸ͨ����
	
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

//��鱨����Ҫ�Ĳ���
typedef struct CheckParam{
	enumDVBType DVBType;			//�������
	enumAlarmType AlarmType;		//��������
	std::string TypeID;				//����ID
	std::string TypeDesc;			//��������
	std::string STD;				//�����׼
	std::string Freq;				//Ƶ��
	std::string SymbolRate;			//������
	std::string ChannelID;			//Ƶ��ID
	std::string TypedValue;			//����ֵ
	time_t CheckTime;
	std::string AlarmID;
	std::string DeviceID;           //�豸ͨ����
	std::string mode;
	std::string ServiceID;
	std::string VideoID;
	std::string AudioID;
	std::string StrCheckTime;
}sCheckParam;
//�����ϱ���ʽ
struct  AlarmInfo
{
	enumDVBType DVBType;
	enumAlarmType AlarmType;
	std::string STD;				//�����׼
	std::string Freq;				//Ƶ��
	std::string SymbolRate;			//������
	std::string ChannelID;			//Ƶ��ID
	std::string TypeID;				//����ID
	std::string TypeDesc;			//��������
	std::string AlarmValue;			//����ֵ
	std::string CheckTime;
};
//���������Ϣ
struct AlarmResult
{
	enumDVBType DVBType;			//�������
	enumAlarmType AlarmType;		//��������
	std::string TypeID;				//����ID
	std::string TypeDesc;			//��������
	std::string STD;				//�����׼
	std::string Freq;				//Ƶ��
	std::string SymbolRate;			//������
	std::string ChannelID;			//Ƶ��ID
	std::string TypedValue;			//����ֵ
	std::string AlarmTime;			//��������ʱ��
};
typedef struct  RunPlanParam
{
	enumDVBType dvbtype;			//�������
	std::string ChannelID;			//Ƶ��ID
	int Type;						//ʱ������� 0�����Σ�1�����
	std::string Month;				//�·�
	std::string DayOfMonth;			//�·��е�����
	std::string DayOfWeek;			//����
	std::string StartTime;			//��ʼʱ��
	std::string EndTime;			//����ʱ��
	std::string ValidStartDateTime;	//��Ч��ʼʱ��
	std::string ValidEndDateTime;	//��Ч����ʱ��
	std::string StartDateTime;		//��ʼʱ��
	std::string EndDateTime;		//����ʱ��
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

//CPU��Ϣ
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
	std::string setdatetime;	//�����·�����
	int deviceid;
	time_t startdatetime;		//��ʼʱ������
	time_t enddatetime;			//��ʼʱ������
	long cycleinterval;			//���ڼ��

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
//������Ϣ
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
//�����ļ�videoprotocol�ڵ���Ϣ�ṹ
typedef struct _sVideoProtocol
{
	std::string streamprotocol;
	std::string fileprotocol;
	std::string fileurltype;
	std::string	fileoffset;
} sVideoProcotol, *psVideoProtocol;

//wz_110309
//recorddown�ڵ���Ϣ�ṹ
typedef struct _sRecordDownInfo
{
	std::string protocol;
	std::string urltype;
	std::string offset;
} sRecordDownInfo, *pRecordDownInfo;

//�����ϱ��ṹ
struct UpAlarmInfo
{ 
	std::string type;		//0����ѯ��1�������ϱ�
	std::string interval;	//�ϱ�ʱ���� ��λ����
	std::string onceday;    //1:���챨��;0 �������챨��
	std::string alarmurl;	//�����ϱ���ַ
	std::string runplanrecord; // ����ͼ�ڼ��Ƿ񱣴�¼�� 0������ 1 ������
};
//¼���ļ������ṹ
typedef struct _sRecordParamInfo
{ 
	std::string recordexpire;		//¼���������
	std::string alarmrecordexpire;	//��̬¼���������
	std::string alarmheadoffset;	//��̬¼��ͷƫ�Ƴ���
	std::string alarmtailoffset;	//��̬¼��βƫ�Ƴ���
	std::string alarmrecordmode;	//��̬¼��洢ģʽ
} sRecordParamInfo, psRecordParamInfo;

//��ʱ������Ͷ���  week,day,single;
typedef enum eCheckType              
{
	PERWEEK				= 0,		//ÿ����
	PERDAY				= 1,		//ÿ��
	PERSINGLE			= 2,		//����
	NOTHING             = 3         //��������
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
	
	string AlarmStorage_switch;		//��̬¼��洢����	wz
	string AlarmStorage_filenum;	//��̬¼��洢�ļ�����	wz
}sDvbConfigParam;
//ģ�����ز�������ƽ����
typedef struct
{
	eDVBType dvbtype;
	string freq;
	string downthreshod;
}ThresHold;

//�ֱ��ʣ�����
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
    string sucFailmode;//0 ʧ�� 1 �ɹ�
    string alarmid;
    string mode;
    string url;
    string alarmxml;
}AlarmResendInfo;
#endif
