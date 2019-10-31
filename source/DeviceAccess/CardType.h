#pragma once

#ifndef  _CARDTYPE_H_
#define _CARDTYPE_H_

#include <time.h>
#include <iostream>

//时间设置模式
#define OSD_CYEAR       0x00100000
#define OSD_YEAR4       0x00020000
#define OSD_YEAR2       0x00010000

#define OSD_EMONTH      0x00002000
#define OSD_MONTH       0x00001000

#define OSD_EWEEK       0x00000200
#define OSD_CWEEK       0x00000100

#define OSD_CHOUR       0x00000010
#define OSD_HOUR24      0x00000002
#define OSD_HOUR12      0x00000001

//报警定义
#define V_BLACK     0x1          //视频黑场
#define C_SLICE     0x2          //彩条
#define V_STATIC	0x4          //画面静止
#define V_MISS      0x8          //无视频信号
#define S_UNLOCK	0xE          //无载波
#define A_SILENCE	0x10         //音频静音

#define NO_SIGNAL   0x81         //无信号
#define NO_VIDEO    0x82         //无视频
#define NO_AUDIO    0x84         //无音频
#define UNCA_VIDEO  0x88         //视频CA失败
#define UNCA_AUDIO  0x90         //音频CA失败


#define V_BLACK_V_STATIC     0x5          //视频黑场+画面静止
#define C_SLICE_V_STATIC     0x6          //彩条+画面静止
#define CARDREBOOT			 0xFFFF		  //板卡重启


//消息类型定义
enum MSG_TYPE
{
	MSG_UNKNOWN					= 0x0,				//未知消息
	
	MSG_SET_REBOOT				= 0x100,			//重启
	MSG_RET_REBOOT				= 0x200,

	MSG_SET_INIT				= 0x101,			//初始化
	MSG_RET_INIT				= 0x201,
	
	MSG_SET_START				= 0x102,			//开始转码
	MSG_RET_START				= 0x202,
	
	MSG_SET_STOP				= 0x103,			//停止转码
	MSG_RET_STOP				= 0x203,
	
	MSG_SET_VIDEOBITRATE		= 0x104,			//视频码率
	MSG_RET_VIDEOBITRATE		= 0x204,
	
	MSG_SET_AUDIOBITRATE		= 0x105,			//音频码率
	MSG_RET_AUDIOBITRATE		= 0x205,
	
	MSG_SET_OSD0	        	= 0x106,			//0通道OSD
	MSG_RET_OSD0				= 0x206,
	
	MSG_SET_OSD1	        	= 0x107,			//1通道OSD
	MSG_RET_OSD1				= 0x207,
	
	MSG_SET_OSD2	       		= 0x108,			//2通道OSD
	MSG_RET_OSD2				= 0x208,
	
	MSG_SET_OSD3	        	= 0x109,			//3通道OSD
	MSG_RET_OSD3				= 0x209,
	
	MSG_SET_TIME				= 0x110,			//系统时间
	MSG_RET_TIME				= 0x210,
	
	MSG_SET_DSTINFO        		= 0x111,			//上位机信息
	MSG_RET_DSTINFO				= 0x211,

	MSG_SET_VIDEOTUNER			= 0x112,			//视频（电视）调频
	MSG_RET_VIDEOTUNER			= 0x212,
	
	MSG_SET_FMTUNER				= 0x113,			//音频（FM）调频
	MSG_RET_FMTUNER				= 0x213,

	MSG_SET_AMTUNER				= 0x120,			//音频（AM）调频
	MSG_RET_AMTUNER				= 0x220,
	
	MSG_SET_ALERTVIDEO			= 0x114,			//视频报警参数
	MSG_RET_ALERTVIDEO			= 0x214,
	
	MSG_SET_ALERTAUDIO     		= 0x115,			//音频报警参数
	MSG_RET_ALERTAUDIO			= 0x215,
	
	MSG_GET_RUNSTATUS			= 0x116,			//运行状态
	MSG_RET_RUNSTATUS			= 0x216,
	
	MSG_GET_STDCHANSCAN			= 0x117,			//标准频道扫描
	MSG_RET_STDCHANSCAN			= 0x217,
	
	MSG_GET_CHANSCAN			= 0x118,			//全频段扫描
	MSG_RET_CHANSCAN			= 0x218,

