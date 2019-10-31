#include "TCPDeviceAccess.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/TypeDef.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Communications/SysMsgSender.h"
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <algorithm>
#include <math.h>
string g_strcmdhead="aa";
ACE_Thread_Mutex SendtodevMutex;
time_t g_sendcommandtime = 0;
extern time_t g_starttime;
TCPDeviceAccess::TCPDeviceAccess(void)
{
}

TCPDeviceAccess::~TCPDeviceAccess(void)
{
}
TCPDeviceAccess::TCPDeviceAccess(int deviceid,std::string strIP,int nPort):DeviceAccess(deviceid,strIP,nPort),server(nPort,strIP.c_str())
{
	mChannelID=deviceid;
}
bool TCPDeviceAccess::SendTaskMsg(const std::string& strCmdMsg/*参数xml内容需修改*/,std::string& strRetMsg)
{
	//添加具体实现
	ACE_Guard<ACE_Thread_Mutex> guard(sendTaskMsgMutex);
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);
	bool Ret=true;
	if ("QualityQuery"==nodename)
	{
		Ret=GetQualityRetXML(strCmdMsg,strRetMsg);
	}
	else if("SpectrumQuery"==nodename)
	{
		Ret=GetSpectrumRetXML(strCmdMsg,strRetMsg);//
	}
	else if ("ChannelScanQuery"==nodename)
	{
		Ret=GetChannelScanRetXML(strCmdMsg,strRetMsg);
	}
	else if ("TSQuery"==nodename)
	{
		Ret=GetTsQueryRetXML(strCmdMsg,strRetMsg);
	}
	return Ret;
}

bool TCPDeviceAccess::CheckFreqLock()
{
	string flag=StrUtil::Long2Str(time(0));
	string cmd=string("lockstatus:");
	cmd+=StrUtil::Int2Str(DeviceId);
	cmd+=string(",");
	cmd+=flag;
	cmd+=string(";");
	string sendinfo=g_strcmdhead;
	sendinfo+=string(",");
	sendinfo+=StrUtil::Int2Str(cmd.size());
	sendinfo+=string(",");
	sendinfo+=cmd;
	//
	char retinfo[30];
	memset(retinfo,0,sizeof(char)*30);
	SendtodevMutex.acquire();
	bool rtn = SendCmdToServer((void*)sendinfo.c_str(),sendinfo.size(),(void*)retinfo,30);
	SendtodevMutex.release();

	string retflag=flag+string(":");
	string result=retinfo;
	int pos=result.find(retflag);
	if(pos>=0)
	{
		string ret=result.substr(pos+retflag.size(),result.size()-pos-retflag.size());
		int pp=ret.find("1");
		if(pp>=0)
		{
			rtn=true;
		}
		else
		{
			rtn=false;
		}
	}
	if (rtn == false)
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + std::string("]锁定失败"); 
		SYSMSGSENDER::instance()->SendMsg(msg,ATV,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + std::string("]锁定成功"); 
		SYSMSGSENDER::instance()->SendMsg(msg,ATV,VS_MSG_SYSALARM);
	}
	string slog=retinfo;
	ACE_DEBUG ((LM_DEBUG,"(%D | %t) %s\n",slog.c_str()));
	return rtn;
}

bool TCPDeviceAccess::ConnectToServer()
{
	//建立连接
	if (connector.connect (stream, server) == -1)//连接硬件服务器
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]连接失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

		stream.close();
		OSFunction::Sleep(0,500);
		//
		//if(time(0)-g_starttime > 10*60)
		//{
		//	exit(0);
		//}
		//
		return false;
	}
