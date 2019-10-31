///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����BusinessLayoutMgr.cpp
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-05-27
// ����������ҵ����ȹ滮��ӿڹ�����
///////////////////////////////////////////////////////////////////////////////////////////
#include "BusinessLayoutMgr.h"
#include "../Foundation/PropManager.h"
#include "../Task/XMLTask.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/StrUtil.h"
#include "../Communications/SysMsgSender.h"

BusinessLayoutMgr::BusinessLayoutMgr()
{
	CreateInstance();
}

BusinessLayoutMgr::~BusinessLayoutMgr()
{
	DestoryInstance();
}

void BusinessLayoutMgr::CreateInstance()
{
	pTaskMonitor = new TaskMonitor;
	pTaskRealTimeMonitor = new TaskRealTimeMonitor;
	pDiskSpaceMgr = new DiskSpaceMgr;
	pDeciceDataDealMgr = new DeciceDataDealMgr;
	pClientInfoMgr = new ClientInfoMgr;
	pAutoAnalysisTimeMonitor = new AutoAnalysisTimeMonitor;
	pDeviceManager = new DeviceManager;
}

void BusinessLayoutMgr::DestoryInstance()
{
	if (NULL != pTaskMonitor)
	{
		delete pTaskMonitor;
		pTaskMonitor = NULL;
	}

	if (NULL != pTaskRealTimeMonitor)
	{
		delete pTaskRealTimeMonitor;
		pTaskRealTimeMonitor = NULL;
	}

	if (NULL != pDiskSpaceMgr)
	{
		delete pDiskSpaceMgr;
		pDiskSpaceMgr = NULL;
	}

	if (NULL != pDeciceDataDealMgr)
	{
		delete pDeciceDataDealMgr;
		pDeciceDataDealMgr = NULL;
	}
	if (NULL != pClientInfoMgr)
	{
		delete pClientInfoMgr;
		pClientInfoMgr = NULL;
	}

	if (NULL != pAutoAnalysisTimeMonitor)
	{
		delete pAutoAnalysisTimeMonitor;
		pAutoAnalysisTimeMonitor =NULL;
	}
	if (NULL != pDeviceManager)
	{
		delete pDeviceManager;
		pDeviceManager = NULL;
	}
}

int BusinessLayoutMgr::Start()
{
	//�豸����������ִ��ģ���ʼ��
	if (pTaskMonitor->Init() == -1)
	{
		ACE_DEBUG((LM_ERROR,"(%T | %t)����ִ��ģ���ʼ������!\n"));
		return -1;
	}
	else
	{
		pTaskMonitor->open(0);
	}
	
	if (pTaskRealTimeMonitor != NULL)
		pTaskRealTimeMonitor->open(0); //�豸�޹��������ִ��ģ���ʼ��
	
	if (pDiskSpaceMgr != NULL)	
		pDiskSpaceMgr->open(0);			//���̹���ģ���ʼ��ִ��
	
	if (pDeciceDataDealMgr != NULL)	
		pDeciceDataDealMgr->open(0);	//�����ݴ������
	
	if (pClientInfoMgr != NULL)
		pClientInfoMgr->open(0);		//�û���Ϣ�����߳�����
	
	if (pAutoAnalysisTimeMonitor != NULL)
		pAutoAnalysisTimeMonitor->open(0);//����ҵ�������ʱ������
	
	if (pDeviceManager != NULL)
		pDeviceManager->open(0);		//�豸���������̣߳�
	
	return 0;
}

