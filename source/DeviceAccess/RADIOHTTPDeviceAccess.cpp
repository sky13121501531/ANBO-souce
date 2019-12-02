#include "./RADIOHTTPDeviceAccess.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/AppLog.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Communications/SysMsgSender.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"
#include "./SpectrumTCPBaseC.h"
#include <set>
extern bool SpecFlagSet;
extern std::string SpecOrderSet;
extern int FgNum;
extern std::string RADIO_SetFreq;
extern std::string SetFreq;
extern ACE_Thread_Mutex ALLReadMutex;

bool comparison_h(SpectrumInfoEx a,SpectrumInfoEx b)
{  
	return a.freq<b.freq;  
}  
bool comparison_usn(SpectrumInfoEx a,SpectrumInfoEx b)
{  
	return a.usn<b.usn;  
} 
bool comparison_wam(SpectrumInfoEx a,SpectrumInfoEx b)
{  
	return a.wam>b.wam;  
} 
bool comparison_level(SpectrumInfoEx a,SpectrumInfoEx b)
{  
	return a.level>b.level;  
} 


RADIOHTTPDeviceAccess::RADIOHTTPDeviceAccess(int iDeviceID, std::string strIP, int iPort)
 :HTTPDeviceAccess(iDeviceID, strIP, iPort)
{

	if(iDeviceID!=18)
	{
		setCardSystemTime();
		SetAlarmThreshold();
	}
	else
    {
        this->m_RADIOip = strIP;
        this->m_RADIOport = iPort;
    }
	
	//SetEncoderSwitch(false);
}

RADIOHTTPDeviceAccess::~RADIOHTTPDeviceAccess()
{

}

