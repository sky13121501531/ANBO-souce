#pragma once

#include "DeviceRelatedTask.h"
using namespace std;

class SpectrumScanTask : public DeviceRelatedTask
{
public:
	SpectrumScanTask();
	SpectrumScanTask(std::string strXML);
	virtual ~SpectrumScanTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
	virtual bool IsRealTimeTask(){return true;};	//ʵʱ����

	int CreateRADIOTcpData(string RtnXML,unsigned char* TcpData,int size_n);
	int CreateTVTcpData(string RtnXML,unsigned char* TcpData,int size_n);

private:
	float mStartFreq;               //Ƶ��ɨ��Ŀ�ʼƵ��
	float mEndFreq;                 //Ƶ��ɨ��Ľ���Ƶ��
	float mStepFreq;                //Ƶ��ɨ��Ĳ���
	float mNowStartFreq;            //��ǰƵ�׶εĿ�ʼƵ��
	float mNowEndFreq;              //��ǰƵ�׶εĽ���Ƶ��
	float mNowFreq;                 //��ǰƵ�׶��ڵĵ�ǰ��Ƶ��
	float mVbw;
	float mRbw;
	float mReferDbu;
	int mIntevalLen;                //ÿ�εĳ���
	string mTaskType;
	
};