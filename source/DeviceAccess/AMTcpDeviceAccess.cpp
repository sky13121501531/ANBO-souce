
#include "./AMTcpDeviceAccess.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/OSFunction.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Communications/SysMsgSender.h"
#include "../BusinessProcess/AlarmParamInfoMgr.h"

#include <algorithm>
using namespace std;
extern string g_strcmdhead;
extern ACE_Thread_Mutex SendtodevMutex;

AmTcpDeviceAccess::AmTcpDeviceAccess(int deviceid,std::string strIP,int nPort) : TCPDeviceAccess(deviceid, strIP, nPort)
{
	setThresholdInfo();
}

bool AmTcpDeviceAccess::GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
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

		if(dvbtype=="AM")
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
					f_value=rqr.level_int;
					if(f_value<6)
						f_value=0.0f;
					break;
				case 2:
					f_value = rqr.dev_int;;
					break;
				case 3:
					f_value=rqr.dF;
					break;
				case 4:
					f_value=rqr.Audio_Freq;
					break;
				case 5:
					f_value=rqr.Audio_THD;
					break;
				case 6:
					f_value=rqr.CNR;
					break;
				}
			}
			else
			{
				f_value =0.0f;
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
	Retparser.SetAttrNode("Freq",StrUtil::Float2Str(freq),reportNode);
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
	return true;
}

bool AmTcpDeviceAccess::GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	vector<SpectrumInfo> SpectrumVec;
	std::string StartFreq,EndFreq,StepFreq;
	pXMLNODE QueryNode = parser.GetNodeFromPath("Msg/SpectrumQuery");
	parser.GetAttrNode(QueryNode,"EndFreq",EndFreq);
	parser.GetAttrNode(QueryNode,"StartFreq",StartFreq);
	parser.GetAttrNode(QueryNode,"StepFreq",StepFreq);
	//
	int startfreq = StrUtil::Str2Int(StartFreq);
	int endfreq = StrUtil::Str2Int(EndFreq);
	int scanstep = StrUtil::Str2Int(StepFreq);
	int num = (endfreq - startfreq)/scanstep;
	for(int sp=0;sp<3;sp++)
	{
		SpectrumVec.clear();
		Spectrumscan(startfreq,endfreq,scanstep,9,SpectrumVec);
		if(SpectrumVec.size()<=(num+10))
		{
			break;
		}
		else
		{
			Sleep(100);
			continue;
		}
	}
	//
	//pXMLNODE ParamNode=parser.GetNodeFromPath("Msg/SpectrumParam");
	//pXMLNODELIST NodeList =parser.GetNodeList(ParamNode);
	//int ListCount =parser.GetChildCount(ParamNode);
	//for(int i=0;i<ListCount;i++)
	//{
	//	OSFunction::Sleep(0,50);
	//	SpectrumInfo Temp;
	//	string freq,level;
	//	pXMLNODE IndexNode=parser.GetNextNode(NodeList);
	//	parser.GetAttrNode(IndexNode,"Freq",freq);

	//	if(dvbtype =="AM")
	//	{
	//		float ffreq = (float)StrUtil::Str2Float(freq);

	//		RadioSpecRetMessage_Obj rsr;
	//		if(GetAMSpec(ffreq,rsr))
	//		{
	//			if(rsr.freq>53100)
	//			{
	//				for(int tt=-10,i=0; tt<10; ++tt,++i)	//
	//				{
	//					float tempfreq = rsr.freq + tt*100;
	//					if ( tempfreq > ( StrUtil::Str2Float(EndFreq)*100 ) )
	//						break;
	//					Temp.freq = tempfreq*10;

	//					float templevel = (float)rsr.spec[i];
	//					if (templevel < 0.0f)
	//						templevel = 1.0f;
	//					Temp.level = templevel;

	//					SpectrumVec.push_back(Temp);

	//				}
	//			}
	//		}	
	//	}
	//	else
	//	{
	//		return false;
	//	}
	//}

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
	return true;
}

