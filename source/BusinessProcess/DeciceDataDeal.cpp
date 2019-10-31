#include "DeciceDataDeal.h"
#include "BusinessLayoutMgr.h"
#include "PSISIMgr.h"
#include <Sddl.h>
#include <windows.h> 
#include <conio.h>
#include "../Foundation/PropManager.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/OSFunction.h"
#include "../Alarm/AlarmMgr.h"
#include "../Foundation/AppLog.h"
#include "../Communications/XMLSend.h"
#include "../Communications/SysMsgSender.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "../DeviceAccess/TsFetcherMgr.h"
#include "../DeviceAccess/CardType.h"

#include "ace/Synch.h"
#include "ace/OS.h"
#include <fstream>
#include <string>
int g_size=0;
static int rcvAlarmCount = 0;
//
time_t g_freqlocktime[64];
bool g_freqlockval[64]; // 通道锁定状态 [DeviceID]=false 未锁定 true 锁定 
bool g_freqneedlock[64];
bool g_unalarmchanscanval[64];
bool g_alarmchanscanval[64];
//
const int RECVBUFLEN = 1024*5;

DeciceDataDeal::DeciceDataDeal(ACE_SOCK_Stream socket)
{
	Finished = false;
	mSocket = socket;
}

DeciceDataDeal::~DeciceDataDeal()
{
	// mSocket.close_writer();
	mSocket.close();
}

int DeciceDataDeal::open(void*)
{
	activate();
	wait();
	return 0;
}
int DeciceDataDeal::Stop()
{
	this->wait();
	return 0;
}

static time_t TempTime ;

bool TempFlag = true;
bool TempLock = true;

int DeciceDataDeal::svc()
{
	if (TempFlag)
	{
		TempFlag = false;
		TempTime = time(0);
	}
	char buf[RECVBUFLEN] = {0};
	ssize_t recvlen = 0;

	RecvInfo(buf,RECVBUFLEN,recvlen);
	if (recvlen <= 0)
	{
		mSocket.close();
		Finished = true;
		return 0;
	}
	std::string xml = buf;
	g_size+=xml.size();
	//ACE_DEBUG ((LM_DEBUG,"alarm info:%s**************size:%d\n",xml.c_str(),g_size));
	if (xml.find("<?xml") != string::npos)		//PSISI or ETR290 找到了
	{
		std::string tempxml;
		StrUtil::ConvertUtf8ToGBK(xml.c_str(),tempxml);
		ProcessXmlInfo(tempxml);
	}
	else  //没找到
	{
		ProcessProgramAlarm(buf,recvlen);
	}
	//mSocket.close_writer();
	mSocket.close();
	Finished = true;
	return 0;
}

bool DeciceDataDeal::RecvInfoEx(char* Info,const int buflen,ssize_t& rcvlen)
{
	BOOL fConnected; 
	HANDLE hPipe;
	LPTSTR lpszPipename = "\\\\.\\pipe\\AlarmInfo";
	SECURITY_ATTRIBUTES  sa, *psa;
	psa = &sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = FALSE;  
	TCHAR * szSD = TEXT("D:")       // Discretionary ACL
		TEXT("(A;OICI;GA;;;WD)");    // Allow full control to everyone

	if(!ConvertStringSecurityDescriptorToSecurityDescriptor(
		szSD,
		SDDL_REVISION_1,
		&(sa.lpSecurityDescriptor),
		NULL))
	{
		psa = NULL;
	}

	hPipe = CreateNamedPipe( 
		lpszPipename,             // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		1,						// one. instances  
		0,                  // output buffer size 
		1024,                  // input buffer size 
		2000,             // client time-out 
		psa);                    //  security attribute 

	if (hPipe == INVALID_HANDLE_VALUE)
		return false;//error
	int i = 0;
	while(1 < 1000)
	{
		fConnected = ConnectNamedPipe(hPipe, NULL) ? 
TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
		if (fConnected) 
		{
			CHAR chRequest[1024]; 		
			DWORD cbBytesRead; 
			BOOL fSuccess;
			DWORD dwTotalSize = 0;
			string strXML;
			
			do{
				fSuccess = ReadFile( 
					hPipe,        // handle to pipe 
					chRequest,    // buffer to receive data 
					1024-1,      // size of buffer 
					&cbBytesRead, // number of bytes read 
					NULL);        // not overlapped I/O 
				if (! fSuccess && GetLastError() != ERROR_MORE_DATA) 
					break; 

				chRequest[cbBytesRead] = 0;
				strXML += string(chRequest);
				dwTotalSize += cbBytesRead;
				::Sleep(10);
			} while (! fSuccess);  // repeat loop if ERROR_MORE_DATA 
			
			memcpy(Info,strXML.c_str(),strXML.size());
			rcvlen = strXML.size();
			cout<<"rec alarm:"<<strXML.c_str();
			FlushFileBuffers(hPipe); 
			DisconnectNamedPipe(hPipe); 
		}

	}
	CloseHandle(hPipe); 
}
//int TempCount =0;
//FILE *fpw;

