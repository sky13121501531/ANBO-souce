#include "./ATVHTTPDeviceAccess.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/AppLog.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Communications/SysMsgSender.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "./SpectrumTCPBaseC.h"
#define sky 1
#define nosky 0
SpectrumParm AllparmSet;
extern ACE_Thread_Mutex ALLReadMutex;
extern bool SpecFlagSet;
extern std::string SpecOrderSet;
extern int FgNum;
extern std::string SetFreq;
bool flagSpec = false;
int ScanFreq[] = {
	49750,
	57750,
	65750,
	77250,
	85250,
	112250,
	120250,
	128250,
	136250,
	144250,
	152250,
	160250,
	168250,
	176250,
	184250,
	192250,
	200250,
	208250,
	216250,
	224250,
	232250,
	240250,
	248250,
	256250,
	264250,
	272250,
	280250,
	288250,
	296250,
	304250,
	312250,
	320250,
	328250,
	336250,
	344250,
	352250,
	360250,
	368250,
	376250,
	384250,
	392250,
	400250,
	408250,
	416250,
	424250,
	432250,
	440250,
	448250,
	456250,
	471250,
	479250,
	487250,
	495250,
	503250,
	511250,
	519250,
	527250,
	535250,
	543250,
	551250,
	559250,
	567250,
	575250,
	583250,
	591250,
	599250,
	607250,
	615250,
	623250,
	631250,
	639250,
	647250,
	655250,
	663250,
	671250,
	679250,
	687250,
	695250,
	703250,
	711250,
	719250,
	727250,
	735250,
	743250,
	751250,
	759250,
	767250,
	775250,
	783250,
	791250,
	799250,
	807250,
	815250,
	823250,
	831250,
	839250,
	847250,
	855250
};

ATVHTTPDeviceAccess::ATVHTTPDeviceAccess(int iDeviceID, std::string strIP, int iPort)
:HTTPDeviceAccess(iDeviceID, strIP, iPort)
{
	if(iDeviceID!=17)
	{
		setCardSystemTime();
		SetAlarmThreshold();
	}
	else
	{
		this->m_ip = strIP;
		this->m_port = iPort;
	}
}
ATVHTTPDeviceAccess::~ATVHTTPDeviceAccess()
{

}
bool ATVHTTPDeviceAccess::GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	int iCmdPort;
	PROPMANAGER::instance()->GetDeviceCmdPort(DeviceId, iCmdPort);

	string SpectrumFlag,SpectrumVBW,SpectrumRBW,SpecPlusRet,SpecCNRRet,SpecPlusRetStr="";//默认发中间件
	SpectrumFlag = PROPMANAGER::instance()->GetIsSpectrumFlag();//获取type
    SpecPlusRetStr = PROPMANAGER::instance()->GetSpectrumLitRet();//获取频谱level补偿值
	if(SpectrumFlag!="0")
	{
		SpectrumFlag = "3";//type非0 1 2的任何值
	}
    if(SpecPlusRetStr=="")
    {
        SpecPlusRetStr = "0;15";//默认补偿0及底噪15
    }
    int StrPos = SpecPlusRetStr.find(";");
    if(StrPos!=-1)
    {
        SpecPlusRet = SpecPlusRetStr.substr(0,StrPos);
        SpecCNRRet = SpecPlusRetStr.substr(++StrPos,SpecPlusRetStr.length() - StrPos);
    }
	if(SpecPlusRet=="")
	{
		SpecPlusRet = "0";
	}
	if(SpecCNRRet=="")
	{
		SpecCNRRet = "15";
	}
    
	int SpecType = StrUtil::Str2Int(SpectrumFlag);
	if(SpecType==0)//本机直通逻辑
	{
		std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"GB2312\"?>\
								<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
								<Type>GetATVQualityInfo</Type>\
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

		XmlParser srcParser;
		srcParser.Set_xml(strSrcXml);
		int tmpDeviceID = 0;
		PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
		srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());

		string strDeviceXml,strRetXml;
		srcParser.SaveToString(strDeviceXml);
		string strRetDeviceXml;
		
		
        pXMLNODE retrootNode=Retparser.GetRootNode();
        Retparser.SetAttrNode("DVBType",dvbtype,retrootNode);
        pXMLNODE retNode=Retparser.CreateNodePtr(retrootNode,"Return");
        Retparser.SetAttrNode("Type",string("QualityQuery"),retNode);
        Retparser.SetAttrNode("Value",string("0"),retNode);
        Retparser.SetAttrNode("Desc",string("成功"),retNode);
        Retparser.SetAttrNode("Comment",string(""),retNode);

        pXMLNODE reportNode=Retparser.CreateNodePtr(retrootNode,"QualityQueryReport");
        Retparser.SetAttrNode("STD",string(""),reportNode);
        Retparser.SetAttrNode("Freq",StrFreq,reportNode);
        Retparser.SetAttrNode("SymbolRate",string(""),reportNode);

        pXMLNODE paramNode=Retparser.CreateNodePtr(reportNode,"QualityParam");

        std::string strTemp,strEnd;
        bool isIpic = false,isIaud = false;
        pXMLNODE indexNode;
        float cnr = 0;
        float iPicLevel=0;
        float iAudioLevel=0;
        float iLevelDiff=0;
        std::string strPicLevel="",strAudioLevel="",strDiffFreq="",strCnr = "";
		int m_QUInum = 0;
		bool ATV_CNRFlag = false,ATV_DIFFFlag = false,ATV_Level = false,ATV_RadioFIFFFlag = false;
		do 
		{
			if(!ATV_DIFFFlag)
			{
				SpecFlagSet = true;
				FgNum = 4;
				SetFreq = StrUtil::Int2Str(freq*1000);
			}
			Sleep(300);
			ALLReadMutex.acquire();
			SpectrumTcpBaseC SpectrumATVQuality(m_ip.c_str(),m_port);
			if(SpectrumATVQuality.ConnectSpecEquiCard())
			{
				std::string strpram = "MEASure:TV:FREQerror:SETTing?";
				strpram += "\r\n";
				int parmNUM = 0;
				std::string strpramRet = "";
				while(parmNUM<5)
				{
					if(SpectrumATVQuality.sendSpecOrder(strpram.c_str(),strpram.length()))
					{
						SpectrumATVQuality.recSpecOrder(strpramRet);
					}
					if(strpramRet!="")
					{
						break;
					}
					parmNUM++;
				}
				if(strpramRet.find(SetFreq.c_str()) != -1)
				{
					//cout<<"图像频偏设置成功："<<strpramRet<<endl;
					string SpectrumRL = "MEASure:TV:FREQerror:RESult?\r\n";
					int m = 0;
					while(m<5)
					{
						if(SpectrumATVQuality.sendSpecOrder(SpectrumRL.c_str(),SpectrumRL.length()))
						{
							SpectrumATVQuality.recSpecOrderforATVQualityDiffFreq(strRetXml);
							int npos = strRetXml.find(';');
							if(npos != -1)
							{
								strPicLevel = strRetXml.substr(0, npos);
								strEnd = strRetXml.substr(++npos, strRetXml.length() - npos);
								int mpos = strEnd.find(";");
								strDiffFreq = strEnd.substr(0, mpos);
							}
							if(strDiffFreq!="")
							{
								//cout<<"strRetXml--TV:"<<strRetXml<<endl;
								cout<<"图像电平："<<strPicLevel<<"-----频偏："<<strDiffFreq<<endl;
								ATV_DIFFFlag = true;
								break;
							}
						}
						m++;
					}
				}
				else
				{
					cout<<"图像频偏设置失败："<<strpramRet<<endl;
					if(m_QUInum==3)
					{
						m_QUInum = 5;//设定 设置4次不成功，退出去
					}
				}
			}
			ALLReadMutex.release();
			m_QUInum++;
		} while (!ATV_DIFFFlag && m_QUInum<5);
