#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <direct.h>	
#include <string>
#include <iostream>
using namespace std;
void ExecuteAndMessage(char * sql,CEditArea* editArea){//����ִ�е���������ڽ�������ʾִ�н�����˺������޸�
	std::string s_sql = sql;
	if(s_sql.find("select") == 0){
		SelResult res;
		Init_Result(&res);
		//rc = Query(sql,&res);
		//����ѯ�������һ�£����������������ʽ
		//����editArea->ShowSelResult(col_num,row_num,fields,rows);
		int col_num = 5;
		int row_num = 3;
		char ** fields = new char *[5];
		for(int i = 0;i<col_num;i++){
			fields[i] = new char[20];
			memset(fields[i],0,20);
			fields[i][0] = 'f';
			fields[i][1] = i+'0';
		}
		char *** rows = new char**[row_num];
		for(int i = 0;i<row_num;i++){
			rows[i] = new char*[col_num];
			for(int j = 0;j<col_num;j++){
				rows[i][j] = new char[20];
				memset(rows[i][j],0,20);
				rows[i][j][0] = 'r';
				rows[i][j][1] = i + '0';
				rows[i][j][2] = '+';
				rows[i][j][3] = j + '0';
			}
		}
		editArea->ShowSelResult(col_num,row_num,fields,rows);
		for(int i = 0;i<5;i++){
			delete[] fields[i];
		}
		delete[] fields;
		Destory_Result(&res);
		return;
	}
	RC rc = execute(sql);
	int row_num = 0;
	char**messages;
	switch(rc){
	case SUCCESS:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "�����ɹ�";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "���﷨����";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	default:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "����δʵ��";
		editArea->ShowMessage(row_num,messages);
	delete[] messages;
		break;
	}
}

RC execute(char * sql){
	sqlstr *sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();
  	rc = parse(sql, sql_str);//ֻ�����ַ��ؽ��SUCCESS��SQL_SYNTAX
	RC tmp=FAIL;
	if (rc == SUCCESS)
	{
		int i = 0;
		switch (sql_str->flag)
		{
			case 1:
			//�ж�SQL���Ϊselect���
				AfxMessageBox("��ģʽ��֧��select��䣡");
				tmp=FAIL;
				break;

			case 2:
			//�ж�SQL���Ϊinsert���
				tmp=Insert(sql_str->sstr.ins.relName,sql_str->sstr.ins.nValues,sql_str->sstr.ins.values);
				break;

			case 3:	
			//�ж�SQL���Ϊupdate���
				tmp=Update(sql_str->sstr.upd.relName,sql_str->sstr.upd.attrName,
					&sql_str->sstr.upd.value,sql_str->sstr.upd.nConditions,sql_str->sstr.upd.conditions);
				break;

			case 4:					
			//�ж�SQL���Ϊdelete���
				tmp=Delete(sql_str->sstr.del.relName,sql_str->sstr.del.nConditions,sql_str->sstr.del.conditions);
				break;

			case 5:
			//�ж�SQL���ΪcreateTable���
				tmp=CreateTable(sql_str->sstr.cret.relName,sql_str->sstr.cret.attrCount,sql_str->sstr.cret.attributes);
				break;

			case 6:	
			//�ж�SQL���ΪdropTable���
				tmp=DropTable(sql_str->sstr.drt.relName);
				break;

			case 7:
			//�ж�SQL���ΪcreateIndex���
				tmp=_CreateIndex(sql_str->sstr.crei.indexName,sql_str->sstr.crei.relName,sql_str->sstr.crei.attrName);
				break;
	
			case 8:	
			//�ж�SQL���ΪdropIndex���
				tmp=DropIndex(sql_str->sstr.dri.indexName);
				break;
			
			case 9:
			//�ж�Ϊhelp��䣬���Ը���������ʾ

				break;
		
			case 10: 
			//�ж�Ϊexit��䣬�����ɴ˽����˳�����
				break;		
		}
	}else{
		AfxMessageBox(sql_str->sstr.errors);//���������sql���ʷ�����������Ϣ
		return rc;
	}
	return tmp;
}

