///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：BusinessLayoutMgr.h
// 创建者：jiangcheng
// 创建时间：2009-05-27
// 内容描述：业务调度规划层接口管理类
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "./TaskMonitor.h"
#include "./TaskRealTimeMonitor.h"
#include "./AutoAnalysisTimeMonitor.h"
#include "./ClientInfoMgr.h"
#include "DeciceDataDealMgr.h"
#include "DiskSpaceMgr.h"
#include "../DeviceMgr/DeviceManager.h"

class BusinessLayoutMgr
{
public:
	BusinessLayoutMgr();
	virtual ~BusinessLayoutMgr();
public:
	int Start();
	int Stop();	

public:

	bool AddTask(XMLTask* task);			//调度对外统一的添加任务接口
	bool AddRecTask(XMLTask* task);			//调度对外统一的添加任务接口
	bool DelTask(std::string taskid);		//调度对外统一的删除任务接口
	bool DelTask(int deviceid,std::string taskid);
	bool DelTask(eDVBType dvbtype,std::string taskid);
	bool DelRecInfo(eDVBType dvbtype,std::string taskid);

	bool DeviceStatusQuery(int deviceid,int& DeviceStatus,std::string& TaskName);	//通道状态查询接口
	bool SetManualRecord(int deviceid,enumDVBType dvbtype,std::string& Taskxml);	//设置手动录像

	bool QueryRunTaskInfo(int deviceid,sTaskInfo& taskinfo);								//获取指定通道内运行任务信息
	bool QueryRecRunTaskInfo(int deviceid,sTaskInfo& taskinfo);								//
	bool QueryAllTaskInfo(eDVBType dvbtype,std::vector<sTaskInfo>& taskinfovec);			//获取指定类型的全部任务信息
	bool GetAutoRecordDeviceIdByFreq(eDVBType dvbtype,std::string freq,int& deviceid);		//通过类型和频点获取通道信息
	bool GetAutoRecordDeviceIdByChannelID(eDVBType dvbtype,std::string channelid,int& deviceid);  //通过监测类型，channelID获取通道信息
	bool AddDeviceSocket(ACE_SOCK_Stream socket);

	//音视频用户管理
	bool AddUser(ACE_SOCK_Stream client,string& xml);
	bool GetUser(enumDVBType DvbType,std::vector<sVedioUserInfo>& uservec);
	bool StopUser(int deviceid);

private:
	bool TaskChannleIsValid(XMLTask* task);			//判断任务频道是否合法
	bool ProcessTaskDeviceID(XMLTask* task);		//处理任务通道信息，有通道判断合法性，无通道自动设置通道
	bool ProcessTaskRunPriority(XMLTask* task);		//处理任务运行优先级
	bool ProcessRealtimeTask(XMLTask* task);		//处理实时类型任务
	bool ProcessAutorecordTask(XMLTask* task);		//处理自动录像类型任务

private:
	void CreateInstance();
	void DestoryInstance();

private:
	TaskMonitor* pTaskMonitor;
	TaskRealTimeMonitor* pTaskRealTimeMonitor;
	DiskSpaceMgr *pDiskSpaceMgr;
	DeciceDataDealMgr* pDeciceDataDealMgr;
	ClientInfoMgr* pClientInfoMgr;
	AutoAnalysisTimeMonitor* pAutoAnalysisTimeMonitor;
	DeviceManager* pDeviceManager;
};

typedef ACE_Singleton<BusinessLayoutMgr,ACE_Mutex>  BUSINESSLAYOUTMGR;