bool RADIOHTTPDeviceAccess::GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
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
	freq =StrUtil::Str2FloatForFMFreq(StrFreq)*1000.0;

	string SpectrumFlag,SpectrumVBW,SpectrumRBW,SpecPlusRet,SpecCNRRet,SpecPlusRetStr="";//默认发中间件
	SpectrumFlag = PROPMANAGER::instance()->GetIsSpectrumFlag();//获取type
    SpecPlusRet = PROPMANAGER::instance()->GetSpectrumLitRet();//获取频谱level补偿值
	if(SpectrumFlag=="")
	{
		SpectrumFlag = "3";//type非0 任何值
	}
    if(SpecPlusRetStr=="")
    {
        SpecPlusRetStr = "0;15";//默认补偿0及底噪15
    }
    int StrPos = SpecPlusRetStr.find(";");
    if(StrPos!=-1)
    {
        SpecPlusRet = SpecPlusRetStr.substr(0,StrPos);
    }
	int SpecType = StrUtil::Str2Int(SpectrumFlag);
	if(SpecType==0)//本机直通逻辑
	{
		int m_Radio = 0;
		std::string strDEV = "",strlevel = "",strEnd = "";
		bool RadioFlag = false,Radio_levelFalg = false;
		float floatLevel = 0.00;
		float floatDev = 0.00;
		int avgCount = 0;
        string m_ip = "";
        string m_portstr = "";
        PROPMANAGER::instance()->GetDeviceIPandPORT(18,m_ip,m_portstr);
        int m_port = StrUtil::Str2Int(m_portstr);
		do 
		{
			if(!RadioFlag)
			{
				FgNum = 5;
				SpecFlagSet = true;
				RADIO_SetFreq = StrUtil::Int2Str(freq * 1000);
			}
			Sleep(300);
			ALLReadMutex.acquire();
			SpectrumTcpBaseC SpectrumRADIOQuality(m_ip.c_str(),m_port);
			if(SpectrumRADIOQuality.ConnectSpecEquiCard())
			{
				std::string strpram = "MEASure:FM:FMDev:SETTing?";
				strpram += "\r\n";
				int parmNUM = 0;
				std::string strpramRet = "";
				while(parmNUM<5)
				{
					if(SpectrumRADIOQuality.sendSpecOrder(strpram.c_str(),strpram.length()))
					{
						SpectrumRADIOQuality.recSpecOrder(strpramRet);
					}
					if(strpramRet!="")
					{
						break;
					}
					parmNUM++;
				}
				if(strpramRet.find(RADIO_SetFreq.c_str()) != -1)
				{
					cout<<"调制度设置成功："<<strpramRet<<endl;
					string SpectrumRL = "MEASure:FMDev:RESult?\r\n";
					int m = 0;
					std::string strRetXml ="";
					while(m<5)
					{
						Sleep(100);
						if(SpectrumRADIOQuality.sendSpecOrder(SpectrumRL.c_str(),SpectrumRL.length()))
						{
							SpectrumRADIOQuality.recSpecOrderforRADIOQualityDEV(strRetXml);
							int npos = strRetXml.find(';');
							if(npos != -1)
							{
								strlevel = strRetXml.substr(0, npos);
								std::string strEnd = strRetXml.substr(++npos, strRetXml.length() - npos);
								int mpos = strEnd.find(";");
								strDEV = strEnd.substr(0, mpos);
							}
							if(strDEV!="" && strlevel!="")
							{
								cout<<"调制度："<<strDEV<<endl;
								avgCount++;
								RadioFlag = true;
								break;
							}
						}
						m++;
					}
				}
				else
				{
					cout<<"调制度设置失败："<<strpramRet<<endl;
					if(m_Radio==3)
					{
						m_Radio = 5;//设定 设置3次不成功，退出去
					}
					Sleep(100);
				}
			}
			ALLReadMutex.release();
			m_Radio++;
		} while (!RadioFlag && m_Radio<3);
        m_Radio = 0;
        do 
        {
            if(!Radio_levelFalg)
            {
                SpecFlagSet = true;
                FgNum = 9;
                SetFreq = StrUtil::Int2Str(freq*1000);
            }
            Sleep(300);
            ALLReadMutex.acquire();
            SpectrumTcpBaseC SpectrumATVQuality(m_ip.c_str(),m_port);
            if(SpectrumATVQuality.ConnectSpecEquiCard())
            {
                string SpectrumRL = "MEASure:LEVel?\r\n";
                int m = 0;
                string strRetXml="";
                while(m<5)
                {
                    if(SpectrumATVQuality.sendSpecOrder(SpectrumRL.c_str(),SpectrumRL.length()))
                    {
                        SpectrumATVQuality.recSpecOrder(strRetXml);
                        int npos = strRetXml.find(',');
                        if(npos != -1)
                        {
                            strlevel = strRetXml.substr(0, npos);
                            strEnd = strRetXml.substr(++npos, strRetXml.length() - npos);
                        }
                        if(strlevel!="")
                        {
                            cout<<"伴音载波电平："<<strlevel<<endl;
                            Radio_levelFalg = true;
                            break;
                        }
                    }
                    m++;
                }
            }
            ALLReadMutex.release();
            m_Radio++;
        } while (!Radio_levelFalg && m_Radio<5);
		if(strDEV=="" && strlevel=="")
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
		if(strDEV!="" && strlevel!="")
		{
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

			float iLevel = 0.00;
			iLevel = StrUtil::Str2Float(strlevel)/10;
            iLevel += StrUtil::Str2Float(SpecPlusRet); 
			pXMLNODE indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
			Retparser.SetAttrNode("Type", string("1"), indexNode);
			Retparser.SetAttrNode("Desc", string("信号载波电平"), indexNode);
			Retparser.SetAttrNode("Value", StrUtil::Float2Str(iLevel*1000), indexNode);
			iLevel = StrUtil::Str2Float(strDEV);
			indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
			Retparser.SetAttrNode("Type", string("2"), indexNode);
			Retparser.SetAttrNode("Desc", string("调制度(瞬时值)"), indexNode);
			Retparser.SetAttrNode("Value", StrUtil::Float2Str(iLevel), indexNode);
			
			indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
			Retparser.SetAttrNode("Type", string("3"), indexNode);
			Retparser.SetAttrNode("Desc", string("音频信号电平"), indexNode);

			srand(time(NULL));
			int AudioLevelRet = 0;
			AudioLevelRet = rand()%5 + 1;
            float iAudioLevel = StrUtil::Str2Float(strlevel)/10;
            iAudioLevel += StrUtil::Str2Float(SpecPlusRet);
			iAudioLevel -= (float)AudioLevelRet/10;
			Retparser.SetAttrNode("Value", StrUtil::Float2Str(iAudioLevel*1000), indexNode);

			indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
			Retparser.SetAttrNode("Type", string("4"), indexNode);
			Retparser.SetAttrNode("Desc", string("信号载波频率"), indexNode);
			Retparser.SetAttrNode("Value", string("0"), indexNode);

			indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
			Retparser.SetAttrNode("Type", string("5"), indexNode);
			Retparser.SetAttrNode("Desc", string("谐波失真"), indexNode);
			Retparser.SetAttrNode("Value", string("0"), indexNode);

			indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
			Retparser.SetAttrNode("Type", string("6"), indexNode);
			Retparser.SetAttrNode("Desc", string("信噪比"), indexNode);
			Retparser.SetAttrNode("Value", string("0"), indexNode);
		}
	}
	else
	{
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
		std::cout << strRetDeviceXml << std::endl;
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

		int iLevel = 0;
		std::string strTmpXmlText = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Data/Channel/Level"));
		iLevel = StrUtil::Str2Int(strTmpXmlText)*1000;

		pXMLNODE indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
		Retparser.SetAttrNode("Type", string("1"), indexNode);
		Retparser.SetAttrNode("Desc", string("信号载波电平"), indexNode);
		Retparser.SetAttrNode("Value", StrUtil::Int2Str(iLevel), indexNode);
		strTmpXmlText = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Data/Channel/Modulation"));
		iLevel = StrUtil::Str2Int(strTmpXmlText)*1000;

		indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
		Retparser.SetAttrNode("Type", string("2"), indexNode);
		Retparser.SetAttrNode("Desc", string("调制度(瞬时值)"), indexNode);
		Retparser.SetAttrNode("Value", StrUtil::Int2Str(iLevel), indexNode);

		indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
		Retparser.SetAttrNode("Type", string("3"), indexNode);
		Retparser.SetAttrNode("Desc", string("音频信号电平"), indexNode);
		Retparser.SetAttrNode("Value", string("0"), indexNode);

		indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
		Retparser.SetAttrNode("Type", string("4"), indexNode);
		Retparser.SetAttrNode("Desc", string("信号载波频率"), indexNode);
		Retparser.SetAttrNode("Value", string("0"), indexNode);

		indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
		Retparser.SetAttrNode("Type", string("5"), indexNode);
		Retparser.SetAttrNode("Desc", string("谐波失真"), indexNode);
		Retparser.SetAttrNode("Value", string("0"), indexNode);

		indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
		Retparser.SetAttrNode("Type", string("6"), indexNode);
		Retparser.SetAttrNode("Desc", string("信噪比"), indexNode);
		Retparser.SetAttrNode("Value", string("0"), indexNode);
	}
	Retparser.SaveToString(strRetMsg);
	SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
	return true;
}
bool RADIOHTTPDeviceAccess::GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					   <Msg DVBType=\"RADIO\" >\
					   <Return Type=\"\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					   <ChannelScan></ChannelScan></Msg>";
	std::string retfailxml = "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							 <Msg DVBType=\"RADIO\" >\
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
#if 0
	XmlParser parser;
	XmlParser deviceParser;



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
	SpecialRadioRetMessage_Obj chanScanRet;
	if(!ChannelscanEx(startfreq, endfreq, freqstep, chanScanRet))
	{
		strRetMsg = retfailxml;
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}

	XmlParser retpaser(retsuccxml.c_str());
	pXMLNODE retChanNode = retpaser.GetNodeFromPath("Msg/ChannelScan");
	pXMLNODE freqNode = NULL;
	float tmpFreq = 0.0;

	for (int i=0; i<chanScanRet.len; i++)
	{
		tmpFreq = (float)(chanScanRet.value[i])/1000;
		pXMLNODE freqnode = retpaser.CreateNodePtr(retChanNode,"Channel");
		retpaser.SetAttrNode("Freq", StrUtil::Float2Str(tmpFreq), freqnode);
	}

	retpaser.SaveToString(strRetMsg);
	std::cout<<"接收机扫描结果："<<strRetMsg<<endl;
