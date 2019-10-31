
#include "./ATVTcpDeviceAccess.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/OSFunction.h"
#include "../Communications/SysMsgSender.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "../BusinessProcess/AlarmParamInfoMgr.h"
#include <stdio.h>
extern string g_strcmdhead;
extern ACE_Thread_Mutex DeviceCmdMutex;
extern time_t g_sendcommandtime;
bool g_realstreamround = false;
extern bool g_atvchanscan;

static	const long pStandardConstFreq[55] = {
	49750000,
	57750000,
	65750000,
	77250000,
	168250000,
	176250000,
	184250000,
	192250000,
	200250000,
	208250000,
	216250000,
	471250000,
	479250000,
	487250000,
	495250000,
	503250000,
	511250000,
	519250000,
	527250000,
	535250000,
	543250000,
	551250000,
	559250000,
	607250000,
	615250000,
	623250000,
	631250000,
	639250000,
	647250000,
	655250000,
	663250000,
	671250000,
	679250000,
	687250000,
	695250000,
	703250000,
	711250000,
	719250000,
	727250000,
	735250000,
	743250000,
	751250000,
	759250000,
	767250000,
	775250000,
	783250000,
	791250000,
	799250000,
	807250000,
	815250000,
	823250000,
	831250000,
	839250000,
	847250000,
	855250000,
};


static	const long AudioConstFreq[110] = {
	49750000,
	57750000,
	65750000,
	77250000,
	85250000,
	112250000,
	120250000,
	128250000,
	136250000,
	144250000,
	152250000,
	160250000,
	168250000,
	176250000,
	184250000,
	192250000,
	200250000,
	208250000,
	216250000,
	224250000,
	232250000,
	240250000,
	248250000,
	256250000,
	264250000,
	272250000,
	280250000,
	288250000,
	296250000,
	304250000,
	312250000,
	320250000,
	328250000,
	336250000,
	344250000,
	352250000,
	360250000,
	368250000,
	376250000,
	384250000,
	392250000,
	400250000,
	408250000,
	416250000,
	424250000,
	432250000,
	440250000,
	448250000,
	456250000,
	471250000,
	479250000,
	487250000,
	495250000,
	503250000,
	511250000,
	519250000,
	527250000,
	535250000,
	543250000,
	551250000,
	559250000,
	567250000,
	575250000,
	583250000,
	591250000,
	599250000,
	607250000,
	615250000,
	623250000,
	631250000,
	639250000,
	647250000,
	655250000,
	663250000,
	671250000,
	679250000,
	687250000,
	695250000,
	703250000,
	711250000,
	719250000,
	727250000,
	735250000,
	743250000,
	751250000,
	759250000,
	767250000,
	775250000,
	783250000,
	791250000,
	799250000,
	807250000,
	815250000,
	823250000,
	831250000,
	839250000,
	847250000,
	855250000,
	863250000,
	871250000,
	879250000,
	887250000,
	895250000,
	903250000,
	911250000,
	919250000,
	927250000,
	935250000,
	943250000,
	951250000
};


AtvTcpDeviceAccess::AtvTcpDeviceAccess(int deviceid,std::string strIP,int nPort) : TCPDeviceAccess(deviceid, strIP, nPort)
{
	mImageLevel = 0.00f;
	mAudioLevel = 0.00f;
	mI2AOffLevel = 0.00f;
	mCN = 0.00f;
	mFreqOffSet = 0.00f;
	mSlope = 0.00f;

	mAnalyserImageOffset = 0;
	mAnalyserAudioOffset = 0;
	mAnalyserI2AOffset = 0;
	mAnalyserCnOffset = 0;
	mAnalyserOffOffset = 0;
	mAnalyserSlopeOffset = 0;

	for(int i=0;i<5;i++)
	{
		fOldAnalyser[i] = 13.0f;
	}
 	setCardSystemTimeFor6U();
	setThresholdInfo();
}

bool AtvTcpDeviceAccess::GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
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
	TVQuality tqr;
	int nFreq = (freq+0.0002)*1000;
	//
	for(int i=0;i<4;i++)
	{
		if(time(0)-g_sendcommandtime>=3)
		{
			break;
		}
		Sleep(1000);
	}
	for(int sp=0;sp<3;sp++)
	{
		if(GetTVQulity(nFreq,tqr)>0)
		{
			break;
		}
		Sleep(100);
	}
	g_sendcommandtime = time(0);
	//
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
		if(dvbtype=="ATV"||dvbtype=="CTV")
		{
			switch(i_type)
			{
			case 1:
				f_value=tqr.ImageLevel;
				break;
			case 2:
				f_value=tqr.AudioLevel;
				break;
			case 3:
				f_value=tqr.I2AOffLevel;
				break;
			case 4:
				f_value=tqr.CN;
				break;
			case 5:
				f_value=tqr.FreqOffSet;
				break;
			case 6:
				f_value=tqr.Slope;
				break;
			}
		}
		else 
			return false; 

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
	SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
	return true;
}

