
#include "QualityRealtimeQueryTask.h"
#include "Scheduler.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/XmlParser.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "../DeviceAccess/TCPDeviceAccess.h"
#include "./TranslateXMLForDevice.h"
#include "../Communications/QualitySender.h"
#include "../Communications/SysMsgSender.h"
#include "../Alarm/AlarmMgr.h"
#include "../BusinessProcess/QualityCompensationMgr.h"
#include "ace/Synch.h"
#include "ace/OS.h"
#include "../DeviceAccess/FetchQualityLevel.h"
#include <vector>
#include <iostream>
using namespace std;
extern bool g_realqulity;
extern int FgNum;


QualityRealtimeQueryTask::QualityRealtimeQueryTask() : DeviceRelatedTask()
{

}

QualityRealtimeQueryTask::QualityRealtimeQueryTask(std::string strXML) : DeviceRelatedTask(strXML)
{
	//获取执行周期和频点信息
	XmlParser parser( strXML.c_str());
	pXMLNODE rootNode ;
	pXMLNODE childNode ;
	rootNode=parser.GetRootNode();
	childNode=parser.GetNodeFirstChild(rootNode);
	parser.GetAttrNode( childNode,"Periodicity",Periodicity );

	pXMLNODE tempChildNode = parser.GetNodeFirstChild( childNode );
	parser.GetAttrNode(tempChildNode,"Freq",Freq);

	TaskScheduler = new Scheduler();
	
	TaskScheduler->SetRunTime(TimeUtil::GetCurDateTime());
	

	m_lastLevel = "0.00";
}

QualityRealtimeQueryTask::~QualityRealtimeQueryTask()
{

}

void QualityRealtimeQueryTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]实时指标任务执行 !\n",DeviceID));
	bRun = true;
	SetRunning(); 
	
	strDeviceXML=TranslateXMLForDevice::TranslateQualityRealTimeQuery(strStandardXML);

	string rtnxml;
	XmlParser parse(strDeviceXML.c_str());
	pXMLNODE rootNode = parse.GetNodeFromPath("Msg");
	string DVB_Type = "";
	parse.GetAttrNode(rootNode,"DVBType",DVB_Type);

	if(RetValue != SET_SUCCESS)
	{			
		SetFinised();//任务停止
		string msg = string("通道[") + StrUtil::Int2Str(DeviceID) + string("]实时指标任务执行失败，有更高优先级的任务占用通道");
		SYSMSGSENDER::instance()->SendMsg(msg,DVBType,VS_MSG_SYSALARM);
		return;
	}
	
	while (bRun == true && IsRunning())
	{
		g_realqulity = true;
		rtnxml="";
		if(Periodicity=="Cycle" && REALTIMEQUALITYSENDER::instance()->HasClient(strMsgID)==false)//没有连接用户，任务退出
			break;

		bool rtn = DEVICEACCESSMGR::instance()->SendTaskMsg(DeviceID,strDeviceXML,rtnxml);
		if(rtn == false || rtnxml.empty())
		{
			if(Periodicity=="Once")
			{
				if(strProtocol=="HTTP")
				{
					RetValue = RUN_FAILED;
					SendXML(TranslateUpXML::TranslateQualityRealtimeQuery(this,rtnxml));
				}
				break;
			}
			OSFunction::Sleep(0,500);
			continue;
		}
		else
		{
			std::string value;
			XmlParser parser(rtnxml.c_str());
			pXMLNODE returtnNode = parser.GetNodeFromPath("Msg/Return");
			parser.GetAttrNode(returtnNode,"Value",value);

			if (value != "0")
			{
				if(Periodicity=="Once")
				{
					if(strProtocol=="HTTP")
					{
						RetValue = RUN_FAILED;
						SendXML(TranslateUpXML::TranslateQualityRealtimeQuery(this,rtnxml));
					}

					break;
				}
				continue;
			}
		}
		
		if(DVBType==DVBC||DVBType==CTTB||DVBType==DVBS)//获取数字电视场强卡场强值
		{
			string freq;
			XmlParser rtnparser(rtnxml.c_str());
			pXMLNODE returtnNode = rtnparser.GetNodeFromPath("Msg/Return");	
			pXMLNODE ReportNode = rtnparser.GetNodeFromPath("Msg/QualityQueryReport");
			rtnparser.GetAttrNode(ReportNode,"Freq",freq);
			pXMLNODE paramNode = rtnparser.GetNodeFromPath("Msg/QualityQueryReport/QualityParam");
			pXMLNODELIST paramList = rtnparser.GetNodeList(paramNode);
			int count=rtnparser.GetChildCount(paramNode);
			for(int k=0;k<count;k++)
			{
				string type,desc,val;
				pXMLNODE paramnode = rtnparser.GetNextNode(paramList);	
				rtnparser.GetAttrNode(paramnode,"Type",type);
				rtnparser.GetAttrNode(paramnode,"Desc",desc);
				rtnparser.GetAttrNode(paramnode,"Value",val);
				if(type=="1")
				{
					string ip;
					int port;
					PROPMANAGER::instance()->GetQualityCardInfo(DVBType,ip,port);
					int Ifreq=StrUtil::Str2Int(freq);
					FetchQualityLevel * FetchLevel=new FetchQualityLevel(DVBType,ip,port);
					int level=FetchLevel->GetLevelFromCard(Ifreq);
					if(level>0)
					{   
						float f_level=(float)level/100.0;
						rtnparser.SetAttrNode("Value",StrUtil::Float2Str(f_level),paramnode);
						rtnparser.SaveToString(rtnxml);
					}
					delete FetchLevel;
					break;
				}
				ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]指标:%s\t%s\t%s\t\n",DeviceID,type,desc,val));
			}
		}
		//std::vector<sCheckParam> alarmVec;//报警信息处理 hjw 2012-7-30 real qulity not alarm check
		//std::vector<sCheckParam>::iterator itr;
		//ReadyForAlarm(rtnxml,alarmVec);
		//for(itr=alarmVec.begin();itr!=alarmVec.end();itr++)
		//{
		//	ALARMMGR::instance()->CheckAlarm(*itr,true);
		//}
		if(strProtocol=="HTTP")
		{
			RetValue = RUN_SUCCESS;
			SendXML(TranslateUpXML::TranslateQualityRealtimeQuery(this,rtnxml));
		}
		else if(strProtocol=="TCP")
		{
			RetValue = RUN_SUCCESS;
			
			unsigned char TcpData[1024];
			memset(TcpData,0,1024);

			int len = 0;
			switch(DVBType)
			{
			case CTTB:
				{
					len=CreateTcpDataForCTTB(rtnxml,&TcpData[10],1024-10);
					break;
				}
			case ATV:
			case CTV:
				{
					len=CreateTcpDataForATV(rtnxml,&TcpData[10],1024-10 );
					break;
				}
			case RADIO:
			case AM:
				{
					len=CreateTcpDataForRadio(rtnxml,&TcpData[10],1024-10 );
					break;
				}
			}
			
			int msgid=StrUtil::Str2Int(GetMsgID());

			memcpy(&TcpData[0],&msgid,sizeof(msgid));
			TcpData[4] = 0;
			TcpData[5] = 4;
			

			memcpy(&TcpData[6],&len,sizeof(len));
			len+=10;
			ACE_Message_Block *MBTcpData = new ACE_Message_Block(len);

			memcpy(MBTcpData->wr_ptr(),TcpData,len);
			MBTcpData->wr_ptr(len);

			//指标数据放入到指标发送线程的队列
			ACE_Time_Value OutTime(ACE_OS::time(0)+1);
			if (-1 == REALTIMEQUALITYSENDER::instance()->putq(MBTcpData,&OutTime))
			{
				MBTcpData->release();
			}
		}
		
		if(Periodicity=="Once")
			break;
		if(PROPMANAGER::instance()->GetIsSpectrumFlag()=="0")
			OSFunction::Sleep(0,10);
		else
			OSFunction::Sleep(3);
	}
	g_realqulity = false;

	bRun = false;

	if (TaskScheduler != NULL)
		TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));

	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]实时指标任务停止 !\n",DeviceID));
}
string QualityRealtimeQueryTask::GetTaskName()
{
	return std::string("实时指标任务");
}
std::string QualityRealtimeQueryTask::GetObjectName()
{
	return std::string("QualityRealtimeQueryTask");
}

