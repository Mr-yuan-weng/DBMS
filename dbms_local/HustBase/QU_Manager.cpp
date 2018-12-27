#include "StdAfx.h"
#include "QU_Manager.h"
void Init_Result(SelResult * res){
	res->next_res = NULL;
}

void Destory_Result(SelResult * res){
	for(int i = 0;i<res->row_num;i++){
		for(int j = 0;j<res->col_num;j++){
			delete[] res->res[i][j];
		}
		delete[] res->res[i];
	}
	if(res->next_res != NULL){
		Destory_Result(res->next_res);
	}
}

RC Query(char * sql,SelResult * res)
{
	RC tmp;
	sqlstr * sqlType = NULL;
	sqlType = get_sqlstr();
	tmp = parse(sql, sqlType);
	if(SUCCESS!=tmp)
	{
		return tmp;
	}
	selects tmpSel=sqlType->sstr.sel;
	tmp=Select(tmpSel.nSelAttrs,tmpSel.selAttrs,tmpSel.nRelations,tmpSel.relations,
		tmpSel.nConditions,tmpSel.conditions,res);
	return tmp;
}
RC getAttrMap(int nSelAttrs,RelAttr **selAttrs,map<string,vector<AttrNode>> &attrMap)
{
	RM_FileHandle* tabHandle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	RM_FileScan* tabScan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
	RM_Record* tmpRec=(RM_Record*)malloc(sizeof(RM_Record));
	tmpRec->bValid=false;
	tabScan->bOpen=false;
	tabHandle->bOpen=false;
	RC tmp;
	Con cons[2];
	cons[0].attrType = chars;
	cons[0].bLhsIsAttr = 1;
	cons[0].LattrOffset = 0;
	cons[0].LattrLength = 21;
	cons[0].compOp = EQual;
	cons[0].bRhsIsAttr = 0;
	cons[1].attrType = chars;
	cons[1].bLhsIsAttr = 1;
	cons[1].LattrOffset = 21;
	cons[1].LattrLength = 21;
	cons[1].compOp = EQual;
	cons[1].bRhsIsAttr = 0;
	tmp = RM_OpenFile("SYSCOLUMNS", tabHandle);
	if (tmp!= SUCCESS) return tmp;
	for(int i=0;i<nSelAttrs;++i)
	{
		AttrNode attrNode;
		string tableName=selAttrs[i]->relName;
		cons[0].Rvalue=selAttrs[i]->relName;
		cons[1].Rvalue=selAttrs[i]->attrName;
		tmp=OpenScan(tabScan,tabHandle,2,cons);
		if(tmp!=SUCCESS) return tmp;
		tmp=GetNextRec(tabScan,tmpRec);
		if(tmp!=SUCCESS) return tmp;
		strcpy(attrNode.relName,selAttrs[i]->relName);
		memcpy(&attrNode.type,tmpRec->pData+42,sizeof(int));
		memcpy(&attrNode.offset,tmpRec->pData+50,sizeof(int));
		memcpy(&attrNode.length,tmpRec->pData+46,sizeof(int));
		if(*(tmpRec->pData+54)=='1')
			attrNode.ifHasIndex=true;
		else attrNode.ifHasIndex=false;
		if(attrMap.count(tableName)==0)
		{
			vector<AttrNode> attrVec;
			attrMap[tableName]=attrVec;
		}
		else	
			attrMap[tableName].push_back(attrNode);
		CloseScan(tabScan);
	}
	RM_CloseFile(tabHandle);
	free(tabHandle);
	free(tabScan);
	free(tmpRec);
	return SUCCESS;
}


//从指定的表中查询字段并返回
RC getFieldsFromTable(string &tableName,vector<AttrNode> &attrVec,map<string,vector<string>> &recMap)
{
	if(recMap.count(tableName)==0)
	{
		vector<string> tmp;
		recMap[tableName]=tmp;
	}
	RC tmp;
	RM_FileHandle* recfileHandle=new RM_FileHandle();
	recfileHandle->bOpen=false;
	RM_FileScan* recfileScan=new RM_FileScan();
	recfileScan->bOpen=false;
	RM_Record* tmpRec=new RM_Record();
	tmpRec->bValid=false;
	tmp=RM_OpenFile((char*)tableName.c_str(),recfileHandle);
	if(SUCCESS!=tmp) return tmp;
	tmp=OpenScan(recfileScan,recfileHandle,0,NULL);
	if(SUCCESS!=tmp) return tmp;
	while(GetNextRec(recfileScan,tmpRec)==SUCCESS)
	{
		string tmpRes="";
		for(int i=0;i<attrVec.size();++i)
			tmpRes+=attrVec[i].relName;
		if(tmpRes.size()!=0)
			recMap[tableName].push_back(tmpRes);
	}
	return SUCCESS;
}