#if 0
		m_QUInum = 0;
		strRetXml="";
		do 
		{
			
			if(!ATV_CNRFlag)
			{
				SpecFlagSet = true;
				FgNum = 3;
				SetFreq = StrUtil::Int2Str(freq*1000);
			}
			Sleep(300);
			ALLReadMutex.acquire();
			SpectrumTcpBaseC SpectrumATVQuality(m_ip.c_str(),m_port);
			if(SpectrumATVQuality.ConnectSpecEquiCard())
			{
				std::string strpram = "MEASure:TV:CNR:SETTing?";
				strpram += "\r\n";
				int parmNUM = 0;
				std::string strpramRet = "";
				while(parmNUM<5)
				{
					if(SpectrumATVQuality.sendSpecOrder(strpram.c_str(),strpram.length()))
					{
						SpectrumATVQuality.recSpecOrder(strpramRet);
					}
					if(strpramRet!="")
					{
						break;
					}
					parmNUM++;
				}
				if(strpramRet.find(SetFreq.c_str()) != -1)
				{
					cout<<"CNR设置成功："<<strpramRet<<endl;
					string SpectrumRL = "MEASure:TV:CNR:RESult?\r\n";
					int m = 0;
					while(m<5)
					{
						if(SpectrumATVQuality.sendSpecOrder(SpectrumRL.c_str(),SpectrumRL.length()))
						{
							SpectrumATVQuality.recSpecOrderforATVQualityCNR(strRetXml);
							int npos = strRetXml.find(';');
							if(npos != -1)
							{
								std::string strlevel = strRetXml.substr(0, npos);
								strEnd = strRetXml.substr(++npos, strRetXml.length() - npos);
								int mpos = strEnd.find(";");
								strCnr = strEnd.substr(0, mpos);
							}
							if(strCnr!="")
							{
								cout<<"载噪比："<<strCnr<<endl;
								ATV_CNRFlag = true;
								break;
							}
						}
						m++;
					}
				}
				else
				{
					cout<<"CNR设置失败："<<strpramRet<<endl;
					if(m_QUInum==3)
					{
						m_QUInum = 5;//设定 设置4次不成功，退出去
					}
				}
			}
			ALLReadMutex.release();
			m_QUInum++;
		} while (!ATV_CNRFlag && m_QUInum<5);

		m_QUInum = 0;
        strRetXml="";
		do 
		{
			if(!ATV_RadioFIFFFlag)
			{
				SpecFlagSet = true;
				FgNum = 9;
				SetFreq = StrUtil::Int2Str(freq*1000 + 6500000);
			}
			Sleep(300);
			ALLReadMutex.acquire();
			SpectrumTcpBaseC SpectrumATVQuality(m_ip.c_str(),m_port);
			if(SpectrumATVQuality.ConnectSpecEquiCard())
			{
                string SpectrumRL = "MEASure:LEVel?\r\n";
                int m = 0;
                while(m<5)
                {
                    if(SpectrumATVQuality.sendSpecOrder(SpectrumRL.c_str(),SpectrumRL.length()))
                    {
                        SpectrumATVQuality.recSpecOrder(strRetXml);
                        int npos = strRetXml.find(',');
                        if(npos != -1)
                        {
                            strAudioLevel = strRetXml.substr(0, npos);
                            strEnd = strRetXml.substr(++npos, strRetXml.length() - npos);
                            int mpos = strEnd.find(";");
                            std::string strdFreq = strEnd.substr(0, mpos);
                        }
                        if(strAudioLevel!="")
                        {
                            cout<<"伴音载波电平："<<strAudioLevel<<endl;
                            ATV_RadioFIFFFlag = true;
                            break;
                        }
                    }
                    m++;
                }
			}
			ALLReadMutex.release();
			m_QUInum++;
		} while (!ATV_RadioFIFFFlag && m_QUInum<5);