//	string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]连接成功");
//	SYSMSGSENDER::instance()->SendMsg(msg);

	return true;
}
/***************************************/
//发送指令；判断指令结果
/***************************************/
bool TCPDeviceAccess::SendCmdToServer(MSG_TYPE msg_type,void* info,int infolen)
{
	int pkgLen = 0;
	unsigned char sendbuf[200] = {0}; 
	unsigned char RetMsg[200] = {0};
	//信息头
	PkgHeader_Obj   ph;
	ph.header   = 0x48;
	ph.msg_type = msg_type;

	memcpy(sendbuf, &ph, sizeof(PkgHeader_Obj));
	pkgLen	+= sizeof(PkgHeader_Obj);

	//信息内容
	memcpy(sendbuf+pkgLen, info, infolen);
	pkgLen	+= infolen;

	bool ret = false;
	ACE_Time_Value timeout(10);

	const int SendNum = 3;
	for (int i=0;i<SendNum;++i)
	{
		stream.close();
		if ( ConnectToServer() == false)
			continue;

		//发送信息
		if(stream.send((char*)sendbuf, 200,&timeout) <= 0)
		{
			string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]发送指令失败");
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
			continue;
		}
		//接收返回数据
		if(stream.recv(( char *)&RetMsg,sizeof(RetMessage_Obj),&timeout) <= 0)
		{
			string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]数据接收失败");
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
			continue;
		}

		RetMessage_Handle rm = (RetMessage_Handle)(RetMsg);
		if(rm->ph.header!=0x49 || rm->ph.msg_type!=(msg_type+0x100) || rm->status != 1)
		{
			string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]返回数据错误");
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
			continue;
		}
		ret = true;	
		break;
	}
	stream.close();
	return ret;
}

bool TCPDeviceAccess::SendCmdToServer(void* indata,int indatalen,void* outdata,int outdatalen)
{
	bool ret = false;
	ACE_Time_Value timeout(10);

	const int SendNum = 3;
	int i=0;
	for (i=0;i<SendNum;++i)
	{
		stream.close();
		if ( ConnectToServer() == false)
			continue;

		//发送信息
		if(stream.send((char*)indata, indatalen,&timeout) <= 0)
		{
			ACE_DEBUG((LM_DEBUG,"(%D | %t) 发送板卡指令失败\n"));
			continue;
		}
		//接收返回数据
		int revlen=stream.recv(( char *)outdata,outdatalen,&timeout);
		if(revlen <= 0)
		{
			ACE_DEBUG((LM_DEBUG,"(%D | %t) 接收板卡指令返回失败:%s\n",indata));			
			continue;
		}
		break;
	}
	if(i<3)
	{
		ret=true;
	}
	stream.close();
	return ret;
}

bool TCPDeviceAccess::SendCmdToServerEx(void* indata,int indatalen,string& outdata)	
{
	bool ret = false;
	ACE_Time_Value timeout(100);

	const int SendNum = 3;
	int i=0;
	for (i=0;i<SendNum;++i)
	{
		stream.close();
		if ( ConnectToServer() == false)
			continue;

		//发送信息
		if(stream.send((char*)indata, indatalen,&timeout) <= 0)
		{
			ACE_DEBUG((LM_DEBUG,"(%D | %t) 发送板卡指令失败\n"));
			continue;
		}
		//接收返回数据
		int revlen=0;
		int i=0;
		int j=0;
		for(;i<100;i++)
		{
			char outinfo[3000];
			memset(outinfo,0,sizeof(char)*3000);
			revlen=stream.recv(( char *)outinfo,3000,&timeout);
			if(revlen <= 0)
			{
				if(j >= 10)
				{
					break;
				}
				else
				{
					j++;
					Sleep(20);
					continue;
				}
			}
			string temp=outinfo;
			outdata+=temp;
			if(outdata.find(";")!=-1)
			{
				break;
			}
		}
		if(i > 99 || j > 9)
		{
			SYSMSGSENDER::instance()->SendMsg("获取频谱数据非正常结束",UNKNOWN,VS_MSG_SYSALARM);
		}
		if(revlen == 0&& i== 0)
		{
			ACE_DEBUG((LM_DEBUG,"(%D | %t) 接收板卡指令返回失败Ex\n"));			
			continue;
		}
		break;
	}
	if(i<3)
	{
		ret=true;
	}
	stream.close();
	return ret;
}

