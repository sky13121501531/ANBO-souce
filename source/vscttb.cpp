#include "cmdLine.h"
#include "./Communications/CommunicationMgr.h"
#include "./Communications/QualitySender.h"
#include "./BusinessProcess/BusinessIterfaceMgr.h"
#include "./BusinessProcess/BusinessLayoutMgr.h"
#include "./Task/TranslateDownXML.h"
#include "./Communications/TSServer.h"
#include "./Communications/TsSenderMgr.h"
#include "./Communications/SysMsgSender.h"
#include "./DeviceAccess/TsFetcherMgr.h"
#include "./DeviceAccess/UdpAlarmRecvThreadMgr.h"
#include "./DeviceMgr/EnvironmentChecher.h"
#include "./Communications/RecordQualitySender.h"
#include "./Communications/SysMsgSender.h"
#include "DeviceAccess/DeviceAccessMgr.h"
#include "./Alarm/AlarmMgr.h"
#include "./Alarm/GetLevelForUnLockForATV.h"
#include "./Alarm/GetLevelForUnLockForRADIO.h"
#include "./Alarm/GetLevelForUnLockForAM.h"
#include "./Alarm/CheckLevelForUnLock.h"
#include "./DeviceAccess/ALLSetTime.h"
#include "./Foundation/OSFunction.h"
#include "./Foundation/StrUtil.h"
#include "./DeviceAccess/SpecTrumOrderSet.h"
#include "./BusinessProcess/PSISIMgr.h"
#include "./Foundation/PropManager.h"
#include "./Foundation/AppLog.h"
#include "./DeviceMgr/ServerManager.h"
#include <ace/Thread.h>

time_t g_starttime = 0;
ACE_Thread_Mutex ALLReadMutex;
static void Init(void* arg)//PSISI��ʼ���̺߳���
{
	//PSISIMGR::instance()->Init();
	return;
}
void Start()
{
	std::string RunInfo = std::string("(������)�������� ") + VSCTTB_PROG_VERSION;
	std::string SysTitel = string("title ")+VSCTTB_PROG_VERSION;
	PROPMANAGER::instance()->InitInfoFromDB();

	CheckDirectoryExsit();	//�������ļ����Ƿ���ڣ������ڣ��ʹ�����.

	//
	g_starttime = time(0);
	//
	system(SysTitel.c_str());

	SYSMSGSENDER::instance()->Start();
	SYSMSGSENDER::instance()->SendMsg(RunInfo,UNKNOWN,VS_MSG_SYSALARM,DEVICE);

	DEVICEACCESSMGR::instance()->Start();
	
	SendDeviceOrder();			//����Ӳ���忨ָ��
	
	TSFETCHERMGR::instance()->Start();
	TSSENDERMGR::instance()->Start();
	UDPALARMRECVTHREADMGR::instance()->Start();

	REALTIMEQUALITYSENDER::instance()->Start();

	RECORDQUALITYSENDER::instance()->Start();

	if (COMMUNICATIONMGR::instance()->Start() != 0)	//ֻ�ж�������صĳ�ʼ��������Ĭ�ϳɹ�
	{
		OSFunction::ExitProcess("ϵͳ��ʼ��(����)ʧ��");
	}

	SEVERMANAGER::instance()->Start();
	BUSINESSITERFACEMGR::instance()->Start();
	BUSINESSLAYOUTMGR::instance()->Start();
	
	ALARMMGR::instance()->Start();	
	CHECKLEVELFORUNLOCK::instance()->Start();
	//GETLEVELFORUNLOCKFORATV::instance()->Start();
	//GETLEVELFORUNLOCKFORRADIO::instance()->Start();
	GETLEVELFORUNLOCKFORAM::instance()->Start();
//	ENVIRONMENTCHECHER::instance()->Start();
	SPECTRUMORDERSET::instance()->Start();
	ACE_Thread_Manager::instance()->spawn((ACE_THR_FUNC)Init,0);
	
	OSFunction::Sleep(0,50);
	cout<<endl<<"\n\t[VSCTTB]-:VSDVB Wacth and Measure System Run."<<endl<<endl;
	LoadXMLTask();
	ALLSETTIME::instance()->Start();
}

void Stop()
{
	TSFETCHERMGR::instance()->Stop();
	TSSENDERMGR::instance()->Stop();
	UDPALARMRECVTHREADMGR::instance()->Stop();
	REALTIMEQUALITYSENDER::instance()->Stop();

	RECORDQUALITYSENDER::instance()->Stop();

	COMMUNICATIONMGR::instance()->Stop();
	SEVERMANAGER::instance()->Stop();
	BUSINESSITERFACEMGR::instance()->Stop();
	BUSINESSLAYOUTMGR::instance()->Stop();
	

	// GETLEVELFORUNLOCKFORATV::instance()->Stop();
	GETLEVELFORUNLOCKFORRADIO::instance()->Stop();
	GETLEVELFORUNLOCKFORAM::instance()->Stop();

	//ENVIRONMENTCHECHER::instance()->Stop();
	CHECKLEVELFORUNLOCK::instance()->Stop();
	ALARMMGR::instance()->Stop();
	SPECTRUMORDERSET::instance()->Stop();
	SYSMSGSENDER::instance()->Stop();
	ALLSETTIME::instance()->Stop();
	OSFunction::Sleep(0,500);
	cout<<endl<<"\t[VSCTTB]-:VSDVB Wacth and Measure System is stopped."<<endl<<endl;
}

int main(int argc, char *argv[])
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	CheckSoleProcess();

	string cmd;
	bool bRunning = false;

	PrintTitle();

	cout<<"-VSCTTB_CMD->"<<endl;

	OSFunction::Sleep(0,10);
	Start();
	bRunning = true;

	while(true)
	{
		cmd.clear();
		cin>>cmd;

		if(cmd == "run" && bRunning == true)
		{
			cout<<"\t[VSCTTB]-:System is running."<<endl;
			continue;
		}
		else if(cmd == "run" && bRunning == false)
		{
			Start();
			bRunning = true;
			continue;
		}
		else if(cmd == "stop" && bRunning == true)
		{
			Stop();
			bRunning = false;
			cout<<"-VSCTTB_CMD->";
			continue;
		}
		else if(cmd == "stop" && bRunning == false)
		{
			cout<<"\t[VSCTTB]-:System is stopped."<<endl;
			cout<<"-VSCTTB_CMD->";
			continue;
		}
		else if(cmd == "quit" && bRunning == true)
		{
			Stop();
			break; 
		}
		else if(cmd == "quit" && bRunning == false)
		{
			break;
		}
		else if (cmd == "sendmsg")
		{
			SYSMSGSENDER::instance()->SetSendFlag(true);
		}
		else if (cmd == "stopmsg")
		{
			SYSMSGSENDER::instance()->SetSendFlag(false);
		}
		else if(cmd == ">") 
		{
			cout<<"-VSDVBTH_CMD->";
			continue;
		}
		else if(cmd == "help")
		{
			PrintUsage(); 
			cout<<"-VSCTTB_CMD->"; 
			continue;
		}
		else
		{
			cout<<"\t[Cmd_Tip]-:invalid command!"<<endl; 
			cout<<"-VSCTTB_CMD->"; 
			continue;
		}
	}
	
	return 0;
}