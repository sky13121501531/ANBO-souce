
#include "TranslateDownXML.h"
#include "TranslateDownXMLForCTTB.h"
#include "TranslateDownXMLForDVBC.h"
#include "TranslateDownXMLForRADIO.h"
#include "TranslateDownXMLForATV.h"
#include "TranslateDownXMLForAM.h"
#include "TranslateDownXMLForDVBS.h"
#include "TranslateDownXMLForCTV.h"
#include "TranslateDownXMLForTHREED.h"
#include "../Foundation/XmlParser.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Foundation/TypeDef.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/PropManager.h"
#include "../Communications/SysMsgSender.h"
#include "../Foundation/StrUtil.h"

bool TranslateDownXML::TranslateTaskXML(const std::string& strOriginalXML,std::string& strStandardXML)
{
	pXMLNODE RootNode = NULL;
	pXMLNODE childNode = NULL;
	std::string RootName,TaskName,DVBType,DstCode;

	try
	{
		if (strOriginalXML.empty()) return false;

		XmlParser pXP((const char *)strOriginalXML.c_str());
		RootName= pXP.GetRootNodeName();				//���ڵ�����
		
		RootNode = pXP.GetRootNode();					//���ڵ�
		SYSMSGSENDER::instance()->SendMsg(strOriginalXML,UNKNOWN,VS_MSG_SYSALARM);

		if (RootName != "Msg" && RootName != "msg")//�жϸ��ڵ�������Ƿ�Ϊmsg
		{
			string msg = "XML�ļ���ʽ�����޷����ɱ�׼XML:" + strOriginalXML;
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

			return false;
		}

		childNode = pXP.GetNodeFirstChild(RootNode);		//���ڵ�ĵ�һ���ӽڵ�
		pXP.GetAttrNode(RootNode,"Type",DVBType);			//�����Ϣ����
		pXP.GetAttrNode(RootNode,"DstCode",DstCode);
		if (DVBType.empty())
		{
			pXP.GetAttrNode(RootNode,"DVBType",DVBType);	//�����Ϣ����
		}

		if (DVBType=="SMSDown"||DVBType=="CASDown")
		{
			strStandardXML=strOriginalXML;
			return true;
		}
		TaskName = pXP.GetNodeName(childNode);				//�����������

		if (DVBType.empty() || TaskName.empty())
		{
			string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

			return false;
		}
		/*2018-10-15
		 *
		 *ɽ��ͭ�� û��AM�ӿڵ�����ҪAM��ҵ�� 
		 *
		 *����ֻ�Ǽ򵥵��ж�Ƶ��Ĵ�С��С��ĳһֵ�ͼ���ΪAMҵ��
		 */
		if (DVBType == "RadioDown" && TaskName=="StreamRealtimeQuery")//ģ��㲥��������
		{
			pXMLNODE strSRTNode = pXP.GetNodeFirstChild(childNode);
			std::string strFreq;
			pXP.GetAttrNode(strSRTNode, "Freq", strFreq);
			if(StrUtil::Str2Float(strFreq) < 30.0)
			{
				DVBType = "AMDown";
				pXP.SetAttrNode("Type", DVBType, RootNode);
				std::string strTmpOriginalXML;
				pXP.SaveToString(strTmpOriginalXML);
				return TranslateDownXMLForAM::TranslateStreamRealtimeQuery(strTmpOriginalXML,strStandardXML);
			}
		}
		//�����໭��ӿ�
		if(TaskName == "MultiVideoQuery")
		{
			strStandardXML = strOriginalXML;
			pXP.SetAttrNode("TaskType",string("MultiVideoQueryTask"),RootNode);
			pXP.SaveToString(strStandardXML);
			return true;
		}

		//¼��ָ������
		if ((DVBType == "CTTB" || DVBType == "DVBC" || DVBType == "DVBS" || DVBType == "RADIO" || DVBType == "ATV" || DVBType == "AM" || DVBType == "CTV" || DVBType == "THREED") && 
			(TaskName=="AutoRecord" || TaskName=="TaskRecord" || TaskName=="QualityTask" ||TaskName=="SpectrumTask" || TaskName=="RoundRecord"))
		{
			strStandardXML = strOriginalXML;
			return true;
		}
		if(DVBType==""&&TaskName=="")
		{
			return TranslateDownXMLForATV::TranslateServerStatusSet(strOriginalXML,strStandardXML);
		}
		if(DVBType == "CommonDown" && TaskName=="DeviceManage" )
		{
			pXP.SetAttrNode("TaskType",string("DeviceManage"),RootNode);
			pXP.SaveToString(strStandardXML);
			return true;
		}
		if(/*DVBType == "" && */TaskName=="DeviceInfo")
		{
			pXP.SetAttrNode("TaskType",string("DeviceInfo"),RootNode);//�ܾ��½ӿڣ��豸��Ϣ��ѯû�м������ȫ��Ϊ��·��������
			pXP.SetAttrNode("DVBType",string("ATV"),RootNode);
			pXP.SaveToString(strStandardXML);
			return true;
		}
		if (DVBType == "DVBTHDown")	//������������
		{
			if (TaskName=="StreamRealtimeQuery")										//ʵʱ��Ƶ
			{
				return TranslateDownXMLForCTTB::TranslateStreamRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamRoundQuery")										//��Ƶ�ֲ�
			{
				return TranslateDownXMLForCTTB::TranslateStreamRoundQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SetAutoRecordChannel" || TaskName=="TaskRecordSet")		//¼������
			{
				return TranslateDownXMLForCTTB::TranslateRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="AutoRecordFileQuery" || TaskName=="AutoRecordQuery" || 
					 TaskName=="TaskRecordFileQuery" || TaskName=="TaskRecordQuery")	//¼���ѯ������
			{
				return TranslateDownXMLForCTTB::TranslateRecordQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityRealtimeQuery")									//ʵʱָ���ѯ
			{
				return TranslateDownXMLForCTTB::TranslateQualityRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityTaskSet")										//��ʱָ������
			{
				return TranslateDownXMLForCTTB::TranslateQualityTaskSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ChannelSet")                                            //Ƶ������
			{
				return TranslateDownXMLForCTTB::TranslateChannelSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ManualRecordQuery")										//�ֶ�¼��
			{
				return TranslateDownXMLForCTTB::TranslateManualRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ClientInfoQuery")                                       //�ͻ���Ϣ��ѯ
			{
				return TranslateDownXMLForCTTB::TranslateClientInfoQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="GetDeviceStatus")                                       //ͨ��״̬��ѯ
			{
				return TranslateDownXMLForCTTB::TranslateDeviceStatusQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="IndexCompensationSet")								    //ָ�겹������
			{
				return TranslateDownXMLForCTTB::TranslateQualityCompensationSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="RunPlanSet")                                            //����ͼ����
			{
				return TranslateDownXMLForCTTB::TranslateRunPlanSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName =="AutoAnalysisTimeQuery")								//����ҵ�����ʱ������
			{
				return TranslateDownXMLForCTTB::TranslateAutoAnalysisTimeSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ChannelScanQuery")										//Ƶ��ɨ��
			{
				return TranslateDownXMLForCTTB::TranslateChannelScan(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="TableQuery")                                             //���ѯ
			{
				return TranslateDownXMLForCTTB::TranslateTableQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName =="EPGQuery")												//EPG��ѯ
			{
				return TranslateDownXMLForCTTB::TranslateEPGQuery(strOriginalXML,strStandardXML);	
			}
			else if (TaskName=="MHPQuery")												//MHP���ݲ�ѯ
			{
				return TranslateDownXMLForCTTB::TranslateMHPQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityQuery")											//��ʱָ������ѯ
			{
				return TranslateDownXMLForCTTB::TranslateQualityQuery(strOriginalXML,strStandardXML); 
			}
			else if(TaskName=="AlarmParamSet")											//������������
			{
				return TranslateDownXMLForCTTB::TranslateAlarmParamSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="GetIndexESet")											//����ָ���ѯ
			{
				return TranslateDownXMLForCTTB::TranslateEnvironmentQualityQuery(strOriginalXML,strStandardXML); 
			}
			else
			{
				string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
				SYSMSGSENDER::instance()->SendMsg(msg,CTTB,VS_MSG_SYSALARM);

				return false;
			}
		}
		else if (DVBType == "MonDown")//������������
		{
			if (TaskName=="ChangeProgramQuery")													//ʵʱ��Ƶ
			{
				return TranslateDownXMLForDVBC::TranslateStreamRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamRoundInfoQuery")											//��Ƶ�ֲ�
			{
				return TranslateDownXMLForDVBC::TranslateStreamRoundQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SetAutoRecordChannel" || TaskName=="ProvisionalRecordTaskSet")	//¼������
			{
				return TranslateDownXMLForDVBC::TranslateRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="RoundRecordSet")												//����¼��
			{
				return TranslateDownXMLForDVBC::TranslateRoundRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="NVRVideoHistoryInquiry" || TaskName=="NVRVideoHistoryDownInquiry" ||
				TaskName=="NVRTaskRecordInquiry"   || TaskName=="NVRTaskRecordDownInquiry" )//¼���ѯ������
			{
				return TranslateDownXMLForDVBC::TranslateRecordQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="GetIndexSet")													//ʵʱָ���ѯ
			{
				return TranslateDownXMLForDVBC::TranslateQualityRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityTaskSet")												//��ʱָ������
			{
				return TranslateDownXMLForDVBC::TranslateQualityTaskSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityQuery")													//��ʱָ������ѯ
			{
				return TranslateDownXMLForDVBC::TranslateQualityQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ChannelSet")													//Ƶ������
			{
				return TranslateDownXMLForDVBC::TranslateChannelSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ManualRecordQuery")												//�ֶ�¼��
			{
				return TranslateDownXMLForDVBC::TranslateManualRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="AutoAnalysisTimeQuery")											//����ҵ�����ʱ������
			{
				return TranslateDownXMLForDVBC::TranslateAutoAnalysisTimeSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ChannelScanQuery")												//Ƶ��ɨ��
			{
				return TranslateDownXMLForDVBC::TranslateChannelScan(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="EPGQuery")														//EPG��ѯ
			{
				return TranslateDownXMLForDVBC::TranslateEPGQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="table")															//���ѯ
			{
				return TranslateDownXMLForDVBC::TranslateTableQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="MHPQuery")														//MHP���ݲ�ѯ
			{
				return TranslateDownXMLForDVBC::TranslateMHPQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SpectrumScanQuery")												//Ƶ��ɨ��
			{
				return TranslateDownXMLForDVBC::TranslateSpectrumScan(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="AlarmTimeSet")													//����ͼ����
			{
				return TranslateDownXMLForDVBC::TranslateRunPlanSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="IndexCompensationSet")											//ָ�겹������
			{
				return TranslateDownXMLForDVBC::TranslateQualityCompensationSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ClientInfoQuery")												//�ͻ���Ϣ��ѯ
			{
				return TranslateDownXMLForDVBC::TranslateClientInfoQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="GetNvrStatus")													//ͨ��״̬��ѯ
			{
				return TranslateDownXMLForDVBC::TranslateDeviceStatusQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="GetIndexESet")													//����ָ���ѯ
			{
				return TranslateDownXMLForDVBC::TranslateEnvironmentQualityQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="AlarmThresholdSet"||TaskName=="AlarmProgramSwitchSet"||			//������������
				TaskName=="AlarmSwitchSet"||TaskName=="AlarmProgramThresholdSet")														//����ָ���ѯ
			{
				return TranslateDownXMLForDVBC::TranslateAlarmParamSet(strOriginalXML,strStandardXML); 
			}
			else if(TaskName=="AlarmTypeSet")													//������ʽ��������
			{
				return TranslateDownXMLForDVBC::TranslateAlarmReportSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="OSDSet")
			{
				return TranslateDownXMLForDVBC::TranslateOSDSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="NVRSteamRateSet")
			{
				return TranslateDownXMLForDVBC::TranslateStreamRealtimeRate(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="RecordCapabilityQuery")
			{
				return TranslateDownXMLForDVBC::TranslateRecordCapabilityQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="HDDefAudioParamSet")
			{
				return TranslateDownXMLForDVBC::TranslateHDDefAudioParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="AudioParamSet")
			{
				return TranslateDownXMLForDVBC::TranslateAudioParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="ClearAlarmState")  //����״̬���
			{
				return TranslateDownXMLForDVBC::TranslateClearAlarmState(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="MatrixQuery")  //�����л�
			{
				return TranslateDownXMLForDVBC::TranslateMatrixQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="ChangeQAMQuery")  //QAM����
			{
				return TranslateDownXMLForDVBC::TranslateChangeQAMQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="LoopAlaInf")  //ѭ�б�������
			{
				return TranslateDownXMLForDVBC::TranslateLoopAlarmInfo(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="RecordParamSet")  //��Ƶת��¼��Ĭ�ϲ�������
			{
				return TranslateDownXMLForDVBC::TranslateRecordParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="AlarmSearchESet"||TaskName=="AlarmSearchFSet"||TaskName=="AlarmSearchPSet")
			{
				return TranslateDownXMLForDVBC::TranslateAlarmQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="VideoClientStop")
			{
				return TranslateDownXMLForDVBC::TranslateStopClient(strOriginalXML,strStandardXML);
			}
			else
			{
				string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
				SYSMSGSENDER::instance()->SendMsg(msg,DVBC,VS_MSG_SYSALARM);

				return false;
			}
		}
		else if (DVBType == "3DDown")//������������
		{
			if (TaskName=="ChangeProgramQuery")													//ʵʱ��Ƶ
			{
				return TranslateDownXMLForTHREED::TranslateStreamRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamRoundInfoQuery")											//��Ƶ�ֲ�
			{
				return TranslateDownXMLForTHREED::TranslateStreamRoundQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SetAutoRecordChannel" || TaskName=="ProvisionalRecordTaskSet")	//¼������
			{
				return TranslateDownXMLForTHREED::TranslateRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="RoundRecordSet")												//����¼��
			{
				return TranslateDownXMLForTHREED::TranslateRoundRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="NVRVideoHistoryInquiry" || TaskName=="NVRVideoHistoryDownInquiry" ||
				TaskName=="NVRTaskRecordInquiry"   || TaskName=="NVRTaskRecordDownInquiry" )//¼���ѯ������
			{
				return TranslateDownXMLForTHREED::TranslateRecordQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="GetIndexSet")													//ʵʱָ���ѯ
			{
				return TranslateDownXMLForTHREED::TranslateQualityRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityTaskSet")												//��ʱָ������
			{
				return TranslateDownXMLForTHREED::TranslateQualityTaskSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityQuery")													//��ʱָ������ѯ
			{
				return TranslateDownXMLForTHREED::TranslateQualityQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ChannelSet")													//Ƶ������
			{
				return TranslateDownXMLForTHREED::TranslateChannelSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ManualRecordQuery")												//�ֶ�¼��
			{
				return TranslateDownXMLForTHREED::TranslateManualRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="AutoAnalysisTimeQuery")											//����ҵ�����ʱ������
			{
				return TranslateDownXMLForTHREED::TranslateAutoAnalysisTimeSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ChannelScanQuery")												//Ƶ��ɨ��
			{
				return TranslateDownXMLForTHREED::TranslateChannelScan(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="EPGQuery")														//EPG��ѯ
			{
				return TranslateDownXMLForTHREED::TranslateEPGQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="table")															//���ѯ
			{
				return TranslateDownXMLForTHREED::TranslateTableQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="MHPQuery")														//MHP���ݲ�ѯ
			{
				return TranslateDownXMLForTHREED::TranslateMHPQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SpectrumScanQuery")												//Ƶ��ɨ��
			{
				return TranslateDownXMLForTHREED::TranslateSpectrumScan(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="AlarmTimeSet")													//����ͼ����
			{
				return TranslateDownXMLForTHREED::TranslateRunPlanSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="IndexCompensationSet")											//ָ�겹������
			{
				return TranslateDownXMLForTHREED::TranslateQualityCompensationSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ClientInfoQuery")												//�ͻ���Ϣ��ѯ
			{
				return TranslateDownXMLForTHREED::TranslateClientInfoQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="GetNvrStatus")													//ͨ��״̬��ѯ
			{
				return TranslateDownXMLForTHREED::TranslateDeviceStatusQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="GetIndexESet")													//����ָ���ѯ
			{
				return TranslateDownXMLForTHREED::TranslateEnvironmentQualityQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="AlarmThresholdSet"||TaskName=="AlarmProgramSwitchSet"||			//������������
				TaskName=="AlarmSwitchSet"||TaskName=="AlarmProgramThresholdSet")														//����ָ���ѯ
			{
				return TranslateDownXMLForTHREED::TranslateAlarmParamSet(strOriginalXML,strStandardXML); 
			}
			else if(TaskName=="AlarmTypeSet")													//������ʽ��������
			{
				return TranslateDownXMLForTHREED::TranslateAlarmReportSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="OSDSet")
			{
				return TranslateDownXMLForTHREED::TranslateOSDSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="NVRSteamRateSet")
			{
				return TranslateDownXMLForTHREED::TranslateStreamRealtimeRate(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="RecordCapabilityQuery")
			{
				return TranslateDownXMLForTHREED::TranslateRecordCapabilityQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="HDDefAudioParamSet")
			{
				return TranslateDownXMLForTHREED::TranslateHDDefAudioParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="AudioParamSet")
			{
				return TranslateDownXMLForTHREED::TranslateAudioParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="ClearAlarmState")  //����״̬���
			{
				return TranslateDownXMLForTHREED::TranslateClearAlarmState(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="MatrixQuery")  //�����л�
			{
				return TranslateDownXMLForTHREED::TranslateMatrixQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="ChangeQAMQuery")  //QAM����
			{
				return TranslateDownXMLForTHREED::TranslateChangeQAMQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="LoopAlaInf")  //ѭ�б�������
			{
				return TranslateDownXMLForTHREED::TranslateLoopAlarmInfo(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="RecordParamSet")  //��Ƶת��¼��Ĭ�ϲ�������
			{
				return TranslateDownXMLForTHREED::TranslateRecordParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="AlarmSearchESet"||TaskName=="AlarmSearchFSet"||TaskName=="AlarmSearchPSet")
			{
				return TranslateDownXMLForTHREED::TranslateAlarmQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="VideoClientStop")
			{
				return TranslateDownXMLForTHREED::TranslateStopClient(strOriginalXML,strStandardXML);
			}
			else
			{
				string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
				SYSMSGSENDER::instance()->SendMsg(msg,THREED,VS_MSG_SYSALARM);

				return false;
			}
		}
		else if (DVBType == "SatDown")//������������
		{
			if (TaskName=="ChangeProgramQuery")													//ʵʱ��Ƶ
			{
				return TranslateDownXMLForDVBS::TranslateStreamRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamRoundInfoQuery")											//��Ƶ�ֲ�
			{
				return TranslateDownXMLForDVBS::TranslateStreamRoundQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SetAutoRecordChannel" || TaskName=="ProvisionalRecordTaskSet")	//¼������
			{
				return TranslateDownXMLForDVBS::TranslateRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="RoundRecordSet")												//����¼��
			{
				return TranslateDownXMLForDVBS::TranslateRoundRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="NVRVideoHistoryInquiry" || TaskName=="NVRVideoHistoryDownInquiry" ||
				TaskName=="NVRTaskRecordInquiry"   || TaskName=="NVRTaskRecordDownInquiry" )//¼���ѯ������
			{
				return TranslateDownXMLForDVBS::TranslateRecordQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="GetIndexSet")													//ʵʱָ���ѯ
			{
				return TranslateDownXMLForDVBS::TranslateQualityRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityTaskSet")												//��ʱָ������
			{
				return TranslateDownXMLForDVBS::TranslateQualityTaskSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityQuery")													//��ʱָ������ѯ
			{
				return TranslateDownXMLForDVBS::TranslateQualityQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ChannelSet")													//Ƶ������
			{
				return TranslateDownXMLForDVBS::TranslateChannelSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ManualRecordQuery")												//�ֶ�¼��
			{
				return TranslateDownXMLForDVBS::TranslateManualRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="AutoAnalysisTimeQuery")											//����ҵ�����ʱ������
			{
				return TranslateDownXMLForDVBS::TranslateAutoAnalysisTimeSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ChannelScanQuery")												//Ƶ��ɨ��
			{
				return TranslateDownXMLForDVBS::TranslateChannelScan(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="EPGQuery")														//EPG��ѯ
			{
				return TranslateDownXMLForDVBS::TranslateEPGQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="table")															//���ѯ
			{
				return TranslateDownXMLForDVBS::TranslateTableQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="MHPQuery")														//MHP���ݲ�ѯ
			{
				return TranslateDownXMLForDVBS::TranslateMHPQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SpectrumScanQuery")												//Ƶ��ɨ��
			{
				return TranslateDownXMLForDVBS::TranslateSpectrumScan(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="AlarmTimeSet")													//����ͼ����
			{
				return TranslateDownXMLForDVBS::TranslateRunPlanSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="IndexCompensationSet")											//ָ�겹������
			{
				return TranslateDownXMLForDVBS::TranslateQualityCompensationSet(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ClientInfoQuery")												//�ͻ���Ϣ��ѯ
			{
				return TranslateDownXMLForDVBS::TranslateClientInfoQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="GetNvrStatus")													//ͨ��״̬��ѯ
			{
				return TranslateDownXMLForDVBS::TranslateDeviceStatusQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="GetIndexESet")													//����ָ���ѯ
			{
				return TranslateDownXMLForDVBS::TranslateEnvironmentQualityQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="AlarmThresholdSet"||TaskName=="AlarmProgramSwitchSet"||			//������������
				TaskName=="AlarmSwitchSet"||TaskName=="AlarmProgramThresholdSet")														//����ָ���ѯ
			{
				return TranslateDownXMLForDVBS::TranslateAlarmParamSet(strOriginalXML,strStandardXML); 
			}
			else if(TaskName=="AlarmTypeSet")													//������ʽ��������
			{
				return TranslateDownXMLForDVBS::TranslateAlarmReportSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="OSDSet")
			{
				return TranslateDownXMLForDVBS::TranslateOSDSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="NVRSteamRateSet")
			{
				return TranslateDownXMLForDVBS::TranslateStreamRealtimeRate(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="RecordCapabilityQuery")
			{
				return TranslateDownXMLForDVBS::TranslateRecordCapabilityQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="HDDefAudioParamSet")
			{
				return TranslateDownXMLForDVBS::TranslateHDDefAudioParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="AudioParamSet")
			{
				return TranslateDownXMLForDVBS::TranslateAudioParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="ClearAlarmState")  //����״̬���
			{
				return TranslateDownXMLForDVBS::TranslateClearAlarmState(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="MatrixQuery")  //�����л�
			{
				return TranslateDownXMLForDVBS::TranslateMatrixQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="ChangeQAMQuery")  //QAM����
			{
				return TranslateDownXMLForDVBS::TranslateChangeQAMQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="LoopAlaInf")  //ѭ�б�������
			{
				return TranslateDownXMLForDVBS::TranslateLoopAlarmInfo(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="RecordParamSet")  //��Ƶת��¼��Ĭ�ϲ�������
			{
				return TranslateDownXMLForDVBS::TranslateRecordParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="AlarmSearchESet"||TaskName=="AlarmSearchFSet"||TaskName=="AlarmSearchPSet")
			{
				return TranslateDownXMLForDVBS::TranslateAlarmQuery(strOriginalXML,strStandardXML);
			}
			else
			{
				string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
				SYSMSGSENDER::instance()->SendMsg(msg,DVBS,VS_MSG_SYSALARM);

				return false;
			}
		}
		else if (DVBType == "RadioDown")//ģ��㲥��������
		{
			if (TaskName=="StreamRealtimeQuery")												//ʵʱ��Ƶ
			{
				return TranslateDownXMLForRADIO::TranslateStreamRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SetAutoRecordChannel"||TaskName=="TaskSet")											//¼������
			{
				return TranslateDownXMLForRADIO::TranslateRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamSimpleQuery"||"TaskRecordQuery"==TaskName || TaskName == "TaskRecordFileQuery" || TaskName == "RecordFileQuery")												//¼���ѯ������
			{
				return TranslateDownXMLForRADIO::TranslateRecordQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SpectrumRealtimeScan")											//Ƶ��ɨ��
			{
				return TranslateDownXMLForRADIO::TranslateSpectrumScanQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityRealtimeQuery")											//ʵʱָ���ѯ
			{
				return TranslateDownXMLForRADIO::TranslateQualityRealtimeQuery(strOriginalXML,strStandardXML); 
			}	
			else if (TaskName=="ChannelScanQuery")
			{
				return TranslateDownXMLForRADIO::TranslateChannelScanQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamRoundQuery")
			{
				return TranslateDownXMLForRADIO::TranslateStreamRound(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ChannelSet")
			{
				return TranslateDownXMLForRADIO::TranslateChannelSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityAlarmParamSet")
			{
				return TranslateDownXMLForRADIO::TranslateAlarmParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="QualityReportTaskSet")
			{
				return TranslateDownXMLForRADIO::TranslateQualityTaskSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="SpectrumScanTaskSet")
			{
				return TranslateDownXMLForRADIO::TranslateSpectrumTaskSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName =="FreqScanQuery")
			{
				return TranslateDownXMLForRADIO::TranslateSpectrumQueryTask(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "QualityQuery")
			{
				return TranslateDownXMLForRADIO::TranslateQualityQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "ClientInfoQuery")
			{
				return TranslateDownXMLForRADIO::TranslateClientInfoQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "VideoClientStop")
			{
				return TranslateDownXMLForRADIO::TranslateStopClient(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="RunPlanSet")                                            //����ͼ����
			{
				return TranslateDownXMLForRADIO::TranslateRunPlanSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="TaskSchedulerQuery")                                            
			{
				return TranslateDownXMLForRADIO::TranslateTaskSchedulerQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ManualTaskSchedulerQuery")                                            
			{
				return TranslateDownXMLForRADIO::TranslateManualTaskSchedulerQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="EquipmentAlarmParamSet")
			{
				return TranslateDownXMLForRADIO::TranslateEquipmentAlarmParamSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName == "InfoCheckRequest")		//������Ϣ��ѯ
			{
				return TranslateDownXMLForRADIO::TranslateInfoCheckRequest(strOriginalXML, strStandardXML);
			}
			else if (TaskName == "RunPlanCheckRequest")		//����ͼ��Ϣ��ѯ
			{
				return TranslateDownXMLForRADIO::TranslateRunPlanCheck(strOriginalXML, strStandardXML);
			}
			else if(TaskName == "TaskAndAlarmExpireRequest")
			{
				return TranslateDownXMLForRADIO::TranslateAlarmExpireTask(strOriginalXML,strStandardXML);
			}
			else
			{
				string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
				SYSMSGSENDER::instance()->SendMsg(msg,RADIO,VS_MSG_SYSALARM);

				return false;
			}
		}
		else if (DVBType == "TVMonDown"/* &&(DstCode==PROPMANAGER::instance()->GetDefSrcCode(ATV))*/)//��·ģ�������������
		{
			if (TaskName=="StreamRealtimeQuery")												//ʵʱ��Ƶ
			{
				return TranslateDownXMLForATV::TranslateStreamRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SetAutoRecordChannel"||TaskName=="TaskSet")											//¼������
			{
				return TranslateDownXMLForATV::TranslateRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamSimpleQuery"||"TaskRecordQuery"==TaskName || TaskName == "TaskRecordFileQuery")												//¼���ѯ������
			{
				return TranslateDownXMLForATV::TranslateRecordQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SpectrumRealtimeScan")											//Ƶ��ɨ��
			{
				return TranslateDownXMLForATV::TranslateSpectrumScanQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityRealtimeQuery")											//ʵʱָ���ѯ
			{
				return TranslateDownXMLForATV::TranslateQualityRealtimeQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ChannelScanQuery")
			{
				return TranslateDownXMLForATV::TranslateChannelScanQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ChannelSet")
			{
				return TranslateDownXMLForATV::TranslateChannelSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamRoundQuery")
			{
				return TranslateDownXMLForATV::TranslateStreamRound(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityAlarmParamSet")
			{
				return TranslateDownXMLForATV::TranslateAlarmParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="QualityReportTaskSet")
			{
				return TranslateDownXMLForATV::TranslateQualityTaskSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="SpectrumScanTaskSet")
			{
				return TranslateDownXMLForATV::TranslateSpectrumTaskSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName =="FreqScanQuery")
			{
				return TranslateDownXMLForATV::TranslateSpectrumQueryTask(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "QualityQuery")
			{
				return TranslateDownXMLForATV::TranslateQualityQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "ClientInfoQuery")
			{
				return TranslateDownXMLForATV::TranslateClientInfoQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "VideoClientStop")
			{
				return TranslateDownXMLForATV::TranslateStopClient(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="RunPlanSet")                                            //����ͼ����
			{
				return TranslateDownXMLForATV::TranslateRunPlanSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="TaskSchedulerQuery")                                            //����ͼ����
			{
				return TranslateDownXMLForATV::TranslateTaskSchedulerQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ManualTaskSchedulerQuery")                                            //����ͼ����
			{
				return TranslateDownXMLForATV::TranslateManualTaskSchedulerQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="EquipmentAlarmParamSet")
			{
				return TranslateDownXMLForATV::TranslateEquipmentAlarmParamSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName == "InfoCheckRequest")
			{
				return TranslateDownXMLForATV::TranslateInfoCheckRequest(strOriginalXML, strStandardXML);
			}
			else if (TaskName == "RunPlanCheckRequest")		//����ͼ��Ϣ��ѯ
			{
				return TranslateDownXMLForATV::TranslateRunPlanCheck(strOriginalXML, strStandardXML);
			}
			else if(TaskName == "TaskAndAlarmExpireRequest")
			{
				return TranslateDownXMLForATV::TranslateAlarmExpireTask(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "ServerStatusSet")
			{
				return TranslateDownXMLForATV::TranslateServerStatusSet(strOriginalXML,strStandardXML);
			}
			else
			{
				string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
				SYSMSGSENDER::instance()->SendMsg(msg,ATV,VS_MSG_SYSALARM);

				return false;
			}
		}
		else if (DVBType == "TVMonDown" &&(DstCode==PROPMANAGER::instance()->GetDefSrcCode(CTV)))//����ģ�������������
		{
			if (TaskName=="StreamRealtimeQuery")												//ʵʱ��Ƶ
			{
				return TranslateDownXMLForCTV::TranslateStreamRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SetAutoRecordChannel"||TaskName=="TaskSet")											//¼������
			{
				return TranslateDownXMLForCTV::TranslateRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamSimpleQuery"||"TaskRecordQuery"==TaskName || TaskName == "TaskRecordFileQuery")												//¼���ѯ������
			{
				return TranslateDownXMLForCTV::TranslateRecordQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SpectrumRealtimeScan")											//Ƶ��ɨ��
			{
				return TranslateDownXMLForCTV::TranslateSpectrumScanQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityRealtimeQuery")											//ʵʱָ���ѯ
			{
				return TranslateDownXMLForCTV::TranslateQualityRealtimeQuery(strOriginalXML,strStandardXML); 
			}
			else if (TaskName=="ChannelScanQuery")
			{
				return TranslateDownXMLForCTV::TranslateChannelScanQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ChannelSet")
			{
				return TranslateDownXMLForCTV::TranslateChannelSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamRoundQuery")
			{
				return TranslateDownXMLForCTV::TranslateStreamRound(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityAlarmParamSet")
			{
				return TranslateDownXMLForCTV::TranslateAlarmParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="QualityReportTaskSet")
			{
				return TranslateDownXMLForCTV::TranslateQualityTaskSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="SpectrumScanTaskSet")
			{
				return TranslateDownXMLForCTV::TranslateSpectrumTaskSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName =="SpectrumScanQuery")
			{
				return TranslateDownXMLForCTV::TranslateSpectrumQueryTask(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "QualityQuery")
			{
				return TranslateDownXMLForCTV::TranslateQualityQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "ClientInfoQuery")
			{
				return TranslateDownXMLForCTV::TranslateClientInfoQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "VideoClientStop")
			{
				return TranslateDownXMLForCTV::TranslateStopClient(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="RunPlanSet")                                            //����ͼ����
			{
				return TranslateDownXMLForCTV::TranslateRunPlanSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="TaskSchedulerQuery")                                            //����ͼ����
			{
				return TranslateDownXMLForCTV::TranslateTaskSchedulerQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ManualTaskSchedulerQuery")                                            //����ͼ����
			{
				return TranslateDownXMLForCTV::TranslateManualTaskSchedulerQuery(strOriginalXML,strStandardXML);
			}
			else
			{
				string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
				SYSMSGSENDER::instance()->SendMsg(msg,CTV,VS_MSG_SYSALARM);

				return false;
			}
		}
		else if (DVBType == "AMDown")//ģ��㲥��������
		{
			if (TaskName=="StreamRealtimeQuery")												//ʵʱ��Ƶ
			{
				return TranslateDownXMLForAM::TranslateStreamRealtimeQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SetAutoRecordChannel"||TaskName=="TaskSet")											//¼������
			{
				return TranslateDownXMLForAM::TranslateRecordSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamSimpleQuery"||"TaskRecordQuery"==TaskName || TaskName == "TaskRecordFileQuery")												//¼���ѯ������
			{
				return TranslateDownXMLForAM::TranslateRecordQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="SpectrumRealtimeScan")											//Ƶ��ɨ��
			{
				return TranslateDownXMLForAM::TranslateSpectrumScanQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityRealtimeQuery")											//ʵʱָ���ѯ
			{
				return TranslateDownXMLForAM::TranslateQualityRealtimeQuery(strOriginalXML,strStandardXML); 
			}	
			else if (TaskName=="ChannelScanQuery")
			{
				return TranslateDownXMLForAM::TranslateChannelScanQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="StreamRoundQuery")
			{
				return TranslateDownXMLForAM::TranslateStreamRound(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ChannelSet")
			{
				return TranslateDownXMLForAM::TranslateChannelSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="QualityAlarmParamSet")
			{
				return TranslateDownXMLForAM::TranslateAlarmParamSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="QualityReportTaskSet")
			{
				return TranslateDownXMLForAM::TranslateQualityTaskSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName=="SpectrumScanTaskSet")
			{
				return TranslateDownXMLForAM::TranslateSpectrumTaskSet(strOriginalXML,strStandardXML);
			}
			else if(TaskName =="FreqScanQuery")
			{
				return TranslateDownXMLForAM::TranslateSpectrumQueryTask(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "QualityQuery")
			{
				return TranslateDownXMLForAM::TranslateQualityQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "ClientInfoQuery")
			{
				return TranslateDownXMLForAM::TranslateClientInfoQuery(strOriginalXML,strStandardXML);
			}
			else if(TaskName == "VideoClientStop")
			{
				return TranslateDownXMLForAM::TranslateStopClient(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="RunPlanSet")                                            //����ͼ����
			{
				return TranslateDownXMLForAM::TranslateRunPlanSet(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="TaskSchedulerQuery")                                            //����ͼ����
			{
				return TranslateDownXMLForAM::TranslateTaskSchedulerQuery(strOriginalXML,strStandardXML);
			}
			else if (TaskName=="ManualTaskSchedulerQuery")                                            //����ͼ����
			{
				return TranslateDownXMLForAM::TranslateManualTaskSchedulerQuery(strOriginalXML,strStandardXML);
			}
			else
			{
				string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
				SYSMSGSENDER::instance()->SendMsg(msg,AM,VS_MSG_SYSALARM);

				return false;
			}
		}
		else
		{
			string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

			return false;
		}
	}
	catch(...)
	{
		string msg = "������ͻ����������ʹ����޷����ɱ�׼XML:" + strOriginalXML;
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

		return false;
	}
	string msg = "����XML���޷����ɱ�׼XML:" + strOriginalXML;
	SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

	return false;
}

bool TranslateDownXML::TranslateRecordTask(const std::string& strOriginalXML,std::vector<std::string>& strStandardXMLVec)
{
	string strType;
	XmlParser parser;
	parser.Set_xml(strOriginalXML);

	pXMLNODE rootnode=parser.GetRootNode();			//���ڵ�msg
	parser.GetAttrNode(rootnode,"DVBType",strType);	//��Ϣ����

	if (strType=="CTTB")
	{
		return TranslateDownXMLForCTTB::TranslateRecordTask(strOriginalXML,strStandardXMLVec);
	}
	else if (strType=="DVBC")
	{
		return TranslateDownXMLForDVBC::TranslateRecordTask(strOriginalXML,strStandardXMLVec);
	}
	else if (strType=="DVBS")
	{
		return TranslateDownXMLForDVBS::TranslateRecordTask(strOriginalXML,strStandardXMLVec);
	}
	else if (strType=="ATV")
	{
		return TranslateDownXMLForATV::TranslateRecordTask(strOriginalXML,strStandardXMLVec);
	}
	else if (strType=="RADIO")
	{
		return TranslateDownXMLForRADIO::TranslateRecordTask(strOriginalXML,strStandardXMLVec);
	}
	else if (strType=="AM")
	{
		return TranslateDownXMLForAM::TranslateRecordTask(strOriginalXML,strStandardXMLVec);
	}
	else if (strType=="CTV")
	{
		return TranslateDownXMLForCTV::TranslateRecordTask(strOriginalXML,strStandardXMLVec);
	}
	else if (strType=="THREED")
	{
		return TranslateDownXMLForTHREED::TranslateRecordTask(strOriginalXML,strStandardXMLVec);
	}

	return false;
}
//���ر�׼�Ă���¼��ִ������XML
 bool TranslateDownXML::TranslateRoundRecordTask(const std::string& strOriginalXML,std::vector<std::string>& strStandardXMLVec)
{
	XmlParser tempparser( strOriginalXML.c_str() );
	pXMLNODE tempRootNode = tempparser.GetRootNode();
	std::string dvbtype;
	tempparser.GetAttrNode(tempRootNode, "DVBType", dvbtype);
	if (dvbtype =="DVBC")
	{
		return TranslateDownXMLForDVBC::TranslateRoundRecordTask(strOriginalXML, strStandardXMLVec);
	}
	else if(dvbtype =="DVBS")
	{
		return TranslateDownXMLForDVBS::TranslateRoundRecordTask(strOriginalXML, strStandardXMLVec);
	}

	return false;
}
bool TranslateDownXML::TranslateQualityTask(const std::string& strOriginalXML,std::vector<std::string>& strStandardXMLVec)
{
	string strType;
	XmlParser parser;
	parser.Set_xml(strOriginalXML);

	pXMLNODE rootnode=parser.GetRootNode();			//���ڵ�msg
	parser.GetAttrNode(rootnode,"DVBType",strType);	//��Ϣ����

	if (strType=="CTTB")
	{
		return TranslateDownXMLForCTTB::TranslateQualityTask(strOriginalXML,strStandardXMLVec);
	}
	else if (strType=="DVBC")
	{
		return TranslateDownXMLForDVBC::TranslateQualityTask(strOriginalXML,strStandardXMLVec);
	}
	else if (strType=="DVBS")
	{
		return TranslateDownXMLForDVBS::TranslateQualityTask(strOriginalXML,strStandardXMLVec);
	}
	else if(strType=="ATV")
	{
		return TranslateDownXMLForATV::TranslateQualityTask(strOriginalXML,strStandardXMLVec);
	}
	else if(strType=="RADIO")
	{
		return TranslateDownXMLForRADIO::TranslateQualityTask(strOriginalXML,strStandardXMLVec);
	}
	else if(strType=="AM")
	{
		return TranslateDownXMLForAM::TranslateQualityTask(strOriginalXML,strStandardXMLVec);
	}
	else if(strType=="CTV")
	{
		return TranslateDownXMLForCTV::TranslateQualityTask(strOriginalXML,strStandardXMLVec);
	}
	return false;
}

bool TranslateDownXML::TranslateSpectrumTask(const std::string& strOriginalXML,std::vector<std::string>& strStandardXMLVec)
{
	string strType;
	XmlParser parser;
	parser.Set_xml(strOriginalXML);

	pXMLNODE rootnode=parser.GetRootNode();			//���ڵ�msg
	parser.GetAttrNode(rootnode,"DVBType",strType);	//��Ϣ����

	if(strType=="ATV")
	{
		return TranslateDownXMLForATV::TranslateSpectrumTask(strOriginalXML,strStandardXMLVec);
	}
	else if(strType=="RADIO")
	{
		return TranslateDownXMLForRADIO::TranslateSpectrumTask(strOriginalXML,strStandardXMLVec);
	}
	else if(strType=="AM")
	{
		return TranslateDownXMLForAM::TranslateSpectrumTask(strOriginalXML,strStandardXMLVec);
	}
	else if(strType=="CTV")
	{
		return TranslateDownXMLForCTV::TranslateSpectrumTask(strOriginalXML,strStandardXMLVec);
	}
	return false;

}