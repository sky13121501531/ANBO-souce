#pragma warning(disable:4996)

#include <math.h>
#include "StrUtil.h"
#if defined(WIN32) || defined(__WIN32__)
#include <Windows.h>
#pragma comment(lib,"kernel32.lib")
#endif

static unsigned char ascii_hei_40_width_data[128] = 
{	
	30,30,30,30,30,30,30,30,30,30,30,30,30,
	30,30,30,30,30,30,30,30,30,30,30,30,30,
	30,30,30,30,30,30,30,12,16,26,25,38,31,
	10,16,16,18,26,11,16,12,15,25,19,25,25,
	25,25,25,25,25,25,11,11,26,26,26,25,45,
	31,29,32,32,29,27,34,30,11,21,31,25,35,
	30,34,30,35,33,29,28,30,31,43,31,31,28,
	14,15,12,22,27,13,25,25,24,24,25,17,24,
	24,10,10,24,10,36,24,25,25,24,18,23,15,
	24,24,33,24,24,24,17,11,16,26,30
}; 

static unsigned char ascii_hei_20_width_data[128] = 
{
	0, 16, 16, 16, 16, 10,  0, 24,  10,  0, 11,  0,  0,  6,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	6,  5,  7, 12, 11, 15, 15,  5,  8,  7, 12, 10,  5, 11,  5,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10,  8,  7, 11, 10, 11, 10,
	18, 14, 14, 14, 14, 13, 12, 14, 14,  5, 10, 15, 11, 16, 14, 15,
	13, 15, 14, 13, 12, 14, 14, 17, 13, 14, 12,  7,  6,  6, 11, 11,
	5, 12, 12, 11, 12, 11,  8, 12, 11,  5,  5, 11,  5, 17, 11, 12,
	12, 12,  8, 11,  7, 11, 10, 16, 11, 11, 10,  9,  6,  8, 11, 16
};

string StrUtil::RoundFloat(double fOriginal,int nSaveBits)
{
    static const int DECIMAL_BASE = 10;    
    unsigned int n,scale;
    double fEnd;
    char sEnd[100] = {0};
    string retValue;

    n = 0;
    scale = 1;
    double fOriginal_f = fabs(fOriginal);
    for (int i=0;i<nSaveBits;i++)  
        scale *= DECIMAL_BASE;
    scale *= 2;
    n = (int)(fOriginal_f * scale); 

    n += (n % 2);
    fEnd = ((double)n /scale);
    if(fOriginal < 0)
        sprintf(sEnd,"-%0.2f",fEnd);
    else
        sprintf(sEnd,"%0.2f",fEnd);
    retValue = string(sEnd);
    return retValue;
}

int StrUtil::Str2Int(string Str)
{
    if (!Str.empty())
        return atoi(Str.c_str());
    else return 0;
}
int StrUtil::Str2Int1( string Str )
{
	float ret = -1;
	if (!Str.empty())
	{
		ret = atof(Str.c_str());
	}
	return (ret>0) ? floor(ret + 0.5) : ceil(ret - 0.5);
}
string StrUtil::Int2Str(int integer)
{
    char ret[64] = {0};
    ::sprintf(ret,"%d",integer);
    return ret;
}

long StrUtil::Str2Long(string Str)
{
    if (!Str.empty())
        return atol(Str.c_str());
    else return 0;
}

string StrUtil::Long2Str(long value)
{
    char ret[64] = {0};
    	::sprintf(ret,"%d",value);
    return ret;
}

long StrUtil::CiFang(long baseInt,long numCF)
{  
    if (0 ==numCF) return 1;
    long result = baseInt;
    for ( int i = 2 ; i <= numCF; i ++)
        result = result * baseInt;
    return result;
}

float StrUtil::Str2Float(string str)
{
    if (str.empty() || str.size() == 0) 
		str="0";
    return (float)atof(str.c_str());
}
float StrUtil::Str2FloatForFMFreq(string str)
{
	if (str.empty() || str.size() == 0) 
		str="0";
	return (float)atof(str.c_str()) + 0.0001F;
}

