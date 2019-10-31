///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����TSRecorder.h
// �����ߣ�lichunfeng
// ����ʱ�䣺2009-06-04
// ��������������¼���ļ���
///////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include "../Foundation/FileOperater.h"
#include "../Foundation/TypeDef.h"
class TSRecorder
{
public:
	TSRecorder(int deviceid,std::string taskxml,std::string Taskid="");
	virtual~TSRecorder(void);
	bool CreateRecFile(void);
	bool SaveFile(unsigned char* buf,size_t num);
	bool SaveManualRecordFile(unsigned char* buf,size_t num);//�洢�ֶ�¼���ļ�
	void CloseFile(void);
	bool Save2DB(void);
private:
	TSRecorder();
private:
	std::string TaskXml;
	int DeviceID;
	int mChannelID;//Ƶ��id
	//��ʼ������
	std::string downType;std::string TaskID;std::string Freq;std::string Code;
	std::string OrgNetID;std::string TsID;std::string ServiceID;
	std::string VideoPID;std::string AudioPID;
	time_t mCurTime; 
	std::string mStartTime;//���ʱ��
	FileOperater mPFile;
	bool mIsCreate;
	std::string mFileName;
	sRecordInfo mRecordInfo;
};
