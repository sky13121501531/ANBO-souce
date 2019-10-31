
#include "StreamRealtimeQueryTask.h"
#include "Scheduler.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Communications/TsSenderMgr.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "../DeviceAccess/TsFetcherMgr.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/AppLog.h"
#include "ace/Synch.h"
#include "./TranslateXMLForDevice.h"
#include "../FileSysAccess/TSRecorder.h"
#include "../DBAccess/DBManager.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include <vector>
#include <iostream>
extern bool g_realstream;
using namespace std;

const std::string MANURECORD_EXPIREDAYS = "7";

class ManuRecord_Task : public ACE_Task<ACE_MT_SYNCH>
{
public:
	ManuRecord_Task();
	~ManuRecord_Task(){};

};

ManuRecord_Task::ManuRecord_Task()
{
	msg_queue()->high_water_mark(188*10000);
	msg_queue()->low_water_mark(188*10000);
}

StreamRealtimeQueryTask::StreamRealtimeQueryTask() : DeviceRelatedTask()
{

}

StreamRealtimeQueryTask::StreamRealtimeQueryTask(std::string strXML) : DeviceRelatedTask(strXML)
{
	 bManualRecord=false;
	std::string freq,OrgNetID,TsID,ServiceID,VideoPID,AudioPID,Code;
	XmlParser parser(strXML.c_str());
	pXMLNODE queryNode = parser.GetNodeFromPath("Msg/StreamRealtimeQuery/RealtimeQueryTask");
	parser.GetAttrNode(queryNode,"Freq",freq);            //��ñ�׼xml��RealtimeQueryTask�ڵ��Freq����ֵ
	parser.GetAttrNode(queryNode,"OrgNetID",OrgNetID);    //��ñ�׼xml��RealtimeQueryTask�ڵ��OrgNetID����ֵ
	parser.GetAttrNode(queryNode,"TsID",TsID);            //��ñ�׼xml��RealtimeQueryTask�ڵ��TsID����ֵ
	parser.GetAttrNode(queryNode,"ServiceID",ServiceID);  //��ñ�׼xml��RealtimeQueryTask�ڵ��ServiceID����ֵ
	parser.GetAttrNode(queryNode,"VideoPID",VideoPID);  //��ñ�׼xml��RealtimeQueryTask�ڵ��VideoPID����ֵ
	parser.GetAttrNode(queryNode,"AudioPID",AudioPID);  //��ñ�׼xml��RealtimeQueryTask�ڵ��AudioPID����ֵ

	Freq = freq;                                          
	strDeviceXML = TranslateXMLForDevice::TranslateStreamRealTimeXML(strXML);//�Զ����׼XMLת��Ϊ��Ӳ��ͨ�ŵ�xml
	if (ATV==DVBType || RADIO==DVBType || DVBType==AM || DVBType==CTV)
	{
		ChannelID=freq;
	}
	else
	{
        CHANNELINFOMGR::instance()->GetChannelID(DVBType,OrgNetID,TsID,ServiceID,VideoPID,AudioPID,Code,ChannelID);
	}
	
	mRecordFileName = "";	//�ֶ�¼���Ϊ�浽tempfile����  jidushan 110406

    TaskScheduler = new Scheduler();
	if (TaskScheduler != NULL)
		TaskScheduler->SetRunTime(TimeUtil::GetCurDateTime());
}