string StrUtil::Float2Str(float floater)
{
    string ret = RoundFloat((double)floater,2);

    return ret;
}
std::string StrUtil::Float2Str1( float floater )
{
	char result[100];
	double dOrginaData=(double)floater*10;
	int iOrginalData=(int)dOrginaData;
	double subRes=dOrginaData-(double)iOrginalData;
	int iLeft=(int)(subRes*2.0);

	int iEnd=dOrginaData;
	iEnd+=iLeft;
	double dResult=(double)iEnd/10;

	sprintf(result,"%0.1f",dResult);

	string strResult=string(result);
	return strResult;
}

int StrUtil::Str2IntForQam(string str)
{
	if(!strcmp(str.c_str(),"QAM16"))
	{
		return 0;
	}
	else if(!strcmp(str.c_str(),"QAM32"))
	{
		return 1;
	}
	else if(!strcmp(str.c_str(),"QAM64"))
	{
		return 2;
	}
	else if(!strcmp(str.c_str(),"QAM128"))
	{
		return 3;
	}
	else if(!strcmp(str.c_str(),"QAM256"))
	{
		return 4;
	}
	else if(!strcmp(str.c_str(),"QAM4"))
	{
		return 5;
	}
	else
	{
		return Str2Int(str);
	}
}

void StrUtil::ConvertUtf8ToGBK( const char* strUtf8,std::string& strGBK )
{
#if defined(WIN32) || defined(__WIN32__)
	WCHAR *strSrc;
	TCHAR *szRes;
	//UTF8转化为Unicode
	int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8, -1, NULL, 0);
	strSrc = new WCHAR[len+1];
	MultiByteToWideChar(CP_UTF8, 0, strUtf8, -1, strSrc, len);

	//Unicode转化为GBK
	len = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
	szRes = new TCHAR[len+1];
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, len, NULL, NULL);
	strGBK = szRes;
	//改变xml的encoding属性为GB2312
	size_t pos=strGBK.find("encoding=");
	size_t next_pos1=strGBK.find("\"",pos)+1;
	size_t next_pos2=strGBK.find("\"",next_pos1);
	strGBK.erase(next_pos1,next_pos2-next_pos1);
	strGBK.insert(next_pos1,"GB2312");

	delete []strSrc;
	delete []szRes;
#endif
}

void StrUtil::ConvertGBKToUtf8( const char* strGBK,std::string& strUtf8 )
{
#if defined(WIN32) || defined(__WIN32__)
	WCHAR *strSrc;
	TCHAR *szRes;
	//GBK转化为Unicode
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	strSrc = new WCHAR[len+1];
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, strSrc, len);

	//Unicode转化为Utf8
	len = WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, NULL, 0, NULL, NULL);
	szRes = new TCHAR[len+1];
	WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, szRes, len, NULL, NULL);

	strUtf8 = szRes;
	//改变xml的encoding属性为UTF-8
	size_t pos=strUtf8.find("encoding=");
	size_t next_pos1=strUtf8.find("\"",pos)+1;
	size_t next_pos2=strUtf8.find("\"",next_pos1);
	strUtf8.erase(next_pos1,next_pos2-next_pos1);
	strUtf8.insert(next_pos1,"UTF-8");
	
	delete []strSrc;
	delete []szRes;
#endif
}

