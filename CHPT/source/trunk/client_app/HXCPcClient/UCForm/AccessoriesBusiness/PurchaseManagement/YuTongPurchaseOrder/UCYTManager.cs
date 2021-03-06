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
using SYSModel;
using HXCPcClient.Chooser;
using Utility.Common;
using Model;
using System.Drawing.Printing;

namespace HXCPcClient.UCForm.AccessoriesBusiness.PurchaseManagement.YuTongPurchaseOrder
{
    public partial class UCYTManager : UCBase
    {
        List<string> listIDs = new List<string>();//已选择项的ID列表
        List<string> listStart = new List<string>();//启用状态
        List<string> listStop = new List<string>();//停用状态
        BusinessPrint businessPrint;//业务打印功能
        #region 变量
        tb_parts_purchase_order_2 yt_purchaseorder_model = new tb_parts_purchase_order_2();
        tb_parts_purchase_order_p_2 yt_partsorder_model = new tb_parts_purchase_order_p_2();
        #endregion

        #region 初始化窗体
        /// <summary>
        /// 初始化窗体
        /// </summary>
        public UCYTManager()
        {
            InitializeComponent();
            base.AddEvent += new ClickHandler(UCYTManager_AddEvent);
            base.CopyEvent += new ClickHandler(UCYTManager_CopyEvent);
            base.EditEvent += new ClickHandler(UCYTManager_EditEvent);
            base.DeleteEvent += new ClickHandler(UCYTManager_DeleteEvent);
            base.VerifyEvent += new ClickHandler(UCYTManager_VerifyEvent);
            base.SubmitEvent += new ClickHandler(UCYTManager_SubmitEvent);
            base.ExportEvent += new ClickHandler(UCYTManager_ExportEvent);
            base.ViewEvent += new ClickHandler(UCYTManager_ViewEvent);
            base.PrintEvent += new ClickHandler(UCYTManager_PrintEvent);
            base.SetEvent += new ClickHandler(UCYTManager_SetEvent);
            #region 预览、打印设置
            string printObject = "tb_parts_purchase_order_ytcg";
            string printTitle = "宇通采购订单";
            List<string> listNotPrint = new List<string>();
            listNotPrint.Add(purchase_order_yt_id.Name);
            listNotPrint.Add(viewfile.Name);
            PaperSize paperSize = new PaperSize();
            paperSize.Width = 297;
            paperSize.Height = 210;
            businessPrint = new BusinessPrint(gvYTPurchaseOrderList, printObject, printTitle, paperSize, listNotPrint);
            #endregion
        }
        /// <summary>
        /// 窗体加载
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UCYTManager_Load(object sender, EventArgs e)
        {
            //base.SetBaseButtonStatus();
            //base.SetButtonVisiableManager();
            dateTimeReqDeliveryTimeStart.Value = DateTime.Now.AddDays(-DateTime.Now.Day + 1);
            dateTimeReqDeliveryTimeEnd.Value = DateTime.Now;

            string[] NotReadOnlyColumnsName = new string[] { "colCheck" };
            CommonFuncCall.SetColumnReadOnly(gvYTPurchaseOrderList, NotReadOnlyColumnsName);
            //设置查询按钮和清除按钮样式
            UIAssistants.SetButtonStyle4QueryAndClear(this,btnSearch, btnClear);
            //列表的右键操作功能
            base.SetContentMenuScrip(gvYTPurchaseOrderList);
            //绑定宇通采购订单类型
            CommonFuncCall.BindYTPurchaseOrderType(ddlorder_type, true, "全部");
            //绑定紧急程度
            CommonFuncCall.BindComBoxDataSource(ddlemergency_level, "emergency_level_yt", "全部");
            //调拨类型
            CommonFuncCall.BindYTAllotType(ddlallot_type, true, "全部");

            CommonFuncCall.BindCompany(ddlCompany, "全部");
            CommonFuncCall.BindDepartment(ddlDepartment, "", "全部");
            CommonFuncCall.BindHandle(ddlhandle, "", "全部");
            CommonFuncCall.BindOrderStatus(ddlorder_status, true);
            BindgvYTPurchaseOrderList();

            Choosefrm.PartsCodeChoose(txtparts_code, Choosefrm.delDataBack = PartsName_DataBack);
        } 
        #endregion

        #region 控件事件
        #region 打印事件
        void UCYTManager_PrintEvent(object sender, EventArgs e)
        {
            businessPrint.Print(gvYTPurchaseOrderList.GetBoundData());
        }
        #endregion

        #region 预览事件
        void UCYTManager_ViewEvent(object sender, EventArgs e)
        {
            businessPrint.Preview(gvYTPurchaseOrderList.GetBoundData());
        }
        #endregion

        #region 导出事件
        void UCYTManager_ExportEvent(object sender, EventArgs e)
        {
            if (this.gvYTPurchaseOrderList.Rows.Count == 0)
            {
                return;
            }
            try
            {
                string fileName = "宇通采购订单" + DateTime.Now.ToString("yyyy-MM-dd") + ".xls";
                ExcelHandler.ExportExcel(fileName, gvYTPurchaseOrderList);
            }
            catch (Exception ex)
            {
                Utility.Log.Log.writeLineToLog("【宇通采购订单】" + ex.Message, "server");
                MessageBoxEx.ShowWarning("导出失败！");
            }
        }
        #endregion

        #region 预览、打印设置
        void UCYTManager_SetEvent(object sender, EventArgs e)
        {
            businessPrint.PrintSet(gvYTPurchaseOrderList);
        } 
        #endregion