#endif
		m_QUInum = 0;
		strRetXml="";
		do 
		{
			if(!ATV_RadioFIFFFlag)
			{
				SpecFlagSet = true;
				FgNum = 4;
				SetFreq = StrUtil::Int2Str(freq*1000 + 6500000);
			}
			Sleep(300);
			ALLReadMutex.acquire();
			SpectrumTcpBaseC SpectrumATVQuality(m_ip.c_str(),m_port);
			if(SpectrumATVQuality.ConnectSpecEquiCard())
			{
				std::string strpram = "MEASure:TV:FREQerror:SETTing?";
				strpram += "\r\n";
				int parmNUM = 0;
				std::string strpramRet = "";
				while(parmNUM<3)
				{
					if(SpectrumATVQuality.sendSpecOrder(strpram.c_str(),strpram.length()))
					{
						SpectrumATVQuality.recSpecOrder(strpramRet);
					}
					if(strpramRet!="")
					{
						break;
					}
					parmNUM++;
				}
				if(strpramRet.find(SetFreq.c_str()) != -1)
				{
					cout<<"伴音电平设置成功："<<strpramRet<<endl;
					string SpectrumRL = "MEASure:TV:FREQerror:RESult?\r\n";
					int m = 0;
					while(m<5)
					{
						if(SpectrumATVQuality.sendSpecOrder(SpectrumRL.c_str(),SpectrumRL.length()))
						{
							SpectrumATVQuality.recSpecOrderforATVQualityDiffFreq(strRetXml);
							int npos = strRetXml.find(';');
							if(npos != -1)
							{
								strAudioLevel = strRetXml.substr(0, npos);
								strEnd = strRetXml.substr(++npos, strRetXml.length() - npos);
								int mpos = strEnd.find(";");
								std::string strdFreq = strEnd.substr(0, mpos);
							}
							if(strAudioLevel!="")
							{
								cout<<"伴音电平："<<strAudioLevel<<endl;
								ATV_RadioFIFFFlag = true;
								break;
							}
						}
						m++;
					}
				}
				else
				{
					cout<<"伴音电平设置失败："<<strpramRet<<endl;
					if(m_QUInum==3)
					{
						m_QUInum = 5;//设定 设置4次不成功，退出去
					}
				}
			}
			ALLReadMutex.release();
			m_QUInum++;
		} while (!ATV_RadioFIFFFlag && m_QUInum<5);
        indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
        Retparser.SetAttrNode(string("Type"), string("1"), indexNode);
        Retparser.SetAttrNode(string("Desc"), string("图像载波电平，单位:dbuv"), indexNode);
        iPicLevel = StrUtil::Str2Float(strPicLevel)/10;
        iPicLevel += StrUtil::Str2Float(SpecPlusRet);
        Retparser.SetAttrNode("Value", StrUtil::Float2Str(iPicLevel*1000), indexNode);

        indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
        Retparser.SetAttrNode(string("Type"), string("2"), indexNode);
        Retparser.SetAttrNode(string("Desc"),string("伴音载波电平，单位:dbuv"), indexNode);
        iAudioLevel = StrUtil::Str2Float(strAudioLevel)/10;
        iAudioLevel += StrUtil::Str2Float(SpecPlusRet);
        Retparser.SetAttrNode("Value", StrUtil::Float2Str(iAudioLevel*1000), indexNode);

        indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
        Retparser.SetAttrNode(string("Type"), string("3"), indexNode);
        Retparser.SetAttrNode(string("Desc"), string("图像载波与伴音载波的电平差"), indexNode);
        iLevelDiff = abs(iAudioLevel - iPicLevel);
        Retparser.SetAttrNode("Value", StrUtil::Float2Str(iLevelDiff*1000), indexNode);

        float Scnr = iPicLevel/StrUtil::Str2Float(SpecCNRRet);
        indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
        Retparser.SetAttrNode(string("Type"), string("4"), indexNode);
        Retparser.SetAttrNode(string("Desc"), string("载噪比，单位:dbuv"), indexNode);
        Retparser.SetAttrNode("Value", StrUtil::Float2Str(Scnr*100), indexNode);

        indexNode=Retparser.CreateNodePtr(paramNode,"QualityIndex");
        Retparser.SetAttrNode(string("Type"), string("5"), indexNode);
        Retparser.SetAttrNode(string("Desc"), string("载频频偏，单位:KHz"), indexNode);
        Retparser.SetAttrNode("Value", StrUtil::Float2Str(StrUtil::Str2Float(strDiffFreq)), indexNode);  
		Retparser.SaveToString(strRetMsg);
	}
	else
	{
		string strDeviceSrcXml ="<?xml version=\"1.0\" encoding=\"GB2312\"?>\
								<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
								<Type>GetATVQualityInfo</Type>\
								<Data>\
								<Channel>All</Channel>\
								</Data>\
								</Msg>";
		
		char* source="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg></Msg>";
		XmlParser parser;
		XmlParser deviceParser;
		XmlParser retpaser(source);

		deviceParser.Set_xml(strDeviceSrcXml);

		parser.Set_xml(strCmdMsg);
		pXMLNODE root=parser.GetRootNode();
		string dvbtype;
		parser.GetAttrNode(root,"DVBType",dvbtype);
		pXMLNODE childnode=parser.GetNodeFirstChild(root);
		string nodename=parser.GetNodeName(childnode);

		if(dvbtype != "ATV")
		{
			return false;
		}
		int freq;
		string StrFreq;
		pXMLNODE QuaryNode = parser.GetNodeFirstChild(root);
		parser.GetAttrNode(QuaryNode,"Freq",StrFreq);
		freq =(StrUtil::Str2Float(StrFreq)+0.0002)*1000;
		deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(freq).c_str());
		string strDeviceXml;
		deviceParser.SaveToString(strDeviceXml);
		string strRetDeviceXml;
		if(!SendXmlTaskToDevice(strDeviceXml, strRetDeviceXml))
		{
			pXMLNODE retrootNode=retpaser.GetRootNode();
			retpaser.SetAttrNode("DVBType",dvbtype,retrootNode);

			pXMLNODE retNode=retpaser.CreateNodePtr(retrootNode,"Return");
			retpaser.SetAttrNode("Type",string("QualityQuery"),retNode);
			retpaser.SetAttrNode("Value",string("1"),retNode);
			retpaser.SetAttrNode("Desc",string("失败"),retNode);
			retpaser.SetAttrNode("Comment",string(""),retNode);

			retpaser.SaveToString(strRetMsg);

			SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
			return false;
		}
		XmlParser retDeviceParser;
		retDeviceParser.Set_xml(strRetDeviceXml);
		std::string strXmlRet = retDeviceParser.GetNodeText(retDeviceParser.GetNodeFromPath("Msg/Status"));
		if( strXmlRet != std::string("SUCCESS"))
		{
			pXMLNODE retrootNode=retpaser.GetRootNode();
			retpaser.SetAttrNode("DVBType",dvbtype,retrootNode);

			pXMLNODE retNode=retpaser.CreateNodePtr(retrootNode,"Return");
			retpaser.SetAttrNode("Type",string("QualityQuery"),retNode);
			retpaser.SetAttrNode("Value",string("1"),retNode);
			retpaser.SetAttrNode("Desc",string("失败"),retNode);
			retpaser.SetAttrNode("Comment",string(""),retNode);

			retpaser.SaveToString(strRetMsg);
			SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
			return false;
		}

		pXMLNODE freqNode = NULL;
	
		int iPicLevel=0;
		int iAudioLevel=0;
		int iLevelDiff=0;
		
		std::string strPicLevel, strAudioLevel,strCNR;
		pXMLNODELIST channelNodeList = retDeviceParser.GetNodeList(retDeviceParser.GetNodeFromPath("Msg/Data/ScanResult"));

		//生成上报的指标XML
		pXMLNODE retrootNode=retpaser.GetRootNode();

		retpaser.SetAttrNode("DVBType",dvbtype,retrootNode);

		pXMLNODE retNode=retpaser.CreateNodePtr(retrootNode,"Return");
		retpaser.SetAttrNode("Type",string("QualityQuery"),retNode);
		retpaser.SetAttrNode("Value",string("0"),retNode);
		retpaser.SetAttrNode("Desc",string("成功"),retNode);
		retpaser.SetAttrNode("Comment",string(""),retNode);

		pXMLNODE reportNode=retpaser.CreateNodePtr(retrootNode,"QualityQueryReport");
		retpaser.SetAttrNode("STD",string(""),reportNode);
		retpaser.SetAttrNode("Freq",StrFreq,reportNode);
		retpaser.SetAttrNode("SymbolRate",string(""),reportNode);

		pXMLNODE paramNode=retpaser.CreateNodePtr(reportNode,"QualityParam");

		pXMLNODE indexNode=retpaser.CreateNodePtr(paramNode,"QualityIndex");
		
		for (int i=0; i<channelNodeList->Size(); i++)
		{
			freqNode = retDeviceParser.GetNextNode(channelNodeList);
			retDeviceParser.GetAttrNode(freqNode, "PicLevel", strPicLevel);
			retDeviceParser.GetAttrNode(freqNode, "AudioLevel", strAudioLevel);
			retDeviceParser.GetAttrNode(freqNode, "CNR", strCNR);
			
			iPicLevel = StrUtil::Str2Int(strPicLevel);
			iAudioLevel = StrUtil::Str2Int(strAudioLevel);
			iLevelDiff = iPicLevel - iAudioLevel;
			
			retpaser.SetAttrNode("Type", string("1"), indexNode);
			retpaser.SetAttrNode("Desc", string("图像载波电平，单位:dbuv"), indexNode);
			retpaser.SetAttrNode("Value", strPicLevel, indexNode);

			indexNode=retpaser.CreateNodePtr(paramNode,"QualityIndex");

			retpaser.SetAttrNode("Type", string("2"), indexNode);
			retpaser.SetAttrNode("Desc", string("伴音载波电平，单位:dbuv"), indexNode);
			retpaser.SetAttrNode("Value", strAudioLevel, indexNode);

			indexNode=retpaser.CreateNodePtr(paramNode,"QualityIndex");
			retpaser.SetAttrNode("Type", string("3"), indexNode);
			retpaser.SetAttrNode("Desc", string("图像载波与伴音载波的电平差，单位:dbuv"), indexNode);
			retpaser.SetAttrNode("Value", StrUtil::Int2Str(iLevelDiff), indexNode);
			
			indexNode=retpaser.CreateNodePtr(paramNode,"QualityIndex");
			retpaser.SetAttrNode("Type", string("4"), indexNode);
			retpaser.SetAttrNode("Desc", string("载噪比，单位:dbuv"), indexNode);
			retpaser.SetAttrNode("Value", strCNR, indexNode);

			indexNode=retpaser.CreateNodePtr(paramNode,"QualityIndex");
			retpaser.SetAttrNode("Type", string("5"), indexNode);
			retpaser.SetAttrNode("Desc", string("载频频偏，单位:kHz"), indexNode);
			retpaser.SetAttrNode("Value", string("0"), indexNode);
			retpaser.SaveToString(strRetMsg);
		}
	}
	SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
	return true;
}