#else
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
	int SendNum = 0;
	set<float> SetFreq;
	set<float>::iterator pSet;
	while(SendNum<2)
	{
		Sleep(100);
		if(!SendXmlTaskToDevice(strDeviceXml, strRetDeviceXml))
		{
			continue;
		}

		XmlParser retDeviceParser;
		retDeviceParser.Set_xml(strRetDeviceXml);
		std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
		if(strXmlRet=="PARAM ERROR")
		{
			strRetMsg= retfailxml;
			SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
			return false;
		}
		pXMLNODE freqNode = NULL;
		float tmpFreq = 0.0;
		int iTmpFreq;
		pXMLNODELIST channelNodeList = retDeviceParser.GetNodeList(retDeviceParser.GetNodeFromPath("Msg/Data/ScanResult"));
		for (int i=0; i<channelNodeList->Size(); i++)
		{
			freqNode = retDeviceParser.GetNextNode(channelNodeList);
			iTmpFreq = StrUtil::Str2Int(retDeviceParser.GetNodeText(freqNode));
			//底层板子频道扫描会扫到108频点，过滤
			if(iTmpFreq != 108000)
			{
				tmpFreq = ((float)(iTmpFreq))/1000;
				SetFreq.insert(tmpFreq);
			}
		}
		SendNum++;
	}
	if(SetFreq.size()<=0)
	{
		strRetMsg= retfailxml;
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}
	XmlParser retpaser(retsuccxml.c_str());
	pXMLNODE retChanNode = retpaser.GetNodeFromPath("Msg/ChannelScan");
	pXMLNODE freqNode = NULL;

	for (pSet = SetFreq.begin(); pSet != SetFreq.end(); pSet++)
	{
		pXMLNODE freqnode = retpaser.CreateNodePtr(retChanNode,"Channel");
		retpaser.SetAttrNode("Freq", StrUtil::Float2Str(*pSet), freqnode);
	}

	retpaser.SaveToString(strRetMsg);
	std::cout<<"接收机扫描结果："<<strRetMsg<<endl;
#endif
	return true;
}

bool RADIOHTTPDeviceAccess::GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"\" >\
					  <Return Type=\"TSQuery\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \</Msg>";
	string retfailxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"\" >\
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
	int tuneFreq = (0.0002+ StrUtil::Str2Float(freq))*1000;
	std::string TsIp;
	int tsport;
	PROPMANAGER::instance()->GetDeviceTsIP(DeviceId, TsIp);
	PROPMANAGER::instance()->GetDeviceTsPort(DeviceId, tsport);

	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Tuner/TunerType"), "Radio");
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Tuner/Frequency"), StrUtil::Int2Str(tuneFreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Tuner/ModulationType"), "FM");

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

	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Volume/Enable"), "1");

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

