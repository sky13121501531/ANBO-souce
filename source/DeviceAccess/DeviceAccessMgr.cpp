///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：DeviceAccessMgr.cpp
// 创建者：jiangcheng
// 创建时间：2009-06-09
// 内容描述：硬件设备访问管理类
///////////////////////////////////////////////////////////////////////////////////////////
#include "DeviceAccessMgr.h"
#include "DeviceAccess.h"
#include "HTTPDeviceAccess.h"
#include "TCPDeviceAccess.h"
#include "./ATVTcpDeviceAccess.h"
#include "./ATVHTTPDeviceAccess.h"
#include "./RADIOTcpDeviceAccess.h"
#include "./RADIOHTTPDeviceAccess.h"
#include "./AMTcpDeviceAccess.h"
#include "./AMHTTPDeviceAccess.h"
#include "../Foundation/PropManager.h"
#include "HttpOpe.h"
#include "../Foundation/StrUtil.h"
#include "../Communications/SysMsgSender.h"

DeviceAccessMgr::DeviceAccessMgr()
{
	Init();
}

DeviceAccessMgr::~DeviceAccessMgr()
{
	UnInit();
}

void DeviceAccessMgr::Init()
{
	std::list<int> deviceList;
	if (PROPMANAGER::instance()->GetAllDeviceList(deviceList))//初始化设备列表
	{
		std::list<int>::iterator iter = deviceList.begin();
		for (; iter!=deviceList.end(); ++iter)
		{
			int deviceID = *iter,cmdport = -1;
			std::string strIPAddress = "",cmdprotocol = "";

			PROPMANAGER::instance()->GetDeviceIP(deviceID,strIPAddress);
			PROPMANAGER::instance()->GetDeviceCmdPort(deviceID,cmdport);
			PROPMANAGER::instance()->GetDeviceCmdProtocol(deviceID,cmdprotocol);

			if (strIPAddress=="" || cmdport==-1 || cmdprotocol=="")
			{
				string msg = string("获取设备[") + StrUtil::Int2Str(deviceID) + string("]IP地址、端口信息失败");
				SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
				continue;
			}

			DeviceAccess *pDeviceAccess = NULL;
			if (cmdprotocol=="http")
			{
				pDeviceAccess=new HTTPDeviceAccess(deviceID,strIPAddress,cmdport);
			}
			else
			{
				eDVBType eDvbtype = UNKNOWN;
				PROPMANAGER::instance()->GetDeviceType(deviceID, eDvbtype);
				switch (eDvbtype)
				{
				case ATV:
				case CTV:
					pDeviceAccess=new ATVHTTPDeviceAccess(deviceID,strIPAddress,cmdport);
					break;
				case RADIO:
					pDeviceAccess=new RADIOHTTPDeviceAccess(deviceID,strIPAddress,cmdport);
					break;
				case AM:
					pDeviceAccess=new AMHTTPDeviceAccess(deviceID,strIPAddress,cmdport);
					break;
				default:
					break;
				}
			}
			if(pDeviceAccess==NULL)
			{
				continue;
			}
			deviceAccessMap.insert(make_pair(deviceID,pDeviceAccess));//插入通道和设备对应信息
		}
	}
	else
	{
		string msg = string("设备访问管理类获取全部设备ID失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
}

void DeviceAccessMgr::UnInit()
{
	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.begin();
	for (; iter!=deviceAccessMap.end(); ++iter)
	{
		delete iter->second;
		iter->second = NULL;
	}
	deviceAccessMap.clear();
}

bool DeviceAccessMgr::SendTaskMsg(int deviceID, const std::string& strCmdMsg,std::string& strRetMsg)
{
	bool bRet = false;
	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->SendTaskMsg(strCmdMsg,strRetMsg);//向硬件设备发送命令

	return bRet;
}

bool DeviceAccessMgr::GetTsReceiveHandle(int deviceID,const std::string& strAddress,ACE_SOCK_Stream& streamHandle)
{
	bool bRet = false;
	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
	{
		return bRet;
	}
	//else
	//{
	//	deviceAccess = &((iter->second));
	//}
	bRet = iter->second->GetTsReceiveHandle(strAddress,streamHandle);//获得连接硬件设备的socket


	return bRet;
}
bool DeviceAccessMgr::CheckDeviceIDLock(int deviceID)
{
	bool bRet = false;
	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->CheckDeviceIDLock();

	return bRet;
}
#if 0
bool DeviceAccessMgr::GetRadioTunerQulity(int deviceID,VSTuner_Quality_Result_Obj& radiotunQ)
{
	bool bRet = false;
	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->GetRadioTunerQulity(radiotunQ);

	return bRet;
}
#endif

bool DeviceAccessMgr::CheckFreqLock(int freq)
{
	bool bRet = false;
	std::string strTempIP;
	int tempport;


	TunerConfig_Obj tuner_obj;	

	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.begin();
	for(;iter!=deviceAccessMap.end();iter++)
	{
		tuner_obj.freq = -1;
		iter->second->GetFreqInfo(tuner_obj);
		//cout<<"tuner_obj.freq>>>>>>>>" <<iter->first<<"=="<< tuner_obj.freq <<" === "<<freq <<endl;
		if(tuner_obj.freq == freq)
		{
			bRet = iter->second->CheckDeviceIDLock();
			return bRet;
		}
	}

	int realTaskdeviceid = 1;
	std::list<int> devicelist;
	PROPMANAGER::instance()->GetTaskDeviceList(string("StreamRealtimeQueryTask"),ATV,devicelist);
	if (devicelist.size() >= 1)
	{
		realTaskdeviceid = devicelist.front();
		// PROPMANAGER::instance()->GetDeviceIP(realTaskdeviceid,strTempIP);
		// PROPMANAGER::instance()->GetDeviceCmdPort(realTaskdeviceid,tempport);

	}
	iter = deviceAccessMap.find(realTaskdeviceid);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;
	PROPMANAGER::instance()->GetDeviceTunerID(realTaskdeviceid,tuner_obj.chan);
	tuner_obj.freq = freq;
//	ACE_DEBUG ((LM_DEBUG,"(%T | %t)before TurnFreqFor6U!\n"));
	bRet = iter->second->TurnFreqFor6U(freq);
	Sleep(100);
	
	if(bRet)
	{
		bRet = iter->second->CheckDeviceIDLock();
	}

	return bRet;
}
bool DeviceAccessMgr::SetSysTime(int deviceID)
{
	bool bRet = false;
	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->SetSysTime(deviceID);

	return bRet;
}


bool DeviceAccessMgr::GetTsReceiveHandle(const std::string& strAddress,ACE_SOCK_Stream& streamHandle)
{
	bool bRet = false;
	std::string strUrl = strAddress;
	strUrl = strUrl.substr(strUrl.find_first_of("/"));
	stringstream stream(strUrl);
	std::string strTemp;
	std::string strIPAddress,strSubRoot;

	int i = 0;
	while (getline(stream,strTemp,'/'))
	{
		if (strTemp != "")
		{
			if (i == 0)
				strIPAddress = strTemp;
			else
				strSubRoot = strTemp;
			i++;
		}
	}

	std::string strIP = strIPAddress.substr(0,strIPAddress.find(":"));
	std::string port = strIPAddress.substr(strIPAddress.find(":")+1);

	HttpOpe httpOpe(strIP.c_str(),StrUtil::Str2Int(port));
	ACE_Guard<ACE_Thread_Mutex> guard(getTsMutex);
	if (httpOpe.GetRecvHandle(strSubRoot,streamHandle))
		bRet = true;

	return bRet;
}


bool DeviceAccessMgr::setThresholdInfo(int deviceID)
{
	bool bRet = false;
// 	if(!(PROPMANAGER::instance()->IsRecordDeviceID(deviceID)))
// 		return bRet;

	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备
	
	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->setThresholdInfo();

	return bRet;
}


bool DeviceAccessMgr::getQualityInfoFor6U(int deviceID, VSTuner_Quality_Result_Obj& retObj)
{
	bool bRet = false;
	// 	if(!(PROPMANAGER::instance()->IsRecordDeviceID(deviceID)))
	// 		return bRet;

	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->getQualityInfoFor6U(retObj);

	return bRet;
}


bool DeviceAccessMgr::getAllQualityInfoFor6UInCard(int deviceID, VSTuner_AllQuality_Result_Obj& retObj)
{
	bool bRet = false;
	// 	if(!(PROPMANAGER::instance()->IsRecordDeviceID(deviceID)))
	// 		return bRet;

	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->getAllQualityInfoFor6UInCard(retObj);

	return bRet;
}

bool DeviceAccessMgr::getChanFixInfoFor6U(int deviceID)
{
	bool bRet = false;
	// 	if(!(PROPMANAGER::instance()->IsRecordDeviceID(deviceID)))
	// 		return bRet;

	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->getChanFixInfoFor6U();

	return bRet;
}


bool DeviceAccessMgr::getAllChanFixInfoFor6UInCard(int deviceID, VSTuner_AllChanFix_Result_Obj& retObj)
{
	bool bRet = false;
	// 	if(!(PROPMANAGER::instance()->IsRecordDeviceID(deviceID)))
	// 		return bRet;

	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->getAllChanFixInfoFor6UInCard(retObj);

	return bRet;
}

bool DeviceAccessMgr::LockFreqFor6U(int deviceID, LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj)
{
	bool bRet = false;

	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->LockFreqFor6U(lockFreqArr, retObj);

	return bRet;
}


bool DeviceAccessMgr::Start()
{
	std::cout << "设备管理类开始执行"<< std::endl;
	return true;
}

bool DeviceAccessMgr::defaultSpectRum(int deviceID, std::vector<int>& freqVec)
{
	bool bRet = false;

	std::map<int,DeviceAccess*>::iterator iter = deviceAccessMap.find(deviceID);//根据通道号查找硬件设备

	if (iter == deviceAccessMap.end())
		return bRet;

	bRet = iter->second->defaultSpectRum(freqVec);

	return bRet;
}