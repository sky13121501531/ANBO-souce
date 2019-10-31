#include "TCPDeviceAccess.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/TypeDef.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Communications/SysMsgSender.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/StrUtil.h"
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <algorithm>
#include <math.h>
string g_strcmdhead="aa";

ACE_Thread_Mutex RadioTunerdevMutex;
ACE_Thread_Mutex RadioEncoderdevMutex;


ACE_Thread_Mutex DeviceCmdMutex; //ATV
time_t g_sendcommandtime = 0;
extern time_t g_starttime;
bool   g_realqulity = false;
bool   g_atvchanscan = false;
bool   g_realspec = false;
bool   g_realstream = false;
TCPDeviceAccess::TCPDeviceAccess(void)
{
}

TCPDeviceAccess::~TCPDeviceAccess(void)
{
}
TCPDeviceAccess::TCPDeviceAccess(int deviceid,std::string strIP,int nPort):DeviceAccess(deviceid,strIP,nPort),server(nPort,strIP.c_str())
{
	mChannelID=deviceid;
	strIPAddress = strIP;
	m_tChanFixKeepTime = time(0)-1;
	tLastSpectrum = time(0)-1;
	t_m_curTunerQuality = time(0)-2;
	m_bIsFreqLock = true;
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

bool TCPDeviceAccess::CheckDeviceIDLock()
{
	bool rtn = m_bIsFreqLock;
	int pkgLen = 0;
	char sendbuf[200] = {0}; 
	VSTuner_ChanFix_Param_Obj chanFixObj;
	VSTuner_ChanFix_Result_Obj retObj;
	PkgHeader_Obj   ph;
	ph.header   = 0x48;
	ph.msg_type = MSG_GET_6U_CHANFIX;

	memcpy(sendbuf, &ph, sizeof(PkgHeader_Obj));
	pkgLen	+= sizeof(PkgHeader_Obj);

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,chanFixObj.chanNum);	//获得该通道对应的Tuner
	chanFixObj.TunerType = TUNER_TYPE_TV;
	
	//信息内容
	memcpy(sendbuf+pkgLen, &chanFixObj, sizeof(VSTuner_ChanFix_Param_Obj));
	pkgLen	+= sizeof(VSTuner_ChanFix_Param_Obj);

	if((time(0)-m_tChanFixKeepTime)>=1) //默认1秒内不重复检测
	{
		string sysmsg;
		if(1 != SendCmdToServer((void*)&sendbuf, 200, (void*)&retObj, sizeof(VSTuner_ChanFix_Result_Obj)))
		{
			sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(DeviceId) + string("]获取频道锁定状态返回失败");
			SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);
			rtn = false;
		}
		else
		{
			if(retObj.status != 1 || retObj.ChanFixObj.status != 65535)
				rtn = false;
			else
				rtn = true;
		}
		m_tChanFixKeepTime = time(0);	
		m_bIsFreqLock  = rtn;
		if (rtn == false)
		{
			string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + std::string("]锁定失败"); 
			SYSMSGSENDER::instance()->SendMsg(msg,ATV,VS_MSG_PROALARM);
		}
		else
		{
			string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + std::string("]锁定成功"); 
			SYSMSGSENDER::instance()->SendMsg(msg);
		}
	}
	return rtn;
}
#if 0
bool TCPDeviceAccess::GetRadioTunerQulity(VSTuner_Quality_Result_Obj& radiotunQ)
{
	int pkgLen = 0;
	char sendbuf[1024] = {0}; 
	VSTuner_Quality_Param obj;

	if(time(0)<t_m_curTunerQuality+2)
	{
		radiotunQ = m_curTunerQuality;
		return true;
	}

	//信息头
	PkgHeader_Obj   ph;
	ph.header   = 0x48;
	ph.msg_type = MSG_GET_6U_QUALITY;

	memcpy(sendbuf, &ph, sizeof(PkgHeader_Obj));
	pkgLen	+= sizeof(PkgHeader_Obj);

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,obj.chanNum);
	obj.TunerType = TUNER_TYPE_RADIO;

	//信息内容
	memcpy(sendbuf+pkgLen, &obj, sizeof(VSTuner_Quality_Param));
	pkgLen	+= sizeof(VSTuner_Quality_Param);


	string sysmsg;

	if(1 != SendCmdToServer((void*)&sendbuf, 1024, (void*)&radiotunQ, sizeof(VSTuner_Quality_Result_Obj)))