RC CreateTable(char *relName, int attrCount, AttrInfo *attributes)
{
	int attroffset=0;
	int charmount=0;
	RM_FileHandle *table_filehandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	table_filehandle->bOpen=false;
	RM_FileHandle *column_filehandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	column_filehandle->bOpen=false;
	RID *column_rid=(RID*)malloc(sizeof(RID));
	column_rid->bValid=false;
	RID *table_rid=(RID*)malloc(sizeof(RID));
	column_rid->bValid=false;
	RC tmp;
	tmp=RM_OpenFile("SYSCOLUMNS.xx",column_filehandle);
	if(tmp!=SUCCESS){
		AfxMessageBox("ϵͳ���ļ���ʧ��");
		return tmp;
	}
	for(int i=0;i<attrCount;i++)
	{
		char column[76];
		char ix_flag='0';
		strcpy(column,relName);
		strcpy(column+21,attributes[i].attrName);
		memcpy(column+42,&attributes[i].attrType,4);
		memcpy(column+46,&attributes[i].attrLength,4);
		memcpy(column+50,&attroffset,4);
		memcpy(column+54,&ix_flag,1);
		tmp=InsertRec(column_filehandle,column,column_rid);
		if (tmp!=SUCCESS)
		{
			AfxMessageBox("�����м�¼ʧ��");
			return tmp;
		}
		if(attributes[i].attrType==ints)
		{
			attroffset=attroffset+sizeof(int);
		}
		if(attributes[i].attrType==chars)
		{
			charmount++;
			attroffset=attroffset+21;
		}
		if(attributes[i].attrType==floats)
		{
			attroffset=attroffset+sizeof(float);
		}
	}
	tmp=RM_CloseFile(column_filehandle);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�ر����ļ�ʧ��");
		return tmp;
	}
	tmp=RM_CreateFile(relName,attroffset);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�������ļ�ʧ��");
		return tmp;
	}
	//char* table=relName+attrCount;
	tmp=RM_OpenFile("SYSTABLES.xx",table_filehandle);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�����ļ�ʧ��");
		return tmp;
	}
	char *table=(char*)malloc(sizeof(char)*25);
	strcpy(table,relName);
	memcpy(table+21,&attrCount,sizeof(int));
	tmp=InsertRec(table_filehandle,table,table_rid);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�����м�¼ʧ��");
		return tmp;
	}
	tmp=RM_CloseFile(table_filehandle);
	if(tmp!=SUCCESS){
		AfxMessageBox("�ر����ļ�ʧ��");
		return tmp;
	}
	free(table);
	free(table_filehandle);
	free(column_filehandle);
	free(table_rid);
	free(column_rid);
	return SUCCESS;
}


RC DropTable(char *relName)
{
	remove(relName);//ɾ���ñ�ļ�¼�ļ�
	//����Ҫɾ��SYSTABLES��SYSTABLES�еĶ�Ӧ������
	RM_FileHandle *table_filehandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	RM_FileHandle *column_filehandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	RM_FileScan *table_mfilescan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
	RM_FileScan *column_mfilescan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
	RM_Record *table_rec=(RM_Record*)malloc(sizeof(RM_Record));
	RM_Record *column_rec=(RM_Record*)malloc(sizeof(RM_Record));
	RC tmp;
	int attrcount=0;
	tmp=RM_OpenFile("SYSTABLES.xx",table_filehandle);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�����ļ�ʧ��");
		return tmp;
	}
	tmp=RM_OpenFile("SYSCOLUMNS.xx",column_filehandle);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�����ļ�ʧ��");
		return tmp;
	}
	Con *con=(Con*)malloc(sizeof(Con));
	con->bLhsIsAttr=1;
	con->bRhsIsAttr=0;
	con->attrType=chars;
	con->LattrLength=21;
	con->LattrOffset=0;
	con->compOp=EQual;
	strcmp((char*)con->Rvalue,relName);
	tmp=OpenScan(table_mfilescan,table_filehandle,1,con);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�����ļ�ɨ��ʧ��");
		return tmp;
	}
	tmp=GetNextRec(table_mfilescan,table_rec);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�Ҳ������������ļ�¼");
		return tmp;
	}
	memcpy(&attrcount,table_rec->pData+21,sizeof(int));
	tmp=DeleteRec(table_filehandle,table_rec->rid);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("ɾ�����ļ���¼ʧ��");
		return tmp;
	}
	tmp=OpenScan(column_mfilescan,column_filehandle,1,con);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�����ļ�ɨ��ʧ��");
		return tmp;
	}
	for (int i=0;i<attrcount;i++)
	{
		tmp=GetNextRec(column_mfilescan,column_rec);
		if (tmp!=SUCCESS)
		{
			AfxMessageBox("�Ҳ����ʺ������ļ�¼");
			return tmp;
		}
		tmp=DeleteRec(column_filehandle,column_rec->rid);
		if (tmp!=SUCCESS)
		{
			AfxMessageBox("ɾ�����ļ���¼ʧ��");
			return tmp;
		}
	}
	tmp=RM_CloseFile(column_filehandle);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�ر����ļ�ʧ��");
		return tmp;
	}
	tmp=RM_CloseFile(table_filehandle);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�ر����ļ�ʧ��");
		return tmp;
	}
	tmp=CloseScan(table_mfilescan);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�ر����ļ�ɨ��ʧ��");
		return tmp;
	}
	tmp=CloseScan(column_mfilescan);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�ر����ļ�ɨ��ʧ��");
		return tmp;
	}
	free(con);
	free(table_mfilescan);
	free(table_filehandle);
	free(table_rec);
	free(column_mfilescan);
	free(column_filehandle);
	free(column_rec);
	return SUCCESS;
}

