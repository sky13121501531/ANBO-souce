#pragma once
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "PSISIParser.h"

class PSISIMgr 
{
public:
	PSISIMgr();
	~PSISIMgr();
public:
	bool CreateChannelXML(eDVBType dvbtype);
	bool CreateEPGXML(eDVBType dvbtype);
	bool CreateTableXML(eDVBType dvbtype);
	bool UpdatePSISI(eDVBType dvbtype,std::string strXML);
	bool GetVersionInfo(eDVBType dvbtype,mapFileVersion& mapfileversion);
	bool Init(void);
	bool Init(eDVBType dvbtype,string taskname);
private:
	PSISIParser* pDVBCPsiSiParser;
	PSISIParser* pCTTBPsiSiParser;
	PSISIParser* pDVBSPsiSiParser;
	PSISIParser* pTHREEDPsiSiParser;
};

typedef ACE_Singleton<PSISIMgr,ACE_Mutex>  PSISIMGR;