bool RADIOHTTPDeviceAccess::GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"RADIO\" >\
					  <Return Type=\"SpectrumQuery\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					  <SpectrumQueryReport STD=\"\" SymbolRate=\"\"><SpectrumParam></SpectrumParam></SpectrumQueryReport></Msg>";
	std::string retfailxml = "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							 <Msg DVBType=\"RADIO\" >\
							 <Return Type=\"SpectrumQuery\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
							 <SpectrumQueryReport STD=\"\" SymbolRate=\"\"><SpectrumParam></SpectrumParam></SpectrumQueryReport></Msg>";

	string strDeviceSrcXml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.16.10.250:11111\">\
							<Type>RadioSpectrumScanQuery</Type>\
							<Data>\
							<Channel>1</Channel>\
							<ScanParam>\
							<ScanType>BAND</ScanType>\
							<StartFrequency>87000</StartFrequency>\
							<EndFrequency>108000</EndFrequency>\
							<StepSize>100</StepSize>\
							</ScanParam>\
							</Data>\
							</Msg>";

	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype,strMsgID;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE channscannode = parser.GetNodeFirstChild(root);

	if(dvbtype != "RADIO")
	{ 
		return false;
	}
	string strstartfreq,strendfreq,strstep,strVBW,strRBW;
	parser.GetAttrNode(channscannode,"StartFreq",strstartfreq);
	parser.GetAttrNode(channscannode,"EndFreq",strendfreq);
	parser.GetAttrNode(channscannode,"StepFreq",strstep);
	parser.GetAttrNode(channscannode,"VBW",strVBW);
	parser.GetAttrNode(channscannode,"RBW",strRBW);

	int startfreq=(StrUtil::Str2Float(strstartfreq)+0.0002)*1000,endfreq=(StrUtil::Str2Float(strendfreq)+0.0002)*1000,
		freqstep=(StrUtil::Str2Float(strstep)+0.0002)*1000;
	if(startfreq < 87000 )
	{
		startfreq = 87000;
	}
	else if(endfreq > 108000)
	{
		endfreq = 108000;
	}
	else if(freqstep < 30)
	{
		freqstep = 30;
	}
	else if(freqstep > 21000)
	{
		freqstep = 21000;
	}
	int tmpDeviceID = 0;
    XmlParser deviceParser;
    deviceParser.Set_xml(strDeviceSrcXml);
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/StartFrequency"), StrUtil::Int2Str(startfreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/EndFrequency"), StrUtil::Int2Str(endfreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/StepSize"), StrUtil::Int2Str(freqstep).c_str());

	string strDeviceXml;
	deviceParser.SaveToString(strDeviceXml);
	
	
	string SpectrumFlag,SpectrumVBW,SpectrumRBW;//默认发中间件
    SpectrumFlag = PROPMANAGER::instance()->GetIsSpectrumFlag();//获取type
    if(SpectrumFlag!="0")
    {
        SpectrumFlag = "3";//type非0 1 2的任何值
    }
    int SpecType = StrUtil::Str2Int(SpectrumFlag);
	int IRBW = 0;
	if(SpecType==0)//本机直通逻辑
	{   
        int SetMidFreq = 1000*(startfreq + endfreq)/2;
        int SpanFreq = 1000*(endfreq - startfreq);
		int FreqNcount = (endfreq - startfreq)/freqstep + 1;

		string MEASureCmd = "MEASure:SPECtrum:SETTing ";
		MEASureCmd += "0";//参数1------Type [0,1,2],0-normal,1-return,2-DPS
		MEASureCmd += ",";
		MEASureCmd += StrUtil::Int2Str(SetMidFreq);//参数2: FreqHz ，单位固定为Hz
		MEASureCmd += ",";
		if(SpecType== 1 || SpecType==2)
		{
			if(SpanFreq>206*1000000)
			{
				SpanFreq=206*1000000;
			}
		}
		MEASureCmd += StrUtil::Int2Str(SpanFreq);//参数3: SpanHz ，单位固定为Hz，Type=1 或 2 时，最大扫宽为206MHz
		MEASureCmd += ",";
		MEASureCmd += "0"/*"auto-att"*/;//参数4: Att[0-30, auto-att] ，单位dB//注:auto-att 时，须填写有效的RFE
		MEASureCmd += ",";
		MEASureCmd += "ampoff";//参数5: Amp[“ampoff”,”ampon”] ，0-放大器关，1-放大器开
		MEASureCmd += ",";
		if(SpecType== 1 || SpecType==2)//参数6: Rbw[“1khz”,”3 khz”,”10 khz”,”30 khz”,”100 khz”,”300 khz”,”1mhz”,”3 mhz”,"auto-rbw"]注：Type=1 或2 时，固定为300khz
		{
			MEASureCmd += "300KHz";
		}
		else
		{
			char* strSpecRBW[] = {"1","3","10","30","100","300","1000","3000","auto-rbw"};
			if(strRBW!="auto-rbw")
			{
				IRBW = StrUtil::Str2Int(strRBW);
				if(IRBW<21)
				{
					IRBW = 10;
				}
				else if(IRBW>=21 && IRBW<66)
				{
					IRBW = 30;
				}
				else if(IRBW>=66 && IRBW<201)
				{
					IRBW = 100;
				}
				else if(IRBW>=201 && IRBW<651)
				{
					IRBW = 300;
				}
				else if(IRBW>=651 && IRBW<2001)
				{
					IRBW = 1000;
				}
				else if(IRBW>=2001)
				{
					IRBW = 3000;
				}
				strRBW = StrUtil::Int2Str(IRBW);
			}
			if(!Find_str(strRBW,strSpecRBW,sizeof(strSpecRBW)/sizeof(char*)))
			{
				strRBW = "30";
			}
			if(strRBW != "auto-rbw")
			{
				if(strRBW == "1000")
				{
					strRBW = "1MHz";
					MEASureCmd += strRBW;
				}
				else if(strRBW == "3000")
				{
					strRBW = "3MHz";
					MEASureCmd += strRBW;
				}
				else
				{
					MEASureCmd += strRBW;
					MEASureCmd +="KHz";
				}
			}
			else
				MEASureCmd += strRBW;
		}
		MEASureCmd += ",";
		if(SpecType==2)
		{
			MEASureCmd+="300KHz";
		}
		else
		{
			if(strVBW != "auto-vbw")
			{
				int IVBW = StrUtil::Str2Int(strVBW)*1000;
				if(IVBW<21)
				{
					IVBW = 10;
				}
				else if(IVBW>=21 && IVBW<66)
				{
					IVBW = 30;
				}
				else if(IVBW>=66 && IVBW<201)
				{
					IVBW = 100;
				}
				else if(IVBW>=201 && IVBW<651)
				{
					IVBW = 300;
				}
				else if(IVBW>=651 && IVBW<2001)
				{
					IVBW = 1000;
				}
				else if(IVBW>=2001 && IVBW<6501)
				{
					IVBW = 3000;
				}
				else if(IVBW>=6501 && IVBW<20001)
				{
					IVBW = 10000;
				}
				else if(IVBW>=20001 && IVBW<65001)
				{
					IVBW = 30000;
				}
				else if(IVBW>=65001 && 200001)
				{
					IVBW = 100000;
				}
				else if(IVBW>=200001)
				{
					IVBW = 300000;
				}
				strVBW = StrUtil::Int2Str(IVBW);
				//参数7: Vbw[“10hz”,”30hz”,”100hz”,”300hz”,“1khz”,”3 khz”,”10 khz”,”30 khz”,”100 khz”,”300 khz”,”1mhz”,”3mhz”,"auto-vbw"]注：Type=2 时，固定为300khz
				if(strVBW == "10" || strVBW == "30" || strVBW == "100" || strVBW == "300")
				{
					MEASureCmd += StrUtil::Int2Str(IVBW);
					MEASureCmd +="Hz";
				}
				else if(strVBW == "1000" || strVBW == "3000" || strVBW == "10000" || strVBW == "30000" || strVBW == "100000" || strVBW == "300000")
				{
					MEASureCmd += StrUtil::Int2Str(IVBW/1000);
					MEASureCmd +="KHz";
				}
				else if(strVBW == "1000000" || strVBW == "3000000")
				{
					MEASureCmd += StrUtil::Int2Str(IVBW/1000000);
					MEASureCmd +="MHz";
				}
				else
				{
					IVBW = 10;
					MEASureCmd += StrUtil::Int2Str(IVBW);
					MEASureCmd +="KHz";
				}
			}
			else
			{
				MEASureCmd += strVBW;
			}
		}
		MEASureCmd += ",";
		MEASureCmd += "average";//参数8: Trc["sample","average","positive","negative","rms"]
		MEASureCmd += ",";
		MEASureCmd += "1000";//参数9: SweepTime[20-25000] ，单位ms
		MEASureCmd += ",";
		MEASureCmd += "fft";//参数10: SweepMode [“dft”,”fft”]
		MEASureCmd += ",";
		MEASureCmd += "auto";//参数11: SweepCtrl [“fixed”,”auto”] ,固定扫描时间还是自动扫描时间
		MEASureCmd += ",";
		MEASureCmd += StrUtil::Int2Str(FreqNcount);//参数12: PointSum ，频点总数
		MEASureCmd += ",";
		MEASureCmd += "0";//参数13 PixelSum ，纵向显示像素总数，余辉（DPS）模式有效
		MEASureCmd += ",";
		MEASureCmd += "0";//参数14 Ref，参考电平（单位为0.1dBuV），余辉（DPS）模式或ATT=auto 时有效
		MEASureCmd += ",";
		MEASureCmd += "0";//参数15 Scale [1, 2, 5, 10, 20]，余辉（DPS）模式有效。
		MEASureCmd += ",";
		MEASureCmd += "0";//参数16 Tp（单位为0.1dB），余辉（DPS）模式有效.
		MEASureCmd += "\r\n";
		
		bool RADIO_ReadFlag = false;
		int m_num = 0;
		int Radio_num =0;
		std::string strtpRetXml = "";
		while(Radio_num<3)
		{
			std::string strtempRetXml = "";
			do
			{
				if(!SpecFlagSet)
				{
					SpecOrderSet = MEASureCmd;
					SpecFlagSet = true;
				}
				Sleep(300);
				ALLReadMutex.acquire();
				string m_ip = getipforRADIO();
				int m_port = getportforRADIO();
				SpectrumTcpBaseC Spectrum(m_ip.c_str(),m_port);
				string SpectrumRL = "MEASure:SPECtrum:DATa?\r\n";//读取频谱曲线有效数据
				int Ncount = 0;
				if(Spectrum.ConnectSpecEquiCard())
				{
					string SpectrumSetPA = "MEASure:SPECtrum:SETTing?";
					SpectrumSetPA +="\r\n";
					string strPa = "";
					if(Spectrum.sendSpecOrder(SpectrumSetPA.c_str(),SpectrumSetPA.length()))
					{
						Spectrum.recSpecOrder(strPa);
					}
					if(strPa.find(StrUtil::Int2Str(FreqNcount).c_str())!=-1)
					{
						cout<<"设置成功："<<strPa<<endl;
						while(Ncount<15)//设定5次读取有效数据
						{
							cout<<"读取频谱曲线指令："<<SpectrumRL<<endl;
							string strtmpxml = "";
							if(Spectrum.sendSpecOrder(SpectrumRL.c_str(),SpectrumRL.length()))
							{
								Spectrum.recSpecOrder(strtmpxml);
							}
							int kn = Spectrum.SpectrumMap.size();
							if(strtmpxml!="")
							{
								if(kn==FreqNcount)
								{
									if(strtmpxml.find("3264")!=-1)
									{
										Sleep(300);
										RADIO_ReadFlag = true;
									}
									else
									{
										RADIO_ReadFlag = false;
										strtempRetXml = strtmpxml;
										break;
									}
								}
								else
									RADIO_ReadFlag = true;
							}
							else
								RADIO_ReadFlag = true;
							Ncount++;
						}
					}
					else
					{
						cout<<"设置失败："<<strPa<<endl;
						RADIO_ReadFlag = true;
					}
				}
				ALLReadMutex.release();
				m_num++;
			}while(RADIO_ReadFlag && m_num<5);
			if(strtempRetXml!="")
			{
				strtpRetXml += strtempRetXml;
				strtpRetXml += "_";
			}
			Radio_num++;
		}
		std::string strRetXml = "";
		GetSpecRetStr(strtpRetXml,strRetXml);
		if(strRetXml!="")
		{
			strRetXml+=";";
		}
		if(strRetXml.size()<=0)
		{
			strRetMsg= retfailxml;
			SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
			return false;
		}
		else
		{
			XmlParser retpaser(retsuccxml.c_str());
			pXMLNODE SpectrumParamNode = retpaser.GetNodeFromPath("Msg/SpectrumQueryReport/SpectrumParam");
			pXMLNODE SpectrumNode;
            string strTemp, strEnd;
            int pBegin = startfreq;
			while (1)
			{
				int npos = strRetXml.find(';');
				if (strRetXml != "")
				{
					if (npos != -1)
					{
						strTemp = strRetXml.substr(0, npos);
						strEnd = strRetXml.substr(++npos, strRetXml.length() - npos);
						SpectrumNode = retpaser.CreateNodePtr(SpectrumParamNode,"SpectrumIndex");
						retpaser.SetAttrNode("Freq",StrUtil::Int2Str(pBegin),SpectrumNode);
						retpaser.SetAttrNode("Value",strTemp,SpectrumNode);
						if(pBegin==endfreq)
							break;
						pBegin += freqstep;
						strRetXml = strEnd;
					}
				}
				else
					break;
			}
			retpaser.SaveToString(strRetMsg);
			//std::cout<<"接收机扫描结果："<<strRetMsg<<endl;
		}
	}
	else//中间件逻辑
	{
#if 0		
		string strRXml;
		string strRetDeviceXml;
		int mCount = 0;
		bool isRxml = false;

		SpectrumTcpBaseC Spectrum(m_RADIOip.c_str(),m_RADIOport);
		while(mCount<3)
		{
            if(Spectrum.ConnectSpecEquiCard())
            {
                if(Spectrum.sendSpecOrder(strDeviceXml.c_str(),strDeviceXml.length()))
                {
                    Spectrum.recSpecOrder(strRXml);
                }
            }
			if(strRXml.size()>0)
			{
				isRxml = true;
				break;
			}
			mCount++;
		}

		if(isRxml)
		{
			int pos = strRXml.find('<');
			strRetDeviceXml = strRXml.substr(pos,strRXml.length()-pos);
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
			pXMLNODE retChanNode = retpaser.GetNodeFromPath("Msg/SpectrumQueryReport/SpectrumParam");
			pXMLNODE freqNode = NULL;
			int iTemFreq=0;
			float fTmpLevel=0;
			std::string strTmpFreq, strTmpLevel;
			pXMLNODELIST channelNodeList = retDeviceParser.GetNodeList(retDeviceParser.GetNodeFromPath("Msg/Data/ScanResult"));

			for (int i=0; i<channelNodeList->Size(); i++)
			{
				freqNode = retDeviceParser.GetNextNode(channelNodeList);
				retDeviceParser.GetAttrNode(freqNode, "Frequency", strTmpFreq);
				retDeviceParser.GetAttrNode(freqNode, "Level", strTmpLevel);
				iTemFreq = StrUtil::Str2Int(strTmpFreq);
				//fTmpLevel = (float)(StrUtil::Str2Int(strTmpLevel));
				fTmpLevel = StrUtil::Str2Float(strTmpLevel);
				pXMLNODE freqnode = retpaser.CreateNodePtr(retChanNode,"SpectrumIndex");
				retpaser.SetAttrNode("Freq", StrUtil::Int2Str(iTemFreq), freqnode);
				retpaser.SetAttrNode("Value",StrUtil::Float2Str(fTmpLevel),freqnode);
			}
			retpaser.SaveToString(strRetMsg);
			std::cout<<"接收机扫描结果："<<strRetMsg<<endl;
			return true;
		}
		else
			return false;
#else
		int SpecCount = 0,nodeNum = 0;;
		std::string strTmpFreq, strTmpLevel;
		int iTemFreq=0;
		float fTmpLevel=0;

		while(SpecCount<3)
		{
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
			pXMLNODELIST channelNodeList = retDeviceParser.GetNodeList(retDeviceParser.GetNodeFromPath("Msg/Data/ScanResult"));
			nodeNum = channelNodeList->Size();
			pXMLNODE freqNode = NULL;
			std::string Templev,TempFreq;
			if(strTmpLevel!="")
			{
				strTmpLevel += "_";
			}
			for (int i=0; i<channelNodeList->Size(); i++)
			{
				if(Templev!="")
				{
					strTmpLevel += ";";
				}
				freqNode = retDeviceParser.GetNextNode(channelNodeList);
				retDeviceParser.GetAttrNode(freqNode, "Frequency", strTmpFreq);
				retDeviceParser.GetAttrNode(freqNode, "Level", Templev);
				strTmpLevel += Templev;
			}
			Templev = "";
			freqNode = NULL;
			SpecCount++;
		}
		std::string strRET,strEnd1,strlev;
		GetSpecRetStr(strTmpLevel,strRET);
		if(strRET!="")
		{
			XmlParser retpaser(retsuccxml.c_str());
			pXMLNODE retChanNode = retpaser.GetNodeFromPath("Msg/SpectrumQueryReport/SpectrumParam");
			int mpos = 0;
			for(int i =0;i<nodeNum;++i)
			{
				mpos = strRET.find(';');
				strlev = strRET.substr(0, mpos);
				strEnd1 = strRET.substr(++mpos, strRET.length() - mpos);
				strRET = strEnd1;
				pXMLNODE Freqnode = retpaser.CreateNodePtr(retChanNode,"SpectrumIndex");
				retpaser.SetAttrNode("Freq", StrUtil::Int2Str(startfreq), Freqnode);
				retpaser.SetAttrNode("Value",strlev,Freqnode);
				startfreq += freqstep;
			}
			retpaser.SaveToString(strRetMsg);
		}
	}
	std::cout<<"接收机扫描结果："<<strRetMsg<<endl;
#endif
	APPLOG::instance()->WriteLog(RECORD,LOG_EVENT_WARNNING,strRetMsg,LOG_OUTPUT_FILE);
	return true;
}