int BusinessLayoutMgr::Stop()
{
	if (NULL != pTaskMonitor)
		pTaskMonitor->Stop();

	if (NULL != pTaskRealTimeMonitor)
		pTaskRealTimeMonitor->Stop();

	if (NULL != pDiskSpaceMgr)
		pDiskSpaceMgr->Stop();

	if (NULL != pDeciceDataDealMgr)
		pDeciceDataDealMgr->Stop();

	if (NULL != pClientInfoMgr)
		pClientInfoMgr->Stop();

	if (NULL != pAutoAnalysisTimeMonitor)
		pAutoAnalysisTimeMonitor->Stop();

	if (NULL != pDeviceManager)
		pDeviceManager->Stop();

	return 0;
}
bool BusinessLayoutMgr::AddDeviceSocket(ACE_SOCK_Stream socket)
{
	if (pDeciceDataDealMgr != NULL)
	{
		return pDeciceDataDealMgr->AddDeviceSocket(socket);
	}
	return false;
}
///////////////////���������ؽӿ�/////////////////////////
bool BusinessLayoutMgr::AddRecTask(XMLTask* task)
{
	if(task == NULL)
		return false;

	if (task->GetBaseObjectName() == "DeviceIndependentTask")
	{
		return pTaskRealTimeMonitor->AddTask(task);	//�������ͨ���޹�ִ�ж�����
	}
	//��ͨ�������Ҫ���ȵ�����
	if(	TaskChannleIsValid(task)		&&		//�ж�����Ƶ����ͨ���Ƿ�Ϸ�
		ProcessTaskDeviceID(task)		&&		//��������ͨ�����жϺϷ��Ի����Զ�����ͨ����
		ProcessTaskRunPriority(task)	&&		//���������������ȼ�
		ProcessRealtimeTask(task)		&&		//����ʵʱ�����ж��Ƿ�ɹ���ִ�У��Ƿ��и������ȼ�ռ��
		ProcessAutorecordTask(task))			//�����Զ�¼������
	{
		return pTaskMonitor->AddRecTask(task);//pTaskMonitor->AddRecTaskInfo(task);
	}
	else
	{
		return pTaskRealTimeMonitor->AddTask(task);			//�����ȼ�����ֱ�ӽ���ʵʩִ�ж������У�������Ӧ��XML
	}
	return false;
}

bool BusinessLayoutMgr::AddTask(XMLTask* task)
{
	if(task == NULL)
		return false;

	if (task->GetBaseObjectName() == "DeviceIndependentTask")
	{
		return pTaskRealTimeMonitor->AddTask(task);	//�������ͨ���޹�ִ�ж�����
	}
	//��ͨ�������Ҫ���ȵ�����
	if(	TaskChannleIsValid(task)		&&		//�ж�����Ƶ����ͨ���Ƿ�Ϸ�
		ProcessTaskDeviceID(task)		&&		//��������ͨ�����жϺϷ��Ի����Զ�����ͨ����
		ProcessTaskRunPriority(task)	&&		//���������������ȼ�
		ProcessRealtimeTask(task)		&&		//����ʵʱ�����ж��Ƿ�ɹ���ִ�У��Ƿ��и������ȼ�ռ��
		ProcessAutorecordTask(task))			//�����Զ�¼������
	{
		return pTaskMonitor->AddTask(task);
	}
	else
	{
		return pTaskRealTimeMonitor->AddTask(task);			//�����ȼ�����ֱ�ӽ���ʵʩִ�ж������У�������Ӧ��XML
	}
	return false;
}