bool AmTcpDeviceAccess::GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retAMxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					<Msg DVBType=\"AM\" >\
					<Return Type=\"\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					<ChannelScan></ChannelScan></Msg>";
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	XmlParser retpaser(retAMxml.c_str());
	pXMLNODE channscannode=parser.GetNodeFirstChild(childnode);
	string startfreq,endfreq,strstep;
	parser.GetAttrNode(channscannode,"StartFreq",startfreq);
	parser.GetAttrNode(channscannode,"EndFreq",endfreq);
	parser.GetAttrNode(channscannode,"StepFreq",strstep);

	VSScan_Result_Obj scanRetObj;
	if(!Channelscan(startfreq, endfreq, strstep, scanRetObj))
	{
		strRetMsg="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
				  <Msg DVBType=\"AM\" >\
				  <Return Type=\"\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
				  <ChannelScan></ChannelScan></Msg>";
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}

	vector<int> ErrnewFrlis;
	PROPMANAGER::instance()->SeparateStrVec(PROPMANAGER::instance()->GetNewFrAlarmErrNewFreq(),ErrnewFrlis);
	vector<int> OknewFrlis;
	PROPMANAGER::instance()->SeparateStrVec(PROPMANAGER::instance()->GetNewFrAlarmOkNewFreq(),OknewFrlis);
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
}

bool AmTcpDeviceAccess::GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"\" >\
					  <Return Type=\"TSQuery\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \</Msg>";
	string retfailxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"\" >\
					  <Return Type=\"TSQuery\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \</Msg>";
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
	
	int tuneFreq = StrUtil::Str2Float(freq)*1000;

	if ((name.empty() && name.length()< 39))
	{
		name = string("未知频道");
	}

	//调频->设置转码->设置台标OSD->设置时间OSD
	//这里是不是需要添加启用背景图片
	if (/*(dvbtype == string("ATV")||dvbtype == string("CTV")) && */TurnFreqFor6U(tuneFreq) && SetEncodeFor6U(bps) && SetOSDFor6U(dvbtype, name) && SetTimeOSDFor6U(dvbtype, 1))
	{
		strRetMsg=retsuccxml;
		return true;
	}
	else
	{
		strRetMsg=retfailxml;
		return false;
	}
	return true;
	/*
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	pXMLNODE tsnode=parser.GetNodeFromPath("Msg/TSQuery/TS");
	string freq,bps;
	parser.GetAttrNode(tsnode,"Bps",bps);
	parser.GetAttrNode(tsnode,"Freq",freq);

	MediaConfig_Obj media_obj;
	TunerConfig_Obj tuner_obj;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,media_obj.chan);		
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tuner_obj.chan);

	if (dvbtype != string("AM"))
	{
		strRetMsg=retfailxml;
		return false;
	}

	tuner_obj.freq=int(StrUtil::Str2Float(freq));
	media_obj.audio_idx=GetAudioIndex(StrUtil::Str2Int(bps));

	if (TurnFreq(tuner_obj) && SetAudioRate(media_obj))
	{
		string name;
		CHANNELINFOMGR::instance()->GetProNameByFreq(OSFunction::GetEnumDVBType(dvbtype),freq,name);
		SubtitleConfig_Obj osdinfo;

		if ((name.empty() && name.length()< 39))
		{
			memcpy(osdinfo.subtitle0,"未知频道",8);
		}
		else
		{
			memcpy(osdinfo.subtitle0,name.c_str(),name.length());
		}
		OSDInfo osd;
		eDVBType etype=OSFunction::GetEnumDVBType(dvbtype);
		PROPMANAGER::instance()->GetOSDInfo(etype,"0",osd);	//模拟无高清
		if(osd.Info=="")
			osd.Info="金石.北京";
		memcpy(osdinfo.subtitle1,osd.Info.c_str(),40);
		osdinfo.size=StrUtil::Str2Int(osd.FontSize);
		osdinfo.subtitle0_x=StrUtil::Str2Int(osd.ProgramX);
		osdinfo.subtitle0_y=StrUtil::Str2Int(osd.ProgramY);
		osdinfo.subtitle1_x=StrUtil::Str2Int(osd.InfoX);
		osdinfo.subtitle1_y=StrUtil::Str2Int(osd.InfoY);
		osdinfo.time_x=StrUtil::Str2Int(osd.TimeX);
		osdinfo.time_y=StrUtil::Str2Int(osd.TimeY);
		osdinfo.mode=StrUtil::Str2Int(osd.TimeType);
		SetOSD(tuner_obj.chan,osdinfo);
		SetSysTime(tuner_obj.chan);
		strRetMsg=retsuccxml;
		return true;
	}
	else
	{
		strRetMsg=retfailxml;
		return false;
	}
	return true;
*/
}