bool DeciceDataDeal::RecvInfo(char* Info,const int buflen,ssize_t& rcvlen)
{
	int CountTimes = 200;	//200个字符都不正确，断开链接
	rcvlen = 0;
	int OneAlarmInfoLen = sizeof(Alarm_Info_t);
	int deviceid =0;

	ACE_INET_Addr Addr;
	mSocket.get_remote_addr(Addr);
	std::string IP=Addr.get_host_addr();
	std::list<int> devicedlist;
	PROPMANAGER::instance()->GetDeviceIndex(devicedlist,IP);

	try
	{
		char tempbuf[RECVBUFLEN] = {0};
		int templen = 0;
		ACE_Time_Value TimeOut(10);

		while(true)
		{
			templen=mSocket.recv(tempbuf,buflen,&TimeOut);

			if(templen <= 0 || templen == SOCKET_ERROR)
			{
				memset(tempbuf,0,RECVBUFLEN);
				OSFunction::Sleep(0,50);
				break;
			}
			while(templen>OneAlarmInfoLen+1)//收到的数据够一条报警长度
			{
				if((tempbuf[0]== (char)0xff)&&(tempbuf[OneAlarmInfoLen+1]== (char)0xee))
				{//存在一条报警
					//处理报警
					Alarm_Info_t alarm;
					memcpy(&alarm,&tempbuf[1],OneAlarmInfoLen);
					int DeviceID[100];
					memset(DeviceID,0,sizeof(int)*100);
					//
					std::list<int>::iterator ptr = devicedlist.begin();
					for (int num=0;ptr!=devicedlist.end();ptr++)
					{
						DeviceID[num] = (*ptr);
						num++;
					}
					eDVBType dvbtype = UNKNOWN;
					PROPMANAGER::instance()->GetDeviceID(IP,StrUtil::Int2Str(alarm.ChannelNO),deviceid);		//数字板卡默认为0;
					PROPMANAGER::instance()->GetDeviceType(deviceid,dvbtype);	//获取板卡监测类型

					sTaskInfo taskinfo;
					bool Rtn = BUSINESSLAYOUTMGR::instance()->QueryRunTaskInfo(deviceid,taskinfo);
					//有AutoRecord和TaskRecord任务，使用报警
					if(Rtn && (taskinfo.taskname == "AutoRecord" || taskinfo.taskname == "TaskRecord"))
					{
						sCheckParam sCheck;

						sCheck.AlarmType	= ALARM_PROGRAM;
						sCheck.DVBType		= taskinfo.dvbtype;
						sCheck.ChannelID	= taskinfo.channelid;
						sCheck.Freq			= taskinfo.freq;
						sCheck.STD			= "";
						sCheck.SymbolRate	= "";
						sCheck.TypedValue	= "";
						sCheck.DeviceID		= StrUtil::Int2Str(deviceid);
						sCheck.CheckTime	= (time_t)(((alarm.AlarmTimeMs+500)/1000)-28800);//四舍五入
						//sCheck.CheckTime	= time(0);
						
						
						if(alarm.AlarmType == ALARM_FREEZE)
						{
							sCheck.TypeID = "0x4";
							sCheck.TypeDesc = "图像静止";
							sCheck.mode			= "0";//报警
						}
						else if(alarm.AlarmType == ALARM_BLACK)
						{
							sCheck.TypeID = "0x1";
							sCheck.TypeDesc = "黑屏";
							sCheck.mode			= "0";//报警
						}
						else if(alarm.AlarmType == ALARM_COLORBAR)
						{
							sCheck.TypeID = "0x2";
							sCheck.TypeDesc = "彩条";
							sCheck.mode			= "0";//报警
						}
						// 	else if(alarm.AlarmType == ALARM_VOLUMEHIGH)
						// 	{
						// 		sCheck.TypeID		= ;
						// 		sCheck.TypeDesc		= ;
						// 		sCheck.mode			= "0";//报警
						// 	}
						// 	else if(alarm.AlarmType == ALARM_VOLUMELOW)
						// 	{
						// 		sCheck.TypeID		= ;
						// 		sCheck.TypeDesc		= ;
						// 		sCheck.mode			= "0";//报警
						// 	}
						else if(alarm.AlarmType == ALARM_AUDIOLOST)
						{
							sCheck.TypeID = "0x10";
							//sCheck.TypeID = "24";
							sCheck.TypeDesc = "无声音";
							sCheck.mode			= "0";//报警
							Rtn  = true;

						}
						else if(alarm.AlarmType == ALARM_LOSTSIGNAL)
						{
							sCheck.TypeID = "0xE";		
							sCheck.TypeDesc = "无载波";
							sCheck.mode			= "0";//报警
						}
						else
						{
							//cout<< "通道[" << DeviceID[alarm.ChannelNO] << "]出现未知报警, 报警类型: "<< alarm.AlarmType << endl;
							return true;
						}
					
						if(g_freqlockval[deviceid])
						{
							if(alarm.AlarmType == ALARM_FREEZE)
							{
								cout<< "通道[" << deviceid << "]出现报警, 报警类型: "<< sCheck.TypeDesc;
								cout<<"当前时间:" << TimeUtil::DateTimeToStr(sCheck.CheckTime) << endl;	
							}
									
							if(taskinfo.dvbtype == ATV && alarm.AlarmType == ALARM_LOSTSIGNAL)
							{
								for(int i =0; i <= 3; i++)
								{
									sCheckParam sTmpCheck1;
									sTmpCheck1.AlarmType	= sCheck.AlarmType;
									sTmpCheck1.DVBType		= sCheck.DVBType;		
									sTmpCheck1.ChannelID	= sCheck.ChannelID;
									sTmpCheck1.Freq			= sCheck.Freq;
									sTmpCheck1.STD			= sCheck.STD;
									sTmpCheck1.SymbolRate	= sCheck.SymbolRate;
									sTmpCheck1.TypedValue	= sCheck.TypedValue;
									sTmpCheck1.DeviceID		= sCheck.DeviceID;
									sTmpCheck1.CheckTime	= sCheck.CheckTime + 1 + i;
									sTmpCheck1.TypeID		= sCheck.TypeID;
									sTmpCheck1.TypeDesc		= sCheck.TypeDesc;
									sTmpCheck1.mode			= sCheck.mode;
									ALARMMGR::instance()->CheckAlarm(sTmpCheck1,true);
								}

							}
							ALARMMGR::instance()->CheckAlarm(sCheck,true);

						}
					}
					//多条处理
					templen= templen-OneAlarmInfoLen-2;
					if(templen>OneAlarmInfoLen+1)	
						memcpy(tempbuf,&tempbuf[OneAlarmInfoLen+1],templen);
					else
						break;
				}
				else
				{//
					templen=templen-1;
					memcpy(tempbuf,&tempbuf[1],templen);
					CountTimes--;
				}	
				if(0>CountTimes)//200次均未找到正确的报警数据退出
				{
					rcvlen = 0;
					cout<<"200次均未找到正确的报警数据退出"<<endl;
					return false;
				}
			}
		}
		return false;
	}
	catch(...)
	{
		;
	}
	return false;
}


