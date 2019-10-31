///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：TSRecorder.cpp
// 创建者：lichunfeng
// 创建时间：2009-06-04
// 内容描述：保存录像文件类
///////////////////////////////////////////////////////////////////////////////////////////
#include "../BusinessProcess/RunPlanInfoMgr.h"
#include "TSRecorder.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/OSFunction.h"
#include "../DBAccess/DBManager.h"
#include "DiskOperation.h"

TSRecorder::TSRecorder(int deviceid,std::string taskxml,std::string taskid)
{
	mIsCreate=false;
	mCurTime = 0;
	TaskXml = taskxml;
	DeviceID = deviceid;//通道号
	TaskID=taskid;

	XmlParser parser( TaskXml.c_str() );
	pXMLNODE rootNode = parser.GetNodeFromPath( "Msg" );
	parser.GetAttrNode( rootNode,"DVBType",downType );
	pXMLNODE recordNode=NULL;
	if(TaskXml.find("AutoRecord")!=string::npos)
	{
		recordNode=parser.GetNodeFromPath("Msg/AutoRecord/Record");
	}
	else if(TaskXml.find("TaskRecord")!=string::npos)
	{
		recordNode=parser.GetNodeFromPath("Msg/TaskRecord/Record");
	}
	else if(TaskXml.find("TSQuery")!=string::npos)
	{
		recordNode=parser.GetNodeFromPath("Msg/TSQuery/TS");
	}
	//获得视频流的信息
	if(recordNode)
	{
		std::string ExpireDays;
		parser.GetAttrNode(recordNode,"TaskID",TaskID);
		parser.GetAttrNode(recordNode,"Freq",Freq);
		parser.GetAttrNode(recordNode,"Code",Code );
		parser.GetAttrNode(recordNode,"OrgNetID",OrgNetID);
		parser.GetAttrNode(recordNode,"TsID",TsID);
		parser.GetAttrNode(recordNode,"ServiceID",ServiceID);
		parser.GetAttrNode(recordNode,"VideoPID",VideoPID);
		parser.GetAttrNode(recordNode,"AudioPID",AudioPID);	
		parser.GetAttrNode(recordNode,"ExpireDays",ExpireDays);
		if (ExpireDays!="")
		{
			mRecordInfo.expiredays = ExpireDays;
		}
	}

	//属性赋值给mRecordInfo结构体
	if (TaskID.length()==0)
	{
		TaskID="0";
	}
	mRecordInfo.dvbtype = OSFunction::GetEnumDVBType(downType);	//监测类型

	CHANNELINFOMGR::instance()->GetChannelID(mRecordInfo.dvbtype,OrgNetID,TsID,ServiceID,VideoPID,AudioPID,Code,mRecordInfo.channelID);//获得频道id
	if(mRecordInfo.channelID=="")
		mRecordInfo.channelID=Freq;
	
	mRecordInfo.taskid=TaskID;
}

TSRecorder::~TSRecorder(void)
{
	DBMANAGER::instance()->UpdateFileEndTime(mFileName,TimeUtil::GetCurDateTime());	
}