	MSG_GET_CHANFIX				= 0x119,			//获取是否锁定
	MSG_RET_CHANFIX				= 0x219,
	
	MSG_RET_ALERT				= 0x301,			//异态上报

	MSG_I2C_OPEN				= 0x401,			//打开I2C设备
	MSG_RET_I2C_OPEN			= 0x501,
	
	MSG_I2C_CLOSE				= 0x402,			//关闭I2C设备
	MSG_RET_I2C_CLOSE			= 0x502,
	
	MSG_I2C_SWITCH				= 0x403,			//选择...
	MSG_RET_I2C_SWITCH			= 0x503,
	
	MSG_I2C_WRITE				= 0x404,			//写I2C设备
	MSG_RET_I2C_WRITE			= 0x504,
	
	MSG_I2C_READ				= 0x405,			//读I2C设备
	MSG_RET_I2C_READ			= 0x505,

	//DVB
	MSG_SET_DIGITALINIT			= 0x150,			//数字卡初始化
	MSG_RET_DIGITALINIT			= 0x250,

	MSG_SET_TUNERPID			= 0x151,			//调频的PID
	MSG_RET_TUNERPID			= 0x251,

	MSG_SET_CODECPID			= 0x152,			//编码后TS流中的PID
	MSG_RET_CODECPID			= 0x252,

	MSG_SET_DIGITALTUNER		= 0x153,			//数字卡调频
	MSG_RET_DIGITALTUNER		= 0x253,

	MSG_SET_HDSD				= 0x154,			//高清节目标识
	MSG_RET_HDSD				= 0x254,

	//指标相关
	MSG_GET_QUA					=0x415,
	MSG_RET_QUA					=0x515,

	MSG_GET_RADIO				=0x416,
	MSG_RET_RADIO				=0x516,

	MSG_GET_SPECIAL_AUDIO_CHANSCAN		=0x417,
	MSG_RET_SPECIAL_AUDIO_CHANSCAN		=0x517,


	//start: 6U设置新加
	MSG_SET_6U_REBOOT				=0x120,				//重启
	MSG_RET_6U_REBOOT				=0x220,

	MSG_SET_6U_SYSTIME				=0x121,				//设置系统时间
	MSG_RET_6U_SYSTIME				=0x221,

	MSG_SET_6U_START				=0x122,				//使指定通道开始转码
	MSG_RET_6U_START				=0x222,

	MSG_SET_6U_STOP					=0x123,				//使指定通道停止转码
	MSG_RET_6U_STOP					=0x223,

	MSG_SET_6U_TUNE					=0x124,				//设置调频
	MSG_RET_6U_TUNE					=0x224,

	MSG_SET_6U_ENCODER				=0x125,				//设置编码参数
	MSG_RET_6U_ENCODER				=0x225,				

	MSG_GET_6U_ENCODERSTATUS		=0x126,				//获取编码参数
	MSG_RET_6U_ENCODERSTATUS		=0x226,

	MSG_SET_6U_SCAN_FM				=0x127,				//FM频道扫描
	MSG_RET_6U_SCAN_FM				=0x227,

	MSG_SET_6U_OSD					=0x128,				//设置OSD
	MSG_RET_6U_OSD					=0x228,

	MSG_SET_6U_ENABLETIMEINFO		=0x129,				//显示时间戳
	MSG_RET_6U_ENABLETIMEINFO		=0x229,

	MSG_SET_6U_VOLUME				=0x130,				//设置音柱
	MSG_RET_6U_VOLUME				=0x230,

	MSG_SET_6U_BACKGROUND			=0x131,				//设置是否显示广播背景
	MSG_RET_6U_BACKGROUND			=0x231,

	MSG_SET_6U_THRESHOLD			=0x132,				//设置报警灵敏度
	MSG_RET_6U_THRESHOLD			=0x232,

	MSG_GET_6U_QUALITY				=0x133,				//获取指标
	MSG_RET_6U_QUALITY				=0x233,

	MSG_SET_6U_SCANPARAM			=0x134,				//设置频道扫描参数
	MSG_RET_6U_SCANPARAM			=0x234,

	MSG_GET_6U_CHANFIX 				=0x135,				//获取频道锁定状态
	MSG_RET_6U_CHANFIX				=0x235,

