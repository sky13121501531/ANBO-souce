#include "./AMHTTPDeviceAccess.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/AppLog.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Communications/SysMsgSender.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"

AMHTTPDeviceAccess::AMHTTPDeviceAccess(int iDeviceID, std::string strIP, int iPort)
:HTTPDeviceAccess(iDeviceID, strIP, iPort)
{
	setCardSystemTime();
	SetAlarmThreshold();
	//SetEncoderSwitch(false);
}

AMHTTPDeviceAccess::~AMHTTPDeviceAccess()
{

}

bool AMHTTPDeviceAccess::GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
							<Type>GetRadioQualityInfo</Type>\
							<Data>\
							<Channel>All</Channel>\
							</Data>\
							</Msg>";
	char* source="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg></Msg>";

	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	XmlParser Retparser(source);
	int freq;
	string StrFreq;
	pXMLNODE QuaryNode = parser.GetNodeFirstChild(root);
	parser.GetAttrNode(QuaryNode,"Freq",StrFreq);
	pXMLNODE SrcparamNode = parser.GetNodeFromPath("Msg/QualityParam");
	pXMLNODELIST paramList = parser.GetNodeList(SrcparamNode);
	freq =(int)(StrUtil::Str2Float(StrFreq)*1000.0);
	if(!setTunerInfo(DeviceId, freq))
	{
		pXMLNODE retrootNode=Retparser.GetRootNode();
		Retparser.SetAttrNode("DVBType",dvbtype,retrootNode);

		pXMLNODE retNode=Retparser.CreateNodePtr(retrootNode,"Return");
		Retparser.SetAttrNode("Type",string("QualityQuery"),retNode);
		Retparser.SetAttrNode("Value",string("1"),retNode);
		Retparser.SetAttrNode("Desc",string("失败"),retNode);
		Retparser.SetAttrNode("Comment",string(""),retNode);

		Retparser.SaveToString(strRetMsg);
		return false;
	}

	XmlParser srcParser;
	srcParser.Set_xml(strSrcXml);
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());

	string strDeviceXml;
	srcParser.SaveToString(strDeviceXml);
	string strRetDeviceXml;

	if(!SendXmlTaskToDevice(strDeviceXml, strRetDeviceXml))
	{
		pXMLNODE retrootNode=Retparser.GetRootNode();
		Retparser.SetAttrNode("DVBType",dvbtype,retrootNode);

		pXMLNODE retNode=Retparser.CreateNodePtr(retrootNode,"Return");
		Retparser.SetAttrNode("Type",string("QualityQuery"),retNode);
		Retparser.SetAttrNode("Value",string("1"),retNode);
		Retparser.SetAttrNode("Desc",string("失败"),retNode);
		Retparser.SetAttrNode("Comment",string(""),retNode);

		Retparser.SaveToString(strRetMsg);
		return false;
	}

	XmlParser retDeviceParser;
	retDeviceParser.Set_xml(strRetDeviceXml);
	std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
	if( strXmlRet != std::string("SUCCESS"))
	{
		pXMLNODE retrootNode=Retparser.GetRootNode();
		Retparser.SetAttrNode("DVBType",dvbtype,retrootNode);

		pXMLNODE retNode=Retparser.CreateNodePtr(retrootNode,"Return");
		Retparser.SetAttrNode("Type",string("QualityQuery"),retNode);
		Retparser.SetAttrNode("Value",string("1"),retNode);
		Retparser.SetAttrNode("Desc",string("失败"),retNode);
		Retparser.SetAttrNode("Comment",string(""),retNode);

		Retparser.SaveToString(strRetMsg);
		return false;
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

	std::string strTmpXmlText = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Data/Channel/Level"));
	pXMLNODE indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
	Retparser.SetAttrNode("Type", "1", indexNode);
	Retparser.SetAttrNode("Desc", "信号载波电平", indexNode);
	Retparser.SetAttrNode("Value", strTmpXmlText, indexNode);

	strTmpXmlText = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Data/Channel/Modulation"));

	Retparser.SetAttrNode("Type", "2", indexNode);
	Retparser.SetAttrNode("Desc", "调制度(瞬时值)", indexNode);
	Retparser.SetAttrNode("Value", strTmpXmlText, indexNode);

	indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
	Retparser.SetAttrNode("Type", "3", indexNode);
	Retparser.SetAttrNode("Desc", "音频信号电平", indexNode);
	Retparser.SetAttrNode("Value", "0", indexNode);

	indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
	Retparser.SetAttrNode("Type", "4", indexNode);
	Retparser.SetAttrNode("Desc", "信号载波频率", indexNode);
	Retparser.SetAttrNode("Value", "0", indexNode);

	indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
	Retparser.SetAttrNode("Type", "5", indexNode);
	Retparser.SetAttrNode("Desc", "谐波失真", indexNode);
	Retparser.SetAttrNode("Value", "0", indexNode);

	indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
	Retparser.SetAttrNode("Type", "6", indexNode);
	Retparser.SetAttrNode("Desc", "信噪比", indexNode);
	Retparser.SetAttrNode("Value", "0", indexNode);


	Retparser.SaveToString(strRetMsg);
	SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
	return true;
}

