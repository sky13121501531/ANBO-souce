
#include "./RADIOTcpDeviceAccess.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/AppLog.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Communications/SysMsgSender.h"
#include"../DBAccess/DBManager.h"
#include "../BusinessProcess/AlarmParamInfoMgr.h"
extern string g_strcmdhead;
extern ACE_Thread_Mutex RadioTunerdevMutex;
extern ACE_Thread_Mutex RadioEncoderdevMutex;
extern time_t g_sendcommandtime;

bool comparison(SpectrumInfo a,SpectrumInfo b);

RadioTcpDeviceAccess::RadioTcpDeviceAccess(int deviceid,std::string strIP,int nPort) : TCPDeviceAccess(deviceid, strIP, nPort)
{	
	setCardSystemTimeFor6U();
	setThresholdInfo();
}

#if 0
bool RadioTcpDeviceAccess::GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	vector<QualityDesc> vecDesc;
	char* source="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg></Msg>";
	XmlParser Retparser(source);
	float freq,f_value=0.0f;
	string StrFreq;
	pXMLNODE QuaryNode = parser.GetNodeFirstChild(root);
	parser.GetAttrNode(QuaryNode,"Freq",StrFreq);
	pXMLNODE SrcparamNode = parser.GetNodeFromPath("Msg/QualityParam");
	pXMLNODELIST paramList = parser.GetNodeList(SrcparamNode);
	freq =StrUtil::Str2Float(StrFreq);
	RadioQuaRetMessage_Obj rqr;
	bool bRadioQua=false;
	int ff[7]={0,0,0,0,0,0,0};
	int count = paramList->Size();
	for(int k=0;k<count;k++)
	{
		std::string Type,Desc,Value;
		QualityDesc TempDesc;
		pXMLNODE childNode = parser.GetNextNode(paramList);
		parser.GetAttrNode(childNode,"Type",Type);
		parser.GetAttrNode(childNode,"Desc",Desc);
		parser.GetAttrNode(childNode,"Value",Value);
		int i_type=StrUtil::Str2Int(Type);

		if(dvbtype=="RADIO")
		{
			string scantype=PROPMANAGER::instance()->GetScanType(RADIO);
			if(scantype=="2")//针对陕西项目作的特殊处理，从开路电视指标卡获得广播的电平值
			{
				int temfreq = (int)(freq*1000.0);
				int level = SendCmdToTVCom(MSG_GET_QUA,(void *)&temfreq,sizeof(int));
				if(level>0)
				{
					f_value=level*10;
				}
				else
				{
					f_value=0.0f;
				}

			}
			else
			{
				if(!bRadioQua)
				{
					if(GetQuality(freq,rqr))
						bRadioQua=true;
				}	
				if(bRadioQua)
				{
					switch(i_type)
					{
					case 1:
						f_value=rqr.level_int;		//信号电平
						if(f_value<6)
							f_value=0.0f;
						break;
					case 2:
						f_value = rqr.dev_int;		//调制度
						break;
					case 3:
						f_value=(rand()%(5500-4300))+4300+1; //音频信号电平，随机值，造假
						break;
					case 4:
						f_value=rqr.Audio_Freq; //信号载波频率
						break;
					case 5:
						f_value=rqr.Audio_THD; //音频谐波失真
						break;
					case 6:
						f_value=rqr.CNR; // 信噪比
						break;
					}
				}
				else
				{
					f_value =0.0f;
				}
			}
		}
		else
		{
			return false;
		}
		ff[k]=(int)f_value;
		TempDesc.Desc=Desc;
		TempDesc.Type=Type;
		TempDesc.Value=StrUtil::Float2Str(f_value);
		vecDesc.push_back(TempDesc);
	}
	pXMLNODE retrootNode=Retparser.GetRootNode();
	Retparser.SetAttrNode("DVBType",dvbtype,retrootNode);

	pXMLNODE retNode=Retparser.CreateNodePtr(retrootNode,"Return");
	Retparser.SetAttrNode("Type",string("QualityQuery"),retNode);
	Retparser.SetAttrNode("Value",string("0"),retNode);
	Retparser.SetAttrNode("Desc",string("成功"),retNode);
	Retparser.SetAttrNode("Comment",string(""),retNode);

	pXMLNODE reportNode=Retparser.CreateNodePtr(retrootNode,"QualityQueryReport");

	Retparser.SetAttrNode("STD",string(""),reportNode);
	Retparser.SetAttrNode("Freq",StrUtil::Float2Str1(freq),reportNode);

	Retparser.SetAttrNode("SymbolRate",string(""),reportNode);

	pXMLNODE paramNode=Retparser.CreateNodePtr(reportNode,"QualityParam");
	for(int i=0;i<count;i++)
	{
		pXMLNODE indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
		Retparser.SetAttrNode("Type",vecDesc[i].Type,indexNode);
		Retparser.SetAttrNode("Desc",vecDesc[i].Desc,indexNode);
		Retparser.SetAttrNode("Value",vecDesc[i].Value,indexNode);
	}

	Retparser.SaveToString(strRetMsg);
	SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
	return true;
}
#endif

bool RadioTcpDeviceAccess::GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	vector<QualityDesc> vecDesc;
	char* source="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg></Msg>";
	XmlParser Retparser(source);
	float freq,f_value=0.0f;
	string StrFreq;
	pXMLNODE QuaryNode = parser.GetNodeFirstChild(root);
	parser.GetAttrNode(QuaryNode,"Freq",StrFreq);
	pXMLNODE SrcparamNode = parser.GetNodeFromPath("Msg/QualityParam");
	pXMLNODELIST paramList = parser.GetNodeList(SrcparamNode);
	freq =StrUtil::Str2Float(StrFreq);
	RadioQuaRetMessage_Obj rqr;
	bool bRadioQua=false;
	int ff[7]={0,0,0,0,0,0,0};
	int count = paramList->Size();
	for(int k=0;k<count;k++)
	{
		std::string Type,Desc,Value;
		QualityDesc TempDesc;
		pXMLNODE childNode = parser.GetNextNode(paramList);
		parser.GetAttrNode(childNode,"Type",Type);
		parser.GetAttrNode(childNode,"Desc",Desc);
		parser.GetAttrNode(childNode,"Value",Value);
		int i_type=StrUtil::Str2Int(Type);

		if(dvbtype=="RADIO")
		{
			string scantype=PROPMANAGER::instance()->GetScanType(RADIO);
			if(scantype=="2")//针对陕西项目作的特殊处理，从开路电视指标卡获得广播的电平值
			{
				int temfreq = (int)(freq*1000.0);
				int level = SendCmdToTVCom(MSG_GET_QUA,(void *)&temfreq,sizeof(int));
				if(level>0)
				{
					f_value=level*10;
				}
				else
				{
					f_value=0.0f;
				}

			}
			else
			{
				if(!bRadioQua)
				{
					if(GetQuality(freq,rqr))
						bRadioQua=true;
				}	
				if(bRadioQua)
				{
					switch(i_type)
					{
					case 1:
						f_value=rqr.level_int;		//信号电平
						if(f_value<6)
							f_value=0.0f;
						break;
					case 2:
						f_value = rqr.dev_int;		//调制度
						break;
					case 3:
						f_value=(rand()%(5500-4300))+4300+1; //音频信号电平，随机值，造假
						break;
					case 4:
						f_value=rqr.Audio_Freq; //信号载波频率
						break;
					case 5:
						f_value=rqr.Audio_THD; //音频谐波失真
						break;
					case 6:
						f_value=rqr.CNR; // 信噪比
						break;
					}
				}
				else
				{
					f_value =0.0f;
				}
			}
		}
		else
		{
			return false;
		}
		ff[k]=(int)f_value;
		TempDesc.Desc=Desc;
		TempDesc.Type=Type;
		TempDesc.Value=StrUtil::Float2Str(f_value);
		vecDesc.push_back(TempDesc);
	}
	pXMLNODE retrootNode=Retparser.GetRootNode();
	Retparser.SetAttrNode("DVBType",dvbtype,retrootNode);

	pXMLNODE retNode=Retparser.CreateNodePtr(retrootNode,"Return");
	Retparser.SetAttrNode("Type",string("QualityQuery"),retNode);
	Retparser.SetAttrNode("Value",string("0"),retNode);
	Retparser.SetAttrNode("Desc",string("成功"),retNode);
	Retparser.SetAttrNode("Comment",string(""),retNode);

	pXMLNODE reportNode=Retparser.CreateNodePtr(retrootNode,"QualityQueryReport");

	Retparser.SetAttrNode("STD",string(""),reportNode);
	Retparser.SetAttrNode("Freq",StrUtil::Float2Str1(freq),reportNode);

	Retparser.SetAttrNode("SymbolRate",string(""),reportNode);

	pXMLNODE paramNode=Retparser.CreateNodePtr(reportNode,"QualityParam");
	for(int i=0;i<count;i++)
	{
		pXMLNODE indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
		Retparser.SetAttrNode("Type",vecDesc[i].Type,indexNode);
		Retparser.SetAttrNode("Desc",vecDesc[i].Desc,indexNode);
		Retparser.SetAttrNode("Value",vecDesc[i].Value,indexNode);
	}

	Retparser.SaveToString(strRetMsg);
	SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
	return true;
}



