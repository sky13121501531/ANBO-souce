#include "SMSProdPurchaseQueryTask.h"
#include "../Foundation/XmlParser.h"
#include "../DeviceAccess/HttpOpe.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/FileOperater.h"
#include "ace/SOCK_Connector.h"
#include "ace/INET_Addr.h"
#include "ace/Time_value.h"
#include "ace/Handle_Set.h"
#include "ace/os.h"
#include "../Communications/SMSReceiver.h"
#include "../Communications/XMLSend.h"
#include "../Communications/CommunicationMgr.h"
#include "../Foundation/OSFunction.h"

SMSProdPurchaseQueryTask::SMSProdPurchaseQueryTask(void)
{
}

SMSProdPurchaseQueryTask::SMSProdPurchaseQueryTask( std::string strXML ):DeviceIndependentTask(strXML)
{
	strStandardXML=strXML;
}
SMSProdPurchaseQueryTask::~SMSProdPurchaseQueryTask(void)
{
}

void SMSProdPurchaseQueryTask::Run()
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]SMS 产品购买信息查询任务开始执行 !\n",DeviceID));

	std::string RetXml;//回复给上层页面的xml
	std::string Date;
	std::string InstQuery;
	XmlParser StdParser(strStandardXML.c_str());
	pXMLNODE TaskNode=StdParser.GetNodeFromPath("Msg/ProdPurchaseQuery");
	StdParser.GetAttrNode(TaskNode,"InstQuery",InstQuery);
	
	if (InstQuery=="0")
	{
		StdParser.GetAttrNode(TaskNode,"Date",Date);
	}
	else
	{
		Date=TimeUtil::GetCurDate();
	}
	Date=Date.erase(4,1);//删除日期字符串中的字符‘—’
	Date=Date.erase(6,1);

	std::string SMSIP;
	std::string SMSPath;
	std::string RecvSMSXml;
	int SMSPort;
    PROPMANAGER::instance()->GetSMSIP(SMSIP);
	PROPMANAGER::instance()->GetSMSURL(SMSPath);
	PROPMANAGER::instance()->GetSMSPort(SMSPort);
	HttpOpe HttpSMS(SMSIP.c_str(),SMSPort);
	std::string tempstr=strStandardXML;
	size_t SrcURLPos=tempstr.find("SrcURL=\"");
	size_t LowerPos=tempstr.find("\"",SrcURLPos+8);
	tempstr.erase(SrcURLPos,LowerPos-SrcURLPos+1);
	
	//向sms服务端的smsAgent发送任务xml
	XMLSend::SendXML(SMSPath,strStandardXML);
	
	/* 循环判断tomcat有没有接收到sms服务器回复的xml */
	std::string SMSUpFileName;
	while ( !COMMUNICATIONMGR::instance()->AccessSmsRecv()->getSmsFileName(GetObjectName(),SMSUpFileName) )
	{
		OSFunction::Sleep( 1 );
	}