int AmTcpDeviceAccess::SendCmdForSpecScan(MSG_TYPE msg_type,void* info, RadioSpecRetMessage_Handle pRetObj,int infolen)
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
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);

		stream.close();
		return -2;
	}
	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]发送指令成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);

	//接收返回数据
	int lenth=0;
	if((lenth=stream.recv((char *)&RetMsg,sizeof(RadioSpecRetMessage_Obj),&RecvTimeOut) <= 0))
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]数据接收失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);

		stream.close();
		OSFunction::Sleep(0,500);
		return -3;
	}
	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]数据接收成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);

	OSFunction::Sleep(0,100);

	RadioSpecRetMessage_Handle rm = (RadioSpecRetMessage_Handle)(RetMsg);
	
	stream.close();
	if(rm->ph.header!=0x54 || rm->ph.msg_type!=(msg_type+0x100) || rm->status != 1)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]返回数据错误");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);
		return -4;
	}
	pRetObj->ph=rm->ph;
	pRetObj->freq=rm->freq;
	pRetObj->status=rm->status;
	pRetObj->level_int=rm->level_int;
	memcpy(pRetObj->spec,rm->spec,1024*sizeof(char));

	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]频谱扫描成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);

	return 1;
}

int AmTcpDeviceAccess::SendCmdForQuality(MSG_TYPE msg_type,void* info, RadioQuaRetMessage_Handle pRetObj,int infolen)
{
	Radiopayload_Obj radiopay = (*((Radiopayload_Obj*)(info)));
	RadioQuaRetMessage_Obj RetObj;
	//
	int rnt=GetRadioQulity(radiopay.freq,RetObj);
	//
	if(rnt>0)
	{
		pRetObj->ph=RetObj.ph;
		pRetObj->dev_int=RetObj.dev_int;
		pRetObj->dF=RetObj.dF;
		pRetObj->freq=RetObj.freq;
		pRetObj->level_int=RetObj.level_int;
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

bool AmTcpDeviceAccess::SpecScan(int freq,RadioSpecRetMessage_Obj& RadioSpec)
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

void AmTcpDeviceAccess::CaculateCenterFreq(int startfreq,int endfreq,int *centerfreqs,int &len)
{
	int I=(endfreq-startfreq) /(20*1000);

	int middlefreq=startfreq;
	int k=0;
	centerfreqs[0]=startfreq + 10*1000;
	for(k=1;k<=I;k++)
	{
		centerfreqs[k]=(centerfreqs[k-1]+20*1000);
	}

	len = k;
}

void AmTcpDeviceAccess::FindChannels(int startfreq, int endfreq, const vector<float>& vecSpecValue, vector<int>& vecChanFreq)
{
	//每10KHz取一个电平最大的频点
	const int FIND_CHANNEL_STEP =  10;

	//取10个电平值最大的
	const int TOP_CHANNEL_NUM = 10;

	//20KHz内，去一个频点当做频道扫描结果
	const int FREQ_STEP = 20000;

	try
	{
		/** 每十个频点取一个电平最高的
		*/
		std::multimap<float, int, greater<float> > mapLevelFreq;
		float fMaxlevel = -1000.0f;
		int iMaxFreq = -1;
		for (int specNum=0; specNum<vecSpecValue.size(); specNum++)
		{
			if (vecSpecValue[specNum] > fMaxlevel)
			{
				fMaxlevel = vecSpecValue[specNum];
				iMaxFreq = startfreq + specNum*1000;
			}
			if (specNum % 10 == 0 && specNum != 0)
			{
				mapLevelFreq.insert( make_pair(fMaxlevel, iMaxFreq) );		//map自动按照电平排序
				fMaxlevel = -1000.0f;
			}

		}
		if (fMaxlevel != -1000.0f)	//最后一个最大值放入mapLevelFreq中
			mapLevelFreq.insert( make_pair(fMaxlevel, iMaxFreq) );

		//如果取出来的频点个数小于TOP_CHANNEL_NUM，就用频点个数
		int iChanCount = mapLevelFreq.size();
		if (iChanCount > TOP_CHANNEL_NUM)
			iChanCount = TOP_CHANNEL_NUM;

		/**	将最大的iChanCount个频点放入tempMap中
		*/	
		std::multimap<float, int, greater<float> >::iterator beginIter = mapLevelFreq.begin();
		std::multimap<float, int, greater<float> >::iterator endIter = beginIter;
		//调整iter的位置
		for (int i=0; i<iChanCount; i++)
		{
			if (endIter == mapLevelFreq.end())
				break;

			endIter ++ ;
		}
		std::multimap<float, int, greater<float> > tempMap(beginIter, endIter);

		/**	如果mapLevelFreq中还有与tempMap中电平值相同的，就取出来放入tempMap中
		*/
		std::multimap<float, int, greater<float> >::iterator getlevelIter = endIter;
		if(beginIter!=endIter)
		{
			getlevelIter--;
			float tempLastLevel = getlevelIter->first;
			while (endIter != mapLevelFreq.end())
			{
				if ((*endIter).first != tempLastLevel)
					break;
				tempMap.insert( *endIter++ );
			}
		}

		/** 每隔FREQ_STEP消除电平低的频点
		*/
		std::multimap<float, int, greater<float> >::iterator ChanIter = tempMap.begin();
		for (; ChanIter!=tempMap.end(); ChanIter++)
		{
			std::multimap<float, int, greater<float> >::iterator inerIter = ChanIter;
			inerIter++;

			while ( inerIter != tempMap.end() )
			{
				//频点在20KHz内，就删除电平值小的
				if ( abs((ChanIter->second) - (inerIter->second))<= FREQ_STEP )
				{
					tempMap.erase(inerIter ++);
				}
				else
					inerIter ++;
			}
		}

		/**	将结果放入返回容器vecChanFreq中
		*/
		std::multimap<float, int, greater<float> >::iterator inputIter = tempMap.begin();
		for (;inputIter!=tempMap.end();inputIter++)
		{
			vecChanFreq.push_back( (*inputIter).second );
		}

		//从小到大排序
		sort( vecChanFreq.begin(), vecChanFreq.end() );
	}
	catch (...)
	{
		string sysmsg = string("频道扫描异常");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);
	}

	return;
}


bool AmTcpDeviceAccess::GetAMSpec(float fFreq, RadioSpecRetMessage_Obj &rqr)
{
	if(fFreq<2610&&fFreq>=531)
	{
		int nfreq = (int)fFreq;//修正float to int 修正添加0.2
		if(SpecScan(nfreq,rqr))
		{
			return true;
		}
		else
		{
			string sysmsg = string("广播频点[") + StrUtil::Int2Str(nfreq) + string("]测量失败");
			SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);
		}
	}
	return false;
}


