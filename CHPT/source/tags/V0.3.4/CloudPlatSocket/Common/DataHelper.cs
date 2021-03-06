﻿using System.Data;
using SYSModel;
using BLL;
using CloudPlatSocket.Protocol;

namespace CloudPlatSocket
{
    /// <summary>
    /// 数据操作帮助类
    /// 创建人：杨天帅
    /// 创建时间：2014.11.10
    /// 修改日期：2014.11.17
    /// </summary>
    public class DataHelper
    {
        #region --获取新增数据 
       
        /// <summary> 获取添加类别的上传数据 
        /// </summary>
        /// <param name="tableName">表名</param>
        /// <param name="preTableName">关联表</param>        
        /// <param name="dbName">帐套名</param>
        /// <param name="lastTime">最后一次上传时间</param>
        /// <param name="time">下一次开始上传时间</param>
        /// <returns></returns>
        public static DataTable GetAddData(string tableName, string realTableName, string preTableName, string dbName, string lastTime, string time)
        {
            if (preTableName.Length == 0)
            {
                preTableName = tableName;
            }
            string where = string.Format("(create_time = update_time or update_time is null) and {3}.enable_flag='{0}' and {3}.create_time>='{1}' and {3}.create_time<'{2}'",
                DataSources.EnumStatus.Start.ToString("d"), lastTime, time, preTableName);
            DataTable dt = DBHelper.GetTable("获取新增上传表[" + tableName + "]数据", dbName, tableName, realTableName + ".*", where, "", "");
            if (dt != null)
            {
                dt.TableName = realTableName;
            }
            return dt;
        }
        public static DataTable GetAddData(string tableName, string dbName, string lastTime, string time)
        {
            return GetAddData(tableName, tableName, tableName, dbName, lastTime, time);
        }
        public static DataTable GetAddData(string tableName, ProtocolValue pv, string dbName, string lastTime, string time)        
        {
            if (pv != null && pv.PreTableName.Length > 0)
            {      
                return GetAddData(
                    string.Format("{0} inner join {1} on {0}.{2}={1}.{3}", tableName, pv.PreTableName, pv.Key, pv.PreKey),
                    tableName, pv.PreTableName, dbName, lastTime, time);
            }
            return GetAddData(tableName, dbName, lastTime, time);            
        }

        /// <summary> 获取添加类别的上传数据 
        /// </summary>
        /// <param name="tableName">表名</param>
        /// <param name="preTableName">关联表</param>        
        /// <param name="dbName">帐套名</param>
        /// <param name="lastTime">最后一次上传时间</param>
        /// <param name="time">下一次开始上传时间</param>
        /// <returns></returns>
        public static DataTable GetAddDataFromHXC(string tableName, string realTableName, string preTableName, string dbName, string lastTime, string time)
        {
            if (preTableName.Length == 0)
            {
                preTableName = tableName;
            }
            string where = string.Format("({3}.create_time = {3}.update_time or {3}.update_time is null) and {3}.enable_flag='{0}' and {3}.create_time>='{1}' and {3}.create_time<'{2}' and {3}.data_source='{4}'",
                DataSources.EnumStatus.Start.ToString("d"), lastTime, time, preTableName, DataSources.EnumDataSources.SELFBUILD.ToString("d"));
            DataTable dt = DBHelper.GetTable("获取新增上传表[" + tableName + "]数据", dbName, tableName, realTableName + ".*", where, "", "");
            if (dt != null)
            {
                dt.TableName = realTableName;
            }
            return dt;
        }
        public static DataTable GetAddDataFromHXC(string tableName, string dbName, string lastTime, string time)
        {
            return GetAddDataFromHXC(tableName,tableName, tableName, dbName, lastTime, time);
        }
        public static DataTable GetAddDataFromHXC(string tableName, ProtocolValue pv, string dbName, string lastTime, string time)
        {
            if (pv != null && pv.PreTableName.Length > 0)
            {
                return GetAddDataFromHXC(
                    string.Format("{0} inner join {1} on {0}.{2}={1}.{3}", tableName, pv.PreTableName, pv.Key, pv.PreKey),
                    tableName, pv.PreTableName, dbName, lastTime, time);
            }
            return GetAddDataFromHXC(tableName, dbName, lastTime, time);           
        }

