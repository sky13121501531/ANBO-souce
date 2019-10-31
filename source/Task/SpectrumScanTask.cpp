
#include "SpectrumScanTask.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "Scheduler.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "../DeviceAccess/TCPDeviceAccess.h"
#include "../DBAccess/DBManager.h"
#include <vector>
#include <iostream>
#include "ace/Task.h"
#include "../Foundation/XmlParser.h"
#include "./TranslateXMLForDevice.h"
#include "../Foundation/StrUtil.h"
#include "../Communications/QualitySender.h"
#include "../Communications/SysMsgSender.h"
#include "ace/Synch.h"
#include "ace/OS.h"
using namespace std;
extern bool g_realspec;

SpectrumScanTask::SpectrumScanTask() : DeviceRelatedTask()
{

}
SpectrumScanTask::SpectrumScanTask(std::string strXML) : DeviceRelatedTask(strXML)
{
	mStartFreq = 0.00f;
	mEndFreq = 0.00f;
	mStepFreq = 0.00f;
	mNowStartFreq = 0.00f;
	mNowEndFreq = 0.00f;
	mNowFreq = 0.00f;
	mVbw = 0.00f;
	mRbw = 0.00f;
	mReferDbu = 0.00f;
	string templen("0");
	XmlParser parser(strXML.c_str());
	pXMLNODE rootNode=parser.GetRootNode();
	pXMLNODE SpectrumNode=parser.GetNodeFirstChild(rootNode);
	parser.GetAttrNode(SpectrumNode,"StartFreq",mStartFreq);
	parser.GetAttrNode(SpectrumNode,"EndFreq",mEndFreq);
	parser.GetAttrNode(SpectrumNode,"StepFreq",mStepFreq);
	parser.GetAttrNode(SpectrumNode,"Steps",templen);
	parser.GetAttrNode(SpectrumNode,"TaskType",mTaskType);
	parser.GetAttrNode(SpectrumNode,"VBW",mVbw);
	parser.GetAttrNode(SpectrumNode,"RBW",mRbw);
	parser.GetAttrNode(SpectrumNode,"ReferDbu",mReferDbu);

	if(mStepFreq<=0)
	{
		mTaskType = "1";
		mStepFreq = 8;
	}
	mIntevalLen = (int)StrUtil::Str2Int(templen);
	if (mEndFreq < mStartFreq)
	{
		float temp = mEndFreq;
		mEndFreq = mStartFreq;
		mStartFreq = temp;
	}
	mNowStartFreq = mStartFreq;
	mNowEndFreq = mNowStartFreq + mIntevalLen*mStepFreq;
	if(mNowEndFreq > mEndFreq)
	{
		mNowEndFreq = mEndFreq;
	}
	mNowFreq = mNowStartFreq;

	TaskScheduler = new Scheduler();
	TaskScheduler->SetRunTime(TimeUtil::GetCurDateTime());
}

SpectrumScanTask::~SpectrumScanTask()
{

}

void SpectrumScanTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]实时频谱扫描任务执行 !\n",DeviceID));
	bRun = true;
	SetRunning();
	//判断频谱扫描任务是否设置成功
	strDeviceXML=TranslateXMLForDevice::TranslateSpectrumScan(strStandardXML);

	string rtnxml;
	if(RetValue != SET_SUCCESS)
	{			
		SetFinised();//任务停止
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)实时频谱扫描任务执行失败，有更高优先级的任务占用通道 !\n"));
		return;
	}
	g_realspec = true;
	bool rtn = DEVICEACCESSMGR::instance()->SendTaskMsg(DeviceID,strDeviceXML,rtnxml);
	if(rtnxml!="")
	{
		// SYSMSGSENDER::instance()->SendMsg(rtnxml,UNKNOWN,VS_MSG_SYSALARM);
		const int size_n=1024*100;
		unsigned char TcpData[size_n];
		memset(TcpData,0,size_n);
	
		int len=0;
		if(DVBType==RADIO || DVBType==AM)
		{
			len=CreateRADIOTcpData(rtnxml,&TcpData[10],size_n-10);
			//cout<<"\n这是发送的频谱数据<<<<<<<<<<<<<<<"<<len<<":"<<rtnxml<<"<<<<<<<<<<<<<<<\n";
		//将结果发送到前端系统
			OSFunction::Sleep(3); //等待发送结束
		}
		else if(DVBType==ATV||DVBType==CTV)
		{
			len=CreateTVTcpData(rtnxml,&TcpData[10],size_n-10);
		}
		int msgid=StrUtil::Str2Int(GetMsgID());

		memcpy(&TcpData[0],&msgid,sizeof(msgid));
		TcpData[4] = 0;
		TcpData[5] = 3;

		memcpy(&TcpData[6],&len,sizeof(len));

		len += 10;
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
	g_realspec = false;
	OSFunction::Sleep(0,500);
	
	bRun = false;
	if (TaskScheduler != NULL)
		TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));

	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]实时频谱扫描任务停止 !\n",DeviceID));
}

