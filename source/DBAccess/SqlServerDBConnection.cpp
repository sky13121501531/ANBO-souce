#include "SqlServerDBConnection.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/SqlServerStrUtil.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/TypeDef.h"
SqlServerDBConnection::SqlServerDBConnection(void)
{
	DBCon=new CADODatabase;
}

SqlServerDBConnection::~SqlServerDBConnection(void)
{
	if (DBCon!=NULL)
	{
		delete DBCon;
		DBCon=NULL;
	}
}

bool SqlServerDBConnection::InitConnect( void )
{
	bool ret = false;
	int count = 0;

	string path = "Provider=SQLOLEDB.1;Persist Security Info=False;User ID=" + PROPMANAGER::instance()->GetDbUsername()+";Password="+PROPMANAGER::instance()->GetDbPwd()
		+ ";Initial Catalog=" + PROPMANAGER::instance()->GetDbName() + ";Data Source=" + PROPMANAGER::instance()->GetDbIp();//连接sqlserver字符串，oledb方式

	try
	{
		while(!DBCon->IsOpen() && count < 3)
		{
			DBCon->Open(_bstr_t(path.c_str()));//连接数据库
			count ++;
		}
		if(DBCon->IsOpen())
			ret = true;
	}
	catch (...) 
	{
		APPLOG::instance()->WriteLog(OTHER, LOG_EVENT_ERROR, "打开数据库出错");
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::DisConnect( void )
{
	if (DBCon->IsOpen())
	{
		 DBCon->Close();
	}
	return true;
}

bool SqlServerDBConnection::AddXmlTask( eDVBType type,std::string xml )
{
	int id=type;
	bool ret=false;
	if(QueryISTask(type,xml))
	{
		return true;
	}
	std::string sql="insert into task(dvbtype,taskinfo) values('"+StrUtil::Int2Str(id)+"','"+xml+"')";
	try
	{
		ret=DBCon->Execute(sql.c_str());//执行添加任务sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"向数据库中添加任务出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::DeleteTask( eDVBType type,std::string xml )
{
	int id=type;
	bool ret=false;
	std::string sql="";
	if(type==UNKNOWN)
	{
		sql="delete from task where dvbtype='"+StrUtil::Int2Str(id)+"'";
	}
	else
	{
		// sql="delete from task where dvbtype='"+StrUtil::Int2Str(id)+"' and taskinfo='"+xml+"'";
		sql="delete from task where dvbtype='"+StrUtil::Int2Str(id)+"' and taskinfo like '%"+xml+"%'";
	}
	try
	{
		ret=DBCon->Execute(sql.c_str());//执行删除任务sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"从数据库中删除任务出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}
bool SqlServerDBConnection::QueryISTask(eDVBType type,std::string xml)
{
	int id=type;
	bool ret=false;
	std::string sql="select taskinfo from task where dvbtype='"+StrUtil::Int2Str(id)+"'";
	try
	{

		CADORecordset record;
		if (record.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行查询任务sql
		{
			std::string str;
			while(!record.IsEof())
			{
				record.GetFieldValue("taskinfo",str);//获得任务xml
				if(str.compare(xml.c_str()) == 0)
				{
					return true;
				}
				// vecXml.push_back(str);
				record.MoveNext();//移动到下一条记录
			}
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryTask( eDVBType type,std::vector<std::string>& vecXml )
{
	int id=type;
	bool ret=false;
	std::string sql="select taskinfo from task where dvbtype='"+StrUtil::Int2Str(id)+"'";
	try
	{

		CADORecordset record;
		if (record.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行查询任务sql
		{
			std::string str;
			while(!record.IsEof())
			{
				record.GetFieldValue("taskinfo",str);//获得任务xml
				vecXml.push_back(str);
				record.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryTask( std::vector<std::string>& vecXml )
{
	bool ret=false;
	std::string sql="select taskinfo from task";
	try
	{

		CADORecordset record;
		if (record.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行查询任务sql
		{
			std::string str;
			while(!record.IsEof())
			{
				record.GetFieldValue("taskinfo",str);//获得任务xml
				vecXml.push_back(str);
				record.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::AddChannel( eDVBType type,std::string channelid,std::string xml )
{
	int id=type;
	bool ret=false;
	//std::string delsql="delete from channel where dvbtype='"+StrUtil::Int2Str(id)+"' and channelid='"+channelid+"'";//删除sql
	std::string addsql="insert into channel(dvbtype,channelid,channelinfo) values('"+StrUtil::Int2Str(id)+"','"+channelid+"','"+xml+"')";
	try
	{
		ret=DBCon->Execute(addsql.c_str());//执行删除频道，再执行添加频道信息
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"向数据库中添加频道扫描结果出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::DeleteChannel( eDVBType type,std::string channelid)
{
	int id=type;
	bool ret=false;
	std::string sql="delete from channel where dvbtype='"+StrUtil::Int2Str(id)+"' and channelid='"+channelid+"'";
	try
	{
		ret=DBCon->Execute(sql.c_str());//执行删除频道信息sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"从数据库中删除频道扫描结果出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::DeleteChannelByType( eDVBType type)
{
	int id=type;
	bool ret=false;
	std::string sql="delete from channel where dvbtype='"+StrUtil::Int2Str(id)+"' ";
	try
	{
		ret=DBCon->Execute(sql.c_str());//执行删除频道信息sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"从数据库中删除频道扫描结果出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryChannel( eDVBType type,std::string channelid,std::string& xml )
{
	int id=type;
	bool ret=false;
	std::string sql="select channelinfo from channel where dvbtype='"+StrUtil::Int2Str(id)+"' and channelid='"+channelid+"'";
	try
	{

		CADORecordset record;
		if (record.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行查询频道sql
		{
			while(!record.IsEof())
			{
				record.GetFieldValue("channelinfo",xml);//获得频道信息的xml
				record.MoveNext();//移动下一条记录到
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryAllChannel( std::vector<std::string>&vecXml )
{
	bool ret=false;
	std::string sql="select channelinfo from channel";
	try
	{
		CADORecordset record;
		if (record.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行频道查询sql
		{
			while(!record.IsEof())
			{
				string xml;
				record.GetFieldValue("channelinfo",xml);//获得频道信息xml
				vecXml.push_back(xml);
				record.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryTypeChannel( eDVBType type,std::vector<std::string>&vecXml )
{
	bool ret=false;
	int id=type;
	std::string sql="select channelinfo from channel where dvbtype='"+StrUtil::Int2Str(id)+"'";
	try
	{
		CADORecordset record;
		if (record.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行频道查询sql
		{
			while(!record.IsEof())
			{
				string xml;
				record.GetFieldValue("channelinfo",xml);//获得频道信息xml
				vecXml.push_back(xml);
				record.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::AddRecord( sRecordInfo record )
{
	bool ret=false;
	int type=record.dvbtype;
	string sql="insert into record(dvbtype,channelid,taskid,filename,starttime,endtime,expiredays) values('";
	sql+=StrUtil::Int2Str(type)+"','";
	sql+=record.channelID+"','";
	sql+=record.taskid+"','";
	sql+=record.filename+"','";
	sql+=record.starttime+"','";
	sql+=record.endtime+"','";
	sql+=record.expiredays+"')";
	try
	{
		ret=DBCon->Execute(sql.c_str());//执行添加录像信息xml
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryRecordByTaskid( eDVBType type,std::string taskId,std::vector<sRecordInfo>& vecRecordInfo,string StartTime,string EndTime)
{
	int id=type;
	bool ret=false;
	string sql=string("select * from record where dvbtype='") + StrUtil::Int2Str(id) + string("' and taskid='") + taskId + string("'");

	if (StartTime != "" && EndTime != "")
	{
		sql += string(" and StartTime <='") + EndTime + string(" '");
		sql += string(" and EndTime >='") + StartTime + string("'");
	}
	else if (StartTime != "" && EndTime == "")
	{
		sql += string(" and StartTime <='") + EndTime + string("'");
	}
	else if (StartTime == "" && EndTime != "")
	{
		sql += string(" and EndTime >='") + StartTime + string("'");
	}
	sql+=" order by starttime asc";

	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行录像文件查询sql
		{
			while (!recordset.IsEof())
			{
				sRecordInfo record;
				recordset.GetFieldValue("filename",record.filename);//获得录像文件名
				recordset.GetFieldValue("starttime",record.starttime);//录像文件开始时间
				recordset.GetFieldValue("endtime",record.endtime);//录像文件结束时间
				recordset.GetFieldValue("ChannelID",record.channelID);//新安播要求获取channelId
				vecRecordInfo.push_back(record);
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
		
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryRecordByChannelID( eDVBType type,std::string taskID,std::string channelID,std::vector<sRecordInfo>& vecRecordInfo,string StartTime,string EndTime)
{
	int id=type;
	bool ret=false;
	string sql=string("select * from record where dvbtype='") + StrUtil::Int2Str(id)+ string("' and channelid='") + channelID+"' and taskid='"+taskID+"'";

	if (StartTime != "" && EndTime != "")
	{
		sql += string(" and StartTime <='") + EndTime + string("'");
		sql += string(" and EndTime >='") + StartTime + string("'");
	}
	else if (StartTime != "" && EndTime == "")
	{
		sql += string(" and StartTime <='") + EndTime + string("'");
	}
	else if (StartTime == "" && EndTime != "")
	{
		sql += string(" and EndTime >='") + StartTime + string("'");
	}
	sql+=" order by starttime asc";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行录像文件查询sql
		{
			while (!recordset.IsEof())
			{
				sRecordInfo record;
				recordset.GetFieldValue("filename",record.filename);//获得录像文件名
				recordset.GetFieldValue("starttime",record.starttime);//录像文件开始时间
				recordset.GetFieldValue("endtime",record.endtime);//录像文件结束时间
				recordset.GetFieldValue("ChannelID",record.channelID);//新安播要求获取channelId
				vecRecordInfo.push_back(record);
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
		
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryRecordByDeviceID(eDVBType type,std::string taskID,std::string DeviceID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime,std::string EndTime)
{
	int id=type;
	bool ret=false;
	string sql=string("select * from record where dvbtype='") + StrUtil::Int2Str(id);
	sql += string(string("' and taskid='")+taskID);
	
	if(StrUtil::Str2Int(DeviceID) <= 0) return false;
	
	sql += string(string("' and filename like '%/")+ DeviceID + "[_]%'");

	if (StartTime != "" && EndTime != "")
	{
		sql += string(" and StartTime <='") + EndTime + string("'");
		sql += string(" and EndTime >='") + StartTime + string("'");
	}
	else if (StartTime != "" && EndTime == "")
	{
		sql += string(" and StartTime <='") + EndTime + string("'");
	}
	else if (StartTime == "" && EndTime != "")
	{
		sql += string(" and EndTime >='") + StartTime + string("'");
	}
	sql+=" order by starttime asc";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行录像文件查询sql
		{
			while (!recordset.IsEof())
			{
				sRecordInfo record;
				recordset.GetFieldValue("filename",record.filename);//获得录像文件名
				recordset.GetFieldValue("starttime",record.starttime);//录像文件开始时间
				recordset.GetFieldValue("endtime",record.endtime);//录像文件结束时间
				recordset.GetFieldValue("ChannelID",record.channelID);//新安播要求获取channelId
				vecRecordInfo.push_back(record);
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}

	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::DeleteRecord( std::string filename )
{
	bool ret=false;
	string sql="delete from record where filename='"+filename+"'";
	try
	{
		ret=DBCon->Execute(sql.c_str());//执行删除录像sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryRecordByDays( unsigned int days,std::vector<std::string>& vecFilename )
{
	return true;		//该函数有问题，暂时忽略//gxd 2011-03-31
	/************************************************************************/

	bool ret=false;
	string sql="select filename from record where expiredays='"+StrUtil::Int2Str(days)+"'";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行录像文件查询sql
		{
			while (!recordset.IsEof())
			{
				string file;
				recordset.GetFieldValue("filename",file);//获得录像文件名
				vecFilename.push_back(file);
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
		
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryRecordTopInPath(unsigned int count, std::string recPath, std::vector<std::string>& vecFilename)
{
	bool ret = false;

	string sql = "select top "+ StrUtil::Int2Str(count) +" * from record where filename like \'%"+recPath
		+"%\' order by expiredays,starttime";

	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行录像文件查询sql
		{
			while (!recordset.IsEof())
			{
				string file;
				recordset.GetFieldValue("filename",file);//获得录像文件名
				vecFilename.push_back(file);
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryRecordTop(unsigned int count, std::vector<std::string>& vecFilename )
{
	bool ret=false;
	string sql = "select top "+ StrUtil::Int2Str(count) +" * from record order by expiredays,starttime";

	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行录像文件查询sql
		{
			while (!recordset.IsEof())
			{
				string file;
				recordset.GetFieldValue("filename",file);//获得录像文件名
				vecFilename.push_back(file);
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}
bool SqlServerDBConnection::DeleteSpecDataExpire()//删除过期频谱数据 默认30天
{
	bool ret=true;
	string sql="delete from Spec where dateadd(day,30,CheckTime)<='";
	sql = sql + TimeUtil::GetCurDateTime() + "'";
	try
	{
		ret=DBCon->Execute(sql.c_str());//执行删除录像sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除频谱数据出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}
bool SqlServerDBConnection::DeleteQualityDataExpire()//删除过期指标数据 默认30天
{
	bool ret=true;
	string sql="delete from Quality where dateadd(day,30,CheckTime)<='";
	sql = sql + TimeUtil::GetCurDateTime() + "'";
	try
	{
		ret=DBCon->Execute(sql.c_str());//执行删除录像sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除指标数据出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryRecordExpire( std::vector<std::string>& vecFilename )
{
	bool ret=false;
	string sql = "select  * from record where dateadd(day,expiredays,starttime)<='";
	sql = sql + TimeUtil::GetCurDateTime() + "'";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行录像文件查询sql
		{
			while (!recordset.IsEof())
			{
				string file;
				recordset.GetFieldValue("filename",file);//获得录像文件名
				vecFilename.push_back(file);
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::IsConnected( void )
{
	return DBCon->IsOpen();
}

bool SqlServerDBConnection::QueryChannelID( eDVBType type,std::vector<std::string>&vecChannelID )
{
	bool ret=false;
	int id=type;
	std::string sql="select channelid from channel where dvbtype='"+StrUtil::Int2Str(id)+"'";
	try
	{

		CADORecordset record;
		if (record.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行查询sql
		{
			while(!record.IsEof())
			{
				string channelid;
				record.GetFieldValue("channelid",channelid);//获得频道id
				vecChannelID.push_back(channelid);
				record.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::UpdateAlarmParam(sAlarmParam& alarmparam)
{
	bool ret = false;

	string insertsql = "insert into AlarmParam(DVBType,DeviceID,AlarmType,TypeID,TypeDesc,STD,Freq,SymbolRate,";

	insertsql += "ChannelID,Duration,Num,TimeInterval,DownThreshold,UpThreshold,Switch) values('";
	insertsql += StrUtil::Int2Str(alarmparam.DVBType)+string("','")+alarmparam.DeviceID+string("','")+StrUtil::Int2Str(alarmparam.AlarmType)+string("','");
	insertsql += alarmparam.TypeID+string("','")+alarmparam.TypeDesc+string("','")+alarmparam.STD+string("','")+alarmparam.Freq+string("','");
	insertsql += alarmparam.SymbolRate+string("','")+alarmparam.ChannelID+string("','")+alarmparam.Duration+string("','");
	insertsql += alarmparam.Num+string("','")+alarmparam.TimeInterval+string("','")+alarmparam.DownThreshold+string("','");
	insertsql += alarmparam.UpThreshold+string("','")+alarmparam.Switch+string("')");

	string selsql = string("select * from AlarmParam  ") ;
	string wheresql=string("where DVBType = '") +  StrUtil::Int2Str(alarmparam.DVBType)+  string("' and DeviceID = '") + alarmparam.DeviceID+
		string("' and AlarmType = '") + StrUtil::Int2Str(alarmparam.AlarmType) + string("' and TypeID = '") + alarmparam.TypeID+"'";

	if (alarmparam.AlarmType == ALARM_FREQ ||alarmparam.AlarmType == ALARM_PROGRAM)
	{
		wheresql += string((" and Freq = '")+alarmparam.Freq + string("' and SymbolRate = '") + alarmparam.SymbolRate+string("'  and ChannelID = '")+alarmparam.ChannelID)+"'";
	}
	else if(alarmparam.AlarmType == ALARM_TR101_290)
	{
		wheresql += string((" and SymbolRate = '") + alarmparam.SymbolRate+string("'  and ChannelID = '")+alarmparam.ChannelID)+"'";
	}
	else if (alarmparam.AlarmType == ALARM_ENVIRONMENT)
	{
		;
	}
	else
	{
		return false;
	}
	selsql+=wheresql;
	string updatesql="";
	if(alarmparam.Switch!="")
	{
		updatesql="update AlarmParam set Switch = '"+alarmparam.Switch+"' ";
	}
	if(alarmparam.Duration!=""||alarmparam.TimeInterval!=""||alarmparam.Num!=""||alarmparam.DownThreshold!=""||alarmparam.UpThreshold!="")
	{
		updatesql="update AlarmParam set Duration = '"+alarmparam.Duration+"' ,Num = '"+alarmparam.Num+"' ,TimeInterval = '"+alarmparam.TimeInterval+"' ,DownThreshold = '"+alarmparam.DownThreshold+"' ,UpThreshold = '"+alarmparam.UpThreshold+"' " ;
		if(alarmparam.Switch!="")
		{
			updatesql+=",Switch = '"+alarmparam.Switch+"' ";
		}
	}
	updatesql+=wheresql;
	try
	{
		CADORecordset record;
		int rowcount=0;
		if (record.Open(DBCon->GetActiveConnection(),selsql.c_str()))//执行查询任务sql
		{
			rowcount=record.GetRecordCount();
		}
		if(rowcount==0)
		{
			ret=DBCon->Execute(insertsql.c_str());//添加报警参数信息
		}
		else
		{
			ret=DBCon->Execute(updatesql.c_str());
		}

	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新报警参数出错！",LOG_OUTPUT_FILE);
		return false;

	}
	return ret;
}
bool SqlServerDBConnection::QueryAlarmParam(eDVBType type,std::vector<sAlarmParam>& AlarmParamVec)
{
	bool ret=false;
	std::string sql="select * from AlarmParam where DVBType = ";
	sql += StrUtil::Int2Str(type);
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行报警参数查询sql
		{
			while (!recordset.IsEof())
			{
				sAlarmParam param;
				param.DVBType=type;
				string alarmtype;
				//获得报警参数信息
				recordset.GetFieldValue("AlarmType",alarmtype);
				switch(StrUtil::Str2Int(alarmtype))
				{
				case 0:
					param.AlarmType=ALARM_NONE;
					break;
				case 1:
					param.AlarmType=ALARM_ENVIRONMENT;
					break;
				case 2:
					param.AlarmType=ALARM_FREQ;
					break;
				case 3:
					param.AlarmType=ALARM_PROGRAM;
					break;
				case 4:
					param.AlarmType=ALARM_TR101_290;
					break;
				default:
					param.AlarmType=ALARM_NONE;
					break;
				}
				recordset.GetFieldValue("TypeID",param.TypeID);
				recordset.GetFieldValue("TypeDesc",param.TypeDesc);
				recordset.GetFieldValue("STD",param.STD);
				recordset.GetFieldValue("Freq",param.Freq);
				recordset.GetFieldValue("SymbolRate",param.SymbolRate);
				recordset.GetFieldValue("DeviceID",param.DeviceID);
				recordset.GetFieldValue("ChannelID",param.ChannelID);
				recordset.GetFieldValue("Duration",param.Duration);
				recordset.GetFieldValue("Num",param.Num);
				recordset.GetFieldValue("TimeInterval",param.TimeInterval);
				recordset.GetFieldValue("DownThreshold",param.DownThreshold);
				recordset.GetFieldValue("UpThreshold",param.UpThreshold);
				recordset.GetFieldValue("Switch",param.Switch);

				AlarmParamVec.push_back(param);
				recordset.MoveNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryAlarmParam(std::vector<sAlarmParam>& AlarmParamVec)
{
	bool ret=false;
	std::string sql="select * from AlarmParam";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行报警参数查询sql
		{
			while (!recordset.IsEof())
			{
				sAlarmParam param;
				string dvbtype;
				recordset.GetFieldValue("dvbtype",dvbtype);
				switch(StrUtil::Str2Int(dvbtype))
				{
				case 1:
					param.DVBType=CTTB;
					break;
				case 2:
					param.DVBType=CMMB;
					break;
				case 3:
					param.DVBType=DVBC;
					break;
				case 4:
					param.DVBType=RADIO;
					break;
				case 5:
					param.DVBType=ATV;
					break;
				case 6:
					param.DVBType=AM;
					break;
				case 7:
					param.DVBType=DVBS;
					break;
				case 8:
					param.DVBType=CTV;
					break;
				default:
					param.DVBType=UNKNOWN;
					break;
				}
				string alarmtype;
				//获得报警参数信息
				recordset.GetFieldValue("AlarmType",alarmtype);
				switch(StrUtil::Str2Int(alarmtype))
				{
				case 0:
					param.AlarmType=ALARM_NONE;
					break;
				case 1:
					param.AlarmType=ALARM_ENVIRONMENT;
					break;
				case 2:
					param.AlarmType=ALARM_FREQ;
					break;
				case 3:
					param.AlarmType=ALARM_PROGRAM;
					break;
				case 4:
					param.AlarmType=ALARM_TR101_290;
					break;
				default:
					param.AlarmType=ALARM_NONE;
					break;
				}
				recordset.GetFieldValue("TypeID",param.TypeID);
				recordset.GetFieldValue("TypeDesc",param.TypeDesc);
				recordset.GetFieldValue("STD",param.STD);
				recordset.GetFieldValue("Freq",param.Freq);
				recordset.GetFieldValue("SymbolRate",param.SymbolRate);
				recordset.GetFieldValue("DeviceID",param.DeviceID);
				recordset.GetFieldValue("ChannelID",param.ChannelID);
				recordset.GetFieldValue("Duration",param.Duration);
				recordset.GetFieldValue("Num",param.Num);
				recordset.GetFieldValue("TimeInterval",param.TimeInterval);
				recordset.GetFieldValue("DownThreshold",param.DownThreshold);
				recordset.GetFieldValue("UpThreshold",param.UpThreshold);
				recordset.GetFieldValue("Switch",param.Switch);

				AlarmParamVec.push_back(param);
				recordset.MoveNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::AddRunPlanParam( sRunPlanParam runPlanParam )
{
	bool ret=false;
	std::string sql="insert into runplanparam(dvbtype,channelid,type,dayofweek,mmonth,dayofmonth,starttime,endtime,validstartdatetime,validenddatetime,startdatetime,enddatetime) values('"+StrUtil::Int2Str(runPlanParam.dvbtype)+"','";
	sql+=runPlanParam.ChannelID+"','";
	sql+=StrUtil::Int2Str(runPlanParam.Type)+"','";
	sql+=runPlanParam.DayOfWeek+"','";
	sql+=runPlanParam.Month+"','";
	sql+=runPlanParam.DayOfMonth+"','";
	sql+=runPlanParam.StartTime+"','";
	sql+=runPlanParam.EndTime+"','";
	sql+=runPlanParam.ValidStartDateTime+"','";
	sql+=runPlanParam.ValidEndDateTime+"','";
	sql+=runPlanParam.StartDateTime+"','";
	sql+=runPlanParam.EndDateTime+"')";
	try
	{
		ret=DBCon->Execute(sql.c_str());//添加运行图参数
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加运行图参数出错！",LOG_OUTPUT_FILE);
		return false;

	}
	return ret;
}

bool SqlServerDBConnection::QueryRunPlanParam( eDVBType type,std::vector<sRunPlanParam>& runPlanParam )
{
	bool ret=false;
	std::string sql="select * from runplanparam where dvbtype='"+StrUtil::Int2Str(type)+"' order by dvbtype,channelid";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询运行图参数信息
		{
			while (!recordset.IsEof())
			{
				//生成运行图参数
				sRunPlanParam param;
				param.dvbtype=type;
				recordset.GetFieldValue("channelid",param.ChannelID);
				recordset.GetFieldValue("type",param.Type);
				recordset.GetFieldValue("dayofweek",param.DayOfWeek);
				recordset.GetFieldValue("mmonth",param.Month);
				recordset.GetFieldValue("dayofmonth",param.DayOfMonth);
				recordset.GetFieldValue("starttime",param.StartTime);
				recordset.GetFieldValue("endtime",param.EndTime);
				recordset.GetFieldValue("validstartdatetime",param.ValidStartDateTime);
				recordset.GetFieldValue("validenddatetime",param.ValidEndDateTime);
				recordset.GetFieldValue("startdatetime",param.StartDateTime);
				recordset.GetFieldValue("enddatetime",param.EndDateTime);
				runPlanParam.push_back(param);
				recordset.MoveNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询运行图参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}

bool SqlServerDBConnection::QueryAllRunPlanParam( std::vector<sRunPlanParam>& runPlanParam )
{
	bool ret=false;
	std::string sql="select * from runplanparam order by dvbtype,channelid";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询运行图参数
		{
			while (!recordset.IsEof())
			{
				sRunPlanParam param;
				string dvbtype;
				recordset.GetFieldValue("dvbtype",dvbtype);
				switch(StrUtil::Str2Int(dvbtype))
				{
				case 1:
					param.dvbtype=CTTB;
					break;
				case 2:
					param.dvbtype=CMMB;
					break;
				case 3:
					param.dvbtype=DVBC;
					break;
				case 4:
					param.dvbtype=RADIO;
					break;
				case 5:
					param.dvbtype=ATV;
					break;
				case 6:
					param.dvbtype=AM;
					break;
				case 7:
					param.dvbtype=DVBS;
					break;
				case 8:
					param.dvbtype=CTV;
					break;
				default:
					param.dvbtype=UNKNOWN;
					break;
				}
				//获得运行图参数信息
				recordset.GetFieldValue("channelid",param.ChannelID);
				recordset.GetFieldValue("type",param.Type);
				recordset.GetFieldValue("dayofweek",param.DayOfWeek);
				recordset.GetFieldValue("mmonth",param.Month);
				recordset.GetFieldValue("dayofmonth",param.DayOfMonth);
				recordset.GetFieldValue("starttime",param.StartTime);
				recordset.GetFieldValue("endtime",param.EndTime);
				recordset.GetFieldValue("validstartdatetime",param.ValidStartDateTime);
				recordset.GetFieldValue("validenddatetime",param.ValidEndDateTime);
				recordset.GetFieldValue("startdatetime",param.StartDateTime);
				recordset.GetFieldValue("enddatetime",param.EndDateTime);
				runPlanParam.push_back(param);
				recordset.MoveNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询运行图参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}

/**	channelid不会用到了
*/
bool SqlServerDBConnection::DeleteRunPlanParam(eDVBType type, std::string channelid )
{
	bool ret=false;
	std::string sql="delete from runplanparam where dvbtype='"+StrUtil::Int2Str(type)+"'";
	try
	{
		ret=DBCon->Execute(sql.c_str());//删除运行图参数
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除运行图参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::UpdateRunPlanParam(eDVBType dvbtype,std::string channelid,std::vector<sRunPlanParam> runPlanParam )
{
	if (runPlanParam.size()>0)
	     DeleteRunPlanParam(dvbtype,channelid);//删除运行图参数

	for (size_t i=0;i!=runPlanParam.size();++i)
	{
		AddRunPlanParam(runPlanParam[i]);//添加运行图参数
	}
	return true;
}

bool SqlServerDBConnection::AddQualityInfo( eDVBType dvbtype,std::string freq,std::string taskid,eQualityInfo quality ,std::string checktime)
{
	bool ret=false;
	string sql="insert into quality(dvbtype,freq,taskid,type,des,valu,checktime) values('"+StrUtil::Int2Str(dvbtype)+"','";
	sql+=freq+"','";
	sql+=taskid+"','";
	sql+=quality.type+"','";
	sql+=quality.desc+"','";
	sql+=quality.valu+"','";
	sql+=checktime+"')";
	try
	{
		ret=DBCon->Execute(sql.c_str());//添加指标信息
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加指标信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryQualityInfo( eDVBType dvbtype,std::string freq,std::string taskid,std::vector<eQualityInfo>& vecQuality,std::string startdatetime,std::string enddatetime )
{
	bool ret=false;
	string sql;
	if(freq!="")
	{
		sql="select * from quality where dvbtype='"+StrUtil::Int2Str(dvbtype)+"' and freq='"+freq;
	}
	else
	{
		sql="select * from quality where dvbtype='"+StrUtil::Int2Str(dvbtype);
	}
	sql+="' and taskid='";
	sql+=taskid+"' and checktime>='";
	sql+=startdatetime+"' and checktime<='";
	sql+=enddatetime+"'";
	try
	{
		CADORecordset recordset;
		{
			if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询指标信息
			{
				while(!recordset.IsEof())
				{
					//获得指标信息
					eQualityInfo qulity;
					recordset.GetFieldValue("type",qulity.type);
					recordset.GetFieldValue("des",qulity.desc);
					recordset.GetFieldValue("valu",qulity.valu);
					recordset.GetFieldValue("checktime",qulity.checktime);
					recordset.GetFieldValue("freq",qulity.freq);
					recordset.MoveNext();
					vecQuality.push_back(qulity);
				}
				ret=true;
			}

		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询指标出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::GetMsgID( std::string &MsgID )
{
	bool ret=false;
	string sql="select msgid from MsgID ";
	
	try
	{
		CADORecordset recordset;
		{
			if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询指标信息
			{
				while(!recordset.IsEof())
				{
					recordset.GetFieldValue("msgid",MsgID);//获得msgid
					recordset.MoveNext();
				}
				ret=true;
			}
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询MsgID出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::UpdateMsgID( std::string MsgID )
{
	bool ret=false;
	string sql="update Msgid set msgid='"+MsgID+"'";
	try
	{
		ret=DBCon->Execute(sql.c_str());//更新MsgID信息
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新MsgID信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryCompensationValu( std::vector<sQualityCompensation>& qualityCmpVec)
{
	bool ret=false;
	string sql="select * from qualitycompensation";
	try
	{
		CADORecordset recordset;
		{
			if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询指标信息
			{
				while(!recordset.IsEof())
				{
					sQualityCompensation quality;
					string dvbtype;
					recordset.GetFieldValue("dvbtype",dvbtype);
					switch(StrUtil::Str2Int(dvbtype))
					{
					case 1:
						quality.dvbtype=CTTB;
						break;
					case 2:
						quality.dvbtype=CMMB;
						break;
					case 3:
						quality.dvbtype=DVBC;
						break;
					case 4:
						quality.dvbtype=RADIO;
						break;
					case 5:
						quality.dvbtype=ATV;
						break;
					case 6:
						quality.dvbtype=AM;
						break;
					case 7:
						quality.dvbtype=DVBS;
						break;
					case 8:
						quality.dvbtype=CTV;
						break;
					default:
						quality.dvbtype=UNKNOWN;
						break;
					}
					recordset.GetFieldValue("deviceid",quality.deviceid);
					recordset.GetFieldValue("type",quality.type);
					recordset.GetFieldValue("valu",quality.valu);
					qualityCmpVec.push_back(quality);
					recordset.MoveNext();
				}
				ret=true;
			}	
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询指标补偿值出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::UpdateCompensationValu(sQualityCompensation& qualityCmp)
{
	return DelCompensationValu(qualityCmp)&&AddCompensationValu(qualityCmp);
}

bool SqlServerDBConnection::AddCompensationValu(sQualityCompensation& qualityCmp)
{
    bool ret=false;
	string sql="insert into qualitycompensation(dvbtype,deviceid,type,valu) values('";
	sql+=StrUtil::Int2Str(qualityCmp.dvbtype)+"','";
	sql+=qualityCmp.deviceid+"','";
	sql+=qualityCmp.type+"','";
	sql+=qualityCmp.valu+"')";
	try
	{
		ret=DBCon->Execute(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加指标补偿信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::DelCompensationValu(sQualityCompensation& qualityCmp)
{
	bool ret=false;
	string sql="delete from qualitycompensation where dvbtype='";
	sql+=string(StrUtil::Int2Str(qualityCmp.dvbtype)+"' and deviceid='");
	sql+=string(qualityCmp.deviceid+"' and type='");
	sql+=string(qualityCmp.type+"'");
	try
	{
		ret=DBCon->Execute(sql.c_str());
	}
	catch (...)
	{ 
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除指标补偿信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::AddAlarmInfo( sCheckParam& alarminfo,string mode )
{
	bool ret=false;
	string sql="insert into alarminfo(dvbtype,alarmtype,std,freq,symbolrate,deviceid,channelid,typeid,typedesc,alarmvalue,starttime,alarmid,mode) values('";
	sql+=string(StrUtil::Int2Str(alarminfo.DVBType)+"','");
	sql+=StrUtil::Int2Str(alarminfo.AlarmType)+"','";
	sql+=string(alarminfo.STD+"','");
	sql+=string(alarminfo.Freq+"','");
	sql+=string(alarminfo.SymbolRate+"','");
	sql+=string(alarminfo.DeviceID+"','");
	sql+=string(alarminfo.ChannelID+"','");
	sql+=string(alarminfo.TypeID+"','");
	sql+=string(alarminfo.TypeDesc+"','");
	sql+=string(alarminfo.TypedValue+"','");
	sql+=TimeUtil::DateTimeToStr(alarminfo.CheckTime)+"','";
	sql+=string(alarminfo.AlarmID+"','");
	sql+=string(mode+"')");
	try
	{
		ret=DBCon->Execute(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加报警结果出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryAlarmInfo( string mode,std::vector<sCheckParam>& vecAlarmInfo,bool bRadioNosignal)
{
	bool ret=false;
	string sql="select * from alarminfo where mode='";
	sql+=mode;
	sql+="'";
	try
	{
		CADORecordset recordset;
		{
			if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询指标信息
			{
				while(!recordset.IsEof())
				{
					sCheckParam alarm;
					string dvbtype;
					recordset.GetFieldValue("dvbtype",dvbtype);
					alarm.DVBType=(enumDVBType)StrUtil::Str2Int(dvbtype);
					recordset.GetFieldValue("std",alarm.STD);
					recordset.GetFieldValue("freq",alarm.Freq);
					recordset.GetFieldValue("symbolrate",alarm.SymbolRate);
					recordset.GetFieldValue("channelid",alarm.ChannelID);
					recordset.GetFieldValue("typeid",alarm.TypeID);
					recordset.GetFieldValue("typedesc",alarm.TypeDesc);
					recordset.GetFieldValue("alarmvalue",alarm.TypedValue);
					//string checktime;
					//recordset.GetFieldValue("starttime",checktime);
					//alarm.CheckTime=TimeUtil::StrToDateTime(checktime);
					alarm.CheckTime = time(0);
					recordset.GetFieldValue("alarmid",alarm.AlarmID);
					recordset.GetFieldValue("deviceid",alarm.DeviceID);
					string alarmtype;
					//获得报警参数信息
					recordset.GetFieldValue("AlarmType",alarmtype);
					switch(StrUtil::Str2Int(alarmtype))
					{
					case 0:
						alarm.AlarmType=ALARM_NONE;
						break;
					case 1:
						alarm.AlarmType=ALARM_ENVIRONMENT;
						break;
					case 2:
						alarm.AlarmType=ALARM_FREQ;
						break;
					case 3:
						alarm.AlarmType=ALARM_PROGRAM;
						break;
					case 4:
						alarm.AlarmType=ALARM_TR101_290;
						break;
					default:
						alarm.AlarmType=ALARM_NONE;
						break;
					}
					if((bRadioNosignal&&("23" == alarm.TypeID))||(!bRadioNosignal&&("23" != alarm.TypeID)))
					{
						vecAlarmInfo.push_back(alarm);
					}
					recordset.MoveNext();
				}
				ret=true;
			}	
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警结果信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::UpdateAlarmInfo( eDVBType dvbtype,std::string Alarmid,string mode,string unAlarmTime)
{
	bool ret=false;

	string sql="update alarminfo set endtime='" + unAlarmTime + "' , mode='" + mode;
	sql+="' where dvbtype='"+StrUtil::Int2Str(dvbtype);
	sql+="' and alarmid='"+Alarmid;
	sql+="'";
	try
	{
		ret=DBCon->Execute(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;

}

bool SqlServerDBConnection::QueryFreqAlarmID( eDVBType dvbtype,string deviceid,string freq,string typeID,string& alarmid )
{
	bool ret=false;
	string sql="select alarmid from alarminfo where dvbtype='";
	sql+=StrUtil::Int2Str(dvbtype)+"' and deviceid='";
	sql+=deviceid+"' and freq='";
	sql+=freq+"' and typeid='";
	sql+=typeID+"' and mode='";
	sql+="0'";
	try
	{
		CADORecordset recordset;
		{
			if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询指标信息
			{
				while(!recordset.IsEof())
				{
					recordset.GetFieldValue("alarmid",alarmid);
					recordset.MoveNext();
				}
				ret=true;
			}	
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警结果信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryProgramAlarmID( eDVBType dvbtype,string deviceid,string channelid,string typeID,string& alarmid )
{
	bool ret=false;
	string sql="select alarmid from alarminfo where dvbtype='";
	sql+=StrUtil::Int2Str(dvbtype)+"' and deviceid='";
	sql+=deviceid+"' and channelid='";
	sql+=channelid+"' and typeid='";
	sql+=typeID+"'";
	try
	{
		CADORecordset recordset;
		{
			if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询指标信息
			{
				while(!recordset.IsEof())
				{
					recordset.GetFieldValue("alarmid",alarmid);
					recordset.MoveNext();
				}
				ret=true;
			}	
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警结果信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::UpdateFileEndTime( std::string filename,std::string endtime )
{
	bool ret=false;
	string sql="update record set endtime='"+endtime;
	sql+="' where filename='";
	sql+=filename;
	sql+="'";
	try
	{
		ret=DBCon->Execute(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::AddSpectrumInfo( eDVBType dvbtype,std::string freq,std::string taskid,eSpecInfo spec ,std::string checktime)
{
	bool ret=false;
	string sql="insert into spec(dvbtype,freq,taskid,type,des,valu,status,checktime) values('"+StrUtil::Int2Str(dvbtype)+"','";
	sql+=freq+"','";
	sql+=taskid+"','";
	sql+=spec.type+"','";
	sql+=spec.desc+"','";
	sql+=spec.valu+"','";
	sql+=spec.status+"','";
	sql+=checktime+"')";
	try
	{
		ret=DBCon->Execute(sql.c_str());//添加指标信息
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加频谱信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QuerySpecInfo( eDVBType dvbtype,std::string taskid,std::vector<eSpecInfo>& vecSpec,std::string startdatetime,std::string enddatetime )
{
	bool ret=false;
	string sql="select * from spec where dvbtype='"+StrUtil::Int2Str(dvbtype);
	sql+="' and taskid='";
	sql+=taskid+"' and checktime>='";
	sql+=startdatetime+"' and checktime<='";
	sql+=enddatetime+"'";
	try
	{
		CADORecordset recordset;
		{
			if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询指标信息
			{
				while(!recordset.IsEof())
				{
					//获得指标信息
					eSpecInfo spec;
					recordset.GetFieldValue("type",spec.type);
					recordset.GetFieldValue("des",spec.desc);
					recordset.GetFieldValue("valu",spec.valu);
					recordset.GetFieldValue("checktime",spec.checktime);
					recordset.GetFieldValue("freq",spec.freq);
					recordset.GetFieldValue("status",spec.status);
					recordset.MoveNext();
					vecSpec.push_back(spec);
				}
				ret=true;
			}

		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询频谱出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//wz_101229
bool SqlServerDBConnection::AddTempFile(string filename, string createtime, string expiredays)
{

	bool ret=false;
	string sql="insert into TempFile(FileName,CreateTime,ExpireDays) values('";
	sql += string(filename + "','");
	sql += string(createtime + "','");
	sql += string(expiredays + "')");
	//cout<<sql<<endl;
	try
	{
		ret=DBCon->Execute(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//wz_101229
bool SqlServerDBConnection::QueryTempFilesExpire( std::vector<std::string>& vecFilename )
{
	bool ret=false;
	string sql = "select  * from TempFile where dateadd(day,ExpireDays,CreateTime)<='";
	sql = sql + TimeUtil::GetCurDateTime() + "'";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行录像文件查询sql
		{
			while (!recordset.IsEof())
			{
				string file;
				recordset.GetFieldValue("FileName",file);//获得临时文件的文件名
				vecFilename.push_back(file);
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//wz_101229
bool SqlServerDBConnection::DeleteTempFile( std::string filename )
{
	bool ret=false;
	string sql="delete from TempFile where FileName='"+filename+"'";
	try
	{
		ret=DBCon->Execute(sql.c_str());//执行删除临时文件sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//wz_101229
bool SqlServerDBConnection::QueryTempFileByDays(unsigned int days,std::vector<std::string>& vecFilename)
{
	bool ret=false;
	string sql="select filename from TempFile where ExpireDays='"+StrUtil::Int2Str(days)+"'";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行临时文件查询sql
		{
			while (!recordset.IsEof())
			{
				string file;
				recordset.GetFieldValue("FileName",file);//获得临时文件名
				vecFilename.push_back(file);
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}

	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;

}

//wz_101229
bool SqlServerDBConnection::QueryTempFileTop(unsigned int count,std::vector<std::string>& vecFilename)
{
	bool ret=false;
	string sql = "select top "+StrUtil::Int2Str(count)+" * from TempFile order by CreateTime";

	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//执行临时文件查询sql
		{
			while (!recordset.IsEof())
			{
				string file;
				recordset.GetFieldValue("FileName",file);//获得临时文件名
				vecFilename.push_back(file);
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//报警查询 2011-03-19
bool SqlServerDBConnection::AlarmInfoQuery(vector<sCheckParam> &vecAlarm)
{
	vector<sCheckParam>::iterator itor=vecAlarm.begin();
	for(;itor!=vecAlarm.end();itor++)
	{
		bool ret=false;
		string sql = "select starttime,endtime,mode from alarminfo where DVBType='"+StrUtil::Int2Str(itor->DVBType)+"' and AlarmType='"+StrUtil::Int2Str(itor->AlarmType);
		sql+="' and Freq='"+itor->Freq+"' and TypeID='"+itor->TypeID+"' and ChannelID='"+itor->ChannelID+"'";

		try
		{
			CADORecordset recordset;
			if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))
			{
				int rowcount=0;
				rowcount=recordset.GetRecordCount();
				if(rowcount==0)
				{
					itor->StrCheckTime=TimeUtil::DateTimeToStr(time(0));
					itor->mode="0";
				}
				while (!recordset.IsEof())
				{
					string mode,starttime,endtime;
					recordset.GetFieldValue("starttime",starttime);
					recordset.GetFieldValue("endtime",endtime);
					recordset.GetFieldValue("mode",mode);
					if(mode.substr(0,1)=="0")
					{
						itor->StrCheckTime=starttime;
						itor->mode="1";
					}
					else if(mode.substr(0,1)=="1")
					{
						itor->StrCheckTime=endtime;
						itor->mode="2";
					}
					recordset.MoveNext();
				}
			}
		}
		catch (...)
		{
			APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时信息出错！",LOG_OUTPUT_FILE);
			continue;
		}
	}
	return true;
}

bool SqlServerDBConnection::AlarmStateClear(string Freq)
{
	bool ret=false;
	string sql="";
	if(Freq=="ALL")
		sql="delete from alarminfo where Freq<>''";
	else
		sql="delete from alarminfo where Freq='"+Freq+"'";
	try
	{
		ret=DBCon->Execute(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"报警状态删除信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}

bool SqlServerDBConnection::UpdateAlarmRecordExpireTime(eDVBType dvbtype,std::string Alarmid)
{
	bool ret=false;
	string sql="select channelid,starttime,endtime from alarminfo where alarmid='"+Alarmid+"'";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询sql
		{
			while (!recordset.IsEof())
			{
				string channelid,starttime,endtime;
				recordset.GetFieldValue("ChannelID",channelid);
				recordset.GetFieldValue("StartTime",starttime);
				recordset.GetFieldValue("EndTime",endtime);

				string days=PROPMANAGER::instance()->GetAlarmRecordExpire(dvbtype);
				string updatesql,selectForeSql,selectBackSql;

				//获取异态录像存储配置信息
				sAlarmRecordStorageCfg alarmRecCfgInfo;
				PROPMANAGER::instance()->GetAlarmRecStorageCfg(dvbtype, alarmRecCfgInfo);
				if (alarmRecCfgInfo._switch == "1")		//开关打开
				{
					string whereSql = "where dvbtype='" + StrUtil::Int2Str(dvbtype);
					whereSql += "' and channelid='" + channelid;
					whereSql += "' and starttime<='" + endtime;
					whereSql += "' and endtime>='" + starttime + "'";

					string totalSelectSql = "select * from record ";
					totalSelectSql += whereSql;
					try
					{
						CADORecordset tempRecSet;
						if (tempRecSet.Open(DBCon->GetActiveConnection(),totalSelectSql.c_str()))//查询sql
						{
							int count = tempRecSet.GetRecordCount();	//获取异态录像文件个数
							if (count > 2*StrUtil::Str2Int(alarmRecCfgInfo._filenum))	//如果异态录像文件个数 > 2*filenum
							{
								updatesql = "update record set expiredays='" + days + "' where filename in ";
								selectForeSql = "(select top " + alarmRecCfgInfo._filenum + " filename from record " + whereSql + " order by starttime)";
								selectBackSql = "(select top " + alarmRecCfgInfo._filenum + " filename from record " + whereSql + "order by starttime desc)";

								try
								{
									string execSql = updatesql + selectForeSql;
									ret=DBCon->Execute(execSql.c_str());
									execSql = updatesql + selectBackSql;
									ret=DBCon->Execute(execSql.c_str());
								}
								catch (...)
								{
									APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新异态录像过期时间出错！",LOG_OUTPUT_FILE);
								}
								recordset.MoveNext();//移动到下一条记录
								continue;
							}
						}
					}//try
					catch (...)
					{
						APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询异态录像出错！",LOG_OUTPUT_FILE);
					}	
				}

				updatesql="update record set expiredays='"+days+"' where dvbtype='"+StrUtil::Int2Str(dvbtype);
				updatesql+="' and channelid='"+channelid;
				updatesql+="' and starttime<='"+endtime;
				updatesql+="' and endtime>='"+starttime+"'";

				try
				{
					ret=DBCon->Execute(updatesql.c_str());
				}
				catch (...)
				{
					APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新异态录像过期时间出错！",LOG_OUTPUT_FILE);
				}
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询异态报警信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;

}
bool SqlServerDBConnection::QueryFileConfig(eDVBType dvbtype,std::string NodeName,std::string& NodeValue)
{
	bool ret=false;
	string sql="";
	
		sql="select ConfigValue from Config where dvbtype='"+StrUtil::Int2Str(dvbtype);
		sql+="' and ConfigName='"+NodeName+"'";
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询sql
		{
			while (!recordset.IsEof())
			{
				recordset.GetFieldValue("ConfigValue",NodeValue);			
				recordset.MoveNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询配置信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}


bool SqlServerDBConnection::QuerySystemConfig(eDVBType type, sSysConfigParam &config)
{
	bool bFind = false;
	string sql = string("declare @sql varchar(8000)\
				set @sql = 'select DVBType '\
				select @sql = @sql + ', max(case ConfigName when ''' + ConfigName + ''' then ConfigValue else STR(NULL) end) [' + ConfigName + ']'\
				from (select distinct ConfigName from Config where DVBType='") + StrUtil::Int2Str(type)
				+ string("') as a\
				set @sql = @sql + ' from Config group by DVBType'\
				exec(@sql)");
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))
		{
			while(!recordset.IsEof())
			{
				string type_;
				recordset.GetFieldValue("DVBType",type_);
				if(StrUtil::Str2Int(type_) != type)
				{
					recordset.MoveNext();
					continue;
				}
				bFind = true;
				recordset.GetFieldValue("SystemIp",config.SystemIp);
				recordset.GetFieldValue("VideoHttpServerIp",config.VideoHttpServerIp);
				recordset.GetFieldValue("VideoHttpPort",config.VideoHttpPort);
				recordset.GetFieldValue("VideoHttpMaxnum",config.VideoHttpMaxnum);
				recordset.GetFieldValue("VideoRtspServerIp",config.VideoRtspServerIp);
				recordset.GetFieldValue("VideoRtspPort",config.VideoRtspPort);
				recordset.GetFieldValue("VideoRtspMaxnum",config.VideoRtspMaxnum);
				recordset.GetFieldValue("QualityServerIp",config.QualityServerIp);
				recordset.GetFieldValue("QualityPort",config.QualityPort);
				recordset.GetFieldValue("QualityMaxnum",config.QualityMaxnum);
				recordset.GetFieldValue("DeviceServerIp",config.DeviceServerIp);
				recordset.GetFieldValue("DevicePort",config.DevicePort);
				recordset.GetFieldValue("DeviceMaxnum",config.DeviceMaxnum);
				recordset.GetFieldValue("LogServerIp",config.LogServerIp);
				recordset.GetFieldValue("LogPort",config.LogPort);
				recordset.GetFieldValue("LogMaxnum",config.LogMaxnum);
				recordset.GetFieldValue("XmlServerIp",config.XmlServerIp);
				recordset.GetFieldValue("XmlPort",config.XmlPort);
				recordset.GetFieldValue("HttpServerIp",config.HttpServerIp);
				recordset.GetFieldValue("HttpPort",config.HttpPort);
				recordset.GetFieldValue("FtpServerIp",config.FtpServerIp);
				recordset.GetFieldValue("FtpPort",config.FtpPort);
				recordset.GetFieldValue("RecordQualityServer",config.RecordQualityIp);
				recordset.GetFieldValue("RecordQualityPort",config.RecordQualityPort);
				recordset.GetFieldValue("RecordQualityMaxnum",config.RecordQualityMaxnum);
				recordset.GetFieldValue("Record_FileLocation",config.Record_FileLocation);
				recordset.GetFieldValue("Record_FileSharePath",config.Record_FileSharePath);
				recordset.GetFieldValue("Record_DeviceId",config.Record_DeviceId);
				recordset.GetFieldValue("Record_Period",config.Record_Period);
				recordset.GetFieldValue("Record_MaxAvailableSize",config.Record_MaxAvailableSize);
				recordset.GetFieldValue("Record_DBCleanInterval",config.Record_DBCleanInterval);
				recordset.GetFieldValue("Record_SystemCleanTime",config.Record_SystemCleanTime);
				recordset.GetFieldValue("Record_StoreType",config.Record_StoreType);
				recordset.GetFieldValue("Log_Path",config.Log_Path);
				recordset.GetFieldValue("Log_Expire",config.Log_Expire);
				recordset.GetFieldValue("Log_Type",config.Log_Type);
				recordset.GetFieldValue("Log_outputToFile",config.Log_outputToFile);
				recordset.GetFieldValue("LogPathType_Analyser",config.LogPathType_Analyser);
				recordset.GetFieldValue("LogPathType_Record",config.LogPathType_Record);
				recordset.GetFieldValue("LogPathType_Video",config.LogPathType_Video);
				recordset.GetFieldValue("LogPathType_Device",config.LogPathType_Device);
				recordset.GetFieldValue("LogPathType_Other",config.LogPathType_Other);
				recordset.GetFieldValue("LogPathType_Default",config.LogPathType_Default);
				/*recordset.GetFieldValue("Center_SrcCode",config.Center_SrcCode);
				recordset.GetFieldValue("Center_Url",config.Center_Url);
				recordset.GetFieldValue("SubCenter_SrcCode",config.SubCenter_SrcCode);
				recordset.GetFieldValue("SubCenter_Url",config.SubCenter_Url);*/
				recordset.GetFieldValue("XmlSendTime",config.XmlSendTime);
				recordset.GetFieldValue("XmlOverTime",config.XmlOverTime);
				recordset.GetFieldValue("TempFileLocation",config.TempFileLocation);
				recordset.GetFieldValue("TempSharePath",config.TempSharePath);
				recordset.GetFieldValue("TempFileExpireTime",config.TempFileExpireTime);

				recordset.GetFieldValue("DeviceSchedulerTask_Type",config.DeviceSchedulerTask_Type);
				recordset.GetFieldValue("DeviceSchedulerTask_WeekDay",config.DeviceSchedulerTask_WeekDay);
				recordset.GetFieldValue("DeviceSchedulerTask_Date",config.DeviceSchedulerTask_Date);
				recordset.GetFieldValue("DeviceSchedulerTask_Time",config.DeviceSchedulerTask_Time);

				recordset.GetFieldValue("PsisiSchedulerTask_Type",config.PsisiSchedulerTask_Type);
				recordset.GetFieldValue("PsisiSchedulerTask_WeekDay",config.PsisiSchedulerTask_WeekDay);
				recordset.GetFieldValue("PsisiSchedulerTask_Date",config.PsisiSchedulerTask_Date);
				recordset.GetFieldValue("PsisiSchedulerTask_Time",config.PsisiSchedulerTask_Time);
				break;
			}
			if(!bFind)
			{
				APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"没有查到相关系统配置信息！",LOG_OUTPUT_FILE);
				return false;
			}
			return true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询系统配置出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return false;
}

bool SqlServerDBConnection::QueryDvbConfig( eDVBType type,vector<sDvbConfigParam> &VecConfig)
{
	bool bFind = false;
	string sql = string("declare @sql varchar(8000)\
						set @sql = 'select DVBType '\
						select @sql = @sql + ', max(case ConfigName when ''' + ConfigName + ''' then ConfigValue else STR(NULL) end) [' + ConfigName + ']'\
						from (select distinct ConfigName from Config where DVBType='") + StrUtil::Int2Str(type)
						+ string("') as a\
								 set @sql = @sql + ' from Config group by DVBType'\
								 exec(@sql)");
	try
	{
		CADORecordset recordset;
		if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))
		{
			while(!recordset.IsBof())
			{
				sDvbConfigParam config;
				recordset.GetFieldValue("DVBType",config.type);
				if(StrUtil::Str2Int(config.type) !=type)//0 为系统配置，其他为监测类型相关配置
				{
					recordset.MoveNext();
					continue;
				}
				bFind = true;
				if(config.type == "1" || config.type == "3" || config.type == "7" || config.type == "9")
				{
					recordset.GetFieldValue("CenterFreq",config.CenterFreq);
					recordset.GetFieldValue("Symbolrate",config.Symbolrate);
					recordset.GetFieldValue("Qam",config.Qam);
					recordset.GetFieldValue("TablePath",config.TablePath);
					recordset.GetFieldValue("SharePath",config.SharePath);

					recordset.GetFieldValue("HDStreamBps",config.HDRealStreamBps);
					recordset.GetFieldValue("HDStreamWidth",config.HDRealStreamWidth);
					recordset.GetFieldValue("HDStreamHeight",config.HDRealStreamHeight);

					recordset.GetFieldValue("SDStreamBps",config.SDRealStreamBps);
					recordset.GetFieldValue("SDStreamWidth",config.SDRealStreamWidth);
					recordset.GetFieldValue("SDStreamHeight",config.SDRealStreamHeight);

					recordset.GetFieldValue("HDRecordBps",config.HDRecordBps);
					recordset.GetFieldValue("HDRecordWidth",config.HDRecordWidth);
					recordset.GetFieldValue("HDRecordHeight",config.HDRecordHeight);

					recordset.GetFieldValue("SDRecordBps",config.SDRecordBps);
					recordset.GetFieldValue("SDRecordWidth",config.SDRecordWidth);
					recordset.GetFieldValue("SDRecordHeight",config.SDRecordHeight);

				}
				else if(config.type == "4" || config.type == "6")
				{

					recordset.GetFieldValue("StreamBps",config.SDRealStreamBps);
					recordset.GetFieldValue("RecordBps",config.SDRecordBps);
				}
				else
				{

					recordset.GetFieldValue("StreamBps",config.SDRealStreamBps);
					recordset.GetFieldValue("StreamWidth",config.SDRealStreamWidth);
					recordset.GetFieldValue("StreamHeight",config.SDRealStreamHeight);

					recordset.GetFieldValue("RecordBps",config.SDRecordBps);
					recordset.GetFieldValue("RecordWidth",config.SDRecordWidth);
					recordset.GetFieldValue("RecordHeight",config.SDRecordHeight);

				}
				recordset.GetFieldValue("ChannelScanType",config.ChannelScanType);
				recordset.GetFieldValue("Osd_fontsize",config.Osd_fontsize);
				recordset.GetFieldValue("Osd_infoosd",config.Osd_infoosd);
				recordset.GetFieldValue("Osd_infoosdx",config.Osd_infoosdx);
				recordset.GetFieldValue("Osd_infoosdy",config.Osd_infoosdy);
				recordset.GetFieldValue("Osd_timeosdtype",config.Osd_timeosdtype);
				recordset.GetFieldValue("Osd_timeosdx",config.Osd_timeosdx);
				recordset.GetFieldValue("Osd_timeosdy",config.Osd_timeosdy);
				recordset.GetFieldValue("Osd_programx",config.Osd_programx);
				recordset.GetFieldValue("Osd_programy",config.Osd_programy);
				recordset.GetFieldValue("AlarmType",config.AlarmType);
				recordset.GetFieldValue("AlarmInterval",config.AlarmInterval);
				recordset.GetFieldValue("AlarmOnceDay",config.OnceDay);
				recordset.GetFieldValue("AlarmRunPlanRecord",config.RunplanRecord);
				recordset.GetFieldValue("AlarmServer",config.AlarmServer);
				recordset.GetFieldValue("VideoStreamProtocol",config.VideoStreamProtocol);
				recordset.GetFieldValue("VideoFileProtocol",config.VideoFileProtocol);
				recordset.GetFieldValue("VideoFileUrlType",config.VideoFileUrlType);
				recordset.GetFieldValue("VideoFileOffSet",config.VideoFileOffSet);
				recordset.GetFieldValue("RecordParam_recordexpire",config.RecordParam_recordexpire);
				recordset.GetFieldValue("RecordParam_alarmrecordexpire",config.RecordParam_alarmrecordexpire);
				recordset.GetFieldValue("RecordParam_alarmheadoffset",config.RecordParam_alarmheadoffset);
				recordset.GetFieldValue("RecordParam_alarmtailoffset",config.RecordParam_alarmtailoffset);
				recordset.GetFieldValue("RecordParam_alarmrecordmode",config.RecordParam_alarmrecordmode);
				recordset.GetFieldValue("RecordDown_protocol",config.RecordDown_protocol);
				recordset.GetFieldValue("RecordDown_urltype",config.RecordDown_urltype);
				recordset.GetFieldValue("RecordDown_offset",config.RecordDown_offset);
				recordset.GetFieldValue("GeneralDestCode",config.GeneralDestCode);
				recordset.GetFieldValue("GeneralSrcCode",config.GeneralSrcCode);
				recordset.GetFieldValue("RealtimeStreamUseRecordId",config.RealTimeFromRecord);
				recordset.GetFieldValue("AlarmRecordSwitch", config.AlarmStorage_switch);		//异态录像存储开关	wz
				recordset.GetFieldValue("AlarmRecordFilenum", config.AlarmStorage_filenum);	//异态录像存储文件个数	wz
				VecConfig.push_back(config);
				break;
			}
			if(!bFind)
			{
				APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"没有查到监测类型相关配置信息！",LOG_OUTPUT_FILE);
				return false;
			}
			return true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询监测类型相关配置出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return false;
}

bool SqlServerDBConnection::QueryAlarmLevelThreshold( vector<ThresHold> &VecThresHold)
{
	std::string sql="select * from LevelThresholdConfig ";
	bool ret=false;
	try
	{
		CADORecordset recordset;
		{
			if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询报警电平门限信息
			{
				while(!recordset.IsEof())
				{
					ThresHold threshold;
					string dvbtype;
					recordset.GetFieldValue("DVBType",dvbtype);
					recordset.GetFieldValue("Freq",threshold.freq);
					recordset.GetFieldValue("DownThreshold",threshold.downthreshod);
					threshold.dvbtype = OSFunction::GetEnumDVBType(dvbtype);
					recordset.MoveNext();
					VecThresHold.push_back(threshold);
				}
				ret=true;
			}

		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警电平门限出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::SetAlarmLevelThreshold( eDVBType dvbtype,string freq,string alarmthreshold)
{
	bool ret=false;
	string sql  ="update LevelThresholdConfig set DownThreshold='"; 
		   sql += string(alarmthreshold+"' ");
	       sql += string("where dvbtype='"+StrUtil::Int2Str(dvbtype));
	       sql += string("' and freq='"+freq+"'");
	string delsql  ="delete from LevelThresholdConfig "; 
	       delsql += string("where dvbtype='"+StrUtil::Int2Str(dvbtype));
	       delsql += string("' and freq='"+freq+"'");

	string insertsql  ="insert into LevelThresholdConfig values('"+StrUtil::Int2Str(dvbtype)+"','";
		   insertsql += freq+"','";
		   insertsql += alarmthreshold+"')";
	try
	{
		ret=DBCon->Execute(delsql.c_str());
		ret=DBCon->Execute(insertsql.c_str());
		
		/*
		ret=DBCon->Execute(sql.c_str());
		if(DBCon->GetRecordsAffected()==0)
		{
			string insertsql="insert into LevelThresholdConfig values('"+StrUtil::Int2Str(dvbtype)+"','";
			insertsql += freq+"','";
			insertsql += alarmthreshold+"')";
			ret=DBCon->Execute(insertsql.c_str());
		}*/
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"设置无载波报警门限出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryAlarmPriority(std::vector<sAlarmPriority> &vecAlarmPriority)
{
	std::string sql="select * from AlarmLevelConfig ";
	bool ret=false;
	try
	{
		CADORecordset recordset;
		{
			if (recordset.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询报警电平门限信息
			{
				while(!recordset.IsEof())
				{
					sAlarmPriority alarmpriority;
					string dvbtype,alarmtype,val,TypeID;
					recordset.GetFieldValue("DVBType",dvbtype);
					recordset.GetFieldValue("AlarmType",alarmtype);
					recordset.GetFieldValue("Value",val);
					alarmpriority.dvbtype = OSFunction::GetEnumDVBType(dvbtype);
					alarmpriority.priority = val;
					alarmpriority.type = OSFunction::GetAlarmTypeID(OSFunction::GetEnumDVBType(dvbtype),alarmtype);
					recordset.MoveNext();
					vecAlarmPriority.push_back(alarmpriority);
				}
				ret=true;
			}

		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警电平门限出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqlServerDBConnection::QueryScheduleAlarmInfo( string taskid )
{
	bool ret=true;
	string sql="select * from alarminfo where TypeID='";
	sql+=taskid;
	sql+="'";
	try
	{
		CADORecordset record;
		{
			if (record.Open(DBCon->GetActiveConnection(),sql.c_str()))//查询指标信息
			{
				int rowcount=0;
				{
					rowcount=record.GetRecordCount();
				}
				if(rowcount==0)
				{
					return true;
				}
				else
				{
					return false;
				}
			}	
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警结果信息出错！",LOG_OUTPUT_FILE);
		return true;
	}
	return ret;
}