	MSG_SET_6U_SCAN_ATV				=0x136,				//ATV频道扫描
	MSG_RET_6U_SCAN_ATV				=0x236,

	MSG_GET_6U_ALLQUALITY			=0x137,				//获取全部通道的指标
	MSG_RET_6U_ALLQUALITY			=0x237,

	MSG_GET_6U_ALLCHANFIX			=0x138,				//获取全部通道频道锁定状态
	MSG_RET_6U_ALLCHANFIX			=0x238,

	MSG_SET_6U_CHANGECHAN			=0x139,				//完成一次调台流程
	MSG_RET_6U_CHANGECHAN			=0x239

	//end: 6U设置新加
};

//命令结果标识定义
enum MSG_RETURN
{      
	RET_OK		=	0x1,       //表示设置信息设置成功
	RET_FAIL 	=	0x0,       //表示设置信息设置失败
};

//前端设备信息结构定义
typedef struct _tagDstInfo
{
	char dstIP[30];
	int  dstport;
	int  alarmport;
}DstInfo_Obj, *DstInfo_Handle;

//板卡初始化信息结构定义
typedef struct _tagInitBoard
{
	DstInfo_Obj	dstinfo;		//目标地址信息
	int video_bitrate;			//视频转码默认码率
	int audio_idx;				//音频转码默认码率序号；
	int videosize;				//初始画面大小  videosize=1 为CIF，videosize=2为QCIF
	time_t systime;				//系统时间

	_tagInitBoard()
	{
		video_bitrate	= 700;
		audio_idx		= 5;
		videosize		= 1;
		systime			= time(0);
	};
}InitBoard_Obj, *InitBoard_Handle;

//调频信息结构定义
typedef struct _tagTunerConfig
{
	int chan;  //通道号
	int freq;   //频点 
}TunerConfig_Obj, *TunerConfig_Handle;

//码率信息结构定义
typedef struct _tagMediaConfig
{
	int chan;					//通道
	int video_bitrate;			//视频码率（对广播转码无效）
	int audio_idx;				//音频码率索引

	_tagMediaConfig()
	{
		chan = 0;
		video_bitrate = 400;
		audio_idx = 5;
	};
}MediaConfig_Obj, *MediaConfig_Handle;

//OSD信息结构定义
typedef struct _tagSubtitleConfig
{
	int size;				// 字体大小，默认使用16号，推荐使用16号字

	int subtitle0_Enable;  //是否显示
	char subtitle0[40];    //显示的字幕内容
	int subtitle0_dislen;   //每行显示的长度，字节为单位，每个汉字2个字节
	int subtitle0_x;       //显示位置
	int subtitle0_y;

	int subtitle1_Enable;
	char subtitle1[40];
	int subtitle1_dislen;
	int subtitle1_x;
	int subtitle1_y;

	int time_Enable;     //是否显示时间
	int mode;           //时间显示模式
	int time_x;          //时间显示位置
	int time_y;

	_tagSubtitleConfig()
	{
		size				= 16;
		subtitle0_Enable	= 1;		
		subtitle0_dislen	= 20;
		subtitle0_x			= 10;
		subtitle0_y			= 10;

		subtitle1_Enable	= 1;		
		subtitle1_dislen	= 20;
		subtitle1_x			= 250;
		subtitle1_y			= 260;

		time_Enable			= 1;
		mode				= OSD_CYEAR | OSD_YEAR4 |  OSD_MONTH | OSD_HOUR24;
		time_x				= 160;
		time_y				= 10;
		memset(subtitle0,0,40);
		memset(subtitle1,0,40);
	};
}SubtitleConfig_Obj, *SubtitleConfig_Handle;

//频道扫描结构定义
typedef struct _tagChannelScanConfig
{
	int chan;  			//通道号
	int Startfreq;   	//开始频点
	int Endfreq;		//结束频点
	int step;			//频点累加步长 （标准频点扫描时无意义）
}ChannelScan_Obj, *ChannelScan_Handle;

//报警配置信息结构定义
typedef struct _tagAlertConfig
{
	int video_black;		//黑场
	int color_slice;		//彩条
	int video_static;		//静帧
	int video_miss;			//视频丢失
	int audio_silence;		//静音
}AlertConfig_Obj, *AlertConfig_Handle;