bool RadioTcpDeviceAccess::GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	vector<SpectrumInfo> SpectrumVec;
	std::string StartFreq,EndFreq,StepFreq,VBW;
	pXMLNODE QueryNode = parser.GetNodeFromPath("Msg/SpectrumQuery");
	parser.GetAttrNode(QueryNode,"EndFreq",EndFreq);
	parser.GetAttrNode(QueryNode,"StartFreq",StartFreq);
	parser.GetAttrNode(QueryNode,"StepFreq",StepFreq);
	parser.GetAttrNode(QueryNode,"VBW",VBW);
	//
	for(int i=0;i<4;i++)
	{
		if(time(0)-g_sendcommandtime>=3)
		{
			break;
		}
		Sleep(1000);
	}
	int startfreq = (StrUtil::Str2Float(StartFreq)+0.0002)*1000;
	int endfreq = (StrUtil::Str2Float(EndFreq)+0.0002)*1000;
	int scanstep = (StrUtil::Str2Float(StepFreq)+0.0002)*1000;
	int num = (endfreq - startfreq)/scanstep;
	for(int sp=0;sp<3;sp++)
	{
		SpectrumVec.clear();
		Spectrumscan(startfreq,endfreq,scanstep,int(StrUtil::Str2Float(VBW)+0.0002),SpectrumVec);

		if((SpectrumVec.size()<=(num+10))&&(SpectrumVec.size()>=1))
		{
			break;
		}
		else
		{
			Sleep(1000);
			continue;
		}
	}
	g_sendcommandtime = time(0);
	char* source="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg></Msg>";
	XmlParser Retparser(source);

	pXMLNODE retrootNode=Retparser.GetRootNode();
	Retparser.SetAttrNode("DVBType",dvbtype,retrootNode);

	pXMLNODE retNode=Retparser.CreateNodePtr(retrootNode,"Return");
	Retparser.SetAttrNode("Type",string("SpectrumQuery"),retNode);
	Retparser.SetAttrNode("Value",string("0"),retNode);
	Retparser.SetAttrNode("Desc",string("成功"),retNode);
	Retparser.SetAttrNode("Comment",string(""),retNode);

	pXMLNODE reportNode=Retparser.CreateNodePtr(retrootNode,"SpectrumQueryReport");

	Retparser.SetAttrNode("STD",string(""),reportNode);
	Retparser.SetAttrNode("SymbolRate",string(""),reportNode);

	pXMLNODE paramNode=Retparser.CreateNodePtr(reportNode,"SpectrumParam");

	for(int k=0;k<SpectrumVec.size();k++)
	{
		pXMLNODE indexNode=Retparser.CreateNodePtr(paramNode,"SpectrumIndex");
		Retparser.SetAttrNode("Freq",StrUtil::Float2Str(SpectrumVec[k].freq),indexNode);
		Retparser.SetAttrNode("Value",StrUtil::Float2Str(SpectrumVec[k].level),indexNode);
	}

	Retparser.SaveToString(strRetMsg);
	//SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
	return true;
}



bool RadioTcpDeviceAccess::GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
/*
//这里是用板卡做的频道扫描，下面 #if 0注释掉的是用接收机做的
	string retRADIOxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					   <Msg DVBType=\"RADIO\" >\
					   <Return Type=\"\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					   <ChannelScan></ChannelScan></Msg>";
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	XmlParser retpaser(retRADIOxml.c_str());
	pXMLNODE channscannode=parser.GetNodeFirstChild(childnode);
	string startfreq,endfreq,strstep;
	parser.GetAttrNode(channscannode,"StartFreq",startfreq);
	parser.GetAttrNode(channscannode,"EndFreq",endfreq);
	parser.GetAttrNode(channscannode,"StepFreq",strstep);

	VSScan_Result_Obj scanRetObj;

	setScanControlParam();
	if(!Channelscan(startfreq, endfreq, strstep, scanRetObj))
	{
		strRetMsg="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
				  <Msg DVBType=\"RADIO\" >\
				  <Return Type=\"\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
				  <ChannelScan></ChannelScan></Msg>";
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}

	vector<int> ErrnewFrlis;
	PROPMANAGER::instance()->SeparateStrVec(PROPMANAGER::instance()->GetNewFrAlarmErrNewFreq(),ErrnewFrlis);
	vector<int> OknewFrlis;
	PROPMANAGER::instance()->SeparateStrVec(PROPMANAGER::instance()->GetNewFrAlarmOkNewFreq(),OknewFrlis);


	cout << "ErrnewFrlis" << ErrnewFrlis.size() << endl;
	cout << "OknewFrlis" << OknewFrlis.size() << endl;

	cout << "scanRetObj.resultObj.ValuedNum" << scanRetObj.resultObj.ValuedNum << endl;


	float ResultFreq = 0.0;
	if(scanRetObj.resultObj.ValuedNum>0)
	{
		pXMLNODE retchannode=retpaser.GetNodeFromPath("Msg/ChannelScan");
		for(int k=0;k<scanRetObj.resultObj.ValuedNum;k++)
		{
			if(!(IsInList(scanRetObj.resultObj.freqArr[k],ErrnewFrlis)))
			{
				ResultFreq=((float)(scanRetObj.resultObj.freqArr[k]))/1000;
				string tmpResult=StrUtil::Float2Str(ResultFreq);
				if(abs(ResultFreq-108)<0.02)continue;
				pXMLNODE freqnode=retpaser.CreateNodePtr(retchannode,"Channel");
				retpaser.SetAttrNode("Freq",tmpResult,freqnode);
			}
		}
		for(int i=0;i<OknewFrlis.size();i++)
		{
			if(OknewFrlis[i]<87000||OknewFrlis[i]>108000)
			{
				string tmpResult=StrUtil::Float2Str(((float)(OknewFrlis[i]))/1000);
				pXMLNODE freqnode=retpaser.CreateNodePtr(retchannode,"Channel");
				retpaser.SetAttrNode("Freq",tmpResult,freqnode);
			}
		}
		retpaser.SaveToString(strRetMsg);
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
	}
	return true;
*/
#if 1
	string retRADIOxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					   <Msg DVBType=\"RADIO\" >\
					   <Return Type=\"\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					   <ChannelScan></ChannelScan></Msg>";
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);
	vector<int> ErrnewFrlis;
	PROPMANAGER::instance()->SeparateStrVec(PROPMANAGER::instance()->GetNewFrAlarmErrNewFreq(),ErrnewFrlis);
	vector<int> OknewFrlis;
	PROPMANAGER::instance()->SeparateStrVec(PROPMANAGER::instance()->GetNewFrAlarmOkNewFreq(),OknewFrlis);

	if(dvbtype=="RADIO")
	{
		XmlParser retpaser(retRADIOxml.c_str());
		pXMLNODE channscannode=parser.GetNodeFirstChild(childnode);
		string strstartfreq,strendfreq,strstep;
		parser.GetAttrNode(channscannode,"StartFreq",strstartfreq);
		parser.GetAttrNode(channscannode,"EndFreq",strendfreq);
		parser.GetAttrNode(channscannode,"StepFreq",strstep);

		float ResultFreq=0.0;
		int centerfreqs[40]={0};
		int pFreq[300]={0};
		RadioSpecRetMessage_Obj SpecRetObj[40];
		int len=0,lenth=0;
		int startfreq=(StrUtil::Str2Float(strstartfreq)+0.0002)*1000,endfreq=(StrUtil::Str2Float(strendfreq)+0.0002)*1000,
			freqstep=(StrUtil::Str2Float(strstep)+0.0002)*1000;

		string scantype=PROPMANAGER::instance()->GetScanType(RADIO);
		SpecialRadiopayload_Obj obj;
		SpecialRadioRetMessage_Obj	Retobj;
		obj.startfreq = startfreq;
		obj.endfreq   = endfreq;
		obj.step = freqstep;
		PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,obj.chan);
		string strRetMsg1;
		// //此处可以考虑用芯片做频道扫描
		//VSScan_Result_Obj scanRetObj;
		//
		//setScanControlParam();
		//if(!Channelscan(startfreq, endfreq, strstep, scanRetObj))
		//{
		//	strRetMsg="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
		//			  <Msg DVBType=\"RADIO\" >\
		//			  <Return Type=\"\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
		//			  <ChannelScan></ChannelScan></Msg>";
		//	SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		//	return false;
		//}

	
		strRetMsg1="";
		if(!(ChannelscanEx(obj.startfreq,obj.endfreq,obj.step,Retobj)))
		{
			strRetMsg="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"RADIO\" >\
					  <Return Type=\"\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
					  <ChannelScan></ChannelScan></Msg>";
			SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
			return false;
		}
		else
		{
			if (Retobj.len == 0)
			{
				retRADIOxml = "";
				return true;
			}
			pXMLNODE retchannode=retpaser.GetNodeFromPath("Msg/ChannelScan");
			for(int k=0;k<Retobj.len;k++)
			{
				if(!(IsInList(Retobj.value[k],ErrnewFrlis)))
				{
					ResultFreq=((float)(Retobj.value[k]))/1000;
					string tmpResult=StrUtil::Float2Str(ResultFreq);

					pXMLNODE freqnode=retpaser.CreateNodePtr(retchannode,"Channel");
					retpaser.SetAttrNode("Freq",tmpResult,freqnode);
				}
			}
			for(int i=0;i<OknewFrlis.size();i++)
			{
				if(OknewFrlis[i]>=80000&&OknewFrlis[i]<=108000)
				{
					string tmpResult=StrUtil::Float2Str(((float)(OknewFrlis[i]))/1000);
					pXMLNODE freqnode=retpaser.CreateNodePtr(retchannode,"Channel");
					retpaser.SetAttrNode("Freq",tmpResult,freqnode);
				}
			}

			retpaser.SaveToString(strRetMsg);
			cout<<"\n接收机扫描结果："<<strRetMsg<<endl;
			SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
			return true;
		}
	}
	else
	{
		return false;
	}
	return true;
