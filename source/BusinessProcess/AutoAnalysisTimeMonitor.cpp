
#include "AutoAnalysisTimeMonitor.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../DeviceAccess/DeviceAccessMgr.h"

AutoAnalysisTimeMonitor::AutoAnalysisTimeMonitor()
{
	bFlag = false;
	ManageType = "day";					//默认为每天执行
	ManageWeekday = "2";				//默认为每个星期二
	ManageSingleday = "2010-05-01";		//默认...
	ManageTime = "00:00:00";			//默认为凌晨

	ManageType = PROPMANAGER::instance()->GetPsisiManagetype();
	ManageWeekday = PROPMANAGER::instance()->GetPsisiManageweekday();
	ManageSingleday = PROPMANAGER::instance()->GetPsisiManagesingleday();
	ManageTime = PROPMANAGER::instance()->GetPsisiManagetime();

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
		meCheckType = PERDAY;
	}
}

AutoAnalysisTimeMonitor::~AutoAnalysisTimeMonitor()
{

}

int AutoAnalysisTimeMonitor::open(void *)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)码流分析管理线程开始执行 !\n"));
	this->activate();
	return 0;
}

int AutoAnalysisTimeMonitor::svc()
{
	bFlag = true;	
	SetNextRunTime();//设置下次运行时间

	while (bFlag)
	{
		time_t currentTime = time(0);//当前时间
		std::string strCurTime = TimeUtil::DateTimeToStr(currentTime);

		if (abs(TimeUtil::DiffMinute(strCurTime,mNextRunTime)) < 3)//与重启时间上下不超过3分钟
		{
			SendTSAnalyzeOrder();			//给所有板卡发送指令			
			SetNextRunTime();				//设置下次运行时间
			OSFunction::Sleep(60*30);		//每检查完一次停止30分钟
		}
		OSFunction::Sleep(60);				// 每分钟检查一次
	}

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)码流分析管理线程线程停止执行 !\n"));

	return 0;
}

int AutoAnalysisTimeMonitor::Start()
{
	open(0);
	return 0;
};
int AutoAnalysisTimeMonitor::Stop()
{
	bFlag = false;
	this->wait();
	return 0;
}

bool AutoAnalysisTimeMonitor::SendTSAnalyzeOrder(void)
{
	std::list<int> devicelist;
	PROPMANAGER::instance()->GetTaskDeviceList("TSAnalyze",CTTB,devicelist);
	PROPMANAGER::instance()->GetTaskDeviceList("TSAnalyze",DVBC,devicelist);
	PROPMANAGER::instance()->GetTaskDeviceList("TSAnalyze",THREED,devicelist);

	std::list<int>::iterator ptr = devicelist.begin();

	for (;ptr!=devicelist.end();++ptr)
	{
		string rtnxml;
		string tasktypexml = OSFunction::CreateTaskTypeXml(*ptr);
		if (tasktypexml == "")
			continue;
		DEVICEACCESSMGR::instance()->SendTaskMsg(*ptr,tasktypexml,rtnxml);
		for (int i=0;i<3&&rtnxml=="";++i)
		{
			DEVICEACCESSMGR::instance()->SendTaskMsg(*ptr,tasktypexml,rtnxml);
			OSFunction::Sleep(0,50);
		}
	}
	return true;
}

bool AutoAnalysisTimeMonitor::SetNextRunTime()
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