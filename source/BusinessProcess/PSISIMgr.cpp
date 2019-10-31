
#include "PSISIMgr.h"

PSISIMgr::PSISIMgr()
{
	pDVBCPsiSiParser=new PSISIParser(DVBC);
	pCTTBPsiSiParser=new PSISIParser(CTTB);
	pDVBSPsiSiParser=new PSISIParser(DVBS);
	pTHREEDPsiSiParser = new PSISIParser(THREED);
}
PSISIMgr::~PSISIMgr()
{
	if(NULL!=pDVBCPsiSiParser)
	{
		delete pDVBCPsiSiParser;
		pDVBCPsiSiParser=NULL;
	}
	if(NULL!=pCTTBPsiSiParser)
	{
		delete pCTTBPsiSiParser;
		pCTTBPsiSiParser=NULL;
	}
	if(NULL!=pDVBSPsiSiParser)
	{
		delete pDVBSPsiSiParser;
		pDVBSPsiSiParser=NULL;
	}
	if(NULL!=pTHREEDPsiSiParser)
	{
		delete pTHREEDPsiSiParser;
		pTHREEDPsiSiParser=NULL;
	}
}

bool PSISIMgr::CreateChannelXML( eDVBType dvbtype )
{
	if(dvbtype==CTTB)
	{
		return pCTTBPsiSiParser->CreateChannelXML();
	}
	else if(dvbtype==DVBC)
	{
		return pDVBCPsiSiParser->CreateChannelXML();
	}
	else if(dvbtype==DVBS)
	{
		return pDVBSPsiSiParser->CreateChannelXML();
	}
	else if(dvbtype==THREED)
	{
		return pTHREEDPsiSiParser->CreateChannelXML();
	}
	return false;

}

bool PSISIMgr::CreateEPGXML( eDVBType dvbtype )
{
	if(dvbtype==CTTB)
	{
		return pCTTBPsiSiParser->CreateEPGXML();
	}
	else if(dvbtype==DVBC)
	{
		return pDVBCPsiSiParser->CreateEPGXML();
	}
	else if(dvbtype==DVBS)
	{
		return pDVBSPsiSiParser->CreateEPGXML();
	}
	else if(dvbtype==THREED)
	{
		return pTHREEDPsiSiParser->CreateEPGXML();
	}
	return false;
}

bool PSISIMgr::CreateTableXML( eDVBType dvbtype )
{
	if(dvbtype==CTTB)
	{
		return pCTTBPsiSiParser->CreateTableXML();
	}
	else if(dvbtype==DVBC)
	{
		return pDVBCPsiSiParser->CreateTableXML();
	}
	else if(dvbtype==DVBS)
	{
		return pDVBSPsiSiParser->CreateTableXML();
	}
	else if(dvbtype==THREED)
	{
		return pTHREEDPsiSiParser->CreateTableXML();
	}
	return false;
}

bool PSISIMgr::UpdatePSISI( eDVBType dvbtype,std::string strXML )
{
	if(dvbtype==CTTB)
	{
		return pCTTBPsiSiParser->UpdatePSISI(strXML);
	}
	else if(dvbtype==DVBS)
	{
		return pDVBSPsiSiParser->UpdatePSISI(strXML);
	}
	return false;
}

bool PSISIMgr::GetVersionInfo( eDVBType dvbtype,mapFileVersion& mapfileversion )
{
	if(dvbtype==CTTB)
	{
		return pCTTBPsiSiParser->GetVersionInfo(mapfileversion);
	}
	else if(dvbtype==DVBC)
	{
		return pDVBCPsiSiParser->GetVersionInfo(mapfileversion);
	}
	else if(dvbtype==DVBS)
	{
		return pDVBSPsiSiParser->GetVersionInfo(mapfileversion);
	}
	else if(dvbtype==THREED)
	{
		return pTHREEDPsiSiParser->GetVersionInfo(mapfileversion);
	}
	return false;
}

bool PSISIMgr::Init( void )
{
	pCTTBPsiSiParser->Init();
	pDVBCPsiSiParser->Init();
	pDVBSPsiSiParser->Init();
	pTHREEDPsiSiParser->Init();
	return true;
}
bool PSISIMgr::Init(eDVBType dvbtype,string taskname)
{
	switch(dvbtype)
	{
	case CTTB:
		{
			pCTTBPsiSiParser->Init(taskname);
			break;
		}		
	case DVBC:
		{
			pDVBCPsiSiParser->Init(taskname);
			break;
		}	
	case DVBS:
		{
			pDVBSPsiSiParser->Init(taskname);
			break;
		}
	case THREED:
		{
			pTHREEDPsiSiParser->Init(taskname);
			break;
		}
	default:
		break;
	}
	return true;
}