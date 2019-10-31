#include "QualityCompensationMgr.h"
#include "../DBAccess/DBManager.h"

QualityCompensationMgr::QualityCompensationMgr()
{
	init();
}
QualityCompensationMgr::~QualityCompensationMgr()
{

}

bool QualityCompensationMgr::UpdateQualityCompensation(std::vector<sQualityCompensation> qualityCmpVec)
{
	std::vector<sQualityCompensation>::iterator ptr=qualityCmpVec.begin();
	for (;ptr!=qualityCmpVec.end();++ptr)
	{
		ACE_Guard<ACE_Thread_Mutex> guard(QualityComMutex);
		std::vector<sQualityCompensation>::iterator pptr=mQulaityCmpVec.begin();
		for (;pptr!=mQulaityCmpVec.end();++pptr)
		{
			if((*pptr).dvbtype==(*ptr).dvbtype&&(*pptr).deviceid==(*ptr).deviceid&&(*pptr).type==(*ptr).type)
			{
				(*pptr).valu=(*ptr).valu;
				break;
			}
		}
		if(pptr==mQulaityCmpVec.end())
		{
			mQulaityCmpVec.push_back(*ptr);
		}
		DBMANAGER::instance()->UpdateCompensationValu(*ptr);
	}
	return true;
}

bool QualityCompensationMgr::GetQualityCompensation( eDVBType dvbtype,std::string deviceid,std::string type,std::string& valu )
{
	ACE_Guard<ACE_Thread_Mutex> guard(QualityComMutex);
	std::vector<sQualityCompensation>::iterator ptr=mQulaityCmpVec.begin();
	for (;ptr!=mQulaityCmpVec.end();++ptr)
	{
		if((*ptr).dvbtype==dvbtype&&(*ptr).deviceid==deviceid&&(*ptr).type==type)
		{
			valu=(*ptr).valu;
			break;
		}
	}
	return true;
}
void QualityCompensationMgr::init()
{
	DBMANAGER::instance()->QueryCompensationValu(mQulaityCmpVec);
}
