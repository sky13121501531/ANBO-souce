
#include "ServerManager.h"
#include "OS_Environment.h"
#include "../DeviceAccess/TsFetcherMgr.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"

ServerManager::ServerManager()
{
	bFlag = false;
	meCheckType = NOTHING;
	ManageWeekday = "2";				//默认为每个星期二
	ManageSingledatetime = "2010-05-01 00:00:00";		//默认...
	ManageTime = "00:00:00";
}

ServerManager::~ServerManager()
{

}

int ServerManager::open(void *)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)设备运行状态管理线程开始执行 !\n"));
	this->activate();
	return 0;
}

int ServerManager::svc()
{
	bFlag = true;	
	SetNextRunTime();	//设置下次运行时间

	while (bFlag)
	{
		//time_t currentTime = time(0);//当前时间
		//std::string strCurTime = TimeUtil::DateTimeToStr(currentTime);
		time_t time_m_set = TimeUtil::StrToDateTime(mNextRunTime);
		//if (abs(TimeUtil::DiffSecond(strCurTime,mNextRunTime)) < 1)//与重启时间上下不超过3分钟
		if(time(0) - time_m_set == 0)
		{	
			if(Action=="Restart")
				RebootServer();
			else if(Action=="Reset")
			{
		
#ifdef _DEBUG
		string killcmd="taskkill -im vscttbd.exe ";	
#else
		string killcmd="taskkill -im vscttb.exe";
#endif
				system(killcmd.c_str());
				//MainThread::Stop();
				//OSFunction::ExitProcess("设备状态管理重启前端程序");
			}
			SetNextRunTime();				//设置下次运行时间
		}
		//OSFunction::Sleep(2);				// 每分钟检查一次
		Sleep(500);
	}

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)设备运行状态管理线程停止执行 !\n"));

	return 0;
}

int ServerManager::Start()
{
	open(0);
	return 0;
};
int ServerManager::Stop()
{
	bFlag = false;
	this->wait();
	return 0;
}
bool ServerManager::RebootServer()
{
	std::string cmd="shutdown -r -t 0";
	system(cmd.c_str());
	return true;
}
void ServerManager::SetStatus(int Type,std::string action,std::string strTime)
{
	Action = action;
	if(Type==-1)
	{
		meCheckType = PERSINGLE;
		ManageSingledatetime=strTime;
	}
	else if(Type>=0)
	{
		meCheckType = PERWEEK;
		ManageWeekday = StrUtil::Int2Str(Type);
		ManageTime    = strTime;
	}
	SetNextRunTime();
}

bool ServerManager::SetNextRunTime()
{
	if (meCheckType == PERSINGLE)		//单次
	{
		mNextRunTime = ManageSingledatetime ;
	}
	else if (meCheckType == PERWEEK)	//每星期
	{
		time_t currentTime = time(0);		//当前时间
		std::string strCurTime = TimeUtil::DateTimeToStr(currentTime);
		long CurWeekday = TimeUtil::DateIsWeekDay(strCurTime);						//获取当前星期
		long diffday = StrUtil::Str2Long(ManageWeekday) - CurWeekday;				//星期的差值

		std::string strCurDate = TimeUtil::CalDay(TimeUtil::GetCurDate(),diffday);
		mNextRunTime = TimeUtil::GetDateFromDatetime(strCurDate) + std::string(" ") + ManageTime;					//本星期应该检查的时间
		if (TimeUtil::DiffSecond(mNextRunTime,strCurTime) < 0)						//本星期检查时间大于当前时间
		{
			mNextRunTime = TimeUtil::CalDay(mNextRunTime,7);										//下次检查时间顺延为下七天
		}
	}
	return true;
}