bool AMHTTPDeviceAccess::GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"AM\" >\
					  <Return Type=\"\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					  <ChannelScan></ChannelScan></Msg>";
	std::string retfailxml = "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							 <Msg DVBType=\"AM\" >\
							 <Return Type=\"\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
							 <ChannelScan></ChannelScan></Msg>";

	string strDeviceSrcXml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.16.10.250:11111\">\
							<Type>RadioChannelScanQuery</Type>\
							<Data>\
							<Channel>1</Channel>\
							<ScanParam>\
							<ScanType>BAND</ScanType>\
							<StartFrequency>87600</StartFrequency>\
							<EndFrequency>108000</EndFrequency>\
							<StepSize>100</StepSize>\
							</ScanParam>\
							</Data>\
							</Msg>";

	XmlParser parser;
	XmlParser deviceParser;

	deviceParser.Set_xml(strDeviceSrcXml);


	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	if(dvbtype != "RADIO")
	{
		return false;
	}

	pXMLNODE channscannode=parser.GetNodeFirstChild(childnode);
	string strstartfreq,strendfreq,strstep;
	parser.GetAttrNode(channscannode,"StartFreq",strstartfreq);
	parser.GetAttrNode(channscannode,"EndFreq",strendfreq);
	parser.GetAttrNode(channscannode,"StepFreq",strstep);

	int startfreq=(StrUtil::Str2Float(strstartfreq)+0.0002)*1000,endfreq=(StrUtil::Str2Float(strendfreq)+0.0002)*1000,
		freqstep=(StrUtil::Str2Float(strstep)+0.0002)*1000;
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/StartFrequency"), StrUtil::Int2Str(startfreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/EndFrequency"), StrUtil::Int2Str(endfreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/StepSize"), StrUtil::Int2Str(freqstep).c_str());

	string strDeviceXml;
	deviceParser.SaveToString(strDeviceXml);
	string strRetDeviceXml;

	if(!SendXmlTaskToDevice(strDeviceXml, strRetDeviceXml))
	{
		strRetMsg = retfailxml;
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}

	XmlParser retDeviceParser;
	retDeviceParser.Set_xml(strRetDeviceXml);
	std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
	if( strXmlRet != std::string("SUCCESS"))
	{
		strRetMsg= retfailxml;
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}

	XmlParser retpaser(retsuccxml.c_str());
	pXMLNODE retChanNode = retpaser.GetNodeFromPath("Msg/ChannelScan");
	pXMLNODE freqNode = NULL;
	float tmpFreq = 0.0;
	pXMLNODELIST channelNodeList = retDeviceParser.GetNodeList(retDeviceParser.GetNodeFromPath("Msg/Data/ScanResult"));

	for (int i=0; i<channelNodeList->Size(); i++)
	{
		freqNode = retDeviceParser.GetNextNode(channelNodeList);
		tmpFreq = (float)(StrUtil::Str2Int(retDeviceParser.GetNodeText(freqNode)))/1000;

		pXMLNODE freqnode = retpaser.CreateNodePtr(retChanNode,"Channel");
		retpaser.SetAttrNode("Freq", StrUtil::Float2Str(tmpFreq), freqnode);
	}

	retpaser.SaveToString(strRetMsg);
	std::cout<<"接收机扫描结果："<<strRetMsg<<endl;

	return true;
}

