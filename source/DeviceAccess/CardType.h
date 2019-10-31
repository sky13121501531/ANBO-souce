#pragma once

#ifndef  _CARDTYPE_H_
#define _CARDTYPE_H_

#include <time.h>
#include <iostream>

//ʱ������ģʽ
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

//��������
#define V_BLACK     0x1          //��Ƶ�ڳ�
#define C_SLICE     0x2          //����
#define V_STATIC	0x4          //���澲ֹ
#define V_MISS      0x8          //����Ƶ�ź�
#define S_UNLOCK	0xE          //���ز�
#define A_SILENCE	0x10         //��Ƶ����

#define NO_SIGNAL   0x81         //���ź�
#define NO_VIDEO    0x82         //����Ƶ
#define NO_AUDIO    0x84         //����Ƶ
#define UNCA_VIDEO  0x88         //��ƵCAʧ��
#define UNCA_AUDIO  0x90         //��ƵCAʧ��


#define V_BLACK_V_STATIC     0x5          //��Ƶ�ڳ�+���澲ֹ
#define C_SLICE_V_STATIC     0x6          //����+���澲ֹ
#define CARDREBOOT			 0xFFFF		  //�忨����


//��Ϣ���Ͷ���
enum MSG_TYPE
{
	MSG_UNKNOWN					= 0x0,				//δ֪��Ϣ
	
	MSG_SET_REBOOT				= 0x100,			//����
	MSG_RET_REBOOT				= 0x200,

	MSG_SET_INIT				= 0x101,			//��ʼ��
	MSG_RET_INIT				= 0x201,
	
	MSG_SET_START				= 0x102,			//��ʼת��
	MSG_RET_START				= 0x202,
	
	MSG_SET_STOP				= 0x103,			//ֹͣת��
	MSG_RET_STOP				= 0x203,
	
	MSG_SET_VIDEOBITRATE		= 0x104,			//��Ƶ����
	MSG_RET_VIDEOBITRATE		= 0x204,
	
	MSG_SET_AUDIOBITRATE		= 0x105,			//��Ƶ����
	MSG_RET_AUDIOBITRATE		= 0x205,
	
	MSG_SET_OSD0	        	= 0x106,			//0ͨ��OSD
	MSG_RET_OSD0				= 0x206,
	
	MSG_SET_OSD1	        	= 0x107,			//1ͨ��OSD
	MSG_RET_OSD1				= 0x207,
	
	MSG_SET_OSD2	       		= 0x108,			//2ͨ��OSD
	MSG_RET_OSD2				= 0x208,
	
	MSG_SET_OSD3	        	= 0x109,			//3ͨ��OSD
	MSG_RET_OSD3				= 0x209,
	
	MSG_SET_TIME				= 0x110,			//ϵͳʱ��
	MSG_RET_TIME				= 0x210,
	
	MSG_SET_DSTINFO        		= 0x111,			//��λ����Ϣ
	MSG_RET_DSTINFO				= 0x211,

	MSG_SET_VIDEOTUNER			= 0x112,			//��Ƶ�����ӣ���Ƶ
	MSG_RET_VIDEOTUNER			= 0x212,
	
	MSG_SET_FMTUNER				= 0x113,			//��Ƶ��FM����Ƶ
	MSG_RET_FMTUNER				= 0x213,

	MSG_SET_AMTUNER				= 0x120,			//��Ƶ��AM����Ƶ
	MSG_RET_AMTUNER				= 0x220,
	
	MSG_SET_ALERTVIDEO			= 0x114,			//��Ƶ��������
	MSG_RET_ALERTVIDEO			= 0x214,
	
	MSG_SET_ALERTAUDIO     		= 0x115,			//��Ƶ��������
	MSG_RET_ALERTAUDIO			= 0x215,
	
	MSG_GET_RUNSTATUS			= 0x116,			//����״̬
	MSG_RET_RUNSTATUS			= 0x216,
	
	MSG_GET_STDCHANSCAN			= 0x117,			//��׼Ƶ��ɨ��
	MSG_RET_STDCHANSCAN			= 0x217,
	
