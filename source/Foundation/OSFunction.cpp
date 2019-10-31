#include "OSFunction.h"
#include "ace/os.h"
#include "PropManager.h"
#include <list>
#include "../BusinessProcess/PSISIMgr.h"
#include "../DBAccess/DBManager.h"  //wz_101229
#include "../Communications/SysMsgSender.h"
#include "StrUtil.h"
#include "TimeUtil.h"
#include "AppLog.h"

using namespace std;
void OSFunction::Sleep(int sencond, int millisecond)
{
	ACE_Time_Value tv(sencond,millisecond*1000);
	ACE_OS::sleep(tv);
}


string OSFunction::GetXmlMsgID(string smsgid)
{
	try
	{
		string strtime = "0000";
		string smsgid = StrUtil::Long2Str(GetTickCount());
		//
		if(smsgid.size()>5)
		{
			string tem = smsgid.substr(smsgid.size()-5,1);
			int itex = StrUtil::Str2Int(tem);
			if(itex <= 0)
			{
				itex = 1;
			}
			//
			int z = StrUtil::Str2Int(smsgid.substr(smsgid.size()-(itex%smsgid.size())-1,1));
			int y = StrUtil::Str2Int(smsgid.substr(smsgid.size()-((itex*2)%smsgid.size())-1,1));
			int x = StrUtil::Str2Int(smsgid.substr(smsgid.size()-((itex*itex)%smsgid.size())-1,1));
			strtime.replace(strtime.size()-4,1,StrUtil::Int2Str((x+3)%10));
			strtime.replace(strtime.size()-2,1,StrUtil::Int2Str((y+2)%10));
			strtime.replace(strtime.size()-1,1,StrUtil::Int2Str((z+1)%10));
			x=(x+3)%10;
			y=(y+2)%10;
			z=(z+1)%10;
			int r = x+y+z;
			int h = x-y-z;
			h=h+30;
			int q=r*h;
			int tm = q%9;
			tm=tm+1;
			strtime.replace(strtime.size()-3,1,StrUtil::Int2Str(tm));
		}
		smsgid += strtime;
		return smsgid;
	}
	catch (...)
	{
		std::cout<<""<<std::endl;
		return "0";
	}
}