bool BusinessLayoutMgr::TaskChannleIsValid(XMLTask* task)
{
	if(task == NULL)
		return false;

	if (task->IsVedioTask() && task->GetChannelID() == "")
	{
		string msg = "��Ƶ����Ƶ��Ƿ���Ƶ�������Ƶ��IDΪ��";
		SYSMSGSENDER::instance()->SendMsg(msg,task->GetDVBType(),VS_MSG_SYSALARM);

		task->SetRetValue(CHANNELID_UNAVAIABLE);
		return false;
	}

	return true;
}
bool BusinessLayoutMgr::ProcessTaskDeviceID(XMLTask* task)
{
	if(task == NULL)
		return false;

	//Ϊ�˹㲥ҳ��ָ��ͨ��(1~6)�����޷����óɹ�,ͳһ��Ϊ�Զ�����
	//if(RADIO == task->GetDVBType()&&task->GetObjectName() == "RecordSetTask"&&task->GetDeviceID() >= 0)
	//{
	//	task->SetDeciveID(-1);
	//}
	//
	if (task->GetDeviceID() >= 0)		//��ͨ���ж�
	{
		/*2018-10-15
		 *
		 *ɽ��ͭ�� û��AM�ӿڵ�����ҪAM��ҵ�� 
		 *
		 *����ֻ�Ǽ򵥵��ж�Ƶ��Ĵ�С��С��ĳһֵ�ͼ���ΪAMҵ��
		 */
		if(task->GetDVBType() == RADIO && (StrUtil::Str2Float(task->GetTaskFreq()) < 30.0) && (task->GetObjectName()=="AutoRecord"||task->GetObjectName()=="TaskRecord"))
		{
			if (false == PROPMANAGER::instance()->IsDeviceAvaiable(task->GetDeviceID(),task->GetObjectName(), AM))
			{
				task->SetRetValue(DEVICEID_UNAVAIABLE);
				string errstr = task->GetTaskName() + string("ͨ��[") + StrUtil::Int2Str(task->GetDeviceID()) + string("]�����ڻ��޷��������");
				SYSMSGSENDER::instance()->SendMsg(errstr,task->GetDVBType(),VS_MSG_SYSALARM);

				errstr =task->GetTaskName() + string("����ʧ��");
				SYSMSGSENDER::instance()->SendMsg(errstr,task->GetDVBType(),VS_MSG_SYSALARM);
				return false;
			}
		}
		else if (false == PROPMANAGER::instance()->IsDeviceAvaiable(task->GetDeviceID(),task->GetObjectName(),task->GetDVBType()))
		{
			task->SetRetValue(DEVICEID_UNAVAIABLE);
			string errstr = task->GetTaskName() + string("ͨ��[") + StrUtil::Int2Str(task->GetDeviceID()) + string("]�����ڻ��޷��������");
			SYSMSGSENDER::instance()->SendMsg(errstr,task->GetDVBType(),VS_MSG_SYSALARM);

			errstr =task->GetTaskName() + string("����ʧ��");
			SYSMSGSENDER::instance()->SendMsg(errstr,task->GetDVBType(),VS_MSG_SYSALARM);
			return false;
		}
		return true;
	}
	else	//��ͨ������
	{
		//��ÿ�ִ�и���������ͨ����
		std::list<int> devicelist;
		PROPMANAGER::instance()->GetTaskDeviceList(task->GetObjectName(),task->GetDVBType(),devicelist);
		/*2018-10-15
		 *
		 *ɽ��ͭ�� û��AM�ӿڵ�����ҪAM��ҵ�� 
		 *
		 *����ֻ�Ǽ򵥵��ж�Ƶ��Ĵ�С��С��ĳһֵ�ͼ���ΪAMҵ��
		 */
		if(StrUtil::Str2Float(task->GetTaskFreq()) < 30.0 && (task->GetObjectName()=="AutoRecord"||task->GetObjectName()=="TaskRecord"))
		{
			devicelist.clear();
			PROPMANAGER::instance()->GetTaskDeviceList(task->GetObjectName(), AM ,devicelist);
		}

		//Ѱ��¼��ͨ��ʱ����ȥͨ���б��е�ʵʱͨ��
		if(task->GetObjectName()=="AutoRecord"||task->GetObjectName()=="TaskRecord")
		{
			std::list<int> realTimeList;
			PROPMANAGER::instance()->GetTaskDeviceList("StreamRealtimeQueryTask",task->GetDVBType(),realTimeList);
			for(std::list<int>::iterator rtIter = realTimeList.begin(); rtIter != realTimeList.end(); rtIter++)
			{
				for(std::list<int>::iterator devIter = devicelist.begin(); devIter != devicelist.end(); devIter++)
				{
					if(*rtIter == *devIter)
					{
						devicelist.erase(devIter);
						break;
					}
				}
			}
		}
		
		for (list<int>::iterator Ptr = devicelist.begin();Ptr!=devicelist.end();++Ptr)
		{
			if(((task->GetTaskID()!="888888")&&((*Ptr)==StrUtil::Str2Int(PROPMANAGER::instance()->GetAlarmrecTvDeciceid())))
				||((task->GetTaskID()!="666666")&&((*Ptr)==StrUtil::Str2Int(PROPMANAGER::instance()->GetAlarmrecRadioDeciceid()))))//����¼��������ռ�ñ���¼��ͨ��
			{
				devicelist.erase(Ptr);
				break;
			}
		}
		if (devicelist.empty())
		{
			task->SetRetValue(NODEVICEIDMATCH);
			string msg = string("û�����[") + task->GetTaskName() +  string("]��ͨ��");
			SYSMSGSENDER::instance()->SendMsg(msg,task->GetDVBType(),VS_MSG_SYSALARM);
			return false;
		}
		//������������������Ŀ���ͨ��
		std::vector<sTaskInfo> taskinfovec;
		for (list<int>::iterator Ptr = devicelist.begin();Ptr!=devicelist.end();++Ptr)
		{
			taskinfovec.clear();
			pTaskMonitor->QueryTaskInfo((*Ptr),taskinfovec);
			if (taskinfovec.size() == 0)//��ͨ��û������
			{
				task->SetDeciveID(*Ptr);
				return true;
			}
		}
		//û���ҵ���������������Ŀ���ͨ�����ж��Ƿ���������Ӧͨ���������óɹ�
		for (list<int>::iterator Ptr = devicelist.begin();Ptr!=devicelist.end();++Ptr)
		{
			//�жϵ�ǰͨ���Ƿ�����ͬ���߸������ȼ�������
			taskinfovec.clear();
			pTaskMonitor->QueryTaskInfo((*Ptr),taskinfovec);
			std::vector<sTaskInfo>::iterator pTaskPtr=taskinfovec.begin();
			for (;pTaskPtr!=taskinfovec.end();++pTaskPtr)
			{
				//������������ȼ����ڵ��ڵ�ǰ��������ȼ��Ž��бȽ�
				if (StrUtil::Str2Int((*pTaskPtr).priority) >= StrUtil::Str2Int(task->GetPriority()))
					break;
			}
			if (pTaskPtr == taskinfovec.end())//����������ȼ����ڸ�ͨ������������
			{
				task->SetDeciveID(*Ptr);
				return true;
			}
		}
		//���������񼶱𶼵��ڵ�ǰ�����ͨ�������������ٵ�ͨ�����ø�����
		int tempDeviceId = *devicelist.begin();
		taskinfovec.clear();
		pTaskMonitor->QueryTaskInfo(tempDeviceId,taskinfovec);

		size_t task_num =taskinfovec.size();

		for (list<int>::iterator Ptr = devicelist.begin();Ptr!=devicelist.end();++Ptr)
		{
			taskinfovec.clear();
			pTaskMonitor->QueryTaskInfo((*Ptr),taskinfovec);
			if (taskinfovec.size() < task_num)
			{
				tempDeviceId = *Ptr;
				task_num = taskinfovec.size();
			}
		}
		task->SetDeciveID(tempDeviceId);
		return true;
	}
}
bool BusinessLayoutMgr::ProcessTaskRunPriority(XMLTask* task)
{
	if(task == NULL)
		return false;

	return true;
}
bool BusinessLayoutMgr::ProcessRealtimeTask(XMLTask* task)
{
	if(task == NULL)
		return false;

	if (task->IsRealTimeTask() == false) //��ʵʱ����
		return true;

	if(pTaskMonitor->ShareDevice(task) == true)
		return true;

	vector<sTaskInfo> taskinfovec;
	pTaskMonitor->QueryTaskInfo(task->GetDeviceID(),taskinfovec);

	vector<sTaskInfo>::iterator ptr = taskinfovec.begin();
	for (;ptr!=taskinfovec.end();++ptr)
	{
		if ((*ptr).status == RUNNING)
			break;
	}
	if (ptr!=taskinfovec.end() && StrUtil::Str2Int((*ptr).priority) > StrUtil::Str2Int(task->GetPriority()))	//�����ȼ�����ռ��
	{
		task->SetRetValue(PREFERENTIALTASK_USE);
		return false;
	}
	return true;
}
bool BusinessLayoutMgr::ProcessAutorecordTask(XMLTask* task)
{
	if(task == NULL)
		return false;

	std::string tasktype = task->GetObjectName();
	if (tasktype!="AutoRecord")
		return true;

	std::vector<sTaskInfo> taskinfovec;
	pTaskMonitor->QueryTaskInfo(task->GetDeviceID(),taskinfovec);

	std::vector<sTaskInfo>::iterator Ptr=taskinfovec.begin();
	for (;Ptr!=taskinfovec.end();++Ptr)
	{
		if ((*Ptr).taskname=="AutoRecord")
			break;
	}
	if (Ptr != taskinfovec.end())
	{
		if (StrUtil::Str2Int(task->GetPriority()) >= StrUtil::Str2Int((*Ptr).priority)) //Ҫ���õ��Զ�¼���������ȼ�����ڵ��������Զ�¼����������ȼ���
		{
			return pTaskMonitor->DelTask(task->GetDeviceID(),"0");	//��ʷ�������ù���
		}
		else
		{
			task->SetRetValue(PREFERENTIALTASK_USE);
			return false;
		}
	}
	//���û���ҵ��Զ�¼������ֱ�����ü���
	return true;
}
bool BusinessLayoutMgr::DeviceStatusQuery( int deviceid,int& DeviceStatus,std::string& TaskName )
{
	//�ж�Ĭ��ͨ�����Ƿ�Ϸ�
	list<int> DeviceList;
	PROPMANAGER::instance()->GetAllDeviceList(DeviceList);
	list<int>::iterator pDeviceList = DeviceList.begin();
	for (;pDeviceList!=DeviceList.end();++pDeviceList)
	{
		if (deviceid == *pDeviceList)
			break;
	}
	if (pDeviceList == DeviceList.end())//��Ĭ��ͨ�����ڸõ�����������ͨ������
	{
		DeviceStatus = 1;
		TaskName = string("�ް忨");
		return true;
	}
	std::vector<sTaskInfo> taskinfo;
	pTaskMonitor->QueryTaskInfo(deviceid,taskinfo);    //��ѯ��ͨ����������Ϣ
	if (taskinfo.size()>0) 
	{
		size_t i=0;
		for (;i!=taskinfo.size();++i)
		{
			if(taskinfo[i].status==RUNNING)
			{
				DeviceStatus = 2;
				TaskName = taskinfo[i].taskname;      //�����������
				break;
			}
		}
		if (i==taskinfo.size())
		{
			DeviceStatus = 0;
			TaskName = string("����״̬");
		}	
	}
	else
	{
		DeviceStatus = 0;//�豸���ڿ���״̬
        TaskName = string("����״̬");
	}
	return true;
}