int QualityRealtimeQueryTask::CreateTcpDataForCTTB(std::string  strRtnXML,unsigned char* TcpData,int size_n)
{
	char buffer[54] = {};
	memset(buffer,0,54*sizeof(char));
	int index = 0;

	memcpy(&buffer[0],"TRQU",4*sizeof(char));
	index = index + 4*sizeof(char);

	time_t t64 = time(0);
	memcpy(&buffer[index],&t64,8*sizeof(char));
	index = index + 8*sizeof(char);

	int intFreq = atoi(Freq.c_str());
	memcpy(&buffer[index],&intFreq,4*sizeof(char));
	index = index + 4*sizeof(char);

	string Type,Value;
	XmlParser rtnparser;
	rtnparser.Set_xml(strRtnXML);

	pXMLNODE queryParamNode = rtnparser.GetNodeFromPath("Msg/QualityQueryReport/QualityParam");
	pXMLNODELIST queryIndexList = rtnparser.GetNodeList( queryParamNode );
	int count = rtnparser.GetChildCount( queryParamNode );

	memcpy(&buffer[index],&count,2*sizeof(char));
	index = index + 2*sizeof(char);
	for (int i = 0;i<count;i++)
	{
		pXMLNODE indexNode = rtnparser.GetNextNode( queryIndexList);
		
		//设置QualityIndex 属性
		rtnparser.GetAttrNode( indexNode,"Type",Type );
		rtnparser.GetAttrNode( indexNode,"Value",Value );
		float FValue = StrUtil::Str2Float(Value);
		int IntValue = 	int(FValue*1000);
		string add;
		if(index>size_n-5)
			break;
		switch(atoi(Type.c_str()))
		{
		case 1:
			QUALITYCOMPENSATIONMGR::instance()->GetQualityCompensation(this->GetDVBType(),StrUtil::Int2Str(DeviceID),Type,add);
			IntValue+=StrUtil::Str2Int(add);
			buffer[index] = 1;
			index = index + 1*sizeof(char);
			memcpy(&buffer[index],&IntValue,4*sizeof(char));
			index = index + 4*sizeof(char);
			break;
		case 2:
			buffer[index] = 3;		//特殊处理地面数字的指标类型
			index = index + 1*sizeof(char);
			memcpy(&buffer[index],&IntValue,4*sizeof(char));
			index = index + 4*sizeof(char);
			break;
		case 3:
			buffer[index] = 2;		//特殊处理地面数字的指标类型
			index = index + 1*sizeof(char);
			memcpy(&buffer[index],&IntValue,4*sizeof(char));
			index = index + 4*sizeof(char);
			break;
		case 4:
			buffer[index] = 4;
			index = index + 1*sizeof(char);
			memcpy(&buffer[index],&IntValue,4*sizeof(char));
			index = index + 4*sizeof(char);
			break;
		case 5:	
			buffer[index] = 5;
			index = index + 1*sizeof(char);
			memcpy(&buffer[index],&IntValue,4*sizeof(char));
			index = index + 4*sizeof(char);
			break;
		case 6:
			buffer[index] = 6;
			index = index + 1*sizeof(char);
			memcpy(&buffer[index],&IntValue,4*sizeof(char));
			index = index + 4*sizeof(char);
			break;
		}
	}
	int rtnlen  = sizeof(buffer);
	memcpy(TcpData,buffer,rtnlen);

	rtnlen -= 6;

    return rtnlen;
}