bool AtvTcpDeviceAccess::GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg)//Note:strCmdMsg已经过滤工作模式，需补齐
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
	std::string WorkType;//Note:
	pXMLNODE QueryNode = parser.GetNodeFromPath("Msg/SpectrumQuery");
	parser.GetAttrNode(QueryNode,"EndFreq",EndFreq);
	parser.GetAttrNode(QueryNode,"StartFreq",StartFreq);
	parser.GetAttrNode(QueryNode,"StepFreq",StepFreq);
	parser.GetAttrNode(QueryNode,"WorkType",WorkType);//Note:工作模式
	//
	for(int i=0;i<4;i++)
	{
		if(time(0)-g_sendcommandtime>=3)
		{
			break;
		}
		Sleep(1000);
	}
	int startfreq = StrUtil::Str2Float(StartFreq)*1000;
	int endfreq = StrUtil::Str2Float(EndFreq)*1000;
	int scanstep = StrUtil::Str2Float(StepFreq)*1000;
	int num = (endfreq - startfreq)/scanstep;
	for(int sp=0;sp<3;sp++)
	{
		SpectrumVec.clear();
		Spectrumscan(startfreq,endfreq,scanstep,9,SpectrumVec,0,StrUtil::Str2Int(WorkType));
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
//FOR 6U安播，暂时不用TUNE做频道扫描
#if  0
bool AtvTcpDeviceAccess::GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml = "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
						<Msg DVBType=\"ATV\" >\
						<Return Type=\"\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
						<ChannelScan></ChannelScan></Msg>";

	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	XmlParser retpaser(retsuccxml.c_str());
	pXMLNODE channscannode=parser.GetNodeFirstChild(childnode);
	string startfreq,endfreq,strstep;
	parser.GetAttrNode(channscannode,"StartFreq",startfreq);
	parser.GetAttrNode(channscannode,"EndFreq",endfreq);
	parser.GetAttrNode(channscannode,"StepFreq",strstep);

	VSScan_Result_Obj scanRetObj;
	if(!Channelscan(startfreq, endfreq, strstep, scanRetObj))
	{
		strRetMsg = "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					<Msg DVBType=\"ATV\" >\
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


#endif


#if 0
bool AtvTcpDeviceAccess::GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retATVxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					 <Msg DVBType=\"ATV\" >\
					 <Return Type=\"\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					 <ChannelScan></ChannelScan></Msg>";
	string retCTVxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					 <Msg DVBType=\"CTV\" >\
					 <Return Type=\"\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					 <ChannelScan></ChannelScan></Msg>";

	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	XmlParser retpaser(retATVxml.c_str());
	pXMLNODE channscannode=parser.GetNodeFirstChild(childnode);
	string startfreq,endfreq,strstep;
	parser.GetAttrNode(channscannode,"StartFreq",startfreq);
	parser.GetAttrNode(channscannode,"EndFreq",endfreq);
	parser.GetAttrNode(channscannode,"StepFreq",strstep);
	ChannelScan_Obj obj;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,obj.chan);
	obj.Startfreq = (StrUtil::Str2Float(startfreq)+0.0002)*1000.0;
	obj.Endfreq = (StrUtil::Str2Float(endfreq)+0.0002)*1000.0;
	obj.step = (StrUtil::Str2Float(strstep)+0.0002)*1000.0;

	if (obj.Startfreq < 49750)
		obj.Startfreq = 49750;
	if (obj.Endfreq > 855250)
		obj.Endfreq = 855250;

	vector<int> ErrnewFrlis;
	PROPMANAGER::instance()->SeparateStrVec(PROPMANAGER::instance()->GetNewFrAlarmErrNewFreq(),ErrnewFrlis);
	vector<int> OknewFrlis;
	PROPMANAGER::instance()->SeparateStrVec(PROPMANAGER::instance()->GetNewFrAlarmOkNewFreq(),OknewFrlis);

	SpecialRadioRetMessage_Obj Retobj,retATVobj;
	float ResultFreq=0.0;
	//目前默认为标准频道扫描
	if(ChannelscanEx(obj.Startfreq,obj.Endfreq,obj.step,Retobj))
	{
#if 0
		Retobj.len = 10;
		Retobj.value[0] = 168250;
		Retobj.value[1] = 184250;

		Retobj.value[2] = 168250;
		Retobj.value[3] = 184250;

		Retobj.value[4] = 168250;
		Retobj.value[5] = 184250;

		Retobj.value[6] = 168250;
		Retobj.value[7] = 184250;

		Retobj.value[8] = 168250;
		Retobj.value[9] = 184250;
#endif
		LockFreq_Obj lockFreqObj;
		lockFreqObj.ValuedNum = 0;

		int realTaskdeviceid = 0;
		std::list<int> devicelist;
		PROPMANAGER::instance()->GetTaskDeviceList(string("StreamRealtimeQueryTask"),ATV,devicelist);
		if (devicelist.size() >= 1)
		{
			realTaskdeviceid = devicelist.front();
		}
		else
		{
			strRetMsg="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"ATV\" >\
					  <Return Type=\"\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
					  <ChannelScan></ChannelScan></Msg>";
			SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
			return false;
		}

		PROPMANAGER::instance()->GetDeviceTunerID(realTaskdeviceid,lockFreqObj.chanNum);

		memset(&(lockFreqObj.freqArr), 0, sizeof(int) * MAX_CHANNEL_SCAN_NUM);
		bool v_msign;

		for(int k=0;k<Retobj.len;k++)
		{
			v_msign = false;
			vector<int>::iterator v_mtemp = v_mTurnedFreq.begin();
			for(;v_mtemp != v_mTurnedFreq.end();v_mtemp++)
			{

				if(Retobj.value[k] == *v_mtemp)
				{
					v_msign = true;
					retATVobj.value[retATVobj.len] = Retobj.value[k];
					retATVobj.len++;
					break;
				}
			}
			if(v_msign == true)
				continue;
#if 0
			if(DEVICEACCESSMGR::instance()->CheckFreqLock(Retobj.value[k]))//查看此频率是否能锁定
			{
				v_mTurnedFreq.push_back(Retobj.value[k]);

				retATVobj.value[retATVobj.len] = Retobj.value[k];
				retATVobj.len++;

				cout<<"锁定频率："<<Retobj.value[k]<<endl;
			}
#endif
			lockFreqObj.freqArr[lockFreqObj.ValuedNum] = Retobj.value[k];
			lockFreqObj.ValuedNum ++;
		}

		VSScan_Result_Obj scanResultObj;
		bool lockRet = DEVICEACCESSMGR::instance()->LockFreqFor6U(realTaskdeviceid, &lockFreqObj, scanResultObj);
		if(lockRet && scanResultObj.resultObj.ValuedNum > 0)
		{
			pXMLNODE retchannode=retpaser.GetNodeFromPath("Msg/ChannelScan");
			for(int k=0;k<scanResultObj.resultObj.ValuedNum;k++)
			{
				if(!(IsInList(scanResultObj.resultObj.freqArr[k],ErrnewFrlis)))
				{
					ResultFreq=((float)(scanResultObj.resultObj.freqArr[k]))/1000;
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
			return true;
		}
	}
	else
	{
		strRetMsg="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
				  <Msg DVBType=\"ATV\" >\
				  <Return Type=\"\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
				  <ChannelScan></ChannelScan></Msg>";
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}

	return true;
}
#endif

#if 1
bool AtvTcpDeviceAccess::GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retATVxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					 <Msg DVBType=\"ATV\" >\
					 <Return Type=\"\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					 <ChannelScan></ChannelScan></Msg>";

	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	XmlParser retpaser(retATVxml.c_str());
	pXMLNODE channscannode=parser.GetNodeFirstChild(childnode);
	string startfreq,endfreq,strstep;
	parser.GetAttrNode(channscannode,"StartFreq",startfreq);
	parser.GetAttrNode(channscannode,"EndFreq",endfreq);
	parser.GetAttrNode(channscannode,"StepFreq",strstep);

	float fStartFreq = StrUtil::Str2Float(startfreq);
	float fEndFreq = StrUtil::Str2Float(endfreq);

	vector<int> ErrnewFrlis;
	PROPMANAGER::instance()->SeparateStrVec(PROPMANAGER::instance()->GetNewFrAlarmErrNewFreq(),ErrnewFrlis);
	vector<int> OknewFrlis;
	PROPMANAGER::instance()->SeparateStrVec(PROPMANAGER::instance()->GetNewFrAlarmOkNewFreq(),OknewFrlis);

	LockFreq_Obj lockFreqObj;
	lockFreqObj.ValuedNum = 0;
	float ResultFreq=0.0;

	VSScan_Result_Obj scanResultObj;
	
	//等待3秒再查询频道扫描结果
	bool lockRet = false;
	time_t startTime = time(0);
	for(int loop = 0; loop < 10; loop++)
	{
		lockRet = DEVICEACCESSMGR::instance()->LockFreqFor6U(DeviceId, &lockFreqObj, scanResultObj);
		if(lockRet && scanResultObj.resultObj.ValuedNum > 0)
		{
			break;
		}
		else
			std::cout << "DEVICEACCESSMGR::instance()->LockFreqFor6U返回的频点数量为0，再获取一次" << std::endl;
		Sleep(1000);
	}

	time_t sleepTime = time(0) - startTime ;
	if(sleepTime <  1)
		Sleep(3 * 1000);

	if(lockRet && scanResultObj.resultObj.ValuedNum > 0)
	{
		pXMLNODE retchannode=retpaser.GetNodeFromPath("Msg/ChannelScan");
		for(int k=0;k<scanResultObj.resultObj.ValuedNum;k++)
		{
			if(!(IsInList(scanResultObj.resultObj.freqArr[k],ErrnewFrlis)))
			{
				ResultFreq=((float)(scanResultObj.resultObj.freqArr[k]))/1000;
				if(ResultFreq < fStartFreq || ResultFreq > fEndFreq)
					continue;
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
		return true;
	}
	else
	{
		strRetMsg="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
				  <Msg DVBType=\"ATV\" >\
				  <Return Type=\"\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
				  <ChannelScan></ChannelScan></Msg>";
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}
	return true;
}
#endif

bool AtvTcpDeviceAccess::GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"\" >\
					  <Return Type=\"TSQuery\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \</Msg>";
	string retfailxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"\" >\
					  <Return Type=\"TSQuery\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \</Msg>";

	XmlParser parser;
	bool ret = true;
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
	parser.GetAttrNode(tsnode, "Width", width);
	parser.GetAttrNode(tsnode, "height", height);
	
	string name;
	CHANNELINFOMGR::instance()->GetProNameByFreq(OSFunction::GetEnumDVBType(dvbtype),freq,name);
	if ((name.empty() && name.length()< 39))
	{
		name = string("未知频道") + freq;
	}
	int tuneFreq = StrUtil::Str2Float(freq)*1000;
	//调频->设置转码->设置台标OSD->设置时间OSD

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
bool AtvTcpDeviceAccess::ChannelscanEx(int startfreq,int endfreq,int scanstep,SpecialRadioRetMessage_Obj& ChanScanRet)
{
	int MaxFreq[110];//开路电视扫描110个频点
	int iIndex=0;
	vector<SpectrumInfo> SpectrumVec;
	int validnum=0;
	ChanScanRet.len=0;
	//
	int num=110;
	//for(int sp=0;sp<3;sp++)
	//{
	//	SpectrumVec.clear();
	// g_atvchanscan = true;
	int sp=0;
	while(!Spectrumscan(startfreq,endfreq,scanstep,9,SpectrumVec,1,1))
	{
		if((sp++)>3) break;
	}
	g_atvchanscan = false;
	//	if((SpectrumVec.size()<=110)&&(SpectrumVec.size()>=1))
	//	{
	//		num = SpectrumVec.size();
	//		break;
	//	}
	//	else
	//	{
	//		Sleep(1000);
	//		continue;
	//	}
	//}

	g_sendcommandtime = time(0);
	if(SpectrumVec.size()<=0)
	{
		return false;
	}
	for(int j=0;j<SpectrumVec.size();j++)
	{
		//int tempLevel=0;
		//iIndex = 0;
		//for(int k=0;k<SpectrumVec.size();k++)
		//{
		//	if(tempLevel<SpectrumVec[k].level)
		//	{
		//		tempLevel=SpectrumVec[k].level;
		//		iIndex = k;
		//	}
		//}
		if(SpectrumVec[j].level<40)//电平<40不进行锁频处理
		{
			MaxFreq[j]=-1;
		}
		else
		{
			// MaxFreq[j]=SpectrumVec[iIndex].freq;
			//cout<<"＞＞＞＞＞＞＞＞＞＞＞＞　频道扫描频率:"<<SpectrumVec[j].freq<<endl;
			ChanScanRet.value[ChanScanRet.len]=SpectrumVec[j].freq;
			ChanScanRet.len++;
		}
		// SpectrumVec[iIndex].level=0;
	}
	//TunerConfig_Obj tuner_obj;
	//for(int k=0;k<num;k++)
	//{
	//	if(MaxFreq[k]>=0)
	//	{
	//		// tuner_obj.chan=DeviceId;
	//		tuner_obj.freq=MaxFreq[k];
	//		cout<<""<<tuner_obj.freq<<endl;
	//		// TurnFreq(tuner_obj);
	//		// Sleep(10);
	//		// if(CheckDeviceIDLock() == TRUE)
	//			cout<<"＞＞＞＞＞＞＞＞＞＞＞＞　频道扫描频率:"<<tuner_obj.freq<<endl;
	//			ChanScanRet.value[ChanScanRet.len]=tuner_obj.freq;
	//			ChanScanRet.len++;
	//	}
	//}
	return true;
}

int AtvTcpDeviceAccess::SendCmdForChannelScan(MSG_TYPE msg_type,void* info,ChannelScanRetMessage_Handle pRetObj, int infolen)
{
	if ( ConnectToServer() == false)
		return -1;

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

	string sysmsg;
	ACE_Time_Value sendtimeout(10);
	if(stream.send((char*)sendbuf, 200,&sendtimeout)<=0)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]发送指令失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);

		stream.close();
		return -1;
	}
	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]发送指令成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);

	int len = -1;

	ACE_Time_Value TimeOut(60);

	len =stream.recv((char*)&RetMsg,sizeof(RetMsg),&TimeOut);

	if(len<=0)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]数据接收失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);

		stream.close();
		return -1;
	}

	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]数据接收成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);

	ChannelScanRetMessage_Handle rm = (ChannelScanRetMessage_Handle)(RetMsg);

	stream.close();
	if(rm->ph.header!=0x49 || rm->ph.msg_type!=(msg_type+0x100) || rm->status != 1)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]返回数据错误");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
		return -4;
	}
	pRetObj->ph=rm->ph;
	pRetObj->status=rm->status;
	memcpy(pRetObj->freq,rm->freq,sizeof(pRetObj->freq));

	sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]频道扫描成功");
	SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);

	return 1;
}