void StrUtil::ConvertETR290TypeIDToAlarmID(sCheckParam& parm)
{
	if (parm.DVBType == DVBC)
	{
		if (parm.TypeID=="1.1")
		{
			parm.TypeID="2";
		}
		else if (parm.TypeID=="1.2")
		{
			parm.TypeID="3";
		}
		else if (parm.TypeID=="1.3")
		{
			parm.TypeID="4";
		} 
		else if (parm.TypeID=="1.4")
		{
			parm.TypeID="5";
		} 
		else if (parm.TypeID=="1.5")
		{
			parm.TypeID="6";
		} 
		else if (parm.TypeID=="1.6")
		{
			parm.TypeID="7";
		} 
		else if (parm.TypeID=="2.1")
		{
			parm.TypeID="8";
		} 
		else if (parm.TypeID=="2.2")
		{
			parm.TypeID="9";
		} 
		else if (parm.TypeID=="2.3a")
		{
			parm.TypeID="10";
		} 
		else if (parm.TypeID=="2.4")
		{
			parm.TypeID="11";
		} 
		else if (parm.TypeID=="2.5")
		{
			parm.TypeID="12";
		} 
		else if (parm.TypeID=="2.6")
		{
			parm.TypeID="13";
		} 
		else
			parm.TypeID="";
	}
	else if (parm.DVBType == CTTB)
	{
		if (parm.TypeID=="1.1")
		{
			parm.TypeID="1";
		}
		else if (parm.TypeID=="1.2")
		{
			parm.TypeID="2";
		}
		else if (parm.TypeID=="1.3")
		{
			parm.TypeID="3";
		} 
		else if (parm.TypeID=="1.4")
		{
			parm.TypeID="4";
		} 
		else if (parm.TypeID=="1.5")
		{
			parm.TypeID="5";
		} 
		else if (parm.TypeID=="1.6")
		{
			parm.TypeID="6";
		} 
		else if (parm.TypeID=="2.1")
		{
			parm.TypeID="7";
		} 
		else if (parm.TypeID=="2.2")
		{
			parm.TypeID="8";
		} 
		else if (parm.TypeID=="2.3a")
		{
			parm.TypeID="9";
		} 
		else if (parm.TypeID=="2.4")
		{
			parm.TypeID="10";
		} 
		else if (parm.TypeID=="2.5")
		{
			parm.TypeID="11";
		} 
		else if (parm.TypeID=="2.6")
		{
			parm.TypeID="12";
		} 
		else
			parm.TypeID="";
	}
	else if(parm.DVBType == DVBS)
	{
		if (parm.TypeID=="1.1")
		{
			parm.TypeID="2";
		}
		else if (parm.TypeID=="1.2")
		{
			parm.TypeID="3";
		}
		else if (parm.TypeID=="1.3")
		{
			parm.TypeID="4";
		} 
		else if (parm.TypeID=="1.4")
		{
			parm.TypeID="5";
		} 
		else if (parm.TypeID=="1.5")
		{
			parm.TypeID="6";
		} 
		else if (parm.TypeID=="1.6")
		{
			parm.TypeID="7";
		} 
		else if (parm.TypeID=="2.1")
		{
			parm.TypeID="8";
		} 
		else if (parm.TypeID=="2.2")
		{
			parm.TypeID="9";
		} 
		else if (parm.TypeID=="2.3a")
		{
			parm.TypeID="10";
		} 
		else if (parm.TypeID=="2.4")
		{
			parm.TypeID="11";
		} 
		else if (parm.TypeID=="2.5")
		{
			parm.TypeID="12";
		} 
		else if (parm.TypeID=="2.6")
		{
			parm.TypeID="13";
		} 
		else
			parm.TypeID="";
	}
	else if (parm.DVBType == THREED)
	{
		if (parm.TypeID=="1.1")
		{
			parm.TypeID="2";
		}
		else if (parm.TypeID=="1.2")
		{
			parm.TypeID="3";
		}
		else if (parm.TypeID=="1.3")
		{
			parm.TypeID="4";
		} 
		else if (parm.TypeID=="1.4")
		{
			parm.TypeID="5";
		} 
		else if (parm.TypeID=="1.5")
		{
			parm.TypeID="6";
		} 
		else if (parm.TypeID=="1.6")
		{
			parm.TypeID="7";
		} 
		else if (parm.TypeID=="2.1")
		{
			parm.TypeID="8";
		} 
		else if (parm.TypeID=="2.2")
		{
			parm.TypeID="9";
		} 
		else if (parm.TypeID=="2.3a")
		{
			parm.TypeID="10";
		} 
		else if (parm.TypeID=="2.4")
		{
			parm.TypeID="11";
		} 
		else if (parm.TypeID=="2.5")
		{
			parm.TypeID="12";
		} 
		else if (parm.TypeID=="2.6")
		{
			parm.TypeID="13";
		} 
		else
			parm.TypeID="";
	}
	return;
}

