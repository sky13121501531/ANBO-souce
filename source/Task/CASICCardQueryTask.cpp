
#include "CASICCardQueryTask.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/TypeDef.h"
#include "../Foundation/FileOperater.h"
#include "../Communications/TsSenderMgr.h"
#include "../DeviceAccess/HttpOpe.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "../Communications/XMLSend.h"

using namespace std;

CASICCardQueryTask::CASICCardQueryTask() : DeviceIndependentTask()
{

}

CASICCardQueryTask::CASICCardQueryTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}
CASICCardQueryTask::~CASICCardQueryTask()
{
}
void CASICCardQueryTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]CAS IC卡信息查询任务开始 !\n",DeviceID));

	XmlParser psr(strStandardXML.c_str());
	pXMLNODE rootNode=psr.GetRootNode();
	std::string msgid,srccode,dstcode,URL;
	psr.GetAttrNode(rootNode,"MsgID",msgid);
	psr.GetAttrNode(rootNode,"SrcCode",srccode);
	psr.GetAttrNode(rootNode,"DstCode",dstcode);
	psr.GetAttrNode(rootNode,"SrcURL",URL);

	pXMLNODE queryNode=psr.GetNodeFirstChild(rootNode);
	std::string queryName=psr.GetNodeName(queryNode);
	if (queryName!="ICCardQuery")
	{
		printf("错误的IC卡信息查询任务\n");
		return;
	}

	std::string date;
	psr.GetAttrNode(queryNode,"Date",date);
	if (date=="")
	{
		SetFinised();
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]CAS IC卡信息查询任务获取时间失败 !\n",DeviceID));
		return;
	}
	std::string tmp=date.erase(4,1);
	tmp=tmp.erase(6,1);

	std::string PATH,HttpPath;
	PROPMANAGER::instance()->GetCASFilePath(PATH);
	PROPMANAGER::instance()->GetHttpPath(HttpPath);

	std::string CasIp;
	int  CasPort;
	PROPMANAGER::instance()->GetCASIP(CasIp);
	PROPMANAGER::instance()->GetCASPort(CasPort);
	std::string reqURL="http://"+CasIp+":"+StrUtil::Int2Str(CasPort)+PATH+tmp+"_"+"CAS"+"_"+"ICCardReport.zip";//构造请求的URL
	std::string localurl="http://" + PROPMANAGER::instance()->GetHttpServerIP() + ":" + \
		PROPMANAGER::instance()->GetHttpServerPort() +HttpPath+tmp+"_"+"CAS"+"_"+"ICCardReport.zip";
	
	HttpOpe http(CasIp.c_str(),CasPort);
	ACE_SOCK_Stream socketHandle;
	std::string recXML;
	http.GetRecvHandle(reqURL,socketHandle,false);
	
	char RecvBuf[1024] = {0};
	std::string LocalPath;
	PROPMANAGER::instance()->GetShareDir(LocalPath);
	
	
	ssize_t RecvCount=0;
	bool zipflag=false;
	std::string filename;
	bool isheader=false;
	FileOperater ZipFile;
	while(1)
	{
		ACE_Time_Value TimeOut(10);
		RecvCount= socketHandle.recv(RecvBuf,1024,&TimeOut);
		if(RecvCount<=0)
			break;
		char *p=NULL;
		if(!isheader)
		{
			std::string str=RecvBuf;
			if(str.find("200 OK")==std::string::npos)
			{
				break;
			}
			else
			{
				filename=LocalPath+tmp+"_"+"CAS"+"_"+"ICCardReport.zip";
				ZipFile.CreateNewFile(filename.c_str());
			}
			p=strstr(RecvBuf,"\r\n\r\n");
			if(p!=NULL)
			{
				isheader=true;
				ZipFile.WriteBuf(p+4,RecvCount-(p+4-RecvBuf));
			}
			if(!zipflag)
				zipflag =true;
			continue;
		}
		else
		{
			ZipFile.WriteBuf(RecvBuf,RecvCount);
		}
		if(!zipflag)
			zipflag =true;
	}
	if(zipflag)
		ZipFile.Close();
	if(!zipflag)
	{
		bool xmlflag=false;
		reqURL="http://"+CasIp+":"+StrUtil::Int2Str(CasPort)+PATH+tmp+"_"+"CAS"+"_"+"ICCardReport.xml";
		HttpOpe http1(CasIp.c_str(),CasPort);
		ACE_SOCK_Stream socketHandle1;
		http1.GetRecvHandle(reqURL,socketHandle1,false);
		memset(RecvBuf,0,sizeof(RecvBuf));
		while(1)
		{
			ACE_Time_Value TimeOut(10);
			RecvCount= socketHandle1.recv(RecvBuf,1024,&TimeOut);
			char *p=NULL;
			if(!isheader)
			{
				std::string str=RecvBuf;
				if(str.find("200 OK")==std::string::npos)
				{
					break;
				}
				p=strstr(RecvBuf,"\r\n\r\n");
				if(p!=NULL)
				{
					isheader=true;
					recXML.append(p+4,RecvCount-(p+4-RecvBuf));
				}
				memset(RecvBuf,0,sizeof(RecvBuf));
				if(!xmlflag)
					xmlflag=true;
				continue;
			}
			if(RecvCount<=0)
				break;
			recXML.append(RecvBuf,RecvCount);
			memset(RecvBuf,0,sizeof(RecvBuf));
			if(!xmlflag)
				xmlflag=true;
		}
		if(xmlflag)
		{
			char* msg=(char*)recXML.c_str();
			int count=0;
			while (((*msg)!='<')&&(count++<recXML.length()))
			{
				msg++;
			}
			std::string srcXML=std::string(msg);
			filename=LocalPath+tmp+"_"+"CAS"+"_"+"ICCardReport.xml";
			localurl="http://" + PROPMANAGER::instance()->GetHttpServerIP() + ":" + \
				PROPMANAGER::instance()->GetHttpServerPort() + HttpPath+tmp+"_"+"CAS"+"_"+"ICCardReport.xml";
			XmlParser file;
			file.Set_xml(srcXML);
			file.SaveAsFile(filename.c_str());
			char * cheader= "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg><Return></Return></Msg>";
			XmlParser headParser(cheader);
			pXMLNODE headRootNode = headParser.GetNodeFromPath( "Msg" );//根节点msg

			headParser.SetAttrNode( "Version",this->GetVersion(),headRootNode );//版本
			headParser.SetAttrNode( "MsgID",msgid,headRootNode );//消息id属性
			headParser.SetAttrNode( "Type",string("CASUp"),headRootNode );//消息类型
			headParser.SetAttrNode( "DateTime",TimeUtil::GetCurDateTime(),headRootNode );//当前时间
			headParser.SetAttrNode( "SrcCode",dstcode,headRootNode );//本机标识，可通过接口获得
			headParser.SetAttrNode( "DstCode",srccode,headRootNode );//目标机器标识
			headParser.SetAttrNode("DstURL",URL,headRootNode);
			headParser.SetAttrNode( "ReplyID",msgid,headRootNode );//回复的消息id

			pXMLNODE retNode = headParser.GetNodeFromPath("Msg/Return");
			headParser.SetAttrNode( "Type",string("ICCardQuery"),retNode );

			headParser.SetAttrNode( "Value",string("0"),retNode );//return节点的value属性
			headParser.SetAttrNode( "Desc",string("成功"),retNode );//return节点的Desc属性
			headParser.SetAttrNode( "Redirect",string(""),retNode );//return节点的Comment属性
			headParser.SaveToString( recXML );
			size_t pos=srcXML.find("<ICCardQueryReport");
			if(pos!=string::npos)
			{
				size_t pos1=srcXML.find("</Msg>");
				std::string body=srcXML.substr(pos,pos1-pos);
				size_t pos2=recXML.find("</Msg>");
				recXML.insert(pos2-1,body);
			}
		}
		else
		{
			char * cheader= "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg><Return></Return><ICCardQueryReport></ICCardQueryReport></Msg>";
			XmlParser headParser(cheader);
			pXMLNODE headRootNode = headParser.GetNodeFromPath( "Msg" );//根节点msg

			headParser.SetAttrNode( "Version",this->GetVersion(),headRootNode );//版本
			headParser.SetAttrNode( "MsgID",msgid,headRootNode );//消息id属性
			headParser.SetAttrNode( "Type",string("CASUp"),headRootNode );//消息类型
			headParser.SetAttrNode( "DateTime",TimeUtil::GetCurDateTime(),headRootNode );//当前时间
			headParser.SetAttrNode( "SrcCode",dstcode,headRootNode );//本机标识，可通过接口获得
			headParser.SetAttrNode( "DstCode",srccode,headRootNode );//目标机器标识
			headParser.SetAttrNode("DstURL",URL,headRootNode);
			headParser.SetAttrNode( "ReplyID",msgid,headRootNode );//回复的消息id

			pXMLNODE retNode = headParser.GetNodeFromPath("Msg/Return");
			headParser.SetAttrNode( "Type",string("ICCardQuery"),retNode );

			headParser.SetAttrNode( "Value",string("1"),retNode );//return节点的value属性
			headParser.SetAttrNode( "Desc",string("没有查到相应数据"),retNode );//return节点的Desc属性
			headParser.SetAttrNode( "Redirect",string(""),retNode );//return节点的Comment属性
			pXMLNODE ReportNode=headParser.GetNodeFromPath("Msg/ICCardQueryReport");
			headParser.SetAttrNode("Date",TimeUtil::GetCurDate(),ReportNode);
			headParser.SaveToString( recXML );
		}
	}
	else
	{
		char * cheader= "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg><Return></Return></Msg>";
		XmlParser headParser(cheader);
		pXMLNODE headRootNode = headParser.GetNodeFromPath( "Msg" );//根节点msg

		headParser.SetAttrNode( "Version",this->GetVersion(),headRootNode );//版本
		headParser.SetAttrNode( "MsgID",msgid,headRootNode );//消息id属性
		headParser.SetAttrNode( "Type",string("CASUp"),headRootNode );//消息类型
		headParser.SetAttrNode( "DateTime",TimeUtil::GetCurDateTime(),headRootNode );//当前时间
		headParser.SetAttrNode( "SrcCode",dstcode,headRootNode );//本机标识，可通过接口获得
		headParser.SetAttrNode( "DstCode",srccode,headRootNode );//目标机器标识
		headParser.SetAttrNode("DstURL",URL,headRootNode);
		headParser.SetAttrNode( "ReplyID",msgid,headRootNode );//回复的消息id

		pXMLNODE retNode = headParser.GetNodeFromPath("Msg/Return");
		headParser.SetAttrNode( "Type",string("ICCardQuery"),retNode );

		headParser.SetAttrNode( "Value",string("0"),retNode );//return节点的value属性
		headParser.SetAttrNode( "Desc",string("成功"),retNode );//return节点的Desc属性
		headParser.SetAttrNode( "Redirect",localurl,retNode );//return节点的Comment属性
		headParser.SaveToString( recXML );

	} 
	SendXML(recXML);
	//SendCASXML(URL,recXML);
	SetFinised(); 

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]CAS IC卡信息查询任务停止 !\n",DeviceID));
}

string CASICCardQueryTask::GetTaskName()
{
	return std::string("IC卡信息查询任务");
}
std::string CASICCardQueryTask::GetObjectName()
{
	return std::string("CASICCardQueryTask");
}

