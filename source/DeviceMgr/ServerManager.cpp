
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
	ManageWeekday = "2";				//Ĭ��Ϊÿ�����ڶ�
	ManageSingledatetime = "2010-05-01 00:00:00";		//Ĭ��...
	ManageTime = "00:00:00";
}

ServerManager::~ServerManager()
{

}

int ServerManager::open(void *)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)�豸����״̬�����߳̿�ʼִ�� !\n"));
	this->activate();
	return 0;
}

int ServerManager::svc()
{
	bFlag = true;	
	SetNextRunTime();	//�����´�����ʱ��

	while (bFlag)
	{
		//time_t currentTime = time(0);//��ǰʱ��
		//std::string strCurTime = TimeUtil::DateTimeToStr(currentTime);
		time_t time_m_set = TimeUtil::StrToDateTime(mNextRunTime);
		//if (abs(TimeUtil::DiffSecond(strCurTime,mNextRunTime)) < 1)//������ʱ�����²�����3����
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
				//OSFunction::ExitProcess("�豸״̬��������ǰ�˳���");
			}
			SetNextRunTime();				//�����´�����ʱ��
		}
		//OSFunction::Sleep(2);				// ÿ���Ӽ��һ��
		Sleep(500);
	}

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)�豸����״̬�����߳�ִֹͣ�� !\n"));

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
	if (meCheckType == PERSINGLE)		//����
	{
		mNextRunTime = ManageSingledatetime ;
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
	return true;
}