int QualityRealtimeQueryTask::CreateTcpDataForATV(std::string strRtnXML,unsigned char* TcpData,int size_n )
{
	vector<QualityDesc> QualityParamVec;
	float mFreq=StrUtil::Str2Float(Freq);
	
	bool retVal = false;
	int TcpDataLen = 0;
	int len = 0,templen = 0;
	templen = len;
	unsigned char ch[128];
	memset(ch,0,128*sizeof(char));

	XmlParser parser;
	parser.Set_xml(strRtnXML);

	pXMLNODE rootNode=parser.GetRootNode();
	pXMLNODE SrcparamNode = parser.GetNodeFromPath("Msg/QualityQueryReport/QualityParam");
	pXMLNODELIST paramList = parser.GetNodeList(SrcparamNode);
	int count = paramList->Size();
	for(int k=0;k<count;k++)
	{
		std::string Type,Desc,Value;
		QualityDesc TempDesc;
		pXMLNODE childNode = parser.GetNextNode(paramList);
		parser.GetAttrNode(childNode,"Type",Type);
		parser.GetAttrNode(childNode,"Desc",Desc);
		parser.GetAttrNode(childNode,"Value",Value);
		if (Type == "1")
		{
			if (Value != "0.00")
			{
				m_lastLevel = Value;
			}
			else
			{
				Value = m_lastLevel;
			}
		}

		TempDesc.Desc=Desc;
		TempDesc.Type=Type;
		TempDesc.Value=Value;
		QualityParamVec.push_back(TempDesc);
	}
	try
	{
		//保存data_type_tag	接口类型。电视实时指标测量上报是”TRQU”。
		memcpy(&ch[templen],"TRQU",4*sizeof(char));
		templen = templen + 4*sizeof(char);

		//保存report_time	上报时间, 从1970年1月1日0时0分0秒到该时间点所经过的秒
		time_t t64 = time(0);
		memcpy(&ch[templen],&t64,8*sizeof(char));
		templen = templen + 8*sizeof(char);
		float ff = 1000*mFreq;

		//保存freq	测量频率。单位为kHz。
		int ff1 = (int)ff;
		memcpy(&ch[templen],&ff1,4*sizeof(char));
		templen = templen + 4*sizeof(char);

		//保存data_length	数据长度
		short datalen = (short)QualityParamVec.size();
		memcpy(&ch[templen],&datalen,2*sizeof(char));
		templen = templen + 2*sizeof(char);

		BYTE nCount =(BYTE)QualityParamVec.size();

		int	ff2[7] = {0,0,0,0,0,0,0};
		for(BYTE i = 0; i < nCount; i++)
		{
			if(templen>size_n-5)
				break;
			switch(StrUtil::Str2Int(QualityParamVec[i].Type))
			{
			case 1:
				ff = StrUtil::Str2Float(QualityParamVec[i].Value);
				ff2[0] =(int)ff;
				//cout<<"图像电平:"<<ff2[0]<<endl;
				ch[templen] = 1;
				templen = templen + 1*sizeof(char);
				memcpy(&ch[templen],&ff2[0],4*sizeof(char));
				templen = templen + 4*sizeof(char);
				break;
			case 2:
				ff  = StrUtil::Str2Float(QualityParamVec[i].Value);
				ff2[1] =(int)ff;
				//cout<<"伴音电平:"<<ff2[1]<<endl;
				ch[templen] = 2;
				templen = templen + 1*sizeof(char);
				memcpy(&ch[templen],&ff2[1],4*sizeof(char));
				templen = templen + 4*sizeof(char);
				break;
			case 3:

				ff  = StrUtil::Str2Float(QualityParamVec[i].Value);
				ff2[2] =(int)ff;
				//cout<<"图像伴音电平差:"<<ff2[2]<<endl;
				ch[templen] = 3;
				templen = templen + 1*sizeof(char);
				memcpy(&ch[templen],&ff2[2],4*sizeof(char));
				templen = templen + 4*sizeof(char);
				break;
			case 4:
				ff  = StrUtil::Str2Float(QualityParamVec[i].Value);
				ff2[3] =(int)ff;
				//cout<<"载噪比:"<<ff2[3]<<endl;
				ch[templen] = 4;
				templen = templen + 1*sizeof(char);
				memcpy(&ch[templen],&ff2[3],4*sizeof(char));
				templen = templen + 4*sizeof(char);
				break;
			case 5:
				ff  = StrUtil::Str2Float(QualityParamVec[i].Value);
				ff2[4] =(int)ff;
				//cout<<"载频频偏:"<<ff2[4]<<endl;
				ch[templen] = 5;
				templen = templen + 1*sizeof(char);
				memcpy(&ch[templen],&ff2[4],4*sizeof(char));
				templen = templen + 4*sizeof(char);
				break;
			case 6:
				ff  = StrUtil::Str2Float(QualityParamVec[i].Value);
				ff2[5] =(int)ff;
				//cout<<"斜率:"<<ff1<<endl;
				ch[templen] = 6;
				templen = templen + 1*sizeof(char);
				memcpy(&ch[templen],&ff2[5],4*sizeof(char));
				templen = templen + 4*sizeof(char);
				break;
			case 7:
				//mQualityParamVec[6].fReturnVal = 0.00;    //误码率
				break;
			default:
				break;
			}
		}

		memcpy(TcpData,ch,templen);
		TcpDataLen = templen;
		retVal = true;

	}
	catch(...)
	{
		string msg = string("通道[") + StrUtil::Int2Str(DeviceID) + string("]实时指标测量出错");
		SYSMSGSENDER::instance()->SendMsg(msg,DVBType,VS_MSG_SYSALARM);
		return -1;
	}
	return TcpDataLen;
}