bool DeciceDataDeal::ProcessTableEIT(const std::string& Info)
{
	XmlParser EITParser( Info.c_str() );
	pXMLNODE rootNode = EITParser.GetRootNode();
	if (rootNode == NULL)
	{
		return false;
	}

	string dvbtype,freq,type,pid;
	EITParser.GetAttrNode(rootNode,"dvbtype",dvbtype);
	EITParser.GetAttrNode(rootNode,"Freq",freq);
	EITParser.GetAttrNode(rootNode,"type",type);
	EITParser.GetAttrNode(rootNode,"pid",pid);
	if (type != "EIT")
	{
		return false;
	}

	std::string tableId,serviceId,transStreamId,orgNetWortId;
	int secCount = EITParser.GetChildCount( rootNode );
	pXMLNODELIST secList = EITParser.GetNodeList( rootNode );
	for (int i=0; i<secCount; i++)
	{
		pXMLNODE secNode = EITParser.GetNextNode(secList);
		//<Section TableID="79" SectionSyntaxIndicator="128" SectionLength="56" ServiceID="115" TransportStreamID="11" 
		//	OriginalNetworkID="16512" VersionNumber="24" CurrentNextIndicator="1" SectionNumber="0" LastSectionNumber="1"
		//	LastTableID="79" CrcValue="4294967295">
		std::string secSynIndicator,secLength,versionNum,curNextIndicator,secNum,lastSecNum,lastTableId,crcValue;
		EITParser.GetAttrNode(secNode, "TableID", tableId);
		EITParser.GetAttrNode(secNode, "SectionSyntaxIndicator", secSynIndicator);
		EITParser.GetAttrNode(secNode, "SectionLength", secLength);
		EITParser.GetAttrNode(secNode, "ServiceID", serviceId);
		EITParser.GetAttrNode(secNode, "TransportStreamID", transStreamId);
		EITParser.GetAttrNode(secNode, "OriginalNetworkID", orgNetWortId);
		EITParser.GetAttrNode(secNode, "VersionNumber", versionNum);
		EITParser.GetAttrNode(secNode, "CurrentNextIndicator", curNextIndicator);
		EITParser.GetAttrNode(secNode, "SectionNumber", secNum);
		EITParser.GetAttrNode(secNode, "LastSectionNumber", lastSecNum);
		EITParser.GetAttrNode(secNode, "LastTableID", lastTableId);
		EITParser.GetAttrNode(secNode, "CrcValue", crcValue);

		pXMLNODE eventListNode = EITParser.GetNodeFirstChild( secNode );	//暂时认为每个section下只有一个eventList
		int eventCount = EITParser.GetChildCount( eventListNode );
		pXMLNODELIST eventNodeList = EITParser.GetNodeList( eventListNode );
		for (int j=0; j<eventCount; j++)
		{
			/* savexml 创建相关节点 */
			std::string saveXml = "<?xml version=\"1.0\" encoding=\"GB2312\" ?><PSISI ></PSISI>";
			XmlParser saveParser( saveXml.c_str() );
			pXMLNODE saveRootNode = saveParser.GetRootNode();
			saveParser.SetAttrNode("type", type,saveRootNode);
			saveParser.SetAttrNode("pid", pid,saveRootNode);
			saveParser.SetAttrNode("dvbtype", dvbtype,saveRootNode);
			saveParser.SetAttrNode("Freq", freq,saveRootNode);
			pXMLNODE saveSecNode = saveParser.CreateNodePtr(saveRootNode, "Section");
			saveParser.SetAttrNode("TableID", tableId,saveSecNode);
			saveParser.SetAttrNode("SectionSyntaxIndicator", secSynIndicator, saveSecNode);
			saveParser.SetAttrNode("SectionLength", secLength, saveSecNode);
			saveParser.SetAttrNode("ServiceID", serviceId, saveSecNode);
			saveParser.SetAttrNode("TransportStreamID", transStreamId, saveSecNode);
			saveParser.SetAttrNode("OriginalNetworkID", orgNetWortId, saveSecNode);
			saveParser.SetAttrNode("VersionNumber", versionNum, saveSecNode);
			saveParser.SetAttrNode("CurrentNextIndicator", curNextIndicator, saveSecNode);
			saveParser.SetAttrNode("SectionNumber", secNum, saveSecNode);
			saveParser.SetAttrNode("LastSectionNumber", lastSecNum, saveSecNode);
			saveParser.SetAttrNode("LastTableID", lastTableId, saveSecNode);
			saveParser.SetAttrNode("CrcValue", crcValue, saveSecNode);
			pXMLNODE saveListNode = saveParser.CreateNodePtr(saveSecNode, "EventList");
			pXMLNODE saveEventNode = saveParser.CreateNodePtr(saveListNode, "Event");
			
			/* 原始xml中读取event信息 */
			pXMLNODE eventNode = EITParser.GetNextNode( eventNodeList );
			//<Event EventID="385" TimeOfChange="2011/11/21  16:53:00" Duration="00:33:00" RunningStatus="4" FreeCAMode="0" 
			//DescriptorsLoopLength="29">
			std::string eventId,timeOfChange,duration,runingStatus,freeCAMode,descLoopLength;
			std::string eventStartTime;
			EITParser.GetAttrNode(eventNode, "EventID", eventId);
			EITParser.GetAttrNode(eventNode, "TimeOfChange", timeOfChange);
			EITParser.GetAttrNode(eventNode, "Duration", duration);
			EITParser.GetAttrNode(eventNode, "RunningStatus", runingStatus);
			EITParser.GetAttrNode(eventNode, "FreeCAMode", freeCAMode);
			EITParser.GetAttrNode(eventNode, "DescriptorsLoopLength", descLoopLength);
	
			saveParser.SetAttrNode("EventID", eventId, saveEventNode);
			saveParser.SetAttrNode("TimeOfChange", timeOfChange, saveEventNode);
			saveParser.SetAttrNode("Duration", duration, saveEventNode);
			saveParser.SetAttrNode("RunningStatus", runingStatus, saveEventNode);
			saveParser.SetAttrNode("FreeCAMode", freeCAMode, saveEventNode);
			saveParser.SetAttrNode("DescriptorsLoopLength", descLoopLength, saveEventNode);

			try
			{	//event 开始时间处理
				if (timeOfChange != "")
				{
					int pos = 0;
					while (pos<timeOfChange.length())
					{
						if (timeOfChange.at(pos)=='/' ||
							timeOfChange.at(pos)==' ' ||
							timeOfChange.at(pos)==':'	)
						{
							timeOfChange.replace(pos,1,"");
							continue;
						}
						pos++;
					}
				}
				eventStartTime = timeOfChange;
			}
			catch (...)
			{
				return false;
			}

			int descCount = EITParser.GetChildCount( eventNode );
			pXMLNODELIST descNodeList = EITParser.GetNodeList( eventNode );
			for (int n=0; n<descCount; n++)
			{
				pXMLNODE descNode = EITParser.GetNextNode( descNodeList );

				std::string descNodeName = EITParser.GetNodeName( descNode );
				pXMLNODE saveDescNode = saveParser.CreateNodePtr(saveEventNode, const_cast<char*>(descNodeName.c_str()));
				if (descNodeName == "ComponentDescriptor")
				{
					std::string descTag,descLen,streamContent,componentType,componentTag,languageCode;
					EITParser.GetAttrNode(descNode, "DescriptorTag", descTag);
					EITParser.GetAttrNode(descNode, "DescriptorLen", descLen);
					EITParser.GetAttrNode(descNode, "StreamContent", streamContent);
					EITParser.GetAttrNode(descNode, "ComponentType", componentType);
					EITParser.GetAttrNode(descNode, "ComponentTag", componentTag);
					EITParser.GetAttrNode(descNode, "ISO639_2LanguageCode", languageCode);

					saveParser.SetAttrNode("DescriptorTag", descTag, saveDescNode);
					saveParser.SetAttrNode("DescriptorLen", descLen, saveDescNode);
					saveParser.SetAttrNode("StreamContent", streamContent, saveDescNode);
					saveParser.SetAttrNode("ComponentType", componentType, saveDescNode);
					saveParser.SetAttrNode("ComponentTag", componentTag, saveDescNode);
					saveParser.SetAttrNode("ISO639_2LanguageCode", languageCode, saveDescNode);
				}
				else if (descNodeName == "EventDescriptor")
				{
					std::string descTag,descLen,languageCode,eventNameLen,eventName;
					EITParser.GetAttrNode(descNode, "DescriptorTag", descTag);
					EITParser.GetAttrNode(descNode, "DescriptorLen", descLen);
					EITParser.GetAttrNode(descNode, "ISO639_2LanguageCode", languageCode);
					EITParser.GetAttrNode(descNode, "EventNameLength", eventNameLen);
					EITParser.GetAttrNode(descNode, "EventName", eventName);
					
					saveParser.SetAttrNode("DescriptorTag", descTag, saveDescNode);
					saveParser.SetAttrNode("DescriptorLen", descLen, saveDescNode);
					saveParser.SetAttrNode("ISO639_2LanguageCode", languageCode, saveDescNode);
					saveParser.SetAttrNode("EventNameLength", eventNameLen, saveDescNode);
					saveParser.SetAttrNode("EventName", eventName, saveDescNode);
				}
				else if (descNodeName == "ContentDescriptor")
				{
					std::string descTag,descLen,contentNibLevel1,contentNibLevel2,userNibLevel1,userNibLevel2;
					EITParser.GetAttrNode(descNode, "DescriptorTag", descTag);
					EITParser.GetAttrNode(descNode, "DescriptorLen", descLen);
					EITParser.GetAttrNode(descNode, "ContentNibbleLevel1", contentNibLevel1);
					EITParser.GetAttrNode(descNode, "ContentNibbleLevel2", contentNibLevel2);
					EITParser.GetAttrNode(descNode, "UserNibbleLevel1", userNibLevel1);
					EITParser.GetAttrNode(descNode, "UserNibbleLevel2", userNibLevel2);

					saveParser.SetAttrNode("DescriptorTag", descTag, saveDescNode);
					saveParser.SetAttrNode("DescriptorLen", descLen, saveDescNode);
					saveParser.SetAttrNode("ContentNibbleLevel1", contentNibLevel1, saveDescNode);
					saveParser.SetAttrNode("ContentNibbleLevel2", contentNibLevel2, saveDescNode);
					saveParser.SetAttrNode("UserNibbleLevel1", userNibLevel1, saveDescNode);
					saveParser.SetAttrNode("UserNibbleLevel2", userNibLevel2, saveDescNode);
				}
			}	//descriptor 
			
			eDVBType etype = OSFunction::GetEnumDVBType(dvbtype);
			std::string path,dir;
			PROPMANAGER::instance()->GetXmlTablePath(etype,path);
			dir = path + "EIT_Table/";

			ACE_DIR   *directory_pointer;   
			if((directory_pointer=ACE_OS::opendir(dir.c_str()))==NULL)   
			{
				mkdir(dir.c_str());
			} 

			//std::string filename = dir+tableId+eventId+eventStartTime;
			std::string filename = dir+type+"_"+pid+"_"+tableId;
			if(orgNetWortId!="")
			{
				filename += string("_" +orgNetWortId);
			}
			if (transStreamId != "")
			{
				filename += string("_" +transStreamId);
			}
			if (serviceId != "")
			{
				filename += string("_" +serviceId);
			}
			if (eventId != "")
			{
				filename += string("_" +eventId);
			}
			if (eventStartTime != "")
			{
				filename += string("_"+eventStartTime);
			}

			filename += ".xml";
			saveParser.SaveAsFile( filename.c_str() );
		}	//event
	}	//section

	return true;
}