//	OSFunction::Sleep(3);

	//获取sms服务器返回的xml的内容
	XmlParser parser;
	parser.LoadFromFile(SMSUpFileName.c_str());
	parser.SaveToString(RecvSMSXml);
	DeleteFile(SMSUpFileName.c_str());

	std::string URL;
	if (RecvSMSXml.length()>0)	//接收到的xml不为空
	{
		pXMLNODE RetNode=parser.GetNodeFromPath("Msg/Return");
		std::string RedirectURL;
		parser.GetAttrNode(RetNode,"Redirect",RedirectURL);
		if (RedirectURL.length()>0)		//有重定向文件，即压缩包
		{
			std::string ZipIP;	//zip文件的ip
			std::string ZipPort="80";
			std::string ZipFileName=Date+"_SMS_ProdPurchaseQueryR.zip";
			std::string FileName=ZipFileName;
			std::string ShareDir;
			PROPMANAGER::instance()->GetShareDir(ShareDir);
			ZipFileName=ShareDir+ZipFileName;

			/* 获取端口号(默认为80) */
			std::string tempUrl = "";
			if (RedirectURL.find("http://")!=std::string::npos)
			{
				size_t pos1=RedirectURL.find(":",7);
				size_t pos2=RedirectURL.find("/",7);
				if(pos1!=std::string::npos)
				{
					ZipIP=RedirectURL.substr(7,pos1-7);
				}
				else if (pos2!=std::string::npos)
				{
					ZipIP=RedirectURL.substr(7,pos2-7);
				}
				if (pos1!=std::string::npos&&pos2!=std::string::npos)
				{
					ZipPort=RedirectURL.substr(pos1+1,pos2-pos1-1);
				}
				tempUrl = RedirectURL.substr( pos2+1 );
			}

			//连接zip文件所在的服务器，下载zip文件
			HttpOpe HttpZipFileSvr(ZipIP.c_str(),StrUtil::Str2Int(ZipPort));
			ACE_SOCK_Stream ZipFileSvrSock;
			HttpZipFileSvr.GetRecvHandle(tempUrl,ZipFileSvrSock,false);
			
			char RecvBuf[1024] = {0};
			ssize_t RecvCount=0;
			FileOperater ZipFile;
			bool IsFindResHeader=false;		//是否http返回的第一个包
			while(true)
			{
				ACE_Time_Value TimeOut(10);
				RecvCount= ZipFileSvrSock.recv(RecvBuf,1024,&TimeOut);
				if(RecvCount<=0)
					break;
				char *p=NULL;
				if(!IsFindResHeader)
				{
					std::string str=RecvBuf;
					if(str.find("200 OK")==std::string::npos)
					{
						break;
					}
					else
					{
						ZipFile.CreateNewFile(ZipFileName.c_str());
					}
					p=strstr(RecvBuf,"\r\n\r\n");
					if(p!=NULL)
					{
						IsFindResHeader=true;
						ZipFile.WriteBuf(p+4,RecvCount-(p+4-RecvBuf));
					}

					continue;
				}
				else
				{
					ZipFile.WriteBuf(RecvBuf,RecvCount);
				}
			}
			ZipFile.Close();
			std::string dir;
			PROPMANAGER::instance()->GetHttpPath(dir);
			URL=std::string("http://")+PROPMANAGER::instance()->GetHttpServerIP()+":"+PROPMANAGER::instance()->GetHttpServerPort()+dir+FileName;
		
		}	//RedirectURL.length()>0
	
	}	//RecvSMSXml.length()>0

	CreateRXml(URL,RecvSMSXml,RetXml);
	SendXML(RetXml);
	SetFinised();
	bRun = false;
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]SMS 产品购买信息查询任务停止 !\n",DeviceID));
	
}

std::string SMSProdPurchaseQueryTask::GetObjectName()
{
	return "SMSProdPurchaseQuery";
}
std::string SMSProdPurchaseQueryTask::GetTaskName()
{
	return "SMS 产品购买信息查询任务";
}

bool SMSProdPurchaseQueryTask::CreateRXml( std::string URL,std::string XmlFromSMS,std::string& RXml )
{
	if (XmlFromSMS.length()>0)
	{
		XmlParser parser(XmlFromSMS.c_str());
		pXMLNODE RootNode=parser.GetRootNode();
		pXMLNODE RetNode=parser.GetNodeFromPath("Msg/Return");
		parser.SetAttrNode("Version",GetVersion(),RootNode);
		parser.SetAttrNode("MsgID",GetMsgID(),RootNode);
		parser.SetAttrNode("Type",std::string("SMSUp"),RootNode);
		parser.SetAttrNode("DateTime",TimeUtil::GetCurDateTime(),RootNode);
		parser.SetAttrNode("SrcCode",GetDstCode(),RootNode);
		parser.SetAttrNode("DstCode",GetSrcCode(),RootNode);
		parser.SetAttrNode("DstURL",GetSrcURL(),RootNode);
		parser.SetAttrNode("ReplyID",GetMsgID(),RootNode);
		parser.SetAttrNode("Redirect",URL,RetNode);
		parser.SaveToString(RXml);

	}
	return true;
}