int QualityRealtimeQueryTask::CreateTcpDataForRadio(std::string strRtnXML,unsigned char* TcpData,int size_n )
{
	vector<QualityDesc> QualityParamVec;
	float mFreq=StrUtil::Str2Float(Freq);

	bool retVal = false;
	int TcpDataLen = 0;
	int len = 0,templen = 0;
	templen = len;
	unsigned char ch[128];
	memset(ch,0,128*sizeof(char));

	XmlParser parser;
	parser.Set_xml(strRtnXML);

	pXMLNODE rootNode=parser.GetRootNode();
	pXMLNODE SrcparamNode = parser.GetNodeFromPath("Msg/QualityQueryReport/QualityParam");
	pXMLNODELIST paramList = parser.GetNodeList(SrcparamNode);
	int count = paramList->Size();
	for(int k=0;k<count;k++)
	{
		std::string Type,Desc,Value;
		QualityDesc TempDesc;
		pXMLNODE childNode = parser.GetNextNode(paramList);
		parser.GetAttrNode(childNode,"Type",Type);
		parser.GetAttrNode(childNode,"Desc",Desc);
		parser.GetAttrNode(childNode,"Value",Value);
		if (Type == "1")
		{
			if (Value != "0.00")
			{
				m_lastLevel = Value;
			}
			else
			{
				Value = m_lastLevel;
			}
		}
		TempDesc.Desc=Desc;
		TempDesc.Type=Type;
		TempDesc.Value=Value;
		QualityParamVec.push_back(TempDesc);
	}
	try
	{
		//保存data_type_tag	接口类型。电视实时指标测量上报是”TRQU”。
		memcpy(&ch[templen],"TRQU",4*sizeof(char));
		templen = templen + 4*sizeof(char);

		//保存report_time	上报时间, 从1970年1月1日0时0分0秒到该时间点所经过的秒
		time_t t64 = time(0);
		memcpy(&ch[templen],&t64,8*sizeof(char));
		templen = templen + 8*sizeof(char);
		float ff = 1000*mFreq;

		//保存freq	测量频率。单位为kHz。
		int ff1 = (int)ff;
		memcpy(&ch[templen],&ff1,4*sizeof(char));
		templen = templen + 4*sizeof(char);

		//保存data_length	数据长度
		short datalen = (short)QualityParamVec.size();
		memcpy(&ch[templen],&datalen,2*sizeof(char));
		templen = templen + 2*sizeof(char);

		BYTE nCount =(BYTE)QualityParamVec.size();

		int	ff2[7] = {0,0,0,0,0,0,0};
		for(BYTE i = 0; i < nCount; i++)
		{
			if(templen>size_n-5)
				break;
			switch(StrUtil::Str2Int(QualityParamVec[i].Type))
			{
			case 1:                                     //音频信号场强
				{
					ff = StrUtil::Str2Float(QualityParamVec[i].Value);
					string add;
					QUALITYCOMPENSATIONMGR::instance()->GetQualityCompensation(this->GetDVBType(),StrUtil::Int2Str(DeviceID),string("1"),add);
					ff+=StrUtil::Str2Int(add);
					ff2[0] =(int)ff;
					ch[templen] = 1;
					templen = templen + 1*sizeof(char);
					memcpy(&ch[templen],&ff2[0],4*sizeof(char));
					templen = templen + 4*sizeof(char);
					break;
				}
			case 2:                                       //调制度
				{
					ff  = StrUtil::Str2Float(QualityParamVec[i].Value);
					string add;
					QUALITYCOMPENSATIONMGR::instance()->GetQualityCompensation(this->GetDVBType(),StrUtil::Int2Str(DeviceID),string("2"),add);
					ff+=StrUtil::Str2Int(add);
					ff2[1] =(int)ff;
					//cout<<"伴音电平:"<<ff2[1]<<endl;
					ch[templen] = 2;
					templen = templen + 1*sizeof(char);
					memcpy(&ch[templen],&ff2[1],4*sizeof(char));
					templen = templen + 4*sizeof(char);
					break;
				}
			case 3:                                       //频偏

				ff  = StrUtil::Str2Float(QualityParamVec[i].Value);
				ff2[2] =(int)ff;
				//cout<<"图像伴音电平差:"<<ff2[2]<<endl;
				ch[templen] = 3;
				templen = templen + 1*sizeof(char);
				memcpy(&ch[templen],&ff2[2],4*sizeof(char));
				templen = templen + 4*sizeof(char);
				break;
			case 4:                                        //音频信号主谐波频率 信号载波频率 (音频信号主谐波频率?)
				ff  = StrUtil::Str2Float(QualityParamVec[i].Value);
				ff2[3] =(int)ff;
				//cout<<"载噪比:"<<ff2[3]<<endl;
				ch[templen] = 4;
				templen = templen + 1*sizeof(char);
				memcpy(&ch[templen],&ff2[3],4*sizeof(char));
				templen = templen + 4*sizeof(char);
				break;
			case 5:                                       //谐波失真
				ff  = StrUtil::Str2Float(QualityParamVec[i].Value);
				ff2[4] =(int)ff;
				//cout<<"载频频偏:"<<ff2[4]<<endl;
				ch[templen] = 5;
				templen = templen + 1*sizeof(char);
				memcpy(&ch[templen],&ff2[4],4*sizeof(char));
				templen = templen + 4*sizeof(char);
				break;
			case 6:                                        //信噪比
				ff  = StrUtil::Str2Float(QualityParamVec[i].Value);
				ff2[5] =(int)ff;
				ch[templen] = 6;
				templen = templen + 1*sizeof(char);
				memcpy(&ch[templen],&ff2[5],4*sizeof(char));
				templen = templen + 4*sizeof(char);
				break;
			case 7:
				//mQualityParamVec[6].fReturnVal = 0.00;    //误码率
				break;
			default:
				break;
			}
		}

		memcpy(TcpData,ch,templen);
		TcpDataLen = templen;
		retVal = true;

	}
	catch(...)
	{
		string msg = string("通道[") + StrUtil::Int2Str(DeviceID) + string("]实时指标测量出错");
		SYSMSGSENDER::instance()->SendMsg(msg,DVBType,VS_MSG_SYSALARM);
		return -1;
	}
	return TcpDataLen;
}