void StrUtil::ConvertProgramTypeIDToAlarmID(sCheckParam& parm)
{
	if (parm.DVBType==ATV || parm.DVBType==RADIO || parm.DVBType==AM ||parm.DVBType==CTV)	//模拟
	{
		if (parm.TypeID=="0x1")	//视频黑场
		{
			parm.TypeID="18";
		}
		else if (parm.TypeID=="0x2")	//彩条
		{
			parm.TypeID="14";
		}
		else if (parm.TypeID=="0x4")	//画面静止
		{
			parm.TypeID="13";
		}
		else if (parm.TypeID=="0x8")	//无图像
		{
			parm.TypeID="11";
		}
		else if (parm.TypeID=="0xE")	//无载波
		{
			if(parm.DVBType==ATV||parm.DVBType==CTV)
			{
				parm.TypeID="10";
			}
			else if (parm.DVBType==RADIO||parm.DVBType==AM)
			{
				parm.TypeID="23";
			}
		}
		else if (parm.TypeID=="0x10")	//音频静音
		{
			if(parm.DVBType==ATV||parm.DVBType==CTV)
			{
				parm.TypeID="12";
			}
			else if (parm.DVBType==RADIO||parm.DVBType==AM)
			{
				parm.TypeID="24";
			}
		}
		else if (parm.TypeID=="0x5")	//视频黑场+画面静止
		{
			parm.TypeID="19";
		}
		else if (parm.TypeID=="0x6")	//彩条+画面静止
		{
			parm.TypeID="20";
		}
		else
		{
			parm.TypeID="";
		}
	}
	else if (parm.DVBType == DVBC)
	{
		if (parm.TypeID=="0x1")			//视频黑场
		{
			parm.TypeID="32";
		}
		else if (parm.TypeID=="0x2")	//彩条
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="0x4")	//画面静止
		{
			parm.TypeID="31";
		}
		else if (parm.TypeID=="0x8")	//无视频信号
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="0xE")	//无载波
		{
			parm.TypeID="1";
		}
		else if (parm.TypeID=="0x10")	//音频静音
		{
			parm.TypeID="33";
		}
		else if (parm.TypeID=="0x5")	//视频黑场+画面静止 -- 黑场
		{
			parm.TypeID="32";
		}
		else if (parm.TypeID=="0x6")	//彩条+画面静止
		{
			parm.TypeID="";
		}
		else
		{
			parm.TypeID="";
		}
	}
	else if (parm.DVBType == CTTB)
	{
		if (parm.TypeID=="0x1")			//视频黑场
		{
			parm.TypeID="31";
		}
		else if (parm.TypeID=="0x2")	//彩条
		{
			parm.TypeID="32";
		}
		else if (parm.TypeID=="0x4")	//画面静止
		{
			parm.TypeID="30";
		}
		else if (parm.TypeID=="0x8")	//无视频信号
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="0xE")	//无载波
		{
			parm.TypeID="0";
		}
		else if (parm.TypeID=="0x10")	//音频静音
		{
			parm.TypeID="33";
		}
		else if (parm.TypeID=="0x5")	//视频黑场+画面静止 -- 黑场
		{
			parm.TypeID="31";
		}
		else if (parm.TypeID=="0x6")	//彩条+画面静止
		{
			parm.TypeID="32";
		}
		else
		{
			parm.TypeID="";
		}
	}
	else if(parm.DVBType == DVBS)
	{
		if (parm.TypeID=="0x1")			//视频黑场
		{
			parm.TypeID="32";
		}
		else if (parm.TypeID=="0x2")	//彩条
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="0x4")	//画面静止
		{
			parm.TypeID="31";
		}
		else if (parm.TypeID=="0x8")	//无视频信号
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="0xE")	//无载波
		{
			parm.TypeID="1";
		}
		else if (parm.TypeID=="0x10")	//音频静音
		{
			parm.TypeID="33";
		}
		else if (parm.TypeID=="0x5")	//视频黑场+画面静止 -- 黑场
		{
			parm.TypeID="32";
		}
		else if (parm.TypeID=="0x6")	//彩条+画面静止
		{
			parm.TypeID="";
		}
		else
		{
			parm.TypeID="";
		}
	}
	else if (parm.DVBType == THREED)
	{
		if (parm.TypeID=="0x1")			//视频黑场
		{
			parm.TypeID="32";
		}
		else if (parm.TypeID=="0x2")	//彩条
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="0x4")	//画面静止
		{
			parm.TypeID="31";
		}
		else if (parm.TypeID=="0x8")	//无视频信号
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="0xE")	//无载波
		{
			parm.TypeID="1";
		}
		else if (parm.TypeID=="0x10")	//音频静音
		{
			parm.TypeID="33";
		}
		else if (parm.TypeID=="0x5")	//视频黑场+画面静止 -- 黑场
		{
			parm.TypeID="32";
		}
		else if (parm.TypeID=="0x6")	//彩条+画面静止
		{
			parm.TypeID="";
		}
		else
		{
			parm.TypeID="";
		}
	}

	return;
}

