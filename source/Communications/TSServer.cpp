#include "TSServer.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/OSFunction.h"
#pragma comment(lib,"../../thirdlib/rtsp/RTSPServer.lib")
TSServer::TSServer(void)
{
}

TSServer::~TSServer(void)
{
}

bool TSServer::CreateTsServer( char *szIP, int nPort )
{
	bool ret=RTSPCreateServer(szIP,nPort);
	if(ret)
	{
		std::list<int> DeviceList;
		PROPMANAGER::instance()->GetAllDeviceList(DeviceList);	
		//为每个通道创建TS流
		for (std::list<int>::iterator ptr = DeviceList.begin();ptr!=DeviceList.end();++ptr)
		{
			std::string streamName=StrUtil::Int2Str((*ptr));
			int streamID=CreateTsStream((char*)streamName.c_str());
			mapStreamID.insert(make_pair(streamName,streamID));
		}
		int roundStreamID=CreateTsStream("roundstream");
		mapStreamID.insert(make_pair("roundstream",roundStreamID));
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)TS流服务器创建成功 !\n"));
		return true;
	}
	else
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)TS流服务器创建失败 !\n"));
		return false;
	}
}
bool TSServer::AddTsData(std::string szStrName, char *szBuf, int nDataLen)
{
	
	std::map<std::string,int>::iterator ptr=mapStreamID.find(szStrName);
	if (ptr!=mapStreamID.end())
	{
		int streamID=ptr->second;
		return RTSPAddData(streamID,szBuf,nDataLen);
	}
	else
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)添加流数据失败，指定的流名字不存在 !\n"));
		return false;
	}
}
int TSServer::CreateTsStream( char *szStrName )
{
	return RTSPCreateStream(szStrName);
}

bool TSServer::DeleteTsStream( char *szStrName )
{
	return RTSPDeleteStream(szStrName);
}

bool TSServer::DeleteTsStream( int nStrID )
{
	return RTSPDeleteStream(nStrID);
}

bool TSServer::ReleaseTsServer()
{
	return RTSPReleaseServer();
}

bool TSServer::DelTsData( std::string szStrName )
{
	return true;
}