bool ATVHTTPDeviceAccess::GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					   <Msg DVBType=\"TV\" >\
					   <Return Type=\"\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					   <ChannelScan></ChannelScan></Msg>";
	std::string retfailxml = "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							 <Msg DVBType=\"TV\" >\
							 <Return Type=\"\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
							 <ChannelScan></ChannelScan></Msg>";

	string strDeviceSrcXml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.16.10.250:11111\">\
							<Type>ATVChannelScanQuery</Type>\
							<Data>\
							<Channel>1</Channel>\
							<ScanParam>\
							<ScanType>STANDARD</ScanType>\
							</ScanParam>\
							</Data>\
							</Msg>";

	XmlParser parser;
	XmlParser deviceParser;

	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);

	pXMLNODE childSEnode=parser.GetNodeFirstChild(childnode);
	if(dvbtype != "ATV")
	{
		return false;
	}
    string strstartfreq,strendfreq,strstep;
    parser.GetAttrNode(childSEnode,"StartFreq",strstartfreq);
    parser.GetAttrNode(childSEnode,"EndFreq",strendfreq);
    int beginfreq=(StrUtil::Str2Float(strstartfreq)+0.0002)*1000,m_endfreq=(StrUtil::Str2Float(strendfreq)+0.0002)*1000;
    /***/
	//获得执行电视频谱扫描的任务通道
	std::list<int> devicelist;
	PROPMANAGER::instance()->GetTaskDeviceList("SpectrumScanTask",ATV, devicelist);
	if(devicelist.size() ==0 )
	{
		std::cout << "获得执行电视频谱扫描的任务通道失败" << std::endl;
		return false;
	}
	int sprecDevice = devicelist.front();
	std::vector<int> freqVec;
	bool ret = DEVICEACCESSMGR::instance()->defaultSpectRum(sprecDevice,freqVec);
	deviceParser.Set_xml(strDeviceSrcXml);
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	pXMLNODE scanParmNode = deviceParser.GetNodeFromPath("Msg/Data/ScanParam");
	pXMLNODE scanTypeNode = deviceParser.GetNodeFromPath("Msg/Data/ScanParam/ScanType");
	if(!ret || freqVec.size() == 0)
	{
		deviceParser.SetNodeText(scanTypeNode, "STANDARD");
	}
	else
	{
        int FreqNum = 0;
        for(int i=0;i<freqVec.size();i++)
        {
            int tmpFreq = freqVec[i];
            if(tmpFreq>=beginfreq && tmpFreq<=m_endfreq)
            {
                FreqNum++;
            }
        }
		deviceParser.SetNodeText(scanTypeNode, "FREQUENCY");
		pXMLNODE freqInfonode = deviceParser.CreateNodePtr(scanParmNode,"FrequencyInfo");
		deviceParser.SetAttrNode("FreqNum", StrUtil::Int2Str(FreqNum), freqInfonode);
		std::vector<int>::iterator iter = freqVec.begin();
		for(; iter != freqVec.end(); iter++)
		{
            int tmpIter = *iter;
            if(tmpIter>=beginfreq && tmpIter<=m_endfreq)
            {
                pXMLNODE freqnode = deviceParser.CreateNodePtr(freqInfonode,"Freq");
                deviceParser.SetNodeText(freqnode, StrUtil::Int2Str(tmpIter).c_str());
            }
		}
	}	
	string strDeviceXml;
	deviceParser.SaveToString(strDeviceXml);
	string strRetDeviceXml;
	APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_WARNNING,strDeviceXml,LOG_OUTPUT_SCREEN);
	if(!SendXmlTaskToDevice(strDeviceXml, strRetDeviceXml))
	{
		strRetMsg = retfailxml;
		SYSMSGSENDER::instance()->SendMsg(strRetMsg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}
	APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_WARNNING,strRetDeviceXml,LOG_OUTPUT_SCREEN);
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
	//
	/*pXMLNODE temfreqnode = retpaser.CreateNodePtr(retChanNode,"Channel");
	retpaser.SetAttrNode("Freq", string("535.25"), temfreqnode);*/

	retpaser.SaveToString(strRetMsg);
	std::cout<<"接收机扫描结果："<<strRetMsg<<endl;


	return true;
}

