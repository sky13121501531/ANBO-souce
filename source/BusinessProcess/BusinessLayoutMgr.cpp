///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：BusinessLayoutMgr.cpp
// 创建者：jiangcheng
// 创建时间：2009-05-27
// 内容描述：业务调度规划层接口管理类
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
	//设备相关任务调度执行模块初始化
	if (pTaskMonitor->Init() == -1)
	{
		ACE_DEBUG((LM_ERROR,"(%T | %t)调度执行模块初始化错误!\n"));
		return -1;
	}
	else
	{
		pTaskMonitor->open(0);
	}
	
	if (pTaskRealTimeMonitor != NULL)
		pTaskRealTimeMonitor->open(0); //设备无关任务调度执行模块初始化
	
	if (pDiskSpaceMgr != NULL)	
		pDiskSpaceMgr->open(0);			//磁盘管理模块初始化执行
	
	if (pDeciceDataDealMgr != NULL)	
		pDeciceDataDealMgr->open(0);	//表数据处理程序
	
	if (pClientInfoMgr != NULL)
		pClientInfoMgr->open(0);		//用户信息管理线程启动
	
	if (pAutoAnalysisTimeMonitor != NULL)
		pAutoAnalysisTimeMonitor->open(0);//数据业务分析定时任务监控
	
	if (pDeviceManager != NULL)
		pDeviceManager->open(0);		//设备管理（重启线程）
	
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
///////////////////任务调度相关接口/////////////////////////
bool BusinessLayoutMgr::AddRecTask(XMLTask* task)
{
	if(task == NULL)
		return false;

	if (task->GetBaseObjectName() == "DeviceIndependentTask")
	{
		return pTaskRealTimeMonitor->AddTask(task);	//任务添加通道无关执行队列中
	}
	//与通道相关需要调度的任务
	if(	TaskChannleIsValid(task)		&&		//判断音视频任务通道是否合法
		ProcessTaskDeviceID(task)		&&		//处理任务通道（判断合法性或者自动分配通道）
		ProcessTaskRunPriority(task)	&&		//处理任务运行优先级
		ProcessRealtimeTask(task)		&&		//处理实时任务，判断是否可共享执行，是否有更高优先级占用
		ProcessAutorecordTask(task))			//处理自动录像任务
	{
		return pTaskMonitor->AddRecTask(task);//pTaskMonitor->AddRecTaskInfo(task);
	}
	else
	{
		return pTaskRealTimeMonitor->AddTask(task);			//低优先级任务直接进入实施执行队列运行，返回相应的XML
	}
	return false;
}

bool BusinessLayoutMgr::AddTask(XMLTask* task)
{
	if(task == NULL)
		return false;

	if (task->GetBaseObjectName() == "DeviceIndependentTask")
	{
		return pTaskRealTimeMonitor->AddTask(task);	//任务添加通道无关执行队列中
	}
	//与通道相关需要调度的任务
	if(	TaskChannleIsValid(task)		&&		//判断音视频任务通道是否合法
		ProcessTaskDeviceID(task)		&&		//处理任务通道（判断合法性或者自动分配通道）
		ProcessTaskRunPriority(task)	&&		//处理任务运行优先级
		ProcessRealtimeTask(task)		&&		//处理实时任务，判断是否可共享执行，是否有更高优先级占用
		ProcessAutorecordTask(task))			//处理自动录像任务
	{
		return pTaskMonitor->AddTask(task);
	}
	else
	{
		return pTaskRealTimeMonitor->AddTask(task);			//低优先级任务直接进入实施执行队列运行，返回相应的XML
	}
	return false;
}