void StrUtil::ConvertQualityTypeIDToAlarmID(sCheckParam& parm)
{
	if (parm.DVBType == DVBC)
	{
		if (parm.TypeID=="1")	//Level
		{
			parm.TypeID="41";
		}
		else if (parm.TypeID=="2")	//BER
		{
			parm.TypeID="42";
		}
		else if (parm.TypeID=="3")	//C/N
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="4")	//MER
		{
			parm.TypeID="43";
		}
		else if (parm.TypeID=="5")	//EVM
		{
			parm.TypeID="44";
		}
		else if (parm.TypeID=="6")	//Freq Offset
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="7")	//Symbol Offset
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="8")	//EB/N0
		{
			parm.TypeID="";
		}
		else
		{
			parm.TypeID="";
		}
	}
	else if (parm.DVBType == CTTB)
	{
		if (parm.TypeID=="1")	//Level
		{
			parm.TypeID="40";
		}
		else if (parm.TypeID=="2")	//BER
		{
			parm.TypeID="41";
		}
		else if (parm.TypeID=="3")	//CN
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="4")	//MER
		{
			parm.TypeID="42";
		}
		else if (parm.TypeID=="5")	//EVM
		{
			parm.TypeID="43";
		}
		else if (parm.TypeID=="6")	//Freq Offset
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="7")	//Symbol Offset
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="8")	//EB/N0
		{
			parm.TypeID="";
		}
		else
		{
			parm.TypeID="";
		}
	}
	else if (parm.DVBType == DVBS)
	{
		if (parm.TypeID=="1")	//Level
		{
			parm.TypeID="41";
		}
		else if (parm.TypeID=="2")	//BER
		{
			parm.TypeID="42";
		}
		else if (parm.TypeID=="3")	//C/N
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="4")	//MER
		{
			parm.TypeID="43";
		}
		else if (parm.TypeID=="5")	//EVM
		{
			parm.TypeID="44";
		}
		else if (parm.TypeID=="6")	//Freq Offset
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="7")	//Symbol Offset
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="8")	//EB/N0
		{
			parm.TypeID="";
		}
		else
		{
			parm.TypeID="";
		}
	}
	else if (parm.DVBType == THREED)
	{
		if (parm.TypeID=="1")	//Level
		{
			parm.TypeID="41";
		}
		else if (parm.TypeID=="2")	//BER
		{
			parm.TypeID="42";
		}
		else if (parm.TypeID=="3")	//C/N
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="4")	//MER
		{
			parm.TypeID="43";
		}
		else if (parm.TypeID=="5")	//EVM
		{
			parm.TypeID="44";
		}
		else if (parm.TypeID=="6")	//Freq Offset
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="7")	//Symbol Offset
		{
			parm.TypeID="";
		}
		else if (parm.TypeID=="8")	//EB/N0
		{
			parm.TypeID="";
		}
		else
		{
			parm.TypeID="";
		}
	}
	return;
}

void StrUtil::ConvertEnvTypeIDToAlarmID(sCheckParam& parm)
{
	return;
}