//命令头结构定义
typedef struct _tagPkgHeader
{
	char header;
	int  msg_type;
}PkgHeader_Obj, *PkgHeader_Handle;

//命令结果结构定义
typedef struct _tagRetMessage
{
	PkgHeader_Obj ph;
	int	status;
}
RetMessage_Obj, *RetMessage_Handle;

// 以下是电视广播指标与控制卡通讯的结构定义

//板卡返回特定频点的电平值结构
typedef struct _tagRadioInfo
{
	char flag;      // 区别指标或者频谱 01 频谱 02 指标 
	int freq;		//频点
	int gain;       //增益
}Radiopayload_Obj, * Radiopayload_Handle;

typedef struct _tagSpecialRadioInfo  //针对陕西项目实现的临时解决方案
{
	int startfreq;  //起始频点
	int endfreq;    //结束频点
	int step;       //步长
	int chan;    //通道号
}SpecialRadiopayload_Obj,* SpecialRadiopayload_Handle;

typedef struct _tagSpecialRadioRetMessage  
{
	PkgHeader_Obj ph;
	int	status;
	int len;              //value的长度，
	int value[240];       //如果是频道扫描返回的数据为锁定的频点，如果是频谱扫描返回的为对应频点的电平值
}SpecialRadioRetMessage_Obj,* SpecialRadioRetMessage_Handle;

typedef struct _tagTVQuaRetMessage
{
	PkgHeader_Obj ph;
	int	status;
	int freq;
	int level;
}TVQuaRetMessage_Obj, * TVQuaRetMessage_Handle;

typedef struct _tagRadioQuaRetMessage
{
	PkgHeader_Obj ph;
	int status;			//返回值状态 0失败 1正确
	int freq;			//频点
	int level_int;		//信号电平
	int dev_int;		//调制度
	int dF;				//频偏

	int OBW;			//带宽
	int CNR;			//载噪比
	int FM_Dev;			//调制度
	int FM_Dev_Pos;		//正调制度
	int	FM_Dev_Neg;		//负调制度
	int FM_Dev_RMS;     //RMS调制度
	int Dev_75kHz_Prob; //75kHz分布概率
	int Dev_80kHz_Prob; //80kHz分布概率
	int Dev_85kHz_Prob; //85kHz分布概率
	int Audio_Freq ;	//音频频率
	int Audio_THD;		//总谐波失真
	int Audio_SINAD;	//音频信噪比
}RadioQuaRetMessage_Obj, *RadioQuaRetMessage_Handle;

typedef struct _tagRadioSpecRetMessage
{
	PkgHeader_Obj ph;
	int status;
	int freq;
	int level_int;
	int gain;
	char spec[1024];	//16个为一个频点的，共64个频点	
}RadioSpecRetMessage_Obj, *RadioSpecRetMessage_Handle;

typedef struct _tagChannelScanRetMessage
{
	PkgHeader_Obj ph;
	int status;
	char freq[110];
}ChannelScanRetMessage_Obj,*ChannelScanRetMessage_Handle;
typedef struct _tagChannelScanAudio
{
	int startfreq;
	int endfreq;
}ChanelScanAudio_Obj,*ChannelScanAudio_Handle;

typedef struct _tagChannelScanAudioRetMessage
{
	PkgHeader_Obj ph;
	int status;
	int freqnum;
	int freqs[100];
}ChannelScanAudioRetMessage_Obj,*ChannelsScanAudioRetMessage_Handle;

typedef struct 
{
	int freq;			//频点
	float ImageLevel;  //图像电平,单位:dbuv
	float AudioLevel;  //伴音电平,单位:dbuv
	float I2AOffLevel; //图像载波与伴音载波的电平差,单位:dbuv
	float CN;          //载噪比,单位:dbuv
	float FreqOffSet;  //载频频偏,单位:kHz
	float Slope;       //斜率
}TVQuality;

typedef struct  
{
	std::string Type;
	std::string Desc;
	std::string Value;
}QualityDesc;


typedef struct TypeDesc
{
	std::string Type;
	std::string Desc;
}sTypeDesc;