// 	{
// 		//memcpy((void)retObj,&scanResultObj,sizeof(VSScan_Result_Obj));
// 		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]指标返回成功");
// 		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
// 		return true;
// 	}
// 	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]指标返回失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
		return false;
	}
	m_curTunerQuality = radiotunQ;
	t_m_curTunerQuality = time(0);
	return true;
}
#endif
bool TCPDeviceAccess::ConnectToServer()
{
	//建立连接
	if (connector.connect (stream, server) == -1)//连接硬件服务器
	{
		string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]连接失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
		Sleep(300);
		stream.close();
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

	ACE_Guard<ACE_Thread_Mutex> guard(SendtodevMutex);
	int pkgLen = 0;
	unsigned char sendbuf[1024] = {0}; 
	unsigned char RetMsg[1024] = {0};
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
		if(stream.send((char*)sendbuf, 1024,&timeout) <= 0)
		{
			string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]发送指令失败");
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
			Sleep(1000);
			continue;
		}
		//接收返回数据
		if(stream.recv(( char *)&RetMsg,sizeof(RetMessage_Obj),&timeout) <= 0)
		{
			string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]数据接收失败");
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
			Sleep(1000);
			continue;
		}
		ret = true;

		break;
	}
// 	stream.close_writer();
//	stream.close_reader();
	stream.close();
	return ret;
}

bool TCPDeviceAccess::SendCmdToServer(void* indata,int indatalen,void* outdata,int outdatalen)
{
	ACE_Guard<ACE_Thread_Mutex> guard(SendtodevMutex);
	bool ret = false;
	ACE_Time_Value timeout(60);

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
			ACE_DEBUG((LM_DEBUG,"(%T | %t)发送板卡指令失败\n"));
			continue;
		}
		//接收返回数据
		int revlen=stream.recv(( char *)outdata,outdatalen,&timeout);
		if(revlen <= 0)
		{
			ACE_DEBUG((LM_DEBUG,"(%T | %t)接收板卡指令返回失败:%s\n",indata));			
			continue;
		}
		break;
	}
	if(i<3)
	{
		ret=true;
	}
//	stream.close_writer();
//	stream.close_reader();
	stream.close();
	return ret;
}

