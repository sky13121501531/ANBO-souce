
#include "cmdLine.h"
#include "DBAccess/DBManager.h"
#include "Task/TaskFactory.h"
#include "Foundation/AppLog.h"
#include "Foundation/OSFunction.h"
#include "Foundation/PropManager.h"
#include "DeviceAccess/DeviceAccessMgr.h"
#include "Communications/CommunicationMgr.h"
#include "ace/OS.h"
#include <vector>
#include <string>
using namespace std;
void PrintTitle (void)
{
	cout<<"\t*****************************************************************"<<endl;;
	cout<<"\t* VSCTTB   [CTTB] wacth and measure system]"<<endl;
	cout<<"\t* Version:"<<VSCTTB_VERSION<<endl;
	cout<<"\t*\t"<<VSCTTB_URL<<endl;
	cout<<"\t*\t"<<VSCTTB_COPYRIGHT<<endl;
	cout<<"\t*****************************************************************"<<endl<<endl;
}

void PrintUsage(void)
{
	cout<<"\t|-----------------------------------------|"<<endl;
	cout<<"\t| Usage: VSCTTB help						"<<endl;
	cout<<"\t|\trun:\tstart work process				"<<endl;
	cout<<"\t|\tstop:\tstop work process				"<<endl;
	cout<<"\t|\t>:\tdisplay the cmd line				"<<endl;
	cout<<"\t|\thelp:\tPrint The Help Info				"<<endl;
	cout<<"\t|\tquit:\tExit The System					"<<endl;
	cout<<"\t|-----------------------------------------|"<<endl;
}

bool SendDeviceOrder(void)
{
	std::list<int> alldevicelist;
	PROPMANAGER::instance()->GetAllDeviceList(alldevicelist);
	std::list<int>::iterator ptr = alldevicelist.begin();

	for (;ptr!=alldevicelist.end();ptr++)
	{
		//����6U�忨�ı���������
//		DEVICEACCESSMGR::instance()->setThresholdInfo(*ptr);

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
void LoadXMLTask(void)
{
	std::vector<std::string>  vctXML;
	bool rnt1 = DBMANAGER::instance()->QueryTask(CTTB,vctXML);
	bool rnt2 = DBMANAGER::instance()->QueryTask(DVBC,vctXML);
	bool rnt3 = DBMANAGER::instance()->QueryTask(RADIO,vctXML);
	bool rnt4 = DBMANAGER::instance()->QueryTask(ATV,vctXML);
	bool rnt5 = DBMANAGER::instance()->QueryTask(AM,vctXML);
	bool rnt6 = DBMANAGER::instance()->QueryTask(DVBS,vctXML);
	bool rnt7 = DBMANAGER::instance()->QueryTask(CTV,vctXML);

	if (rnt1==false||rnt2==false||rnt3==false||rnt4==false||rnt5==false||rnt6==false||rnt7==false )
	{
		OSFunction::ExitProcess("ϵͳ��ʼ��(���ݿ�����)ʧ��");
	}

	for (size_t i = 0;i<vctXML.size();i++)
	{
		std::string strXML = vctXML.at(i);
		ACE_Message_Block *mbXML = new ACE_Message_Block(strXML.length());
		memcpy(mbXML->wr_ptr(),strXML.c_str(),strXML.length());
		mbXML->wr_ptr(strXML.length());

		if (mbXML != NULL)
		{
			ACE_Time_Value OutTime(ACE_OS::time(0)+1);
			COMMUNICATIONMGR::instance()->AccessOrderReceive()->putq(mbXML,&OutTime);
			OSFunction::Sleep(0,50);
		}
	}
	return;
}

void CheckSoleProcess()
{
	HANDLE m_hMutex = CreateMutex(NULL, FALSE, "VSDVBUNIQUENESS");
	if (GetLastError() == ERROR_ALREADY_EXISTS)// ���������
	{
		CloseHandle(m_hMutex);// ������л������������ͷž������λ������
		m_hMutex = NULL;

		OSFunction::ExitProcess("ϵͳ�Ѿ�����");
	}
	return;
}


void CheckDirectoryExsit()
{
	std::vector<std::string> vecVscttbDir;

	/* xml�����ļ��� */
	vecVscttbDir.push_back( "C:/SMSDown/" );
	vecVscttbDir.push_back( "C:/CASDown/" );
	vecVscttbDir.push_back( "C:/MonDown/" );
	vecVscttbDir.push_back( "C:/DVBTHDown/" );
	vecVscttbDir.push_back( "C:/TVMonDown/" );
	vecVscttbDir.push_back( "C:/RadioDown/" );
	vecVscttbDir.push_back( "C:/AMDown/" );
	vecVscttbDir.push_back( "C:/CommonDown/" );
	vecVscttbDir.push_back( "C:/SatDown/" );
	vecVscttbDir.push_back( "C:/3DDown/" );
	vecVscttbDir.push_back( "C:/SMSUp/" );
	vecVscttbDir.push_back( "C:/sms/" );

	/* ��ʱ�ļ��� */
	vecVscttbDir.push_back( "C:/vscttb/temp/" );

	/* ��־�ļ��� */
	std::string logPath = PROPMANAGER::instance()->GetLogPath();
	vecVscttbDir.push_back( logPath );
	vecVscttbDir.push_back( logPath+PROPMANAGER::instance()->GetLogOther()+"/" );
	vecVscttbDir.push_back( logPath+PROPMANAGER::instance()->GetLogRecord()+"/" );
	vecVscttbDir.push_back( logPath+PROPMANAGER::instance()->GetLogAnalyser()+"/" );
	vecVscttbDir.push_back( logPath+PROPMANAGER::instance()->GetLogDevice()+"/" );
	vecVscttbDir.push_back( logPath+PROPMANAGER::instance()->GetLogDefault()+"/" );

	/* sms cas */
	std::string dir = "";
	PROPMANAGER::instance()->GetShareDir( dir );
	vecVscttbDir.push_back( dir );

	/* record path */
	std::vector<std::string> vecLoc;
	PROPMANAGER::instance()->GetRecFileLocVec( vecLoc );
	for (int i=0; i<vecLoc.size(); i++)
	{
		vecVscttbDir.push_back( vecLoc[i] );
	}


	/* ���ݼ�����Ͷ�������ͬ�� */
	vecVscttbDir.push_back( "C:/table/" );
	for (int type=1; type<=9; type++)
	{
		/* table path */
		std::string tablePath = "";
		PROPMANAGER::instance()->GetXmlTablePath((enumDVBType)type, tablePath);
		if (tablePath != "")
		{
			vecVscttbDir.push_back( tablePath );
		}
	}


	/* �����Щ�ļ����Ƿ��Ѿ���������û�У��ʹ����� */
	for (int dirIndex=0; dirIndex<vecVscttbDir.size(); dirIndex++)
	{
		if ( !CreateDirectory(vecVscttbDir[dirIndex].c_str(), NULL) )
		{
			//std::cout<<"�ļ��� ["<<vecVscttbDir[dirIndex]<<"] �Ѿ�����,���ô���"<<std::endl;
		}
		else
			std::cout<<"�ļ��� ["<<vecVscttbDir[dirIndex]<<"] �����ɹ�"<<std::endl;
	}

	return;
}