#endif
}


bool RadioTcpDeviceAccess::GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"\" >\
					  <Return Type=\"TSQuery\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \</Msg>";
	string retfailxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"\" >\
					  <Return Type=\"TSQuery\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \</Msg>";
	bool  ret = true;
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	pXMLNODE tsnode=parser.GetNodeFromPath("Msg/TSQuery/TS");
	string freq,bps, width, height;
	parser.GetAttrNode(tsnode,"Bps",bps);
	parser.GetAttrNode(tsnode,"Freq",freq);

	string name;
	CHANNELINFOMGR::instance()->GetProNameByFreq(OSFunction::GetEnumDVBType(dvbtype),freq,name);
	SubtitleConfig_Obj osdinfo;

	if ((name.empty() && name.length()< 39))
	{
		name = string("未知频道") + freq;
	}
	int tuneFreq = StrUtil::Str2Float(freq)*1000;
	//调频->设置转码->设置台标OSD->设置时间OSD
	//这里是不是需要添加启用背景图片
// 	if(!SetOSDFor6U(dvbtype, name))
// 		ret = false;
// 	if(!SetTimeOSDFor6U(dvbtype, 1))
// 		ret = false;
// 	if(!TurnFreqFor6U(tuneFreq))
// 		ret = false;
// 	if(!SetEncodeFor6U(bps))
// 		ret = false;

	ret = changeChan(dvbtype, tuneFreq, bps, name, 1, 1, 0);
	if(ret)
	{
		strRetMsg=retsuccxml;
	}
	else
	{
		strRetMsg=retfailxml;
	}
	return ret;
}


int RadioTcpDeviceAccess::SendCmdForRadioSpecialChannelScan(MSG_TYPE msg_type,void* info,SpecialRadioRetMessage_Handle pRetObj, int infolen)
{
	if ( ConnectToServer() == false)
		return -1;
	int pkgLen = 0;
	unsigned char sendbuf[200] = {0}; 
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

	string sysmsg;

	ACE_Time_Value sendtimeout(10);
	if(stream.send((char*)sendbuf, 200,&sendtimeout)<=0)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]发送指令失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);

		stream.close();
		return -1;
	}

	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]发送指令成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);

	int len = -1;

	ACE_Time_Value TimeOut(60);

	len =stream.recv((char*)&RetMsg,sizeof(RetMsg),&TimeOut);

	if(len<=0)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]数据接收失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);

		stream.close();
		OSFunction::Sleep(0,500);
		return -1;
	}

	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]数据接收成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);

	SpecialRadioRetMessage_Handle rm = (SpecialRadioRetMessage_Handle)(RetMsg);

	stream.close();
	if(rm->ph.header!=0x49 || rm->ph.msg_type!=(msg_type+0x100) || rm->status != 1)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]返回数据错误");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);

		return -4;
	}
	pRetObj->ph=rm->ph;
	pRetObj->status=rm->status;
	pRetObj->len = rm->len;
	for(int i=0;i<(pRetObj->len);i++)
	{
		pRetObj->value[i]=rm->value[i];
	}

	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]频道扫描成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);

	return 1;
}