bool AtvTcpDeviceAccess::GetFreqLevel(float fFreq, float &fLevel)//
{
	if((fFreq>0.01&&fFreq<1000)) //100k~1000M
	{
		int nFreq = (fFreq+0.0002)*1000;
		if(QualityRealtimeQueryTV(nFreq,fLevel))
			return true;
	}
	return false;
}

bool AtvTcpDeviceAccess::QualityRealtimeQueryTV(int freq,float& f_level)
{
	f_level = SendCmdToTVCom(MSG_GET_QUA,(void *)&freq,sizeof(int));
	string sysmsg;
	if(f_level>=0)
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]指标测量成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
		return true;
	}
	else
	{
		sysmsg = string("通道[") + StrUtil::Int2Str(mChannelID) + string("]指标测量失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
		return false;
	}
	return false;
}

float AtvTcpDeviceAccess::GetImageLevel(float Freq)
{
	if(GetFreqLevel(Freq,mImageLevel))
	{
		float ff = 0;
		mImageLevel += ff;			
		if(mImageLevel > (ff+1))
		{
			fOldAnalyser[0] = mImageLevel;
		}
		else 
		{
			mImageLevel = fOldAnalyser[0];
		}
	}
	else
	{
		mImageLevel = fOldAnalyser[0];
		mImageLevel = 0.00f;
	}
	return mImageLevel;
}