bool DeciceDataDeal::ProcessXmlInfo(const std::string& alarmInfo)
{
	//判断上报信息类型
	//表信息，直接存文件、报警信息放到报警管理中处理
	XmlParser parser;
	string dvbtype,freq,type,pid,tableid,serviceid,tsid,orgnetid,eventid;
	eDVBType etype = UNKNOWN;
	parser.Set_xml(alarmInfo);
	pXMLNODE rootNode=parser.GetRootNode();
	if (alarmInfo.find("Return")!=string::npos)
	{
		parser.GetAttrNode(rootNode,"DVBType",dvbtype);
		etype = OSFunction::GetEnumDVBType(dvbtype);
		
		PROPMANAGER::instance()->SetPSISIInfo(etype,"1");
		return true;
	}

	pXMLNODE childNode=parser.GetNodeFirstChild(rootNode);
	if(parser.GetRootNodeName()=="PSISI")
	{
		rootNode=parser.GetRootNode();
		parser.GetAttrNode(rootNode,"dvbtype",dvbtype);
		parser.GetAttrNode(rootNode,"Freq",freq);
		parser.GetAttrNode(rootNode,"type",type);
		parser.GetAttrNode(rootNode,"pid",pid);
		
		if(type=="HDTV")
		{
			parser.GetAttrNode(childNode,"ServiceID",serviceid);
			parser.GetAttrNode(childNode,"TransportStreamID",tsid);
			parser.GetAttrNode(childNode,"OriginalNetworkID",orgnetid);
		}
		else
		{
			parser.GetAttrNode(childNode,"TableID",tableid);
		}
		

		static long num=0;
		if(num>10000)
			num=0;
		if (type == "SDT"||type=="NIT")//EIT和NIT获得原始网络id传输流id
		{
			APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_DEBUG,string("收到"+type+"表"),LOG_OUTPUT_FILE);
			parser.GetAttrNode(childNode,"OriginalNetworkID",orgnetid);
			parser.GetAttrNode(childNode,"TransportStreamID",tsid);
			if (tsid=="")
			{
				string msg="频点"+freq+type+" 的tsid为空";
				APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_DEBUG,msg,LOG_OUTPUT_FILE);
			}
		}
		if (type == "EIT")
		{
			bool ret = ProcessTableEIT( alarmInfo );
			return ret;
		}

		if (freq=="" || type=="" || pid=="" || tableid=="")
		{
//			printf("硬件上传XML信息不完整%d\r",num++);
		}
		if(freq=="")
		{
			APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_DEBUG,string("频点值为空"+freq),LOG_OUTPUT_FILE);
			freq=TimeUtil::DateTimeToString(time(0));
		}
		if (type=="")
		{
			APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_DEBUG,string("表类型值为空"+type),LOG_OUTPUT_FILE);
		}
		if (pid=="")
		{
			APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_DEBUG,string("pid值为空"+pid),LOG_OUTPUT_FILE);
		}
		if (tableid=="")
		{
			APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_DEBUG,string("tableid值为空"+tableid),LOG_OUTPUT_FILE);
		}

		etype = OSFunction::GetEnumDVBType(dvbtype);

		string path,dir;
		PROPMANAGER::instance()->GetXmlTablePath(etype,path);
		dir=path+freq;

		ACE_DIR   *directory_pointer;   
		if((directory_pointer=ACE_OS::opendir(dir.c_str()))==NULL)   
		{
			mkdir(dir.c_str());
		} 
		string filename;
		if(type=="HDTV")
		{
			filename=dir+"/"+type;
		}
		else
		{
			filename=dir+"/"+type+"_"+pid+"_"+tableid;
		}
		if(orgnetid!="")
		{
			filename += string("_" +orgnetid);
		}
		if (tsid != "")
		{
			filename += string("_" +tsid);
		}
		if (serviceid != "")
		{
			filename += string("_" +serviceid);
		}
		if (eventid != "")
		{
			filename += string("_" +eventid);
		}

		filename += ".xml";

		parser.SaveAsFile(filename.c_str());

		if(type == "EIT"||type == "NIT"||type == "PAT"||type == "PMT"||type == "SDT"||type == "HDTV")
			PSISIMGR::instance()->UpdatePSISI(etype,alarmInfo);
	}
	else if (parser.GetRootNodeName()=="Msg")
	{
		sCheckParam sCheck;
		string STD,Freq,SymbolRate;
		string Dtype;
		parser.GetAttrNode(rootNode,"DVBType",dvbtype);
		parser.GetAttrNode(childNode,"STD",STD);
		parser.GetAttrNode(childNode,"Freq",Freq);
		parser.GetAttrNode(childNode,"SymbolRate",SymbolRate);

		if(dvbtype=="CTTB")
		{
			etype=CTTB;
			Dtype="DVBTHUp";
		}
		else if(dvbtype=="DVBC")
		{
			etype=DVBC;
			Dtype="MonUp";
		}
		else if(dvbtype=="DVBS")
		{
			etype=DVBS;
			Dtype="SatUp";
		}
		else if(dvbtype=="THREED")
		{
			etype=THREED;
			Dtype="3DUp";
		}

		pXMLNODELIST childList=parser.GetNodeList(childNode);
		int count=parser.GetChildCount(childNode);

		int deviceid =0;
		ACE_INET_Addr Addr;
		mSocket.get_remote_addr(Addr);
		std::string IP=Addr.get_host_addr();

		eDVBType dvbtype = UNKNOWN;
		PROPMANAGER::instance()->GetDeviceID(IP,"0",deviceid);		//数字板卡默认为0;
		PROPMANAGER::instance()->GetDeviceType(deviceid,dvbtype);	//获取板卡监测类型

		for(int i=0;i<count;i++)
		{
			string Type,Desc,val;
			pXMLNODE temNode=parser.GetNextNode(childList);
			parser.GetAttrNode(temNode,"Type",Type);
			parser.GetAttrNode(temNode,"Desc",Desc);
			parser.GetAttrNode(temNode,"ErrNum",val);

			sCheck.AlarmType=ALARM_TR101_290;
			sCheck.DVBType=etype;
			sCheck.Freq=Freq;
			sCheck.STD=STD;
			sCheck.SymbolRate=SymbolRate;
			sCheck.TypeDesc=Desc;
			sCheck.TypedValue=val;
			sCheck.TypeID=Type;
			sCheck.DeviceID=StrUtil::Int2Str(deviceid);
			sCheck.CheckTime=time(0);
			APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_DEBUG,string("收到290错误：频点为"+Freq+"TypeID="+Type+"Value="+val),LOG_OUTPUT_FILE);
			ALARMMGR::instance()->CheckAlarm(sCheck,true);
		}
	}
	return true;
}