/*START******************************6U设备*****************************START*/
/*单条OSD叠加文字最大长度*/
#define MAX_OSD_INFO_TEXT_LENGTH  128   
/*报警描述信息文字最大长度*/
#define MAX_ALARM_DESCRIPTION_TEXT_LENGTH	32
/* 返回的最大频点数*/
#define MAX_CHANNEL_SCAN_NUM 300
/*定义台标的TextID为1*/
#define DIGITAL_ON_SCREEN_GRAPHIC_TEXTID 1
/*定义6U板卡最大的通道数量*/
#define MAX_CHANNEL_NUM 8

/*
* Tuner类型枚举
*/
typedef enum Tuner_Type
{
	TUNER_TYPE_TV = 0,			//电视Tuner
	TUNER_TYPE_RADIO,			//广播Tuner
	TUNER_TYPE_COUNT
}Tuner_Type_e;

/*
* 电视调制方式类型枚举
*/
typedef enum Modulation_TV
{
	MODULATION_TV_PALDK = 0,			//PAL-D or PAL-K
	MODULATION_TV_PALI,					//PALI
	MODULATION_TV_SECAM,				//SECAM
	MODULATION_TV_COUNT
}Modulation_TV_e;

/*
* 广播调制方式类型枚举
*/
typedef enum Modulation_Radio
{
	MODULATION_RADIO_FM = 0,			//广播调频
	MODULATION_RADIO_AM, 				//广播中频
	MODULATION_RADIO_COUNT
}Modulation_Radio_e;

/*
* 频道扫描方式枚举
*/
typedef enum Scan_Type
{
	SCAN_TYPE_BAND = 0,				//给定区间频点扫描
	SCAN_TYPE_FAST, 				//快速扫描
	SCAN_TYPE_FULL, 				//全频点扫描
	SCAN_TYPE_COUNT
}Scan_Type_e;

/*
* 频道扫描方式枚举
*/
typedef enum Encode_Control
{
	ENCODE_CTL_CBR = 0,				//固定码率编码
	ENCODE_CTL_VBR, 				//动态码率编码
	ENCODE_CTL_FIXQP, 				//固定QP值编码
	ENCODE_CTL_COUNT
}Encode_Control_e;

/*
* 数据结构定义
*/


/*
* 调谐参数结构
*/
typedef struct Tune_Info
{
	int TunerType;		//Tuner的类型，对应于电视或者广播。取值参考Tuner_Type_e
	int Frequency;		//频率
	union	//调制方式。根据TunerType的不同对应不同的类型，取值参见Modulation_Radio_e
	{
		Modulation_Radio_e RadioModulation;	
		Modulation_TV_e TvModulation;
	};
}Tune_Info_Obj, *Tune_Info_Handle;


typedef struct VSTune_Info
{
	int chanNum;
	Tune_Info_Obj tuneInfo;
}VSTune_Info_Obj, *VSTune_Info_Handle;

/*
* 频道扫描中的参数信息
*/
typedef struct Scan_Param
{
	int TunerType;			//Tuner类型。取值参见 Tuner_Type_e
	int ScanType;			//扫描类型。取值参见 Scan_Type_e
	int StartFrequency;		//起始频率。 以下三项在ScanType为SCAN_TYPE_BAND时有效
	int EndFrequency;		//截止频率
	int StepSize;			//步长
}Scan_Param_Obj, *Scan_Param_Handle;


typedef struct VSScan_Param
{
	int chanNum;
	Scan_Param_Obj scanParam;
}VSScan_Param_Obj, *VSScan_Param_Handle;


/*
* 频道扫描中的结果信息
*/
typedef struct Scan_Result
{
	int ValuedNum;		//有效数据个数
	//需要将指针指向的地址delete吗
	int freqArr[MAX_CHANNEL_SCAN_NUM];	//频点列表，长度为ValuedNum
}Scan_Result_Obj, *Scan_Result_Handle;

/*
* ATV频道扫描时传入的频点信息
*/
typedef struct LockFreq_Param
{
	int chanNum;
	int ValuedNum;		//有效数据个数
	int freqArr[MAX_CHANNEL_SCAN_NUM];	//频点列表，长度为ValuedNum
}LockFreq_Obj, *LockFreq_Handle;

typedef struct VSScan_Result
{
	PkgHeader_Obj ph;
	int status;
	Scan_Result_Obj resultObj;
}VSScan_Result_Obj, *VSScan_Result_Handle;


