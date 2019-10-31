
#include "CheckLevelForUnLock.h"

#include "GetLevelForUnLockForATV.h"
#include "GetLevelForUnLockForAM.h"
#include "GetLevelForUnLockForRADIO.h"

#include "AlarmMgr.h"
#include "ace/OS.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/AlarmPropManager.h"
#include "../Communications/RecordQualitySender.h"
#include "../DBAccess/DBManager.h"

CheckLevelForUnLock::CheckLevelForUnLock(void)
{	
	AlarmRecordInfo.clear();
}

CheckLevelForUnLock::~CheckLevelForUnLock(void)
{
}
bool CheckLevelForUnLock::Start()
{
	bFlag = true;
	this->open(0);
	return true;
}

int CheckLevelForUnLock::open( void* )
{
	bFlag=true;
	activate();
	return 0;
}
int CheckLevelForUnLock::svc()
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)录像失锁(电平)判断线程开始执行 !\n"));
	while(bFlag)
	{
		if(AlarmRecordInfo.size()==0)
		{
			OSFunction::Sleep(0,500);
			continue;
		}
		ProcessQuality();
		OSFunction::Sleep(0,800);
		CheckMutex.acquire();
		std::vector<sSignalCheck>::iterator ptr=AlarmRecordInfo.begin();
		for(;ptr!=AlarmRecordInfo.end();ptr++)
		{
			CheckLock(*ptr);
		}
		CheckMutex.release();
		InsertLevelToDB();
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)录像失锁(电平)判断线程停止执行 !\n"));
	return 0;
}
int CheckLevelForUnLock::Stop()
{
	bFlag=false;
	this->wait();
	return 0;
}
bool CheckLevelForUnLock::AddRecordInfo(sSignalCheck Info)
{
	CheckMutex.acquire();
	if (Info.dvbtype == ATV)
	{
		GETLEVELFORUNLOCKFORATV::instance()->AddRecordInfo(Info);
	}
	if (Info.dvbtype == RADIO)
	{
		GETLEVELFORUNLOCKFORRADIO::instance()->AddRecordInfo(Info);
	}
	//else if (Info.dvbtype == AM)
	//{
	//	GETLEVELFORUNLOCKFORAM::instance()->AddRecordInfo(Info);
	//}
	CheckMutex.release();
	
	return true;
}
bool CheckLevelForUnLock::RemoveRecordInfo(sSignalCheck Info)
{

	CheckMutex.acquire();
	std::vector<sSignalCheck>::iterator ptr=AlarmRecordInfo.begin();
	for(;ptr!=AlarmRecordInfo.end();ptr++)
	{
		if( (*ptr).dvbtype==Info.dvbtype && (*ptr).ChannelID==Info.ChannelID && (*ptr).DeviceID==Info.DeviceID)
		{
			AlarmRecordInfo.erase(ptr);
			break;
		}
	}
	if (Info.dvbtype == ATV)
	{
		GETLEVELFORUNLOCKFORATV::instance()->RemoveRecordInfo(Info);
	}
	else if (Info.dvbtype == RADIO)
	{
		GETLEVELFORUNLOCKFORRADIO::instance()->RemoveRecordInfo(Info);
	}
	else if (Info.dvbtype == AM)
	{
		GETLEVELFORUNLOCKFORAM::instance()->RemoveRecordInfo(Info);
	}
	CheckMutex.release();
	return true;
}

bool CheckLevelForUnLock::CheckLock(sSignalCheck param)
{
	if (param.level <= 0)
		return false;
	
	std::string downthreshod;
	bool ret=ALARMPROPMANAGER::instance()->GetDownThreShold(param.dvbtype,param.Freq,downthreshod);		//频点本身门限
	if (ret==false)
	{
		ret=ALARMPROPMANAGER::instance()->GetDownThreShold(param.dvbtype,std::string("All"),downthreshod);//通用门限
	}
	if (ret==false)							//无门限值，不做无载波报警判断
		return true;

	int i_downthreshod = StrUtil::Str2Int(downthreshod);
	if (param.level<=i_downthreshod)		//如果测量值小于门限值则认为报警
	{
		sCheckParam sCheck;
		sCheck.AlarmType	= ALARM_PROGRAM;
		sCheck.DVBType		= param.dvbtype;
		sCheck.ChannelID	= param.ChannelID;
		sCheck.Freq			= param.Freq;
		sCheck.STD			= "";
		sCheck.SymbolRate	= "";
		sCheck.TypeDesc		= string("无载波");
		sCheck.TypedValue	= "";
		sCheck.TypeID		= "0xE";
		sCheck.DeviceID		= param.DeviceID;
		sCheck.CheckTime	= time(0);

#ifdef _DEBUG
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%s]节目异态:%s\n",sCheck.DeviceID.c_str(),sCheck.TypeDesc.c_str()));
#endif
		ALARMMGR::instance()->CheckAlarm(sCheck,true);

		return true;
	}
	return false;
}
bool CheckLevelForUnLock::UpdateLevel(sSignalCheck Info)
{
	CheckMutex.acquire();
	std::vector<sSignalCheck>::iterator ptr=AlarmRecordInfo.begin();
	for(;ptr!=AlarmRecordInfo.end();ptr++)
	{
		if((*ptr)==Info)
		{
			(*ptr).level=Info.level;
			break;
		}
	}
	CheckMutex.release();
	return true;
}