bool RadioTcpDeviceAccess::ChannelscanEx(int startfreq,int endfreq,int scanstep,SpecialRadioRetMessage_Obj& ChanScanRet)
{
	string RetScan("");
	int	PowerValue = 13;   //门限为整个频谱数据平均值的1.3倍
	//int MaxFreq[230];//调频广播扫描25个频点
	int freqnum=230;
	//	int iIndex=0;
	vector<SpectrumInfo> SpectrumVec;
	vector<SpectrumInfo> newSpectrumVec;
	std::vector<SpectrumInfo>::iterator i_newSpecVec;
	//	int lowvalue=0;
	//	int lownum=0;
	//
	//for(int i=0;i<4;i++)
	//{
	//	if(time(0)-g_sendcommandtime>=3)
	//	{
	//		break;
	//	}
	//	Sleep(1000);
	//}
	// 	 	for(int sp=0;sp<3;sp++)
	// 	 	{

	//scanstep = 50;
	// 			SpectrumVec.clear();
	if(!Spectrumscan(startfreq,endfreq,scanstep,9,SpectrumVec,0,0))
	{
		return false;
	}
	//cout<<"*******************"<<SpectrumVec.size()<<"***********************"<<endl;
	// 			// cout<<"********************"<<SpectrumVec.back().level<<"**********************"<<endl;
	if(SpectrumVec.size()<200)
	{
		return false;
	}

	// 	 		if((SpectrumVec.size()<=230)&&(SpectrumVec.size()>=1))
	// 	 		{
	// 	 			break;
	// 	 		}
	// 	 		else
	// 	 		{
	// 	 			Sleep(1000);              
	// 	 			continue;
	// 	 		}
	// 		}
	float avg = 0;
	for(int i = 0;i<SpectrumVec.size();i++)
	{	
		avg = avg +SpectrumVec[i].level;
	}
	int sum = 211;//(endfreq-startfreq)/scanstep;
	avg = avg/sum;
	//	g_sendcommandtime = time(0);
	//
	//if(SpectrumVec.size()<freqnum)
	//{
	//	freqnum = SpectrumVec.size();
	//}
	//high value into newSpect rumVec
	int temp[230]={0};
	int SpectrumPeak[230] = {0};

	//DBMANAGER::instance()->QueryChannelID(RADIO, m_vecChannelID);  //扫描频道表添加频道表中高电平值频率
	//std::vector<string>::iterator vC = m_vecChannelID.begin();
	//for(;vC != m_vecChannelID.end();vC++)
	//{
	//	int tempfreq = (int)((StrUtil::Str2Float(*vC)+0.0001)*1000);
	//	for(int i = 0;i<SpectrumVec.size();i++)
	//	{
	//		if((abs(tempfreq-SpectrumVec[i].freq)<10)&&(SpectrumVec[i].level>(avg*PowerValue)/10)&&(SpectrumVec[i].level>20))
	//		{
	//			if(((SpectrumVec[i].freq-startfreq)>-0.01)&&((endfreq-SpectrumVec[i].freq)>0.01)) ////广播手动设置频道扫描开始结束频率特殊处理,过滤不在用户设置的开始结束频率范围内的频率
	//			{
	//				SpectrumInfo SpectInfo ;
	//				SpectInfo.freq=SpectrumVec[i].freq;
	//				SpectInfo.level=SpectrumVec[i].level; 
	//				newSpectrumVec.push_back(SpectInfo);
	//				RetScan.append(StrUtil::Float2Str(SpectInfo.freq));
	//				RetScan.append("-");
	//				RetScan.append(StrUtil::Float2Str(SpectInfo.level));
	//				RetScan.append(";");
	//			}
	//		}
	//	}
	//}

	//第一遍扫瞄
	for(int kk = 1;kk<SpectrumVec.size();kk++)
	{
		if(SpectrumVec[kk].level>SpectrumVec[kk-1].level)
			temp[kk] = 1;
		else if(SpectrumVec[kk].level<SpectrumVec[kk-1].level)
			temp[kk] = 2;
		else
			temp[kk] = 0;
	}
	//第二遍扫瞄
	for(int kk = 1;kk<SpectrumVec.size()-1;kk++)
	{
		//	cout<<"111111"<<endl;
		if((/*((temp[kk-1]==1)&&temp[kk]==0)||*/temp[kk-1]==1) && temp[kk+1]==2)
		{
			if( (SpectrumVec[kk].level-SpectrumVec[kk-1].level)/1 > 3 ) //斜率
			{
				SpectrumInfo SpectInfo ;
				SpectrumPeak[kk] = 1;
				SpectInfo.freq=SpectrumVec[kk].freq;
				SpectInfo.level=SpectrumVec[kk].level; 
				if((SpectInfo.freq -87300)>150) //过滤87.1到87.4的频道
				{
					if(((SpectrumVec[kk].freq-startfreq)>-0.01)&&((endfreq-SpectrumVec[kk].freq)>0.01)) //广播手动设置频道扫描开始结束频率特殊处理,过滤不在用户设置的开始结束频率范围内的频率
					{   
						int mflag = 0;  //频道表及其周边频率（正负0.1）标志
						for(i_newSpecVec = newSpectrumVec.begin();i_newSpecVec !=newSpectrumVec.end();i_newSpecVec++)  //过滤已添加频率及其周边频率
						{
							if(abs(SpectrumVec[kk].freq- (*i_newSpecVec).freq)<101)
								mflag = 1;
						}
						if((mflag == 0)&&(SpectrumVec[kk].level>20))
						{
							newSpectrumVec.push_back(SpectInfo);
						}
					}
					//std::cout<<"峰值为："<<i<<"="<<SpectrumVec[i]<<endl;
				}
			}
			else
			{
				SpectrumPeak[kk] = 0;
			}
		}
	}
	// 第三遍添加特别峰值
	for(int kk = 1;kk<SpectrumVec.size()-1;kk++ )
	{
		//if(kk == 178)
		//	cout<<"111111"<<endl;
		if(SpectrumPeak[kk] == 0)  //已添加的频率不做处理
		{
			if((SpectrumVec[kk].level>(avg*PowerValue)/10)&&(SpectrumPeak[kk-1]+SpectrumPeak[kk+1]<1))
			{
				SpectrumInfo SpectInfo ;
				SpectrumPeak[kk] = 1;
				SpectInfo.freq=SpectrumVec[kk].freq;
				SpectInfo.level=SpectrumVec[kk].level;
				if((SpectInfo.freq -87300)>150) //过滤87.1到87.4的频道
				{
					if(((SpectrumVec[kk].freq-startfreq)>-0.01)&&((endfreq-SpectrumVec[kk].freq)>0.01)) ////广播手动设置频道扫描开始结束频率特殊处理,过滤不在用户设置的开始结束频率范围内的频率
					{
						int mflag = 0;  //频道表及其周边频率（正负0.1）标志
						for(i_newSpecVec = newSpectrumVec.begin();i_newSpecVec !=newSpectrumVec.end();i_newSpecVec++)  //过滤已添加频率频率及其周边频率
						{
							if(abs(SpectrumVec[kk].freq- (*i_newSpecVec).freq)<101)
								mflag = 1;
						}
						if((mflag == 0)&&(SpectrumVec[kk].level>25))
						{
							newSpectrumVec.push_back(SpectInfo);
						}
					}
				}
			}
		}
	}

	//if(freqnum<=6)
	//{
	//	ChanScanRet.len=0;
	//	for(int jj=0;jj<newSpectrumVec.size();jj++)
	//	{
	//		if(newSpectrumVec[jj].level>=StrUtil::Str2Int(PROPMANAGER::instance()->GetScanlimitlevel()))
	//		{
	//			ChanScanRet.value[ChanScanRet.len]=newSpectrumVec[jj].freq;
	//			ChanScanRet.len++;
	//		}
	//	}
	//	return true;
	//}
	m_vecChannelID.clear();
	ChanScanRet.len = 0;
	sort(newSpectrumVec.begin(),newSpectrumVec.end(),comparison);
	
	int specsum = newSpectrumVec.size();
	
	int jumpchannel = 0;

	for(int jj=0;jj<specsum;jj++)
	{
		{// 对过调广播节目导致的一个频点扫描为2个频率进行过滤
			if((jj>=1)&&jj<(specsum-1))
			{
				// cout<<"XXXXXXXXXXXX有问题的电台过滤222:";
				// cout<<newSpectrumVec[jj+1].freq-newSpectrumVec[jj].freq<<endl;
				if(newSpectrumVec[jj+1].freq-newSpectrumVec[jj].freq<210)
				{//相隔200kHz的频率，需要对中间频率的信号强度进行判断
					int n=SpectrumVec.size();
					for(int m=0;m<n;m++)
					{					
						if(abs(SpectrumVec[m].freq-newSpectrumVec[jj].freq)<10)
						{
							if(m<n-2)
							{
								if((SpectrumVec[m].level-SpectrumVec[m+1].level<3)&&(abs(SpectrumVec[m].level-SpectrumVec[m+2].level)<3)&&(abs(SpectrumVec[m+1].level-SpectrumVec[m+2].level)<3))
								{
									// cout<<"XXXXXXXXXXXX有问题的电台过滤"<<SpectrumVec[m].freq<<endl;
									jumpchannel=2;		
								}
							};
						}
					}

				}
			}
			if(jumpchannel>1)
			{//修改第一个
				jumpchannel=1;	
				newSpectrumVec[jj].freq = newSpectrumVec[jj].freq+100;
				//改频点
			}
			if(jumpchannel==1)
			{//跳过第二个，跳过一个频点
				jumpchannel=0;	
				continue;
			}
		}
		
		ChanScanRet.value[ChanScanRet.len]=newSpectrumVec[jj].freq;

		RetScan.append(StrUtil::Int2Str(newSpectrumVec[jj].freq));
		RetScan.append("-");
		RetScan.append(StrUtil::Int2Str(newSpectrumVec[jj].level));
		RetScan.append(";");

		ChanScanRet.len++;
	}
	RetScan.append("Sum=");	
	RetScan.append(StrUtil::Int2Str(ChanScanRet.len));
	// APPLOG::instance()->WriteLog(ALARM,LOG_EVENT_DEBUG,RetScan,LOG_OUTPUT_FILE);
	return true;
}

int RadioTcpDeviceAccess::SendCmdForSpecScan(MSG_TYPE msg_type,void* info, RadioSpecRetMessage_Handle pRetObj,int infolen)
{
	if ( ConnectToServer() == false)
		return -1;
	int pkgLen = 0;
	char sendbuf[200] = {0}; 
	char RetMsg[2000] = {0};

	//信息头
	PkgHeader_Obj   ph;

	ph.header   = 0x53;
	ph.msg_type = msg_type;

	memcpy(sendbuf, &ph, sizeof(PkgHeader_Obj));
	pkgLen	+= sizeof(PkgHeader_Obj);

	//信息内容
	memcpy(sendbuf+pkgLen, info, infolen);
	pkgLen	+= infolen;
	//发送信息
	ACE_Time_Value SendTimeOut(10);
	ACE_Time_Value RecvTimeOut (60);

	string sysmsg;

	if(stream.send((char*)sendbuf, pkgLen,&SendTimeOut) <= 0)		//int lenth = send(m_sSocket,(char*)sendbuf, len, 0)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]发送指令失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);

		stream.close();
		return -2;
	}
	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]发送指令成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);

	//接收返回数据
	int lenth=0;
	if((lenth=stream.recv((char *)&RetMsg,sizeof(RadioSpecRetMessage_Obj),&RecvTimeOut) <= 0))
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]数据接收失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);

		stream.close();
		OSFunction::Sleep(0,500);
		return -3;
	}
	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]数据接收成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);

	OSFunction::Sleep(0,100);

	RadioSpecRetMessage_Handle rm = (RadioSpecRetMessage_Handle)(RetMsg);

	stream.close();
	if(rm->ph.header!=0x54 || rm->ph.msg_type!=(msg_type+0x100) || rm->status != 1)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]返回数据错误");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
		return -4;
	}
	pRetObj->ph=rm->ph;
	pRetObj->freq=rm->freq;
	pRetObj->status=rm->status;
	pRetObj->level_int=rm->level_int;
	memcpy(pRetObj->spec,rm->spec,1024*sizeof(char));

	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]频谱扫描成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);

	return 1;
}