StreamRealtimeQueryTask::~StreamRealtimeQueryTask()
{

}
void StreamRealtimeQueryTask::Run(void)
{
	g_realstream = true;
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]ʵʱ��Ƶ����ִ�� !\n",DeviceID));

	bRun = true;
	SetRunning();//����״̬����Ϊ��������

	if(RetValue != SET_SUCCESS)	//ʵʱ��Ƶ��������ʧ��
	{			
		SendXML(TranslateUpXML::TranslateStreamRealtimeQuery(this));
		SetFinised();//����ֹͣ
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ʵʱ��Ƶ��������ʧ��!\n"));
		g_realstream = false;
		return;
	}

	TSFETCHERMGR::instance()->StopHistoryTask(DeviceID);			//�����ʷ��Ƶ

	bool FindRecordDevice = false;
	int tempID = -1;
	if(PROPMANAGER::instance()->IsRealTimeFromRecord(DVBType)&&BUSINESSLAYOUTMGR::instance()->GetAutoRecordDeviceIdByChannelID(DVBType,ChannelID,tempID))
	{
		FindRecordDevice = true;
		DeviceID = tempID;			
	}

	RetValue=RUN_SUCCESS;
	
	if (FindRecordDevice)	//����ظ������Զ�¼���ͨ��������ֱ�ӷ����˳�
	{
		SendXML(TranslateUpXML::TranslateStreamRealtimeQuery(this));	//����ת�����xml

		if (TaskScheduler != NULL)
			TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));

		SetFinised();																			//����ֹͣ
		bRun = false;
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]ʵʱ��Ƶ����¼��ͨ����ֹͣ !\n",DeviceID));
		g_realstream = false;
		return;
	}
	else//���û���ҵ�����Ƶ�������¼��ͨ��
	{
		SendXML(TranslateUpXML::TranslateStreamRealtimeQuery(this));	//����ת�����xml
		TSFETCHERMGR::instance()->SetTsDeviceXml(DeviceID,strDeviceXML);
		// OSFunction::Sleep(1); //�ȴ���̨����
		TSFETCHERMGR::instance()->IncreaseTaskNum(DeviceID);
		
	}
	//OSFunction::Sleep(3); //�ȴ���̨����
	//�ֶ�¼��
	ACE_Task<ACE_MT_SYNCH>* Task = new ManuRecord_Task;
	TSFETCHERMGR::instance()->SetRecordTask(DeviceID,Task);

	const int BufLen = 188*100;
	const int BufSize = 188*1000;

	unsigned char* TsBuf = new unsigned char[BufLen];//�������ݻ�����
	memset(TsBuf,0,BufLen);

	int PacketLen = 0;
	bool FirstCheck = true;
	time_t ctime = time(0);
	while(bRun && IsRunning())
	{
	
		//edit by zhaonan2018_4_12
		if (TSSENDERMGR::instance()->HasClient(DeviceID) == false)//û�������û��������˳�
		{
			if (FirstCheck == true)
			{
				OSFunction::Sleep(0,300);
				if(time(0) - ctime <= 30)
				{
					continue;
				}
				else
				{
					FirstCheck = false;
				}
			}
			break;
		}
		
		FirstCheck = false;
		//���ֶ�¼������,���ֶ�¼��������ִ��ʱ������
		if(bManualRecord&&time(0)-TimeUtil::StrToDateTime(mStartTime)<=StrUtil::Str2Int(mRecordTime)*60/*TimeUtil::StrToSecondTime(mRecordTime)*/)
		{
			ACE_Message_Block *mb = NULL;

			ACE_Time_Value OutTime(ACE_OS::gettimeofday()+ACE_Time_Value(2));
			if (Task->getq(mb,&OutTime) != -1 && mb != NULL)//�����ݽ����߳�ȡ����
			{
				if (mb->length() > 0)
				{
					if (PacketLen>=BufLen-mb->length())
					{
						mRecordFile.WriteBuf(TsBuf,PacketLen);//����д���ļ�
						memset(TsBuf,0,BufLen);
						PacketLen = 0;
					}
				
					memcpy(TsBuf+PacketLen,(unsigned char*)mb->rd_ptr(),mb->length());//�������ڴ�
					PacketLen += mb->length();
				}
				
				mb->release();
			}
		}
		//���ֶ�¼������,���ֶ�¼�������Ѿ���ʱ
		else if(bManualRecord&&time(0)-TimeUtil::StrToDateTime(mStartTime)>StrUtil::Str2Int(mRecordTime)*60)
		{
			ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]�ֶ�¼������ֹͣ !\n",DeviceID));
			bManualRecord=false;
			mRecordFile.Close();
			//�޸��ֶ�¼���ļ����  jidushan 110406
			string temptime = TimeUtil::GetCurDateTime();
			DBMANAGER::instance()->AddTempFile(mRecordFileName, temptime, MANURECORD_EXPIREDAYS);
			SendXML(TranslateUpXML::TranslateManualRecordReport(this,mManualRecordXML,mURL));
		}
		else 
		{
			Task->msg_queue()->flush();
			OSFunction::Sleep(0,100);
		}
		
	}
	//�ֶ�¼��ʱ֮ǰ,ʵʱ��Ƶֹͣ��,�ͽ��ֶ�¼��Ҳֹͣ
	if(bManualRecord&&time(0)-TimeUtil::StrToDateTime(mStartTime)<=StrUtil::Str2Int(mRecordTime)*60)
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]�ֶ�¼������ֹͣ !\n",DeviceID));
		//�޸��ֶ�¼���ļ����  jidushan 110406
		string temptime = TimeUtil::GetCurDateTime();
		DBMANAGER::instance()->AddTempFile(mRecordFileName, temptime, MANURECORD_EXPIREDAYS);

		SendXML(TranslateUpXML::TranslateManualRecordReport(this,mManualRecordXML,mURL));
	}

	delete[] TsBuf;//�ͷ��ڴ�
	TSFETCHERMGR::instance()->DelRecordTask(DeviceID,Task);
	bRun = false;

	if (TaskScheduler != NULL)
		TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));

	SetFinised();	//����ֹͣ
	TSFETCHERMGR::instance()->DecreaseTaskNum(DeviceID);
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]ʵʱ��Ƶ����ֹͣ !\n",DeviceID));
	g_realstream = false;
}