bool BusinessLayoutMgr::TaskChannleIsValid(XMLTask* task)
{
	if(task == NULL)
		return false;

	if (task->IsVedioTask() && task->GetChannelID() == "")
	{
		string msg = "视频任务频点非法或频道表错误，频道ID为空";
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

	//为了广播页面指定通道(1~6)任务无法设置成功,统一改为自动分配
	//if(RADIO == task->GetDVBType()&&task->GetObjectName() == "RecordSetTask"&&task->GetDeviceID() >= 0)
	//{
	//	task->SetDeciveID(-1);
	//}
	//
	if (task->GetDeviceID() >= 0)		//有通道判断
	{
		/*2018-10-15
		 *
		 *山西铜川 没有AM接口但是需要AM的业务 
		 *
		 *这里只是简单的判断频点的大小，小于某一值就假设为AM业务
		 */
		if(task->GetDVBType() == RADIO && (StrUtil::Str2Float(task->GetTaskFreq()) < 30.0) && (task->GetObjectName()=="AutoRecord"||task->GetObjectName()=="TaskRecord"))
		{
			if (false == PROPMANAGER::instance()->IsDeviceAvaiable(task->GetDeviceID(),task->GetObjectName(), AM))
			{
				task->SetRetValue(DEVICEID_UNAVAIABLE);
				string errstr = task->GetTaskName() + string("通道[") + StrUtil::Int2Str(task->GetDeviceID()) + string("]不存在或无法完成任务");
				SYSMSGSENDER::instance()->SendMsg(errstr,task->GetDVBType(),VS_MSG_SYSALARM);

				errstr =task->GetTaskName() + string("设置失败");
				SYSMSGSENDER::instance()->SendMsg(errstr,task->GetDVBType(),VS_MSG_SYSALARM);
				return false;
			}
		}
		else if (false == PROPMANAGER::instance()->IsDeviceAvaiable(task->GetDeviceID(),task->GetObjectName(),task->GetDVBType()))
		{
			task->SetRetValue(DEVICEID_UNAVAIABLE);
			string errstr = task->GetTaskName() + string("通道[") + StrUtil::Int2Str(task->GetDeviceID()) + string("]不存在或无法完成任务");
			SYSMSGSENDER::instance()->SendMsg(errstr,task->GetDVBType(),VS_MSG_SYSALARM);

			errstr =task->GetTaskName() + string("设置失败");
			SYSMSGSENDER::instance()->SendMsg(errstr,task->GetDVBType(),VS_MSG_SYSALARM);
			return false;
		}
		return true;
	}
	else	//无通道处理
	{
		//获得可执行该类型任务通道组
		std::list<int> devicelist;
		PROPMANAGER::instance()->GetTaskDeviceList(task->GetObjectName(),task->GetDVBType(),devicelist);
		/*2018-10-15
		 *
		 *山西铜川 没有AM接口但是需要AM的业务 
		 *
		 *这里只是简单的判断频点的大小，小于某一值就假设为AM业务
		 */
		if(StrUtil::Str2Float(task->GetTaskFreq()) < 30.0 && (task->GetObjectName()=="AutoRecord"||task->GetObjectName()=="TaskRecord"))
		{
			devicelist.clear();
			PROPMANAGER::instance()->GetTaskDeviceList(task->GetObjectName(), AM ,devicelist);
		}

		//寻找录像通道时，除去通道列表中的实时通道
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
				||((task->GetTaskID()!="666666")&&((*Ptr)==StrUtil::Str2Int(PROPMANAGER::instance()->GetAlarmrecRadioDeciceid()))))//其他录制任务不能占用报警录像通道
			{
				devicelist.erase(Ptr);
				break;
			}
		}
		if (devicelist.empty())
		{
			task->SetRetValue(NODEVICEIDMATCH);
			string msg = string("没有完成[") + task->GetTaskName() +  string("]的通道");
			SYSMSGSENDER::instance()->SendMsg(msg,task->GetDVBType(),VS_MSG_SYSALARM);
			return false;
		}
		//先找能做该类型任务的空闲通道
		std::vector<sTaskInfo> taskinfovec;
		for (list<int>::iterator Ptr = devicelist.begin();Ptr!=devicelist.end();++Ptr)
		{
			taskinfovec.clear();
			pTaskMonitor->QueryTaskInfo((*Ptr),taskinfovec);
			if (taskinfovec.size() == 0)//该通道没有任务
			{
				task->SetDeciveID(*Ptr);
				return true;
			}
		}
		//没有找到能做该类型任务的空闲通道，判断是否能在其相应通道组内设置成功
		for (list<int>::iterator Ptr = devicelist.begin();Ptr!=devicelist.end();++Ptr)
		{
			//判断当前通道是否有相同或者更高优先级的任务
			taskinfovec.clear();
			pTaskMonitor->QueryTaskInfo((*Ptr),taskinfovec);
			std::vector<sTaskInfo>::iterator pTaskPtr=taskinfovec.begin();
			for (;pTaskPtr!=taskinfovec.end();++pTaskPtr)
			{
				//已有任务的优先级大于等于当前任务的优先级才进行比较
				if (StrUtil::Str2Int((*pTaskPtr).priority) >= StrUtil::Str2Int(task->GetPriority()))
					break;
			}
			if (pTaskPtr == taskinfovec.end())//该任务的优先级高于该通道的所有任务
			{
				task->SetDeciveID(*Ptr);
				return true;
			}
		}
		//不存在任务级别都低于当前任务的通道，找任务最少的通道设置该任务
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

	if (task->IsRealTimeTask() == false) //非实时任务
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
	if (ptr!=taskinfovec.end() && StrUtil::Str2Int((*ptr).priority) > StrUtil::Str2Int(task->GetPriority()))	//高优先级任务占用
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
		if (StrUtil::Str2Int(task->GetPriority()) >= StrUtil::Str2Int((*Ptr).priority)) //要设置的自动录像任务优先级别大于等于其他自动录像任务的优先级别
		{
			return pTaskMonitor->DelTask(task->GetDeviceID(),"0");	//历史任务设置过期
		}
		else
		{
			task->SetRetValue(PREFERENTIALTASK_USE);
			return false;
		}
	}
	//如果没有找到自动录像任务直接设置即可
	return true;
}
bool BusinessLayoutMgr::DeviceStatusQuery( int deviceid,int& DeviceStatus,std::string& TaskName )
{
	//判断默认通道号是否合法
	list<int> DeviceList;
	PROPMANAGER::instance()->GetAllDeviceList(DeviceList);
	list<int>::iterator pDeviceList = DeviceList.begin();
	for (;pDeviceList!=DeviceList.end();++pDeviceList)
	{
		if (deviceid == *pDeviceList)
			break;
	}
	if (pDeviceList == DeviceList.end())//该默认通道不在该调度所包含的通道组中
	{
		DeviceStatus = 1;
		TaskName = string("无板卡");
		return true;
	}
	std::vector<sTaskInfo> taskinfo;
	pTaskMonitor->QueryTaskInfo(deviceid,taskinfo);    //查询该通道的任务信息
	if (taskinfo.size()>0) 
	{
		size_t i=0;
		for (;i!=taskinfo.size();++i)
		{
			if(taskinfo[i].status==RUNNING)
			{
				DeviceStatus = 2;
				TaskName = taskinfo[i].taskname;      //获得任务名称
				break;
			}
		}
		if (i==taskinfo.size())
		{
			DeviceStatus = 0;
			TaskName = string("正常状态");
		}	
	}
	else
	{
		DeviceStatus = 0;//设备处于空闲状态
        TaskName = string("正常状态");
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

//音视频用户管理
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