bool ATVHTTPDeviceAccess::GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
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
	int tuneFreq = StrUtil::Str2Float(freq)*1000;
	std::string TsIp;
	int tsport;
	PROPMANAGER::instance()->GetDeviceTsIP(DeviceId, TsIp);
	PROPMANAGER::instance()->GetDeviceTsPort(DeviceId, tsport);

	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Tuner/TunerType"), "TV");
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Tuner/Frequency"), StrUtil::Int2Str(tuneFreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Tuner/ModulationType"), "PALDK");

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

bool ATVHTTPDeviceAccess::GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg)
{
	string retsuccxml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg DVBType=\"TV\" >\
					  <Return Type=\"SpectrumQuery\" Value=\"0\" Desc=\"成功\" Comment=\"\"/> \
					  <SpectrumQueryReport STD=\"\" SymbolRate=\"\"><SpectrumParam></SpectrumParam></SpectrumQueryReport></Msg>";
	std::string retfailxml = "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							 <Msg DVBType=\"RADIO\" >\
							 <Return Type=\"SpectrumQuery\" Value=\"1\" Desc=\"失败\" Comment=\"\"/> \
							 <SpectrumQueryReport STD=\"\" SymbolRate=\"\"><SpectrumParam></SpectrumParam></SpectrumQueryReport></Msg>";

	string strDeviceSrcXml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.16.10.250:11111\">\
							<Type>ATVSpectrumScanQuery</Type>\
							<Data>\
							<Channel>1</Channel>\
							<ScanParam>\
							<ScanType>BAND</ScanType>\
							<StartFrequency>45000</StartFrequency>\
							<EndFrequency>860000</EndFrequency>\
							<StepSize>1000</StepSize>\
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
	pXMLNODE channscannode = parser.GetNodeFirstChild(root);
	//	string nodename=parser.GetNodeName(childnode);

	if(dvbtype != "ATV")
	{ 
		return false;
	}
	//	pXMLNODE channscannode=parser.GetNodeFirstChild(childnode);
	string strstartfreq,strendfreq,strstep,strVBW,strRBW,strtaskType;
	parser.GetAttrNode(channscannode,"WorkType",strtaskType);
	parser.GetAttrNode(channscannode,"StartFreq",strstartfreq);
	parser.GetAttrNode(channscannode,"EndFreq",strendfreq);
	parser.GetAttrNode(channscannode,"StepFreq",strstep);
	parser.GetAttrNode(channscannode,"VBW",strVBW);
	parser.GetAttrNode(channscannode,"RBW",strRBW);
	
	
	/*strstartfreq = "45";
	strendfreq = "860";*/
	if(strtaskType!="0")
	{
		strstep = "1";
		strVBW = "100";
		strRBW = "3000";
	}
	int beginfreq=(StrUtil::Str2Float(strstartfreq)+0.0002)*1000,m_endfreq=(StrUtil::Str2Float(strendfreq)+0.0002)*1000;
	/*int startfreq=(StrUtil::Str2Int(strstartfreq)+0.0002)*1000,endfreq=(StrUtil::Str2Int(strendfreq)+0.0002)*1000,
		freqstep=(StrUtil::Str2Float(strstep)+0.0002)*1000;*/
	int startfreq=StrUtil::Str2Int1(strstartfreq)*1000,endfreq=StrUtil::Str2Int1(strendfreq)*1000,
		freqstep=(StrUtil::Str2Float(strstep)+0.0002)*1000;
	if(beginfreq < 48000 )
	{
		beginfreq = 48000;
	}
	else if(m_endfreq > 860000)
	{
		m_endfreq = 860000;
	}
	int CountMax = (m_endfreq - beginfreq)/freqstep + 1;
	if(strtaskType=="0")
	{
		if(CountMax>=500)
		{
			freqstep = 2000;
		}
	}
	
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/StartFrequency"), StrUtil::Int2Str(startfreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/EndFrequency"), StrUtil::Int2Str(endfreq).c_str());
	deviceParser.SetNodeText(deviceParser.GetNodeFromPath("Msg/Data/ScanParam/StepSize"), StrUtil::Int2Str(freqstep).c_str());

	string strDeviceXml;
	deviceParser.SaveToString(strDeviceXml);
    string SpectrumFlag,SpectrumVBW,SpectrumRBW;//默认发中间件
    SpectrumFlag = PROPMANAGER::instance()->GetIsSpectrumFlag();//获取type
    if(SpectrumFlag=="")
    {
        SpectrumFlag = "3";//type非0 1 2的任何值
    }
    int SpecType = StrUtil::Str2Int(SpectrumFlag);
    if(SpecType==0)//本机直通逻辑
    {   
        std::string strRetXml = "";
		bool ATV_ReadFlag = false;
		int m_num = 0;
		string MEASureCmd = "";
		int FreqNcount = (endfreq - startfreq)/freqstep + 1;
		SpectrumParm parmSet;
		parmSet.endFreq = endfreq;
		parmSet.SpecType = SpecType;
		parmSet.startFreq = startfreq;
		parmSet.stepFreq  = freqstep;
		parmSet.SpecRBW = strRBW;
		parmSet.SpecVBW = strVBW;
		GetATVSpecTrumParmSet(parmSet,MEASureCmd);
		if(MEASureCmd!="")
			MEASureCmd += "\r\n";
		do
		{
			if(!SpecFlagSet)
			{
				SpecOrderSet = MEASureCmd;
				SpecFlagSet = true;
			}
			Sleep(300);
			ALLReadMutex.acquire();
			SpectrumTcpBaseC Spectrum(m_ip.c_str(),m_port);
			string SpectrumRL = "MEASure:SPECtrum:DATa?\r\n";//读取频谱曲线有效数据//MEASure:SPECtrum:SETTing?
			int Ncount = 0;
			if(Spectrum.ConnectSpecEquiCard())
			{
				string SpectrumSetPA = "MEASure:SPECtrum:SETTing?";
				SpectrumSetPA +="\r\n";
				string strPa = "";
				if(Spectrum.sendSpecOrder(SpectrumSetPA.c_str(),SpectrumSetPA.length()))
				{
					Spectrum.recSpecOrder(strPa);
				}//StrUtil::Int2Str(SetMidFreq)
				if(strPa.find(StrUtil::Int2Str(FreqNcount).c_str())!=-1)
				{
					cout<<"设置成功："<<strPa<<endl;
					while(Ncount<10)//设定5次读取有效数据
					{
						Sleep(200);
						std::string strtempRetXml = "";
						cout<<"读取频谱曲线指令："<<SpectrumRL<<endl;
						if(Spectrum.sendSpecOrder(SpectrumRL.c_str(),SpectrumRL.length()))
						{
							Spectrum.recSpecOrder(strtempRetXml);
						}
						int kn = Spectrum.SpectrumMap.size();
						if(strtempRetXml!="")
						{
							if(kn==FreqNcount)
							{
								if(strtempRetXml.find("3264")!=-1)
								{
									ATV_ReadFlag = true;
								}
								else
								{
									ATV_ReadFlag = false;
									strRetXml = strtempRetXml;
									break;
								}
							}
							else
								ATV_ReadFlag = true;
						}
						else
							ATV_ReadFlag = true;
						Ncount++;
					}
				}
				else
				{
					cout<<"设置失败："<<strPa<<endl;
					ATV_ReadFlag = true;
				}
			}
			ALLReadMutex.release();
			m_num++;
		}while(ATV_ReadFlag && m_num<5);
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
			int pbeginfreq = beginfreq;
			if(strtaskType=="0")
			{
				//int num = 0;
				while (1)
				{
					int npos = strRetXml.find(';');
					if (strRetXml != "")
					{
						if (npos != -1)
						{
							//num++;
							strTemp = strRetXml.substr(0, npos);
							strEnd = strRetXml.substr(++npos, strRetXml.length() - npos);
							SpectrumNode = retpaser.CreateNodePtr(SpectrumParamNode,"SpectrumIndex");
							if(strEnd!="")
							{
								retpaser.SetAttrNode("Freq",StrUtil::Int2Str(pbeginfreq),SpectrumNode);
								retpaser.SetAttrNode("Value",strTemp,SpectrumNode);
							}
							else
							{
								retpaser.SetAttrNode("Freq",StrUtil::Int2Str(m_endfreq),SpectrumNode);
								retpaser.SetAttrNode("Value",strTemp,SpectrumNode);
							}
							//retpaser.SetAttrNode("NUM",StrUtil::Int2Str(num),SpectrumNode);
						}
						pbeginfreq += freqstep;
						strRetXml = strEnd;
					}
					else
						break;
				}
			}
			else
			{
				while (1)
				{
					int npos = strRetXml.find(';');
					if (strRetXml != "")
					{
						if (npos != -1)
						{
							strTemp = strRetXml.substr(0, npos);
							strEnd = strRetXml.substr(++npos, strRetXml.length() - npos);
							for(int i=0;i<sizeof(ScanFreq)/sizeof(int);++i)
							{
								int tempFreq = ScanFreq[i];
								if(abs(pBegin-tempFreq)<300)
								{
									SpectrumNode = retpaser.CreateNodePtr(SpectrumParamNode,"SpectrumIndex");
									retpaser.SetAttrNode("Freq",StrUtil::Int2Str(tempFreq),SpectrumNode);
									retpaser.SetAttrNode("Value",strTemp,SpectrumNode);
									break;
								}
							}
						}
						pBegin += freqstep;
						strRetXml = strEnd;
					}
					else
						break;
				}
			}
			retpaser.SaveToString(strRetMsg);
		}
	}
	else//中间件逻辑
	{
#if 0
		SpectrumTcpBaseC Spectrum(m_ip.c_str(),m_port);
		string strRXml;
		string strRetDeviceXml;
		int mCount = 0;
		bool isRxml = false;
		while(mCount<3)
		{
			//Spectrum.SendSpectrumCommand(strDeviceXml.c_str(),strDeviceXml.length(),strRXml);
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
			if(strXmlRet != std::string("SUCCESS"))
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
				fTmpLevel = StrUtil::Str2Float(strTmpLevel);
				pXMLNODE freqnode = retpaser.CreateNodePtr(retChanNode,"SpectrumIndex");
				retpaser.SetAttrNode("Freq", StrUtil::Int2Str(iTemFreq), freqnode);
				retpaser.SetAttrNode("Value",StrUtil::Float2Str(fTmpLevel),freqnode);
			}
			retpaser.SaveToString(strRetMsg);
		}
		else
			return false;