void OSFunction::CompressFile( std::string dstZipPath,std::string srcFilePath )
{
	//wz_101229
	string createtime = TimeUtil::GetCurDateTime();						//压缩文件创建时间
	string DBfilename = dstZipPath;										//压缩文件名
	string expiredays = PROPMANAGER::instance()->GetTempFileExpire();	//过期时间从配置文件获取  jidushan 110406
	bool DBret = DBMANAGER::instance()->AddTempFile(DBfilename, createtime, expiredays);
	if (!DBret)
	{
		string msg = string("压缩文件信息入库失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
		//return false;
	}
	string msg = string("压缩文件信息入库成功");
	SYSMSGSENDER::instance()->SendMsg(msg);

	std::string command=std::string("winrar.exe a -ibck -ep -df ")+dstZipPath+std::string(" ")+srcFilePath;
	system(command.c_str());
}
void OSFunction::GetFileName(eDVBType dvbType,std::string taskType,std::string& strFileName)
{
	std::string xmlPath;
	PROPMANAGER::instance()->GetXmlTablePath(dvbType,xmlPath);

	if (stricmp(taskType.c_str(),"ChannelScanQueryTask") == 0)  //频道扫描
	{
		strFileName = xmlPath + string("ChannelScanQuery.xml");
		return;
	}
	else if (stricmp(taskType.c_str(),"EPGQuery") == 0)     //EPG查询
	{
		
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		std::string searchPath = xmlPath + string("*.*");
		hFind = FindFirstFile(searchPath.c_str(), &FindFileData);
        string tempTime;
		while (hFind != INVALID_HANDLE_VALUE)
		{
			if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0)
			{
				string FileNameTemp = string(FindFileData.cFileName);
				string subString = FileNameTemp.substr(0,8); 
				if (strcmp(subString.c_str(),"EPGQuery") == 0)
				{
                  if (strcmp(tempTime.c_str(),FileNameTemp.substr(8,14).c_str())<0)
                  {
					  tempTime = FileNameTemp.substr(8,14);    //获取最新EPG文件
                  }
				}
			}
			if (!FindNextFile(hFind,&FindFileData))
			{
				break;
			}
		}
		FindClose(hFind);
		strFileName =  string("EPGQuery") + tempTime +string(".zip");
		return;

	}
	else if (stricmp(taskType.c_str(),"TableQueryTask") == 0)   //表查询
	{

		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		std::string searchPath = xmlPath + string("*.*");
		hFind = FindFirstFile(searchPath.c_str(), &FindFileData);
		string tempTime;
		while (hFind != INVALID_HANDLE_VALUE)
		{
			if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0)
			{
				string FileNameTemp = string(FindFileData.cFileName);
				string subString = FileNameTemp.substr(0,10); 
				if (strcmp(subString.c_str(),"TableQuery") == 0)
				{
					if (strcmp(tempTime.c_str(),FileNameTemp.substr(10,14).c_str())<0)
					{
						tempTime = FileNameTemp.substr(10,14);    //获取最新TableQuery文件
					}
				}
			}
			if (!FindNextFile(hFind,&FindFileData))
			{
				break;
			}
		}
		FindClose(hFind);
		strFileName = string("TableQuery") + tempTime +string(".zip");
		return;
	}
}
string OSFunction::CreateTaskTypeXml(int deviceid)
{
	string DeviceIp = PROPMANAGER::instance()->GetDeviceIp();
	string DevicePort = PROPMANAGER::instance()->GetDevicePort();

	string LogIp = PROPMANAGER::instance()->GetLogIP();
	string LogPort = PROPMANAGER::instance()->GetLogPort();

	eDVBType dvbtype = UNKNOWN;
	PROPMANAGER::instance()->GetDeviceType(deviceid,dvbtype);
	string tasktype;
	PROPMANAGER::instance()->IsDeviceAvaiable(deviceid,"TSAnalyze",dvbtype) ? tasktype = "TSAnalyze" : tasktype = "TS";

	char* xml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>				\
			   <Msg DVBType=\"\">															\
			   <DeviceTaskType TaskType=\"\" />												\
			   <HostAddr IP=\"\" Port=\"\" />												\
			   <LogAddr IP=\"\" Port=\"\" />												\
			   </Msg>";

	XmlParser Parser( xml );
	pXMLNODE RootNode=Parser.GetRootNode();//根节点msg

	//3D需要保证发送给httpserver的码流分析指令xml中的dvbtype为dvbc
	eDVBType tempeDvbtype = dvbtype;
	if (tempeDvbtype == THREED)
	{
		tempeDvbtype = DVBC;			 
	}
	string strdvbtype = GetStrDVBType(tempeDvbtype);
	Parser.SetAttrNode("DVBType",strdvbtype,RootNode);

	pXMLNODE TypeNode = Parser.GetNodeFromPath( "Msg/DeviceTaskType" );
	Parser.SetAttrNode("TaskType",tasktype,TypeNode);

	pXMLNODE DeviceNode = Parser.GetNodeFromPath( "Msg/HostAddr" );
	Parser.SetAttrNode("IP",DeviceIp,DeviceNode);
	Parser.SetAttrNode("Port",DevicePort,DeviceNode);

	pXMLNODE LogNode = Parser.GetNodeFromPath( "Msg/LogAddr" );
	Parser.SetAttrNode("IP",LogIp,LogNode);
	Parser.SetAttrNode("Port",LogPort,LogNode);
	if (tasktype=="TSAnalyze")
	{
		pXMLNODE ChanneScanNode=Parser.CreateNodePtr(RootNode,"ChannelScanSet");
		
		sFreqScanInfo freqscaninfo;

		PROPMANAGER::instance()->GetFreqScanInfo(dvbtype,freqscaninfo);

		pXMLNODE child=Parser.CreateNodePtr(ChanneScanNode,"ScanInfo");
		Parser.SetAttrNode("CentreFreq",freqscaninfo.CenterFreq,child);
		Parser.SetAttrNode("SymbolRate",freqscaninfo.SymbolRate,child);
		Parser.SetAttrNode("QAM",freqscaninfo.QAM,child);
	}
	else
	{
		/*pXMLNODE OSDNode=Parser.CreateNodePtr(RootNode,"OSDInfo");
		OSDInfo info;
		PROPMANAGER::instance()->GetOSDInfo(info);
		Parser.SetAttrNode("InfoOSD",info.Info,OSDNode);
		Parser.SetAttrNode("InfoOSDX",info.InfoX,OSDNode);
		Parser.SetAttrNode("InfoOSDY",info.InfoY,OSDNode);
		Parser.SetAttrNode("TimeOSDType",info.TimeType,OSDNode);
		Parser.SetAttrNode("TimeOSDX",info.TimeX,OSDNode);
		Parser.SetAttrNode("TimeOSDY",info.TimeY,OSDNode);
		Parser.SetAttrNode("SystemClock",(long)time(0),OSDNode);*/
		return "";
	}

	string retXml;
	Parser.SaveToString(retXml);
	return retXml;
}

