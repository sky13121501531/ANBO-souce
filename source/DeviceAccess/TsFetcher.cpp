
#include "TsFetcher.h"
#include "../Foundation/PropManager.h"

TsFetcher::TsFetcher(int deviceid)
{
	DeviceId = deviceid;//Í¨µÀºÅ
	PROPMANAGER::instance()->GetDeviceTunerID(DeviceId,TunerID);
}

TsFetcher::~TsFetcher()
{

}

unsigned int TsFetcher::FindBegin(unsigned char* tsBuf,unsigned int tsLen)
{
	unsigned int nIndex = 0;

	while((nIndex+188) < tsLen)
	{
		if((tsBuf[nIndex] == 0x47) && (tsBuf[nIndex+188] == 0x47))
		{
			bool bFindGroup = false;
			for (int i=0;i<188;++i)
			{
				if (tsBuf[nIndex+i] == 0x00 && tsBuf[nIndex+i+1] == 0x00 && tsBuf[nIndex+i+2] == 0x00 && tsBuf[nIndex+i+3] == 0x01)
				{
					bFindGroup = true;
					break;
				}
			}
			if (bFindGroup == true)
			{
				break;
			}
			nIndex += 188;
			continue;
		}
		else
		{
			++nIndex;
		}
	}
	return nIndex;
}