bool StreamRealtimeQueryTask::StartManualRecord(std::string ManualRecordXml)
{
	bool ret=false;
	
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]�ֶ�¼������ʼ !\n",DeviceID));

	mManualRecordXML=ManualRecordXml;

	XmlParser parser;
	parser.Set_xml(ManualRecordXml);
	pXMLNODE manualRecordNode=parser.GetNodeFromPath("Msg/ManualRecordSet/ManualRecord");
	parser.GetAttrNode(manualRecordNode,"Time",mRecordTime);
	
	//�������ļ��л��¼���ļ���ʵ�ʱ���λ�ã�������ʱ�ļ���
	std::string realpath=PROPMANAGER::instance()->GetTempFileLoac();

	//�������
	std::string datetime=TimeUtil::DateTimeToString(time(0));
	datetime=datetime.substr(4, datetime.size()- 4);

	//ƴ���ļ�����·��+ͨ����+����id+����
	std::string fileName = StrUtil::Int2Str(DeviceID) + string("_")  + TaskID + string("_") + datetime;
	fileName+=".ts";
	
	std::string fullname = realpath + fileName;		//����ʵ��·�����ļ���
	mRecordFileName = fullname;
	//�����ļ�
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)����[%s]!\n",fullname.c_str()));
	ret=mRecordFile.CreateNewFile(fullname.c_str());

	//��ȡ����·��
	std::string sharepath = PROPMANAGER::instance()->GetTempSharePath();
	
	//��ȡ�ļ�����Э������	wz_110310
	string fileprotocol = "";
	ret = PROPMANAGER::instance()->GetFileProtocol(GetDVBType(), fileprotocol);
	if (!ret)
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)��ȡ�ļ�����Э��ʧ��!\n"));
		return false;
	}
	mURL=fileprotocol+"://" +PROPMANAGER::instance()->GetHttpServerIP() + sharepath + fileName;//¼���ļ���ŵ�url
	mStartTime=TimeUtil::GetCurDateTime();
	bManualRecord=true;
	return ret;
}