bool AmTcpDeviceAccess::GetQuality(float fFreq, RadioQuaRetMessage_Obj &rqr)
{	
	int nfreq = (int)(fFreq);//修正float to int 修正添加0.2

	Radiopayload_Obj freqinfo;
	freqinfo.flag = 0x02;
	freqinfo.freq = nfreq;
	freqinfo.gain = 5;
	int ret = SendCmdForQuality(MSG_GET_RADIO,(void *)&freqinfo,&rqr,sizeof(Radiopayload_Obj));
	if(ret>0)
	{
		return true;
	}
	else
	{
		string sysmsg = string("广播频点[") + StrUtil::Int2Str(nfreq) + string("]测量失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);
		return false;
	}
	return false;
}


bool AmTcpDeviceAccess::TurnFreq( TunerConfig_Obj& tunerinfo)
{
	string flag=StrUtil::Long2Str(time(0));
	string cmd=string("tuner:");
	cmd+=StrUtil::Int2Str(tunerinfo.chan);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(tunerinfo.freq);
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
	bool rtn = SendCmdToServer((void*)sendinfo.c_str(),sendinfo.size(),(void*)retinfo,30);

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]音频调频失败[") + \
			StrUtil::Int2Str(tunerinfo.chan) + string(":") + StrUtil::Int2Str(tunerinfo.freq) + string("]") ;
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]音频调频成功[") + \
			StrUtil::Int2Str(tunerinfo.chan) + string(":") + StrUtil::Int2Str(tunerinfo.freq) + string("]") ;
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);
	}
	string slog=retinfo;
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)%s\n",slog.c_str()));
	return rtn;
}