	MSG_GET_CHANSCAN			= 0x118,			//ȫƵ��ɨ��
	MSG_RET_CHANSCAN			= 0x218,

	MSG_GET_CHANFIX				= 0x119,			//��ȡ�Ƿ�����
	MSG_RET_CHANFIX				= 0x219,
	
	MSG_RET_ALERT				= 0x301,			//��̬�ϱ�

	MSG_I2C_OPEN				= 0x401,			//��I2C�豸
	MSG_RET_I2C_OPEN			= 0x501,
	
	MSG_I2C_CLOSE				= 0x402,			//�ر�I2C�豸
	MSG_RET_I2C_CLOSE			= 0x502,
	
	MSG_I2C_SWITCH				= 0x403,			//ѡ��...
	MSG_RET_I2C_SWITCH			= 0x503,
	
	MSG_I2C_WRITE				= 0x404,			//дI2C�豸
	MSG_RET_I2C_WRITE			= 0x504,
	
	MSG_I2C_READ				= 0x405,			//��I2C�豸
	MSG_RET_I2C_READ			= 0x505,

	//DVB
	MSG_SET_DIGITALINIT			= 0x150,			//���ֿ���ʼ��
	MSG_RET_DIGITALINIT			= 0x250,

	MSG_SET_TUNERPID			= 0x151,			//��Ƶ��PID
	MSG_RET_TUNERPID			= 0x251,

	MSG_SET_CODECPID			= 0x152,			//�����TS���е�PID
	MSG_RET_CODECPID			= 0x252,

	MSG_SET_DIGITALTUNER		= 0x153,			//���ֿ���Ƶ
	MSG_RET_DIGITALTUNER		= 0x253,

	MSG_SET_HDSD				= 0x154,			//�����Ŀ��ʶ
	MSG_RET_HDSD				= 0x254,

	//ָ�����
	MSG_GET_QUA					=0x415,
	MSG_RET_QUA					=0x515,

	MSG_GET_RADIO				=0x416,
	MSG_RET_RADIO				=0x516,

	MSG_GET_SPECIAL_AUDIO_CHANSCAN		=0x417,
	MSG_RET_SPECIAL_AUDIO_CHANSCAN		=0x517,


	//start: 6U�����¼�
	MSG_SET_6U_REBOOT				=0x120,				//����
	MSG_RET_6U_REBOOT				=0x220,

	MSG_SET_6U_SYSTIME				=0x121,				//����ϵͳʱ��
	MSG_RET_6U_SYSTIME				=0x221,

	MSG_SET_6U_START				=0x122,				//ʹָ��ͨ����ʼת��
	MSG_RET_6U_START				=0x222,

	MSG_SET_6U_STOP					=0x123,				//ʹָ��ͨ��ֹͣת��
	MSG_RET_6U_STOP					=0x223,

	MSG_SET_6U_TUNE					=0x124,				//���õ�Ƶ
	MSG_RET_6U_TUNE					=0x224,

	MSG_SET_6U_ENCODER				=0x125,				//���ñ������
	MSG_RET_6U_ENCODER				=0x225,				

	MSG_GET_6U_ENCODERSTATUS		=0x126,				//��ȡ�������
	MSG_RET_6U_ENCODERSTATUS		=0x226,

	MSG_SET_6U_SCAN_FM				=0x127,				//FMƵ��ɨ��
	MSG_RET_6U_SCAN_FM				=0x227,

	MSG_SET_6U_OSD					=0x128,				//����OSD
	MSG_RET_6U_OSD					=0x228,

	MSG_SET_6U_ENABLETIMEINFO		=0x129,				//��ʾʱ���
	MSG_RET_6U_ENABLETIMEINFO		=0x229,

	MSG_SET_6U_VOLUME				=0x130,				//��������
	MSG_RET_6U_VOLUME				=0x230,

	MSG_SET_6U_BACKGROUND			=0x131,				//�����Ƿ���ʾ�㲥����
	MSG_RET_6U_BACKGROUND			=0x231,

