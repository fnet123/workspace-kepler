﻿using System;
using System.Collections.Generic;
using System.Data;
using SYSModel;

namespace HXCPcClient.CommonClass
{
    /// <summary>
    /// 前后置单据关系操作
    /// </summary>
    /// <versioning>
    ///     <version number="1.0.0.0">
    ///         <author>Kord Kuo</author> 
    ///         <datetime>2015/1/15 14:23:47</datetime>
    ///         <comment>create</comment>
    ///     </version>
    /// </versioning>
    public class OrderRelation
    {
        #region Constructor -- 构造函数

        #endregion

        #region Field -- 字段

        #endregion

        #region Property -- 属性

        #endregion

        #region Method -- 方法
        public static DataView GetIsUsePreOrder(String preOrdeName, String preOrderColumnName, String postOrderName, String where)
        {
            if (String.IsNullOrEmpty(where)) where = "1=1";
            var sqlStr = String.Format("select cc.* from (select bb.pre_order_id as tempcolum, bb.status, bb.post_order_name, aa.* from {0} aa left join tr_order_relation bb on aa.{1}=bb.pre_order_id) as cc where cc.tempcolum is null and (cc.post_order_name != '{2}' or cc.post_order_name is null) and {3}", preOrdeName, preOrderColumnName, postOrderName, where);
            var sqlObj = new SQLObj
            {
                cmdType = CommandType.Text,
                sqlString = sqlStr,
                Param = new Dictionary<string, ParamObj>()
            };
            var dt = DBHelper.GetDataSet(String.Format("获取未被{0}占用的{1}单据", postOrderName, preOrdeName), sqlObj);
            if (dt == null || dt.Tables.Count < 1 || dt.Tables[0].DefaultView == null) return null;
            return dt.Tables[0].DefaultView;
        }
        #endregion

        #region Event -- 事件

        #endregion
    }
}