int RadioTcpDeviceAccess::SendCmdForQuality(MSG_TYPE msg_type,void* info, RadioQuaRetMessage_Handle pRetObj,int infolen)
{
	Radiopayload_Obj radiopay = (*((Radiopayload_Obj*)(info)));
	RadioQuaRetMessage_Obj RetObj;
	//
	int rnt = -1;
	for(int i=0;i<4;i++)
	{
		if(time(0)-g_sendcommandtime>=3)
		{
			break;
		}
		Sleep(1000);
	}
	for(int sp=0;sp<2;sp++)
	{
		rnt = GetRadioQulity(radiopay.freq,RetObj);
		Sleep(2000);
		rnt = GetRadioQulity(radiopay.freq,RetObj);
		if(RetObj.level_int != 0)
		{
			break;
		}
	}
	g_sendcommandtime = time(0);
	//
	if(rnt>0)
	{
		pRetObj->ph=RetObj.ph;
		pRetObj->dev_int=RetObj.dev_int;
		pRetObj->dF=RetObj.dF;
		pRetObj->freq=RetObj.freq;
		pRetObj->level_int=RetObj.level_int;
		pRetObj->CNR=RetObj.CNR;
		pRetObj->Audio_Freq=RetObj.Audio_Freq;
		pRetObj->Audio_THD=RetObj.Audio_THD;
		pRetObj->status=RetObj.status;

		string sysmsg = string("频点[") + StrUtil::Int2Str(radiopay.freq) + string("]指标测量成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);
	}
	else
	{
		string sysmsg = string("频点[") + StrUtil::Int2Str(radiopay.freq) + string("]指标测量失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);
	}

	return rnt;
}


bool RadioTcpDeviceAccess::SpecScan(int freq,RadioSpecRetMessage_Obj& RadioSpec)
{
	Radiopayload_Obj freqinfo;

	freqinfo.flag=0x01;
	freqinfo.freq=freq;
	freqinfo.gain=5;
	int ret=SendCmdForSpecScan(MSG_GET_RADIO,(void *)&freqinfo,&RadioSpec,sizeof(Radiopayload_Obj));
	if(ret>0)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void  RadioTcpDeviceAccess::CaculateCenterFreq(int startfreq,int endfreq,int *centerfreqs,int &len)
{
	int I=(endfreq-startfreq)*1000/(3660*1024);

	int middlefreq=startfreq;
	int k=0;
	centerfreqs[0]=startfreq+(511*3660)/1000;
	for(k=1;k<=I;k++)
	{
		centerfreqs[k]=(centerfreqs[k-1]+(3660*1023)/1000);
	}

	len=k;
	return ;
}

int RadioTcpDeviceAccess::Findpolars(char spec_int[1023],int fFreq,int *result)
{
	int ij,k,z,max=0,addk=0,addmax=0;
	double sum=0.0;
	int i_result = 0;
	float polar = 0.0;
	char tmp = 0;
	int throwhold;
	for(ij=0;ij<993;)
	{
		for(k=0;k<29;k++)
		{
			for(z=0;z<(29-k);z++)
			{
				if(((spec_int[ij+z+1+k]-spec_int[ij+k])<10)||
					((spec_int[ij+z+k+1]-spec_int[ij+k])>-10))
				{
					addk++;
				}
				else
				{
					addk = 0;
					sum = 0.0;
					break;
				}
			}
			if(addk==0)
				break;
			sum+=pow(10.0,(double)spec_int[ij+k]/20.0);
		}
		if(addk)
			break;
		ij+=(z+k);
	}
	throwhold = (int)(log10(sum/29.0)*20.0);
	for(ij=0;ij<1023;ij++)
	{
		tmp = spec_int[ij];
		addmax = 0;
		max = 0;
		if((tmp>(throwhold+10))&&(tmp<127))
		{
			if(ij>972)
			{
				for(k=ij+1;k<1023;k++)
				{
					if(tmp>spec_int[k])
						addmax++;
					else
					{
						break;
					}
				}
				for(k=ij+1;(k<(ij+10))&&(k<1023);k++)
				{
					if(tmp==spec_int[k])
					{
						addmax++;
					}
					else
					{
						break;
					}
				}
				if(addmax==(1022-ij))
				{

				}
				else
				{
					continue;
				}
			}
			else
			{
				for(k=ij+1;k<ij+51;k++)
				{
					if(tmp>spec_int[k])
					{
						addmax++;
					}
					else
					{
						break;
					}
				}

				for(k=ij+1;(k<(ij+10))&&(k<1023);k++)
				{
					if(tmp==spec_int[k])
					{
						addmax++;
					}
					else
					{
						break;
					}
				}

				if(addmax==50)
				{

				}
				else
				{
					continue;
				}
			}
			if(ij<50)
			{
				for(k=0;k<ij;k++)
				{
					if(tmp>spec_int[k])
					{
						max++;
					}
					else
					{
						break;
					}
				}
				if(k>0)
				{
					for(k=ij-1;(k>(ij-10))&&(k>=0);k--)
					{
						if(tmp==spec_int[k])
						{
							max++;
						}
						else
						{
							break;
						}
					}
				}

				if(max==ij)
				{
					polar=(float)fFreq*100.0-(512-ij)*3.66;
					if(polar<87000||polar>108000)
						continue;
					result[i_result] = (int)polar;
					if(i_result>0)
					{
						if((result[i_result]-result[i_result-1])<100)
						{
							result[i_result-1]=(result[i_result]+result[i_result-1])/2;
							i_result--;
						}
					}
					i_result++;
				}
				else
				{
					continue;
				}
			}
			else
			{
				for(k=(ij-50);k<ij;k++)
				{
					if(tmp>spec_int[k])
					{
						max++;
					}
					else
					{
						break;
					}
				}
				for(k=(ij-1);(k>(ij-10))&&(k>=0);k--)
				{
					if(tmp==spec_int[k])
					{
						max++;
					}
					else
					{
						break;
					}
				}
				if(max==50)
				{
					polar=(float)fFreq*100.0-(511-ij)*3.66;
					if(polar<87000||polar>108000)
						continue;
					result[i_result] = (int)polar;
					if(i_result>0)
					{
						if((result[i_result]-result[i_result-1])<100)
						{
							result[i_result-1]=(result[i_result]+result[i_result-1])/2;
							i_result--;
						}
					}
					i_result++;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			continue;
		}
	}

	return i_result;
}


bool RadioTcpDeviceAccess::GetRadioSpecResult(float fFreq, RadioSpecRetMessage_Obj &rqr)
{
	if(fFreq<110&&fFreq>=87)
	{
		int nfreq = (int)((fFreq)*1000);	//修正float to int 修正添加0.2
		if(SpecScan(nfreq,rqr))
		{
			return true;
		}
		else
		{
			string sysmsg = string("广播频点[") + StrUtil::Int2Str(nfreq) + string("]测量失败");
			SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
		}
	}
	return false;
}


bool RadioTcpDeviceAccess::GetQuality(float fFreq, RadioQuaRetMessage_Obj &rqr)
{	
	if(fFreq<108&&fFreq>=87)
	{
		int nfreq = (int)((fFreq+0.0002)*1000);	//修正float to int 修正添加0.2

		Radiopayload_Obj freqinfo;
		freqinfo.flag = 0x02;
		freqinfo.freq = nfreq;
		freqinfo.gain = 5;
		int ret=SendCmdForQuality(MSG_GET_RADIO,(void *)&freqinfo,&rqr,sizeof(Radiopayload_Obj));
		if(ret>0)
		{
			return true;
		}
		else
		{
			string sysmsg = string("广播频点[") + StrUtil::Int2Str(nfreq) + string("]测量失败");
			SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);

			return false;
		}
	}
	return false;
}


bool RadioTcpDeviceAccess::GetRadioFreqLevel(float fFreq, float &fLevel)
{
	if(fFreq<108&&fFreq>=87)
	{
		int nfreq = (int)((fFreq*1000));	//修正float to int 修正添加0.2
		int index = int((fFreq-87)*10);		//修正float to int 修正添加0.2

		RadioQuaRetMessage_Obj rqr;
		memset(&rqr, 0, sizeof(RadioQuaRetMessage_Obj) );

		Radiopayload_Obj freqinfo;
		freqinfo.flag = 0x02;
		freqinfo.freq = nfreq;
		freqinfo.gain = 5;
		int ret=SendCmdForQuality(MSG_GET_RADIO,(void *)&freqinfo,&rqr,sizeof(Radiopayload_Obj));
		if(ret>0)
		{
			fLevel = rqr.level_int;	
			return true;
		}
		else
		{
			string sysmsg = string("广播频点[") + StrUtil::Int2Str(nfreq) + string("]测量失败");
			SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);

			return false;
		}
	}
	return false;
}

bool RadioTcpDeviceAccess::TurnFreq( TunerConfig_Obj& tunerinfo)
{
	tunerinfo.freq += 100;		//适应新板卡的频偏
	bool rtn = SendCmdToServer(MSG_SET_FMTUNER,(void*)&tunerinfo,sizeof(TunerConfig_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")+ strIPAddress +string(":") + StrUtil::Int2Str(mChannelID) + string("]音频调频失败[") + \
			StrUtil::Int2Str(tunerinfo.chan) + string(":") + StrUtil::Int2Str(tunerinfo.freq) + string("]") ;
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[") + strIPAddress  +string(":") + StrUtil::Int2Str(mChannelID) + string("]音频调频成功[") + \
			StrUtil::Int2Str(tunerinfo.chan) + string(":") + StrUtil::Int2Str(tunerinfo.freq) + string("]") ;
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
	}

	return rtn;
}

bool RadioTcpDeviceAccess::SetAudioRate(const MediaConfig_Obj& rateinfo)
{
	bool rtn = SendCmdToServer(MSG_SET_AUDIOBITRATE,(void*)&rateinfo,sizeof(MediaConfig_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置音频码率失败[") + \
			StrUtil::Int2Str(rateinfo.chan) + string(":") + StrUtil::Int2Str(rateinfo.video_bitrate)+ \
			string(":") + StrUtil::Int2Str(rateinfo.audio_idx) + string("]") ;

		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置音频码率成功[") + \
			StrUtil::Int2Str(rateinfo.chan) + string(":") + StrUtil::Int2Str(rateinfo.video_bitrate)+ \
			string(":") + StrUtil::Int2Str(rateinfo.audio_idx) + string("]") ;

		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
	}

	return rtn;
}


/*
*6U： Radio调频函数，需要频点参数freq和调制方式参数mode
*
*@freq: 调频频点
*@mode: 广播调频调制方式,默认参数为MODULATION_RADIO_FM
*/
bool RadioTcpDeviceAccess::TurnFreqFor6U(int freq, Modulation_Radio_e mode)
{


	// ACE_Guard<ACE_Thread_Mutex> guard(RadioEncoderdevMutex);

	VSTune_Info_Obj tuneInfo;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tuneInfo.chanNum);
	tuneInfo.tuneInfo.TunerType = TUNER_TYPE_RADIO;
	tuneInfo.tuneInfo.RadioModulation = mode;
	tuneInfo.tuneInfo.Frequency = freq;

	bool rtn = SendCmdToServer(MSG_SET_6U_TUNE,(void*)&tuneInfo, sizeof(VSTune_Info_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(DeviceId) + string("]视频调频失败[") + \
		StrUtil::Int2Str(tuneInfo.chanNum) + string(":") + StrUtil::Int2Str(freq) + string("]") ;
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+StrUtil::Int2Str(DeviceId) + string("]视频调频成功[") + \
		StrUtil::Int2Str(tuneInfo.chanNum) + string(":") + StrUtil::Int2Str(freq) + string("]") ;
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
	}
	return rtn;
}

/*
*6U: Radio转码设置函数，平台下发的XML中有编码后的Bps, 所以通过参数传递进来
*
*@Bps: 编码后的视频码率
*@width: 编码后的视频宽度,默认值为"",当不传值或传入值为""时是不是应该用数据库中的配置参数
*@height: 编码后的视频高度,默认值"",当不传值或传入值为""时是不是应该用数据库中的配置参数
*
*TODO
*确认编码的其他参数，编码控制模式是否可以作为具有默认值的参数给出
*广播转码的码率怎么确认
*/
bool RadioTcpDeviceAccess::SetEncodeFor6U(string Bps, string width, string height)
{

	// ACE_Guard<ACE_Thread_Mutex> guard(RadioEncoderdevMutex);

	VSEncode_Param_Obj encodeObj;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,encodeObj.chanNum);
	
// 	string TsIp = "238.0.0.99" ;
// 	unsigned int tsport=11000+DeviceId;
	//	string deviceIP = "";
	//	PROPMANAGER::instance()->GetDeviceIP(DeviceId, deviceIP);
	//	string TsIp = "238.0.0." + deviceIP.substr(deviceIP.rfind(".") + 1);
	std::string TsIp = "172.16.10.250";
	unsigned int tsport=11000+DeviceId;



	encodeObj.encodeParam.VideoBitrate = StrUtil::Str2Int(Bps);
	if(encodeObj.encodeParam.VideoBitrate <300000||encodeObj.encodeParam.VideoBitrate > 1500000)
	{
		encodeObj.encodeParam.VideoBitrate = 300000;
	}
	//设置默认的音频码率，可能为64K
	encodeObj.encodeParam.AudioBitrate = 64000;
	//视频码率控制模式， 默认为ENCODE_CTL_CBR-固定码率
	encodeObj.encodeParam.EncodeType = ENCODE_CTL_CBR;
	//QPValue: EncodeType为FIXQP时有效
	encodeObj.encodeParam.QPValue = 20;
	//设置编码后的视频Pid，待确认
	encodeObj.encodeParam.VideoPID = 66 + DeviceId;
	//设置编码后的音频Pid，待确认
	encodeObj.encodeParam.AudioPID = 2 + DeviceId;
	//设置编码后的PcrPid，待确认
	encodeObj.encodeParam.PCRPID = 55 + DeviceId;
	//设置编码后的UDPADDR，待确认
	encodeObj.encodeParam.UDPaddr = ntohl(inet_addr(TsIp.c_str()));
	//encodeObj.encodeParam.UDPaddr = 0xEF010101;
	//设置编码后的UDPPort，带确认
	//PROPMANAGER::instance()->GetDeviceTsPort(DeviceId, encodeObj.encodeParam.UDPport);
	encodeObj.encodeParam.UDPport = tsport;
	//设置视频编码类型，待确认
	encodeObj.encodeParam.VideoType = 0;
	//设置音频编码类型，待确认
	encodeObj.encodeParam.AudioType = 0;
	//设置编码分辨率宽度
	encodeObj.encodeParam.Width = StrUtil::Str2Int(width);
	//设置编码分辨率的高度
	encodeObj.encodeParam.Height = StrUtil::Str2Int(height);

	//如果本次设置的编码器参数和上一次一样，则直接返回成功，否则，记录本次的编码参数
	if(encodeObj.encodeParam == mLastEncoderParam)
		return true;
	else
		mLastEncoderParam = encodeObj.encodeParam;

	bool rtn = SendCmdToServer(MSG_SET_6U_ENCODER,(void*)&encodeObj, sizeof(VSEncode_Param_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(DeviceId) + string("]转码设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(DeviceId) + string("]转码设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
	}
	return rtn;
}

/*
*6U: Radio台标OSD设置函数
*
*@dvbtype: 接口中的DVBType，用于获取数据库中配置的OSD信息
*@name: 节目名称
*
*TODO
*由于节目名称的长度不一样，可能需要重新计算台标OSD的位置
*/
bool RadioTcpDeviceAccess::SetOSDFor6U(string dvbtype, string name)
{

	// ACE_Guard<ACE_Thread_Mutex> guard(RadioEncoderdevMutex);

	OSDInfo osd;
	VSOSD_Info_Obj osdInfo;
	eDVBType etype=OSFunction::GetEnumDVBType(dvbtype);
	PROPMANAGER::instance()->GetOSDInfo(etype,"0",osd);	//模拟无高清

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,osdInfo.chanNum);
	//规定台标TEXTID为1， #define DIGITAL_ON_SCREEN_GRAPHIC_TEXTID 1
	osdInfo.osdInfo.TextID = DIGITAL_ON_SCREEN_GRAPHIC_TEXTID;
	//设置osd显示的字体，字体集及取值待确定
	osdInfo.osdInfo.Font = 0;
	//设置osd显示的字体大小，以前默认为16
	osdInfo.osdInfo.Size = 16;
	//设置osd显示的横坐标,具体位置由数据库中的参数确定
	
	//设置osd显示的纵坐标
	osdInfo.osdInfo.PositionY = 1;

	string tmpName = osd.Info + string(" ") + name;
//	osdInfo.osdInfo.PositionX = 720-(14*tmpName.length());//::Str2Int(osd.ProgramX )* 3;
	memset(osdInfo.osdInfo.Text, 0, MAX_OSD_INFO_TEXT_LENGTH);
	memcpy(osdInfo.osdInfo.Text, tmpName.c_str(), tmpName.length());
	osdInfo.osdInfo.PositionX = 720 - StrUtil::GetStrPixelLength(osdInfo.osdInfo.Text, 20) - 64;//::Str2Int(osd.ProgramX )* 3;
	/*
	*TODO
	*
	*由于不同的台标名称长度不一样，具体的横纵坐标可能需要重新计算
	*/
	bool rtn = SendCmdToServer(MSG_SET_6U_OSD,(void*)&osdInfo, sizeof(VSOSD_Info_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]OSD设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]OSD设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);

	}
	return rtn;
}

/*
*6U: Radio时间OSD设置函数
*
*@dvbtype: 接口中的DVBType，用于获取数据库中配置的OSD信息
*@enable: 是否显示时间OSD，1是显示，0是不显示
*/
bool RadioTcpDeviceAccess::SetTimeOSDFor6U(string dvbtype, int enable)
{	

	// ACE_Guard<ACE_Thread_Mutex> guard(RadioEncoderdevMutex);

	OSDInfo osd;
	VSOSD_Time_Info_Obj timeOsdInfo;
	eDVBType etype=OSFunction::GetEnumDVBType(dvbtype);
	PROPMANAGER::instance()->GetOSDInfo(etype,"0",osd);	//模拟无高清

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,timeOsdInfo.chanNum);
	//设置时间osd显示的横坐标,具体位置由数据库中的参数确定
	//timeOsdInfo.posX = StrUtil::Str2Int(osd.TimeX);
	timeOsdInfo.posX = 720 - 192 - 64;
	//设置时间osd显示的纵坐标
	//timeOsdInfo.posY = StrUtil::Str2Int(osd.TimeY);
	timeOsdInfo.posY = 40;
	timeOsdInfo.enable = enable;

	bool rtn = SendCmdToServer(MSG_SET_6U_ENABLETIMEINFO,(void*)&timeOsdInfo, sizeof(VSOSD_Info_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]时间OSD设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]时间OSD设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);

	}
	return rtn;
}