RC CreateDB(char *dbpath,char *dbname)
{
	RC rc1,rc2;
	//����ϵͳ���ļ���ϵͳ���ļ�
	//SetCurrentDirectory(dbpath);
	string path=dbpath;
	string systable=path+"\\SYSTABLES.xx";
	string syscolumns=path+"\\SYSCOLUMNS.xx";
	rc1 = RM_CreateFile((char*)systable.c_str(),25);
	rc2 = RM_CreateFile((char*)syscolumns.c_str(),76);
	if (rc1 != SUCCESS || rc2!=SUCCESS)
	{
		AfxMessageBox("����ϵͳ�ļ�ʧ�ܣ�");
		return rc1==SUCCESS?rc1:rc2;
	}
	return SUCCESS;
}
void deleteFile(char* dbName)
{
	CFileFind finder;
    CString path;
    path.Format("%s/*.*",dbName);
    BOOL bWorking = finder.FindFile(path);
    while(bWorking)
    {
        bWorking = finder.FindNextFile();
        if(finder.IsDirectory() && !finder.IsDots())//�����ļ���
        {
            deleteFile(finder.GetFilePath().GetBuffer(0));//�ݹ�ɾ���ļ���
            if(!RemoveDirectory(finder.GetFilePath()))
			{
				AfxMessageBox(finder.GetFilePath());
			}
        }
        else//�����ļ�
        {
            DeleteFile(finder.GetFilePath());
        }
    }
}
RC DropDB(char *dbname){
	deleteFile(dbname);
	if(!RemoveDirectory(dbname))
	{
		AfxMessageBox("ɾ��ʧ��");
	}
	return SUCCESS;
}

RC OpenDB(char *dbname){
	SetCurrentDirectory(dbname);
	return SUCCESS;
}


RC CloseDB(){
	return SUCCESS;
}

bool CanButtonClick(){//��Ҫ����ʵ��
	//�����ǰ�����ݿ��Ѿ���
	return true;
	//�����ǰû�����ݿ��
	//return false;
}

