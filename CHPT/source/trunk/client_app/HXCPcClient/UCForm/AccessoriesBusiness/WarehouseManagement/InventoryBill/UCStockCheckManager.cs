﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using HXCPcClient.CommonClass;
using ServiceStationClient.ComponentUI;
using HXCPcClient.Chooser;
using Utility.Common;
using SYSModel;
namespace HXCPcClient.UCForm.AccessoriesBusiness.WarehouseManagement.InventoryBill
{
    public partial class UCStockCheckManager : UCBase
    {
        #region 全局变量
        private string CheckTable = "tb_parts_stock_check";//盘点单表
        private string CheckPartTable = "tb_parts_stock_check_p";//盘点单配件表
        private string PartsTable = "tb_parts";//配件档案表
        private string CheckID = "stock_check_id";//盘点单表主键
        private string WareHouseName = "wh_name";//仓库名称
        private string CheckQueryLogMsg = "查询盘点单表信息";//盘点单表操作日志
        private string CheckDelLogMsg = "批量删除盘点单表信息";//盘点单表操作日志
        private string CheckVerifyLogMsg = "批量审核盘点单表信息";//盘点单表操作日志
        private string CheckSubmitLogMsg = "提交盘点单更新信息";//盘点单提交日志
        private string CheckEdit = "编辑";
        private string CheckCopy = "复制";
        private int SearchFlag = 0;
        private const string ExportXlsName = "盘点单";
        List<string> CheckIDValuelist = new List<string>();//存储选中盘点单记录行主键ID
        //盘点单表字段
        private const string OrderNum = "order_num";
        private const string OrderDate = "order_date";
        private const string OrgName = "org_name";
        private const string HandleName = "handle_name";
        private const string OperatorName = "operator_name";
        private const string Remark = "remark";
        private const string OrderStatus = "order_status_name";
        //盘点单配件表字段 
        private const string PartPapCount = "paper_count";//账面数量
        private const string PartFirmCount = "firmoffer_count";//实盘数量
        private const string PartProCount = "profitloss_count";//盈亏数量
        private const string AmountMoney = "money";
        private const string PartID = "parts_id";//配件ID
        private const string PartsCode = "parts_code";//配件编码
        private const string PartName = "parts_name";//配件名称
        private const string PartSpec = "model";//配件规格
        private const string PartBarCode = "parts_barcode";//配件条码
        private const string CarPartsCode = "car_parts_code";//车厂编码
        private const string DrawNum = "drawing_num";//配件图号
        private const string UnitName = "unit_name";//单位名称
        private const string WarehID = "wh_id";//仓库ID
        private const string WarehName = "wh_name";//仓库名称
        #endregion
        public UCStockCheckManager()
        {
            InitializeComponent();

            //注册操作事件
            base.AddEvent += new ClickHandler(UCStockCheckManager_AddEvent);
            base.EditEvent += new ClickHandler(UCStockCheckManager_EditEvent);
            base.CopyEvent += new ClickHandler(UCStockCheckManager_CopyEvent);
            base.DeleteEvent += new ClickHandler(UCStockCheckManager_DeleteEvent);
            base.SubmitEvent += new ClickHandler(UCStockCheckManager_SubmitEvent);
            base.VerifyEvent += new ClickHandler(UCStockCheckManager_VerifyEvent);
            base.ExportEvent += new ClickHandler(UCStockCheckManager_ExportEvent);
            UIAssistants.SetButtonStyle4QueryAndClear(btnSearch, btnClear);//美化查询和清除按钮控件
           // DataGridViewEx.SetDataGridViewStyle(gvCheckBillList, OrderState);//美化表格控件
            //设置列表的可编辑状态
            gvCheckBillList.ReadOnly = false;
            foreach (DataGridViewColumn dgCol in gvCheckBillList.Columns)
            {
                if (dgCol.Name != colCheck.Name) dgCol.ReadOnly = true;
            }
            base.btnImport.Visible = false;
            base.btnCommit.Visible = false;
            base.btnStatus.Visible = false;
            base.btnBalance.Visible = false;
            base.btnSync.Visible = false;
        }
        /// <summary>
        /// 提交盘点单
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
       private  void UCStockCheckManager_SubmitEvent(object sender, EventArgs e)
        {
            try
            {
                List<string> CheckIdLst = GetSubmitRecord();//获取需提交的单据记录
                if (CheckIdLst.Count == 0)
                {
                    MessageBoxEx.Show("请选择符合提交条件的单据!", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }
                else
                {
                    Dictionary<string, string> CheckBillField = new Dictionary<string, string>();//存放更新字段
                    CheckBillField.Add("order_status", Convert.ToInt32(DataSources.EnumAuditStatus.SUBMIT).ToString());//状态ID
                    CheckBillField.Add("order_status_name", DataSources.GetDescription(DataSources.EnumAuditStatus.SUBMIT, true));//状态name
                    bool flag = DBHelper.BatchUpdateDataByIn(CheckSubmitLogMsg, CheckTable, CheckBillField, CheckID, CheckIdLst.ToArray());
                    if (flag)
                    {
                        MessageBoxEx.Show("提交成功！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        long StartDate = Common.LocalDateTimeToUtcLong(Convert.ToDateTime(DateTime.Now.AddMonths(-6).ToShortDateString()));//获取当前日期的半年前的日期
                        long EndDate = Common.LocalDateTimeToUtcLong(Convert.ToDateTime(DateTime.Now.ToShortDateString()));//获取当前日期
                        string DefaultWhere = " enable_flag=1  and CkBillTb.order_date between  " + StartDate + " and " + EndDate;//默认查询条件
                        GetCheckBillList(DefaultWhere);//刷新出入库单列表
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
            }
        }
        /// <summary>
        /// 导出Excel文件菜单
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
       private void UCStockCheckManager_ExportEvent(object sender, EventArgs e)
       {
           try
           {
               if (gvCheckBillList.Rows.Count == 0) //判断gridview中是否有数据记录
               {
                   MessageBoxEx.Show("您要导出的单据列表不能为空！");
                   return;
               }
               else
               {
                   DataTable XlsTable = new DataTable();//导出的数据表格
                   //创建表列项
                   XlsTable.Columns.Add("单据编号", typeof(string));
                   XlsTable.Columns.Add("单据日期", typeof(string));
                   XlsTable.Columns.Add("仓库", typeof(string));
                   XlsTable.Columns.Add("账面数量", typeof(string));
                   XlsTable.Columns.Add("实盘数量", typeof(string));
                   XlsTable.Columns.Add("盈亏数量", typeof(string));
                   XlsTable.Columns.Add("金额", typeof(string));
                   XlsTable.Columns.Add("部门", typeof(string));
                   XlsTable.Columns.Add("经办人", typeof(string));
                   XlsTable.Columns.Add("操作人", typeof(string));
                   XlsTable.Columns.Add("备注", typeof(string));
                   XlsTable.Columns.Add("单据状态", typeof(string));
                   foreach (DataGridViewRow dgRow in gvCheckBillList.Rows)
                   {
                       bool SelectFlag = (bool)((DataGridViewCheckBoxCell)dgRow.Cells["colCheck"]).EditedFormattedValue;//获取当前记录行的选择状态
                       if (SelectFlag == true)
                       {
                           DataRow TableRow = XlsTable.NewRow();//创建表行项

                           TableRow["单据编号"] = dgRow.Cells["BillNum"].Value.ToString();
                           TableRow["单据日期"] = dgRow.Cells["BillDate"].Value.ToString();
                           TableRow["仓库"] = dgRow.Cells["WHName"].Value.ToString();
                           TableRow["账面数量"] = dgRow.Cells["PapCount"].Value.ToString();
                           TableRow["实盘数量"] = dgRow.Cells["FirmCount"].Value.ToString();
                           TableRow["盈亏数量"] = dgRow.Cells["ProfitLosCount"].Value.ToString();
                           TableRow["金额"] = dgRow.Cells["TotalMoney"].Value.ToString();
                           TableRow["部门"] = dgRow.Cells["DepartName"].Value.ToString();
                           TableRow["经办人"] = dgRow.Cells["HandlerName"].Value.ToString();
                           TableRow["操作人"] = dgRow.Cells["OpeName"].Value.ToString();
                           TableRow["备注"] = dgRow.Cells["Remarks"].Value.ToString();
                           TableRow["单据状态"] = dgRow.Cells["OrderState"].Value.ToString();
                           XlsTable.Rows.Add(TableRow);
                       }
                   }
                   if (XlsTable.Rows.Count == 0)
                   {
                       MessageBoxEx.Show("请您选择要导出的单据记录行！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                       return;
                   }
                   ImportExportExcel.NPOIExportExcelFile(XlsTable, ExportXlsName);//生成Excel表格文件
               }
           }
           catch (Exception ex)
           {
               MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
           }
       }
        /// <summary>
        ///  窗体加载初始化
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UCStockCheckManager_Load(object sender, EventArgs e)
        {
            try
            {
                //获取默认系统时间
                dateTimeStart.Value = DateTime.Now.AddMonths(-3).ToShortDateString();
                dateTimeEnd.Value = DateTime.Now.ToString();
                //获取仓库名称
                CommonFuncCall.BindWarehouse(ComBwh_name, "请选择");
                //单据状态
                CommonFuncCall.BindOrderStatus(ComBorder_status_name, true);
                //公司
                CommonFuncCall.BindCompany(ComBcom_name, "全部");
                //部门
                CommonFuncCall.BindDepartment(ComBorg_name, "", "全部");
                //经办人
                CommonFuncCall.BindHandle(ComBhandle_name, "", "全部");
            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message,"异常提示",MessageBoxButtons.OK,MessageBoxIcon.Question);
            }
        }
        /// <summary>
        /// 添加盘点单
        /// </summary>
        /// <param name="send"></param>
        /// <param name="e"></param>
        private void UCStockCheckManager_AddEvent(object send, EventArgs e)
        {
            UCStockCheckAddOrEdit UCCheckBillAdd = new UCStockCheckAddOrEdit(WindowStatus.Add, null, this);
            base.addUserControl(UCCheckBillAdd, "盘点单-添加", "UCCheckBillAdd", this.Tag.ToString(), this.Name);
        }
        /// <summary>
        /// 编辑盘点单
        /// </summary>
        /// <param name="send"></param>
        /// <param name="e"></param>
        private void UCStockCheckManager_EditEvent(object send, EventArgs e)
        {
            EditOrCopyMethod(CheckEdit, WindowStatus.Edit);
        }
        /// <summary>
        /// 复制盘点单
        /// </summary>
        /// <param name="send"></param>
        /// <param name="e"></param>
        private void UCStockCheckManager_CopyEvent(object send, EventArgs e)
        {
            EditOrCopyMethod(CheckCopy, WindowStatus.Copy);
        }
        /// <summary>
        /// 删除盘点单
        /// </summary>
        /// <param name="send"></param>
        /// <param name="e"></param>
        private void UCStockCheckManager_DeleteEvent(object send, EventArgs e)
        {
            try
            {
                List<string> listField = GetSelectedRecord();//获取要删除的其它收货单记录ID
                if (listField.Count == 0 && gvCheckBillList.SelectedRows.Count == 0)
                {
                    MessageBoxEx.Show("请选择要删除的单据!", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }
                else 
                {
                    DialogResult DgResult = MessageBoxEx.Show("确定要删除选中单据？", "提示", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning);
                    if (DgResult == DialogResult.OK)
                    {
                        Dictionary<string, string> CheckBillField = new Dictionary<string, string>();
                        CheckBillField.Add("enable_flag", "0");
                        bool flag = DBHelper.BatchUpdateDataByIn(CheckDelLogMsg, CheckTable, CheckBillField, CheckID, listField.ToArray());//批量更改记录行删除标记为0已删除
                        if (flag)
                        {
                            long StartDate = Common.LocalDateTimeToUtcLong(Convert.ToDateTime(DateTime.Now.AddMonths(-6).ToShortDateString()));//获取当前日期的半年前的日期
                            long EndDate = Common.LocalDateTimeToUtcLong(Convert.ToDateTime(DateTime.Now.ToShortDateString()));//获取当前日期
                            string DefaultWhere = " enable_flag=1  and CkBillTb.order_date between  " + StartDate + " and " + EndDate;//默认查询条件
                            GetCheckBillList(DefaultWhere);//刷新盘点单列表
                            MessageBoxEx.Show("删除成功！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        }
                        else
                        { MessageBoxEx.Show("删除失败！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning); return; }
                    }
                    else
                    {
                        return;
                    }

                }



            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
            }
        }
        /// <summary>
        /// 审核其它收货单
        /// </summary>
        /// <param name="send"></param>
        /// <param name="e"></param>
        private void UCStockCheckManager_VerifyEvent(object send, EventArgs e)
        {
            try
            {
                Dictionary<string, long> OrderIDDateDic = GetVerifyRecord();//获取需要核实的记录行

                if (CheckIDValuelist.Count == 0 && gvCheckBillList.SelectedRows.Count == 0)
                {
                    MessageBoxEx.Show("请选择符合审核条件的单据!", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }

                UCVerify UcVerify = new UCVerify();
                UcVerify.ShowDialog();
                string Content = UcVerify.Content;
                DataSources.EnumAuditStatus UcVerifyStatus = UcVerify.auditStatus;//获取审核状态

                Dictionary<string, string> CheckBillField = new Dictionary<string, string>();
                if (UcVerifyStatus == DataSources.EnumAuditStatus.AUDIT)
                {
                    //获取其它收货单状态(已审核)
                    CheckBillField.Add("order_status", Convert.ToInt32(DataSources.EnumAuditStatus.AUDIT).ToString());
                    CheckBillField.Add("order_status_name", DataSources.GetDescription(DataSources.EnumAuditStatus.AUDIT, true));
                }
                else if (UcVerifyStatus == DataSources.EnumAuditStatus.NOTAUDIT)
                {
                    //获取其它收货单状态(审核不通过)
                    CheckBillField.Add("order_status", Convert.ToInt32(DataSources.EnumAuditStatus.NOTAUDIT).ToString());
                    CheckBillField.Add("order_status_name", DataSources.GetDescription(DataSources.EnumAuditStatus.NOTAUDIT, true));
                }
                bool flag = DBHelper.BatchUpdateDataByIn(CheckVerifyLogMsg, CheckTable, CheckBillField, CheckID, CheckIDValuelist.ToArray());//批量审核获取的盘点单记录
                if (flag)
                {
                    if (UcVerifyStatus == DataSources.EnumAuditStatus.AUDIT)
                    {
                        CommonFuncCall.StatisticStock(GetCheckPart(OrderIDDateDic), "盘点单配件表");//同步更新盘点单账面库存
                    }
                    MessageBoxEx.Show("审核成功！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    long StartDate = Common.LocalDateTimeToUtcLong(Convert.ToDateTime(DateTime.Now.AddMonths(-6).ToShortDateString()));//获取当前日期的半年前的日期
                    long EndDate = Common.LocalDateTimeToUtcLong(Convert.ToDateTime(DateTime.Now.ToShortDateString()));//获取当前日期
                    string DefaultWhere = " enable_flag=1 and CkBillTb.order_date between  " + StartDate + " and " + EndDate;//默认查询条件
                    GetCheckBillList(DefaultWhere);//刷新盘点单列表

                }
                else
                {
                    MessageBoxEx.Show("审核失败！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }
            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
            }
        }


        /// <summary>
        /// 获取盘点单配件表已审核的单据配件信息
        /// </summary>
        /// <param name="OrderIDDateDic">单据ID与日期对</param>
        /// <returns>返回待统计配件信息</returns>
        private DataTable GetCheckPart(Dictionary<string, long> OrderIDDateDic)
        {
            try
            {
                DataTable TemplateTable = null;//模版表
                foreach (KeyValuePair<string, long> KVPair in OrderIDDateDic)
                {
                    StringBuilder sbField = new StringBuilder();
                    sbField.AppendFormat("{0},{1},PtTb.{2},CheckTb.{3},CheckTb.{4},CheckTb.{5},CheckTb.{6},CheckTb.{7},CheckTb.{8},{9},{10}",
                     WarehID, WarehName, PartID, PartsCode, PartName, PartSpec, PartBarCode, CarPartsCode, DrawNum, UnitName, PartProCount);
                    string RelationTable = CheckPartTable + " as CheckTb left join " + PartsTable +
                    " as PtTb on CheckTb.parts_code=PtTb.ser_parts_code ";//要查询的关联表名
                    TemplateTable = CommonFuncCall.CreatePartStatisticTable();//获取要填充的公用表
                    //获取对应单据的配件信息
                    DataTable IOPartTable = DBHelper.GetTable("查询盘点单配件信息", RelationTable, sbField.ToString(), "stock_check_id='" + KVPair.Key.ToString() + "'", "", "");
                    for (int i = 0; i < IOPartTable.Rows.Count; i++)
                    {
                        DataRow dr = TemplateTable.NewRow();//创建模版表行项
                        dr["OrderDate"] = KVPair.Value.ToString();//单据日期
                        dr["WareHouseID"] = IOPartTable.Rows[i][WarehID].ToString();//仓库ID
                        dr["WareHouseName"] = IOPartTable.Rows[i][WarehName].ToString();//仓库名称
                        dr["PartID"] = IOPartTable.Rows[i][PartID].ToString();//配件ID
                        dr["PartCode"] = IOPartTable.Rows[i][PartsCode].ToString();//配件编码
                        dr["PartName"] = IOPartTable.Rows[i][PartName].ToString();//配件名称
                        dr["PartSpec"] = IOPartTable.Rows[i][PartSpec].ToString();//配件规格
                        dr["PartBarCode"] = IOPartTable.Rows[i][PartBarCode].ToString();//配件条码
                        dr["CarPartsCode"] = IOPartTable.Rows[i][CarPartsCode].ToString();//车厂编码
                        dr["DrawNum"] = IOPartTable.Rows[i][DrawNum].ToString();//配件图号
                        dr["UnitName"] = IOPartTable.Rows[i][UnitName].ToString();//单位名称
                        dr["PartCount"] = IOPartTable.Rows[i][PartProCount].ToString();//配件数量
                        dr["StatisticType"] = (int)DataSources.EnumStatisticType.PaperCount;//统计类型

                        TemplateTable.Rows.Add(dr);//添加新的数据行项
                    }
                }
                return TemplateTable;
            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
                return null;
            }

        }


        /// <summary>
        /// 查询盘点单操作
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnSearch_Click(object sender, EventArgs e)
        {
            string QueryWhere = BuildWhereCondation();//获取查询条件
            if (string.IsNullOrEmpty(QueryWhere)) return;
            GetCheckBillList(QueryWhere);
        }
        /// <summary>
        /// 分页操作
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void winFormCheckPage_PageIndexChanged(object sender, EventArgs e)
        {
            try
            {
                if (SearchFlag == (int)DataSources.SearchState.InitSearch)
                {

                    long StartDate = Common.LocalDateTimeToUtcLong(Convert.ToDateTime(DateTime.Now.AddMonths(-6).ToShortDateString()));//获取当前日期的半年前的日期
                    long EndDate = Common.LocalDateTimeToUtcLong(Convert.ToDateTime(DateTime.Now.ToShortDateString()));//获取当前日期
                    string DefaultWhere = " enable_flag=1 and CkBillTb.order_date between  " + StartDate + " and " + EndDate;//默认查询条件
                    GetCheckBillList(DefaultWhere);
                    SearchFlag = (int)DataSources.SearchState.WhereSearch;
                }
                else if (SearchFlag == (int)DataSources.SearchState.WhereSearch)
                {
                    gvCheckBillList.Rows.Clear();
                    string QueryWhere = "";//获取查询条件
                    GetCheckBillList(QueryWhere);
                }
            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
            }

        }
        /// <summary>
        /// 清除按钮事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnClear_Click(object sender, EventArgs e)
        {
            dateTimeStart.Value = DateTime.Now.AddMonths(-3).ToShortDateString();
            dateTimeEnd.Value = DateTime.Now.ToString();
            ComBwh_name.SelectedIndex = 0;
            ComBorder_status_name.SelectedIndex = 0;
            ComBcom_name.SelectedIndex = 0;
            ComBorg_name.SelectedIndex = 0;
            ComBhandle_name.SelectedIndex = 0;

        }


        /// <summary>
        /// 公司选择事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ddlCompany_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(ComBcom_name.SelectedValue.ToString()))
            {
                CommonFuncCall.BindDepartment(ComBorg_name, ComBcom_name.SelectedValue.ToString(), "全部");
            }
            else
            {
                CommonFuncCall.BindDepartment(ComBorg_name, "", "全部");
                CommonFuncCall.BindHandle(ComBhandle_name, "", "全部");
            }
        }
        /// <summary>
        /// 部门选择事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ddlDepartment_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(ComBorg_name.SelectedValue.ToString()))
            {
                CommonFuncCall.BindHandle(ComBhandle_name, ComBorg_name.SelectedValue.ToString(), "全部");
            }
            else
            {
                CommonFuncCall.BindHandle(ComBhandle_name, "", "全部");
            }
        }
        /// <summary>
        /// 列表双击查看盘点单配件明细
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void gvCheckBillList_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            try
            {
                if (e.RowIndex > -1 && e.ColumnIndex > -1)//双击表头或列头时不起作用   
                {
                    string ChkIdValue = gvCheckBillList.CurrentRow.Cells["ChkId"].Value.ToString();//获取出入库单ID
                    string WHName = gvCheckBillList.CurrentRow.Cells["WHName"].Value.ToString();//获取当前出入库单仓库名称
                    UCStockCheckDetail UCCheckBillDetails = new UCStockCheckDetail(ChkIdValue, WHName);
                    base.addUserControl(UCCheckBillDetails, "盘点单-查看", "UCCheckBillDetails" + ChkIdValue + "", this.Tag.ToString(), this.Name);
                }
            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
            }
        }
        /// <summary>
        /// 盘点单编辑或复制
        /// </summary>
        /// <param name="HandleType"></param>
        /// <param name="state"></param>
        private void EditOrCopyMethod(string HandleType, WindowStatus state)
        {
            try
            {
                string CheckId = string.Empty;
                List<string> BillIDlist = GetSelectedRecord();//获取复制的其它收货单记录
                if (BillIDlist.Count == 0 && gvCheckBillList.SelectedRows.Count == 0)
                {
                    MessageBoxEx.Show("请选择要" + HandleType + "的数据!");
                    return;
                }
                else if (BillIDlist.Count > 1 && gvCheckBillList.SelectedRows.Count > 1)
                {
                    MessageBoxEx.Show("一次只能" + HandleType + "一条数据!");
                    return;
                }
                else 
                {
                    if (BillIDlist.Count == 1)
                    {
                        CheckId = BillIDlist[0].ToString();
                    }
                    else if (gvCheckBillList.SelectedRows.Count == 1)
                    {
                        CheckId = gvCheckBillList.CurrentRow.Cells["ChkId"].ToString();
                    }
                    UCStockCheckAddOrEdit UCCheckBillHandle = new UCStockCheckAddOrEdit(state, CheckId, this);
                    base.addUserControl(UCCheckBillHandle, "盘点单-" + HandleType, "UCCheckBillHandle" + CheckId, this.Tag.ToString(), this.Name);
                }
            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
            }
        }

        /// <summary>
        /// 获取盘点单列表选中的记录条数
        /// </summary>
        /// <returns></returns>
        private List<string> GetSelectedRecord()
        {

            if (CheckIDValuelist.Count > 0) CheckIDValuelist.Clear();//清除之前的数据
            foreach (DataGridViewRow dr in gvCheckBillList.Rows)
            {
                object isCheck = dr.Cells["colCheck"].EditedFormattedValue;
                if (isCheck != null && (bool)isCheck)
                {
                    CheckIDValuelist.Add(dr.Cells["ChkId"].Value.ToString());
                }
            }
            if (CheckIDValuelist.Count == 0 && gvCheckBillList.SelectedRows.Count == 1)
            {
                string DelInoutId = gvCheckBillList.CurrentRow.Cells["ChkId"].Value.ToString();
                CheckIDValuelist.Add(DelInoutId);
            }
            return CheckIDValuelist;
        }
        /// <summary>
        /// 获取选中的草稿单据记录行
        /// </summary>
        /// <returns></returns>
        private List<string> GetSubmitRecord()
        {
            try
            {
                if (CheckIDValuelist.Count > 0) CheckIDValuelist.Clear();//清除之前的数据
                foreach (DataGridViewRow dr in gvCheckBillList.Rows)
                {
                    bool isCheck = (bool)dr.Cells["colCheck"].EditedFormattedValue;
                    if (isCheck)
                    {
                        //获取保存草稿状态的单据记录
                        string BillStatusDraft = DataSources.GetDescription(DataSources.EnumAuditStatus.DRAFT, true);
                        string ColOrderStatus = dr.Cells["OrderState"].Value.ToString();
                        if (BillStatusDraft == ColOrderStatus)
                        {
                            CheckIDValuelist.Add(dr.Cells["ChkId"].Value.ToString());//添加草稿状态主键ID
                        }
                    }
                }
                return CheckIDValuelist;

            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
                return null;
            }

        }
        /// <summary>
        /// 查询盘点单列表
        /// </summary>
        public void GetCheckBillList(string WhereStr)
        {
            try
            {
                gvCheckBillList.Rows.Clear();//清空所有记录行
                StringBuilder sbField = new StringBuilder();//其它收货单查询字段集合
                sbField.AppendFormat("CkBillTb.{0},{1},{2},CkBillTb.{3},PapAmount,FirmAmount,ProAmount, AmtMoney,{4},{5},{6},CkBillTb.{7},{8}", CheckID,
                OrderNum, OrderDate, WareHouseName,OrgName, HandleName, OperatorName, Remark, OrderStatus);
                StringBuilder sbRelationTable = new StringBuilder();//关联多表查询
                sbRelationTable.AppendFormat("{0} as CkBillTb inner join "+
                " (select {0}.{2},sum({3}) as PapAmount,sum({4}) as FirmAmount,sum({5}) as ProAmount,sum({6}) as AmtMoney from {0} inner join {1}"+
                " on {0}.{2}={1}.{2} group by {0}.{2}) as CkBillPartTb on CkBillPartTb.{2}=CkBillTb.{2}",
                CheckTable,CheckPartTable, CheckID, PartPapCount, PartFirmCount, PartProCount, AmountMoney);
                int RecCount = 0;//查询记录行数

                DataTable ChkBillTable = DBHelper.GetTableByPage(CheckQueryLogMsg, sbRelationTable.ToString(), sbField.ToString(), WhereStr,
                "", " CkBillTb.create_time desc", winFormCheckPage.PageIndex, winFormCheckPage.PageSize, out RecCount);//获取盘点单表查询记录
                winFormCheckPage.RecordCount = RecCount;//获取总记录行
                if (RecCount == 0)
                {
                    return;

                }
                //把查询的其它收货单列表放入Gridview
                for (int i = 0; i < ChkBillTable.Rows.Count; i++)
                {

                    DataGridViewRow gvRow = gvCheckBillList.Rows[gvCheckBillList.Rows.Add()];//创建行项
                    gvRow.Cells["ChkId"].Value = ChkBillTable.Rows[i][CheckID].ToString();//存放盘点单ID 
                    if (ChkBillTable.Rows[i][OrderStatus].ToString() == DataSources.GetDescription(DataSources.EnumAuditStatus.DRAFT, true))
                    {
                        gvRow.Cells["BillNum"].Value = string.Empty;
                    }
                    else
                    {
                        gvRow.Cells["BillNum"].Value = ChkBillTable.Rows[i][OrderNum].ToString();
                    }
                    DateTime OrdDate = Common.UtcLongToLocalDateTime(Convert.ToInt64(ChkBillTable.Rows[i][OrderDate]));//获取单据日期
                    gvRow.Cells["BillDate"].Value = OrdDate.ToLongDateString();//单据日期
                    gvRow.Cells["WHName"].Value = ChkBillTable.Rows[i][WareHouseName].ToString();
                    gvRow.Cells["PapCount"].Value = ChkBillTable.Rows[i]["PapAmount"].ToString();//账面
                    gvRow.Cells["FirmCount"].Value = ChkBillTable.Rows[i]["FirmAmount"].ToString();//实际
                    gvRow.Cells["ProfitLosCount"].Value = ChkBillTable.Rows[i]["ProAmount"].ToString();//盈亏
                    if (Convert.ToDecimal(gvRow.Cells["ProfitLosCount"].Value.ToString()) < 0)
                    {
                        gvRow.Cells["ProfitLosCount"].Style.ForeColor = Color.Red;
                    }
                    gvRow.Cells["TotalMoney"].Value = ChkBillTable.Rows[i]["AmtMoney"].ToString();
                    gvRow.Cells["DepartName"].Value = ChkBillTable.Rows[i][OrgName].ToString();
                    gvRow.Cells["HandlerName"].Value = ChkBillTable.Rows[i][HandleName].ToString();
                    gvRow.Cells["OpeName"].Value = ChkBillTable.Rows[i][OperatorName].ToString();
                    gvRow.Cells["Remarks"].Value = ChkBillTable.Rows[i][Remark].ToString();
                    gvRow.Cells["OrderState"].Value = ChkBillTable.Rows[i][OrderStatus].ToString();

                }

            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
            }

        }


        /// <summary>
        /// 获取盘点单列表选中要审核的记录
        /// 只有工单状态是已提交的才可以被审核
        /// </summary>
        /// <returns></returns>
        private Dictionary<string, long> GetVerifyRecord()
        {
            try
            {
                if (CheckIDValuelist.Count > 0) CheckIDValuelist.Clear();
                Dictionary<string, long> DicField = new Dictionary<string, long>();
                foreach (DataGridViewRow dr in gvCheckBillList.Rows)
                {
                    object isCheck = dr.Cells["colCheck"].EditedFormattedValue;
                    if (isCheck != null && (bool)isCheck)
                    {
                        //获取已提交/审核未通过的状态的编号
                        string BillStatusSUBMIT =DataSources.GetDescription(DataSources.EnumAuditStatus.SUBMIT,true);
                        string BillStatusNOTAUDIT =DataSources.GetDescription(DataSources.EnumAuditStatus.NOTAUDIT,true);
                        string ColOrderStatus = dr.Cells["OrderState"].Value.ToString();
                        if (BillStatusSUBMIT == ColOrderStatus || BillStatusNOTAUDIT == ColOrderStatus)
                        {
                            long OrdeDate = Common.LocalDateTimeToUtcLong(Convert.ToDateTime(dr.Cells["BillDate"].Value));
                            DicField.Add(dr.Cells["ChkId"].Value.ToString(), OrdeDate);//添加已审核单据主键ID和单据日期键值对
                            CheckIDValuelist.Add(dr.Cells["ChkId"].Value.ToString());//添加选中的主键id 
                        }
                    }
                }
                return DicField;
            }
            catch (Exception ex)
            {
                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
                return null;
            }
        }

        /// <summary>
        /// 创建查询条件
        /// </summary>
        /// <returns></returns>
        private string BuildWhereCondation()
        {
            try
            {
                string Str_Where = " enable_flag=1 "; //未删除记录行
                DateTime dStarttime = Convert.ToDateTime(dateTimeStart.Value.ToString());//开始时间
                DateTime dEndtime = Convert.ToDateTime(dateTimeEnd.Value.ToString());//结束时间
                if (!string.IsNullOrEmpty(ComBwh_name.SelectedValue.ToString()))
                {
                    Str_Where += " and CkBillTb.wh_name = '" + ComBwh_name.Text.ToString() + "'";
                }
                if (!string.IsNullOrEmpty(ComBorder_status_name.SelectedValue.ToString()))
                {
                    Str_Where += " and order_status_name='" + ComBorder_status_name.Text.ToString() + "'";
                }

                 if (!string.IsNullOrEmpty(ComBcom_name.SelectedValue.ToString()))
                {
                    Str_Where += " and com_name='" + ComBcom_name.Text.ToString() + "'";
                }
                 if (!string.IsNullOrEmpty(ComBorg_name.SelectedValue.ToString()))
                {
                    Str_Where += " and org_name='" + ComBorg_name.Text.ToString() + "'";
                }
                 if (!string.IsNullOrEmpty(ComBhandle_name.SelectedValue.ToString()))
                {
                    Str_Where += " and handle_name='" + ComBhandle_name.Text.ToString() + "'";
                }
                 if (dStarttime.ToShortDateString() == dEndtime.ToShortDateString())
                {
                    Str_Where += " and CkBillTb.order_date=" + Common.LocalDateTimeToUtcLong(dStarttime);
                }
                else 
                {
                     if (dateTimeStart.Value.ToString() != null)
                    {

                        Str_Where += " and CkBillTb.order_date>=" + Common.LocalDateTimeToUtcLong(dStarttime);
                    }
                     if (dateTimeEnd.Value.ToString() != null)
                    {

                        Str_Where += " and CkBillTb.order_date<=" + Common.LocalDateTimeToUtcLong(dEndtime);
                    }
                }
                 if (Common.LocalDateTimeToUtcLong(dStarttime) >Common.LocalDateTimeToUtcLong(dEndtime))
                {
                    MessageBoxEx.Show("您输入的开始时间不能大于结束时间！","提示",MessageBoxButtons.OK,MessageBoxIcon.Warning);
                    return Str_Where = string.Empty;
                }

                return Str_Where;
            }
            catch (Exception ex)
            {

                MessageBoxEx.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
                return null;
            }
        }
    }
}
