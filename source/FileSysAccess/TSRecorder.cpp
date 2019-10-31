///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����TSRecorder.cpp
// �����ߣ�lichunfeng
// ����ʱ�䣺2009-06-04
// ��������������¼���ļ���
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
	DeviceID = deviceid;//ͨ����
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
	//�����Ƶ������Ϣ
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

	//���Ը�ֵ��mRecordInfo�ṹ��
	if (TaskID.length()==0)
	{
		TaskID="0";
	}
	mRecordInfo.dvbtype = OSFunction::GetEnumDVBType(downType);	//�������

	CHANNELINFOMGR::instance()->GetChannelID(mRecordInfo.dvbtype,OrgNetID,TsID,ServiceID,VideoPID,AudioPID,Code,mRecordInfo.channelID);//���Ƶ��id
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
		std::string storetype = PROPMANAGER::instance()->GetStoreType();	//��ȡ¼��洢��ʽ����ɢ ���� ˳��
		if (storetype == "0")
			DISKOPERATION::instance()->CheckDiskIsFullForRecord(path);	//˳��deviceid��Ĭ��ֵ����
		else if (storetype == "1")
			DISKOPERATION::instance()->CheckDiskIsFullForRecord(path, DeviceID);  //��ɢ
		if (path.length()==0 || path=="")
		{
			ACE_DEBUG ((LM_DEBUG,"(%T | %t)��ȡ¼��洢·��ʧ��!\n"));
			OSFunction::Sleep(0,50);
			continue;
		}
		break;
	}	

	//����ļ�����ͨ����_����
	std::string datetime=TimeUtil::DateTimeToString((time_t)(time(0)-2)); //���������2����
	datetime=datetime.substr(4, datetime.size()- 4);
	
	m_FreqRecordDir = path+Freq + "/";

	if(!mPFile.IfDirExists(m_FreqRecordDir.c_str()))
	{
		if(mPFile.CreateDir(m_FreqRecordDir.c_str()))
		{
			ACE_DEBUG ((LM_DEBUG,"(%T | %t)����¼��洢·���ɹ�!\n"));
		}else{
			ACE_DEBUG ((LM_DEBUG,"(%T | %t)����¼��洢·��ʧ��!\n"));
		}
	}
	
	
	mFileName=m_FreqRecordDir + StrUtil::Int2Str(DeviceID) + string("_")  + TaskID + string("_") + datetime;
	
	mFileName+=".ts";
	
	//�����ļ�
	mStartTime=TimeUtil::DateTimeToStr((time(0)-2));//TimeUtil::GetCurDateTime();
	ret=mPFile.CreateNewFile(mFileName.c_str());
	mCurTime = time(0)-2;												//���¿�ʼʱ��
	Save2DB();														//¼���ļ���¼�������ݿ�
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)����[%s]!\n",mFileName.c_str()));

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
			CloseFile();//�ر��ļ�
		}
		return false;
	}
	bool ret=false;
	if (!mIsCreate)//�ļ��Ƿ񴴽�
	{
		CreateRecFile();
	}
	/**
		¼���ļ���2�룬�˴���Ҫ��2��
	*/
	if (time(0)-mCurTime-2>StrUtil::Str2Int(PROPMANAGER::instance()->GetRecordPeriod())*60/*��λ���룻�������ļ��л��*/)
	{
		DBMANAGER::instance()->UpdateFileEndTime(mFileName,TimeUtil::DateTimeToStr(time(0)-2));	
		CloseFile();//�ر��ļ�
		CreateRecFile();//�������ļ�
	}
	if (mIsCreate)
	{
		int numWrite=mPFile.WriteBuf((void*)buf,(int)num,true);//����д���ļ�
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
	
	if (mRecordInfo.expiredays.length()==0)//����·���¼������û�й��������Ͷ�ȡ�����ļ��еĹ�������
	{
		mRecordInfo.expiredays=PROPMANAGER::instance()->GetRecordExpiredays(mRecordInfo.dvbtype);//�������ļ��л�ù�������
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
	if (!mIsCreate)//�ļ��Ƿ񴴽�
	{
		CreateRecFile();
	}
	if (mIsCreate)
	{
		int numWrite=mPFile.WriteBuf((void*)buf,(int)num,true);//����д���ļ�
		if (numWrite>0)
			ret=true;
	}
	return ret;
}