bool AMHTTPDeviceAccess::GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"AM\" >\
					  <Return Type=\"TSQuery\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \</Msg>";
	string retfailxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"AM\" >\
					  <Return Type=\"TSQuery\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \</Msg>";

	string strDeviceSrcXml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\">\
							<Type>ChangeProgramInfo</Type>\
							<Data>\
							<Channel>1</Channel>\
							<Tuner>\
							<TunerType>TV</TunerType>\
							<Frequency>200250</Frequency>\
							<ModulationType>PALDK</ModulationType>\
							</Tuner>\
							<Encoder>\
							<VideoBitrate>500000</VideoBitrate>\
							<TsIP>192.168.1.230</TsIP>\
							<TsPort>9000</TsPort>\
							<Stop>1</Stop>\
							</Encoder>\
							<OSD>\
							<Text>CCTV-1</Text>\
							<PositionX>100</PositionX>\
							<PositionY>1</PositionY>\
							<Align>RIGHT</Align>\
							</OSD>\
							<TimeStamp>\
							<Enable>1</Enable>\
							<PositionX>464</PositionX>\
							<PositionY>40</PositionY>\
							<Align>RIGHT</Align>\
							</TimeStamp>\
							<Volume>\
							<Enable>1</Enable>\
							</Volume>\
							</Data>\
							</Msg>";
	XmlParser deviceParser;
	deviceParser.Set_xml(strDeviceSrcXml);

	bool  ret = true;
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	pXMLNODE tsnode=parser.GetNodeFromPath("Msg/TSQuery/TS");
	string freq,bps, width, height, strStopEncoder;
	parser.GetAttrNode(tsnode,"Bps",bps);
	parser.GetAttrNode(tsnode,"Freq",freq);
	parser.GetAttrNode(tsnode,"StopEncoder",strStopEncoder);

	string name;
	OSDInfo getDbInfo;
	CHANNELINFOMGR::instance()->GetProNameByFreq(OSFunction::GetEnumDVBType(dvbtype),freq,name);
	PROPMANAGER::instance()->GetOSDInfo(OSFunction::GetEnumDVBType(dvbtype), "0", getDbInfo);

	if ((name.empty() && name.length()< 39))
	{
		name = string("_未知频道_") + freq +"MHz";
	}
	name = getDbInfo.Info + std::string("_") + name;

	int tuneFreq = StrUtil::Str2Float(freq)*1000;
	std::string TsIp;
	int tsport;
	PROPMANAGER::instance()->GetDeviceTsIP(DeviceId, TsIp);
	PROPMANAGER::instance()->GetDeviceTsPort(DeviceId, tsport);

	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Tuner/TunerType"), "Radio");
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Tuner/Frequency"), StrUtil::Int2Str(tuneFreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Tuner/ModulationType"), "AM");

	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Encoder/VideoBitrate"), bps.c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Encoder/TsIP"), TsIp.c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Encoder/TsPort"), StrUtil::Int2Str(tsport).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Encoder/Stop"), strStopEncoder.c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/OSD/Text"), name.c_str());
	char osdBuf[MAX_OSD_INFO_TEXT_LENGTH] = {0};
	memcpy(osdBuf, name.c_str(), name.length());
	int osdPosX = 720- StrUtil::GetStrPixelLength(osdBuf, 20) - 64;
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/OSD/PositionX"), StrUtil::Int2Str(628).c_str());
	//deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/OSD/PositionX"), "350");
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/OSD/PositionY"), "1");
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/OSD/Align"), "RIGHT");

	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/TimeStamp/Enable"), "1");
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/TimeStamp/PositionX"), "628");
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/TimeStamp/PositionY"), "40");
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/TimeStamp/Align"), "RIGHT");

	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Volume/Enable"), "0");

	string strDeviceXml;
	deviceParser.SaveToString(strDeviceXml);
	string strRetDeviceXml;

	if(!SendXmlTaskToDevice(strDeviceXml, strRetDeviceXml))
	{
		strRetMsg= retfailxml;
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}

	XmlParser retDeviceParser;
	retDeviceParser.Set_xml(strRetDeviceXml);
	std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
	if( strXmlRet != std::string("SUCCESS"))
	{
		strRetMsg= retfailxml;
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}
	strRetMsg= retsuccxml;
	return true;
}

bool AMHTTPDeviceAccess::GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	return true;
}

