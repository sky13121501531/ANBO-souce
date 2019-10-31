
#include "AutoAnalysisTimeMonitor.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../DeviceAccess/DeviceAccessMgr.h"

AutoAnalysisTimeMonitor::AutoAnalysisTimeMonitor()
{
	bFlag = false;
	ManageType = "day";					//Ĭ��Ϊÿ��ִ��
	ManageWeekday = "2";				//Ĭ��Ϊÿ�����ڶ�
	ManageSingleday = "2010-05-01";		//Ĭ��...
	ManageTime = "00:00:00";			//Ĭ��Ϊ�賿

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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)�������������߳̿�ʼִ�� !\n"));
	this->activate();
	return 0;
}

int AutoAnalysisTimeMonitor::svc()
{
	bFlag = true;	
	SetNextRunTime();//�����´�����ʱ��

	while (bFlag)
	{
		time_t currentTime = time(0);//��ǰʱ��
		std::string strCurTime = TimeUtil::DateTimeToStr(currentTime);

		if (abs(TimeUtil::DiffMinute(strCurTime,mNextRunTime)) < 3)//������ʱ�����²�����3����
		{
			SendTSAnalyzeOrder();			//�����а忨����ָ��			
			SetNextRunTime();				//�����´�����ʱ��
			OSFunction::Sleep(60*30);		//ÿ�����һ��ֹͣ30����
		}
		OSFunction::Sleep(60);				// ÿ���Ӽ��һ��
	}

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)�������������߳��߳�ִֹͣ�� !\n"));

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
	if (meCheckType == PERSINGLE)		//����
	{
		mNextRunTime = ManageSingleday + std::string(" ") + ManageTime;
	}
	else if (meCheckType == PERWEEK)	//ÿ����
	{
		time_t currentTime = time(0);		//��ǰʱ��
		std::string strCurTime = TimeUtil::DateTimeToStr(currentTime);
		long CurWeekday = TimeUtil::DateIsWeekDay(strCurTime);						//��ȡ��ǰ����
		long diffday = StrUtil::Str2Long(ManageWeekday) - CurWeekday;				//���ڵĲ�ֵ

		std::string strCurDate = TimeUtil::CalDay(TimeUtil::GetCurDate(),diffday);
		mNextRunTime = TimeUtil::GetDateFromDatetime(strCurDate) + std::string(" ") + ManageTime;					//������Ӧ�ü���ʱ��
		if (TimeUtil::DiffSecond(mNextRunTime,strCurTime) < 0)						//�����ڼ��ʱ����ڵ�ǰʱ��
		{
			mNextRunTime = TimeUtil::CalDay(mNextRunTime,7);										//�´μ��ʱ��˳��Ϊ������
		}
	}
	else if (meCheckType == PERDAY)		//ÿ��
	{
		time_t currentTime = time(0);		//��ǰʱ��
		std::string strCurTime = TimeUtil::DateTimeToStr(currentTime);
		mNextRunTime = TimeUtil::GetCurDate() + std::string(" ") + ManageTime;	//����Ӧ�ü���ʱ��
		if (TimeUtil::DiffSecond(mNextRunTime,strCurTime) < 0)					//������ʱ����ڵ�ǰʱ��
		{
			mNextRunTime = TimeUtil::CalDay(mNextRunTime,1);									//�´μ��ʱ��˳��Ϊ��һ��
		}
	}
	return true;
}