string OSFunction::CreateCardRebootXml(int deviceid)
{
	eDVBType dvbtype = UNKNOWN;
	PROPMANAGER::instance()->GetDeviceType(deviceid,dvbtype);

	string strdvbtype = GetStrDVBType(dvbtype);

	string tasktype = "Reboot";
	
	char* xml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>				\
			   <Msg DVBType=\"\">															\
			   <DeviceTaskType TaskType=\"\" />												\
			   <HostAddr IP=\"\" Port=\"\" />												\
			   <LogAddr IP=\"\" Port=\"\" />												\
			   </Msg>";

	XmlParser Parser( xml );
	pXMLNODE RootNode=Parser.GetRootNode();//根节点msg

	Parser.SetAttrNode("DVBType",strdvbtype,RootNode);

	pXMLNODE TypeNode = Parser.GetNodeFromPath( "Msg/DeviceTaskType" );
	Parser.SetAttrNode("TaskType",tasktype,TypeNode);

	string retXml;
	Parser.SaveToString(retXml);
	return retXml;
}

string OSFunction::CreateTableVersionXml(enumDVBType dvbtype)
{
	return "";
}

eDVBType OSFunction::GetEnumDVBType(string StrDVBType)
{
	eDVBType eDVBType = UNKNOWN;
	if (StrDVBType == "CTTB" || StrDVBType =="1")
	{
		eDVBType = CTTB;
	}
	else if (StrDVBType == "CMMB" || StrDVBType =="2")
	{
		eDVBType = CMMB;
	}
	else if (StrDVBType == "DVBC"|| StrDVBType =="3")
	{
		eDVBType = DVBC;
	}
	else if (StrDVBType == "RADIO"|| StrDVBType =="4")
	{
		eDVBType = RADIO;
	}
	else if (StrDVBType == "ATV"|| StrDVBType =="5")
	{
		eDVBType = ATV;
	}
	else if (StrDVBType == "AM"|| StrDVBType =="6")
	{
		eDVBType = AM;
	}
	else if(StrDVBType == "DVBS"|| StrDVBType =="7")
	{
		eDVBType = DVBS;
	}
	else if (StrDVBType == "CTV"|| StrDVBType =="8")
	{
		eDVBType = CTV;
	}
	else if (StrDVBType == "THREED"|| StrDVBType =="9")
	{
		eDVBType = THREED;
	}
	else
	{
		eDVBType = UNKNOWN;
	}
	return eDVBType;
}
string OSFunction::GetStrDVBType(eDVBType eDVBType)
{
	string StrDVBType;
	switch (eDVBType)
	{
	case CTTB:
		StrDVBType = "CTTB";
		break;
	case CMMB:
		StrDVBType = "CMMB";
		break;
	case DVBC:
		StrDVBType = "DVBC";
		break;
	case RADIO:
		StrDVBType = "RADIO";
		break;
	case ATV:
		StrDVBType = "ATV";
		break;
	case AM:
		StrDVBType = "AM";
		break;
	case DVBS:
		StrDVBType = "DVBS";
		break;
	case CTV:
		StrDVBType = "CTV";
		break;
	case THREED:
		StrDVBType = "THREED";
		break;
	default:
		StrDVBType = "UNKNOWN";
	}
	return StrDVBType;
}
bool OSFunction::SetOSDTimeMode(int mode,OSDInfo& osd)
{
	switch (mode)
	{
	case 1:
		osd.TimeType =  StrUtil::Int2Str(OSD_CYEAR | OSD_YEAR4 |  OSD_MONTH | OSD_CWEEK | OSD_HOUR24);	//2000年1月1日 星期一 15:00:00
		break;
	case 2:
		osd.TimeType =  StrUtil::Int2Str(OSD_CYEAR | OSD_YEAR4 |  OSD_MONTH | OSD_HOUR24);				//2000年1月1日 15:00:00
		break;
	case 3:
		osd.TimeType =  StrUtil::Int2Str(OSD_CYEAR | OSD_YEAR4 |  OSD_MONTH | OSD_CWEEK);				//2000年1月1日 星期一
		break;
	case 4:
		osd.TimeType =  StrUtil::Int2Str(OSD_CYEAR | OSD_YEAR4 |  OSD_MONTH);							//2000年1月1日
		break;
	case 5:
		osd.TimeType =  StrUtil::Int2Str(OSD_YEAR4 |  OSD_MONTH | OSD_CWEEK | OSD_HOUR24);				//2000-1-1 星期一 15:00:00
		break;
	case 6:
		osd.TimeType =  StrUtil::Int2Str(OSD_YEAR4 |  OSD_MONTH | OSD_HOUR24);							//2000-1-1 15:00:00
		break;
	case 7:
		osd.TimeType =  StrUtil::Int2Str(OSD_YEAR4 |  OSD_MONTH | OSD_CWEEK);							//2000-1-1 星期一
		break;
	case 8:
		osd.TimeType =  StrUtil::Int2Str(OSD_YEAR4 |  OSD_MONTH);										//2000-1-1
		break;
	//case 9:
	//	osd.TimeType =  StrUtil::Int2Str(OSD_HOUR24);													//15:00:00
	//	break;
	default:
		osd.TimeType =  StrUtil::Int2Str(OSD_YEAR4 |  OSD_MONTH | OSD_HOUR24);							//2000-1-1 15:00:00
		break;
	}
	return true;
}