        #endregion


        #region --获取修改数据
        /// <summary> 获取更新类别的上传数据
        /// </summary>
        /// <param name="tableName">表名</param>
        /// <param name="preTableName">关联表</param>        
        /// <param name="dbName">帐套名</param>
        /// <param name="lastTime">最后一次上传时间</param>
        /// <param name="time">下一次开始上传时间</param>
        /// <returns></returns>
        public static DataTable GetUpdateData(string tableName, string realTableName, string preTableName, string dbName, string lastTime, string time)
        {
            if (preTableName.Length == 0)
            {
                preTableName = tableName;
            }
            string where = string.Format("{3}.update_time > {3}.create_time and {3}.enable_flag='{0}' and {3}.update_time>='{1}' and {3}.update_time<'{2}'",
                DataSources.EnumStatus.Start.ToString("d"), lastTime, time, preTableName);
            DataTable dt = DBHelper.GetTable("获取更新上传表[" + tableName + "]数据", dbName, tableName, tableName + ".*", where, "", "");
            if (dt != null)
            {
                dt.TableName = realTableName;
            }
            return dt;
        }
        public static DataTable GetUpdateData(string tableName, string dbName, string lastTime, string time)
        {
            return GetUpdateData(tableName,tableName, tableName, dbName, lastTime, time);
        }
        public static DataTable GetUpdateData(string tableName, ProtocolValue pv, string dbName, string lastTime, string time)
        {
            if (pv != null && pv.PreTableName.Length > 0)
            {               
                return GetUpdateData(
                    string.Format("{0} inner join {1} on {0}.{2}={1}.{3}", tableName, pv.PreTableName, pv.Key, pv.PreKey),
                    tableName, pv.PreTableName, dbName, lastTime, time);
            }
            return GetUpdateData(tableName, dbName, lastTime, time);            
        }
        /// <summary> 获取更新类别的上传数据
        /// </summary>
        /// <param name="tableName">表名</param>
        /// <param name="preTableName">关联表</param>        
        /// <param name="dbName">帐套名</param>
        /// <param name="lastTime">最后一次上传时间</param>
        /// <param name="time">下一次开始上传时间</param>
        /// <returns></returns>
        public static DataTable GetUpdateDataFormHXC(string tableName, string realTableName, string preTableName, string dbName, string lastTime, string time)
        {
            if (preTableName.Length == 0)
            {
                preTableName = tableName;
            }
            string where = string.Format("{3}.update_time > {3}.create_time and {3}.enable_flag='{0}' and {3}.update_time>='{1}' and {3}.update_time<'{2}' and {3}.data_source='{4}'",
                DataSources.EnumStatus.Start.ToString("d"), lastTime, time, preTableName, DataSources.EnumDataSources.SELFBUILD.ToString("d"));
            DataTable dt = DBHelper.GetTable("获取更新上传表[" + tableName + "]数据", dbName, tableName, tableName + ".*", where, "", "");
            if (dt != null)
            {
                dt.TableName = realTableName;
            }
            return dt;
        }
        public static DataTable GetUpdateDataFormHXC(string tableName, string dbName, string lastTime, string time)
        {
            return GetUpdateDataFormHXC(tableName, tableName, tableName, dbName, lastTime, time);
        }        
        public static DataTable GetUpdateDataFormHXC(string tableName, ProtocolValue pv, string dbName, string lastTime, string time)
        {
            if (pv != null && pv.PreTableName.Length > 0)
            {              
                return GetUpdateDataFormHXC(
                    string.Format("{0} inner join {1} on {0}.{2}={1}.{3}", tableName, pv.PreTableName, pv.Key, pv.PreKey),
                    tableName, pv.PreTableName, dbName, lastTime, time);
            }
            return GetUpdateDataFormHXC(tableName, dbName, lastTime, time);            
        }
        #endregion