	MSG_SET_6U_THRESHOLD			=0x132,				//���ñ���������
	MSG_RET_6U_THRESHOLD			=0x232,

	MSG_GET_6U_QUALITY				=0x133,				//��ȡָ��
	MSG_RET_6U_QUALITY				=0x233,

	MSG_SET_6U_SCANPARAM			=0x134,				//����Ƶ��ɨ�����
	MSG_RET_6U_SCANPARAM			=0x234,

	MSG_GET_6U_CHANFIX 				=0x135,				//��ȡƵ������״̬
	MSG_RET_6U_CHANFIX				=0x235,

	MSG_SET_6U_SCAN_ATV				=0x136,				//ATVƵ��ɨ��
	MSG_RET_6U_SCAN_ATV				=0x236,

	MSG_GET_6U_ALLQUALITY			=0x137,				//��ȡȫ��ͨ����ָ��
	MSG_RET_6U_ALLQUALITY			=0x237,

	MSG_GET_6U_ALLCHANFIX			=0x138,				//��ȡȫ��ͨ��Ƶ������״̬
	MSG_RET_6U_ALLCHANFIX			=0x238,

	MSG_SET_6U_CHANGECHAN			=0x139,				//���һ�ε�̨����
	MSG_RET_6U_CHANGECHAN			=0x239

	//end: 6U�����¼�
};

//��������ʶ����
enum MSG_RETURN
{      
	RET_OK		=	0x1,       //��ʾ������Ϣ���óɹ�
	RET_FAIL 	=	0x0,       //��ʾ������Ϣ����ʧ��
};

//ǰ���豸��Ϣ�ṹ����
typedef struct _tagDstInfo
{
	char dstIP[30];
	int  dstport;
	int  alarmport;
}DstInfo_Obj, *DstInfo_Handle;

//�忨��ʼ����Ϣ�ṹ����
typedef struct _tagInitBoard
{
	DstInfo_Obj	dstinfo;		//Ŀ���ַ��Ϣ
	int video_bitrate;			//��Ƶת��Ĭ������
	int audio_idx;				//��Ƶת��Ĭ��������ţ�
	int videosize;				//��ʼ�����С  videosize=1 ΪCIF��videosize=2ΪQCIF
	time_t systime;				//ϵͳʱ��

	_tagInitBoard()
	{
		video_bitrate	= 700;
		audio_idx		= 5;
		videosize		= 1;
		systime			= time(0);
	};
}InitBoard_Obj, *InitBoard_Handle;

//��Ƶ��Ϣ�ṹ����
typedef struct _tagTunerConfig
{
	int chan;  //ͨ����
	int freq;   //Ƶ�� 
}TunerConfig_Obj, *TunerConfig_Handle;

//������Ϣ�ṹ����
typedef struct _tagMediaConfig
{
	int chan;					//ͨ��
	int video_bitrate;			//��Ƶ���ʣ��Թ㲥ת����Ч��
	int audio_idx;				//��Ƶ��������

	_tagMediaConfig()
	{
		chan = 0;
		video_bitrate = 400;
		audio_idx = 5;
	};
}MediaConfig_Obj, *MediaConfig_Handle;

