///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����RecordQueryTask.h
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-06-22
// �����������Զ�������¼���ѯ������������
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once 

#include "DeviceIndependentTask.h"

typedef enum eRecordType
{
	AUTORECORDQUERY		= 0,	//�Զ�¼���ѯ
	AUTORECORDFILEQUERY	= 1,	//�Զ�¼������
	TASKRECORDQUERY		= 2,	//����¼���ѯ
	TASKRECORDFILEQUERY	= 3,	//����¼������
} enumRecordType;

class RecordQueryTask : public DeviceIndependentTask
{
public:
	RecordQueryTask();
	RecordQueryTask(std::string strXML);
	virtual ~RecordQueryTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();

private:
	//����wpl����m3u�ļ�
	bool CreatePlayListFile(std::string listtype,std::string tempfilename,std::vector<sRecordInfo> vecFile,std::string Protocol);
	bool CreateDownFile(std::string tempfilename,std::vector<sRecordInfo> vecFile);	//���������ļ�

	bool RecordFileSpliter(vector < sRecordInfo>& fileVector,bool IsAudio);	//�и�¼���ļ�
	bool CreateListURL(std::vector<sRecordInfo> vecFile,std::vector<sRecordFileInfo> &FileInfo,std::string Protocol);//���ɶ���°���¼������URL
private:
	enumRecordType RecordType;
	string RequireDeviceID ;
	string RequireOrgNetID,RequireTsID,RequireServiceID,RequireVideoPID,RequireAudioPID,RequireChannelCode;
	string StartDateTime,EndDateTime;
	string QueryStartDateTime,QueryEndDateTime;
	string RequireURL;

	string VideoStreamProtocol,VideoFileProtocol,VideoFileurltype,FileOffset;	//�ļ��鿴����
	string DownFileProtocol,DownFileurltype,DownOffset;							//�ļ����ز���
};