/*�ú����ڹ�ϵrelName������attrName�ϴ�����ΪindexName��������
�������ȼ���ڱ���������Ƿ��Ѿ�����һ��������������ڣ��򷵻�һ������Ĵ����롣
���򣬴��������������������Ĺ����������ٴ������������ļ��������ɨ�豻�����ļ�¼�����������ļ��в���������۹ر�������*/
RC _CreateIndex(char *indexName,char *relName, char *attrName)
{
	RC tmp;
	RM_FileHandle* colHandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	colHandle->bOpen=false;
	RM_FileScan* fileScan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
	fileScan->bOpen=false;

	//�����ļ�
	tmp=RM_OpenFile("SYSCOLUMNS.xx",colHandle);
	if(tmp!=SUCCESS)
		return tmp;
	//����ɨ������
	Con* con=(Con*)malloc(sizeof(Con)*2);
	con->attrType=chars;
	con->bLhsIsAttr=1;
	con->bRhsIsAttr=0;
	con->compOp=EQual;
	con->LattrLength=21;
	con->LattrOffset=0;
	con->Rvalue=relName;

	(con+1)->attrType=chars;
	(con+1)->bLhsIsAttr=1;
	(con+1)->bRhsIsAttr=0;
	(con+1)->compOp=EQual;
	(con+1)->LattrLength=21;
	(con+1)->LattrOffset=21;
	(con+1)->Rvalue=attrName;

	OpenScan(fileScan,colHandle,2,con);

	RM_Record* colRec=(RM_Record*)malloc(sizeof(RM_Record));
	colRec->bValid=false;
	tmp=GetNextRec(fileScan,colRec); //ɨ��һ�����������ļ�¼
	if(tmp!=SUCCESS) //δ�ҵ�
	{
		cout<<"cannot find column record:table name:"<<relName<<"attr name:"<<attrName<<endl;
		return FLIED_NOT_EXIST;
	}
	if(*(colRec->pData+52)!='0')  //���������Ѿ�������
	{
		return INDEX_EXIST;
	}
	*(colRec->pData+54)='1';  //���Ϊ������
	memset(colRec->pData+55,'\0',21);
	strcpy(colRec->pData+55,indexName);

	UpdateRec(colHandle,colRec); //���¼�¼�м�¼�ļ�

	CloseScan(fileScan);
	free(fileScan);
	RM_CloseFile(colHandle);
	free(colHandle);
	free(con);

	//���������ļ�
	AttrType* attrType=(AttrType*)malloc(sizeof(int));
	memcpy(attrType,colRec->pData+42,sizeof(int));
	int attrLength;
	memcpy(&attrLength,colRec->pData+46,sizeof(int));
	CreateIndex(indexName,*attrType,attrLength);	
	free(attrType);

	//�������ļ�
	IX_IndexHandle* indexHandle=(IX_IndexHandle*)malloc(sizeof(IX_IndexHandle));
	indexHandle->bOpen=false;
	OpenIndex(indexName,indexHandle);

	//ɨ�����еļ�¼���������ļ�¼���������д�������ļ�
	RM_FileHandle* recHandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	recHandle->bOpen=false;
	RM_FileScan* recScan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
	recScan->bOpen=false;
	RM_Record* rec=(RM_Record*)malloc(sizeof(RM_Record));
	rec->bValid=false;
	RM_OpenFile(relName,recHandle);
	OpenScan(recScan,recHandle,0,NULL);
	int attrOffset;
	memcpy(&attrOffset,colRec->pData+42+sizeof(int)*2,sizeof(int));
	free(colRec);
	char* attrValue=(char*)malloc(attrLength);
	//����������
	while(SUCCESS==GetNextRec(recScan,rec))
	{
		memcpy(attrValue,rec->pData+attrOffset,attrLength);
		InsertEntry(indexHandle,attrValue,rec->rid);
	}
	free(attrValue);
	CloseScan(recScan);
	RM_CloseFile(recHandle);
	CloseIndex(indexHandle);
	free(indexHandle);
	free(recScan);
	free(recHandle);
	return SUCCESS;
}
RC DropIndex (char *indexName)
{
	RC tmp;
	IX_IndexHandle* indexHandle=(IX_IndexHandle*)malloc(sizeof(IX_IndexHandle));
	indexHandle->bOpen=false;
	tmp=OpenIndex(indexName,indexHandle);
	if(tmp!=SUCCESS) //����������
	{
		return INDEX_NOT_EXIST;
	}
	CloseIndex(indexHandle);
	free(indexHandle);

	RM_FileHandle* colHandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	colHandle->bOpen=false;
	RM_FileScan* recScan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
	recScan->bOpen=false;
	RM_Record* localRec=(RM_Record*)malloc(sizeof(localRec));
	localRec->bValid=false;
	tmp=RM_OpenFile("SYSCOLUMNS.xx",colHandle);
	if(tmp!=SUCCESS)
	{
		AfxMessageBox("ϵͳ���ļ���ʧ��");
		return tmp;
	}
	//����ɨ������
	Con* con=(Con*)malloc(sizeof(Con));
	con->attrType=chars;
	con->bLhsIsAttr=1;
	con->bRhsIsAttr=0;
	con->compOp=EQual;
	con->LattrLength=21;
	con->LattrOffset=42+3*sizeof(int)+sizeof(char);
	con->Rvalue=indexName;
	tmp=OpenScan(recScan,colHandle,1,con);
	if(SUCCESS!=tmp)
	{
		AfxMessageBox("ϵͳ���ļ�ɨ��ʧ��");
		return tmp;
	}
	tmp=GetNextRec(recScan,localRec);
	if(tmp==SUCCESS)
	{
		*(localRec->pData+42+3*sizeof(int))='0';
		memset(localRec->pData+42+4*sizeof(int)+sizeof(char),'0',21); //������ɾ��
		UpdateRec(colHandle,localRec);//���¼�¼
	}
	CloseScan(recScan);
	free(recScan);
	RM_CloseFile(colHandle);
	free(colHandle);
	free(localRec);
	free(con);
	DeleteFile(indexName);
	return SUCCESS;
}