int SpectrumScanTask::CreateRADIOTcpData(string RtnXML,unsigned char* TcpData,int size_n)
{
	int TcpDataLen;
	bool retVal = false;
	
	try
	{
		XmlParser parser(RtnXML.c_str());
		pXMLNODE ParamNode =parser.GetNodeFromPath("Msg/SpectrumQueryReport/SpectrumParam");
		pXMLNODELIST NodeList = parser.GetNodeList(ParamNode);

		TcpDataLen = 0;
		
		int len = 0,templen = 0;
		templen = len;
		memcpy(&TcpData[templen],"TRSU",4*sizeof(BYTE));
		time_t t64 = time(0);
		templen = templen + 4*sizeof(BYTE);
		memcpy(&TcpData[templen],&t64,8*sizeof(BYTE));
		templen = templen + 8*sizeof(BYTE);

		short datalen = (short)NodeList->Size();
		memcpy(&TcpData[templen],&datalen,sizeof(datalen));
		templen = templen + 2*sizeof(BYTE);

		for(int i=0;i<NodeList->Size();i++)
		{
			SpectrumInfo Temp;
			string freq,level;
			pXMLNODE IndexNode = parser.GetNextNode(NodeList);
			parser.GetAttrNode(IndexNode,"Freq",freq);
			parser.GetAttrNode(IndexNode,"Value",level);
			Temp.freq=StrUtil::Str2Float(freq);
			Temp.level=StrUtil::Str2Float(level);
			//UINT TempFreq = (UINT)(Temp.freq);
			UINT TempFreq = (UINT)(Temp.freq)+50;
			TempFreq=(TempFreq/100)*100;
			UINT TempValue = (UINT)(1000*(Temp.level));
			if(templen>size_n-8)
				break;
			memcpy(&TcpData[templen],&TempFreq,sizeof(int));
			templen =templen + 4*sizeof(BYTE);
			memcpy(&TcpData[templen],&TempValue,sizeof(int));
			templen =templen + 4*sizeof(BYTE);
		}
		TcpDataLen = templen;
		retVal = true;
	}
	catch(...)
	{
		string msg = string("通道[") + StrUtil::Int2Str(DeviceID) + string("]实时频谱测量:取测量结果时时出错");
		SYSMSGSENDER::instance()->SendMsg(msg,DVBType,VS_MSG_SYSALARM);
		retVal = false;
	}
	return TcpDataLen;
}

int SpectrumScanTask::CreateTVTcpData(string RtnXML,unsigned char* TcpData,int size_n)
{
	int TcpDataLen;
	bool retVal = false;

	try
	{
		XmlParser parser(RtnXML.c_str());
		pXMLNODE ParamNode =parser.GetNodeFromPath("Msg/SpectrumQueryReport/SpectrumParam");
		pXMLNODELIST NodeList = parser.GetNodeList(ParamNode);

		TcpDataLen = 0;

		int len = 0,templen = 0;
		templen = len;
		memcpy(&TcpData[templen],"TRSU",4*sizeof(BYTE));
		time_t t64 = time(0);
		templen = templen + 4*sizeof(BYTE);
		memcpy(&TcpData[templen],&t64,8*sizeof(BYTE));
		templen = templen + 8*sizeof(BYTE);

		short datalen = (short)NodeList->Size();
		memcpy(&TcpData[templen],&datalen,sizeof(datalen));
		templen = templen + 2*sizeof(BYTE);

		for(int i=0;i<NodeList->Size();i++)
		{
			SpectrumInfo Temp;
			string freq,level;
			pXMLNODE IndexNode = parser.GetNextNode(NodeList);
			parser.GetAttrNode(IndexNode,"Freq",freq);
			parser.GetAttrNode(IndexNode,"Value",level);
			Temp.freq=StrUtil::Str2Float(freq);
			Temp.level=StrUtil::Str2Float(level);
			UINT TempFreq = (UINT)(Temp.freq);		
			UINT TempValue = (UINT)(1000*(Temp.level));
			if(templen>size_n-8)
				break;
			memcpy(&TcpData[templen],&TempFreq,sizeof(int));
			templen =templen + 4*sizeof(BYTE);
			memcpy(&TcpData[templen],&TempValue,sizeof(int));
			templen =templen + 4*sizeof(BYTE);
		}
		TcpDataLen = templen;
		retVal = true;
	}
	catch(...)
	{
		string msg = string("通道[") + StrUtil::Int2Str(DeviceID) + string("]实时频谱测量:取测量结果时时出错");
		SYSMSGSENDER::instance()->SendMsg(msg,DVBType,VS_MSG_SYSALARM);
		retVal = false;
	}
	return TcpDataLen;
}
string SpectrumScanTask::GetTaskName()
{
	return std::string("实时频谱扫描任务");
}
std::string SpectrumScanTask::GetObjectName()
{
	return std::string("SpectrumScanTask");
}