bool AMHTTPDeviceAccess::setCardSystemTime()
{
	std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
							<Type>SetSystemTimeInfo</Type>\
							<Data>\
							<Time>1533526012</Time>\
							</Data>\
							</Msg>";

	std::string strDevIP;
	std::list<int> devIDList;

	PROPMANAGER::instance()->GetDeviceIP(DeviceId, strDevIP);
	if(strDevIP.size() <= 0)
		return false;

	PROPMANAGER::instance()->GetDeviceIDByIP(strDevIP, devIDList);
	if(devIDList.size() <= 0)
		return false;

	//如果不是该IP板卡上的第一个通道，不做系统时间设置
	if(devIDList.front() != DeviceId)
		return false;

	XmlParser srcParser;
	srcParser.Set_xml(strSrcXml);
	time_t now = time(0) + 2;
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Time"), StrUtil::Long2Str(now).c_str());

	string strDeviceXml;
	srcParser.SaveToString(strDeviceXml);
	string strRetDeviceXml;

	if(!SendXmlTaskToDeviceNoBlock(strDeviceXml, strRetDeviceXml))
	{
		return false;
	}

	XmlParser retDeviceParser;
	retDeviceParser.Set_xml(strRetDeviceXml);
	std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
	if( strXmlRet != std::string("SUCCESS"))
	{
		return false;
	}

	return true;

}

bool AMHTTPDeviceAccess::getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj)
{
	std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
							<Type>GetRadioQualityInfo</Type>\
							<Data>\
							<Channel>All</Channel>\
							</Data>\
							</Msg>";
	try
	{
		XmlParser srcParser;
		srcParser.Set_xml(strSrcXml);
		int tmpDeviceID = 0;
		PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
		srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());

		string strDeviceXml;
		srcParser.SaveToString(strDeviceXml);
		string strRetDeviceXml;

		if(!SendXmlTaskToDevice(strDeviceXml, strRetDeviceXml))
		{
			return false;
		}

		XmlParser retDeviceParser;
		retDeviceParser.Set_xml(strRetDeviceXml);
		std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
		if( strXmlRet != std::string("SUCCESS"))
		{
			return false;
		}

		pXMLNODELIST qualityNodeList = retDeviceParser.GetNodeList(retDeviceParser.GetNodeFromPath("Msg/Data"));
		pXMLNODE tmpNode = retDeviceParser.GetNextNode(qualityNodeList);
		retObj.tunerQualityObj.status = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Status")));
		retObj.tunerQualityObj.level = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Level")));
		retObj.tunerQualityObj.usn = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Usn")));
		retObj.tunerQualityObj.wam = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Wam")));
		retObj.tunerQualityObj.offset = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Offset")));
		retObj.tunerQualityObj.bandwith = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Bandwith")));
		retObj.tunerQualityObj.modulation = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Modulation")));
	}
	catch (...)
	{
		return false;
	}
	
	return true;
}

bool AMHTTPDeviceAccess::getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj)
{
	std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
							<Type>GetRadioQualityInfo</Type>\
							<Data>\
							<Channel>All</Channel>\
							</Data>\
							</Msg>";
	try
	{
		XmlParser srcParser;
		srcParser.Set_xml(strSrcXml);
		int tmpDeviceID = 0;
		PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
		srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), "ALL");

		string strDeviceXml;
		srcParser.SaveToString(strDeviceXml);
		string strRetDeviceXml;

		if(!SendXmlTaskToDevice(strDeviceXml, strRetDeviceXml))
		{
			return false;
		}

		XmlParser retDeviceParser;
		retDeviceParser.Set_xml(strRetDeviceXml);
		std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
		if( strXmlRet != std::string("SUCCESS"))
		{
			return false;
		}

		pXMLNODELIST qualityNodeList = retDeviceParser.GetNodeList(retDeviceParser.GetNodeFromPath("Msg/Data"));
		for(int i = 0; i < qualityNodeList->Size(); i++)
		{
			pXMLNODE tmpNode = retDeviceParser.GetNextNode(qualityNodeList);
			retObj.tunerQualityObj[i].status = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Status")));
			retObj.tunerQualityObj[i].level = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Level")));
			retObj.tunerQualityObj[i].usn = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Usn")));
			retObj.tunerQualityObj[i].wam = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Wam")));
			retObj.tunerQualityObj[i].offset = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Offset")));
			retObj.tunerQualityObj[i].bandwith = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Bandwith")));
			retObj.tunerQualityObj[i].modulation = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Modulation")));
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool AMHTTPDeviceAccess::setTunerInfo(int chanNo, int freq)
{
	std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"gb2312\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
							<Type>SetTunerInfo</Type>\
							<Data>\
							<Channel>1</Channel>\
							<Tuner>\
							<Frequency>714000</Frequency>\
							<TunerType>0</TunerType>\
							<ModulationType>0</ModulationType>\
							</Tuner>\
							</Data>\
							</Msg>";

	XmlParser srcParser;
	srcParser.Set_xml(strSrcXml);
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Tuner/TunerType"), "RADIO");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Tuner/Frequency"), StrUtil::Int2Str(freq).c_str());
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Tuner/ModulationType"), "AM");


	string strDeviceXml;
	srcParser.SaveToString(strDeviceXml);
	string strRetDeviceXml;

	if(!SendXmlTaskToDevice(strDeviceXml, strRetDeviceXml))
	{
		return false;
	}

	XmlParser retDeviceParser;
	retDeviceParser.Set_xml(strRetDeviceXml);
	std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
	if( strXmlRet != std::string("SUCCESS"))
	{
		return false;
	}
	return true;
}

