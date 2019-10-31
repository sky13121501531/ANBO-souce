


#include "./SqliteDBConnection.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include <time.h>
#include <iostream>
#include <algorithm>

const char* SQLITE_DB_PATH = "C:/vscttb/db/vsdvb.db";

SqliteDBConnection::SqliteDBConnection(void)
{
	DBCon=new DeriveLsql;
}

SqliteDBConnection::~SqliteDBConnection(void)
{
	if (DBCon!=NULL)
	{
		delete DBCon;
		DBCon=NULL;
	}
}


bool SqliteDBConnection::DisConnect( void )
{
	try
	{
		if (DBCon != NULL)
		{
			int ret = DBCon->Close();
			if (ret != LSQL_OK)
			{
				const char* errMsg = DBCon->ErrorMessage();
				printf("\n SqliteDBConnection::DisConnect() : ");
				printf(errMsg);
				printf("\n");
				return false;
			}
		}
	}
	catch ( ... )
	{
		std::cout<<"SqliteDBConnection DisConnect fail!!"<<std::endl;
		return false;
	}
	return true;
}



bool SqliteDBConnection::InitConnect( void )
{
	bool ret = false;
	int count = 0;

	try
	{
		/* 连接3次 */
		while (count < 3)
		{
			int ret = DBCon->Open( SQLITE_DB_PATH );
			if (ret !=LSQL_OK)
			{
				if (ret == 0xe)		//数据库所在路径不存在
				{
					std::cout<<"数据库所在路径不存在，请检查C:/vscttb/db/"<<std::endl;
					//APPLOG::instance()->WriteLog(OTHER, LOG_EVENT_ERROR, "数据库所在路径不存在，请检查C:/vscttb/db/",LOG_OUTPUT_BOTH);
					DBCon->Close();
					return false;
				}
				string errorMsg = DBCon->ErrorMessage();
				std::cout<<"SqliteDBConnection::InitConnect error : "<<errorMsg<<std::endl;
				//APPLOG::instance()->WriteLog(OTHER, LOG_EVENT_ERROR, "SqliteDBConnection::InitConnect error!!");
				DBCon->Close();
				count ++;
			}
			else
				break;
		}
	}
	catch ( ... )
	{
		////APPLOG::instance()->WriteLog(OTHER, LOG_EVENT_ERROR, "SqliteDBConnection 创建失败!!");
		std::cout<<"SqliteDBConnection 创建失败!!"<<std::endl;
		return false;
	}

	if (count == 3)	//3次没有连接成功，返回失败
	{
		return false;
	}

	/** 设置超时时间
	*/
	DBCon->SetTimeOut();	/* 默认值为500 ms */

	/* 创建各个表 */
	CreateSqliteTable();
	
	return true;	
}

bool SqliteDBConnection::CreateSqliteTable()
{
	try
	{
		if (DBCon->IsTableExist("AlarmInfo") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table AlarmInfo(DVBType,DeviceID,AlarmType,Freq,STD,TypeID,TypeDesc,	\
								ChannelID,SymbolRate,AlarmValue,StartTime,EndTime,AlarmID,Mode)");
		}
		if (DBCon->IsTableExist("AlarmParam") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table AlarmParam(DVBType,DeviceID,AlarmType,Freq,STD,TypeID,TypeDesc,	\
								ChannelID,SymbolRate,Duration,Num,TimeInterval,DownThreshold,UpThreshold,Switch)");
		}
		if (DBCon->IsTableExist("AlarmType") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table AlarmType(TypeID,TypeName)");
			DBCon->ExecNonQuery("insert into AlarmType values('0','ALARM_NONE')");
			DBCon->ExecNonQuery("insert into AlarmType values('1','ALARM_FREQ')");
			DBCon->ExecNonQuery("insert into AlarmType values('2','ALARM_PROGRAM')");
			DBCon->ExecNonQuery("insert into AlarmType values('3','ALARM_TR101_290')");
		}
		if (DBCon->IsTableExist("Channel") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table Channel(DVBType,ChannelID,ChannelInfo)");
		}
		if (DBCon->IsTableExist("DVBType") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table DVBType(TypeID,TypeName)");
			DBCon->ExecNonQuery("insert into DVBType values('0','UNKNOW')");
			DBCon->ExecNonQuery("insert into DVBType values('1','CTTB')");
			DBCon->ExecNonQuery("insert into DVBType values('2','CMMB')");
			DBCon->ExecNonQuery("insert into DVBType values('3','DVBC')");
			DBCon->ExecNonQuery("insert into DVBType values('4','RADIO')");
			DBCon->ExecNonQuery("insert into DVBType values('5','ATV')");
			DBCon->ExecNonQuery("insert into DVBType values('6','AM')");
		}
		if (DBCon->IsTableExist("MsgID") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table MsgID(MsgID)");
			DBCon->ExecNonQuery("insert into MsgID values('1')");
		}
		if (DBCon->IsTableExist("Quality") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table Quality(DVBType,TaskID,Freq,Type,Valu,Des,CheckTime,ReportTime)");
		}
		if (DBCon->IsTableExist("QualityCompensation") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table QualityCompensation(DVBType,DeviceID,Type,Valu)");
		}
		if (DBCon->IsTableExist("Record") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table Record(DVBType,ChannelID,TaskID,FileName,StartTime,EndTime,ExpireDays)");
		}
		if (DBCon->IsTableExist("RunPlanParam") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table RunPlanParam(DVBType,ChannelID,Type,DayOfWeek,mMonth,		\
								DayofMonth,StartTime,EndTime,ValidStartDateTime,ValidEndDateTime,StartDateTime,EndDateTime)");
		}
		if (DBCon->IsTableExist("Spec") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table Spec(DVBType,TaskID,Freq,Type,Valu,Des,Status,CheckTime,ReportTime)");
		}
		if (DBCon->IsTableExist("Task") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table Task(DVBType,TaskInfo)");
		}
		if (DBCon->IsTableExist("TempFile") != LSQL_OK)
		{
			DBCon->ExecNonQuery("create table TempFile(FileName,CreateTime,ExpireDays)");
		}
	}
	catch ( ... )
	{
		printf("SqliteDBConnection::CreateSqliteTable 失败!!\n\n");
		return false;
	}
	
	return true;
}