bool AmTcpDeviceAccess::SetAudioRate(const MediaConfig_Obj& rateinfo)
{
	string flag=StrUtil::Long2Str(time(0));
	string cmd=string("bitrate:");
	cmd+=StrUtil::Int2Str(rateinfo.chan);
	cmd+=string(",");
	cmd+=StrUtil::Int2Str(rateinfo.video_bitrate);
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
	bool rtn = true;//SendCmdToServer((void*)sendinfo.c_str(),sendinfo.size(),(void*)retinfo,30);

	//string sysmsg;
	//if (rtn == false)
	//{
	//	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置音频码率失败[") + \
	//		StrUtil::Int2Str(rateinfo.chan) + string(":") + StrUtil::Int2Str(rateinfo.video_bitrate)+ \
	//		string(":") + StrUtil::Int2Str(rateinfo.audio_idx) + string("]") ;

	//	SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO,VS_MSG_SYSALARM);
	//}
	//else
	//{
	//	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]设置音频码率成功[") + \
	//		StrUtil::Int2Str(rateinfo.chan) + string(":") + StrUtil::Int2Str(rateinfo.video_bitrate)+ \
	//		string(":") + StrUtil::Int2Str(rateinfo.audio_idx) + string("]") ;

	//	SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
	//}
	//string slog=retinfo;
	//ACE_DEBUG ((LM_DEBUG,"(%T | %t)%s\n",slog.c_str()));

	return rtn;
}


/*
*6U： AM调频函数，需要频点参数freq和调制方式参数mode
*
*@freq: 调频频点
*@mode: 广播调频调制方式,默认参数为MODULATION_RADIO_AM
*/
bool AmTcpDeviceAccess::TurnFreqFor6U(int freq, Modulation_Radio_e mode)
{
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
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+StrUtil::Int2Str(DeviceId) + string("]视频调频成功[") + \
		StrUtil::Int2Str(tuneInfo.chanNum) + string(":") + StrUtil::Int2Str(freq) + string("]") ;
		SYSMSGSENDER::instance()->SendMsg(sysmsg, AM, VS_MSG_SYSALARM);
		SYSMSGSENDER::instance()->SendMsg(sysmsg, AM);
	}
	return rtn;
}

/*
*6U: AM转码设置函数，平台下发的XML中有编码后的Bps, 所以通过参数传递进来
*
*@Bps: 编码后的视频码率
*@width: 编码后的视频宽度,默认值为"",当不传值或传入值为""时是不是应该用数据库中的配置参数
*@height: 编码后的视频高度,默认值"",当不传值或传入值为""时是不是应该用数据库中的配置参数
*
*TODO
*确认编码的其他参数，编码控制模式是否可以作为具有默认值的参数给出
*广播转码的码率怎么确认
*/
bool AmTcpDeviceAccess::SetEncodeFor6U(string Bps, string width, string height)
{	

// 	string TsIp = "238.0.0." + StrUtil::Int2Str(100+DeviceId);
// 	int tsport=9000+DeviceId;
	string deviceIP = "";
	PROPMANAGER::instance()->GetDeviceIP(DeviceId, deviceIP);
	string TsIp = "238.0.0." + deviceIP.substr(deviceIP.rfind(".") + 1);
	unsigned int tsport=11000+DeviceId;

	VSEncode_Param_Obj encodeObj;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,encodeObj.chanNum);

	encodeObj.encodeParam.VideoBitrate = StrUtil::Str2Int(Bps);
	if(encodeObj.encodeParam.VideoBitrate <300000||encodeObj.encodeParam.VideoBitrate > 1500000)
	{
		encodeObj.encodeParam.VideoBitrate = 700000;
	}
	//设置默认的音频码率，可能为64K
	encodeObj.encodeParam.AudioBitrate = 64000;
	//视频码率控制模式， 默认为ENCODE_CTL_CBR-固定码率
	encodeObj.encodeParam.EncodeType = ENCODE_CTL_CBR;
	//QPValue: EncodeType为FIXQP时有效
	encodeObj.encodeParam.QPValue = 20;
	//设置编码后的视频Pid，待确认
	encodeObj.encodeParam.VideoPID = 66;
	//设置编码后的音频Pid，待确认
	encodeObj.encodeParam.AudioPID = 22;
	//设置编码后的PcrPid，待确认
	encodeObj.encodeParam.PCRPID = 55;
	//设置编码后的UDPADDR，待确认
	encodeObj.encodeParam.UDPaddr = ntohl(inet_addr(TsIp.c_str()));
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
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM, VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(DeviceId) + string("]转码设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);
	}
	return rtn;
}