/*
*6U: Radio频道扫描函数
*
*@startFreq 起始频点
*@endFreq   结束频点
*@scanStep  扫描步长
*@retObj    返回的数据结构
*
*TODO
*应该将扫描方式作为参数传入
*/
bool RadioTcpDeviceAccess::Channelscan(string startFreq, string endFreq, string scanStep, VSScan_Result_Obj& retObj)
{
	int pkgLen = 0;
	char sendbuf[200] = {0}; 

	//信息头
	PkgHeader_Obj   ph;
	ph.header   = 0x48;
	ph.msg_type = MSG_SET_6U_SCAN_FM;

	memcpy(sendbuf, &ph, sizeof(PkgHeader_Obj));
	pkgLen	+= sizeof(PkgHeader_Obj);


	ChannelScan_Obj obj;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,obj.chan);
	obj.Startfreq = (StrUtil::Str2Float(startFreq)+0.0002)*1000.0;
	obj.Endfreq = (StrUtil::Str2Float(endFreq)+0.0002)*1000.0;
	obj.step = (StrUtil::Str2Float(scanStep)+0.0002)*1000.0;

	if (obj.Startfreq < 87000)
		obj.Startfreq = 87000;
	if (obj.Endfreq > 108000)
		obj.Endfreq = 108000;

	//测试6U设备
	VSScan_Param_Obj scanParamObj;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,scanParamObj.chanNum);
	//指定Tuner类型，广播调频类型为TUNER_TYPE_RADIO
	scanParamObj.scanParam.TunerType = TUNER_TYPE_RADIO;
	//指定频道扫描类型，默认为SCAN_TYPE_FAST，
	//TODO: 是不是应该将扫描类型作为一个参数传入 如果TunerType = TUNER_TYPE_RADIO SCAN_TYPE_FAST类型作为扫描参数下传
	scanParamObj.scanParam.ScanType = SCAN_TYPE_FULL;
	//起始频点
	scanParamObj.scanParam.StartFrequency = obj.Startfreq;
	//结束频点
	scanParamObj.scanParam.EndFrequency = obj.Endfreq;
	//步长
	scanParamObj.scanParam.StepSize = obj.step;

	//信息内容
	memcpy(sendbuf+pkgLen, &scanParamObj, sizeof(VSScan_Param_Obj));
	pkgLen	+= sizeof(VSScan_Param_Obj);

	// VSScan_Result_Obj scanResultObj;

	string sysmsg;

	if(1 == SendCmdToServer((void*)&sendbuf, 200, (void*)&retObj, sizeof(VSScan_Result_Obj)))
	{
		//memcpy((void)retObj,&scanResultObj,sizeof(VSScan_Result_Obj));
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]频道扫描设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
		return true;
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]频道扫描设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
		return false;
	}

}