float AtvTcpDeviceAccess::GetAudioLevel(float Freq)
{
	if(GetFreqLevel(Freq + 6.5f,mAudioLevel))
	{
		if(mAudioLevel<0)
		{
			mAudioLevel = mImageLevel-15.5;
		}
		float ff = 0;
		if(ff <= 20&&ff >= -20)
		{
			mAudioLevel += ff;
		}
		if(mAudioLevel > (ff+1))
			fOldAnalyser[1] = mAudioLevel;
		else 
			mAudioLevel = fOldAnalyser[1];
	}
	else
	{
		mAudioLevel = 0.00f;
	}
	return mAudioLevel;
}

float AtvTcpDeviceAccess::GetI2AOffLevel(float Freq)
{
	if (mImageLevel == 0.00f)
	{
		GetFreqLevel(Freq,mImageLevel);
	}
	if (mAudioLevel == 0.00f) 
	{
		GetFreqLevel(Freq + 6.5f,mAudioLevel);
	}

	//mAnalyserAudioOffset = mAnalyserImageOffset = COffsetLoad::GetInstance()->GetOffsetLevel(Freq);

	mI2AOffLevel = (mImageLevel + mAnalyserImageOffset - mAudioLevel - mAnalyserAudioOffset) > 0?(mImageLevel + mAnalyserImageOffset - mAudioLevel - mAnalyserAudioOffset):(mImageLevel + mAnalyserImageOffset - mAudioLevel - mAnalyserAudioOffset);

	mI2AOffLevel += mAnalyserI2AOffset;

	return mI2AOffLevel;
}