/*
*6U: AM台标OSD设置函数
*
*@dvbtype: 接口中的DVBType，用于获取数据库中配置的OSD信息
*@name: 节目名称
*
*TODO
*由于节目名称的长度不一样，可能需要重新计算台标OSD的位置
*/
bool AmTcpDeviceAccess::SetOSDFor6U(string dvbtype, string name)
{	
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
	osdInfo.osdInfo.PositionX = StrUtil::Str2Int(osd.ProgramX);
	//设置osd显示的纵坐标
	osdInfo.osdInfo.PositionY = StrUtil::Str2Int(osd.ProgramY);

	string tmpName = osd.Info + string(" ") + name;
	memset(osdInfo.osdInfo.Text, 0, MAX_OSD_INFO_TEXT_LENGTH);
	memcpy(osdInfo.osdInfo.Text, tmpName.c_str(), tmpName.length());
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
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]OSD设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);

	}
	return rtn;
}

/*
*6U: AM时间OSD设置函数
*
*@dvbtype: 接口中的DVBType，用于获取数据库中配置的OSD信息
*@enable: 是否显示时间OSD，1是显示，0是不显示
*/
bool AmTcpDeviceAccess::SetTimeOSDFor6U(string dvbtype, int enable)
{	
	OSDInfo osd;
	VSOSD_Time_Info_Obj timeOsdInfo;
	eDVBType etype=OSFunction::GetEnumDVBType(dvbtype);
	PROPMANAGER::instance()->GetOSDInfo(etype,"0",osd);	//模拟无高清

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,timeOsdInfo.chanNum);
	//设置时间osd显示的横坐标,具体位置由数据库中的参数确定
	timeOsdInfo.posX = StrUtil::Str2Int(osd.TimeX);
	//设置时间osd显示的纵坐标
	timeOsdInfo.posY = StrUtil::Str2Int(osd.TimeY);
	timeOsdInfo.enable = enable;

	bool rtn = SendCmdToServer(MSG_SET_6U_OSD,(void*)&timeOsdInfo, sizeof(VSOSD_Info_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]时间OSD设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]时间OSD设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);

	}
	return rtn;
}

/*
*6U: AM频道扫描函数
*
*@startFreq 起始频点
*@endFreq   结束频点
*@scanStep  扫描步长
*@retObj    返回的数据结构
*
*TODO
*应该将扫描方式作为参数传入
*/
bool AmTcpDeviceAccess::Channelscan(string startFreq, string endFreq, string scanStep, VSScan_Result_Obj& retObj)
{
	int pkgLen = 0;
	char sendbuf[1024] = {0}; 
	char RetMsg[2048] = {0};
	char *pRetMsg = RetMsg;

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

	if (obj.Startfreq < 49750)
		obj.Startfreq = 49750;
	if (obj.Endfreq > 855250)
		obj.Endfreq = 855250;

	//测试6U设备
	VSScan_Param_Obj scanParamObj;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,scanParamObj.chanNum);
	//指定Tunner类型，广播调频类型为TUNER_TYPE_RADIO
	scanParamObj.scanParam.TunerType = TUNER_TYPE_RADIO;
	//指定频道扫描类型，默认为SCAN_TYPE_FAST，
	//TODO: 是不是应该将扫描类型作为一个参数传入
	scanParamObj.scanParam.ScanType = SCAN_TYPE_FAST;
	//起始频点
	scanParamObj.scanParam.StartFrequency = obj.Startfreq;
	//结束频点
	scanParamObj.scanParam.EndFrequency = obj.Endfreq;
	//步长
	scanParamObj.scanParam.StepSize = obj.step;

	//信息内容
	memcpy(sendbuf+pkgLen, &scanParamObj, sizeof(VSScan_Param_Obj));
	pkgLen	+= sizeof(VSScan_Param_Obj);

	VSScan_Result_Obj scanResultObj;

	string sysmsg;

	if(1 == SendCmdToServer((void*)&sendbuf, pkgLen, (void*)&scanResultObj, sizeof(VSScan_Result_Obj)))
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]频道扫描设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);
		return true;
	}else{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]频道扫描设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);
		return false;
	}

}


//设置板卡报警灵敏度

bool AmTcpDeviceAccess::setThresholdInfo()
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
#if 0
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
#endif
	}

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

/*
*6U: AM音柱设置函数
*
*@enable: 是否显示音柱，1是显示，0是不显示
*/
bool AmTcpDeviceAccess::setVolumeFor6U(int enable)
{
	VSVolume_Param_Obj volumeObj;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,volumeObj.chanNum);
	volumeObj.enable = enable;

	bool rtn = SendCmdToServer(MSG_SET_6U_VOLUME,(void*)&volumeObj, sizeof(VSVolume_Param_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]音柱设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]音柱设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,AM);

	}
	return rtn;
}