//设置板卡报警灵敏度

bool RadioTcpDeviceAccess::setThresholdInfo()
{
	int tunerId = -1;
	enumDVBType eType;
	std::vector<sAlarmParam> AlarmParamVec;
	VSThreshold_Info_Obj thresholdObj;

	//如果不是录像通道，那么就不是6U板卡，直接返回
	if(!(PROPMANAGER::instance()->IsRecordDeviceID(DeviceId)))
		return true;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,thresholdObj.chanNum);
	PROPMANAGER::instance()->GetDeviceType(DeviceId, eType);
	ALARMPARAMINFOMGR::instance()->GetAlarmParamByDvbtype(eType, AlarmParamVec);

	//先设置默认参数，如果数据库里有相应的参数，再对应覆盖
	//静帧、黑场、彩条报警参数设置为-1表示关闭报警
	thresholdObj.thresholdInfo.BlackSimilar = -1;
	thresholdObj.thresholdInfo.ColourBarSimilar = -1;
	thresholdObj.thresholdInfo.FreezeSimilar = -1;
	thresholdObj.thresholdInfo.AudioLostValue = 0;

	thresholdObj.thresholdInfo.VolumeHighValue = 70;
	thresholdObj.thresholdInfo.VolumeLowValue = 0;

	//以下参数数据库中没有配置项，设置默认值
	thresholdObj.thresholdInfo.AudioUnsualLastTime  = 500;
#if 0
	int value;
	std::vector<sAlarmParam>::iterator iter = AlarmParamVec.begin();
	for(; iter != AlarmParamVec.end(); iter++)
	{
		if(iter->TypeID == "24") //无伴音
		{
			value = StrUtil::Str2Int(iter->Num);
			thresholdObj.thresholdInfo.AudioLostValue = value;

			value = StrUtil::Str2Int(iter->UpThreshold);
			thresholdObj.thresholdInfo.VolumeHighValue = value;
			value = StrUtil::Str2Int(iter->DownThreshold);
			thresholdObj.thresholdInfo.VolumeLowValue = value;
		}

		else if(iter->TypeID == "14") //静帧
		{
			value = StrUtil::Str2Int(iter->Num);
			thresholdObj.thresholdInfo.FreezeSimilar  = (value >= 0 ? value : 990);
		}
		else if(iter->TypeID == "18") //黑屏
		{
			value = StrUtil::Str2Int(iter->Num);
			thresholdObj.thresholdInfo.BlackSimilar = (value >= 0 ? value : 990);
		}
		else if(iter->TypeID == "14") //彩条
		{
			value = StrUtil::Str2Int(iter->Num);
			thresholdObj.thresholdInfo.ColourBarSimilar  = (value >= 0 ? value : 920);
		}

	}
#endif

	bool rtn = SendCmdToServer(MSG_SET_6U_THRESHOLD,(void*)&thresholdObj, sizeof(VSThreshold_Info_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + string(":")+ StrUtil::Int2Str(DeviceId) + string("]板卡报警灵敏度设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + string(":")+ StrUtil::Int2Str(DeviceId) + string("]板卡报警灵敏度设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);
	}
	return rtn;

}

bool RadioTcpDeviceAccess::getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj)
{
	int pkgLen = 0;
	char sendbuf[200] = {0}; 
	VSTuner_Quality_Param obj;

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

	if(1 != SendCmdToServer((void*)&sendbuf, 200, (void*)&retObj, sizeof(VSTuner_Quality_Result_Obj)))
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]指标返回失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
		return false;
	}
// 	else
// 	{
// 		//memcpy((void)retObj,&scanResultObj,sizeof(VSScan_Result_Obj));
// 		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]指标返回成功");
// 		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	return true;
// 	}

}