/*
* OSD 叠加信息
*/
typedef struct OSD_Info
{
	int TextID;			//区分不同的OSD，不重复的标识
	int Font;			//字体
	int Size;			//大小
	int PositionX;		//绘制起始的X坐标。
	int PositionY;		//绘制起始的Y坐标。
	char Text[MAX_OSD_INFO_TEXT_LENGTH];   	//需要显示的文字信息
}OSD_Info_Obj;

typedef struct VSOSD_Info
{
	int chanNum;
	OSD_Info_Obj osdInfo;
}VSOSD_Info_Obj, *VSOSD_Info_Handle;

/*
*时间戳的设置信息
*/
typedef struct VSOSD_Time_info
{
	int chanNum;
	int posX;
	int posY;
	int enable;
}VSOSD_Time_Info_Obj, *VSOSD_Time_info_Handle;

typedef struct VSVolume_Param
{
	int chanNum;
	int enable;
}VSVolume_Param_Obj, *VSVolume_Param_Handle;

typedef struct VSBackGround_Param
{
	int chanNum;
	int enable;
}VSBackGround_Param_Obj, *VSBackGround_Param_Handle;

/*
* 编码参数信息
*/
typedef struct Encode_Param
{
	int VideoBitrate;			//视频编码码率
	int AudioBitrate;			//音频编码码率
	int EncodeType;		    	//视频码率控制模式.取值参见        Encode_Control_e
	int QPValue;		    	//EncodeType为FIXQP时有效.
	int VideoPID;				//视频PID
	int AudioPID;				//音频PID
	int PCRPID;					//PCRPID
	int UDPaddr;				//组播IP地址
	int UDPport;				//组播端口
	int VideoType;				//编码视频类型
	int AudioType;				//编码音频类型
	int Width;					//编码分辨率宽度
	int Height;					//编码分辨率高度

	Encode_Param()
	{
		VideoBitrate = 0;
		AudioBitrate = 0;
		EncodeType   = 0;
		QPValue      = 0;
		VideoPID     = 0;
		AudioPID     = 0;

		PCRPID       = 0;
		UDPaddr      = 0;
		UDPport      = 0;
		VideoType    = 0;
		AudioType    = 0;
		Width        = 0;
		Height       = 0;

	};

	bool	operator ==(const Encode_Param& other)
	{
		return VideoBitrate==other.VideoBitrate&&AudioBitrate==other.AudioBitrate&&EncodeType==other.EncodeType&&QPValue==other.QPValue\
				&&VideoPID==other.VideoPID&&AudioPID==other.AudioPID&&PCRPID==other.PCRPID&&UDPaddr==other.UDPaddr&&UDPport==other.UDPport\
				&&VideoType==other.VideoType&&AudioType==other.AudioType&&Width==other.Width&&Height==other.Height; 
	};

	Encode_Param&	operator =(const Encode_Param& other)
	{
		VideoBitrate = other.VideoBitrate;
		AudioBitrate = other.AudioBitrate;
		EncodeType   = other.EncodeType;
		QPValue      = other.QPValue;
		VideoPID     = other.VideoPID;
		AudioPID     = other.AudioPID;

		PCRPID       = other.PCRPID;
		UDPaddr      = other.UDPaddr;
		UDPport      = other.UDPport;
		VideoType    = other.VideoType;
		AudioType    = other.AudioType;
		Width        = other.Width;
		Height       = other.Height;

		return *this;
	};
}Encode_Param_t;

typedef struct VSEncode_Param
{
	int chanNum;
	Encode_Param_t encodeParam;
}VSEncode_Param_Obj, *VSEncode_Param_Handle;

/*
* 异态报警门限设置信息相关结构体
*/
typedef struct Threshold_Info
{
	int BlackSimilar;			//黑场相似度	百分比
	int ColourBarSimilar;		//彩条相似度
	int FreezeSimilar;			//静帧相似度	百分比
	int VolumeHighValue;		//音量过高  	db
	int VolumeLowValue;			//音量过低  	db
	int AudioLostValue;			//无声音  		db
	int AudioUnsualLastTime;	//声音异常上报的最短持续时间，单位毫秒
}Threshold_Info_Obj, *Threshold_Info_Handle;