bool AMHTTPDeviceAccess::SetEncoderSwitch(bool enable)
{
	std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"gb2312\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
							<Type>SetEncoderSwitchInfo</Type>\
							<Data>\
							<Channel>1</Channel>\
							<Switch>1</Switch>\
							</Data>\
							</Msg>";


	XmlParser srcParser;
	srcParser.Set_xml(strSrcXml);
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	if(enable)
	{
		srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Switch"), "1");
	}
	else
	{
		srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Switch"), "0");
	}

	string strDeviceXml;
	srcParser.SaveToString(strDeviceXml);
	string strRetDeviceXml;

	if(!SendXmlTaskToDeviceNoBlock(strDeviceXml, strRetDeviceXml))
	{
		return false;
	}

	XmlParser retDeviceParser;
	retDeviceParser.Set_xml(strRetDeviceXml);
	std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
	if( strXmlRet != std::string("SUCCESS"))
	{
		return false;
	}
	return true;
}
bool AMHTTPDeviceAccess::SetAlarmThreshold()
{
	std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
							<Type>SetThresholdInfo</Type>\
							<Data>\
							<!-- 具体参数 -->\
							<Channel>All</Channel>\
							<ThresholdInfo>\
							<BlackSimilar>998</BlackSimilar>\
							<ColourBarSimilar>998</ColourBarSimilar>\
							<FreezeSimilar>998</FreezeSimilar>\
							<VolumeHighValue>70</VolumeHighValue>\
							<VolumeLowValue>5</VolumeLowValue>\
							<AudioLostValue>3</AudioLostValue>\
							<AudioUnsualLastTime>200</AudioUnsualLastTime>\
							</ThresholdInfo>\
							</Data>\
							</Msg>";
	std::string strDevIP;
	std::list<int> devIDList;

	PROPMANAGER::instance()->GetDeviceIP(DeviceId, strDevIP);
	if(strDevIP.size() <= 0)
		return false;

	PROPMANAGER::instance()->GetDeviceIDByIP(strDevIP, devIDList);
	if(devIDList.size() <= 0)
		return false;

	//如果不是该IP板卡上的第一个通道，不做报警门限设置
	if(devIDList.front() != DeviceId)
		return false;


	XmlParser srcParser;
	srcParser.Set_xml(strSrcXml);
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/BlackSimilar"), "998");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/ColourBarSimilar"), "998");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/FreezeSimilar"), "998");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/VolumeHighValue"), "80");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/VolumeLowValue"), "5");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/AudioLostValue"), "0");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/AudioUnsualLastTime"), "200");

	string strDeviceXml;
	srcParser.SaveToString(strDeviceXml);
	string strRetDeviceXml;

	if(!SendXmlTaskToDeviceNoBlock(strDeviceXml, strRetDeviceXml))
	{
		return false;
	}

	XmlParser retDeviceParser;
	retDeviceParser.Set_xml(strRetDeviceXml);
	std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
	if( strXmlRet != std::string("SUCCESS"))
	{
		return false;
	}
	return true;
}