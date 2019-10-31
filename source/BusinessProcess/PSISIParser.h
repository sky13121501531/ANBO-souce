#pragma once

#include <string>
#include <vector>
#include "../Foundation/TypeDef.h"
#include <ace/Task.h>
#include <map>
using namespace std;

typedef struct
{  
	string ProgramNumber;
	string ProgramID;
}PAT;
typedef struct
{
	string OrgNetID;
	string TsID;
	string ServiceID;
	string FreeCAMode;
	string ServiceName;
	string ServiceType;
}SDT;
typedef struct
{
	string Freq;
	string OrgNetID;
	string TsID;
	string SymbolRate;
	string QAM;
}NIT;
typedef struct  
{
	string StreamType;
	string StreamPid;
}STREAM;
typedef struct
{
	string ProgramNumber;
	string VideoPID;
	string AudioPID;
	//wz_101123
	string PmtPID;  
	string PcrPID;
	//wz_101123
	vector<STREAM> vecStream;
}PMT;
typedef struct
{
	string EventID;
	string OrgNetID;
	string TsID;
	string ServiceID;
	string ProgramName;
	string ProgramStartTime;
	string ProgramDuration;
	string FreeCAMode;
	string RunningStatus;
}EIT;

struct FileVersion
{
	string FileName;
	string Version;
};
typedef struct
{
	string Freq;
	string OrgNetID;
	string TsID;
	string ServiceID; 
}HDTV;


typedef vector<NIT> vctNITInfo;			//Ƶ����Ϣ
typedef map< std::string, vector<NIT> >  mapNITInfo;
typedef vector<PAT> vctPATInfo;				//��Ƶ���PAT��Ϣ
typedef map< std::string, vector<PAT> >  mapPATInfo;//��Ƶ���PAT��Ϣ
typedef vector<PMT> vctPMTInfo;
typedef map< std::string, vector<PMT> >  mapPMTInfo;
typedef vector<EIT> vctEITInfo;
typedef map< std::string, vector<EIT> >  mapEITInfo;
typedef vector<SDT> vctSTDInfo;				//��Ƶ���SDT��Ϣ
typedef map< std::string, vector<SDT> >  mapSDTInfo;//��Ƶ���SDT��Ϣ
typedef map< std::string, vector<FileVersion> > mapFileVersion;
typedef vector<HDTV> vctHDTVInfo;//����Ƶ����Ϣ


class PSISIParser 
{
 public:
	PSISIParser(eDVBType dvbtype);
	~PSISIParser();
protected:
	PSISIParser();
public:
	bool CreateChannelXML();
	bool CreateEPGXML();
	bool CreateTableXML();
	bool UpdatePSISI(std::string strXML);
	bool GetVersionInfo(mapFileVersion& mapfileversion);
	bool Init(void);
	bool Init(string taskname);
private:
	bool LoadPSISIInfoFromFile(string taskname);
private:
	string Path;
	eDVBType DVBType;
private:
	mapPATInfo  mapPAT;
	mapPMTInfo  mapPMT;
	vctNITInfo  vecNIT;
	mapSDTInfo  mapSDT;
	mapEITInfo  mapEIT;
	mapFileVersion mapVersion;
	vctEITInfo  vecEITtemp;
	vctSTDInfo  vecSDTtemp;
	vctHDTVInfo vecHDTV;

	ACE_Thread_Mutex Mutex;
};