#else
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
			fTmpLevel = StrUtil::Str2Float(strTmpLevel);
			pXMLNODE freqnode = retpaser.CreateNodePtr(retChanNode,"SpectrumIndex");
			retpaser.SetAttrNode("Freq", StrUtil::Int2Str(iTemFreq), freqnode);
			retpaser.SetAttrNode("Value",StrUtil::Float2Str(fTmpLevel),freqnode);
		}
		retpaser.SaveToString(strRetMsg);
#endif
	}
	std::cout<<"接收机扫描结果："<<strRetMsg<<endl;
	APPLOG::instance()->WriteLog(RECORD,LOG_EVENT_WARNNING,strRetMsg,LOG_OUTPUT_FILE);
	return true;
}
bool ATVHTTPDeviceAccess::setCardSystemTime()
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


bool ATVHTTPDeviceAccess::getChanFixInfoFor6U()
{
	std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
							<Type>GetATVQualityInfo</Type>\
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

		pXMLNODELIST qualityNodeList = retDeviceParser.GetNodeList(retDeviceParser.GetNodeFromPath("Msg/Data"));
		pXMLNODE tmpNode = retDeviceParser.GetNextNode(qualityNodeList);
		std::string status = retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Status"));
// 		std::string strLevel =  retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Level"));
// 		std::cout << "DeviceID[" << DeviceId << "],Level:" << strLevel << std::endl;
		if(status != "1")
		{
			return false;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool ATVHTTPDeviceAccess::getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj)
{
	std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
							<Type>GetATVQualityInfo</Type>\
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
			retObj.ChanFixObj[i].status = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Status")));
			retObj.ChanFixObj[i].strength = StrUtil::Str2Int(retDeviceParser.GetNodeText(retDeviceParser.findNode(tmpNode, "Level")));
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool ATVHTTPDeviceAccess::setTunerInfo(int chanNo, int freq)
{
	std::string strSrcXml = "<?xml version=\"1.0\" encoding=\"gb2312\"?>\
							<Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.17.6.5:1000\">\
							<Type>SetTunerInfo</Type>\
							<Data>\
							<Channel>1</Channel>\
							<Frequency>714000</Frequency>\
							<TunerType>0</TunerType>\
							<ModulationType>0</ModulationType>\
							</Data>\
							</Msg>";

	XmlParser srcParser;
	srcParser.Set_xml(strSrcXml);
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/TunerType"), "TV");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Frequency"), StrUtil::Int2Str(freq).c_str());
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ModulationType"), "PALDK");
	

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

bool ATVHTTPDeviceAccess::SetEncoderSwitch(bool enable)
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

bool ATVHTTPDeviceAccess::SetAlarmThreshold()
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
	std::string strDevIP,strAudioPower,strSignalThresholdRet;
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

	strAudioPower = PROPMANAGER::instance()->GetTVAudioPowerRet();//GetSignalThresholdRet
	if(strAudioPower=="")
	{
		strAudioPower = "60";
	}
	strSignalThresholdRet = PROPMANAGER::instance()->GetSignalThresholdRet();
	if(strSignalThresholdRet=="")
	{
		strSignalThresholdRet = "500";
	}
	XmlParser srcParser;
	srcParser.Set_xml(strSrcXml);
	int tmpDeviceID = 0;
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,tmpDeviceID);
	//srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), StrUtil::Int2Str(tmpDeviceID).c_str());
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/Channel"), "ALL");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/BlackSimilar"), "990");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/ColourBarSimilar"), "990");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/FreezeSimilar"), "950");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/VolumeHighValue"), "100");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/VolumeLowValue"), "5");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/AudioLostValue"), strAudioPower.c_str());
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/AudioUnsualLastTime"), "500");
	srcParser.SetNodeText(srcParser.GetNodeFromPath("Msg/Data/ThresholdInfo/LostSignal"), strSignalThresholdRet.c_str());
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