bool DeciceDataDeal::ProcessProgramAlarm(const char* buf,const int len)
{
	if (len < sizeof(RetMessage_Obj))
	{
		return false;
	}
	RetMessage_Handle alarm=(RetMessage_Obj*)buf;

	if (alarm->ph.header != 0x49 || alarm->ph.msg_type != MSG_RET_ALERT)
	{
		return false;
	}
	ACE_INET_Addr Addr;
	mSocket.get_remote_addr(Addr);
	std::string IP=Addr.get_host_addr();
	//
	std::list<int> devicedlist;
	PROPMANAGER::instance()->GetDeviceIndex(devicedlist,IP);
	//
	int num=0;
	//int Alarm[100];
	//memset(Alarm,0,sizeof(int)*100);
	int DeviceID[100];
	memset(DeviceID,0,sizeof(int)*100);
	//
	std::list<int>::iterator ptr = devicedlist.begin();
	for (;ptr!=devicedlist.end();ptr++)
	{
		DeviceID[num] = (*ptr);
		num++;
	}
	
	int Alarm[4] = {0x0,0x0,0x0,0x0};
	int status=alarm->status;					// 0x01020304 代表 Tuner 3:01；2:02；1:03；0:04
	//
	if (status == CARDREBOOT)		//板卡重启重发任务指令
	{
		//ACE_DEBUG ((LM_DEBUG,"(%T| %t) 板卡[%s]重新启动\n",IP.c_str()));
		string info = "板卡" + IP + "重新启动";
		APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_DEBUG,info,LOG_OUTPUT_FILE);
		std::list<int> devicelist;
		PROPMANAGER::instance()->GetDeviceIDByIP(IP,devicelist);
		for (std::list<int>::iterator ptr=devicelist.begin();ptr!=devicelist.end();++ptr)
		{
			TSFETCHERMGR::instance()->SetReSendSwitch(*ptr,true);
			OSFunction::Sleep(1);
		}
		return true;
	}
	//
	for(int Tunerid=0;Tunerid<4;Tunerid++)
	{
		switch(Tunerid)
		{
		case 0:
			Alarm[0] = (status&0x000000FF);
			break;
		case 1:
			Alarm[1] = (status>>8)&0x000000FF;
			break;
		case 2:
			Alarm[2] = (status>>16)&0x000000FF;
			break;
		case 3:
			Alarm[3] = (status>>24)&0x000000FF;
			break;
		default:
			break;
		}
	}
	//
	for(int Tunerid=0;Tunerid<4;Tunerid++)
	{
		//通过板卡IP和Tunerid（逻辑通道号） 获得板卡的实际通道号
		eDVBType dvbtype = UNKNOWN;
		
		PROPMANAGER::instance()->GetDeviceType(DeviceID[Tunerid],dvbtype);	//获取板卡监测类型

		if(DeviceID[Tunerid] == -1)
		{
			continue;
		}
		//通过实际通道号获取板卡任务
		sTaskInfo taskinfo;
		bool Rtn = BUSINESSLAYOUTMGR::instance()->QueryRunTaskInfo(DeviceID[Tunerid],taskinfo);

		//如果该通道内无运行任务，或者运行任务不是录制任务，则报警无效
		if (Rtn == false || (taskinfo.taskname != "AutoRecord" && taskinfo.taskname != "TaskRecord"))
		{
			continue;
		}
		if (Alarm[Tunerid] == 0x0)	//无异常
		{
			//DeviceID[Tunerid]通道没有报警continue否则往下走
			string header = StrUtil::Int2Str(taskinfo.dvbtype)+std::string("_")+StrUtil::Int2Str(DeviceID[Tunerid])+std::string("_")+taskinfo.channelid+std::string("_");
			if(ALARMMGR::instance()->GetAlarmTypeID(header).size()<=0)
			{
				continue;
			}
		}
		
		std::vector<sTypeDesc> vecTypeDesc;
		sCheckParam sCheck;
		sCheck.AlarmType	= ALARM_PROGRAM;
		sCheck.DVBType		= taskinfo.dvbtype;
		sCheck.ChannelID	= taskinfo.channelid;
		sCheck.Freq			= taskinfo.freq;
		sCheck.STD			= "";
		sCheck.SymbolRate	= "";
		sCheck.TypedValue	= "";
		sCheck.DeviceID		= StrUtil::Int2Str(DeviceID[Tunerid]);
		sCheck.CheckTime	= time(0);
		if (Alarm[Tunerid] != 0x0)
		{
			
			CollectAlarmType(Alarm[Tunerid],DeviceID[Tunerid],vecTypeDesc);
			for(int size=0;size<vecTypeDesc.size();size++)
			{
				sCheck.TypeDesc		= vecTypeDesc[size].Desc;
				sCheck.TypeID		= vecTypeDesc[size].Type;
				sCheck.mode			= "0";//报警

				//std::string info = std::string("通道[") + StrUtil::Int2Str(DeviceID[Tunerid]) + std::string("]节目异态:") + sCheck.TypeID.c_str();
				//SYSMSGSENDER::instance()->SendMsg(info,ATV,VS_MSG_SYSALARM);			
				ALARMMGR::instance()->CheckAlarm(sCheck,true);				
			}
			
		}
		
	}
	
	return true;
}