bool RADIOHTTPDeviceAccess::setCardSystemTime()
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
	cout<<strDevIP<<"---校时成功!!!"<<endl;
	return true;
}

bool RADIOHTTPDeviceAccess::getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj)
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
		//cout<<strRetDeviceXml<<endl;
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

bool RADIOHTTPDeviceAccess::getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj)
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

bool RADIOHTTPDeviceAccess::setTunerInfo(int chanNo, int freq)
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
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Tuner/ModulationType"), "FM");

	string strDeviceXml;
	srcParser.SaveToString(strDeviceXml);
	string strRetDeviceXml;

	if(!SendXmlTaskToDevice(strDeviceXml, strRetDeviceXml))
	{
		return false;
	}
    //SpectrumTcpBaseC Spectrum();

	XmlParser retDeviceParser;
	retDeviceParser.Set_xml(strRetDeviceXml);
	std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
	if( strXmlRet != std::string("SUCCESS"))
	{
		return false;
	}
	return true;
}

bool RADIOHTTPDeviceAccess::SetEncoderSwitch(bool enable)
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


bool RADIOHTTPDeviceAccess::ChannelscanEx(int startfreq,int endfreq,int scanstep,SpecialRadioRetMessage_Obj& ChanScanRet)
{
	string RetScan("");
	int	PowerValue = 13;   //门限为整个频谱数据平均值的1.3倍
	int freqnum=230;

	vector<SpectrumInfoEx> SpectrumVec;
	vector<SpectrumInfoEx> newSpectrumVec;

	vector<SpectrumInfoEx> newSpectrumVec1;
	std::vector<SpectrumInfoEx>::iterator i_newSpecVec;

	if(!Spectrumscan(startfreq,endfreq,scanstep,SpectrumVec))
	{
		return false;
	}

	if(SpectrumVec.size()<200)
	{
		return false;
	}

	float avg = 0;
	for(int i = 0;i<SpectrumVec.size();i++)
	{	
		avg = avg +SpectrumVec[i].level;
	}
	int sum = 211;//(endfreq-startfreq)/scanstep;
	avg = avg/sum;
	for(int i = 0;i<SpectrumVec.size()-1;i++)
	{
		cout<<SpectrumVec[i].freq<<":l:"<<SpectrumVec[i].level<<":o:"<<SpectrumVec[i].offset<<":u:"<<SpectrumVec[i].usn<<":w:"<<SpectrumVec[i].wam<<endl;
		if((SpectrumVec[i].level>avg*1.3 && abs(SpectrumVec[i].offset)<200 && SpectrumVec[i].usn<15 &&  SpectrumVec[i].freq>87500)\
			||(SpectrumVec[i].usn<3&&SpectrumVec[i].level>avg*2))
		{
			newSpectrumVec1.push_back(SpectrumVec[i]);
		}
	}

	//sort(newSpectrumVec1.begin(),newSpectrumVec1.end(),comparison_wam);
	//sort(newSpectrumVec1.begin(),newSpectrumVec1.end(),comparison_level);
	//sort(newSpectrumVec1.begin(),newSpectrumVec1.end(),comparison_usn);
	//for(int j= 0;j<newSpectrumVec1.size();j++)
	//{
	//	cout<<newSpectrumVec1[j].freq<<":u:"<<newSpectrumVec1[j].usn<<":l:"<<newSpectrumVec1[j].level<<":w:"<<newSpectrumVec1[j].wam<<endl;
	//	bool isNewFreq = true;
	//	for(int t=0;t<newSpectrumVec.size();t++)
	//	{
	//		if(abs(newSpectrumVec1[j].freq-newSpectrumVec[t].freq)<101)
	//		{
	//			isNewFreq = false;
	//			//cout<<newSpectrumVec1[j].freq<<":::"<<newSpectrumVec[t].freq<<endl;
	//			//cout<<newSpectrumVec1[j].level<<":::"<<newSpectrumVec[t].level<<endl;
	//			//cout<<newSpectrumVec1[j].wam<<":::"<<newSpectrumVec[t].wam<<endl;
	//			//cout<<newSpectrumVec1[j].usn<<":::"<<newSpectrumVec[t].usn<<endl;
	//		}
	//	}
	//	if(isNewFreq)
	//	{
	//		newSpectrumVec.push_back(newSpectrumVec1[j]);
	//	}
	//}
	

	int temp[230]={0};
	int SpectrumPeak[230] = {0};


	////第一遍扫瞄
	//for(int kk = 1;kk<SpectrumVec.size();kk++)
	//{
	//	if(SpectrumVec[kk].level>SpectrumVec[kk-1].level)
	//		temp[kk] = 1;
	//	else if(SpectrumVec[kk].level<SpectrumVec[kk-1].level)
	//		temp[kk] = 2;
	//	else
	//		temp[kk] = 0;
	//}

	////第二遍扫瞄
	//for(int kk = 1;kk<SpectrumVec.size()-1;kk++)
	//{
	//	//	cout<<"111111"<<endl;
	//	if((/*((temp[kk-1]==1)&&temp[kk]==0)||*/temp[kk-1]==1) && temp[kk+1]==2)
	//	{
	//		if( (SpectrumVec[kk].level-SpectrumVec[kk-1].level)/1 > 3 ) //斜率
	//		{
	//			SpectrumInfoEx SpectInfo ;
	//			SpectrumPeak[kk] = 1;
	//			SpectInfo.freq=SpectrumVec[kk].freq;
	//			SpectInfo.level=SpectrumVec[kk].level; 
	//			if((SpectInfo.freq - 87300)>150) //过滤87.1到87.4的频道
	//			{
	//				if(((SpectrumVec[kk].freq-startfreq)>-0.01)&&((endfreq-SpectrumVec[kk].freq)>0.01)) //广播手动设置频道扫描开始结束频率特殊处理,过滤不在用户设置的开始结束频率范围内的频率
	//				{   
	//					int mflag = 0;  //频道表及其周边频率（正负0.1）标志
	//					for(i_newSpecVec = newSpectrumVec.begin();i_newSpecVec !=newSpectrumVec.end();i_newSpecVec++)  //过滤已添加频率及其周边频率
	//					{
	//						if(abs(SpectrumVec[kk].freq- (*i_newSpecVec).freq)<101)
	//							mflag = 1;
	//					}
	//					if((mflag == 0)&&(SpectrumVec[kk].level>10))
	//					{
	//						newSpectrumVec.push_back(SpectInfo);
	//					}
	//				}
	//				//std::cout<<"峰值为："<<i<<"="<<SpectrumVec[i]<<endl;
	//			}
	//		}
	//		else
	//		{
	//			SpectrumPeak[kk] = 0;
	//		}
	//	}
	//}
	//// 第三遍添加特别峰值
	//for(int kk = 1;kk<SpectrumVec.size()-1;kk++ )
	//{
	//	//if(kk == 178)
	//	//	cout<<"111111"<<endl;
	//	if(SpectrumPeak[kk] == 0)  //已添加的频率不做处理
	//	{
	//		if((SpectrumVec[kk].level>(avg*PowerValue)/10)&&(SpectrumPeak[kk-1]+SpectrumPeak[kk+1]<1))
	//		{
	//			SpectrumInfoEx SpectInfo ;
	//			SpectrumPeak[kk] = 1;
	//			SpectInfo.freq=SpectrumVec[kk].freq;
	//			SpectInfo.level=SpectrumVec[kk].level;
	//			if((SpectInfo.freq -87300)>150) //过滤87.1到87.4的频道
	//			{
	//				if(((SpectrumVec[kk].freq-startfreq)>-0.01)&&((endfreq-SpectrumVec[kk].freq)>0.01)) ////广播手动设置频道扫描开始结束频率特殊处理,过滤不在用户设置的开始结束频率范围内的频率
	//				{
	//					int mflag = 0;  //频道表及其周边频率（正负0.1）标志
	//					for(i_newSpecVec = newSpectrumVec.begin();i_newSpecVec !=newSpectrumVec.end();i_newSpecVec++)  //过滤已添加频率频率及其周边频率
	//					{
	//						if(abs(SpectrumVec[kk].freq- (*i_newSpecVec).freq)<101)
	//							mflag = 1;
	//					}
	//					if((mflag == 0)&&(SpectrumVec[kk].level>25))
	//					{
	//						newSpectrumVec.push_back(SpectInfo);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}


	ChanScanRet.len = 0;
	sort(newSpectrumVec1.begin(),newSpectrumVec1.end(),comparison_h);

	int specsum = newSpectrumVec1.size();

	bool jumpchannel = false;
	for(int jj=0;jj<specsum;jj++)
	{
		bool b_addNew=false;
		jumpchannel = false;
		
		if(jj<(specsum-1))
		{
			if((newSpectrumVec1[jj+1].freq-newSpectrumVec1[jj].freq)<101)
			{//下一个频道是临频，比较usn值，如果小则加入，如果大直接跳过
				if(newSpectrumVec1[jj].usn<newSpectrumVec1[jj+1].usn)
				{//把本次检索的加入，直接跳过下一个
					b_addNew = true;
					jumpchannel = true;
				}
				else if(newSpectrumVec1[jj].usn==newSpectrumVec1[jj+1].usn && abs(newSpectrumVec1[jj].offset)<abs(newSpectrumVec1[jj+1].offset))
				{//把本次检索的加入，直接跳过下一个
					b_addNew = true;
					jumpchannel = true;
				}
			}
			else
				b_addNew = true;
		}
		else
			b_addNew = true;
		if(b_addNew)
		{
			newSpectrumVec.push_back(newSpectrumVec1[jj]);
		}
		if(jumpchannel)
		{
			jj++;
		}
	}

	sort(newSpectrumVec.begin(),newSpectrumVec.end(),comparison_h);

	//for(int jj=0;jj<specsum;jj++)
	//{
	//	{// 对过调广播节目导致的一个频点扫描为2个频率进行过滤
	//		if((jj>=1)&&jj<(specsum-1))
	//		{
	//			// cout<<"XXXXXXXXXXXX有问题的电台过滤222:";
	//			// cout<<newSpectrumVec[jj+1].freq-newSpectrumVec[jj].freq<<endl;
	//			if(newSpectrumVec[jj+1].freq-newSpectrumVec[jj].freq<210)
	//			{//相隔200kHz的频率，需要对中间频率的信号强度进行判断
	//				int n=SpectrumVec.size();
	//				for(int m=0;m<n;m++)
	//				{					
	//					if(abs(SpectrumVec[m].freq-newSpectrumVec[jj].freq)<10)
	//					{
	//						if(m<n-2)
	//						{
	//							if((SpectrumVec[m].level-SpectrumVec[m+1].level<3)&&(abs(SpectrumVec[m].level-SpectrumVec[m+2].level)<3)&&(abs(SpectrumVec[m+1].level-SpectrumVec[m+2].level)<3))
	//							{
	//								// cout<<"XXXXXXXXXXXX有问题的电台过滤"<<SpectrumVec[m].freq<<endl;
	//								jumpchannel=2;		
	//							}
	//						};
	//					}
	//				}

	//			}
	//		}
	//		if(jumpchannel>1)
	//		{//修改第一个
	//			jumpchannel=1;	
	//			newSpectrumVec[jj].freq = newSpectrumVec[jj].freq+100;
	//			//改频点
	//		}
	//		if(jumpchannel==1)
	//		{//跳过第二个，跳过一个频点
	//			jumpchannel=0;	
	//			continue;
	//		}
	//	}
	//}
	specsum = newSpectrumVec.size();
	for(int jj=0;jj<specsum;jj++)
	{
		//if(newSpectrumVec[jj].usn <=10)
		{
			ChanScanRet.value[ChanScanRet.len]=newSpectrumVec[jj].freq;

			RetScan.append(StrUtil::Int2Str(newSpectrumVec[jj].freq));
			RetScan.append("-");
			RetScan.append(StrUtil::Int2Str(newSpectrumVec[jj].level));
			RetScan.append(";");

			ChanScanRet.len++;
		}
	}
	

	RetScan.append("Sum=");	
	RetScan.append(StrUtil::Int2Str(ChanScanRet.len));
	// APPLOG::instance()->WriteLog(ALARM,LOG_EVENT_DEBUG,RetScan,LOG_OUTPUT_FILE);
	return true;
}