float AtvTcpDeviceAccess::GetAlignedFreq(float freq)
{
	float fAlignedFreq = freq * 1000000;
	for(int i=1; i<55; ++i)
	{
		if(pStandardConstFreq[i] > fAlignedFreq)
		{
			if(pStandardConstFreq[i] - fAlignedFreq < fAlignedFreq - pStandardConstFreq[i-1])
			{
				fAlignedFreq = (float)pStandardConstFreq[i];
			}
			else
			{
				fAlignedFreq = (float)pStandardConstFreq[i-1];
			}
			break;
		}
	}
	return (float)(fAlignedFreq/1000000.0);
}

float AtvTcpDeviceAccess::GetCN(float Freq)
{
	GetBaseCN(GetAlignedFreq(Freq), mCN);
	mCN += mAnalyserCnOffset;
	return mCN;
}

float AtvTcpDeviceAccess::GetFreqOffSet(float Freq)
{
	return mFreqOffSet;
}

float AtvTcpDeviceAccess::GetSlope(float Freq)
{
	mSlope = 0.00f;
	mSlope += mAnalyserSlopeOffset;
	return mSlope;
}

bool AtvTcpDeviceAccess::GetBaseCN(float fFreq, float &fCN)
{
	float fLevel, fNoise;
	GetFreqLevel(fFreq,fLevel);
	GetFreqLevel((float)(fFreq - 1.1),fNoise);
	fCN = fLevel - fNoise;
	if(fCN < 1.0)
	{
		fCN = 0.0f;
	}
	return true;
}

float AtvTcpDeviceAccess::GetOpenTVStandardFreq(float freq)
{
	double fAlignedFreq = freq * 1000000;
	for(int i=1; i<110; ++i)
	{
		if((AudioConstFreq[i] - fAlignedFreq >= 0) && (fAlignedFreq - AudioConstFreq[i-1] > 0))
		{
			fAlignedFreq = (double)AudioConstFreq[i];
			break;
		}
	}
	return (fAlignedFreq/1000000);
}

bool AtvTcpDeviceAccess::TurnFreq( TunerConfig_Obj& tunerinfo)
{
	ACE_Guard<ACE_Thread_Mutex> guard(DeviceCmdMutex);

	bool rtn = SendCmdToServer(MSG_SET_VIDEOTUNER,(void*)&tunerinfo,sizeof(TunerConfig_Obj));
	tcTunerFreq.chan = tunerinfo.chan;
	tcTunerFreq.freq = tunerinfo.freq;

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]视频调频失败[") + \
			StrUtil::Int2Str(tunerinfo.chan) + string(":") + StrUtil::Int2Str(tunerinfo.freq) + string("]") ;
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
	}
	else
	{
		//rtn = CheckDeviceIDLock();
		//if(rtn)
		{
			sysmsg = string("通道[")  + strIPAddress +string(":")+StrUtil::Int2Str(mChannelID) + string("]视频调频成功[") + \
				StrUtil::Int2Str(tunerinfo.chan) + string(":") + StrUtil::Int2Str(tunerinfo.freq) + string("]") ;
			SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);
		}
		//else
		//{
		//	sysmsg = string("通道[")  + strIPAddress +string(":")+StrUtil::Int2Str(mChannelID) + string("]视频锁频失败[") + \
		//		StrUtil::Int2Str(tunerinfo.chan) + string(":") + StrUtil::Int2Str(tunerinfo.freq) + string("]") ;
		//	SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);
		//}
	}
	return rtn;
}