bool RadioTcpDeviceAccess::getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj)
{
	int pkgLen = 0;
	char sendbuf[200] = {0}; 
	VSTuner_AllQuality_Param obj;

	//信息头
	PkgHeader_Obj   ph;
	ph.header   = 0x48;
	ph.msg_type = MSG_GET_6U_ALLQUALITY;

	memcpy(sendbuf, &ph, sizeof(PkgHeader_Obj));
	pkgLen	+= sizeof(PkgHeader_Obj);

	obj.TunerType = TUNER_TYPE_RADIO;

	//信息内容
	memcpy(sendbuf+pkgLen, &obj, sizeof(VSTuner_AllQuality_Param));
	pkgLen	+= sizeof(VSTuner_AllQuality_Param);


	string sysmsg;

	if(1 != SendCmdToServer((void*)&sendbuf, 200, (void*)&retObj, sizeof(VSTuner_AllQuality_Result_Obj)))
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]全部指标查询返回失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
		return false;
	}

	return true;
}
bool RadioTcpDeviceAccess::setCardSystemTimeFor6U()
{
	int tunerId = -1;

	//如果不是录像通道，那么就不是6U板卡，直接返回
	if(!(PROPMANAGER::instance()->IsRecordDeviceID(DeviceId)))
		return true;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tunerId);

	if(tunerId != 0)
		return true;

	Rtc_Time_t rtcObj;
	time_t now = time(0);
	rtcObj.year = StrUtil::Str2Int(TimeUtil::GetYearFromDatetime(now)) - 1900;
	rtcObj.month = StrUtil::Str2Int(TimeUtil::GetMonthFromDatetime(now)) -1;
	rtcObj.weekday = StrUtil::Str2Int(TimeUtil::GetWeekDayFromDatetime(now));
	rtcObj.date = StrUtil::Str2Int(TimeUtil::GetDayFromDatetime(now));
	rtcObj.hour = StrUtil::Str2Int(TimeUtil::GetHourFromDatetime(now));
	rtcObj.minute = StrUtil::Str2Int(TimeUtil::GetHMinuteFromDatetime(now));
	rtcObj.second = StrUtil::Str2Int(TimeUtil::GetSecondFromDatetime(now));

	bool rtn = SendCmdToServer(MSG_SET_6U_SYSTIME,(void*)&rtcObj, sizeof(Rtc_Time_t));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + string(":")+ StrUtil::Int2Str(DeviceId) + string("]板卡RTC时间设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + string(":")+ StrUtil::Int2Str(DeviceId) + string("]板卡RTC时间设置成功");
		std::cout <<sysmsg << std::endl;
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
	}
	std::cout <<sysmsg << std::endl;
	return rtn;
}

bool RadioTcpDeviceAccess::setScanControlParam()
{
	Scan_Control_Param_t scanParam;
	XmlParser m_SetScanValue;
	std::string nodePath;

	m_SetScanValue.LoadFromFile("C:/vscttb/SetScanContrValue.xml");
	pXMLNODE node = m_SetScanValue.GetNodeFromPath("ScanContr");
	
	pXMLNODELIST nodeList = m_SetScanValue.GetNodeList(node);
	
	node = nodeList->GetFirstNode();
		

	
	m_SetScanValue.GetAttrNode(node,"FmLevel",scanParam.FmLevel );
	m_SetScanValue.GetAttrNode(node,"AmLevel",scanParam.Amlevel);

	m_SetScanValue.GetAttrNode(node,"FmUsn",scanParam.FmUsn);
	m_SetScanValue.GetAttrNode(node,"FmWam",scanParam.FmWan);
	

	bool rtn = SendCmdToServer(MSG_SET_6U_SCANPARAM,(void*)&scanParam, sizeof(Scan_Control_Param_t));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + string(":")+ StrUtil::Int2Str(DeviceId) + string("]板卡频道扫描参数设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + string(":")+ StrUtil::Int2Str(DeviceId) + string("]板卡频道扫描参数设置成功");
		std::cout <<sysmsg << std::endl;
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
	}
	std::cout <<sysmsg << std::endl;
	return rtn;
}

bool comparison(SpectrumInfo a,SpectrumInfo b)
{  
	return a.freq<b.freq;  
}  

/*
*6U: Radio音柱设置函数
*
*@enable: 是否显示音柱，1是显示，0是不显示
*/
bool RadioTcpDeviceAccess::setVolumeFor6U(int enable)
{
	VSVolume_Param_Obj volumeObj;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,volumeObj.chanNum);
	volumeObj.enable = enable;

	bool rtn = SendCmdToServer(MSG_SET_6U_VOLUME,(void*)&volumeObj, sizeof(VSVolume_Param_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]音柱设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]音柱设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);

	}
	return rtn;
}

bool RadioTcpDeviceAccess::changeChan(std::string dvbtype, int freq, std::string Bps, std::string name, int timeEnable, int volumeEnabel, int bgEnable)
{
	VSChangeChan_Param_Obj chanInfoObj;
	eDVBType etype=OSFunction::GetEnumDVBType(dvbtype);
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,chanInfoObj.chanNum);

	chanInfoObj.tuneInfo.TunerType = TUNER_TYPE_RADIO;
	chanInfoObj.tuneInfo.RadioModulation = MODULATION_RADIO_FM;
	chanInfoObj.tuneInfo.Frequency = freq;

	//	string deviceIP = "";
	//	PROPMANAGER::instance()->GetDeviceIP(DeviceId, deviceIP);
	//	string TsIp = "238.0.0." + deviceIP.substr(deviceIP.rfind(".") + 1);
	std::string TsIp = "172.16.10.250";
	unsigned int tsport=11000+DeviceId;

	chanInfoObj.encodeInfo.VideoBitrate = StrUtil::Str2Int(Bps);
	if(chanInfoObj.encodeInfo.VideoBitrate <200000||chanInfoObj.encodeInfo.VideoBitrate > 1500000)
	{
		chanInfoObj.encodeInfo.VideoBitrate = 200000;
	}
	//设置默认的音频码率，可能为64K
	chanInfoObj.encodeInfo.AudioBitrate = 64000;
	//视频码率控制模式， 默认为ENCODE_CTL_CBR-固定码率
	chanInfoObj.encodeInfo.EncodeType = ENCODE_CTL_CBR;
	//QPValue: EncodeType为FIXQP时有效
	chanInfoObj.encodeInfo.QPValue = 20;
	//设置编码后的视频Pid，待确认
	chanInfoObj.encodeInfo.VideoPID = 66 + DeviceId;
	//设置编码后的音频Pid，待确认
	chanInfoObj.encodeInfo.AudioPID = 2+ DeviceId;
	//设置编码后的PcrPid，待确认
	chanInfoObj.encodeInfo.PCRPID = 55 + DeviceId;
	//设置编码后的UDPADDR，待确认
	chanInfoObj.encodeInfo.UDPaddr =	ntohl(inet_addr(TsIp.c_str()))/*0xEF010101*/;
	//设置编码后的UDPPort，带确认
	//PROPMANAGER::instance()->GetDeviceTsPort(DeviceId, encodeObj.encodeParam.UDPport);
	chanInfoObj.encodeInfo.UDPport = tsport;
	//设置视频编码类型，待确认
	chanInfoObj.encodeInfo.VideoType = 0;
	//设置音频编码类型，待确认
	chanInfoObj.encodeInfo.AudioType = 0;
	//设置编码分辨率宽度
	chanInfoObj.encodeInfo.Width = 0;
	//设置编码分辨率的高度
	chanInfoObj.encodeInfo.Height = 0;

	OSDInfo osd;
	PROPMANAGER::instance()->GetOSDInfo(etype,"0",osd);	//模拟无高清
	//规定台标TEXTID为1， #define DIGITAL_ON_SCREEN_GRAPHIC_TEXTID 1
	chanInfoObj.osdInfo.TextID = DIGITAL_ON_SCREEN_GRAPHIC_TEXTID;
	//设置osd显示的字体，字体集及取值待确定
	chanInfoObj.osdInfo.Font = 0;
	//设置osd显示的字体大小，以前默认为16
	chanInfoObj.osdInfo.Size = 16;
	//设置osd显示的横坐标,具体位置由数据库中的参数确定
	//osdInfo.osdInfo.PositionX = StrUtil::Str2Int(osd.ProgramX);
	//设置osd显示的纵坐标
	chanInfoObj.osdInfo.PositionY = 1;

	string tmpName = osd.Info + string(" ") + name;
	//osdInfo.osdInfo.PositionX = 720-(14*tmpName.length());//::Str2Int(osd.ProgramX )* 3;
	memset(chanInfoObj.osdInfo.Text, 0, MAX_OSD_INFO_TEXT_LENGTH);
	memcpy(chanInfoObj.osdInfo.Text, tmpName.c_str(), tmpName.length());
	chanInfoObj.osdInfo.PositionX = 720- StrUtil::GetStrPixelLength(chanInfoObj.osdInfo.Text, 20) - 64;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,chanInfoObj.timeInfo.chanNum);
	//设置时间osd显示的横坐标,具体位置由数据库中的参数确定
	//timeOsdInfo.posX = StrUtil::Str2Int(osd.TimeX);
	chanInfoObj.timeInfo.posX = 720 - 192 - 64;
	//设置时间osd显示的纵坐标
	//timeOsdInfo.posY = StrUtil::Str2Int(osd.TimeY);
	chanInfoObj.timeInfo.posY = 40;
	chanInfoObj.timeInfo.enable = timeEnable;

	chanInfoObj.volumeEnable = volumeEnabel;
	chanInfoObj.bgEnable = 0;

	bool rtn = SendCmdToServer(MSG_SET_6U_CHANGECHAN,(void*)&chanInfoObj, sizeof(VSChangeChan_Param_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]调台设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]调台设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);

	}
	return rtn;
}