bool QualityRealtimeQueryTask::ReadyForAlarm(std::string strXML,std::vector<sCheckParam>& alarmVec)
{
	//声明解析类的对象
	XmlParser devParser;
	devParser.Set_xml(strXML);
	string freq,STD,SymbolRate,type,desc,val;
	pXMLNODE devRootNode= devParser.GetRootNode();

	pXMLNODE ReportNode = devParser.GetNodeFromPath("Msg/QualityQueryReport");
	devParser.GetAttrNode(ReportNode,"Freq",freq);
	devParser.GetAttrNode(ReportNode,"STD",STD);
	devParser.GetAttrNode(ReportNode,"SymbolRate",SymbolRate);

	pXMLNODE paramNode = devParser.GetNodeFromPath("Msg/QualityQueryReport/QualityParam");

	pXMLNODELIST paramList = devParser.GetNodeList(paramNode);

	int count=devParser.GetChildCount(paramNode);

	for(int k=0;k<count;k++)
	{
		
		string add;
		pXMLNODE paramnode = devParser.GetNextNode(paramList);
	
		devParser.GetAttrNode(paramnode,"Type",type);
		devParser.GetAttrNode(paramnode,"Desc",desc);
		devParser.GetAttrNode(paramnode,"Value",val);
		int logindex=0;
		PROPMANAGER::instance()->GetDeviceLogIndex(this->GetDeviceID(),logindex);
		QUALITYCOMPENSATIONMGR::instance()->GetQualityCompensation(this->GetDVBType(),StrUtil::Int2Str(logindex),type,add);
		int val_int = StrUtil::Str2Int(val)+StrUtil::Str2Int(add);
		sCheckParam param;
		param.DVBType = this->GetDVBType();
		param.AlarmType = ALARM_FREQ;
		param.TypeID = type;
		param.TypeDesc = desc;
		param.STD = STD;
		param.Freq = freq;
		param.SymbolRate = SymbolRate;
		param.ChannelID = this->GetChannelID();
		if(DVBType==ATV||DVBType==RADIO||DVBType==AM ||DVBType==CTV)
		{
			param.TypedValue = StrUtil::Float2Str((float)val_int);
		}
		else if(DVBType==DVBC||DVBType==CTTB||DVBType==DVBS)
		{
			param.TypedValue = StrUtil::Float2Str((float)val_int);
		}
		param.DeviceID = StrUtil::Int2Str(DeviceID);
		param.CheckTime=time(0);

		alarmVec.push_back(param);
	}
	return true;
}