bool TSRecorder::CreateRecFile()
{
	bool ret=false;
	std::string path = "";
	std::string m_FreqRecordDir = "";

	while(1)
	{
		std::string storetype = PROPMANAGER::instance()->GetStoreType();	//获取录像存储方式：分散 还是 顺序
		if (storetype == "0")
			DISKOPERATION::instance()->CheckDiskIsFullForRecord(path);	//顺序，deviceid按默认值传递
		else if (storetype == "1")
			DISKOPERATION::instance()->CheckDiskIsFullForRecord(path, DeviceID);  //分散
		if (path.length()==0 || path=="")
		{
			ACE_DEBUG ((LM_DEBUG,"(%T | %t)获取录像存储路径失败!\n"));
			OSFunction::Sleep(0,50);
			continue;
		}
		break;
	}	

	//获得文件名：通道号_日期
	std::string datetime=TimeUtil::DateTimeToString((time_t)(time(0)-2)); //修正缓存包2秒数
	datetime=datetime.substr(4, datetime.size()- 4);
	
	m_FreqRecordDir = path+Freq + "/";

	if(!mPFile.IfDirExists(m_FreqRecordDir.c_str()))
	{
		if(mPFile.CreateDir(m_FreqRecordDir.c_str()))
		{
			ACE_DEBUG ((LM_DEBUG,"(%T | %t)创建录像存储路径成功!\n"));
		}else{
			ACE_DEBUG ((LM_DEBUG,"(%T | %t)创建录像存储路径失败!\n"));
		}
	}
	
	
	mFileName=m_FreqRecordDir + StrUtil::Int2Str(DeviceID) + string("_")  + TaskID + string("_") + datetime;
	
	mFileName+=".ts";
	
	//创建文件
	mStartTime=TimeUtil::DateTimeToStr((time(0)-2));//TimeUtil::GetCurDateTime();
	ret=mPFile.CreateNewFile(mFileName.c_str());
	mCurTime = time(0)-2;												//更新开始时间
	Save2DB();														//录像文件记录存入数据库
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)创建[%s]!\n",mFileName.c_str()));

	mIsCreate=ret;	
	return ret;
}
bool TSRecorder::SaveFile( unsigned char* buf,size_t num )
{
	UpAlarmInfo info;
	PROPMANAGER::instance()->GetUpAlarmInfo(mRecordInfo.dvbtype,info);

	if(info.runplanrecord=="1" && mRecordInfo.taskid=="0" && RUNPLANINFOMGR::instance()->InRunPlan(mRecordInfo.dvbtype,mRecordInfo.channelID)==false)
	{
		if(mIsCreate)
		{
			DBMANAGER::instance()->UpdateFileEndTime(mFileName,TimeUtil::DateTimeToStr(time(0)-2));	
			CloseFile();//关闭文件
		}
		return false;
	}
	bool ret=false;
	if (!mIsCreate)//文件是否创建
	{
		CreateRecFile();
	}
	/**
		录像文件少2秒，此处需要减2秒
	*/
	if (time(0)-mCurTime-2>StrUtil::Str2Int(PROPMANAGER::instance()->GetRecordPeriod())*60/*单位：秒；从配置文件中获得*/)
	{
		DBMANAGER::instance()->UpdateFileEndTime(mFileName,TimeUtil::DateTimeToStr(time(0)-2));	
		CloseFile();//关闭文件
		CreateRecFile();//创建新文件
	}
	if (mIsCreate)
	{
		int numWrite=mPFile.WriteBuf((void*)buf,(int)num,true);//数据写入文件
		if (numWrite>0)
			ret=true;
	}
	return ret;
}

void TSRecorder::CloseFile( void )
{
	mIsCreate=false;
	mPFile.Close();
}

bool TSRecorder::Save2DB()
{
	
	mRecordInfo.filename=mFileName;
	mRecordInfo.starttime=mStartTime;
	
	string endtime=TimeUtil::CalMinutes(mStartTime,StrUtil::Str2Int(PROPMANAGER::instance()->GetRecordPeriod()));
	mRecordInfo.endtime=endtime;
	
	if (mRecordInfo.expiredays.length()==0)//如果下发的录像任务没有过期天数就读取配置文件中的过期天数
	{
		mRecordInfo.expiredays=PROPMANAGER::instance()->GetRecordExpiredays(mRecordInfo.dvbtype);//从配置文件中获得过期天数
	}
	if (mFileName.length()==0 || mStartTime.length()==0 ||
		PROPMANAGER::instance()->GetRecordExpiredays(mRecordInfo.dvbtype).length()==0)
		return false;
	else
	{
		return DBMANAGER::instance()->AddRecord(mRecordInfo);	
	}
	
}

bool TSRecorder::SaveManualRecordFile( unsigned char* buf,size_t num )
{
	bool ret=false;
	if (!mIsCreate)//文件是否创建
	{
		CreateRecFile();
	}
	if (mIsCreate)
	{
		int numWrite=mPFile.WriteBuf((void*)buf,(int)num,true);//数据写入文件
		if (numWrite>0)
			ret=true;
	}
	return ret;
}