        /// <summary> 添加事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void UCYTManager_AddEvent(object sender, EventArgs e)
        {
            UCYTAddOrEdit UCYTPurchaseOrderAdd = new UCYTAddOrEdit(WindowStatus.Add, null, this);
            base.addUserControl(UCYTPurchaseOrderAdd, "宇通采购订单-添加", "UCYTPurchaseOrderAdd", this.Tag.ToString(), this.Name);
        }
        /// <summary> 复制事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void UCYTManager_CopyEvent(object sender, EventArgs e)
        {
            string order_id = string.Empty;
            List<string> listField = GetSelectedRecord();
            if (listField.Count == 0)
            {
                MessageBoxEx.Show("请选择要复制的数据!");
                return;
            }
            if (listField.Count > 1)
            {
                MessageBoxEx.Show("一次只可以复制一条数据!");
                return;
            }
            order_id = listField[0].ToString();
            UCYTAddOrEdit UCPurchaseOrderCopy = new UCYTAddOrEdit(WindowStatus.Copy, order_id, this);
            base.addUserControl(UCPurchaseOrderCopy, "宇通采购订单-复制", "UCPurchaseOrderCopy" + order_id + "", this.Tag.ToString(), this.Name);
        }
        /// <summary> 编辑事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void UCYTManager_EditEvent(object sender, EventArgs e)
        {
            string order_id = string.Empty;
            bool IsHandle = true;
            List<string> listField = GetSelectedRecordByEditDelete(ref IsHandle);
            if (IsHandle)
            {
                if (listField.Count == 0)
                {
                    MessageBoxEx.Show("请选择要编辑的数据!");
                    return;
                }
                if (listField.Count > 1)
                {
                    MessageBoxEx.Show("一次只可以编辑一条数据!");
                    return;
                }
                order_id = listField[0].ToString();
                UCYTAddOrEdit UCPurchaseOrderEdit = new UCYTAddOrEdit(WindowStatus.Edit, order_id, this);
                base.addUserControl(UCPurchaseOrderEdit, "宇通采购订单-编辑", "UCPurchaseOrderEdit" + order_id + "", this.Tag.ToString(), this.Name);
            }
        }
        /// <summary> 删除事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void UCYTManager_DeleteEvent(object sender, EventArgs e)
        {
            bool IsHandle = true;
            List<string> listField = GetSelectedRecordByEditDelete(ref IsHandle);
            if (IsHandle)
            {
                if (listField.Count == 0)
                {
                    MessageBoxEx.Show("请选择要删除的数据!");
                    return;
                }
                if (MessageBoxEx.Show("确认要删除选中的数据吗？", "提示", MessageBoxButtons.OKCancel, MessageBoxIcon.Question) != DialogResult.OK)
                {
                    return;
                }
                Dictionary<string, string> purchaseOrderField = new Dictionary<string, string>();
                purchaseOrderField.Add("enable_flag", "0");
                bool flag = DBHelper.BatchUpdateDataByIn("批量删除宇通采购订单表", "tb_parts_purchase_order_2", purchaseOrderField, "purchase_order_yt_id", listField.ToArray());
                if (flag)
                {
                    BindgvYTPurchaseOrderList();
                    MessageBoxEx.Show("操作成功！");
                }
                else
                {
                    MessageBoxEx.Show("操作失败！");
                }
            }
        }
        /// <summary> 审核事件 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void UCYTManager_VerifyEvent(object sender, EventArgs e)
        {
            List<string> listField = GetVerifyRecord();
            if (listField.Count == 0)
            {
                MessageBoxEx.Show("请选择要审核的数据!");
                return;
            }
            UCVerify UcVerify = new UCVerify();
            UcVerify.ShowDialog();
            string Content = UcVerify.Content;
            SYSModel.DataSources.EnumAuditStatus UcVerifyStatus = UcVerify.auditStatus;

            List<SysSQLString> list_sql = new List<SysSQLString>();
            for (int i = 0; i < listField.Count; i++)
            {
                SysSQLString sysStringSql = new SysSQLString();
                sysStringSql.cmdType = CommandType.Text;
                Dictionary<string, string> dic = new Dictionary<string, string>();
                if (UcVerifyStatus == DataSources.EnumAuditStatus.AUDIT)
                {
                    //获取宇通采购订单状态(已审核)
                    dic.Add("apply_date_time", Common.LocalDateTimeToUtcLong(DateTime.Now).ToString());
                    dic.Add("order_status", Convert.ToInt32(DataSources.EnumAuditStatus.AUDIT).ToString());
                    dic.Add("order_status_name", DataSources.GetDescription(DataSources.EnumAuditStatus.AUDIT, true));
                }
                else if (UcVerifyStatus == DataSources.EnumAuditStatus.NOTAUDIT)
                {
                    //获取宇通采购订单状态(审核不通过)
                    dic.Add("order_status", Convert.ToInt32(DataSources.EnumAuditStatus.NOTAUDIT).ToString());
                    dic.Add("order_status_name", DataSources.GetDescription(DataSources.EnumAuditStatus.NOTAUDIT, true));
                }
                dic.Add("update_by", GlobalStaticObj.UserID);//修改人Id
                dic.Add("update_name", GlobalStaticObj.UserName);//修改人姓名
                dic.Add("update_time", Common.LocalDateTimeToUtcLong(DateTime.Now).ToString());//修改时间  
                dic.Add("purchase_order_yt_id", listField[i]);
                sysStringSql.sqlString = @"update tb_parts_purchase_order_2 set 
                                               order_status=@order_status,order_status_name=@order_status_name,
                                               update_by=@update_by,update_name=@update_name,update_time=@update_time 
                                               where purchase_order_yt_id=@purchase_order_yt_id";
                sysStringSql.Param = dic;
                list_sql.Add(sysStringSql);
            }
            if (DBHelper.BatchExeSQLStringMultiByTrans("宇通采购订单审核操作", list_sql))
            {
                if (UcVerifyStatus == DataSources.EnumAuditStatus.AUDIT)
                {
                    DealPurascherToYT(listField);
                }
                BindgvYTPurchaseOrderList();
                MessageBoxEx.Show("操作成功！");
            }
            else
            {
                MessageBoxEx.Show("操作失败！");
            }
        }
        /// <summary> 提交事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void UCYTManager_SubmitEvent(object sender, EventArgs e)
        {
            try
            {
                List<string> listField = GetSelectedRecord();
                if (listField.Count == 0)
                {
                    MessageBoxEx.Show("请选择要提交的数据!");
                    return;
                }
                if (MessageBoxEx.Show("确认要提交选中的数据吗？", "提示", MessageBoxButtons.OKCancel, MessageBoxIcon.Question) != DialogResult.OK)
                {
                    return;
                }
                foreach (DataGridViewRow dr in gvYTPurchaseOrderList.Rows)
                {
                    object isCheck = dr.Cells["colCheck"].EditedFormattedValue;
                    if (isCheck != null && (bool)isCheck)
                    {
                        SysSQLString obj = new SysSQLString();
                        obj.cmdType = CommandType.Text;
                        List<SysSQLString> listSql = new List<SysSQLString>();
                        Dictionary<string, string> dicParam = new Dictionary<string, string>();

                        string order_num = string.Empty;
                        if (dr.Cells["order_status"].Value.ToString() == Convert.ToInt32(DataSources.EnumAuditStatus.DRAFT).ToString())//草稿状态
                        {
                            if (dr.Cells["order_num"].Value != null && dr.Cells["order_num"].Value.ToString().Length > 0)
                            {
                                order_num = dr.Cells["order_num"].Value.ToString();
                            }
                            else
                            {
                                order_num = CommonUtility.GetNewNo(DataSources.EnumProjectType.YTPurchaseOrder);
                            }
                        }
                        else if (dr.Cells["order_status"].Value.ToString() == Convert.ToInt32(DataSources.EnumAuditStatus.NOTAUDIT).ToString())//审核未通过
                        {
                            order_num = dr.Cells["order_num"].Value.ToString();
                        }

                        dicParam.Add("order_num", order_num);//获取宇通采购订单编号
                        dicParam.Add("purchase_order_yt_id", dr.Cells["purchase_order_yt_id"].Value.ToString());//单据ID
                        dicParam.Add("order_status", Convert.ToInt32(DataSources.EnumAuditStatus.SUBMIT).ToString());//单据状态ID
                        dicParam.Add("order_status_name", DataSources.GetDescription(DataSources.EnumAuditStatus.SUBMIT, true));//单据状态名称
                        dicParam.Add("update_by", GlobalStaticObj.UserID);//修改人ID
                        dicParam.Add("update_name", GlobalStaticObj.UserName);//修改人姓名
                        dicParam.Add("update_time", Common.LocalDateTimeToUtcLong(DateTime.Now).ToString());//修改时间
                        obj.sqlString = "update tb_parts_purchase_order_2 set order_num=@order_num,order_status=@order_status,order_status_name=@order_status_name,update_by=@update_by,update_name=@update_name,update_time=@update_time where purchase_order_yt_id=@purchase_order_yt_id";
                        obj.Param = dicParam;
                        listSql.Add(obj);
                        GetPre_Order_Code(listSql, dr.Cells["purchase_order_yt_id"].Value.ToString(), dr.Cells["purchase_order_yt_id"].Value.ToString(), order_num);
                        if (DBHelper.BatchExeSQLStringMultiByTrans("更新单据状态为提交", listSql))
                        {
                            SetOrderStatus(dr.Cells["purchase_order_yt_id"].Value.ToString());
                        }
                    }
                }
                MessageBoxEx.Show("提交单据完成!");
                BindgvYTPurchaseOrderList();
            }
            catch (Exception ex)
            { }
        }
        /// <summary>
        /// 清除事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnClear_Click(object sender, EventArgs e)
        {
            txtorder_num.Caption = string.Empty;
            txtparts_code.Text = string.Empty;
            txtparts_name.Caption = string.Empty;
            txtRemark.Caption = string.Empty;
            ddlorder_type.SelectedIndex = 0;
            ddlallot_type.SelectedIndex = 0;
            ddlorder_status.SelectedIndex = 0;
            ddlemergency_level.SelectedIndex = 0;
            ddlCompany.SelectedIndex = 0;
            ddlDepartment.SelectedIndex = 0;
            ddlhandle.SelectedIndex = 0;
            dateTimeEnd.Value = string.Empty;
            dateTimeStart.Value = string.Empty;
            dateTimeReqDeliveryTimeStart.Value = DateTime.Now.AddDays(-DateTime.Now.Day + 1);
            dateTimeReqDeliveryTimeEnd.Value = DateTime.Now;
        }
        /// <summary>
        /// 查询事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnSearch_Click(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(dateTimeStart.Value) && !string.IsNullOrEmpty(dateTimeEnd.Value))
            {
                if (Convert.ToDateTime(Convert.ToDateTime(dateTimeStart.Value).ToShortDateString() + " 00:00:00") > Convert.ToDateTime(Convert.ToDateTime(dateTimeEnd.Value).ToShortDateString() + " 00:00:00"))
                {
                    MessageBoxEx.Show("申请日期的开始时间不可以大于结束时间"); return;
                }
            }
            if (Convert.ToDateTime(dateTimeReqDeliveryTimeStart.Value.ToShortDateString() + " 00:00:00") > Convert.ToDateTime(dateTimeReqDeliveryTimeEnd.Value.ToShortDateString() + " 00:00:00"))
            { MessageBoxEx.Show("要求发货时间的开始时间不可以大于结束时间"); return; }
             BindgvYTPurchaseOrderList();
        }
        /// <summary>
        /// 选择配件编码事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void txtparts_code_ChooserClick(object sender, EventArgs e)
        {
            frmParts chooseParts = new frmParts();
            chooseParts.ShowDialog();
            if (!string.IsNullOrEmpty(chooseParts.PartsID))
            {
                txtparts_code.Text = chooseParts.PartsCode;
                txtparts_name.Text = chooseParts.PartsName;
            }
        }
        /// <summary>
        /// 分页事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void winFormPager1_PageIndexChanged(object sender, EventArgs e)
        {
            BindgvYTPurchaseOrderList();
        }
        /// <summary>
        /// 选择公司事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ddlCompany_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(ddlCompany.SelectedValue.ToString()))
            {
                CommonFuncCall.BindDepartment(ddlDepartment, ddlCompany.SelectedValue.ToString(), "全部");
            }
            else
            {
                CommonFuncCall.BindDepartment(ddlDepartment, "", "全部");
                CommonFuncCall.BindHandle(ddlhandle, "", "全部");
            }
        }
        /// <summary>
        /// 选择部门事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ddlDepartment_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(ddlDepartment.SelectedValue.ToString()))
            {
                CommonFuncCall.BindHandle(ddlhandle, ddlDepartment.SelectedValue.ToString(), "全部");
            }
            else
            {
                CommonFuncCall.BindHandle(ddlhandle, "", "全部");
            }
        }
        /// <summary>
        /// 双击列表查看明细事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void gvYTPurchaseOrderList_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex > -1 && e.ColumnIndex > -1)//双击表头或列头时不起作用   
            {
                string order_status = this.gvYTPurchaseOrderList.CurrentRow.Cells["order_status"].Value.ToString();
                string order_Id = this.gvYTPurchaseOrderList.CurrentRow.Cells["purchase_order_yt_id"].Value.ToString();
                UCYTView UCPurchaseOrderView = new UCYTView(order_Id, this);
                base.addUserControl(UCPurchaseOrderView, "宇通采购订单-查看", "UCPurchaseOrderView" + order_Id + "", this.Tag.ToString(), this.Name);
            }
        }
        /// <summary>
        /// 列表单元格格式化内容事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void gvYTPurchaseOrderList_CellFormatting(object sender, DataGridViewCellFormattingEventArgs e)
        {
            if (e.Value == null || e.Value.ToString().Length == 0)
            {
                return;
            }
            string order_date = gvYTPurchaseOrderList.Columns[e.ColumnIndex].DataPropertyName;
            if (order_date.Equals("order_date"))
            {
                long ticks = (long)e.Value;
                e.Value = Common.UtcLongToLocalDateTime(ticks).ToShortDateString();
            }
            string req_delivery_time = gvYTPurchaseOrderList.Columns[e.ColumnIndex].DataPropertyName;
            if (req_delivery_time.Equals("req_delivery_time"))
            {
                long ticks = (long)e.Value;
                e.Value = Common.UtcLongToLocalDateTime(ticks).ToShortDateString();
            }
            string fieldNmae = gvYTPurchaseOrderList.Columns[e.ColumnIndex].DataPropertyName;
            if (fieldNmae.Equals("order_status_name"))
            {
                string num = gvYTPurchaseOrderList.Rows[e.RowIndex].Cells["order_status"].Value.ToString();
                num = string.IsNullOrEmpty(num) ? "0" : num;
                if (num == "3")
                { gvYTPurchaseOrderList.Rows[e.RowIndex].Cells["order_status_name"].Style.ForeColor = Color.Red; }
                else
                { gvYTPurchaseOrderList.Rows[e.RowIndex].Cells["order_status_name"].Style.ForeColor = Color.Black; }
            }
            //string fieldNmae1 = gvYTPurchaseOrderList.Columns[e.ColumnIndex].DataPropertyName;
            //if (fieldNmae1.Equals("reality_arrival_date") || fieldNmae1.Equals("reality_arrival_date"))
            //{
            //    long ticks = (long)e.Value;
            //    e.Value = Common.UtcLongToLocalDateTime(ticks);
            //}
            //string fieldNmae2 = gvYTPurchaseOrderList.Columns[e.ColumnIndex].DataPropertyName;
            //if (fieldNmae1.Equals("reality_arrival_date") || fieldNmae1.Equals("reality_arrival_date"))
            //{
            //    long ticks = (long)e.Value;
            //    e.Value = Common.UtcLongToLocalDateTime(ticks);
            //}
        }
        /// <summary> 查看配送单事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void gvYTPurchaseOrderList_CellClick(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex > -1 && e.ColumnIndex > -1)//双击表头或列头时不起作用   
            {
                string viewfile = gvYTPurchaseOrderList.Columns[e.ColumnIndex].DataPropertyName;
                if (viewfile == "viewfile")
                {
                    string YTOrder_num = this.gvYTPurchaseOrderList.CurrentRow.Cells["order_num"].Value.ToString();
                    string BusinessCount = this.gvYTPurchaseOrderList.CurrentRow.Cells["application_count"].Value.ToString();
                    string dsn_adjustable_parts = this.gvYTPurchaseOrderList.CurrentRow.Cells["crm_bill_id"].Value.ToString();
                    frmDistributionView frm = new frmDistributionView(YTOrder_num, BusinessCount, dsn_adjustable_parts);
                    frm.ShowDialog();
                }
            }
        }
        #endregion

        #region 方法、函数
        /// <summary>
        /// 组合查询条件
        /// </summary>
        /// <returns></returns>
        string BuildString()
        {
            string Str_Where = " enable_flag=1  ";
            if (!string.IsNullOrEmpty(txtorder_num.Caption.Trim()))
            {
                Str_Where += " and order_num like '%" + txtorder_num.Caption.Trim() + "%'";
            }
            if (!string.IsNullOrEmpty(ddlorder_type.SelectedValue.ToString()))
            {
                Str_Where += " and order_type='" + ddlorder_type.SelectedValue.ToString() + "'";
            }
            if (!string.IsNullOrEmpty(ddlallot_type.SelectedValue.ToString()))
            {
                Str_Where += " and allot_type='" + ddlallot_type.SelectedValue.ToString() + "'";
            }
            if (!string.IsNullOrEmpty(ddlemergency_level.SelectedValue.ToString()))
            {
                Str_Where += " and emergency_level='" + ddlemergency_level.SelectedValue.ToString() + "'";
            }
            if (!string.IsNullOrEmpty(txtparts_code.Text.Trim()))
            {
                Str_Where += " and parts_codes like '%" + txtparts_code.Text.Trim() + "%'";
            }
            if (!string.IsNullOrEmpty(txtparts_name.Caption.Trim()))
            {
                Str_Where += " and parts_names like '%" + txtparts_name.Caption.Trim() + "%'";
            }
            if (!string.IsNullOrEmpty(ddlorder_status.SelectedValue.ToString()))
            {
                Str_Where += " and order_status='" + ddlorder_status.SelectedValue.ToString() + "'";
            }
            if (!string.IsNullOrEmpty(ddlCompany.SelectedValue.ToString()))
            {
                Str_Where += " and com_id='" + ddlCompany.SelectedValue.ToString() + "'";
            }
            if (!string.IsNullOrEmpty(ddlDepartment.SelectedValue.ToString()))
            {
                Str_Where += " and org_id='" + ddlDepartment.SelectedValue.ToString() + "'";
            }
            if (!string.IsNullOrEmpty(ddlhandle.SelectedValue.ToString()))
            {
                Str_Where += " and handle='" + ddlhandle.SelectedValue.ToString() + "'";
            }
            if (!string.IsNullOrEmpty(txtRemark.Caption.Trim()))
            {
                Str_Where += " and remark like '%" + txtRemark.Caption.Trim() + "%'";
            }
            if (!string.IsNullOrEmpty(dateTimeStart.Value))
            {
                DateTime dtime = Convert.ToDateTime(Convert.ToDateTime(dateTimeStart.Value).ToShortDateString() + " 00:00:00");
                Str_Where += " and apply_date_time>=" + Common.LocalDateTimeToUtcLong(dtime);
            }
            if (!string.IsNullOrEmpty(dateTimeEnd.Value))
            {
                DateTime dtime = Convert.ToDateTime(Convert.ToDateTime(dateTimeEnd.Value).ToShortDateString() + " 23:59:59");
                Str_Where += " and apply_date_time<=" + Common.LocalDateTimeToUtcLong(dtime);
            }
            if (dateTimeReqDeliveryTimeStart.Value != null)
            {
                DateTime dtime = Convert.ToDateTime(dateTimeReqDeliveryTimeStart.Value.ToShortDateString() + " 00:00:00");
                Str_Where += " and req_delivery_time>=" + Common.LocalDateTimeToUtcLong(dtime);
            }
            if (dateTimeReqDeliveryTimeEnd.Value != null)
            {
                DateTime dtime = Convert.ToDateTime(dateTimeReqDeliveryTimeEnd.Value.ToShortDateString() + " 23:59:59");
                Str_Where += " and req_delivery_time<=" + Common.LocalDateTimeToUtcLong(dtime);
            }
            return Str_Where;
        }
        /// <summary>
        /// 获取gvYTPurchaseOrderList列表选中的记录条数
        /// </summary>
        /// <returns></returns>
        private List<string> GetSelectedRecord()
        {
            List<string> listField = new List<string>();
            foreach (DataGridViewRow dr in gvYTPurchaseOrderList.Rows)
            {
                object isCheck = dr.Cells["colCheck"].EditedFormattedValue;
                if (isCheck != null && (bool)isCheck)
                {
                    listField.Add(dr.Cells["purchase_order_yt_id"].Value.ToString());
                }
            }
            return listField;
        }
        /// <summary>
        /// 获取gvYTPurchaseOrderList列表选中的记录条数
        /// </summary>
        /// <returns></returns>
        private List<string> GetSelectedRecordByEditDelete(ref bool IsHandle)
        {
            List<string> listField = new List<string>();
            foreach (DataGridViewRow dr in gvYTPurchaseOrderList.Rows)
            {
                object isCheck = dr.Cells["colCheck"].EditedFormattedValue;
                if (isCheck != null && (bool)isCheck)
                {
                    listField.Add(dr.Cells["purchase_order_yt_id"].Value.ToString());
                    //string import_status = dr.Cells["is_occupy"].Value.ToString();
                    //if (import_status == "0")
                    //{ listField.Add(dr.Cells["purchase_order_yt_id"].Value.ToString()); }
                    //else if (import_status == "1")
                    //{
                    //    IsHandle = false;
                    //    MessageBoxEx.Show("单号为：" + dr.Cells["order_num"].Value.ToString() + "的单据，已经被占用,暂时不可操作!");
                    //    return listField;
                    //}
                    //else if (import_status == "2")
                    //{
                    //    IsHandle = false;
                    //    MessageBoxEx.Show("单号为：" + dr.Cells["order_num"].Value.ToString() + "的单据，已经被锁定,不可以再次操作!");
                    //    return listField;
                    //}
                }
            }
            return listField;
        }
        /// <summary>
        /// 获取gvYTPurchaseOrderList列表选中要审核的记录
        /// 只有工单状态是已提交的才可以被审核
        /// </summary>
        /// <returns></returns>
        private List<string> GetVerifyRecord()
        {
            List<string> listField = new List<string>();
            foreach (DataGridViewRow dr in gvYTPurchaseOrderList.Rows)
            {
                object isCheck = dr.Cells["colCheck"].EditedFormattedValue;
                if (isCheck != null && (bool)isCheck)
                {
                    //获取已提交/审核未通过的状态的编号
                    string order_status_SUBMIT = Convert.ToInt32(DataSources.EnumAuditStatus.SUBMIT).ToString();
                    string order_status_NOTAUDIT = Convert.ToInt32(DataSources.EnumAuditStatus.NOTAUDIT).ToString();
                    string colorder_status = dr.Cells["order_status"].Value.ToString();
                    if (order_status_SUBMIT == colorder_status || order_status_NOTAUDIT == colorder_status)
                    {
                        listField.Add(dr.Cells["purchase_order_yt_id"].Value.ToString());
                    }
                }
            }
            return listField;
        }
        /// <summary>
        /// 加载采购订单列表信息
        /// </summary>
        public void BindgvYTPurchaseOrderList()
        {
            try
            {
                int RecordCount = 0;
                DataTable gvYTPurchaseOrder_dt = DBHelper.GetTableByPage("查询宇通采购订单列表信息", "tb_parts_purchase_order_2", "*,'查看' as viewfile", BuildString(), "", " order by create_time desc ", winFormPager1.PageIndex, winFormPager1.PageSize, out RecordCount);
                gvYTPurchaseOrderList.DataSource = gvYTPurchaseOrder_dt;
                winFormPager1.RecordCount = RecordCount;
            }
            catch (Exception ex)
            {
                //异常日志
            }
        }
        /// <summary> 通过审核后向宇通发送宇通采购订单信息
        /// </summary>
        /// <param name="listField">宇通采购订单号集合</param>
        void DealPurascherToYT(List<string> listField)
        {
            try
            {
                if (listField.Count > 0)
                {
                    for (int a = 0; a < listField.Count; a++)
                    {
                        DataTable dt = DBHelper.GetTable("查看一条宇通采购订单信息", "tb_parts_purchase_order_2", "*", " purchase_order_yt_id='" + listField[a] + "'", "", "");
                        if (dt != null && dt.Rows.Count > 0)
                        {
                            yt_purchaseorder_model = new tb_parts_purchase_order_2();
                            yt_purchaseorder_model.listDetails = new List<tb_parts_purchase_order_p_2>();
                            CommonFuncCall.SetModlByDataTable(yt_purchaseorder_model, dt);
                            DataTable dt_parts = DBHelper.GetTable("查看宇通采购订单配件信息", "tb_parts_purchase_order_p_2", "*", " purchase_order_yt_id='" + listField[a] + "'", "", "");
                            if (dt_parts != null && dt_parts.Rows.Count > 0)
                            {
                                for (int i = 0; i < dt_parts.Rows.Count; i++)
                                {
                                    yt_partsorder_model = new tb_parts_purchase_order_p_2();
                                    CommonFuncCall.SetModlByDataTable(yt_partsorder_model, dt_parts, i);
                                    yt_purchaseorder_model.listDetails.Add(yt_partsorder_model);
                                }
                            }
                            if (yt_purchaseorder_model.crm_bill_id == ".")
                            {
                                yt_purchaseorder_model.crm_bill_id = string.Empty;
                            }
                            DBHelper.WebServHandler("", EnumWebServFunName.UpLoadPartPurchase, yt_purchaseorder_model);
                            
                        }
                    }
                }
            }
            catch (Exception ex)
            { }
        }
        /// <summary> 提交时获取当前配件列表中存在的引用单号,保存到中间表中
        /// 并生成执行的sql
        /// </summary>
        /// <returns></returns>
        void GetPre_Order_Code(List<SysSQLString> listSql, string purchase_order_yt_id, string post_order_id, string post_order_code)
        {
            List<string> list = new List<string>();
            SysSQLString sysStringSql = new SysSQLString();
            sysStringSql.cmdType = CommandType.Text;
            Dictionary<string, string> dic = new Dictionary<string, string>();
            string sql1 = "delete from tr_order_relation where post_order_id=@post_order_id ;";
            dic.Add("post_order_id", post_order_id);
            dic.Add("post_order_code", post_order_code);
            sysStringSql.sqlString = sql1;
            sysStringSql.Param = dic;
            listSql.Add(sysStringSql);

            DataTable dt_relation_order = DBHelper.GetTable("查询宇通采购订单配件表的引用单号", "tb_parts_purchase_order_p_2", " purchase_order_yt_id,relation_order ", " purchase_order_yt_id='" + purchase_order_yt_id + "'", "", "");
            if (dt_relation_order != null && dt_relation_order.Rows.Count > 0)
            {
                foreach (DataRow dr in dt_relation_order.Rows)
                {
                    string relation_order = dr["relation_order"] == null ? "" : dr["relation_order"].ToString();
                    if (!string.IsNullOrEmpty(relation_order))
                    {
                        if (!list.Contains(relation_order))
                        {
                            list.Add(relation_order);
                            sysStringSql = new SysSQLString();
                            sysStringSql.cmdType = CommandType.Text;
                            dic = new Dictionary<string, string>();
                            dic.Add("order_relation_id", Guid.NewGuid().ToString());
                            dic.Add("pre_order_id", string.Empty);
                            dic.Add("pre_order_code", relation_order);
                            dic.Add("post_order_id", post_order_id);
                            dic.Add("post_order_code", post_order_code);
                            string sql2 = string.Format(@"Insert Into tr_order_relation(order_relation_id,pre_order_id,pre_order_code,
                                                      post_order_id,post_order_code)  values(@order_relation_id,@pre_order_id,
                                                      @pre_order_code,@post_order_id,@post_order_code);");
                            sysStringSql.sqlString = sql2;
                            sysStringSql.Param = dic;
                            listSql.Add(sysStringSql);
                        }
                    }
                }
            }
        }
        #endregion

        #region 点击行选中复选框的，控制工具栏按钮是否可用的功能代码
        /// <summary> 选中列标头的复选框事件
        /// </summary>
        private void gvYTPurchaseOrderList_HeadCheckChanged()
        {
            SetSelectedStatus();
        }
        /// <summary> 选择复选框
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void gvYTPurchaseOrderList_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {
            if (gvYTPurchaseOrderList.CurrentCell == null)
            {
                return;
            }
            //点击选择框
            if (gvYTPurchaseOrderList.CurrentCell.OwningColumn.Name == colCheck.Name)
            {
                SetSelectedStatus();
            }
        }
        /// <summary> 单击一行，选择或取消选择
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void gvYTPurchaseOrderList_CellMouseClick(object sender, DataGridViewCellMouseEventArgs e)
        {
            if (e.RowIndex < 0 || e.ColumnIndex < 0)
            {
                return;
            }
            if (e.ColumnIndex == colCheck.Index)
            {
                return;
            }
            //清空已选择框
            foreach (DataGridViewRow dgvr in gvYTPurchaseOrderList.Rows)
            {
                object check = dgvr.Cells[colCheck.Name].EditedFormattedValue;
                if (check != null && (bool)check)
                {
                    dgvr.Cells[colCheck.Name].Value = false;
                }
            }
            //选择当前行
            gvYTPurchaseOrderList.Rows[e.RowIndex].Cells[colCheck.Name].Value = true;
            SetSelectedStatus();
        }
        /// <summary> 设置选择项后状态
        /// </summary>
        void SetSelectedStatus()
        {
            listIDs.Clear();
            //已选择状态列表
            List<string> listFiles = new List<string>();
            foreach (DataGridViewRow dgvr in gvYTPurchaseOrderList.Rows)
            {
                if (Convert.ToBoolean(dgvr.Cells[colCheck.Name].EditedFormattedValue))
                {
                    listFiles.Add(dgvr.Cells[order_status.Name].Value.ToString());
                    listIDs.Add(dgvr.Cells[purchase_order_yt_id.Name].Value.ToString());
                }
            }

            //提交
            string submitStr = ((int)DataSources.EnumAuditStatus.SUBMIT).ToString();
            //审核
            string auditStr = ((int)DataSources.EnumAuditStatus.AUDIT).ToString();
            //草稿
            string draftStr = ((int)DataSources.EnumAuditStatus.DRAFT).ToString();
            //审核未通过
            string noAuditStr = ((int)DataSources.EnumAuditStatus.NOTAUDIT).ToString();
            //作废
            string invalid = ((int)DataSources.EnumAuditStatus.Invalid).ToString();
            //复制按钮，只选择一个并且不是作废，可以复制
            if (listFiles.Count == 1 && !listFiles.Contains(invalid))
            {
                btnCopy.Enabled = true;
            }
            else
            {
                btnCopy.Enabled = false;
            }
            //编辑按钮，只选择一个并且是草稿或未通过状态，可以编辑
            if (listFiles.Count == 1 && (listFiles.Contains(draftStr) || listFiles.Contains(noAuditStr)))
            {
                btnEdit.Enabled = true;
            }
            else
            {
                btnEdit.Enabled = false;
            }
            //判断”审核“按钮是否可用
            if (listFiles.Contains(auditStr) || listFiles.Contains(draftStr) || listFiles.Contains(noAuditStr) || listFiles.Contains(invalid))
            {
                btnVerify.Enabled = false;
            }
            else
            {
                btnVerify.Enabled = true;
            }
            //包含已审核、已提交、已作废状态，提交、删除按钮不可用
            if (listFiles.Contains(auditStr) || listFiles.Contains(submitStr) || listFiles.Contains(invalid))
            {
                btnSubmit.Enabled = false;
                btnDelete.Enabled = false;
            }
            else
            {
                btnSubmit.Enabled = true;
                btnDelete.Enabled = true;
            }

            if (listFiles.Contains(invalid))
            {

            }
        } 
        #endregion

        #region 提交成功后对前置单据状态和完成数量进行修改的功能代码
        /// <summary> 提交成功时,对引用的前置单据的状态进行更新 
        /// </summary>
        /// <param name="orderid"></param>
        bool SetOrderStatus(string orderid)
        {
            bool ret = false;
            try
            {
                #region 设置前置单据的状态和完成数量
                //前置单据中的配件信息是否在后置单据中全部导入完成（完成数量>=计划数量）
                List<OrderImportStatus> list_order = new List<OrderImportStatus>();
                List<OrderFinishInfo> list_orderinfo = new List<OrderFinishInfo>();
                OrderImportStatus orderimport_model = new OrderImportStatus();
                OrderFinishInfo orderfinish_info = new OrderFinishInfo();

                DataTable dt_Business = GetBusinessCount(orderid);
                DataTable dt_Finish = GetFinishCount(orderid);

                string plan_id = string.Empty;
                string order_num = string.Empty;
                string parts_code = string.Empty;
                string importtype = string.Empty;
                if (dt_Business.Rows.Count > 0)
                {
                    for (int i = 0; i < dt_Business.Rows.Count; i++)
                    {
                        bool isfinish = true;
                        decimal BusinessCount = decimal.Parse(dt_Business.Rows[i]["business_counts"].ToString());
                        plan_id = dt_Business.Rows[i]["plan_id"].ToString();
                        order_num = dt_Business.Rows[i]["order_num"].ToString();
                        parts_code = dt_Business.Rows[i]["parts_code"].ToString();
                        DataRow[] dr = null;
                        if (dt_Finish != null && dt_Finish.Rows.Count > 0)
                        {
                            dr = dt_Finish.Select(" order_num='" + order_num + "' and parts_code='" + parts_code + "'");
                        }
                        if (dr != null && dr.Count() > 0)
                        {
                            importtype = dr[0]["ImportType"].ToString();

                            orderfinish_info = new OrderFinishInfo();
                            orderfinish_info.plan_id = plan_id;
                            orderfinish_info.parts_code = parts_code;
                            orderfinish_info.finish_num = dr[0]["relation_count"].ToString();
                            orderfinish_info.importtype = importtype;
                            list_orderinfo.Add(orderfinish_info);
                            if (decimal.Parse(dr[0]["relation_count"].ToString()) < BusinessCount)
                            {
                                isfinish = false;
                            }
                        }
                        else
                        {
                            orderfinish_info = new OrderFinishInfo();
                            orderfinish_info.plan_id = plan_id;
                            orderfinish_info.parts_code = parts_code;
                            orderfinish_info.finish_num = "0";
                            orderfinish_info.importtype = importtype;
                            list_orderinfo.Add(orderfinish_info);

                            isfinish = false;
                        }

                        orderimport_model = new OrderImportStatus();
                        orderimport_model.order_num = order_num;
                        orderimport_model.importtype = importtype;
                        orderimport_model.isfinish = isfinish;
                        if (list_order.Count > 0)
                        {
                            if (list_order.Where(p => p.order_num == order_num).Count() > 0)
                            {
                                if (!isfinish)
                                {
                                    for (int a = 0; a < list_order.Count; a++)
                                    {
                                        if (list_order[a].order_num == order_num && list_order[a].isfinish)
                                        { list_order[a].isfinish = isfinish; }
                                    }
                                }
                            }
                            else
                            { list_order.Add(orderimport_model); }
                        }
                        else
                        { list_order.Add(orderimport_model); }
                    }
                }
                ret = ImportPurchasePlanStatus(list_order, list_orderinfo);
                #endregion
            }
            catch (Exception ex)
            { }
            return ret;
        }
        /// <summary> 获取各个前置单据中配件业务数量在后置单据中的已完成的数量
        /// </summary>
        DataTable GetFinishCount(string purchase_order_yt_id)
        {
            DataTable dt = null;
            try
            {
                string files = string.Empty;
                DataTable dt_relation_order = DBHelper.GetTable("查询采购订单配件表的引用单号", "tb_parts_purchase_order_p_2", " purchase_order_yt_id,relation_order ", " purchase_order_yt_id='" + purchase_order_yt_id + "'", "", "");
                if (dt_relation_order != null && dt_relation_order.Rows.Count > 0)
                {
                    foreach (DataRow dr in dt_relation_order.Rows)
                    {
                        string relation_order = dr["relation_order"] == null ? "" : dr["relation_order"].ToString();
                        if (!string.IsNullOrEmpty(relation_order))
                        {
                            if (!files.Contains("'" + relation_order + "',"))
                            {
                                files += "'" + relation_order + "',";
                            }
                        }
                    }
                    files = files.Trim(',');
                    if (files.Trim().Length > 0)
                    {
                        files = " where a.order_num in (" + files + ")";
                    }
                    string FileName = string.Format(@" * ");
                    string TableName = string.Format(@" (
                                                        select 
                                                        tb_plan.order_num,tb_plan.parts_code,sum(tb_order.application_count) relation_count,tb_order.ImportType
                                                         from
                                                         (
                                                           select b.relation_order,b.parts_code,b.application_count,b.ImportType from tb_parts_purchase_order_2 a 
                                                           inner join tb_parts_purchase_order_p_2 b on a.purchase_order_yt_id=b.purchase_order_yt_id
                                                           where a.order_status in ('1','2')
                                                         ) tb_order
                                                        left join 
                                                         (
                                                           select a.order_num,b.parts_code,b.business_counts from tb_parts_purchase_plan a 
                                                           inner join tb_parts_purchase_plan_p b on a.plan_id=b.plan_id {0}
                                                         ) tb_plan 
                                                         on tb_plan.order_num=tb_order.relation_order and tb_plan.parts_code=tb_order.parts_code 
                                                         where len(tb_plan.order_num)>0 and LEN(tb_plan.parts_code)>0
                                                        group by tb_plan.order_num,tb_plan.parts_code
                                                        ,tb_order.ImportType
                                                        union
                                                        select 
                                                         tb_sale_order.order_num,tb_sale_order.parts_code,sum(tb_pur_order.application_count) relation_count,tb_pur_order.ImportType
                                                         from
                                                         (
                                                          select b.relation_order,b.parts_code,b.application_count,b.ImportType from tb_parts_purchase_order_2 a 
                                                          inner join tb_parts_purchase_order_p_2 b on a.purchase_order_yt_id=b.purchase_order_yt_id
                                                          where a.order_status in ('1','2')
                                                         ) tb_pur_order
                                                        left join 
                                                        (
                                                          select a.order_num,b.parts_code,b.business_count from tb_parts_sale_order a 
                                                          inner join tb_parts_sale_order_p b on a.sale_order_id=b.sale_order_id {0}
                                                        ) tb_sale_order 
                                                         on tb_sale_order.order_num=tb_pur_order.relation_order and tb_sale_order.parts_code=tb_pur_order.parts_code 
                                                         where len(tb_sale_order.order_num)>0 and LEN(tb_sale_order.parts_code)>0
                                                         group by tb_sale_order.order_num,tb_sale_order.parts_code
                                                        ,tb_pur_order.ImportType
                                                    ) tb_pur_order_finishcount ", files);
                    return dt = DBHelper.GetTable("查询宇通采购订单导入采购计划单时,获取计划单中配件已完成的数量", TableName, FileName, "", "", "");
                }
                return dt;
            }
            catch (Exception ex)
            { return dt; }
            finally { }
        }
        /// <summary> 获取前置单据的业务信息
        /// </summary>
        /// <param name="order_id"></param>
        /// <returns></returns>
        DataTable GetBusinessCount(string purchase_order_yt_id)
        {
            DataTable dt = null;
            try
            {
                string FileName = string.Format(@" * ");
                string TableName = string.Format(@" (
	                                                    select a.plan_id,b.order_num,a.parts_code,a.business_counts from
	                                                    (
		                                                     select * from tb_parts_purchase_plan_p where plan_id in
		                                                     (
			                                                     select plan_id from tb_parts_purchase_plan where order_num in
			                                                     (
				                                                    select relation_order from tb_parts_purchase_order_p_2 
				                                                    where purchase_order_yt_id='{0}' and len(relation_order)>0 group by relation_order
			                                                     )
		                                                     )
	                                                    ) a left join tb_parts_purchase_plan b on a.plan_id=b.plan_id
	                                                    union
	                                                    select a.sale_order_id as plan_id,b.order_num,a.parts_code,a.business_count as business_counts from
	                                                    (
		                                                     select * from tb_parts_sale_order_p where sale_order_id in
		                                                     (
			                                                     select sale_order_id from tb_parts_sale_order where order_num in
			                                                     (
				                                                    select relation_order from tb_parts_purchase_order_p_2 
				                                                    where purchase_order_yt_id='{0}' and len(relation_order)>0 group by relation_order
			                                                     )
		                                                     )
	                                                    ) a left join tb_parts_sale_order b on a.sale_order_id=b.sale_order_id
                                                    ) tb_pur_order_businesscount ", purchase_order_yt_id);
                 dt = DBHelper.GetTable("查询宇通采购订单导入采购计划单时,获取计划单中配件已完成的数量", TableName, FileName, "", "", "");
            }
            catch (Exception ex)
            {
                
            }
            return dt;
        }
        /// <summary> 对引用的前置单据的状态进行更新的方法
        /// </summary>
        /// <param name="list_order"></param>
        bool ImportPurchasePlanStatus(List<OrderImportStatus> list_order, List<OrderFinishInfo> list_orderinfo)
        {
            bool ret = false;
            string plan_ids = string.Empty;
            SysSQLString sysStringSql = new SysSQLString();
            sysStringSql.cmdType = CommandType.Text;
            List<SysSQLString> listSql = new List<SysSQLString>();
            Dictionary<string, string> dic = new Dictionary<string, string>();
            List<string> list_plan = new List<string>();
            try
            {
                #region 更新前置单据的导入状态字段
                foreach (OrderImportStatus item in list_order)
                {
                    if (item.importtype == "采购计划单")
                    {
                        sysStringSql = new SysSQLString();
                        sysStringSql.cmdType = CommandType.Text;
                        dic = new Dictionary<string, string>();
                        string sql1 = "update tb_parts_purchase_plan set import_status=@import_status where order_num=@order_num;";
                        dic.Add("import_status", !item.isfinish ? "2" : "3");//单据导入状态，0正常，1占用，2锁定(部分导入), 3锁定(全部导入)
                        dic.Add("order_num", item.order_num);
                        sysStringSql.sqlString = sql1;
                        sysStringSql.Param = dic;
                        listSql.Add(sysStringSql);
                    }
                    else if (item.importtype == "销售订单")
                    {
                        sysStringSql = new SysSQLString();
                        sysStringSql.cmdType = CommandType.Text;
                        dic = new Dictionary<string, string>();
                        string sql1 = "update tb_parts_sale_order set is_occupy=@is_occupy where order_num=@order_num;";
                        dic.Add("is_occupy", !item.isfinish ? "2" : "3");//单据导入状态，0正常，1占用，2锁定(部分导入), 3锁定(全部导入)
                        dic.Add("order_num", item.order_num);
                        sysStringSql.sqlString = sql1;
                        sysStringSql.Param = dic;
                        listSql.Add(sysStringSql);
                    }
                }
                #endregion

                #region 更新前置单据中的各个配件的已完成数量
                foreach (OrderFinishInfo item in list_orderinfo)
                {
                    if (!list_plan.Contains(item.plan_id))
                    {
                        list_plan.Add(item.plan_id);
                        plan_ids = plan_ids + "'" + item.plan_id + "',";
                    }
                    if (item.importtype == "采购计划单")
                    {
                        sysStringSql = new SysSQLString();
                        sysStringSql.cmdType = CommandType.Text;
                        dic = new Dictionary<string, string>();
                        string sql1 = "update tb_parts_purchase_plan_p set finish_counts=@finish_counts where plan_id=@plan_id and parts_code=@parts_code;";
                        dic.Add("finish_counts", item.finish_num);
                        dic.Add("plan_id", item.plan_id);
                        dic.Add("parts_code", item.parts_code);
                        sysStringSql.sqlString = sql1;
                        sysStringSql.Param = dic;
                        listSql.Add(sysStringSql);
                    }
                    else if (item.importtype == "销售订单")
                    {
                        sysStringSql = new SysSQLString();
                        sysStringSql.cmdType = CommandType.Text;
                        dic = new Dictionary<string, string>();
                        string sql1 = "update tb_parts_sale_order_p set finish_count=@finish_count where sale_order_id=@sale_order_id and parts_code=@parts_code;";
                        dic.Add("finish_count", item.finish_num);
                        dic.Add("sale_order_id", item.plan_id);
                        dic.Add("parts_code", item.parts_code);
                        sysStringSql.sqlString = sql1;
                        sysStringSql.Param = dic;
                        listSql.Add(sysStringSql);
                    }
                }
                #endregion
                ret = DBHelper.BatchExeSQLStringMultiByTrans("提交宇通采购订单，更新引用的采购计划单或销售订单的导入状态", listSql);
                if (ret)
                {
                    #region 更新采购计划单的完成金额和完成数量
                    if (list_orderinfo.Count > 0)
                    {
                        listSql.Clear();
                        plan_ids = plan_ids.Trim(',');
                        string TableName = string.Format(@"
                        (
                            select plan_id,sum(finish_counts) finish_counts,
                            sum(finish_counts*business_price) as finish_money 
                                from tb_parts_purchase_plan_p 
                            where plan_id in ({0})
                            group by plan_id
                        ) tb_purchase_finish", plan_ids);
                        DataTable dt = DBHelper.GetTable("查询采购计划单各配件完成数量和完成金额", TableName, "*", "", "", "");
                        if (dt != null && dt.Rows.Count > 0)
                        {
                            for (int i = 0; i < dt.Rows.Count; i++)
                            {
                                sysStringSql = new SysSQLString();
                                sysStringSql.cmdType = CommandType.Text;
                                dic = new Dictionary<string, string>();
                                string sql1 = "update tb_parts_purchase_plan set finish_counts=@finish_counts,plan_finish_money=@plan_finish_money where plan_id=@plan_id;";
                                dic.Add("finish_counts", dt.Rows[i]["finish_counts"].ToString());
                                dic.Add("plan_finish_money", dt.Rows[i]["finish_money"].ToString());
                                dic.Add("plan_id", dt.Rows[i]["plan_id"].ToString());
                                sysStringSql.sqlString = sql1;
                                sysStringSql.Param = dic;
                                listSql.Add(sysStringSql);
                            }
                            DBHelper.BatchExeSQLStringMultiByTrans("完成采购订单后，更新采购计划单的完成数量和完成金额", listSql);
                        }
                    }
                    #endregion
                }
            }
            catch (Exception ex)
            { }
            return ret;
        }
        #endregion

        #region --选择器获取数据后需执行的回调函数
        /// <summary> 配件编码速查关联控件赋值
        /// </summary>
        /// <param name="dr"></param>
        private void PartsName_DataBack(DataRow dr)
        {
            if (dr.Table.Columns.Contains("parts_name"))
            {
                this.txtparts_name.Caption = dr["parts_name"].ToString();
            }
        }
        #endregion
    }
}