bool AtvTcpDeviceAccess::SetVideoRate(const MediaConfig_Obj& rateinfo)
{
	bool rtn = SendCmdToServer(MSG_SET_VIDEOBITRATE,(void*)&rateinfo,sizeof(MediaConfig_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[") + strIPAddress +string(":") + StrUtil::Int2Str(mChannelID) + string("]设置视频码率失败[") + \
			StrUtil::Int2Str(rateinfo.chan) + string(":") + StrUtil::Int2Str(rateinfo.video_bitrate)+ \
			string(":") + StrUtil::Int2Str(rateinfo.audio_idx) + string("]") ;

		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[") + strIPAddress +string(":") + StrUtil::Int2Str(mChannelID) + string("]设置视频码率成功[") + \
			StrUtil::Int2Str(rateinfo.chan) + string(":") + StrUtil::Int2Str(rateinfo.video_bitrate)+ \
			string(":") + StrUtil::Int2Str(rateinfo.audio_idx) + string("]") ;

		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);
	}

	return rtn;
}

/*
*6U： ATV调频函数，需要频点参数freq和调制方式参数mode
*
*@freq: 调频频点
*@mode: 电视调制方式,默认参数为MODULATION_TV_PALDK
*/
bool AtvTcpDeviceAccess::TurnFreqFor6U(int freq, Modulation_TV_e mode)
{
	ACE_Guard<ACE_Thread_Mutex> guard(DeviceCmdMutex);
	VSTune_Info_Obj tuneInfo;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tuneInfo.chanNum);
	tuneInfo.tuneInfo.TunerType = TUNER_TYPE_TV;
	tuneInfo.tuneInfo.TvModulation = mode;
	tuneInfo.tuneInfo.Frequency = freq;

	bool rtn = SendCmdToServer(MSG_SET_6U_TUNE,(void*)&tuneInfo, sizeof(VSTune_Info_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(DeviceId) + string("]视频调频失败[") + \
		StrUtil::Int2Str(tuneInfo.chanNum) + string(":") + StrUtil::Int2Str(freq) + string("]") ;
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
	}
	else
	{
		//rtn = CheckDeviceIDLock();
		//if(rtn)
		{
			sysmsg = string("通道[")  + strIPAddress +string(":")+StrUtil::Int2Str(DeviceId) + string("]视频调频成功[") + \
				StrUtil::Int2Str(tuneInfo.chanNum) + string(":") + StrUtil::Int2Str(freq) + string("]") ;
			SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);
		}
	}
	return rtn;
}

/*
*6U: ATV转码设置函数，平台下发的XML中有编码后的Bps、width和height所以通过参数传递进来
*
*@Bps: 编码后的视频码率
*@width: 编码后的视频宽度,默认值为"",当不传值或传入值为""时是不是应该用数据库中的配置参数
*@height: 编码后的视频高度,默认值"",当不传值或传入值为""时是不是应该用数据库中的配置参数
*
*TODO
*确认编码的其他参数，编码控制模式是否可以作为具有默认值的参数给出
*/
bool AtvTcpDeviceAccess::SetEncodeFor6U(string Bps, string width, string height)
{
	ACE_Guard<ACE_Thread_Mutex> guard(DeviceCmdMutex);
// 	string TsIp = "238.0.0.99";
// 	int tsport=11000+DeviceId;
	//	string deviceIP = "";
	//	PROPMANAGER::instance()->GetDeviceIP(DeviceId, deviceIP);
	//	string TsIp = "238.0.0." + deviceIP.substr(deviceIP.rfind(".") + 1);
	std::string TsIp = "172.16.10.250";
	unsigned int tsport=11000+DeviceId;

	VSEncode_Param_Obj encodeObj;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,encodeObj.chanNum);		
	encodeObj.encodeParam.VideoBitrate = StrUtil::Str2Int(Bps);
	if(encodeObj.encodeParam.VideoBitrate <300000||encodeObj.encodeParam.VideoBitrate > 1500000)
	{
		encodeObj.encodeParam.VideoBitrate = 500000;
	}
	//设置默认的音频码率，可能为64K
	encodeObj.encodeParam.AudioBitrate = 64000;
	//视频码率控制模式， 默认为ENCODE_CTL_CBR-固定码率
	encodeObj.encodeParam.EncodeType = ENCODE_CTL_CBR;
	//QPValue: EncodeType为FIXQP时有效
	encodeObj.encodeParam.QPValue = 20 ;
	//设置编码后的视频Pid，待确认
	encodeObj.encodeParam.VideoPID = 66 + DeviceId;
	//设置编码后的音频Pid，待确认
	encodeObj.encodeParam.AudioPID = 2+ DeviceId;
	//设置编码后的PcrPid，待确认
	encodeObj.encodeParam.PCRPID = 55 + DeviceId;
	//设置编码后的UDPADDR，待确认
	encodeObj.encodeParam.UDPaddr =	ntohl(inet_addr(TsIp.c_str()))/*0xEF010101*/;
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
// 	if(encodeObj.encodeParam == mLastEncoderParam)
// 		return true;
// 	else
// 		mLastEncoderParam = encodeObj.encodeParam;

	bool rtn = SendCmdToServer(MSG_SET_6U_ENCODER,(void*)&encodeObj, sizeof(VSEncode_Param_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(DeviceId) + string("]转码设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(DeviceId) + string("]转码设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);
	}
	return rtn;
}