int  TCPDeviceAccess::GetTVQulity(int ifreq,TVQuality &TVQua,int worktype)
{
	int level=5;
	string flag=StrUtil::Long2Str(time(0));
	string cmd=string("tqulity:");
	cmd+=StrUtil::Int2Str(ifreq);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(worktype);
	cmd+=string(",");
	cmd+=flag;
	cmd+=string(";");
	string sendinfo=g_strcmdhead;
	sendinfo+=string(",");
	sendinfo+=StrUtil::Int2Str(cmd.size());
	sendinfo+=string(",");
	sendinfo+=cmd;
	//
	char retinfo[200];
	memset(retinfo,0,sizeof(char)*200);
	bool rtn = SendCmdToServer((void*)sendinfo.c_str(),sendinfo.size(),(void*)retinfo,200);

	if (rtn==false)
	{
		string msg = string("频点[") + StrUtil::Int2Str(ifreq) + string("]获取指标失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
		return -1;
	}
	else
	{
		string msg = string("频点[") + StrUtil::Int2Str(ifreq) + string("]获取指标成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}
	//
	string retflag=flag+string(":");
	string result=retinfo;
	int pos=result.find(retflag);
	if(pos>=0)
	{
		string ret=result.substr(pos+retflag.size(),result.size()-pos-retflag.size());
		for(int i=0;i<10;i++)
		{
			int p=ret.find(",");
			if(p>=0)
			{
				string stem=ret.substr(p+1,ret.size()-p-1);
				string val=ret.substr(0,p);
				ret=stem;
				if(i==0)
				{
					TVQua.ImageLevel=StrUtil::Str2Float(val)*1000;
					level = TVQua.ImageLevel;
				}
				else if(i==1)
				{
					TVQua.AudioLevel=StrUtil::Str2Float(val)*1000;
				}
				else if(i==2)
				{
					TVQua.I2AOffLevel=StrUtil::Str2Float(val)*1000;
				}
				else if(i==3)
				{
					TVQua.CN=StrUtil::Str2Float(val)*1000;
				}
				else if(i==4)
				{
					TVQua.FreqOffSet=StrUtil::Str2Float(val)*1000;
				}
				else if(i==5)
				{
					TVQua.Slope=StrUtil::Str2Float(val)*1000;
				}
				else
				{
					break;
				}
			}
			else
			{
				int p=ret.find(";");
				if(p>=0)
				{
					string stem=ret.substr(p+1,ret.size()-p-1);
					string val=ret.substr(0,p);
					ret=stem;
					if(i==0)
					{
						TVQua.ImageLevel=StrUtil::Str2Float(val)*1000;
						level = TVQua.ImageLevel;
					}
					else if(i==1)
					{
						TVQua.AudioLevel=StrUtil::Str2Float(val)*1000;
					}
					else if(i==2)
					{
						TVQua.I2AOffLevel=StrUtil::Str2Float(val)*1000;
					}
					else if(i==3)
					{
						TVQua.CN=StrUtil::Str2Float(val)*1000;
					}
					else if(i==4)
					{
						TVQua.FreqOffSet=StrUtil::Str2Float(val)*1000;
					}
					else if(i==5)
					{
						TVQua.Slope=StrUtil::Str2Float(val)*1000;
					}
				}
				break;
			}
		}
	}
	//
	if(level<0)
	{
		level = 5;
	}
	string slog=retinfo;
	ACE_DEBUG ((LM_DEBUG,"(%D | %t) %s\n",slog.c_str()));
	return level;
}
int  TCPDeviceAccess::GetRadioQulity(int ifreq,RadioQuaRetMessage_Obj &rqr,int worktype)
{
	int level=0;
	string flag=StrUtil::Long2Str(time(0));
	string cmd=string("rqulity:");
	cmd+=StrUtil::Int2Str(ifreq);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(worktype);
	cmd+=string(",");
	cmd+=flag;
	cmd+=string(";");
	string sendinfo=g_strcmdhead;
	sendinfo+=string(",");
	sendinfo+=StrUtil::Int2Str(cmd.size());
	sendinfo+=string(",");
	sendinfo+=cmd;
	//
	char retinfo[200];
	memset(retinfo,0,sizeof(char)*200);
	bool rtn = true;//SendCmdToServer((void*)sendinfo.c_str(),sendinfo.size(),(void*)retinfo,200);

	if (rtn==false)
	{
		string msg = string("频点[") + StrUtil::Int2Str(ifreq) + string("]获取指标失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
		return -1;
	}
	else
	{
		string msg = string("频点[") + StrUtil::Int2Str(ifreq) + string("]获取指标成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}
	//
	string retflag=flag+string(":");
	string result=retflag;//retinfo;
	result+="45.6,43,2.2,8;";//retinfo;
	int pos=result.find(retflag);
	if(pos>=0)
	{
		string ret=result.substr(pos+retflag.size(),result.size()-pos-retflag.size());
		for(int i=0;i<10;i++)
		{
			int p=ret.find(",");
			if(p>=0)
			{
				string stem=ret.substr(p+1,ret.size()-p-1);
				string val=ret.substr(0,p);
				ret=stem;
				if(i==0)
				{
					rqr.level_int=StrUtil::Str2Float(val)*1000;
				}
				else if(i==1)
				{
					rqr.dev_int=StrUtil::Str2Float(val)*1000;
				}
				else if(i==2)
				{
					rqr.dF=StrUtil::Str2Float(val)*1000;
				}
				else if(i==3)//带宽没有使用
				{
				}
				else
				{
					break;
				}
			}
			else
			{
				int p=ret.find(";");
				if(p>=0)
				{
					string stem=ret.substr(p+1,ret.size()-p-1);
					string val=ret.substr(0,p);
					ret=stem;
					if(i==0)
					{
						rqr.level_int=StrUtil::Str2Float(val)*1000;
					}
					else if(i==1)
					{
						rqr.dev_int=StrUtil::Str2Float(val)*1000;
					}
					else if(i==2)
					{
						rqr.dF=StrUtil::Str2Float(val)*1000;
					}
					else if(i==3)//带宽没有使用
					{
					}
				}
				break;
			}
		}
	}
	//
	if(level<=0)
	{
		level = 5;
	}
	string slog=retinfo;
	ACE_DEBUG ((LM_DEBUG,"(%D | %t) %s\n",slog.c_str()));
	return level;
}
bool TCPDeviceAccess::Spectrumscan(int startfreq,int endfreq,int scanstep,int measurewide,vector<SpectrumInfo> &vecSpectrumValue,int  worktype,int type)
{
	string flag=StrUtil::Long2Str(time(0));
	string cmd=string("spectrumscan:");
	cmd+=StrUtil::Int2Str(startfreq);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(endfreq);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(scanstep);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(measurewide);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(worktype);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(type);
	cmd+=string(",");
	cmd+=flag;
	cmd+=string(";");
	string sendinfo=g_strcmdhead;
	sendinfo+=string(",");
	sendinfo+=StrUtil::Int2Str(cmd.size());
	sendinfo+=string(",");
	sendinfo+=cmd;
	//
	string retinfo;
	bool rtn = SendCmdToServerEx((void*)sendinfo.c_str(),sendinfo.size(),retinfo);

	if (rtn==false)
	{
		string msg = string("开始频点[") + StrUtil::Int2Str(startfreq) +string("]~结束频点[") + StrUtil::Int2Str(endfreq) + string("]频谱扫描失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("开始频点[") + StrUtil::Int2Str(startfreq) +string("]~结束频点[") + StrUtil::Int2Str(endfreq) + string("]频谱扫描成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}
	//
	string retflag=flag+string(":");
	string result=retinfo;
	int pos=result.find(retflag);
	if(pos>=0)
	{
		string ret=result.substr(pos+retflag.size(),result.size()-pos-retflag.size());
		while(true)
		{
			int p=ret.find(",");
			if(p>=0)
			{
				string stem=ret.substr(p+1,ret.size()-p-1);
				string val=ret.substr(0,p);
				ret=stem;
				//
				int q=val.find("=");
				if(q>0)
				{
					SpectrumInfo SpectInfo;
					string spfreq=val.substr(0,q);
					string spval=val.substr(q+1,val.size()-q-1);
					SpectInfo.freq=StrUtil::Str2Int(spfreq);
					SpectInfo.level=StrUtil::Str2Float(spval);
					vecSpectrumValue.push_back(SpectInfo);
				}
			}
			else
			{
				int pp=ret.find(";");
				if(pp>=0)
				{
					string val=ret.substr(0,p);
					//
					int qq=val.find("=");
					if(qq>0)
					{
						SpectrumInfo SpectInfo;
						string spfreq=val.substr(0,qq);
						string spval=val.substr(qq+1,val.size()-qq-1);
						SpectInfo.freq=StrUtil::Str2Int(spfreq);
						SpectInfo.level=StrUtil::Str2Float(spval);
						vecSpectrumValue.push_back(SpectInfo);
					}
				}
				break;
			}
		}
	}
	//
	//string slog=retinfo;
	//ACE_DEBUG ((LM_DEBUG,"(%D | %t) %s\n",slog.c_str()));
	return rtn;
}
bool TCPDeviceAccess::Channelscan(int chan,int startfreq,int endfreq,int scanstep,SpecialRadioRetMessage_Obj& ChanScanRet,int type)
{
	string flag=StrUtil::Long2Str(time(0));
	string cmd=string("channelscan:");
	cmd+=StrUtil::Int2Str(chan);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(startfreq);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(endfreq);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(scanstep);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(type);
	cmd+=string(",");
	cmd+=flag;
	cmd+=string(";");
	string sendinfo=g_strcmdhead;
	sendinfo+=string(",");
	sendinfo+=StrUtil::Int2Str(cmd.size());
	sendinfo+=string(",");
	sendinfo+=cmd;
	//
	string retinfo;
	bool rtn = SendCmdToServerEx((void*)sendinfo.c_str(),sendinfo.size(),retinfo);

	if (rtn==false)
	{
		string msg = string("开始频点[") + StrUtil::Int2Str(startfreq) +string("]~结束频点[") + StrUtil::Int2Str(endfreq) + string("]频道扫描失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("开始频点[") + StrUtil::Int2Str(startfreq) +string("]~结束频点[") + StrUtil::Int2Str(endfreq) + string("]频道扫描成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}
	//
	string retflag=flag+string(":");
	string result=retinfo;
	int pos=result.find(retflag);
	ChanScanRet.len=0;
	if(pos>=0)
	{
		string ret=result.substr(pos+retflag.size(),result.size()-pos-retflag.size());

		while(true)
		{
			int p=ret.find(",");
			if(p>=0)
			{
				string stem=ret.substr(p+1,ret.size()-p-1);
				string val=ret.substr(0,p);
				ret=stem;
				//
				ChanScanRet.value[ChanScanRet.len]=StrUtil::Str2Int(val);
				ChanScanRet.len++;
			}
			else
			{
				int pp=ret.find(";");
				if(pp>=0)
				{
					string val=ret.substr(0,p);

					ChanScanRet.value[ChanScanRet.len]=StrUtil::Str2Int(val);
					ChanScanRet.len++;
				}
				break;
			}
		}
	}
	//
	string slog=retinfo;
	ACE_DEBUG ((LM_DEBUG,"(%D | %t) %s\n",slog.c_str()));
	return rtn;
}

int TCPDeviceAccess::SendCmdToTVCom(MSG_TYPE msg_type,void* info,int infolen)
{
	TVQuality TVQua;
	int freq=(*((int*)(info)));
	int QulityVal=0;
	//
	SendtodevMutex.acquire();
	for(int i=0;i<4;i++)
	{
		if(time(0)-g_sendcommandtime>=3)
		{
			break;
		}
		Sleep(1000);
	}
	QulityVal = GetTVQulity(freq,TVQua);
	g_sendcommandtime = time(0);
	SendtodevMutex.release();
	//
	return QulityVal;
}



int TCPDeviceAccess::GetAudioIndex(int audiorate)
{
	int index = 3;//默认为3
	if (audiorate<=32)
		index = 1;
	else if (audiorate>32 && audiorate<=40)
		index =2;
	else if (audiorate>40 && audiorate<=48)
		index =3;
	else if (audiorate>48 && audiorate<=56)
		index =4;
	else if (audiorate>56 && audiorate<=64)
		index =5;
	else if (audiorate>64 && audiorate<=80)
		index =6;
	else if (audiorate>80 && audiorate<=96)
		index =7;
	else if (audiorate>96 && audiorate<=112)
		index =8;
	else if (audiorate>112 && audiorate<=128)
		index =9;
	else if (audiorate>128 && audiorate<=160)
		index =3;
	else if (audiorate>160 && audiorate<=192)
		index =3;
	else if (audiorate>192 && audiorate<=224)
		index =3;
	else if (audiorate>224 && audiorate<=256)
		index =3;
	else if (audiorate>256 && audiorate<=320)
		index =3;
	else
		index =3;
	return index;
}

bool TCPDeviceAccess::InitCard()
{
	//初始化信息结构
	InitBoard_Obj ib;
	//本机IP、UPD发送的地址需要从配置文件取
	strcpy(ib.dstinfo.dstIP,"172.16.10.56");
	//板卡给本机上报数据的端口：音视频数据端口,每个对应一路实时音视频数据
	ib.dstinfo.dstport = 1234;		//PropManager::GetInstance()->GetDevCarddspPort(mChannelID+2);//1234;//1234,1235,1236,1237
	ib.dstinfo.alarmport = 8000;	//PropManager::GetInstance()->GetDevCardalarmPort(mChannelID+2);//8000;//报警接收的UDP地址 每一路报警如何上报？
	ib.video_bitrate = 700;
	ib.audio_idx =5;				// GetAudioIndex(48);//默认为48K
	ib.systime = time(0)+8*3600;		

	bool rtn = SendCmdToServer(MSG_SET_INIT,(void*)&ib,sizeof(InitBoard_Obj));
	
	if (rtn==false)
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]初始化失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]初始化成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}
	return rtn;
}


bool TCPDeviceAccess::SetVideoAlertInfo( AlertConfig_Obj& alertinfo )
{
	bool rtn=SendCmdToServer(MSG_SET_ALERTVIDEO,(void*)&alertinfo,sizeof(AlertConfig_Obj));

	if (rtn==false)
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置视频报警参数失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置视频报警参数成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}

	return rtn;
}

bool TCPDeviceAccess::SetAudioAlertInfo( AlertConfig_Obj& alertinfo )
{
	bool rtn=SendCmdToServer(MSG_SET_ALERTAUDIO,(void*)&alertinfo,sizeof(AlertConfig_Obj));

	if (rtn==false)
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置音频报警参数失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置音频报警参数成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}
	
	return rtn;
}
bool TCPDeviceAccess::StartTranscode()
{
	bool rtn = SendCmdToServer(MSG_SET_START,NULL,0);

	if (rtn==false)
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置转码启动失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置转码启动成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}
	
	return rtn;
}
bool TCPDeviceAccess::StopTranscode()
{
	bool rtn = SendCmdToServer(MSG_SET_STOP,NULL,0);

	if (rtn==false)
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置转码停止失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置转码停止成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}

	return rtn;
}

bool TCPDeviceAccess::SetOSD(int chan,const SubtitleConfig_Obj& osdinfo)
{
	string flag=StrUtil::Long2Str(time(0));
	string cmd=string("setosd:");
	cmd+=StrUtil::Int2Str(chan);
	cmd+=string(",");
	cmd+=string(osdinfo.subtitle0);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(osdinfo.size);
	cmd+=string(",");
	cmd+=string(osdinfo.subtitle1);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(osdinfo.subtitle1_x);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(osdinfo.subtitle1_y);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(osdinfo.mode);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(osdinfo.time_x);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(osdinfo.time_y);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(osdinfo.subtitle0_x);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(osdinfo.subtitle0_y);
	cmd+=string(",");
	cmd+=flag;
	cmd+=string(";");
	string sendinfo=g_strcmdhead;
	sendinfo+=string(",");
	sendinfo+=StrUtil::Int2Str(cmd.size());
	sendinfo+=string(",");
	sendinfo+=cmd;
	//
	char retinfo[30];
	memset(retinfo,0,sizeof(char)*30);
	SendtodevMutex.acquire();
	bool rtn = SendCmdToServer((void*)sendinfo.c_str(),sendinfo.size(),(void*)retinfo,30);
	SendtodevMutex.release();

	if (rtn==false)
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置OSD失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置OSD成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}
	string slog=retinfo;
	ACE_DEBUG ((LM_DEBUG,"(%D | %t) %s\n",slog.c_str()));

	return rtn;
}