void StrUtil::ConvertEquipTypeIn(string& type)
{
	if(type=="1")
	{
		type="31";
	}
	else if(type=="2")
	{
		type="32";
	}
	else if(type=="3")
	{
		type="33";
	}
	else if(type=="4")
	{
		type="34";
	}
	else if(type=="5")
	{
		type="35";
	}
	else if(type=="6")
	{
		type="36";
	}
	return;
}

int StrUtil::GetStrPixelLength(char* hz, int font)
{
	int i = 0;
	int iDistance = 0;
	char ascBuf[3] = {0, 0, 0};
	int Width = 0;
	unsigned const char* pixelArr;
	unsigned char AscOdd, AscEven;

	char tmpStrArr[128] = {0};
	memcpy(tmpStrArr, hz, strlen(hz));


	if(font == 40)
		pixelArr = ascii_hei_40_width_data;
	else
		pixelArr = ascii_hei_20_width_data;
	for (i = 0; (i < 128) && ('\0' != hz[i]); i++)
	{
		//printf("a       hz[i] =%x\n",hz[i] );
		if(hz[i] & 0x80)
		{
			//	printf("hz[i] =%x\n",hz[i] );
			memcpy(ascBuf, &(hz[i]), 2);
			i++;
		}
		else
		{
			ascBuf[0] = hz[i];
			ascBuf[1] = '\0';
		}
		Width=0;
		{
			/*以下代码取字模*/
			AscOdd = *ascBuf;
			if(AscOdd & 0x80)
			{

				AscEven = *(ascBuf + 1);
				if(AscEven & 0x80)
				{

					//   asc = (unsigned char*)(pFontAttr->gFonthzData) + rec;
					//	printf("asc=%d\n",asc);
					Width = font;
				}
				else
				{
					//AscEven = 0xa0;
					//asc = NULL;
					if (AscEven == /*中文空格*/0xa0)
					{
						Width = font;
					}
				}
			}
			else
			{

				if(AscOdd > 0x20)/*可见字符 0X20 asc空格*/
				{
					//asc = (unsigned char *)(pFontAttr->gAsciiData + rec);
					Width =  (int)pixelArr[AscOdd];
				}
				else if(0X9 == AscOdd)/*tab键处理*/
				{
					//asc = NULL;
					Width = (int)pixelArr[AscOdd];
				}
				else if(0X6 == AscOdd)/*for jinshi*/
				{
					//asc = NULL;
					Width = (int)pixelArr[AscOdd];
				}
				else
				{
					AscOdd = ' ';
					//asc = NULL;

					if (AscOdd == /*asc空格*/0x20)
					{
						Width = (int)pixelArr[AscOdd];
					}
				}
			}
			iDistance +=  Width;
		}

	}

	memset(hz, 0, 128);
	int addNum = 16 - iDistance%16;
	
	if(addNum > 2)
	{
		addNum -= 2;
	}

	for(int i = 0; i < addNum; i++)
		hz[i] = 0x6;

	memcpy(hz+addNum, tmpStrArr, strlen(tmpStrArr));

	return iDistance + 16 - iDistance%16;
}



#if 0
{
	int asciiNo,len = 0;
	const char* index = str;
	char tmpStrArr[128] = {0};
	memcpy(tmpStrArr, str, strlen(str));

	unsigned const char* pixelArr;
	char ch ;
	if(font == 40)
		pixelArr = ascii_hei_40_width_data;
	else
		pixelArr = ascii_hei_20_width_data;
	while(*index)
	{
		ch = *index;
		asciiNo = (int)(ch);
		if((*index)&0x80)
		{
			//是汉字
			index++;//知道是汉字的话跳过一个字节检测
			len+= font;
		}
		else
		{
			len += (int)pixelArr[asciiNo];
		}
		index++;
	}

	memset(str, 0, 128);
	int addNum = 16 - len%16;
	for(int i = 0; i < addNum; i++)
		str[i] = 0x6;
	memcpy(str+addNum, tmpStrArr, strlen(tmpStrArr));
	return len + addNum;
}
#endif