bool TCPDeviceAccess::SendCmdToServerEx(void* indata,int indatalen,string& outdata)	
{
	ACE_Guard<ACE_Thread_Mutex> guard(SendtodevMutex);
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
			ACE_DEBUG((LM_DEBUG,"(%T | %t)发送板卡指令失败\n"));
			continue;
		}
		//接收返回数据
		int revlen=0;
		int i=0;
		int j=0;
		for(;i<100;i++)
		{
			char outinfo[8196];
			memset(outinfo,0,sizeof(char)*8196);
			revlen=stream.recv(( char *)outinfo,8196,&timeout);
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
			ACE_DEBUG((LM_DEBUG,"(%T | %t)接收板卡指令返回失败Ex\n"));			
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
	TVQua.freq = ifreq;
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
				int freq = 0;
				if(i==0)
				{
					freq=StrUtil::Str2Int(val);
					if(freq != ifreq)
					{
						rtn=false;
						break;
					}
				}
				else if(i==1)
				{
					TVQua.ImageLevel=StrUtil::Str2Float(val)*1000;
					if(ifreq>3000&&ifreq<5000)
						level = TVQua.ImageLevel;
					else if(ifreq<3000)
						level = TVQua.ImageLevel;
					else if(ifreq>5000)
						level = TVQua.ImageLevel;
					else 
						level = TVQua.ImageLevel;
				}
				else if(i==2)
				{
					TVQua.AudioLevel=StrUtil::Str2Float(val)*1000;
				}
				else if(i==3)
				{
					TVQua.I2AOffLevel=StrUtil::Str2Float(val)*1000;
				}
				else if(i==4)
				{
					TVQua.CN=StrUtil::Str2Float(val)*1000;
				}
				else if(i==5)
				{
					TVQua.FreqOffSet=StrUtil::Str2Float(val);
				}
				else if(i==6)
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
						TVQua.FreqOffSet=StrUtil::Str2Float(val);
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)%s\n",slog.c_str()));
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
	rqr.freq = ifreq;

	if(pos>=0)
	{
		string ret=result.substr(pos+retflag.size(),result.size()-pos-retflag.size());
		for(int i=0;i<16;i++)
		{
			int p=ret.find(",");
			if(p>=0)
			{
				string stem=ret.substr(p+1,ret.size()-p-1);
				string val=ret.substr(0,p);
				ret=stem;
				int freq = 0;
				if(i==0)
				{
					freq=StrUtil::Str2Int(val);
					if(freq != ifreq)
					{
						rtn=false;
						break;
					}
				}
				if(i==1)
				{
					rqr.level_int=StrUtil::Str2Float(val)*1000;
				}
				else if(i==2)
				{
					rqr.dev_int=StrUtil::Str2Float(val)*1000;
				}
				else if(i==3)
				{
					rqr.dF=StrUtil::Str2Float(val)*1000;
				}
				else if(i==4)
				{
					rqr.OBW = StrUtil::Str2Float(val)*1000;
				}
				else if(i==5)
				{
					rqr.CNR = StrUtil::Str2Float(val)*1000;
				}
				else if(i==6)
				{
					rqr.FM_Dev = StrUtil::Str2Float(val)*1000;
				}
				else if(i==7)
				{
					rqr.FM_Dev_Pos = StrUtil::Str2Float(val)*1000;
				}
				else if(i==8)
				{
					rqr.FM_Dev_Neg = StrUtil::Str2Float(val)*1000;
				}
				else if(i==9)
				{
					rqr.Dev_75kHz_Prob= StrUtil::Str2Float(val)*1000;
				}
				else if(i==10)
				{
					rqr.Dev_80kHz_Prob = StrUtil::Str2Float(val)*1000;
				}
				else if(i==11)
				{
					rqr.Dev_85kHz_Prob = StrUtil::Str2Float(val)*1000;
				}
				else if(i==12)
				{
					rqr.Audio_Freq = StrUtil::Str2Float(val)*1000;
				}
				else if(i==13)
				{
					rqr.Audio_THD = StrUtil::Str2Float(val)*1000;
				}
				else if(i==14)
				{
					rqr.Audio_SINAD = StrUtil::Str2Float(val)*1000;
				}
				else if(i ==15 )
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)%s\n",slog.c_str()));
	return level;
}

bool TCPDeviceAccess::Spectrumscan(int startfreq,int endfreq,int scanstep,int measurewide,vector<SpectrumInfo> &vecSpectrumValue,int  worktype,int type)
{
	if((scanstep == 100)&&(type == 0))  //广播手动设置频道扫描开始结束频率特殊处理,均按标准频谱扫瞄,在频道扫描处过滤
	{
		startfreq = 87000;
		endfreq = 108000;
	}
	if(scanstep == 100)
		scanstep = 50;
	if((time(0)<tLastSpectrum+1)&&(LastStartFreq == startfreq)&&(LastEndFreq == endfreq)&&(LastScanStep == scanstep)) 
	{	
		std::vector<SpectrumInfo>::iterator ptrr=mLastSpectrum.begin();
		for(;ptrr!=mLastSpectrum.end();ptrr++)
		{
			vecSpectrumValue.push_back(*ptrr);	
		}
		return true;
	}
	if((/*(g_atvchanscan)||*/(g_realqulity))&&mLastSpectrum.size()>10)
	{//如果有其它调用vs300的任务，频谱需保持
		std::vector<SpectrumInfo>::iterator ptrr=mLastSpectrum.begin();
		for(;ptrr!=mLastSpectrum.end();ptrr++)
		{
			vecSpectrumValue.push_back(*ptrr);	
		}
		return true;
	}

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

	if(scanstep == 50)
		scanstep = 100;

	tLastSpectrum = time(0);
	mLastSpectrum.clear();
//	bool rtn = SendCmdToServer(MSG_SET_VIDEOTUNER,(void*)&tunerinfo,sizeof(TunerConfig_Obj));
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
	int tempFreq = startfreq-scanstep;
	
	string LOGRetSpec("");

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

					if((SpectInfo.freq>=(tempFreq+scanstep))&&SpectInfo.freq>=(startfreq)&&SpectInfo.freq<=(endfreq)&&(int)(SpectInfo.freq)%25<1)
					{
						tempFreq = SpectInfo.freq;
						SpectInfo.level=StrUtil::Str2Float(spval);
						vecSpectrumValue.push_back(SpectInfo);
						mLastSpectrum.push_back(SpectInfo);

						LOGRetSpec.append(StrUtil::Int2Str(SpectInfo.freq));
						LOGRetSpec.append("=");
						LOGRetSpec.append(StrUtil::Int2Str(SpectInfo.level));
						LOGRetSpec.append(";");
					}
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
						if(SpectInfo.freq>=(startfreq)&&SpectInfo.freq<=(endfreq)&&(SpectInfo.freq>=(tempFreq+scanstep)))
						{
							tempFreq=SpectInfo.freq;
							vecSpectrumValue.push_back(SpectInfo);
							mLastSpectrum.push_back(SpectInfo);

							LOGRetSpec.append(StrUtil::Int2Str(SpectInfo.freq));
							LOGRetSpec.append("=");
							LOGRetSpec.append(StrUtil::Int2Str(SpectInfo.level));
							LOGRetSpec.append(";");
						}
					}
				}
				break;
			}
		}
	}