/*
*6U: ATV台标OSD设置函数
*
*@dvbtype: 接口中的DVBType，用于获取数据库中配置的OSD信息
*@name: 节目名称
*
*TODO
*由于节目名称的长度不一样，可能需要重新计算台标OSD的位置
*/
bool AtvTcpDeviceAccess::SetOSDFor6U(string dvbtype, string name)
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
	//osdInfo.osdInfo.PositionX = StrUtil::Str2Int(osd.ProgramX);
	//设置osd显示的纵坐标
	osdInfo.osdInfo.PositionY = 1;

	string tmpName = osd.Info + string(" ") + name;
	//osdInfo.osdInfo.PositionX = 720-(14*tmpName.length());//::Str2Int(osd.ProgramX )* 3;
	memset(osdInfo.osdInfo.Text, 0, MAX_OSD_INFO_TEXT_LENGTH);
	memcpy(osdInfo.osdInfo.Text, tmpName.c_str(), tmpName.length());
	
	
	osdInfo.osdInfo.PositionX = 720- StrUtil::GetStrPixelLength(osdInfo.osdInfo.Text, 20) - 64;
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
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]OSD设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);

	}
	return rtn;
}

/*
*6U: ATV时间OSD设置函数
*
*@dvbtype: 接口中的DVBType，用于获取数据库中配置的OSD信息
*@enable: 是否显示时间OSD，1是显示，0是不显示
*/
bool AtvTcpDeviceAccess::SetTimeOSDFor6U(string dvbtype, int enable)
{	
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
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]时间OSD设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);

	}
	return rtn;
}

/*
*6U: ATV频道扫描函数
*
*@startFreq 起始频点
*@endFreq   结束频点
*@scanStep  扫描步长
*@retObj    返回的数据结构
*
*TODO
*应该将扫描方式作为参数传入
*/
bool AtvTcpDeviceAccess::Channelscan(string startFreq, string endFreq, string scanStep, VSScan_Result_Obj &retObj)
{
	int pkgLen = 0;
	char sendbuf[1024] = {0}; 
	
	//信息头
	PkgHeader_Obj   ph;
	ph.header   = 0x48;
	ph.msg_type = MSG_SET_6U_SCAN_ATV;

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
	//指定Tunner类型，模拟电视的TunerTUNER_TYPE_TV
	scanParamObj.scanParam.TunerType = TUNER_TYPE_TV;
	//指定频道扫描类型，默认为SCAN_TYPE_FAST，
	//TODO: 是不是应该将扫描类型作为一个参数传入
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

	VSScan_Result_Obj scanResultObj;

	string sysmsg;

	if(1 == SendCmdToServer((void*)&sendbuf, pkgLen, (void*)&scanResultObj, sizeof(VSScan_Result_Obj)))
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]频道扫描设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
		return true;
	}else{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]频道扫描设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
		return false;
	}
	
}

//设置板卡报警灵敏度

bool AtvTcpDeviceAccess::setThresholdInfo()
{
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


// 	int value;
// 	std::vector<sAlarmParam>::iterator iter = AlarmParamVec.begin();
// 	for(; iter != AlarmParamVec.end(); iter++)
// 	{
// 		if(iter->TypeID == "18") //黑屏
// 		{
// 			value = StrUtil::Str2Int(iter->Num);
// 			thresholdObj.thresholdInfo.BlackSimilar = value;
// 		}
// 		else if(iter->TypeID == "13") //静帧
// 		{
// 			value = StrUtil::Str2Int(iter->Num);
// 			thresholdObj.thresholdInfo.FreezeSimilar  = value;//等于-1时默认不报静帧
// 		}
// 		else if(iter->TypeID == "12" ) //无伴音
// 		{
// 			value = StrUtil::Str2Int(iter->Num);
// 			thresholdObj.thresholdInfo.AudioLostValue = value;
// 
// 			value = StrUtil::Str2Int(iter->UpThreshold);
// 			thresholdObj.thresholdInfo.VolumeHighValue = value;
// 			value = StrUtil::Str2Int(iter->DownThreshold);
// 			thresholdObj.thresholdInfo.VolumeLowValue = value;
// 		}
// 		else if(iter->TypeID == "14") //彩条
// 		{
// 			value = StrUtil::Str2Int(iter->Num);
// 			thresholdObj.thresholdInfo.ColourBarSimilar  = value;
// 		}
// 	}

	//静帧报警参数设置为-1表示关闭静帧报警
	//thresholdObj.thresholdInfo.FreezeSimilar  = -1;

	bool rtn = SendCmdToServer(MSG_SET_6U_THRESHOLD,(void*)&thresholdObj, sizeof(VSThreshold_Info_Obj)); //VSThreshold_Info_Obj

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

bool AtvTcpDeviceAccess::getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj)
{
	std::cout << " AtvTcpDeviceAccess::getQualityInfoFor6UL::TODO" << std::endl;
	return true;

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
	obj.TunerType = TUNER_TYPE_TV;

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
}