//无条件表查询
RC selectWithoutCon(map<string,vector<AtrrNode>> &attrMap,map<string,vector<string>> recMap)
{	
	RC tmp;
	map<string,vector<AtrrNode>>::iterator it=attrMap.begin();
	RM_FileHandle* recfileHandle=new RM_FileHandle();
	recfileHandle->bOpen=false;
	for(;it!=attrMap.end();++it)
	{
		tmp=getFieldsFromTable((string)it->first,attrMap[it->first],recMap);
		if(SUCCESS!=tmp) return tmp;
	}
	return SUCCESS;
}
vector<string> getDiKar(vector<string> &res1,vector<string> &res2)
{
	if(res1.size()==0) return res2;
	if(res2.size()==0) return res1;
	vector<string> res;
	for(int i=0;i<res1.size();++i)
		for(int j=0;j<res2.size();++j)
			res.push_back(res1[i]+res1[j]);
	return res;
}
RC Select(int nSelAttrs,RelAttr **selAttrs,int nRelations,char **relations,int nConditions,Condition *conditions,SelResult * res)
{	
	SelResult* tmpRes=res;
	RC tmp;
	tmp=ifHasTable(nRelations,relations);//先检查所有的表是否存在
	if(SUCCESS!=tmp)
	{
		AfxMessageBox("表不存在！");
		return tmp;
	}
	map<string,vector<AtrrNode>> attrMap;
	map<string,vector<string>> recMap;
	getAttrMap(nSelAttrs,selAttrs,attrMap);
	if(nConditions==0)//无条件
		selectWithoutCon(attrMap,recMap);
	map<string,vector<AttrNode>>::iterator it=attrMap.begin();
	int index=0;
	tmpRes->row_num=0;
	for(;it!=attrMap.end();++it)  //初始化结果集字段
	{
		for(int i=0;i<it->second.size();++i)
		{
			tmpRes->col_num++;
			strcpy(tmpRes->fields[i],it->second[i].relName);
			tmpRes->length[index]=it->second[i].length;
			tmpRes->type[index]=it->second[i].type;
		}
	}
	//求结果的笛卡尔积
	map<string,vector<string>>::iterator it1=recMap.begin();
	vector<string> resVec;
	for(;it1!=recMap.end();it1++)
		resVec=getDiKar(resVec,it1->second);
	//合并到最终的结果集
	for(int i=0;i<resVec.size();++i)
	{
		if(tmpRes->row_num>=100) //记录数超过100
		{
			tmpRes->next_res=new SelResult();
			tmpRes=tmpRes->next_res;
			tmpRes->row_num=0;
			tmpRes->next_res=NULL;
		}
		(*tmpRes->res[i%100])=(char*)resVec[i].c_str();
		tmpRes->row_num++;
	}
	return SUCCESS;
}

/*检查数组relations内的表是否都存在,是返回SUCCESS，否则返回TABLE_NOT_EXIST*/
RC ifHasTable(int nRelations,char** relations)
{
	RC tmp;
	RM_FileHandle* tabHnadle=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	tabHnadle->bOpen=false;
	RM_FileScan* tabScan=(RM_FileScan*)malloc(sizeof(RM_FileScan));
	tabScan->bOpen=false;
	RM_Record* tmpRec=(RM_Record*)malloc(sizeof(RM_Record));
	tmpRec->bValid=false;
	tmp=RM_OpenFile("SYSTABLES.xx",tabHnadle);
	if(SUCCESS!=tmp)
	{
		AfxMessageBox("打开系统表文件失败！");
		return tmp;
	}
	Con condition;
	condition.bLhsIsAttr = 1;
	condition.attrType = chars;
	condition.LattrOffset = 0;
	condition.bRhsIsAttr = 0;
	condition.compOp = EQual;

	for(int i=0;i<nRelations;++i)
	{
		condition.Rvalue=*(relations+i);
		condition.RattrLength=strlen(*(relations+i))+1;
		tmp=OpenScan(tabScan,tabHnadle,1,&condition);
		if(SUCCESS!=tmp)
		{
			AfxMessageBox("打开系统表文件扫描失败！");
			return tmp;
		}
		tmp=GetNextRec(tabScan,tmpRec);
		if(SUCCESS!=tmp)
			return TABLE_NOT_EXIST;
		CloseScan(tabScan);
	}
	RM_CloseFile(tabHnadle);
	free(tabHnadle);
	free(tabScan);
	free(tmpRec);
	return SUCCESS;
}