void OSFunction::ExitProcess(string info)
{
	
	cout<<"\n\n";
	for (int i=5;i>0;--i)
	{
		cout<<"\t"<<info<<" 该进程将在"<<i<<"秒后退出\r";
		OSFunction::Sleep(1);
	}
	string retT = info;
	retT.append(string(TimeUtil::GetCurTime())).append("END-END");
	APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,info,LOG_OUTPUT_BOTH);
	exit(-1);
}

string   OSFunction::GetAlarmTypeID(eDVBType type,string alarmtype)
{
	string TypeID;
	if(type==DVBC||type==DVBS||type==CTTB||type==THREED)
	{
		if(alarmtype=="NoSignal")
		{
			if(type==CTTB)
				TypeID = "0";
			else
				TypeID = "1";
		}
		else if(alarmtype=="ImageStill")
		{
			TypeID = "31";
		}
		else if(alarmtype=="BlackBurst")
		{
			TypeID = "32";
		}
		else if(alarmtype=="NoAudio")
		{
			TypeID = "33";
		}
	}
	else if(type==ATV||type ==CTV)
	{
		if(alarmtype=="NoSignal")
		{
				TypeID = "10";
		}
		else if(alarmtype=="ImageStill")
		{
			TypeID = "13";
		}
		else if(alarmtype=="BlackBurst")
		{
			TypeID = "18";
		}
		else if(alarmtype=="NoAudio")
		{
			TypeID = "12";
		}
		else if(alarmtype == "NoVideo")
		{
			TypeID = "11";
		}
		else if(alarmtype == "ColorBar")
		{
			TypeID = "14";
		}
	}
	else if(type==RADIO||type==AM)
	{
		if(alarmtype == "NoSignal")
		{
			TypeID = "23";
		}
		else if(alarmtype =="NoAudio")
		{
			TypeID = "24";
		}
	}
	return TypeID;
}