bool RADIOHTTPDeviceAccess::Spectrumscan(int startfreq, int endfreq, int scanstep, vector<SpectrumInfoEx> &vecSpectrumValue)
{
	string strDeviceSrcXml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.16.10.250:11111\">\
							<Type>RadioSpectrumScanQuery</Type>\
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

// 	int startfreq=(StrUtil::Str2Float(strstartfreq)+0.0002)*1000,endfreq=(StrUtil::Str2Float(strendfreq)+0.0002)*1000,
// 		freqstep=(StrUtil::Str2Float(strstep)+0.0002)*1000;
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/StartFrequency"), StrUtil::Int2Str(startfreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/EndFrequency"), StrUtil::Int2Str(endfreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/StepSize"), StrUtil::Int2Str(scanstep).c_str());

	string strDeviceXml;
	deviceParser.SaveToString(strDeviceXml);
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

	pXMLNODE freqNode = NULL;
	float fTmpFreq = 0.0, fTmpLevel = 0.0;
	std::string strTmpFreq, strTmpLevel, strTmpUsn, strTmpWam,strOffset;
	pXMLNODELIST channelNodeList = retDeviceParser.GetNodeList(retDeviceParser.GetNodeFromPath("Msg/Data/ScanResult"));

	for (int i=0; i<channelNodeList->Size(); i++)
	{
		SpectrumInfoEx SpectInfo;
		
		freqNode = retDeviceParser.GetNextNode(channelNodeList);
		retDeviceParser.GetAttrNode(freqNode, "Frequency", strTmpFreq);
		retDeviceParser.GetAttrNode(freqNode, "Level", strTmpLevel);
		retDeviceParser.GetAttrNode(freqNode, "Usn", strTmpUsn);
		retDeviceParser.GetAttrNode(freqNode, "Wam", strTmpWam);
		retDeviceParser.GetAttrNode(freqNode, "Offset", strOffset);
		SpectInfo.freq = (float)(StrUtil::Str2Int(strTmpFreq));
		SpectInfo.offset =  (float)(StrUtil::Str2Int(strOffset));
		SpectInfo.level = (float)(StrUtil::Str2Int(strTmpLevel));
		SpectInfo.usn = (float)(StrUtil::Str2Int(strTmpUsn));
		SpectInfo.wam = (float)(StrUtil::Str2Int(strTmpWam));
		vecSpectrumValue.push_back(SpectInfo);	
		
	}

	return true;
}