bool BusinessLayoutMgr::DelTask( std::string taskid )
{
	if (pTaskMonitor == NULL)
		return false;

	return pTaskMonitor->DelTask(taskid);
}

bool BusinessLayoutMgr::DelTask( int deviceid,std::string taskid )
{
	if (pTaskMonitor == NULL)
		return false;

	return pTaskMonitor->DelTask(deviceid,taskid);
}

bool BusinessLayoutMgr::DelTask(eDVBType dvbtype,std::string taskid)
{
	if (pTaskMonitor == NULL)
		return false;

	return pTaskMonitor->DelTask(dvbtype,taskid);
}

bool BusinessLayoutMgr::DelRecInfo(eDVBType dvbtype,std::string taskid)
{
	if (pTaskMonitor == NULL)
		return false;

	return pTaskMonitor->DelRecInfo(dvbtype,taskid);
}

bool BusinessLayoutMgr::SetManualRecord(int deviceid,enumDVBType dvbtype,std::string& Taskxml)
{
	if (pTaskMonitor == NULL)
		return false;

	return pTaskMonitor->SetManualRecord(deviceid,dvbtype,Taskxml);
}
bool BusinessLayoutMgr::QueryRunTaskInfo(int deviceid,sTaskInfo& taskinfo)
{
	if (pTaskMonitor == NULL)
		return false;

	return pTaskMonitor->QueryRunTaskInfo(deviceid,taskinfo);
}
bool BusinessLayoutMgr::QueryRecRunTaskInfo(int deviceid,sTaskInfo& taskinfo)
{
	if (pTaskMonitor == NULL)
		return false;

	return pTaskMonitor->QueryRecRunTaskInfo(deviceid,taskinfo);
}
bool BusinessLayoutMgr::QueryAllTaskInfo(eDVBType dvbtype,std::vector<sTaskInfo>& taskinfovec)
{
	if (pTaskMonitor == NULL)
		return false;

	return pTaskMonitor->QueryAllTaskInfo(dvbtype,taskinfovec);
}
bool BusinessLayoutMgr::GetAutoRecordDeviceIdByFreq(eDVBType dvbtype,std::string freq,int& deviceid)
{
	if (pTaskMonitor == NULL)
		return false;

	return pTaskMonitor->GetAutoRecordDeviceIdByFreq(dvbtype,freq,deviceid);
}

bool BusinessLayoutMgr::GetAutoRecordDeviceIdByChannelID(eDVBType dvbtype,std::string channelid,int& deviceid)
{
	if (pTaskMonitor == NULL)
		return false;

	return pTaskMonitor->GetAutoRecordDeviceIdByChannelID(dvbtype,channelid,deviceid);
}

//����Ƶ�û�����
bool BusinessLayoutMgr::AddUser(ACE_SOCK_Stream client,string& xml)
{
	if (pClientInfoMgr == NULL)
		return false;

	return pClientInfoMgr->AddUser(client,xml);
}
bool BusinessLayoutMgr::GetUser(enumDVBType DvbType,std::vector<sVedioUserInfo>& uservec)
{
	if (pClientInfoMgr == NULL)
		return false;

	return pClientInfoMgr->GetUser(DvbType,uservec);
}
bool BusinessLayoutMgr::StopUser(int deviceid)
{
	if (pClientInfoMgr == NULL)
		return false;

	return pClientInfoMgr->StopUser(deviceid);
}