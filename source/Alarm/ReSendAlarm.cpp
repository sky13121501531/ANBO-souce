
#include "ReSendAlarm.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/OSFunction.h"
#include "../Communications/XMLSend.h"
#include "../Communications/SysMsgSender.h"
#include "../Foundation/AppLog.h"
#include "ace/OS.h"
#include "fstream"
#include "../Foundation/TypeDef.h"
std::multimap<std::string,std::string> AlarmUPSnd;
ReSendAlarm::ReSendAlarm()
{

}

ReSendAlarm::~ReSendAlarm()
{
	
}

int ReSendAlarm::open(void *)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)报警检查/发送任务线程开始执行 !\n"));
	this->activate();
	return 0;
}

//通过从xml中解析出来的不同类型发送到不同的监测类型报警服务器
int ReSendAlarm::svc()
{
	m_ReAlarmFileNum = 0;
	bFlag = true;
	m_ReAlarmFileNum = TraverseAlarmFiles();
	time_t taverseTime = time(0);
	while (bFlag)
	{
		try 
		{
			if(time(0)-taverseTime>60)
			{//1分钟遍历一次报警重发目录
				taverseTime = time(0);
				m_ReAlarmFileNum = TraverseAlarmFiles();
			}
			if(m_ReAlarmFileNum>0)
            {
				ProcessMessage();
            }
            else if(AlarmUPSnd.size()>0)
            {
                vProResend();
            }
			OSFunction::Sleep(0,200);
		}
		catch(...)
		{
		}
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)报警检查/发送任务线程停止执行 !\n"));

	return 0;
}
time_t FileTimeToTime_t(const FILETIME &ft)  
{  
	ULARGE_INTEGER ui;  
	ui.LowPart = ft.dwLowDateTime;  
	ui.HighPart = ft.dwHighDateTime;  
	return ((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);  
}  

void GetTimeFromFileName(char*xml_filename,SYSTEMTIME &CurrentTime )
{
	char buf[100];
	char *p = xml_filename;
	bool flag = true;
	int begin ,end ,number = 0;

	while(true)
	{    							
		if (strncmp(p,"0",1)>=0 && strncmp(p,"9",1)<=0 && flag)
		{
			begin = number;
			flag = false;
		}						
		if (strncmp(p,"_",1)==0)
		{
			end = number;
			break;
		}
		number++;
		p++;
	}
	memcpy(buf,xml_filename+begin,end-begin);
	sscanf(buf,"%4d%2d%2d%2d%2d%2d",&CurrentTime.wYear,&CurrentTime.wMonth,&CurrentTime.wDay,
			&CurrentTime.wHour,&CurrentTime.wMinute,&CurrentTime.wSecond);


}
time_t SystemTimeToTimet(SYSTEMTIME st)

{
	FILETIME ft;
	SystemTimeToFileTime( &st, &ft );
	LONGLONG nLL;
	ULARGE_INTEGER ui;
	ui.LowPart = ft.dwLowDateTime;
	ui.HighPart = ft.dwHighDateTime;
	nLL = (ft.dwHighDateTime << 32) + ft.dwLowDateTime;
	time_t pt = (long)((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
	return pt;

}

time_t SystemTimeToTime(SYSTEMTIME st)
{
	time_t pt = st.wSecond+st.wMinute*60+st.wHour*60*60+st.wDay*60*60*24+st.wMonth*60*60*24*30+(st.wYear-1970)*60*60*24*30*12;
	return pt;
}
int ReSendAlarm::TraverseAlarmFiles()
{
	string dirs[]={"C:/AlarmUp/DVBS/","C:\\AlarmUp\\DVBC\\","C:\\AlarmUp\\CTTB\\","C:\\AlarmUp\\ATV\\","C:\\AlarmUp\\RADIO\\",\
		"C:\\AlarmUp\\AM\\"};
	//扫描目录
	string Path="C:/AlarmUp/";
	string TempPath = Path + string("*.*");
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	//先提取NIT信息
	hFind = FindFirstFile(TempPath.c_str(), &FindFileData);

	while (bFlag&&(hFind != INVALID_HANDLE_VALUE))
	{
		if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0)
			{
				string dvbtype= string(FindFileData.cFileName);  //文件夹名为频点
				string FindName=Path+dvbtype+string("/")+string("*.xml");
				string DirName = Path+dvbtype+string("/");
				HANDLE hFile;
				WIN32_FIND_DATA  wf;
				hFile = FindFirstFile(FindName.c_str(), &wf);//查找文件
				if (hFile == INVALID_HANDLE_VALUE)
				{
					if (!FindNextFile(hFind, &FindFileData))
					{
						break;
					}
					continue;
				}
				do
				{
					string strXML = "";
					char xml_filename[300];
					strcpy(xml_filename,DirName.c_str()) ;
					strcat(xml_filename,wf.cFileName);
					DWORD dwAttrs = GetFileAttributes(wf.cFileName); 

					if ((dwAttrs & FILE_ATTRIBUTE_READONLY)) //如果文件属性只读，改为正常
					{ 
						SetFileAttributes(xml_filename, FILE_ATTRIBUTE_NORMAL); 
					} 									
					GetTimeFromFileName(xml_filename,CurrentTime);
					time_t KeyTime =SystemTimeToTime(CurrentTime);					
					node * value;
					value = (node *)malloc(sizeof(node));
					sprintf(value->FileName,xml_filename);
					sprintf(value->cFileName,dvbtype.c_str());
					map<time_t,node*>::iterator FindPtr = NodeMap.find(KeyTime);	
					if (FindPtr!=NodeMap.end())
					{
						int TempCount = KeyTime;
						while (true)
						{
							FindPtr = NodeMap.find(TempCount);
							if (FindPtr!=NodeMap.end())
							{
								TempCount++;
								continue;
							}
							else{break;}
						}
						for (int i = TempCount-KeyTime;i>=1;i--)
						{
							NodeMap[KeyTime+i] = NodeMap[KeyTime+i-1];
						}
						NodeMap[KeyTime] = value;
						m_ReAlarmFileNum++;
						continue;
					}
					else
					{
						NodeMap[KeyTime] = value;
						m_ReAlarmFileNum++;
					}
									
				}while(bFlag&&(FindNextFile(hFile, &wf)));
				FindClose(hFile);	
			}

		}
		if (!FindNextFile(hFind, &FindFileData))
		{
			break;
		}
	}	
	FindClose(hFind);
	return m_ReAlarmFileNum;
}
int ReSendAlarm::ProcessMessage()
{
	map<time_t,node*>::iterator ptr = NodeMap.begin();	
	for (;ptr !=NodeMap.end();)
	{//读取xml文件中的内容
		ifstream xmlFile(ptr->second->FileName);
		string strTemp = "";
		string strXML = "";
		while (getline(xmlFile,strTemp))
		{
			strXML += strTemp;
		}
		xmlFile.close();
		UpAlarmInfo alarminfo;
		PROPMANAGER::instance()->GetUpAlarmInfo(OSFunction::GetEnumDVBType(ptr->second->cFileName),alarminfo);
		if ((strXML.empty()) || (alarminfo.alarmurl.empty()))
			continue;
		if(false == XMLSend::SendXML(alarminfo.alarmurl,strXML))
		{
			return 0;
		}
		else
		{
			m_ReAlarmFileNum--;
			std::string msg = string("发送任务对象XML文件成功！") + "URL:" + alarminfo.alarmurl + "***" + strXML.c_str();
			APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_DEBUG,msg,LOG_OUTPUT_FILE);
		}
		bool ret ;
		ret =DeleteFile(ptr->second->FileName);
		if (!ret)//删除文件
		{
			DeleteFile(ptr->second->FileName);
		}
		
		map<time_t,node*>::iterator DelPtr = ptr;
		ptr++;
		NodeMap.erase(DelPtr);

	}
	return 0;
}

int ReSendAlarm::Stop()
{
	bFlag = false;
	this->wait();
	return 0;
}

int ReSendAlarm::vProResend()
{
    multimap<string,string>::iterator pReAlarmSendIter = AlarmUPSnd.begin();
    string URL = "";
    for(;pReAlarmSendIter!=AlarmUPSnd.end();pReAlarmSendIter++)
    {
        string url = pReAlarmSendIter->first;
        string strXML = pReAlarmSendIter->second;
        if(URL!=url)
        {
            if(false==XMLSend::SendXML(url,strXML))
            {
                URL = url;
            }
            else
            {
                AlarmUPSnd.erase(pReAlarmSendIter);
                pReAlarmSendIter = AlarmUPSnd.begin();
                continue;
            }
        }
    }
    return 0;
}