bool RADIOHTTPDeviceAccess::SetAlarmThreshold()
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
							<LostSignal>500</LostSignal>\
							</ThresholdInfo>\
							</Data>\
							</Msg>";
	std::string strDevIP,strAudioPower,strRadioSignalThresholdRet;
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

	strAudioPower = PROPMANAGER::instance()->GetFMAudioPowerRet();
	if(strAudioPower=="")
	{
		strAudioPower = "100";
	}
	strRadioSignalThresholdRet = PROPMANAGER::instance()->GetSignalThresholdRet();
	if(strRadioSignalThresholdRet=="")
	{
		strRadioSignalThresholdRet = "500";
	}
	XmlParser srcParser;
	srcParser.Set_xml(strSrcXml);
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	//srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), "ALL");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/BlackSimilar"), "990");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/ColourBarSimilar"), "990");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/FreezeSimilar"), "990");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/VolumeHighValue"), "100");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/VolumeLowValue"), "5");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/AudioLostValue"), strAudioPower.c_str());
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/AudioUnsualLastTime"), "500");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/LostSignal"), strRadioSignalThresholdRet.c_str());

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
bool RADIOHTTPDeviceAccess::Find_str(string str,char** desc,int DescLen)
{
    bool isFlag = false;
    for (int i = 0;i< DescLen ; ++i)
    {
        string strCp = desc[i];
        if (strCp == str)
        {
            isFlag  = true;
            break;
        }
    }
    return isFlag;
}
void RADIOHTTPDeviceAccess::GetSpecRetStr( std::string strMore,std::string &strTemp )
{
	string strTemp1, strTemp2, strTemp3,strFirst, strEnd1, strSec, strEnd2,strThr,strEnd3;
	int num = count(strMore.begin(), strMore.end(), '_');
	string strTMP[3];
	int oldVal = 0;
	for(int i=0;i<num;++i)
	{
		int pos = strMore.find('_');
		strTemp1 = strMore.substr(0, pos);
		strTemp2 = strMore.substr(++pos, strMore.length() - pos);
		strTMP[i] = strTemp1;
		strMore = strTemp2;
	}

	int num1 = count(strTMP[0].begin(), strTMP[0].end(), ';');
	int num2 = count(strTMP[1].begin(), strTMP[1].end(), ';');
	vector<int> vecstr;
	if (num1 == num2)
	{
		for (int i = 0; i< num1 + 1; ++i)
		{
			int npos = strTMP[0].find(';');
			int mpos = strTMP[1].find(';');
			int kpos = strTMP[2].find(';');
			if (strTMP[0] != "" || strTMP[1] != "")
			{
				if (npos != -1 || mpos != -1)
				{
					strFirst = strTMP[0].substr(0, npos);
					strEnd1 = strTMP[0].substr(++npos, strTMP[0].length() - npos);

					strSec = strTMP[1].substr(0, mpos);
					strEnd2 = strTMP[1].substr(++mpos, strTMP[1].length() - mpos);

					strThr = strTMP[2].substr(0, kpos);
					strEnd3 = strTMP[2].substr(++kpos, strTMP[2].length() - kpos);
				}
				strTMP[0] = strEnd1;
				strTMP[1] = strEnd2;
				strTMP[2] = strEnd3;
				if (i == num1)
				{
					strFirst = strEnd1;
					strSec = strEnd2;
					strThr = strEnd3;
				}
				int val1 = StrUtil::Str2Float(strFirst) * 100;
				int val2 = StrUtil::Str2Float(strSec) * 100;
				int val3 = StrUtil::Str2Float(strThr) * 100;
				int val = val1 + val2 + val3;
				double ret = val/(double)300.00;
				strTemp3 += StrUtil::Float2Str(ret);
				vecstr.push_back(ret);
				strTemp3 += ";";
			}
		}
	}
	strTemp = strTemp3;
	int sum = vecstr.size();
	/*if(vecstr.size()>0)
	{
		for(int i =0;i<vecstr.size()-1;++i)
		{
			if(vecstr.size()<3)
			{
				if(abs(vecstr[0] - vecstr[1]) <= 50)
				{
					if(vecstr[0]<vecstr[1])
					{
						vecstr[1] += 50;
					}
				}
			}
			else
			{
				if(abs(vecstr[i] - vecstr[i+1])<=100)
				{
					if(vecstr[i]<vecstr[i+1])
					{
						vecstr[i] -= 100;
						vecstr[i+1] += 100;
					}
					else
					{
						vecstr[i] += 100;
						vecstr[i+1] -= 100;
					}
				}
			}
			double ret = vecstr[i]/(double)100.00;
			strTemp += StrUtil::Float2Str(ret);
			strTemp += ";";
		}
		double ret = vecstr[vecstr.size()-1]/(double)100.00;
		strTemp += StrUtil::Float2Str(ret);
	}*/
}