// 	if((15900<((time(0)+28800)%86400) && ((time(0)+28800)%86400)<16800) || (17100<((time(0)+28800)%86400) && ((time(0)+28800)%86400)<18300) || (3300<((time(0)+28800)%86400) && ((time(0)+28800)%86400)<4500) || (6900<((time(0)+28800)%86400) && ((time(0)+28800)%86400)<8100))
// 	{
// 		APPLOG::instance()->WriteLog(ALARM,LOG_EVENT_DEBUG,LOGRetSpec,LOG_OUTPUT_FILE);//Edit by ZN2018-05-24
// 	}
	
// 	 time_t now = time(0);
// 	 struct tm *cur_t = localtime(&now);
// 		
// 	 if((cur_t->tm_wday == 2) && (46800<((time(0)+28800)%86400)) && (((time(0)+28800)%86400)<61200))
// 	 {
// 		APPLOG::instance()->WriteLog(ALARM,LOG_EVENT_DEBUG,LOGRetSpec,LOG_OUTPUT_FILE);//Edit by ZN2018-05-24
// 	 }


	// APPLOG::instance()->WriteLog(ALARM,LOG_EVENT_DEBUG,LOGRetSpec,LOG_OUTPUT_FILE);
	//
	//string slog=retinfo;
	//ACE_DEBUG ((LM_DEBUG,"(%T | %t)%s\n",slog.c_str()));
	LastStartFreq = startfreq;
	LastEndFreq = endfreq;
	LastScanStep = scanstep;
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)%s\n",slog.c_str()));
	return rtn;
}

int TCPDeviceAccess::SendCmdToTVCom(MSG_TYPE msg_type,void* info,int infolen)
{
	TVQuality TVQua;
	int freq=(*((int*)(info)));
	int QulityVal=0;
	//
	//检查节目报警
	// ACE_Guard<ACE_Thread_Mutex> guard(DeviceCmdMutex);
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
	ACE_Guard<ACE_Thread_Mutex> guard(DeviceCmdMutex);

	// cout<<"\n21111111111111111111111111111111="<<DeviceId<<endl;
	bool rtn = false;
	switch(chan)
	{
	case 0:
		rtn = SendCmdToServer(MSG_SET_OSD0,(void*)&osdinfo,sizeof(SubtitleConfig_Obj));
		break;
	case 1:
		rtn = SendCmdToServer(MSG_SET_OSD1,(void*)&osdinfo,sizeof(SubtitleConfig_Obj));
		break;
	case 2:
		rtn = SendCmdToServer(MSG_SET_OSD2,(void*)&osdinfo,sizeof(SubtitleConfig_Obj));
		break;
	case 3:
		rtn = SendCmdToServer(MSG_SET_OSD3,(void*)&osdinfo,sizeof(SubtitleConfig_Obj));
		break;
	default:
		rtn = false;
		break;
	}

	if (rtn==false)
	{
		string msg = string("通道[") + strIPAddress +string(":") + StrUtil::Int2Str(mChannelID) + string("]设置OSD失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("通道[") + strIPAddress +string(":") + StrUtil::Int2Str(mChannelID) + string("]设置OSD成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}

	return rtn;
}

bool TCPDeviceAccess::SetSysTime(int chan)
{
	time_t now = time(0)+8*3600;
	//cout<<"\n31111111111111111111111111111111="<<DeviceId<<endl;
	bool rtn = SendCmdToServer(MSG_SET_TIME,(void*)&now,sizeof(time_t));

	if (rtn==false)
	{
		string msg = string("通道[") + strIPAddress +string(":") + StrUtil::Int2Str(mChannelID) + string("]设置系统时间失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		string msg = string("通道[") + strIPAddress +string(":") + StrUtil::Int2Str(mChannelID) + string("]设置系统时间成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}

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