bool ATVHTTPDeviceAccess::defaultSpectRum(std::vector<int>& freqVec)
{
    string SpectrumFlag;//默认发中间件
    SpectrumFlag = PROPMANAGER::instance()->GetIsSpectrumFlag();//获取type
    if(SpectrumFlag!="0")
    {
        SpectrumFlag = "3";//type非0 1 2的任何值
    }
    int SpecType = StrUtil::Str2Int(SpectrumFlag);
    if(SpecType==0)//本机直通逻辑
    {
		std::string strRetXml = "";
		bool Channel_Flag = false;
		int m_num = 0;
		do{
			if(!SpecFlagSet)
			{
				FgNum = 1;
				SpecFlagSet = true;
			}
			Sleep(500);
			ALLReadMutex.acquire();
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
				}//StrUtil::Int2Str(SetMidFreq)
				if(strPa.find("813")!=-1)
				{
					cout<<"设置成功："<<strPa<<endl;
					while(Ncount<15)//设定5次读取有效数据
					{
						string strtempRetXml = "";
						if(Spectrum.sendSpecOrder(SpectrumRL.c_str(),SpectrumRL.length()))
						{
							Spectrum.recSpecOrder(strtempRetXml);
						}
						if(strtempRetXml!="")
						{
							if(Spectrum.SpectrumMap.size()==813)
							{
								if(strtempRetXml.find("3264")!=-1)
								{
									Sleep(200);
									Channel_Flag = true;
								}
								else /*if(strtempRetXml.size()>0 && strtempRetXml.find("3264")==-1)*/
								{
									Channel_Flag = false;
									strRetXml = strtempRetXml;
									break;
								}
							}
							else
								Channel_Flag = true;
						}
						else
							Channel_Flag = true;
						Ncount++;
					}
				}
				else
				{
					cout<<"设置失败："<<strPa<<endl;
					Channel_Flag = true;
				}
			}
			ALLReadMutex.release();
			m_num++;
		}while(Channel_Flag && m_num<5);

		int pBegin = 48000;
		string strEnd = "",strTemp="";
		int i = 0;
		while (true)
		{
			int npos = strRetXml.find(';');
			if (strRetXml != "")
			{
				if (npos != -1)
				{
					strTemp = strRetXml.substr(0, npos);
					strEnd = strRetXml.substr(++npos, strRetXml.length() - npos);
					if(StrUtil::Str2Float(strTemp)>30)
					{
						//cout<<"频点："<<ScanFreq[i]<<"--- 电平："<<strTemp<<endl;
						for(int i=0;i<sizeof(ScanFreq)/sizeof(int);++i)
						{
							int tempFreq = ScanFreq[i];
							if(abs(pBegin-tempFreq)<300)
							{
								freqVec.push_back(tempFreq);
								break;
							}
						}
					}
				}
				pBegin += 1000;
				strRetXml = strEnd;
			}
			else
				break;
		}
	}
    else
    {
        string strDeviceSrcXml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
                                <Msg Version=\"1.0\" MsgID=\"10000\" SrcUrl=\"http://172.16.10.250:11111\">\
                                <Type>ATVSpectrumScanQuery</Type>\
                                <Data>\
                                <Channel>1</Channel>\
                                <ScanParam>\
                                <ScanType>BAND</ScanType>\
                                <StartFrequency>45000</StartFrequency>\
                                <EndFrequency>860000</EndFrequency>\
                                <StepSize>1000</StepSize>\
                                </ScanParam>\
                                </Data>\
                                </Msg>";

        string strRetDeviceXml;
        freqVec.clear();
        if(!SendXmlTaskToDeviceNoBlock(strDeviceSrcXml, strRetDeviceXml))
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

        int iTemFreq=0;
        int iTmpLevel=0;

        std::string strTmpFreq, strTmpLevel;
        pXMLNODELIST channelNodeList = retDeviceParser.GetNodeList(retDeviceParser.GetNodeFromPath("Msg/Data/ScanResult"));

        for (int i=0; i<channelNodeList->Size(); i++)
        {
            bool isPushBack = false;
            freqNode = retDeviceParser.GetNextNode(channelNodeList);
            retDeviceParser.GetAttrNode(freqNode, "Frequency", strTmpFreq);
            retDeviceParser.GetAttrNode(freqNode, "Level", strTmpLevel);
            iTemFreq = StrUtil::Str2Int(strTmpFreq);
            for(int i = 0 ; i < sizeof(ScanFreq)/sizeof(int); i++)
            {
                //ScanFreq[i] = ScanFreq[i]/1000;
                if(iTemFreq == ScanFreq[i])
                {
                    isPushBack = true;
                    break;
                }
            }
            if(!isPushBack)
                continue;
            iTmpLevel = StrUtil::Str2Int(strTmpLevel);
            if(iTmpLevel > 45)
                freqVec.push_back(iTemFreq);
        }
    }
	return true;
}
bool ATVHTTPDeviceAccess::Find_str(string str,char** desc,int DescLen)
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
void ATVHTTPDeviceAccess::GetATVSpecTrumParmSet(SpectrumParm parmSet,string &MEASureCmd)
{
	int SetMidFreq = 1000*(parmSet.startFreq + parmSet.endFreq)/2;
	int SpanFreq = 1000*(parmSet.endFreq - parmSet.startFreq);
	int FreqNcount = (parmSet.endFreq - parmSet.startFreq)/parmSet.stepFreq + 1;

	string strVBW = "";
	MEASureCmd = "MEASure:SPECtrum:SETTing ";
	MEASureCmd += "0";//参数1------Type [0,1,2],0-normal,1-return,2-DPS
	MEASureCmd += ",";
	MEASureCmd += StrUtil::Int2Str(SetMidFreq);//参数2: FreqHz ，单位固定为Hz
	MEASureCmd += "Hz,";
	if(parmSet.SpecType== 1 || parmSet.SpecType==2)
	{
		if(SpanFreq>206*1000000)
		{
			SpanFreq=206*1000000;
		}
	}
	MEASureCmd += StrUtil::Int2Str(SpanFreq);//参数3: SpanHz ，单位固定为Hz，Type=1 或 2 时，最大扫宽为206MHz
	MEASureCmd += "Hz,";
	MEASureCmd += "auto-att";//参数4: Att[0-30, auto-att] ，单位dB//注:auto-att 时，须填写有效的RFE
	MEASureCmd += ",";
	MEASureCmd += "ampoff";//参数5: Amp[“ampoff”,”ampon”] ，0-放大器关，1-放大器开
	MEASureCmd += ",";
	if(parmSet.SpecType== 1 || parmSet.SpecType==2)//参数6: Rbw[“1khz”,”3 khz”,”10 khz”,”30 khz”,”100 khz”,”300 khz”,”1mhz”,”3 mhz”,"auto-rbw"]注：Type=1 或2 时，固定为300khz
	{
		MEASureCmd += "300KHz";
	}
	else
	{
		int IRBW = 0;
		string strRBW = parmSet.SpecRBW;
		char* strSpecRBW[] = {"1","3","10","30","100","300","1000","3000","auto-rbw"};
		if(strRBW!="auto-rbw")
		{
			IRBW = StrUtil::Str2Int(strRBW);
			/*if(IRBW<2)
			{
				IRBW = 1;
			}
			else if(IRBW>=2 && IRBW<6)
			{
				IRBW = 3;
			}*/
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
			strRBW = "1000";
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
	if(parmSet.SpecType==2)
	{
		MEASureCmd+="300KHz";
	}
	else
	{
		if(parmSet.SpecVBW != "auto-vbw")
		{
			int IVBW = StrUtil::Str2Float(parmSet.SpecVBW) * 1000;
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
				IVBW = 300;
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
	//MEASureCmd += "\r\n";
}