bool  CheckLevelForUnLock::GetQualityLevel(string freq,int& level)
{
	CheckMutex.acquire();
	std::vector<sSignalCheck>::iterator ptr=AlarmRecordInfo.begin();
	for(;ptr!=AlarmRecordInfo.end();ptr++)
	{
		if((*ptr).Freq==freq)
		{
			level=(*ptr).level;
		}
	}
	CheckMutex.release();
	return false;
}

void CheckLevelForUnLock::AddQualityFreq(std::string freq)
{
	QualityMutex.acquire();
	vecFreq.push_back(freq);
	QualityMutex.release();
}
void CheckLevelForUnLock::RemoveQualityFreq(std::string freq)
{
	QualityMutex.acquire();
	for(std::vector<std::string>::iterator itor=vecFreq.begin();itor!=vecFreq.end();itor++)
	{
		if((*itor)==freq)
		{
			vecFreq.erase(itor);
			break;
		}
	}
	QualityMutex.release();
}
void CheckLevelForUnLock::ProcessQuality()
{
	QualityMutex.acquire();
	for(std::vector<std::string>::iterator itor=vecFreq.begin();itor!=vecFreq.end();itor++)
	{
		int level=0;
		GetQualityLevel((*itor),level);
		level=1000*level;//给页面返回的电平值需要乘以1000
		int iFreq=(int)(1000*StrUtil::Str2Float((*itor)));
		bool retVal = false;
		int TcpDataLen = 0;
		int templen = 4;
		unsigned char ch[128];
		memset(ch,0,128*sizeof(char));
		memcpy(&ch,&iFreq,4);
		ch[4]=0;
		ch[5]=4;
		templen=10;
		try
		{
			//保存data_type_tag	接口类型。电视实时指标测量上报是”TRQU”。
			memcpy(&ch[templen],"TRQU",4*sizeof(char));
			templen = templen + 4*sizeof(char);

			//保存report_time	上报时间, 从1970年1月1日0时0分0秒到该时间点所经过的秒
			time_t t64 = time(0);
			memcpy(&ch[templen],&t64,8*sizeof(char));
			templen = templen + 8*sizeof(char);

			memcpy(&ch[templen],&iFreq,4*sizeof(char));
			templen = templen + 4*sizeof(char);

			//保存data_length	数据长度
			short datalen = 1;
			memcpy(&ch[templen],&datalen,2*sizeof(char));
			templen = templen + 2*sizeof(char);


			ch[templen] = 1;
			templen = templen + 1*sizeof(char);
			memcpy(&ch[templen],&level,4*sizeof(char));
			templen = templen + 4*sizeof(char);
			int len=templen-10;
			memcpy(&ch[6],&len,sizeof(int));

			ACE_Message_Block *MBTcpData = new ACE_Message_Block(templen);

			memcpy(MBTcpData->wr_ptr(),ch,templen);
			MBTcpData->wr_ptr(templen);

			//指标数据放入到指标发送线程的队列
			ACE_Time_Value OutTime(ACE_OS::time(0)+1);
			if (-1 == RECORDQUALITYSENDER::instance()->putq(MBTcpData,&OutTime))
			{
				MBTcpData->release();
			}

		}
		catch(...)
		{
			printf("实时指标测量出错！\n");
		}
	}
	QualityMutex.release();
}

void CheckLevelForUnLock::InsertLevelToDB()
{
	time_t curtime=time(0);
	if(curtime%10!=0)
		return;
	CheckMutex.acquire();
	std::vector<sSignalCheck>::iterator ptr=AlarmRecordInfo.begin();
	for(;ptr!=AlarmRecordInfo.end();ptr++)
	{
		eQualityInfo QuaInfo;
		QuaInfo.checktime=TimeUtil::GetCurDateTime();
		QuaInfo.desc="电平值";
		QuaInfo.freq=(*ptr).Freq;
		QuaInfo.valu=StrUtil::Int2Str((*ptr).level*1000);
		QuaInfo.type="1";

		DBMANAGER::instance()->AddQualityInfo((*ptr).dvbtype,QuaInfo.freq,string("0"),QuaInfo,QuaInfo.checktime);
	}
	CheckMutex.release();
}