bool SqliteDBConnection::IsConnected( void )
{
	bool ret = (DBCon!=NULL);
	if (!ret)
	{
		printf("DBcon 为空!! \n");
	}
	return ret;
}


bool SqliteDBConnection::AddXmlTask( enumDVBType type,std::string xml )
{
	
	int id=type;
	bool ret=false;

	ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);

	DeleteTask(type,xml);	//添加任务之前，先删除相同的任务

	std::string sql = "insert into task values('" + StrUtil::Int2Str(type) + "','" + xml + "')";
	try
	{
		int result = DBCon->ExecNonQuery(sql.c_str());//执行添加任务sql
		if (result == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::AddXmlTask() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		////APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"向数据库中添加任务出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqliteDBConnection::DeleteTask( enumDVBType type,std::string xml )
{
	bool ret=false;
	int id=type;
	std::string sql="";
	if(type==UNKNOWN)
	{
		sql="delete from task where taskinfo='"+xml+"'";
	}
	else
	{
		sql="delete from task where dvbtype='"+StrUtil::Int2Str(id)+"' and taskinfo='"+xml+"'";
	}

	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);

		int result = DBCon->ExecNonQuery(sql.c_str());		//执行删除任务sql(没有任务也会返回成功)
		if (result == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::DeleteTask() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		////APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"从数据库中删除任务出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::QueryTask( enumDVBType type,std::vector<std::string>& vecXml )
{
	int id=type;

	std::string sql="select taskinfo from task where dvbtype='"+StrUtil::Int2Str(id)+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);

		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryTask() : "<<errMsg<<std::endl;
			//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"SqliteDBConnection::QueryTask() fail!!",LOG_OUTPUT_FILE);
			
			return false;
		}
		DBCon->MoveFirst();

		int infoNum = DBCon->GetRowCount();
		for (int i=0; i<infoNum; i++)
		{
			string taskinfo;
			DBCon->GetCollect("TaskInfo", taskinfo);
			vecXml.push_back( taskinfo );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}


bool SqliteDBConnection::QueryTask( std::vector<std::string>& vecXml )
{
	std::string sql="select taskinfo from task";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryTask() : "<<errMsg<<std::endl;
			
			return false;
		}
		DBCon->MoveFirst();

		int infoNum = DBCon->GetRowCount();
		for (int i=0; i<infoNum; i++)
		{
			string taskinfo;
			DBCon->GetCollect("TaskInfo", taskinfo);

			vecXml.push_back( taskinfo );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}


bool SqliteDBConnection::AddChannel( enumDVBType type,std::string channelid,std::string xml )
{
	int id=type;
	bool ret = false;

	std::string addsql="insert into channel values('"+StrUtil::Int2Str(id)+"','"+channelid+"','"+xml+"')";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int result = DBCon->ExecNonQuery(addsql.c_str());//执行删除频道，再执行添加频道信息
		if (result == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::AddChannel() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"向数据库中添加频道扫描结果出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::DeleteChannel( enumDVBType type,std::string channelid)
{
	int id=type;
	bool ret=false;
	std::string sql="delete from channel where dvbtype='"+StrUtil::Int2Str(id)+"' and channelid='"+channelid+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int result = DBCon->ExecNonQuery(sql.c_str());//执行删除频道信息sql
		if (result == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::DeleteChannel() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"从数据库中删除频道扫描结果出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}



bool SqliteDBConnection::DeleteChannelByType( enumDVBType type)
{
	int id=type;
	bool ret=false;
	std::string sql="delete from channel where dvbtype='"+StrUtil::Int2Str(id)+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecNonQuery(sql.c_str())==LSQL_OK )//执行删除频道信息sql
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::DeleteChannelByType() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"从数据库中删除频道扫描结果出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::QueryChannel( enumDVBType type,std::string channelid,std::string& xml )
{
	int id=type;

	std::string sql="select channelinfo from channel where dvbtype='"+StrUtil::Int2Str(id)+"' and channelid='"+channelid+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryChannel() : "<<errMsg<<std::endl;
			return false;
		}
		DBCon->MoveFirst();
		DBCon->GetCollect("ChannelInfo", xml);
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}



bool SqliteDBConnection::QueryAllChannel( std::vector<std::string>&vecXml )
{
	std::string sql="select channelinfo from channel";

	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryAllChannel() : "<<errMsg<<std::endl;
			
			return false;
		}
		DBCon->MoveFirst();

		int infoNum = DBCon->GetRowCount();
		for (int i=0; i<infoNum; i++)
		{
			string channelinfo;
			DBCon->GetCollect("ChannelInfo", channelinfo);

			vecXml.push_back( channelinfo );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}


bool SqliteDBConnection::QueryTypeChannel( enumDVBType type,std::vector<std::string>&vecXml )
{
	int id=type;
	std::string sql="select channelinfo from channel where dvbtype='"+StrUtil::Int2Str(id)+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryTypeChannel() : "<<errMsg<<std::endl;
			return false;
		}
		DBCon->MoveFirst();

		int infoNum = DBCon->GetRowCount();
		for (int i=0; i<infoNum; i++)
		{
			string channelinfo;
			DBCon->GetCollect("ChannelInfo", channelinfo);

			vecXml.push_back( channelinfo );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}


bool SqliteDBConnection::AddRecord( sRecordInfo record )
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
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int result = DBCon->ExecNonQuery(sql.c_str());//执行添加录像信息xml
		if (result == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::AddRecord() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}



bool SqliteDBConnection::QueryRecordByTaskid( enumDVBType type,std::string taskId,std::vector<sRecordInfo>& vecRecordInfo,string StartTime,string EndTime)
{
	int id=type;

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
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryRecordByTaskid() : "<<errMsg<<std::endl;
			
			return false;
		}
		DBCon->MoveFirst();

		while ( DBCon->IsEOF() != LSQL_OK )
		{
			sRecordInfo record;

			DBCon->GetCollect("FileName", record.filename);
			DBCon->GetCollect("StartTime", record.starttime);
			DBCon->GetCollect("EndTime", record.endtime);

			vecRecordInfo.push_back( record );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}


bool SqliteDBConnection::QueryRecordByChannelID( enumDVBType type,std::string taskID,std::string channelID,std::vector<sRecordInfo>& vecRecordInfo,string StartTime,string EndTime)
{
	int id=type;

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
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryRecordByChannelID() : "<<errMsg<<std::endl;
			
			return false;
		}
		DBCon->MoveFirst();

		while ( DBCon->IsEOF() != LSQL_OK )
		{
			sRecordInfo record;

			DBCon->GetCollect("FileName", record.filename);
			DBCon->GetCollect("StartTime", record.starttime);
			DBCon->GetCollect("EndTime", record.endtime);

			vecRecordInfo.push_back( record );
			DBCon->MoveNext();
		}	
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}



bool SqliteDBConnection::QueryRecordByDeviceID(enumDVBType type,std::string taskID,std::string DeviceID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime,std::string EndTime)
{
	int id=type;

	string sql=string("select * from record where dvbtype='") + StrUtil::Int2Str(id);
	sql += string(string("' and taskid='")+taskID);

	if(StrUtil::Str2Int(DeviceID) <= 0) return false;

	sql += string(string("' and filename like '%/")+ DeviceID + "_%'");

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
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryRecordByDeviceID() : "<<errMsg<<std::endl;
			return false;
		}
		DBCon->MoveFirst();

		while ( DBCon->IsEOF() != LSQL_OK )
		{
			sRecordInfo record;

			DBCon->GetCollect("FileName", record.filename);
			DBCon->GetCollect("StartTime", record.starttime);
			DBCon->GetCollect("EndTime", record.endtime);

			vecRecordInfo.push_back( record );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}




bool SqliteDBConnection::DeleteRecord( std::string filename )
{
	bool ret=false;
	string sql="delete from record where filename='"+filename+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int result = DBCon->ExecNonQuery(sql.c_str());//执行删除录像sql
		if (result == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::DeleteRecord() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::QueryRecordByDays( unsigned int days,std::vector<std::string>& vecFilename )
{
	return true;		//该函数有问题，暂时忽略//gxd 2011-03-31
	/************************************************************************/

	string sql="select filename from record where expiredays='"+StrUtil::Int2Str(days)+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryRecordByDays() : "<<errMsg<<std::endl;		
			return false;
		}
		DBCon->MoveFirst();

		while (DBCon->IsEOF() != LSQL_OK)
		{
			string strFilename;
			DBCon->GetCollect("FileName", strFilename);
			
			vecFilename.push_back( strFilename );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}


bool SqliteDBConnection::QueryRecordTopInPath(unsigned int count, std::string recPath, std::vector<std::string>& vecFilename)
{
	bool ret = false;

	string sql = "select * from record where filename like '%"+recPath
		+"%' order by expiredays,starttime limit " + StrUtil::Int2Str(count);

	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryRecordTopInPath() : "<<errMsg<<std::endl;
			return false;
		}
		DBCon->MoveFirst();

		while (DBCon->IsEOF() != LSQL_OK)
		{
			string filename;
			DBCon->GetCollect("FileName", filename);

			vecFilename.push_back( filename );
			DBCon->MoveNext();
		}		
	}
	catch (...)
	{	
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}




bool SqliteDBConnection::QueryRecordTop(unsigned int count, std::vector<std::string>& vecFilename )
{
	string sql = "select * from record order by expiredays,starttime limit " + StrUtil::Int2Str(count);

	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryRecordTop() : "<<errMsg<<std::endl;
			
			return false;
		}
		DBCon->MoveFirst();

		while (DBCon->IsEOF() != LSQL_OK)
		{
			string strFilename;
			DBCon->GetCollect("FileName", strFilename);

			vecFilename.push_back( strFilename );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}



bool SqliteDBConnection::QueryChannelID( enumDVBType type,std::vector<std::string>&vecChannelID )
{
	int id=type;
	std::string sql="select channelid from channel where dvbtype='"+StrUtil::Int2Str(id)+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			string errMsg = DBCon->ErrorMessage();
			std::cout<<"SqliteDBConnection::QueryChannelID() : "<<errMsg<<std::endl;
			
			return false;
		}
		DBCon->MoveFirst();

		while (DBCon->IsEOF() != LSQL_OK)
		{
			string strChanId;
			DBCon->GetCollect("ChannelID", strChanId);

			vecChannelID.push_back( strChanId );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}


bool SqliteDBConnection::UpdateAlarmParam(sAlarmParam& alarmparam)
{
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
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(selsql.c_str()) != LSQL_OK)
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::UpdateAlarmParam() : ");
			printf(errMsg);
			printf("\n");
			return false;
		}
		DBCon->MoveFirst();

		int num = DBCon->GetRowCount();
		if (num == 0)
		{
			int result = DBCon->ExecNonQuery(insertsql.c_str());
			if (result != LSQL_OK)
			{
				string errMsg = DBCon->ErrorMessage();
				std::cout<<"SqliteDBConnection::UpdateAlarmParam() : "<<errMsg<<std::endl;
				return false;
			}
		}
		else
		{
			int result = DBCon->ExecNonQuery(updatesql.c_str());
			if (result != LSQL_OK)
			{
				const char* errMsg = DBCon->ErrorMessage();
				printf("\n SqliteDBConnection::UpdateAlarmParam() : ");
				printf(errMsg);
				printf("\n");
				return false;
			}
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新报警参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}



bool SqliteDBConnection::QueryAlarmParam(enumDVBType type,std::vector<sAlarmParam>& AlarmParamVec)
{
	std::string sql="select * from AlarmParam where DVBType = '";
	sql += StrUtil::Int2Str(type) + "'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			printf("\n SqliteDBConnection::QueryAlarmParam() : ");
			const char* errMsg = DBCon->ErrorMessage();
			printf(errMsg);
			printf("\n");
			return false;
		}
		DBCon->MoveFirst();

		while (DBCon->IsEOF() != LSQL_OK)
		{
			sAlarmParam param;
			param.DVBType = type;
			string alarmtype;
			DBCon->GetCollect("AlarmType", alarmtype);
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
			DBCon->GetCollect("TypeID",param.TypeID);
			DBCon->GetCollect("TypeDesc",param.TypeDesc);
			DBCon->GetCollect("STD",param.STD);
			DBCon->GetCollect("Freq",param.Freq);
			DBCon->GetCollect("SymbolRate",param.SymbolRate);
			DBCon->GetCollect("DeviceID",param.DeviceID);
			DBCon->GetCollect("ChannelID",param.ChannelID);
			DBCon->GetCollect("Duration",param.Duration);
			DBCon->GetCollect("Num",param.Num);
			DBCon->GetCollect("TimeInterval",param.TimeInterval);
			DBCon->GetCollect("DownThreshold",param.DownThreshold);
			DBCon->GetCollect("UpThreshold",param.UpThreshold);
			DBCon->GetCollect("Switch",param.Switch);

			AlarmParamVec.push_back( param );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}


bool SqliteDBConnection::QueryAlarmParam(std::vector<sAlarmParam>& AlarmParamVec)
{
	std::string sql="select * from AlarmParam";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			printf("\n SqliteDBConnection::QueryAlarmParam() : ");
			const char* errMsg = DBCon->ErrorMessage();
			printf(errMsg);
			printf("\n");
			return false;
		}
		DBCon->MoveFirst();

		while (DBCon->IsEOF() != LSQL_OK)
		{
			sAlarmParam param;

			string dvbtype,alarmtype;
			DBCon->GetCollect("DVBType", dvbtype);
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
			DBCon->GetCollect("AlarmType", alarmtype);
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
			DBCon->GetCollect("TypeID",param.TypeID);
			DBCon->GetCollect("TypeDesc",param.TypeDesc);
			DBCon->GetCollect("STD",param.STD);
			DBCon->GetCollect("Freq",param.Freq);
			DBCon->GetCollect("SymbolRate",param.SymbolRate);
			DBCon->GetCollect("DeviceID",param.DeviceID);
			DBCon->GetCollect("ChannelID",param.ChannelID);
			DBCon->GetCollect("Duration",param.Duration);
			DBCon->GetCollect("Num",param.Num);
			DBCon->GetCollect("TimeInterval",param.TimeInterval);
			DBCon->GetCollect("DownThreshold",param.DownThreshold);
			DBCon->GetCollect("UpThreshold",param.UpThreshold);
			DBCon->GetCollect("Switch",param.Switch);

			AlarmParamVec.push_back( param );
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}



bool SqliteDBConnection::AddRunPlanParam( sRunPlanParam runPlanParam )
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
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecNonQuery(sql.c_str()) == LSQL_OK )		//添加运行图参数
			ret = true;
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::AddRunPlanParam() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加运行图参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


/**	channelid不会用到了
*/
bool SqliteDBConnection::DeleteRunPlanParam(enumDVBType type, std::string channelid )
{
	bool ret=false;
	std::string sql="delete from runplanparam where dvbtype='"+StrUtil::Int2Str(type)+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int result=DBCon->ExecNonQuery(sql.c_str());//删除运行图参数
		if (result == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::DeleteRunPlanParam() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除运行图参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::UpdateRunPlanParam(enumDVBType dvbtype,std::string channelid,std::vector<sRunPlanParam> runPlanParam )
{
	ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
	if (runPlanParam.size()>0)
		DeleteRunPlanParam(dvbtype,channelid);//删除运行图参数

	for (size_t i=0;i!=runPlanParam.size();++i)
	{
		AddRunPlanParam(runPlanParam[i]);//添加运行图参数
	}
	
	return true;
}


bool SqliteDBConnection::AddQualityInfo( enumDVBType dvbtype,std::string freq,std::string taskid,eQualityInfo quality ,std::string checktime)
{
	bool ret=false;
	string sql="insert into quality(dvbtype,freq,taskid,type,des,valu,checktime,reporttime) values('"+StrUtil::Int2Str(dvbtype)+"','";
	sql+=freq+"','";
	sql+=taskid+"','";
	sql+=quality.type+"','";
	sql+=quality.desc+"','";
	sql+=quality.valu+"','";
	sql+=checktime+"','')";		//reporttime为空
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if (DBCon->ExecNonQuery(sql.c_str()) == LSQL_OK)		//添加指标信息
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::AddQualityInfo() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加指标信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}




bool SqliteDBConnection::GetMsgID( std::string &MsgID )
{
	bool ret = false;
	string sql="select msgid from MsgID ";

	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) == LSQL_OK)
		{
			DBCon->GetCollect("MsgID", MsgID);
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::GetMsgID() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询MsgID出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}

bool SqliteDBConnection::UpdateMsgID( std::string MsgID )
{
	bool ret=false;
	string sql="update Msgid set msgid='"+MsgID+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int retValue = DBCon->ExecNonQuery(sql.c_str());//更新MsgID信息
		if (retValue == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::UpdateMsgID() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新MsgID信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::QueryCompensationValu( std::vector<sQualityCompensation>& qualityCmpVec)
{
	string sql="select * from qualitycompensation";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int ret = DBCon->ExecSql( sql.c_str() );
		if (ret != LSQL_OK)
		{
			printf("\n SqliteDBConnection::QueryCompensationValu() : ");
			const char* errMsg = DBCon->ErrorMessage();
			printf(errMsg);
			printf("\n");
			return false;
		}

		while ( DBCon->IsEOF() != LSQL_OK)
		{
			sQualityCompensation quality;

			string dvbtype;
			DBCon->GetCollect("DVBType",dvbtype);
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
			DBCon->GetCollect("DeviceID",quality.deviceid);
			DBCon->GetCollect("Type",quality.type);
			DBCon->GetCollect("Valu",quality.valu);
			qualityCmpVec.push_back(quality);
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询指标补偿值出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}



bool SqliteDBConnection::UpdateCompensationValu(sQualityCompensation& qualityCmp)
{
	ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
	bool ret = DelCompensationValu(qualityCmp)&&AddCompensationValu(qualityCmp);
	
	return ret;
}

bool SqliteDBConnection::AddCompensationValu(sQualityCompensation& qualityCmp)
{
	bool ret=false;
	string sql="insert into qualitycompensation(DVBType,DeviceID,Type,Valu) values('";
	sql+=StrUtil::Int2Str(qualityCmp.dvbtype)+"','";
	sql+=qualityCmp.deviceid+"','";
	sql+=qualityCmp.type+"','";
	sql+=qualityCmp.valu+"')";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int retValue = DBCon->ExecNonQuery(sql.c_str());
		if (retValue == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::AddCompensationValu() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加指标补偿信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}



bool SqliteDBConnection::DelCompensationValu(sQualityCompensation& qualityCmp)
{
	bool ret=false;
	string sql="delete from qualitycompensation where dvbtype='";
	sql+=string(StrUtil::Int2Str(qualityCmp.dvbtype)+"' and deviceid='");
	sql+=string(qualityCmp.deviceid+"' and type='");
	sql+=string(qualityCmp.type+"'");
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int retValue = DBCon->ExecNonQuery(sql.c_str());
		if (retValue == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::DelCompensationValu() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{ 
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除指标补偿信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::AddAlarmInfo( sCheckParam& alarminfo,string mode )
{
	bool ret=false;
	string sql="insert into alarminfo(dvbtype,alarmtype,std,freq,symbolrate,deviceid,channelid,typeid,typedesc,alarmvalue,starttime,endtime,alarmid,mode) values('";
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
	sql+=string("','");
	sql+=string(alarminfo.AlarmID+"','");
	sql+=string(mode+"')");
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int retValue = DBCon->ExecNonQuery(sql.c_str());
		if (retValue == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::AddAlarmInfo() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加报警结果出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}



bool SqliteDBConnection::QueryAlarmInfo( string mode,std::vector<sCheckParam>& vecAlarmInfo,bool bRadioNosignal)
{
	string sql="select * from alarminfo where mode='";
	sql+=mode;
	sql+="'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int ret = DBCon->ExecSql( sql.c_str() );
		if (ret != LSQL_OK)
		{
			printf("\n SqliteDBConnection::QueryAlarmInfo() : ");
			const char* errMsg = DBCon->ErrorMessage();
			printf(errMsg);
			printf("\n");
			return false;
		}

		while (DBCon->IsEOF() != LSQL_OK)
		{
			sCheckParam alarm;

			string dvbtype;
			DBCon->GetCollect("DVBType",dvbtype);
			alarm.DVBType=(enumDVBType)StrUtil::Str2Int(dvbtype);

			DBCon->GetCollect("STD",alarm.STD);
			DBCon->GetCollect("Freq",alarm.Freq);
			DBCon->GetCollect("SymbolRate",alarm.SymbolRate);
			DBCon->GetCollect("ChannelID",alarm.ChannelID);
			DBCon->GetCollect("TypeID",alarm.TypeID);
			DBCon->GetCollect("TypeDesc",alarm.TypeDesc);
			DBCon->GetCollect("AlarmValue",alarm.TypedValue);
			alarm.CheckTime = TimeUtil::StrToDateTime( TimeUtil::GetCurDateTime() );
			DBCon->GetCollect("AlarmID",alarm.AlarmID);
			DBCon->GetCollect("DeviceID",alarm.DeviceID);
			string alarmtype;
			DBCon->GetCollect("AlarmType",alarmtype);
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
			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警结果信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}



bool SqliteDBConnection::UpdateAlarmInfo( eDVBType dvbtype,std::string Alarmid,string mode,string unAlarmTime)
{
	bool ret=false;

	string sql="update alarminfo set endtime='" + unAlarmTime + "' , mode='" + mode;
	sql+="' where dvbtype='"+StrUtil::Int2Str(dvbtype);
	sql+="' and alarmid='"+Alarmid;
	sql+="'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int retValue = DBCon->ExecNonQuery(sql.c_str());
		if (retValue == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::UpdateAlarmInfo() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}



bool SqliteDBConnection::QueryFreqAlarmID( eDVBType dvbtype,string deviceid,string freq,string typeID,string& alarmid )
{
	bool ret = false;
	string sql="select alarmid from alarminfo where dvbtype='";
	sql+=StrUtil::Int2Str(dvbtype)+"' and deviceid='";
	sql+=deviceid+"' and freq='";
	sql+=freq+"' and typeid='";
	sql+=typeID+"' and mode='";
	sql+="0'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if (DBCon->ExecSql(sql.c_str()) == LSQL_OK)//查询指标信息
		{
			while(DBCon->IsEOF() != LSQL_OK)
			{
				DBCon->GetCollect("AlarmID",alarmid);
				DBCon->MoveNext();
			}
			ret=true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::QueryFreqAlarmID() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警结果信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::QueryProgramAlarmID( eDVBType dvbtype,string deviceid,string channelid,string typeID,string& alarmid )
{
	bool ret=false;
	string sql="select alarmid from alarminfo where dvbtype='";
	sql+=StrUtil::Int2Str(dvbtype)+"' and deviceid='";
	sql+=deviceid+"' and channelid='";
	sql+=channelid+"' and typeid='";
	sql+=typeID+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if (DBCon->ExecSql(sql.c_str()) == LSQL_OK)//查询指标信息
		{
			while(DBCon->IsEOF() != LSQL_OK)
			{
				DBCon->GetCollect("AlarmID",alarmid);
				DBCon->MoveNext();
			}
			ret=true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::QueryProgramAlarmID() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警结果信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqliteDBConnection::UpdateFileEndTime( std::string filename,std::string endtime )
{
	//printf("SqliteDBConnection::UpdateFileEndTime() !!!\n");
	bool ret=false;
	string sql="update record set endtime='"+endtime;
	sql+="' where filename='";
	sql+=filename;
	sql+="'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int retValue=DBCon->ExecNonQuery(sql.c_str());
		if (retValue == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::UpdateFileEndTime() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqliteDBConnection::AddSpectrumInfo( eDVBType dvbtype,std::string freq,std::string taskid,eSpecInfo spec ,std::string checktime)
{
	bool ret=false;
	string sql="insert into spec(dvbtype,freq,taskid,type,des,valu,status,checktime,reporttime) values('"+StrUtil::Int2Str(dvbtype)+"','";
	sql+=freq+"','";
	sql+=taskid+"','";
	sql+=spec.type+"','";
	sql+=spec.desc+"','";
	sql+=spec.valu+"','";
	sql+=spec.status+"','";
	sql+=checktime+"','')";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int retValue = DBCon->ExecNonQuery(sql.c_str());//添加指标信息
		if (retValue == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::AddSpectrumInfo() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加频谱信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::QuerySpecInfo( eDVBType dvbtype,std::string taskid,std::vector<eSpecInfo>& vecSpec,std::string startdatetime,std::string enddatetime )
{
	bool ret=false;
	string sql="select * from spec where dvbtype='"+StrUtil::Int2Str(dvbtype);
	sql+="' and taskid='";
	sql+=taskid+"' and checktime>='";
	sql+=startdatetime+"' and checktime<='";
	sql+=enddatetime+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if (DBCon->ExecSql(sql.c_str()) == LSQL_OK)//查询指标信息
		{
			while(DBCon->IsEOF() != LSQL_OK)
			{
				//获得指标信息
				eSpecInfo spec;
				DBCon->GetCollect("Type",spec.type);
				DBCon->GetCollect("Des",spec.desc);
				DBCon->GetCollect("Valu",spec.valu);
				DBCon->GetCollect("CheckTime",spec.checktime);
				DBCon->GetCollect("Freq",spec.freq);
				DBCon->GetCollect("Status",spec.status);
				DBCon->MoveNext();
				vecSpec.push_back(spec);
			}
			ret=true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::QuerySpecInfo() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询频谱出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqliteDBConnection::AddTempFile(string filename, string createtime, string expiredays)
{
	bool ret=false;
	string sql="insert into TempFile(FileName,CreateTime,ExpireDays) values('";
	sql += string(filename + "','");
	sql += string(createtime + "','");
	sql += string(expiredays + "')");

	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int retValue = DBCon->ExecNonQuery(sql.c_str());
		if (retValue == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::AddTempFile() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqliteDBConnection::DeleteTempFile( std::string filename )
{
	bool ret=false;
	string sql="delete from TempFile where FileName='"+filename+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int retValue = DBCon->ExecNonQuery(sql.c_str());//执行删除临时文件sql
		if (retValue == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::DeleteTempFile() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool SqliteDBConnection::QueryTempFileByDays(unsigned int days,std::vector<std::string>& vecFilename)
{
	bool ret=false;
	string sql="select filename from TempFile where ExpireDays='"+StrUtil::Int2Str(days)+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if (DBCon->ExecSql(sql.c_str()) == LSQL_OK)//执行临时文件查询sql
		{
			while (DBCon->IsEOF() != LSQL_OK)
			{
				string file;
				DBCon->GetCollect("FileName",file);//获得临时文件名
				vecFilename.push_back(file);
				DBCon->MoveNext();//移动到下一条记录
			}
			ret=true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::QueryTempFileByDays() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::QueryTempFileTop(unsigned int count,std::vector<std::string>& vecFilename)
{
	bool ret=false;
	string sql = "select * from TempFile order by CreateTime limit "+StrUtil::Int2Str(count);

	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if (DBCon->ExecSql(sql.c_str()) == LSQL_OK)//执行临时文件查询sql
		{
			while (DBCon->IsEOF() != LSQL_OK)
			{
				string file;
				DBCon->GetCollect("FileName",file);//获得临时文件名
				vecFilename.push_back(file);
				DBCon->MoveNext();//移动到下一条记录
			}
			ret=true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::QueryTempFileTop() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{		
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::AlarmInfoQuery(vector<sCheckParam> &vecAlarm)
{
	ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
	vector<sCheckParam>::iterator checkParamIter=vecAlarm.begin();
	for(;checkParamIter!=vecAlarm.end();checkParamIter++)
	{
		bool ret=false;
		string sql = "select starttime,endtime,mode from alarminfo where DVBType='"+StrUtil::Int2Str(checkParamIter->DVBType)+"' and AlarmType='"+StrUtil::Int2Str(checkParamIter->AlarmType);
		sql+="' and Freq='"+checkParamIter->Freq+"' and TypeID='"+checkParamIter->TypeID+"' and ChannelID='"+checkParamIter->ChannelID+"'";

		try
		{
			if (DBCon->ExecSql(sql.c_str()) == LSQL_OK)
			{
				int rowcount=0;
				rowcount=DBCon->GetRowCount();
				if(rowcount==0)
				{
					checkParamIter->StrCheckTime=TimeUtil::DateTimeToStr(time(0));
					checkParamIter->mode="0";
				}
				while (DBCon->IsEOF() != LSQL_OK)
				{
					string mode,starttime,endtime;
					DBCon->GetCollect("StartTime",starttime);
					DBCon->GetCollect("EndTime",endtime);
					DBCon->GetCollect("Mode",mode);
					if(mode.substr(0,1)=="0")
					{
						checkParamIter->StrCheckTime=starttime;
						checkParamIter->mode="1";
					}
					else if(mode.substr(0,1)=="1")
					{
						checkParamIter->StrCheckTime=endtime;
						checkParamIter->mode="2";
					}
					DBCon->MoveNext();
				}
			}
			else
			{
				const char* errMsg = DBCon->ErrorMessage();
				printf("\n SqliteDBConnection::AlarmInfoQuery() : ");
				printf(errMsg);
				printf("\n");
			}
		}
		catch (...)
		{
			//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时信息出错！",LOG_OUTPUT_FILE);
			continue;
		}
	}
	return true;
}


bool SqliteDBConnection::AlarmStateClear(string Freq)
{
	bool ret=false;
	string sql="";
	if(Freq=="ALL")
		sql="delete from alarminfo where Freq<>''";
	else
		sql="delete from alarminfo where Freq='"+Freq+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		int retValue = DBCon->ExecNonQuery(sql.c_str());
		if (retValue == LSQL_OK)
		{
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::AlarmStateClear() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"报警状态删除信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::UpdateAlarmRecordExpireTime(eDVBType dvbtype,std::string Alarmid)
{
	bool ret=false;
	string sql="select channelid,starttime,endtime from alarminfo where alarmid='"+Alarmid+"'";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if (DBCon->ExecSql(sql.c_str()) == LSQL_OK)//查询sql
		{
			while (DBCon->IsEOF() != LSQL_OK)
			{
				string channelid,starttime,endtime;
				DBCon->GetCollect("ChannelID",channelid);
				DBCon->GetCollect("StartTime",starttime);
				DBCon->GetCollect("EndTime",endtime);
				string days="5";/*PROPMANAGER::instance()->GetAlarmRecordExpire(dvbtype)*/
				string updatesql="update record set expiredays='"+days+"' where dvbtype='"+StrUtil::Int2Str(dvbtype);
				updatesql+="' and channelid='"+channelid;
				updatesql+="' and starttime<='"+endtime;
				updatesql+="' and endtime>='"+starttime+"'";
				try
				{
					int retValue = DBCon->ExecNonQuery(updatesql.c_str());
					if (retValue == LSQL_OK)
					{
						ret = true;
					}
				}
				catch (...)
				{
					//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新异态录像过期时间出错！",LOG_OUTPUT_FILE);
				}
				DBCon->MoveNext();//移动到下一条记录
			}
			ret=true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::UpdateAlarmRecordExpireTime() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询异态报警信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;

}


bool SqliteDBConnection::QueryRunPlanParam( eDVBType type,std::vector<sRunPlanParam>& runPlanParam )
{
	std::string sql="select * from runplanparam where dvbtype='"+StrUtil::Int2Str(type)+"' order by dvbtype,channelid";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) != LSQL_OK)
		{
			printf("\n SqliteDBConnection::QueryRunPlanParam() : ");
			const char* errMsg = DBCon->ErrorMessage();
			printf(errMsg);
			printf("\n");
			return false;
		}
		DBCon->MoveFirst();

		while (DBCon->IsEOF() != LSQL_OK)
		{
			sRunPlanParam param;
			param.dvbtype=type;
			string runplanType;
			DBCon->GetCollect("Type", runplanType);
			param.Type = StrUtil::Str2Int(runplanType);
			DBCon->GetCollect("ChannelID",param.ChannelID);
			DBCon->GetCollect("DayOfWeek",param.DayOfWeek);
			DBCon->GetCollect("mMonth",param.Month);
			DBCon->GetCollect("DayofMonth",param.DayOfMonth);
			DBCon->GetCollect("StartTime",param.StartTime);
			DBCon->GetCollect("EndTime",param.EndTime);
			DBCon->GetCollect("ValidStartDateTime",param.ValidStartDateTime);
			DBCon->GetCollect("ValidEndDateTime",param.ValidEndDateTime);
			DBCon->GetCollect("StartDateTime",param.StartDateTime);
			DBCon->GetCollect("EndDateTime",param.EndDateTime);

			runPlanParam.push_back( param );

			DBCon->MoveNext();
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询运行图参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}



bool SqliteDBConnection::QueryAllRunPlanParam( std::vector<sRunPlanParam>& runPlanParam )
{
	bool ret = false;
	std::string sql="select * from runplanparam order by dvbtype,channelid";
	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if ( DBCon->ExecSql(sql.c_str()) == LSQL_OK)
		{
			DBCon->MoveFirst();

			while (DBCon->IsEOF() != LSQL_OK)
			{
				sRunPlanParam param;
				string Dvbtype, dbtype;
				DBCon->GetCollect("DVBType", Dvbtype);
				switch(StrUtil::Str2Int(Dvbtype))
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
				DBCon->GetCollect("Type", dbtype);
				param.Type = StrUtil::Str2Int(dbtype);
				DBCon->GetCollect("ChannelID",param.ChannelID);
				DBCon->GetCollect("DayOfWeek",param.DayOfWeek);
				DBCon->GetCollect("mMonth",param.Month);
				DBCon->GetCollect("DayofMonth",param.DayOfMonth);
				DBCon->GetCollect("StartTime",param.StartTime);
				DBCon->GetCollect("EndTime",param.EndTime);
				DBCon->GetCollect("ValidStartDateTime",param.ValidStartDateTime);
				DBCon->GetCollect("ValidEndDateTime",param.ValidEndDateTime);
				DBCon->GetCollect("StartDateTime",param.StartDateTime);
				DBCon->GetCollect("EndDateTime",param.EndDateTime);

				runPlanParam.push_back( param );

				DBCon->MoveNext();
			}
			ret = true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::QueryAllRunPlanParam() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询运行图参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}




bool SqliteDBConnection::QueryQualityInfo( enumDVBType dvbtype,std::string freq,std::string taskid,std::vector<eQualityInfo>& vecQuality,std::string startdatetime,std::string enddatetime )
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
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		if (DBCon->ExecSql(sql.c_str()) == LSQL_OK)//查询指标信息
		{
			while(DBCon->IsEOF() != LSQL_OK)
			{
				//获得指标信息
				eQualityInfo qulity;
				DBCon->GetCollect("Type",qulity.type);
				DBCon->GetCollect("Des",qulity.desc);
				DBCon->GetCollect("Valu",qulity.valu);
				DBCon->GetCollect("CheckTime",qulity.checktime);
				DBCon->GetCollect("Freq",qulity.freq);
				vecQuality.push_back(qulity);
				DBCon->MoveNext();
			}
			ret=true;
		}
		else
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::QueryQualityInfo() : ");
			printf(errMsg);
			printf("\n");
		}
	}
	catch (...)
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询指标出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}


bool SqliteDBConnection::QueryTempFilesExpire( std::vector<std::string>& vecFilename )
{
	bool ret=false;
	string curDateTime = TimeUtil::GetCurDateTime();

	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		/* 统计expiredays共有几个值 */
		std::vector<string> vecExpiredayNum;
		string foreSql = "select ExpireDays from TempFile";
		if (DBCon->ExecSql(foreSql.c_str()) == LSQL_OK)
		{
			while (DBCon->IsEOF() != LSQL_OK)
			{
				string expireDays;
				DBCon->GetCollect("ExpireDays", expireDays);

				if (find(vecExpiredayNum.begin(), vecExpiredayNum.end(), expireDays)
					== vecExpiredayNum.end())
				{
					vecExpiredayNum.push_back( expireDays );
				}
				DBCon->MoveNext();
			}
			ret = true;
		}
		else	//fail
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::QueryTempFilesExpire() : ");
			printf(errMsg);
			printf("\n");
			ret = false;
		}

		/* 获取过期的文件 */
		std::vector<string>::iterator expireIter = vecExpiredayNum.begin(); 
		for (;expireIter!=vecExpiredayNum.end(); expireIter++)
		{
			string expire = *expireIter;
			string sql = "select * from (select * from tempfile where expiredays='"+expire+"')";
			sql += "where datetime(CreateTime, '+"+expire+" day')<='";
			sql += curDateTime + "'";

			if (DBCon->ExecSql(sql.c_str()) == LSQL_OK)
			{
				while (DBCon->IsEOF() != LSQL_OK)
				{
					string file;
					DBCon->GetCollect("FileName",file);//获得临时文件的文件名
					vecFilename.push_back(file);
					DBCon->MoveNext();//移动到下一条记录
				}
				ret = true;
			}
			else
			{
				const char* errMsg = DBCon->ErrorMessage();
				printf("\n SqliteDBConnection::QueryTempFilesExpire() : ");
				printf(errMsg);
				printf("\n");
				ret = false;
			}
		}
	}
	catch ( ... )
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;	
	}
	return ret;
}


bool SqliteDBConnection::QueryRecordExpire( std::vector<std::string>& vecFilename )
{
	bool ret = false;
	string curDateTime = TimeUtil::GetCurDateTime();

	try
	{
		ACE_Guard<ACE_Thread_Mutex> guard(m_sqliteMutex);
		/* 统计expiredays共有几个值 */
		std::vector<string> vecExpiredayNum;
		string foreSql = "select ExpireDays from record";
		if (DBCon->ExecSql(foreSql.c_str()) == LSQL_OK)
		{
			while (DBCon->IsEOF() != LSQL_OK)
			{
				string expireDays;
				DBCon->GetCollect("ExpireDays", expireDays);

				if (find(vecExpiredayNum.begin(), vecExpiredayNum.end(), expireDays)
					== vecExpiredayNum.end())
				{
					vecExpiredayNum.push_back( expireDays );
				}
				DBCon->MoveNext();
			}
			ret = true;
		}
		else	//fail
		{
			const char* errMsg = DBCon->ErrorMessage();
			printf("\n SqliteDBConnection::QueryRecordExpire() : ");
			printf(errMsg);
			printf("\n");
			ret = false;
		}

		/* 获取过期的文件 */
		std::vector<string>::iterator expireIter = vecExpiredayNum.begin(); 
		for (;expireIter!=vecExpiredayNum.end(); expireIter++)
		{
			string expire = *expireIter;
			string sql = "select * from (select * from record where expiredays='"+expire+"')";
			sql += "where datetime(StartTime, '+"+expire+" day')<='";
			sql += curDateTime + "'";

			if (DBCon->ExecSql(sql.c_str()) == LSQL_OK)
			{
				while (DBCon->IsEOF() != LSQL_OK)
				{
					string file;
					DBCon->GetCollect("FileName",file);//获得临时文件的文件名
					vecFilename.push_back(file);
					DBCon->MoveNext();//移动到下一条记录
				}
				ret = true;
			}
			else
			{
				const char* errMsg = DBCon->ErrorMessage();
				printf("\n SqliteDBConnection::QueryRecordExpire() : ");
				printf(errMsg);
				printf("\n");
				ret = false;
			}
		}
	}
	catch ( ... )
	{
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;	
	}
	return ret;
}