/*报警类型枚举*/
typedef enum Alarm_Type
{
	ALARM_FREEZE = 10,		//静帧相似度
	ALARM_BLACK,			//黑场相似度		
	ALARM_COLORBAR,			//彩条
	ALARM_VOLUMEHIGH,		//音量过高
	ALARM_VOLUMELOW,		//音量过低
	ALARM_AUDIOLOST,		//音频丢失  无声音
	ALARM_LOSTSIGNAL,		//失锁
	ALARM_LOSTVIDEO			//无视频
}Alarm_Type_e;

/**********************************************************************************************************
* 报警信息
**********************************************************************************************************/
typedef struct Alarm_Info
{
	int AlarmType;			//报警类型
	int CodeValue;			//异常值
	int ThresholdValue;		//当前门限值
	int ChannelNO;			//通道号
	long long AlarmTimeMs;	//报警时间ms
    int isAlarm;            //1 异常  0正常
}Alarm_Info_t;


typedef struct VSThreshold_Info
{
	int chanNum;
	Threshold_Info_Obj thresholdInfo;
}VSThreshold_Info_Obj, *VSThreshold_Info_Handle;

/*
* 指标信息
*/
typedef struct Tune_Signal_Status_Radio
{
	int status;
	int level;
	int usn;
	int wam;
	int offset;
	int bandwith;
	int modulation;
}Tune_Signal_Status_Radio_t;

typedef struct Tuner_Quality_Param
{
	int chanNum;
	int TunerType;
}VSTuner_Quality_Param, *VSTuner_Quality_Param_Handle;

typedef struct Tuner_Quality_Result
{
	PkgHeader_Obj ph;
	int status;
	Tune_Signal_Status_Radio_t tunerQualityObj;
}VSTuner_Quality_Result_Obj, *VSTuner_Quality_Result_Handle;

/*定义查询全部的指标参数*/
typedef struct Tuner_AllQuality_Param
{
	int TunerType;
}VSTuner_AllQuality_Param, *VSTuner_AllQuality_Param_Handle;

typedef struct Tuner_AllQuality_Result
{
	PkgHeader_Obj ph;
	int status;
	Tune_Signal_Status_Radio_t tunerQualityObj[MAX_CHANNEL_NUM];
}VSTuner_AllQuality_Result_Obj, *VSTuner_AllQuality_Result_Handle;


typedef struct Rtc_Time
{
	unsigned int  year;
	unsigned int  month;
	unsigned int  date;
	unsigned int  hour;
	unsigned int  minute;
	unsigned int  second;
	unsigned int  weekday;
}Rtc_Time_t;

typedef struct Scan_Control_Param
{
	int FmLevel;
	int Amlevel;
	int FmUsn;
	int FmWan;
}Scan_Control_Param_t;

/*
 * Tuner信号状态结构----电视Tuner
 */
typedef struct Tune_Signal_Status_Tv
{
	int status;
	int strength;
}Tune_Signal_Status_Tv_t;

/*
*自定义板卡和主程序之间获取频道锁定信息的数据结构
*/
typedef struct Tuner_ChanFix_Param
{
	int chanNum;
	Tuner_Type_e TunerType;
}VSTuner_ChanFix_Param_Obj, *VSTuner_ChanFix_Param_Handle;

typedef struct Tuner_ChanFix_Result
{
	PkgHeader_Obj ph;
	int status;
	Tune_Signal_Status_Tv_t ChanFixObj;
}VSTuner_ChanFix_Result_Obj, *VSTuner_ChanFix_Result_Handle;

typedef struct Tuner_AllChanFix_Param
{
	Tuner_Type_e TunerType;
}VSTuner_AllChanFix_Param_Obj, *VSTuner_AllChanFix_Param_Handle;

typedef struct Tuner_AllChanFix_Result
{
	PkgHeader_Obj ph;
	int status;
	Tune_Signal_Status_Tv_t ChanFixObj[MAX_CHANNEL_NUM];
}VSTuner_AllChanFix_Result_Obj, *VSTuner_AllChanFix_Result_Handle;


typedef struct VSChangeChan_Param
{
	int chanNum;
	Tune_Info_Obj tuneInfo;
	Encode_Param_t encodeInfo;
	OSD_Info_Obj osdInfo;
	VSOSD_Time_Info_Obj timeInfo;
	int volumeEnable;
	int bgEnable;
}VSChangeChan_Param_Obj, *VSChangeChan_Param_Handle;

/*START******************************6U设备*****************************START*/

#endif