//OSD��Ϣ�ṹ����
typedef struct _tagSubtitleConfig
{
	int size;				// �����С��Ĭ��ʹ��16�ţ��Ƽ�ʹ��16����

	int subtitle0_Enable;  //�Ƿ���ʾ
	char subtitle0[40];    //��ʾ����Ļ����
	int subtitle0_dislen;   //ÿ����ʾ�ĳ��ȣ��ֽ�Ϊ��λ��ÿ������2���ֽ�
	int subtitle0_x;       //��ʾλ��
	int subtitle0_y;

	int subtitle1_Enable;
	char subtitle1[40];
	int subtitle1_dislen;
	int subtitle1_x;
	int subtitle1_y;

	int time_Enable;     //�Ƿ���ʾʱ��
	int mode;           //ʱ����ʾģʽ
	int time_x;          //ʱ����ʾλ��
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

//Ƶ��ɨ��ṹ����
typedef struct _tagChannelScanConfig
{
	int chan;  			//ͨ����
	int Startfreq;   	//��ʼƵ��
	int Endfreq;		//����Ƶ��
	int step;			//Ƶ���ۼӲ��� ����׼Ƶ��ɨ��ʱ�����壩
}ChannelScan_Obj, *ChannelScan_Handle;

//����������Ϣ�ṹ����
typedef struct _tagAlertConfig
{
	int video_black;		//�ڳ�
	int color_slice;		//����
	int video_static;		//��֡
	int video_miss;			//��Ƶ��ʧ
	int audio_silence;		//����
}AlertConfig_Obj, *AlertConfig_Handle;

//����ͷ�ṹ����
typedef struct _tagPkgHeader
{
	char header;
	int  msg_type;
}PkgHeader_Obj, *PkgHeader_Handle;

//�������ṹ����
typedef struct _tagRetMessage
{
	PkgHeader_Obj ph;
	int	status;
}
RetMessage_Obj, *RetMessage_Handle;

// �����ǵ��ӹ㲥ָ������ƿ�ͨѶ�Ľṹ����

//�忨�����ض�Ƶ��ĵ�ƽֵ�ṹ
typedef struct _tagRadioInfo
{
	char flag;      // ����ָ�����Ƶ�� 01 Ƶ�� 02 ָ�� 
	int freq;		//Ƶ��
	int gain;       //����
}Radiopayload_Obj, * Radiopayload_Handle;

typedef struct _tagSpecialRadioInfo  //���������Ŀʵ�ֵ���ʱ�������
{
	int startfreq;  //��ʼƵ��
	int endfreq;    //����Ƶ��
	int step;       //����
	int chan;    //ͨ����
}SpecialRadiopayload_Obj,* SpecialRadiopayload_Handle;

typedef struct _tagSpecialRadioRetMessage  
{
	PkgHeader_Obj ph;
	int	status;
	int len;              //value�ĳ��ȣ�
	int value[240];       //�����Ƶ��ɨ�践�ص�����Ϊ������Ƶ�㣬�����Ƶ��ɨ�践�ص�Ϊ��ӦƵ��ĵ�ƽֵ
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
	int status;			//����ֵ״̬ 0ʧ�� 1��ȷ
	int freq;			//Ƶ��
	int level_int;		//�źŵ�ƽ
	int dev_int;		//���ƶ�
	int dF;				//Ƶƫ

	int OBW;			//����
	int CNR;			//�����
	int FM_Dev;			//���ƶ�
	int FM_Dev_Pos;		//�����ƶ�
	int	FM_Dev_Neg;		//�����ƶ�
	int FM_Dev_RMS;     //RMS���ƶ�
	int Dev_75kHz_Prob; //75kHz�ֲ�����
	int Dev_80kHz_Prob; //80kHz�ֲ�����
	int Dev_85kHz_Prob; //85kHz�ֲ�����
	int Audio_Freq ;	//��ƵƵ��
	int Audio_THD;		//��г��ʧ��
	int Audio_SINAD;	//��Ƶ�����
}RadioQuaRetMessage_Obj, *RadioQuaRetMessage_Handle;

typedef struct _tagRadioSpecRetMessage
{
	PkgHeader_Obj ph;
	int status;
	int freq;
	int level_int;
	int gain;
	char spec[1024];	//16��Ϊһ��Ƶ��ģ���64��Ƶ��	
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
	int freq;			//Ƶ��
	float ImageLevel;  //ͼ���ƽ,��λ:dbuv
	float AudioLevel;  //������ƽ,��λ:dbuv
	float I2AOffLevel; //ͼ���ز�������ز��ĵ�ƽ��,��λ:dbuv
	float CN;          //�����,��λ:dbuv
	float FreqOffSet;  //��ƵƵƫ,��λ:kHz
	float Slope;       //б��
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




/*START******************************6U�豸*****************************START*/
/*����OSD����������󳤶�*/
#define MAX_OSD_INFO_TEXT_LENGTH  128   
/*����������Ϣ������󳤶�*/
#define MAX_ALARM_DESCRIPTION_TEXT_LENGTH	32
/* ���ص����Ƶ����*/
#define MAX_CHANNEL_SCAN_NUM 300
/*����̨���TextIDΪ1*/
#define DIGITAL_ON_SCREEN_GRAPHIC_TEXTID 1
/*����6U�忨����ͨ������*/
#define MAX_CHANNEL_NUM 8

/*
* Tuner����ö��
*/
typedef enum Tuner_Type
{
	TUNER_TYPE_TV = 0,			//����Tuner
	TUNER_TYPE_RADIO,			//�㲥Tuner
	TUNER_TYPE_COUNT
}Tuner_Type_e;

/*
* ���ӵ��Ʒ�ʽ����ö��
*/
typedef enum Modulation_TV
{
	MODULATION_TV_PALDK = 0,			//PAL-D or PAL-K
	MODULATION_TV_PALI,					//PALI
	MODULATION_TV_SECAM,				//SECAM
	MODULATION_TV_COUNT
}Modulation_TV_e;

/*
* �㲥���Ʒ�ʽ����ö��
*/
typedef enum Modulation_Radio
{
	MODULATION_RADIO_FM = 0,			//�㲥��Ƶ
	MODULATION_RADIO_AM, 				//�㲥��Ƶ
	MODULATION_RADIO_COUNT
}Modulation_Radio_e;

/*
* Ƶ��ɨ�跽ʽö��
*/
typedef enum Scan_Type
{
	SCAN_TYPE_BAND = 0,				//��������Ƶ��ɨ��
	SCAN_TYPE_FAST, 				//����ɨ��
	SCAN_TYPE_FULL, 				//ȫƵ��ɨ��
	SCAN_TYPE_COUNT
}Scan_Type_e;

/*
* Ƶ��ɨ�跽ʽö��
*/
typedef enum Encode_Control
{
	ENCODE_CTL_CBR = 0,				//�̶����ʱ���
	ENCODE_CTL_VBR, 				//��̬���ʱ���
	ENCODE_CTL_FIXQP, 				//�̶�QPֵ����
	ENCODE_CTL_COUNT
}Encode_Control_e;

/*
* ���ݽṹ����
*/


/*
* ��г�����ṹ
*/
typedef struct Tune_Info
{
	int TunerType;		//Tuner�����ͣ���Ӧ�ڵ��ӻ��߹㲥��ȡֵ�ο�Tuner_Type_e
	int Frequency;		//Ƶ��
	union	//���Ʒ�ʽ������TunerType�Ĳ�ͬ��Ӧ��ͬ�����ͣ�ȡֵ�μ�Modulation_Radio_e
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
* Ƶ��ɨ���еĲ�����Ϣ
*/
typedef struct Scan_Param
{
	int TunerType;			//Tuner���͡�ȡֵ�μ� Tuner_Type_e
	int ScanType;			//ɨ�����͡�ȡֵ�μ� Scan_Type_e
	int StartFrequency;		//��ʼƵ�ʡ� ����������ScanTypeΪSCAN_TYPE_BANDʱ��Ч
	int EndFrequency;		//��ֹƵ��
	int StepSize;			//����
}Scan_Param_Obj, *Scan_Param_Handle;


typedef struct VSScan_Param
{
	int chanNum;
	Scan_Param_Obj scanParam;
}VSScan_Param_Obj, *VSScan_Param_Handle;


/*
* Ƶ��ɨ���еĽ����Ϣ
*/
typedef struct Scan_Result
{
	int ValuedNum;		//��Ч���ݸ���
	//��Ҫ��ָ��ָ��ĵ�ַdelete��
	int freqArr[MAX_CHANNEL_SCAN_NUM];	//Ƶ���б�����ΪValuedNum
}Scan_Result_Obj, *Scan_Result_Handle;

/*
* ATVƵ��ɨ��ʱ�����Ƶ����Ϣ
*/
typedef struct LockFreq_Param
{
	int chanNum;
	int ValuedNum;		//��Ч���ݸ���
	int freqArr[MAX_CHANNEL_SCAN_NUM];	//Ƶ���б�����ΪValuedNum
}LockFreq_Obj, *LockFreq_Handle;

typedef struct VSScan_Result
{
	PkgHeader_Obj ph;
	int status;
	Scan_Result_Obj resultObj;
}VSScan_Result_Obj, *VSScan_Result_Handle;


/*
* OSD ������Ϣ
*/
typedef struct OSD_Info
{
	int TextID;			//���ֲ�ͬ��OSD�����ظ��ı�ʶ
	int Font;			//����
	int Size;			//��С
	int PositionX;		//������ʼ��X���ꡣ
	int PositionY;		//������ʼ��Y���ꡣ
	char Text[MAX_OSD_INFO_TEXT_LENGTH];   	//��Ҫ��ʾ��������Ϣ
}OSD_Info_Obj;

typedef struct VSOSD_Info
{
	int chanNum;
	OSD_Info_Obj osdInfo;
}VSOSD_Info_Obj, *VSOSD_Info_Handle;

/*
*ʱ�����������Ϣ
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
* ���������Ϣ
*/
typedef struct Encode_Param
{
	int VideoBitrate;			//��Ƶ��������
	int AudioBitrate;			//��Ƶ��������
	int EncodeType;		    	//��Ƶ���ʿ���ģʽ.ȡֵ�μ�        Encode_Control_e
	int QPValue;		    	//EncodeTypeΪFIXQPʱ��Ч.
	int VideoPID;				//��ƵPID
	int AudioPID;				//��ƵPID
	int PCRPID;					//PCRPID
	int UDPaddr;				//�鲥IP��ַ
	int UDPport;				//�鲥�˿�
	int VideoType;				//������Ƶ����
	int AudioType;				//������Ƶ����
	int Width;					//����ֱ��ʿ��
	int Height;					//����ֱ��ʸ߶�

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
* ��̬��������������Ϣ��ؽṹ��
*/
typedef struct Threshold_Info
{
	int BlackSimilar;			//�ڳ����ƶ�	�ٷֱ�
	int ColourBarSimilar;		//�������ƶ�
	int FreezeSimilar;			//��֡���ƶ�	�ٷֱ�
	int VolumeHighValue;		//��������  	db
	int VolumeLowValue;			//��������  	db
	int AudioLostValue;			//������  		db
	int AudioUnsualLastTime;	//�����쳣�ϱ�����̳���ʱ�䣬��λ����
}Threshold_Info_Obj, *Threshold_Info_Handle;

/*��������ö��*/
typedef enum Alarm_Type
{
	ALARM_FREEZE = 10,		//��֡���ƶ�
	ALARM_BLACK,			//�ڳ����ƶ�		
	ALARM_COLORBAR,			//����
	ALARM_VOLUMEHIGH,		//��������
	ALARM_VOLUMELOW,		//��������
	ALARM_AUDIOLOST,		//��Ƶ��ʧ  ������
	ALARM_LOSTSIGNAL,		//ʧ��
	ALARM_LOSTVIDEO			//����Ƶ
}Alarm_Type_e;

/**********************************************************************************************************
* ������Ϣ
**********************************************************************************************************/
typedef struct Alarm_Info
{
	int AlarmType;			//��������
	int CodeValue;			//�쳣ֵ
	int ThresholdValue;		//��ǰ����ֵ
	int ChannelNO;			//ͨ����
	long long AlarmTimeMs;	//����ʱ��ms
    int isAlarm;            //1 �쳣  0����
}Alarm_Info_t;


typedef struct VSThreshold_Info
{
	int chanNum;
	Threshold_Info_Obj thresholdInfo;
}VSThreshold_Info_Obj, *VSThreshold_Info_Handle;

/*
* ָ����Ϣ
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

/*�����ѯȫ����ָ�����*/
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
 * Tuner�ź�״̬�ṹ----����Tuner
 */
typedef struct Tune_Signal_Status_Tv
{
	int status;
	int strength;
}Tune_Signal_Status_Tv_t;

/*
*�Զ���忨��������֮���ȡƵ��������Ϣ�����ݽṹ
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

/*START******************************6U�豸*****************************START*/

#endif