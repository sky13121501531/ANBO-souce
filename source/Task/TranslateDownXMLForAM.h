///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����TranslateDownXMLForAM.h
// �����ߣ�gaoxd
// ����ʱ�䣺2010-04-09
// �������������·�XMLת��Ϊ���������XML
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include <string>
#include "../Foundation/XmlParser.h"

class TranslateDownXMLForAM
{
public:
	//���ر�׼��ʵʱ��Ƶ����XML
	static bool TranslateStreamRealtimeQuery(const std::string& strOriginalXML,std::string& strStandardXML);
	//���ر�׼��¼����������XML
	static bool TranslateRecordSet(const std::string& strOriginalXML,std::string& strStandardXML);
	//���ر�׼��¼��ִ������XML
	static bool TranslateRecordTask(const std::string& strOriginalXML,std::vector<std::string>& strStandardXMLVec);
	//���ر�׼��¼���ѯ������XML
	static bool TranslateRecordQuery(const std::string& strOriginalXML,std::string& strStandardXML);
	//���ر�׼��Ƶ��ɨ��XML
	static bool TranslateSpectrumScanQuery( const std::string& strOriginalXML,std::string& strStandardXML);
	//���ر�׼��ʵʱָ���ѯXML
	static bool TranslateQualityRealtimeQuery( const std::string& strOriginalXML,std::string& strStandardXML);
	//���ر�׼�Ķ�ʱָ��ִ������xml
	static bool TranslateQualityTaskSet(const std::string& strOriginalXML,std::string& strStandardXML);
	//���ر�׼�Ķ�ʱָ������ѯ����xml
	static bool TranslateQualityTask(const std::string& strOriginalXML,std::vector<std::string>& strStandardXMLVec);
	//Ƶ��ɨ��
	//Ƶ��ɨ��
	static bool TranslateChannelScanQuery( const std::string& strOriginalXML,std::string& strStandardXML);
	//�ֲ�
	static bool TranslateStreamRound(const std::string& strOriginalXML,std::string& strStandardXML);
	//Ƶ������
	static bool TranslateChannelSet(const std::string& strOriginalXML,std::string& strStandardXML);
	//������������
	static bool TranslateAlarmParamSet(const std::string& strOriginalXML,std::string& strStandardXML);
	//Ƶ����������
	static bool TranslateSpectrumTaskSet(const std::string& strOriginalXML,std::string& strStandardXML);
	//Ƶ������
	static bool TranslateSpectrumTask(const std::string& strOriginalXML,std::vector<std::string>& strStandardXMLVec);
	//Ƶ�ײ�ѯ
	static bool TranslateSpectrumQueryTask(const std::string& strOriginalXML,std::string& strStandardXML);
	//ָ���ѯ
	static bool TranslateQualityQuery(const std::string& strOriginalXML,std::string& strStandardXML);
	//�ͻ���Ϣ
	static bool TranslateClientInfoQuery(const std::string& strOriginalXML,std::string& strStandardXML);
	//ָֹͣ��ͨ���û�
	static bool TranslateStopClient(const std::string& strOriginalXML,std::string& strStandardXML);
	//���ر�׼������ͼ��������xml
	static bool TranslateRunPlanSet(const std::string& strOriginalXML,std::string& strStandardXML);
	//����״̬��ѯ
	static bool TranslateTaskSchedulerQuery(const std::string& strOriginalXML,std::string& strStandardXML);
	//�˹�����״̬��ѯ
	static bool TranslateManualTaskSchedulerQuery(const std::string& strOriginalXML,std::string& strStandardXML);

};