RC Update(char *relName,char *attrName,Value *value,int nConditions,Condition *conditions)
{
	RC tmp;
	RM_FileHandle* tabHandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	tabHandle->bOpen=false;
	RM_FileScan* tabScan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
	tabScan->bOpen=false;
	RM_Record* tabRec=(RM_Record*)malloc(sizeof(RM_Record));
	tabRec->bValid=false;

	//���жϸñ��Ƿ����
	tmp=RM_OpenFile("SYSTABLES.xx",tabHandle);
	if(SUCCESS!=tmp)
	{
		AfxMessageBox("ϵͳ���ļ���ʧ��");
		return tmp;
	}
	Con* con=(Con*)malloc(sizeof(Con));
	con->attrType=chars;
	con->bLhsIsAttr=1;
	con->bRhsIsAttr=0;
	con->compOp=EQual;
	con->LattrLength=21;
	con->LattrOffset=0;
	con->Rvalue=relName;
	tmp=OpenScan(tabScan,tabHandle,1,con);
	if(SUCCESS!=tmp)
	{
		AfxMessageBox("ϵͳ���ļ�ɨ��ʧ��");
		return tmp;
	}
	tmp=GetNextRec(tabScan,tabRec);
	if(SUCCESS==tmp) //�����
	{
		RM_FileHandle* colHandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
		colHandle->bOpen=false;
		RM_FileScan* colScan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
		colScan->bOpen=false;
		RM_Record* colRec=(RM_Record*)malloc(sizeof(RM_Record));
		colRec->bValid=false;
		tmp=RM_OpenFile("SYSCOLUMNS.xx",colHandle);
		if(SUCCESS!=tmp)
		{
			AfxMessageBox("ϵͳ���ļ���ʧ��");
			return tmp;
		}
		con=(Con*)realloc(con,2*sizeof(Con));
		(con+1)->attrType=chars;
		(con+1)->bLhsIsAttr=1;
		(con+1)->bRhsIsAttr=0;
		(con+1)->compOp=EQual;
		(con+1)->LattrLength=21;
		(con+1)->LattrOffset=21;
		(con+1)->Rvalue=attrName;
		tmp=OpenScan(colScan,colHandle,2,con);
		if(SUCCESS!=tmp)
		{
			AfxMessageBox("ϵͳ���ļ�ɨ��ʧ��");
			return tmp;
		}
		tmp=GetNextRec(colScan,colRec);
		if(SUCCESS!=tmp)
		{
			AfxMessageBox("�����Բ����ڣ�");
			return tmp;
		}
		CloseScan(colScan);


		int attrLength,attrOffset,ifHasIndex;
		char* indexName=NULL;
		memset(indexName,0,21);
		memcpy(&attrLength,colRec->pData+42+sizeof(int),sizeof(int));
		memcpy(&attrOffset,colRec->pData+42+2*sizeof(int),sizeof(int));
		if(*(colRec->pData+42+3*sizeof(int))=='1') //�жϸ��������Ƿ�������
		{
			indexName=(char*)malloc(21);
			strcpy(indexName,colRec->pData+42+3*sizeof(int)+sizeof(char));
			ifHasIndex=1;
		}
		else ifHasIndex=0;
		//���ò�ѯ����
		Con* cons=(Con*)malloc(sizeof(Con)*nConditions);
		RM_Record* tmpRec=(RM_Record*)malloc(sizeof(RM_Record));
		for(int i=0;i<nConditions;++i)
		{
			if(conditions[i].bLhsIsAttr==1&&conditions[i].bRhsIsAttr==0)//����������ұ���ֵ
			{
				(con+1)->Rvalue=conditions[i].lhsAttr.attrName;
			}
			else if(conditions[i].bLhsIsAttr==0&&conditions[i].bRhsIsAttr==1)//�����ֵ�ұ�������
			{
				(con+1)->Rvalue=conditions[i].rhsAttr.attrName;
			}
			else if(conditions[i].bLhsIsAttr==1&&conditions[i].bRhsIsAttr==1)//���߶�������
			{
				(con+1)->Rvalue=conditions[i].lhsAttr.attrName;
				OpenScan(colScan,colHandle,2,con);
				GetNextRec(colScan,tmpRec);
				(con+1)->Rvalue=conditions[i].rhsAttr.attrName;
				CloseScan(colScan);
			}
			OpenScan(colScan,colHandle,2,con);
			GetNextRec(colScan,colRec);

			(cons+i)->bLhsIsAttr=conditions[i].bLhsIsAttr;
			(cons+i)->bRhsIsAttr=conditions[i].bRhsIsAttr;
			(cons+i)->compOp=conditions[i].op;
			if(conditions[i].bLhsIsAttr==1&&conditions[i].bRhsIsAttr==0)
			{
				memcpy(&(con+i)->LattrLength,colRec->pData+42+sizeof(int),sizeof(int));
				memcpy(&(con+i)->LattrOffset,colRec->pData+42+2*sizeof(int),sizeof(int));
				(con+i)->attrType=conditions[i].rhsValue.type;
				(con+i)->Rvalue=conditions[i].rhsValue.data;
			}
			else if(conditions[i].bLhsIsAttr==0&&conditions[i].bRhsIsAttr==1)
			{
				memcpy(&(con+i)->RattrLength,colRec->pData+42+sizeof(int),sizeof(int));
				memcpy(&(con+i)->RattrOffset,colRec->pData+42+2*sizeof(int),sizeof(int));
				(con+i)->attrType=conditions[i].lhsValue.type;
				(con+i)->Lvalue=conditions[i].lhsValue.data;
			}
			else if(conditions[i].bLhsIsAttr==1&&conditions[i].bRhsIsAttr==1)
			{
				memcpy(&(con+i)->LattrLength,tmpRec->pData+42+sizeof(int),sizeof(int));
				memcpy(&(con+i)->LattrOffset,tmpRec->pData+42+2*sizeof(int),sizeof(int));
				memcpy(&(con+i)->RattrLength,colRec->pData+42+sizeof(int),sizeof(int));
				memcpy(&(con+i)->RattrOffset,colRec->pData+42+2*sizeof(int),sizeof(int));
				memcpy(&(con+i)->attrType,colRec->pData+42,sizeof(int));
			}
			CloseScan(colScan);
		}
		RM_CloseFile(colHandle);
		free(colHandle);
		free(colScan);
		free(colRec);
		free(con);
		free(tmpRec);


		//�������ļ�
		RM_FileHandle* dataHandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
		dataHandle->bOpen=false;
		RM_FileScan* dataScan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
		dataScan->bOpen=false;
		RM_Record* dataRec=(RM_Record*)malloc(sizeof(RM_Record));
		dataRec->bValid=false;
		IX_IndexHandle* indexHandle=NULL;
		tmp=RM_OpenFile(relName,dataHandle);
		if(SUCCESS!=tmp)
		{
			AfxMessageBox("�����ļ���ʧ��");
			return tmp;
		}
		tmp=OpenScan(dataScan,dataHandle,nConditions,cons);
		if(SUCCESS!=tmp)
		{
			AfxMessageBox("�����ļ�ɨ��ʧ��");
			return tmp;
		}
		if(ifHasIndex)//���������
		{
			indexHandle=(IX_IndexHandle*)malloc(sizeof(IX_IndexHandle));
			OpenIndex(indexName,indexHandle);
		}
		char* oldValue=(char*)malloc(attrLength);
		while(GetNextRec(dataScan,dataRec)==SUCCESS)
		{	
			if(ifHasIndex)
			{
				memcpy(oldValue,dataRec->pData+attrOffset,attrLength);
				DeleteEntry(indexHandle,oldValue,dataRec->rid);
				InsertEntry(indexHandle,value,dataRec->rid);
			}
			memcpy(dataRec->pData+attrOffset,value,attrLength);
			UpdateRec(dataHandle,dataRec);
		}
		RM_CloseFile(dataHandle);
		CloseScan(dataScan);
		free(dataHandle);
		free(dataScan);
		free(dataRec);
		free(cons);
	}
	RM_CloseFile(tabHandle);
	CloseScan(tabScan);
	free(tabHandle);
	free(tabScan);
	free(tabRec);
	return SUCCESS;
}