#if 0
bool DeciceDataDeal::ProcessProgramAlarm(const char* buf,const int len)
{
	if (len < sizeof(Alarm_Info_t))
	{
		return false;
	}

	Alarm_Info_t* alarm=(Alarm_Info_t*)buf;


	ACE_INET_Addr Addr;
	mSocket.get_remote_addr(Addr);
	std::string IP=Addr.get_host_addr();

	std::list<int> devicedlist;
	PROPMANAGER::instance()->GetDeviceIndex(devicedlist,IP);

	int num=0;
	int DeviceID[100];
	memset(DeviceID,0,sizeof(int)*100);
	//
	std::list<int>::iterator ptr = devicedlist.begin();
	for (;ptr!=devicedlist.end();ptr++)
	{
		DeviceID[num] = (*ptr);
		num++;
	}



	//通过板卡IP和Tunerid（逻辑通道号） 获得板卡的实际通道号
	eDVBType dvbtype = UNKNOWN;

	PROPMANAGER::instance()->GetDeviceType(DeviceID[alarm->ChannelNO],dvbtype);	//获取板卡监测类型

	if(DeviceID[alarm->ChannelNO] == -1)
	{
		return true;
	}
	//通过实际通道号获取板卡任务
	sTaskInfo taskinfo;
	bool Rtn = BUSINESSLAYOUTMGR::instance()->QueryRunTaskInfo(DeviceID[alarm->ChannelNO],taskinfo);

	//如果该通道内无运行任务，或者运行任务不是录制任务，则报警无效
	if (Rtn == false || (taskinfo.taskname != "AutoRecord" && taskinfo.taskname != "TaskRecord"))
	{
		return true;
	}

	sCheckParam sCheck;
	if(alarm->AlarmType == ALARM_FREEZE)
	{
		sCheck.TypeDesc = "0x4";
		sCheck.TypeID = "图像静止";
		sCheck.mode			= "0";//报警
	}
	else if(alarm->AlarmType == ALARM_BLACK)
	{
		sCheck.TypeDesc = "0x1";
		sCheck.TypeID = "黑屏";
		sCheck.mode			= "0";//报警
	}
	else if(alarm->AlarmType == ALARM_COLORBAR)
	{
		sCheck.TypeDesc = "0x2";
		sCheck.TypeID = "彩条";
		sCheck.mode			= "0";//报警
	}
// 	else if(alarm->AlarmType == ALARM_VOLUMEHIGH)
// 	{
// 		sCheck.TypeDesc		= ;
// 		sCheck.TypeID		= ;
// 		sCheck.mode			= "0";//报警
// 	}
// 	else if(alarm->AlarmType == ALARM_VOLUMELOW)
// 	{
// 		sCheck.TypeDesc		= ;
// 		sCheck.TypeID		= ;
// 		sCheck.mode			= "0";//报警
// 	}
	else if(alarm->AlarmType == ALARM_AUDIOLOST)
	{
		sCheck.TypeDesc = "0x10";
		sCheck.TypeID = "无伴音";
		sCheck.mode			= "0";//报警
	}
	else if(alarm->AlarmType == ALARM_LOSTSIGNAL)
	{
		sCheck.TypeDesc = "0xE";		
		sCheck.TypeID = "无载波";
		sCheck.mode			= "0";//报警
	}
	else
	{
		cout<< "通道[" << DeviceID[alarm->ChannelNO] << "]出现未知报警" << endl;
		return true;
	}

	sCheck.AlarmType	= ALARM_PROGRAM;
	sCheck.DVBType		= taskinfo.dvbtype;
	sCheck.ChannelID	= taskinfo.channelid;
	sCheck.Freq			= taskinfo.freq;
	sCheck.STD			= "";
	sCheck.SymbolRate	= "";
	sCheck.TypedValue	= "";
	sCheck.DeviceID		= StrUtil::Int2Str(DeviceID[alarm->ChannelNO]);
	sCheck.CheckTime	= (time_t)(alarm->AlarmTimeMs/1000);
			
	ALARMMGR::instance()->CheckAlarm(sCheck,true);				


	return true;
}

