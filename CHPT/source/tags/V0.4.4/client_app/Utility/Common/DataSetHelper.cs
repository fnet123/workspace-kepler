﻿using System;
using System.Collections.Generic;
using System.Data;


namespace Utility.Common
{
   public static class DataSetHelper
    {

        //// <summary> 2012-05-12 15:47 DataSet过滤重复数据 
/// 返回执行Select distinct后的DataTable
/// </summary>
/// <param name="SourceTable">源数据表</param>
/// <param name="FieldNames">字段集</param>
/// <returns></returns>
       public static DataTable SelectDistinct(DataTable SourceTable, params string[] FieldNames)
{
object[] lastValues;
DataTable newTable;
DataRow[] orderedRows;

if (FieldNames == null || FieldNames.Length == 0)
throw new ArgumentNullException("FieldNames");

lastValues = new object[FieldNames.Length];
newTable = new DataTable();

foreach (string fieldName in FieldNames)
newTable.Columns.Add(fieldName, SourceTable.Columns[fieldName].DataType);

orderedRows = SourceTable.Select("", string.Join(",", FieldNames));

foreach (DataRow row in orderedRows)
{
if (!fieldValuesAreEqual(lastValues, row, FieldNames))
{
newTable.Rows.Add(createRowClone(row, newTable.NewRow(), FieldNames));

setLastValues(lastValues, row, FieldNames);
}
}

return newTable;
}

       private static bool fieldValuesAreEqual(object[] lastValues, DataRow currentRow, string[] fieldNames)
{
bool areEqual = true;

for (int i = 0; i < fieldNames.Length; i++)
{
if (lastValues[i] == null || !lastValues[i].Equals(currentRow[fieldNames[i]]))
{
areEqual = false;
break;
}
}

return areEqual;
}

       private static DataRow createRowClone(DataRow sourceRow, DataRow newRow, string[] fieldNames)
{
foreach (string field in fieldNames)
newRow[field] = sourceRow[field];

return newRow;
}

       private static void setLastValues(object[] lastValues, DataRow sourceRow, string[] fieldNames)
{
for (int i = 0; i < fieldNames.Length; i++)
lastValues[i] = sourceRow[fieldNames[i]];
}

//--使用方法----------------------------------------------------------------------------------
//string[] fileds={"Filed1","Filed2"};//DISTINCT字段数组
//DataTable newdt=this.SelectDistinct(dt,fileds);//返回过滤后的DataTable


    }
}