        #region --获取删除数据
        /// <summary> 获取删除类别的上传数据 
        /// </summary>
        /// <param name="tableName">表名</param>
        /// <param name="preTableName">关联表</param>        
        /// <param name="dbName">帐套名</param>
        /// <param name="lastTime">最后一次上传时间</param>
        /// <param name="time">下一次开始上传时间</param>
        /// <returns></returns>
        public static DataTable GetDeleteData(string tableName, string realTableName, string preTableName, string dbName, string lastTime, string time)
        {
            if (preTableName.Length == 0)
            {
                preTableName = tableName;
            }
            string where = string.Format("{3}.enable_flag='{0}' and {3}.update_time>='{1}' and {3}.update_time < '{2}'",
                DataSources.EnumStatus.Start.ToString("d"), lastTime, time, preTableName);
            DataTable dt = DBHelper.GetTable("获取删除上传表[" + tableName + "]数据", dbName, tableName, tableName + ".*", where, "", "");
            if (dt != null)
            {
                dt.TableName = realTableName;
            }
            return dt;
        }
        public static DataTable GetDeleteData(string tableName, string dbName, string lastTime, string time)
        {
            return GetDeleteData(tableName, tableName, tableName, dbName, lastTime, time);
        }
        public static DataTable GetDeleteData(string tableName, ProtocolValue pv, string dbName, string lastTime, string time)
        {
            if (pv != null && pv.PreTableName.Length > 0)
            {                
                return GetDeleteData(
                    string.Format("{0} inner join {1} on {0}.{2}={1}.{3}", tableName, pv.PreTableName, pv.Key, pv.PreKey),
                    tableName, pv.PreTableName, dbName, lastTime, time);
            }
            return GetDeleteData(tableName, dbName, lastTime, time);
        }

        /// <summary> 获取删除类别的上传数据 
        /// </summary>
        /// <param name="tableName">表名</param>  
        /// <param name="dbName">帐套名</param>
        /// <param name="lastTime">最后一次上传时间</param>
        /// <param name="time">下一次开始上传时间</param>
        /// <returns></returns>
        public static DataTable GetDeleteDataFromHXC(string tableName, string realTableName, string preTableName, string dbName, string lastTime, string time)
        {
            if (preTableName.Length == 0)
            {
                preTableName = tableName;
            }
            string where = string.Format("{3}.enable_flag='{0}' and {3}.update_time>='{1}' and {3}.update_time < '{2}' and {3}.data_source='{4}'",
                DataSources.EnumStatus.Start.ToString("d"), lastTime, time, preTableName, DataSources.EnumDataSources.SELFBUILD.ToString("d"));
            DataTable dt = DBHelper.GetTable("获取删除上传表[" + tableName + "]数据", dbName, tableName, tableName + ".*", where, "", "");
            if (dt != null)
            {
                dt.TableName = realTableName;
            }
            return dt;
        }
        public static DataTable GetDeleteDataFromHXC(string tableName, string dbName, string lastTime, string time)
        {
            return GetDeleteDataFromHXC(tableName, tableName, tableName, dbName, lastTime, time);
        }
        public static DataTable GetDeleteDataFromHXC(string tableName, ProtocolValue pv, string dbName, string lastTime, string time)
        {
            if (pv != null && pv.PreTableName.Length > 0)
            {               
                return GetDeleteDataFromHXC(
                    string.Format("{0} inner join {1} on {0}.{2}={1}.{3}", tableName, pv.PreTableName, pv.Key, pv.PreKey),
                    tableName, pv.PreTableName, dbName, lastTime, time);
            }
            return GetDeleteDataFromHXC(tableName, dbName, lastTime, time);
        }
        #endregion

        public static DataTable GetDataTest(string tableName,string dbName, string lastTime, string time)
        {      
            DataTable dt = DBHelper.GetTable("获取上传表[" + tableName + "]数据", dbName, tableName, "top 1 *", "", "", "");
            if (dt != null)
            {
                dt.TableName = tableName;
            }           
            return dt;
        }
    }
}