#endif 
void DeciceDataDeal::CollectAlarmType(int AlarmType,int deviceid,std::vector<sTypeDesc> &vTypeDesc)
{
	sTypeDesc  TypeDesc;
	static int NoSignal=0;
	static int UnCaAudio=0;
	static int UnCaVideo=0;
	if((AlarmType&0x80) ==0x0)
	{
		eDVBType dvbtype = UNKNOWN;
		PROPMANAGER::instance()->GetDeviceType(deviceid,dvbtype);
		if(dvbtype==ATV || dvbtype==CTV)
		{
			if (DEVICEACCESSMGR::instance()->CheckDeviceIDLock(deviceid))//该通道可以锁定 
			{
				if((AlarmType&V_MISS) ==V_MISS || (AlarmType&S_UNLOCK) == S_UNLOCK)
				{
					TypeDesc.Type = "0x8";
					TypeDesc.Desc = "无视频";
					vTypeDesc.push_back(TypeDesc);
					if((AlarmType&A_SILENCE) == A_SILENCE)
					{
						TypeDesc.Type = "0x10";
						TypeDesc.Desc = "无伴音";
						vTypeDesc.push_back(TypeDesc);
					}
				}
				else
				{
					if((AlarmType&A_SILENCE) == A_SILENCE)
					{
						TypeDesc.Type = "0x10";
						TypeDesc.Desc = "无伴音";
						vTypeDesc.push_back(TypeDesc);
					}
					bool bsliceblack = false;
					if((AlarmType&C_SLICE_V_STATIC) == C_SLICE_V_STATIC)
					{
						TypeDesc.Type = "0x2";
						TypeDesc.Desc = "彩条";
						vTypeDesc.push_back(TypeDesc);
						bsliceblack = true;
					}
					if((AlarmType&V_BLACK_V_STATIC) == V_BLACK_V_STATIC)
					{
						TypeDesc.Type = "0x1";
						TypeDesc.Desc = "黑屏";
						vTypeDesc.push_back(TypeDesc);
						bsliceblack = true;
					}
					if(!bsliceblack)
					{
						if((AlarmType&V_STATIC) == V_STATIC)
						{
							TypeDesc.Type = "0x4";
							TypeDesc.Desc = "图像静止";
							vTypeDesc.push_back(TypeDesc);
						}
					}
				}
			}
			else
			{
				TypeDesc.Type = "0xE";		
				TypeDesc.Desc = "无载波";
				vTypeDesc.push_back(TypeDesc);
			}

			if(g_freqneedlock[deviceid] == false)
			{
				g_freqneedlock[deviceid] = true;
			}
		}// end  if(dvbtype==ATV || dvbtype==CTV)
		else
		{
			if((AlarmType&A_SILENCE) == A_SILENCE)
			{
				TypeDesc.Type = "0x10";
				TypeDesc.Desc = "无伴音";
				vTypeDesc.push_back(TypeDesc);
				if(g_alarmchanscanval[deviceid] == false)
				{
					g_alarmchanscanval[deviceid] = true;
				}
			}
		}
	}
	else if((AlarmType&0x80) ==0x80)
	{
		if((AlarmType&NO_SIGNAL) == NO_SIGNAL)
		{
			TypeDesc.Type = "0x81";
			TypeDesc.Desc = "无信号";
			vTypeDesc.push_back(TypeDesc);
		}
		if((AlarmType&NO_VIDEO) == NO_VIDEO)
		{
			TypeDesc.Type = "0x82";
			TypeDesc.Desc = "无视频";
			vTypeDesc.push_back(TypeDesc);
		}
		if((AlarmType&NO_AUDIO) == NO_AUDIO)
		{
			TypeDesc.Type = "0x84";
			TypeDesc.Desc = "无音频";
			vTypeDesc.push_back(TypeDesc);
		}
		if((AlarmType&UNCA_VIDEO) == UNCA_VIDEO)
		{
			TypeDesc.Type = "0x88";
			TypeDesc.Desc = "视频解扰失败";
			vTypeDesc.push_back(TypeDesc);
		}	
		if((AlarmType&UNCA_AUDIO) == UNCA_AUDIO)
		{
			TypeDesc.Type ="0x90";
			TypeDesc.Desc = "音频解扰失败";
			vTypeDesc.push_back(TypeDesc);
		}
	}

}