RC Insert(char *relName,int nValues,Value *values)
{
	RC tmp;
	RM_FileHandle *filehandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	tmp=RM_OpenFile(relName,filehandle);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�򿪸ñ��ļ�ʧ��");
		return tmp;
	}
	char *pdata;
	int offset=0;
	RID *rid=(RID*)malloc(sizeof(RID));
	for (int i=0;i<nValues;i++)
	{
		switch(values[i].type){
		case ints:
			memcpy(pdata+offset,values[i].data,sizeof(int));
			offset=offset+sizeof(int);
			break;
		case floats:
			memcpy(pdata+offset,values[i].data,sizeof(float));
			offset=offset+sizeof(float);
			break;
		case chars:
			memcpy(pdata+offset,values[i].data,21);
			offset=offset+21;
			break;
		}
	}
	tmp=InsertRec(filehandle,pdata,rid);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�����¼ʧ��");
		return tmp;
	}
	free(filehandle);
	free(rid);
	RM_FileHandle *column_filehandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	RM_FileScan *column_filescan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
	RM_Record *column_record=(RM_Record*)malloc(sizeof(RM_Record));
	IX_IndexHandle *indexhandle=(IX_IndexHandle*)malloc(sizeof(IX_IndexHandle));
	//IX_IndexScan *indexscan=(IX_IndexScan*)malloc(sizeof(IX_IndexScan));
	tmp=RM_OpenFile("SYSCOLUMNS.XX",column_filehandle);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�����ļ�ʧ��");
		return tmp;
	}
	Con *con=(Con*)malloc(sizeof(Con));
	con->bLhsIsAttr=1;
	con->bRhsIsAttr=0;
	con->attrType=chars;
	con->LattrLength=21;
	con->LattrOffset=0;
	con->compOp=EQual;
	strcmp((char*)con->Rvalue,relName);
	tmp=OpenScan(column_filescan,column_filehandle,1,con);
	if (tmp!=SUCCESS)
	{
		AfxMessageBox("�����ļ�ɨ��ʧ��");
		return tmp;
	}
	for (int i=0;i<nValues;i++)
	{
		tmp=GetNextRec(column_filescan,column_record);
		if (tmp!=SUCCESS)
		{
			AfxMessageBox("��ȡ��¼ʧ��");
			return tmp;
		}
		char ix_flag='0';
		char* indexname=(char*)malloc(sizeof(char));
		//char* indexdata=(char*)malloc(sizeof(char));
		memcpy(&ix_flag,column_record->pData+54,sizeof(char));
		if (ix_flag=='1')
		{
			//values[i].data;
			memcpy(indexname,column_record->pData+55,21);
			tmp=OpenIndex(indexname,indexhandle);
			if (tmp!=SUCCESS)
			{
				AfxMessageBox("������ʧ��");
				return tmp;
			}
			tmp=InsertEntry(indexhandle,values[i].data,rid);
			if (tmp!=SUCCESS)
			{
				AfxMessageBox("���������ɹ�");
				return tmp;
			}
		}
		free(indexname);
	}
	free(column_filehandle);
	free(column_filescan);
	free(column_record);
	free(indexhandle);
	//free(indexscan);
	return SUCCESS;
}
RC Delete(char *relName,int nConditions,Condition *conditions)
{
	return SUCCESS;
}