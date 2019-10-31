
#include "DeviceManager.h"
#include "OS_Environment.h"
#include "../DeviceAccess/TsFetcherMgr.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"

DeviceManager::DeviceManager()
{
	bFlag = false;
	ManageType = "week";				//默认为每个星期执行
	ManageWeekday = "2";				//默认为每个星期二
	ManageSingleday = "2010-05-01";		//默认...
	ManageTime = "00:00:00";			//默认为凌晨

	ManageType = PROPMANAGER::instance()->GetDeviceManagetype();
	ManageWeekday = PROPMANAGER::instance()->GetDeviceManageweekday();
	ManageSingleday = PROPMANAGER::instance()->GetDeviceManagesingleday();
	ManageTime = PROPMANAGER::instance()->GetDeviceManagetime();

	if (ManageType == "week")
	{
		meCheckType = PERWEEK;
	}
	else if (ManageType == "day")
	{
		meCheckType = PERDAY;
	}
	else if (ManageType == "single")
	{
		meCheckType = PERSINGLE;
	}
	else
	{
		meCheckType = PERWEEK;
	}
}

DeviceManager::~DeviceManager()
{

}

int DeviceManager::open(void *)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)设备管理线程开始执行 !\n"));
	this->activate();
	return 0;
}

int DeviceManager::svc()
{
	bFlag = true;	
	SetNextRunTime();	//设置下次运行时间

	while (bFlag)
	{
		time_t currentTime = time(0);//当前时间
		std::string strCurTime = TimeUtil::DateTimeToStr(currentTime);
		
		if (abs(TimeUtil::DiffMinute(strCurTime,mNextRunTime)) < 3)//与重启时间上下不超过3分钟
		{
			RebootDevice();					//重启所有板卡			
			SetNextRunTime();				//设置下次运行时间
			OSFunction::Sleep(60*30);		//每检查完一次停止30分钟
		}
		OSFunction::Sleep(60);				// 每分钟检查一次
	}

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)设备管理线程线程停止执行 !\n"));

	return 0;
}

int DeviceManager::Start()
{
	open(0);
	return 0;
};
int DeviceManager::Stop()
{
	bFlag = false;
	this->wait();
	return 0;
}
bool DeviceManager::RebootDevice()
{
	std::list<int> devicedlist;
	PROPMANAGER::instance()->GetAllDeviceList(devicedlist);	
	//获取全部IP
	for(std::list<int>::iterator ptr=devicedlist.begin();ptr!=devicedlist.end();ptr++)
	{
		TSFETCHERMGR::instance()->RebootCard(*ptr);
		OSFunction::Sleep(0,50);
	}
	return true;
}

bool DeviceManager::SetNextRunTime()
{
	if (meCheckType == PERSINGLE)		//单次
	{
		mNextRunTime = ManageSingleday + std::string(" ") + ManageTime;
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
	else if (meCheckType == PERDAY)		//每天
	{
		time_t currentTime = time(0);		//当前时间
		std::string strCurTime = TimeUtil::DateTimeToStr(currentTime);
		mNextRunTime = TimeUtil::GetCurDate() + std::string(" ") + ManageTime;	//当天应该检查的时间
		if (TimeUtil::DiffSecond(mNextRunTime,strCurTime) < 0)					//当天检查时间大于当前时间
		{
			mNextRunTime = TimeUtil::CalDay(mNextRunTime,1);									//下次检查时间顺延为下一天
		}
	}
	return true;
}