bool AtvTcpDeviceAccess::setCardSystemTimeFor6U()
{
	int tunerId = -1;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tunerId);
	//如果不是录像通道，那么就不是6U板卡，直接返回
	if(!(PROPMANAGER::instance()->IsRecordDeviceID(DeviceId)))
		return true;
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

bool AtvTcpDeviceAccess::setScanControlParam()
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


bool AtvTcpDeviceAccess::LockFreqFor6U(LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj)
{
		int pkgLen = 0;
		char sendbuf[2048] = {0}; 


		//信息头
		PkgHeader_Obj   ph;
		ph.header   = 0x48;
		ph.msg_type = MSG_SET_6U_SCAN_ATV;

		memcpy(sendbuf, &ph, sizeof(PkgHeader_Obj));
		pkgLen	+= sizeof(PkgHeader_Obj);

		//信息内容
		memcpy(sendbuf+pkgLen, lockFreqArr, sizeof(LockFreq_Obj));
		pkgLen	+= sizeof(LockFreq_Obj);


		string sysmsg;

		if(SendCmdToServer((void*)&sendbuf, 2048, (void*)&retObj, sizeof(VSScan_Result_Obj)))
		{
			sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]ATV频道锁定任务执行成功");
			SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
			return true;
		}else{
			sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]ATV频道锁定任务执行失败");
			SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
			return false;
		}
}

/*
*6U: ATV音柱设置函数
*
*@enable: 是否显示音柱，1是显示，0是不显示
*/
bool AtvTcpDeviceAccess::setVolumeFor6U(int enable)
{
	VSVolume_Param_Obj volumeObj;

	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,volumeObj.chanNum);
	volumeObj.enable = enable;

	bool rtn = SendCmdToServer(MSG_SET_6U_VOLUME,(void*)&volumeObj, sizeof(VSVolume_Param_Obj));

	string sysmsg;
	if (rtn == false)
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]音柱设置失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV,VS_MSG_SYSALARM);
	}
	else
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]音柱设置成功");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,ATV);

	}
	return rtn;
}

bool AtvTcpDeviceAccess::getChanFixInfoFor6U()
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
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
		}
		else
		{
			string msg = string("通道[") + StrUtil::Int2Str(mChannelID) + std::string("]锁定成功"); 
			SYSMSGSENDER::instance()->SendMsg(msg);
		}
	}
	return rtn;
}

bool AtvTcpDeviceAccess::getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj)
{
	int pkgLen = 0;
	char sendbuf[200] = {0}; 
	VSTuner_AllChanFix_Param_Obj obj;

	//信息头
	PkgHeader_Obj   ph;
	ph.header   = 0x48;
	ph.msg_type = MSG_GET_6U_ALLCHANFIX;

	memcpy(sendbuf, &ph, sizeof(PkgHeader_Obj));
	pkgLen	+= sizeof(PkgHeader_Obj);

	obj.TunerType = TUNER_TYPE_TV;

	//信息内容
	memcpy(sendbuf+pkgLen, &obj, sizeof(VSTuner_AllChanFix_Param_Obj));
	pkgLen	+= sizeof(VSTuner_AllChanFix_Param_Obj);


	string sysmsg;

	if(1 != SendCmdToServer((void*)&sendbuf, 200, (void*)&retObj, sizeof(VSTuner_AllChanFix_Result_Obj)))
	{
		sysmsg = string("通道[")  + strIPAddress +string(":")+ StrUtil::Int2Str(mChannelID) + string("]全部指标查询返回失败");
		SYSMSGSENDER::instance()->SendMsg(sysmsg,RADIO);
		return false;
	}

	return true;
}


bool AtvTcpDeviceAccess::changeChan(std::string dvbtype, int freq, std::string Bps, std::string name, int timeEnable, int volumeEnabel, int bgEnable)
{
	VSChangeChan_Param_Obj chanInfoObj;
	eDVBType etype=OSFunction::GetEnumDVBType(dvbtype);
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,chanInfoObj.chanNum);

	chanInfoObj.tuneInfo.TunerType = TUNER_TYPE_TV;
	chanInfoObj.tuneInfo.TvModulation = MODULATION_TV_PALDK;
	chanInfoObj.tuneInfo.Frequency = freq;

//	string deviceIP = "";
//	PROPMANAGER::instance()->GetDeviceIP(DeviceId, deviceIP);
//	string TsIp = "238.0.0." + deviceIP.substr(deviceIP.rfind(".") + 1);
	std::string TsIp = "172.16.10.250";
	unsigned int tsport=11000+DeviceId;
	
	chanInfoObj.encodeInfo.VideoBitrate = StrUtil::Str2Int(Bps);
	if(chanInfoObj.encodeInfo.VideoBitrate <300000||chanInfoObj.encodeInfo.VideoBitrate > 1500000)
	{
		chanInfoObj.encodeInfo.VideoBitrate = 500000;
	}
	//设置默认的音频码率，可能为64K
	chanInfoObj.encodeInfo.AudioBitrate = 64000;
	//视频码率控制模式， 默认为ENCODE_CTL_CBR-固定码率
	chanInfoObj.encodeInfo.EncodeType =ENCODE_CTL_CBR/* ENCODE_CTL_VBR*/;
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