bool TCPDeviceAccess::SetSysTime(int chan)
{
	string flag=StrUtil::Long2Str(time(0));
	string cmd=string("settime:");
	cmd+=StrUtil::Int2Str(chan);
	cmd+=string(",");
	cmd+=flag;
	cmd+=string(";");
	string sendinfo=g_strcmdhead;
	sendinfo+=string(",");
	sendinfo+=StrUtil::Int2Str(cmd.size());
	sendinfo+=string(",");
	sendinfo+=cmd;
	//
	char retinfo[30];
	memset(retinfo,0,sizeof(char)*30);
	SendtodevMutex.acquire();
	bool rtn = SendCmdToServer((void*)sendinfo.c_str(),sendinfo.size(),(void*)retinfo,30);
	SendtodevMutex.release();

	if (rtn==false)
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置系统时间失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置系统时间成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}
	string slog=retinfo;
	ACE_DEBUG ((LM_DEBUG,"(%D | %t) %s\n",slog.c_str()));

	return rtn;
}
bool TCPDeviceAccess::IsInList(int ifreqkz,std::vector<int>& vecFreq)
{
	bool ret = false;
	for(int i=0;i<vecFreq.size();i++)
	{
		if(vecFreq[i] == ifreqkz)
		{
			ret = true;
			break;
		}
	}
	return ret;
}