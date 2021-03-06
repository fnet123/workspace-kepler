﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Utility.Common;
using HXCPcClient.CommonClass;
using ServiceStationClient.ComponentUI;
using SYSModel;
using HXCPcClient.Chooser;
using System.Collections.ObjectModel;
using System.Drawing.Printing;
using System.IO;
using Utility.CommonForm;

namespace HXCPcClient.UCForm.FinancialManagement.Receivable
{
    public partial class UCReceivableManage : UCBase
    {
        #region 属性
        DataSources.EnumOrderType orderType = DataSources.EnumOrderType.PAYMENT;
        protected bool isSearch = false;//是否是查询页面
        List<string> listIDs = new List<string>();//已选择ID
        /// <summary>
        /// 已选择应收应付ID
        /// </summary>
        private string ID
        {
            get
            {
                //if (dgvBillReceivable.CurrentRow == null)
                //{
                //    return string.Empty;
                //}
                //object id = dgvBillReceivable.CurrentRow.Cells["colPayableSingleID"].Value;
                //if (id == null)
                //{
                //    return string.Empty;
                //}
                //else
                //{
                //    return id.ToString();
                //}
                if (listIDs.Count == 1)
                {
                    return listIDs[0];
                }
                else
                {
                    return string.Empty;
                }
            }
        }
        /// <summary>
        /// 标题
        /// </summary>
        string Title
        {
            get
            {
                if (orderType == DataSources.EnumOrderType.PAYMENT)
                {
                    return "财务付款单";
                }
                else
                {
                    return "财务收款单";
                }
            }
        }
        DataTable dtData;//当前查出的数据
        bool isPreview = false;//是否预览
        BusinessPrint businessPrint;//业务打印功能
        #endregion
        public UCReceivableManage(DataSources.EnumOrderType orderType)
        {
            InitializeComponent();
            this.AddEvent += new ClickHandler(UCReceivableManage_AddEvent);
            this.EditEvent += new ClickHandler(UCReceivableManage_EditEvent);
            this.CopyEvent += new ClickHandler(UCReceivableManage_CopyEvent);
            this.DeleteEvent += new ClickHandler(UCReceivableManage_DeleteEvent);
            this.ViewEvent += new ClickHandler(UCReceivableManage_ViewEvent);
            this.VerifyEvent += new ClickHandler(UCReceivableManage_VerifyEvent);
            this.SubmitEvent += new ClickHandler(UCReceivableManage_SubmitEvent);
            this.PrintEvent += new ClickHandler(UCReceivableManage_PrintEvent);
            this.ExportEvent += new ClickHandler(UCReceivableManage_ExportEvent);
            this.orderType = orderType;
            DataGridViewEx.SetDataGridViewStyle(dgvBillReceivable, colOrderStatus);
            dgvBillReceivable.ReadOnly = false;
            dgvBillReceivable.HeadCheckChanged += new DataGridViewEx.DelegateOnClick(dgvBillReceivable_HeadCheckChanged);
            foreach (DataGridViewColumn dgvc in dgvBillReceivable.Columns)
            {
                if (dgvc.Name == colCheck.Name)
                {
                    continue;
                }
                dgvc.ReadOnly = true;
            }
            SetLable();
            string printObject = "tb_receivable";
            string printTitle = "财务收款单";
            if (orderType == DataSources.EnumOrderType.PAYMENT)
            {
                printObject = "tb_payment";
                printTitle = "财务付款单";
            }
            List<string> listNotPrint = new List<string>();
            listNotPrint.Add(colOrgId.Name);
            listNotPrint.Add(colHandle.Name);
            PaperSize paperSize = new PaperSize();
            paperSize.Width = 297;
            paperSize.Height = 210;
            businessPrint = new BusinessPrint(dgvBillReceivable, printObject, printTitle, paperSize, listNotPrint);
        }

        //页面加载
        private void UCReceivableManage_Load(object sender, EventArgs e)
        {
            UIAssistants.SetButtonStyle4QueryAndClear(btnSearch, btnClear);  //设置查询按钮和清除按钮样式
            #region 工具栏设置
            if (isSearch)//如果是查询
            {
                //设置页面按钮可见性
                var btnCols = new ObservableCollection<ButtonEx_sms>
                {
                    btnView,btnPrint,btnSet,btnExport
                };
                UIAssistants.SetUCBaseFuncationVisible(this, btnCols);
                #region 隐藏右键
                tsmiAddF.Visible = false;
                tsmiEditF.Visible = false;
                tsmiCopyF.Visible = false;
                tsmiDeleteF.Visible = false;
                tsmiSubmitF.Visible = false;
                tsmiVerifyF.Visible = false;
                tss1.Visible = false;
                tss2.Visible = false;
                #endregion
            }
            else
            {
                #region 设置右键权限
                tsmiAddF.Visible = btnAdd.Visible;
                tsmiEditF.Visible = btnEdit.Visible;
                tsmiCopyF.Visible = btnCopy.Visible;
                tsmiDeleteF.Visible = btnDelete.Visible;
                tsmiSubmitF.Visible = btnSubmit.Visible;
                tsmiVerifyF.Visible = btnVerify.Visible;
                tsmiView.Visible = btnView.Visible;
                tsmiPrint.Visible = btnPrint.Visible;
                #endregion
                //设置页面按钮可见性
                //var btnCols = new ObservableCollection<ButtonEx_sms>
                //{
                //    btnAdd, btnCopy, btnEdit, btnDelete,btnSubmit,btnVerify, btnExport, btnPrint,btnSet,btnView,btnRevoke
                //};
                //UIAssistants.SetUCBaseFuncationVisible(this, btnCols);
            }
            #endregion
            dtInterval.StartDate = new DateTime(DateTime.Now.Year, DateTime.Now.Month, 1).ToString(dtInterval.customFormat);
            dtInterval.EndDate = DateTime.Now.ToString(dtInterval.customFormat);
            colCheck.Width = 25;
            BindSearch();
            BindData();
        }

        #region 工具栏事件
        //导出
        void UCReceivableManage_ExportEvent(object sender, EventArgs e)
        {
            try
            {
                string dir = Application.StartupPath + @"\ExportFile";
                if (!Directory.Exists(dir))
                {
                    Directory.CreateDirectory(dir);
                }
                SaveFileDialog sfd = new SaveFileDialog();
                sfd.InitialDirectory = dir;
                sfd.Title = "导出文件";
                sfd.DefaultExt = "xls";
                sfd.Filter = "Microsoft Office Excel 文件(*.xls;*.xlsx)|*.xls;*.xlsx|Microsoft Office Excel 文件(*.xls)|*.xls|Microsoft Office Excel 文件(*.xlsx)|*.xlsx";
                sfd.FileName = dir + @"\" + Title + DateTime.Now.ToString("yyyy-MM-dd") + ".xls";
                DialogResult result = sfd.ShowDialog();
                if (result == DialogResult.OK)
                {
                    string strWhere = GetWhere();
                    DataTable dt = DBHelper.GetTable("查询" + Title, "tb_bill_receivable", "*", strWhere, "", "order by create_time desc");
                    if (dt == null || dt.Rows.Count == 0)
                    {
                        MessageBoxEx.ShowInformation("没有可导出的数据！");
                        return;
                    }
                    dt.DataTableToDate("order_date");
                    if (orderType == DataSources.EnumOrderType.RECEIVABLE)
                    {
                        dt.DateTableToEnum("payment_type", typeof(DataSources.EnumReceivableType));
                    }
                    else
                    {
                        dt.DateTableToEnum("payment_type", typeof(DataSources.EnumPaymentType));
                    }
                    dt.DateTableToEnum("order_status", typeof(DataSources.EnumAuditStatus));
                    PercentProcessOperator process = new PercentProcessOperator();
                    #region 匿名方法，后台线程执行完调用
                    process.BackgroundWork =
                        delegate(Action<int> percent)
                        {
                            dt = ExcelHandler.HandleDataTableForExcel(dt, dgvBillReceivable);
                            ExcelHandler.ExportDTtoExcel(dt, "", sfd.FileName, percent);
                        };
                    #endregion
                    process.MessageInfo = "正在执行中";
                    process.Maximum = dt.Rows.Count;
                    #region 匿名方法，后台线程执行完调用
                    process.BackgroundWorkerCompleted += new EventHandler<BackgroundWorkerEventArgs>(
                            delegate(object osender, BackgroundWorkerEventArgs be)
                            {
                                if (be.BackGroundException == null)
                                {
                                    MessageBoxEx.ShowInformation("导出成功！");
                                }
                                else
                                {
                                    Utility.Log.Log.writeLineToLog("【" + Title + "】" + be.BackGroundException.Message, "client");
                                    MessageBoxEx.ShowWarning("导出出现异常");
                                }
                            }
                        );
                    #endregion
                    process.Start();
                }
            }
            catch (Exception ex)
            {
                Utility.Log.Log.writeLineToLog("【" + Title + "】" + ex.Message, "client");
                MessageBoxEx.ShowWarning("导出失败！");
            }
        }
        //打印事件
        void UCReceivableManage_PrintEvent(object sender, EventArgs e)
        {
            if (!isPreview)
            {
                dtData.DataTableToDate("order_date");
                if (orderType == DataSources.EnumOrderType.RECEIVABLE)
                {
                    dtData.DateTableToEnum("payment_type", typeof(DataSources.EnumReceivableType));
                }
                else
                {
                    dtData.DateTableToEnum("payment_type", typeof(DataSources.EnumPaymentType));
                }
                dtData.DateTableToEnum("order_status", typeof(DataSources.EnumAuditStatus));
                isPreview = true;
            }
            businessPrint.Print(dtData);
        }
        //提交事件
        void UCReceivableManage_SubmitEvent(object sender, EventArgs e)
        {
            SubmitData();
        }
        //审核事件
        void UCReceivableManage_VerifyEvent(object sender, EventArgs e)
        {
            VerifyData();
        }

        //预览事件
        void UCReceivableManage_ViewEvent(object sender, EventArgs e)
        {
            //ViewData();
            if (!isPreview)
            {
                dtData.DataTableToDate("order_date");
                if (orderType == DataSources.EnumOrderType.RECEIVABLE)
                {
                    dtData.DateTableToEnum("payment_type", typeof(DataSources.EnumReceivableType));
                }
                else
                {
                    dtData.DateTableToEnum("payment_type", typeof(DataSources.EnumPaymentType));
                }
                dtData.DateTableToEnum("order_status", typeof(DataSources.EnumAuditStatus));
                isPreview = true;
            }
            businessPrint.Preview(dtData);
        }
        //删除事件
        void UCReceivableManage_DeleteEvent(object sender, EventArgs e)
        {
            DeleteData();
        }
        //复制事件
        void UCReceivableManage_CopyEvent(object sender, EventArgs e)
        {
            EditData(WindowStatus.Copy);
        }
        //编辑事件
        void UCReceivableManage_EditEvent(object sender, EventArgs e)
        {
            EditData(WindowStatus.Edit);
        }
        //新增事件
        void UCReceivableManage_AddEvent(object sender, EventArgs e)
        {
            AddData();
        }
        #endregion

        #region 菜单事件
        //查询
        private void tsmiSearch_Click(object sender, EventArgs e)
        {
            BindData();
        }
        //清除
        private void tsmiClear_Click(object sender, EventArgs e)
        {
            ClearSearch();
        }
        //新建
        private void tsmiAdd_Click(object sender, EventArgs e)
        {
            AddData();
        }
        //编辑
        private void tsmiEdit_Click(object sender, EventArgs e)
        {
            EditData(WindowStatus.Edit);
        }
        //复制
        private void tsmiCopy_Click(object sender, EventArgs e)
        {
            EditData(WindowStatus.Copy);
        }
        //删除
        private void tsmiDelete_Click(object sender, EventArgs e)
        {
            DeleteData();
        }
        //提交
        private void tsmiSubmit_Click(object sender, EventArgs e)
        {
            SubmitData();
        }
        //审核
        private void tsmiVerify_Click(object sender, EventArgs e)
        {
            VerifyData();
        }
        //预览
        private void tsmiView_Click(object sender, EventArgs e)
        {
            ViewData();
        }
        //操作记录
        private void tsmiOperation_Click(object sender, EventArgs e)
        {

        }
        //打印
        private void tsmiPrint_Click(object sender, EventArgs e)
        {

        }
        #endregion

        #region 方法
        #region 工具栏方法
        //提交,只有草稿状态才可以提交
        void SubmitData()
        {
            dgvBillReceivable.EndEdit();
            List<SysSQLString> listSql = new List<SysSQLString>();
            string submit = ((int)DataSources.EnumAuditStatus.SUBMIT).ToString();//提交
            string draft = ((int)DataSources.EnumAuditStatus.DRAFT).ToString();//草稿
            string notAudit = ((int)DataSources.EnumAuditStatus.NOTAUDIT).ToString();//审核失败
            string message = string.Empty;//错误消息
            foreach (DataGridViewRow dgvr in dgvBillReceivable.Rows)
            {
                object isCheck = dgvr.Cells["colCheck"].Value;
                string status = CommonCtrl.IsNullToString(dgvr.Cells[colOrderStatus.Name].Tag);
                if (isCheck != null && (bool)isCheck && (status == draft || status == notAudit))
                {
                    string order_id = dgvr.Cells["colPayableSingleID"].Value.ToString();
                    int strOrderType = Convert.ToInt32(dgvr.Cells[colOrderType.Name].Tag);
                    //应收应付验证单据本次结算金额和明细金额是否一样
                    if ((orderType == DataSources.EnumOrderType.RECEIVABLE && (DataSources.EnumReceivableType)strOrderType == DataSources.EnumReceivableType.RECEIVABLE) ||
                        (orderType == DataSources.EnumOrderType.PAYMENT && (DataSources.EnumPaymentType)strOrderType == DataSources.EnumPaymentType.PAYMENT))
                    {
                        if (!DBHelper.IsExist("验证单据和明细金额是否一样", "v_bill_money", string.Format("order_id='{0}'", order_id)))
                        {
                            dgvBillReceivable.CurrentCell = dgvr.Cells[colOrderNum.Name];
                            message = "收/付款金额与结算单据实际金额不一致！";
                            break;
                        }
                    }
                    if (Financial.CheckDocumentMoney(orderType, order_id))
                    {
                        dgvBillReceivable.CurrentCell = dgvr.Cells[colOrderNum.Name];
                        message = "单据本次结算金额大于待结算金额！";
                        break;
                    }
                    //应收应付,重新计算已结算/待结算金额(应对同一张结算单,保存多次,再默认页面提交时)
                    if (strOrderType == 1)
                    {
                        Financial.DocumentMoney(orderType, order_id, listSql);
                    }
                    SysSQLString sql = new SysSQLString();
                    sql.cmdType = CommandType.Text;
                    sql.Param = new Dictionary<string, string>();
                    sql.Param.Add("submit", submit);
                    sql.Param.Add("draft", draft);
                    sql.Param.Add("notAudit", notAudit);
                    if (orderType == DataSources.EnumOrderType.RECEIVABLE)
                    {
                        sql.Param.Add("order_num", CommonUtility.GetNewNo(DataSources.EnumProjectType.RECEIVABLE));
                    }
                    else
                    {
                        sql.Param.Add("order_num", CommonUtility.GetNewNo(DataSources.EnumProjectType.PAYMENT));
                    }
                    sql.Param.Add("payable_single_id", order_id);
                    sql.sqlString = "update tb_bill_receivable set order_status=@submit,order_num=@order_num where payable_single_id=@payable_single_id and (order_status=@draft or order_status=@notAudit)";
                    listSql.Add(sql);
                    SetDocumentImportStatus("is_lock", DataSources.EnumImportStaus.LOCK, listSql, order_id, dgvr.Cells[colOrderType.Name].Tag.ToString());
                    //如果是应收付,则计算已结算金额
                    if (strOrderType == 1)
                    {
                        Financial.DocumentSettlementByBill(orderType, order_id, listSql);
                    }
                    else if (strOrderType == 0)//预收付,则计算预收付金额
                    {
                        Financial.DocumentAdvanceByBill(orderType, order_id, listSql);
                    }
                }
            }
            if (message.Length > 0)
            {
                MessageBoxEx.ShowError(message);
                return;
            }
            if (listSql.Count == 0)
            {
                MessageBoxEx.Show("请选择要提交的数据！");
                return;
            }
            if (!MessageBoxEx.ShowQuestion("是否要提交选择的数据！"))
            {
                return;
            }
            if (DBHelper.BatchExeSQLStringMultiByTrans("提交应收应付", listSql))
            {
                MessageBoxEx.Show("提交成功！");
                BindData();
            }
            else
            {
                MessageBoxEx.ShowWarning("提交失败！");
            }
        }

        /// <summary>
        /// 设置单据导入状态
        /// </summary>
        void SetDocumentImportStatus(string statusName, DataSources.EnumImportStaus importStaus, List<SysSQLString> listSql, string order_id, string order_type)
        {
            DataTable dt = DBHelper.GetTable("", "tb_balance_documents", "documents_id,documents_name", string.Format("order_id='{0}'", order_id), "", "");
            if (dt == null || dt.Rows.Count == 0)
            {
                return;
            }
            Preposition pre = new Preposition();
            foreach (DataRow dr in dt.Rows)
            {
                pre.AddID(dr["documents_id"], dr["documents_name"]);
            }
            listSql.AddRange(pre.GetSql(statusName, importStaus));
        }
        //审核
        void VerifyData()
        {
            string files = string.Empty;
            foreach (DataGridViewRow dgvr in dgvBillReceivable.Rows)
            {
                if (Convert.ToBoolean(dgvr.Cells[colCheck.Name].EditedFormattedValue))
                {
                    files += string.Format("'{0}',", dgvr.Cells[colPayableSingleID.Name].Value);
                }
            }
            if (files.Length == 0)
            {
                MessageBoxEx.Show("请选择要审核的数据！");
                return;
            }
            UCVerify frmVerify = new UCVerify();
            if (frmVerify.ShowDialog() == DialogResult.OK)
            {
                files = files.TrimEnd(',');
                SysSQLString sql = new SysSQLString();
                sql.cmdType = CommandType.Text;
                sql.sqlString = string.Format("update tb_bill_receivable set order_status='{0}',Verify_advice='{1}' where payable_single_id in ({2}) and order_status='{3}';",
                    (int)frmVerify.auditStatus, frmVerify.Content, files, (int)DataSources.EnumAuditStatus.SUBMIT);
                sql.Param = new Dictionary<string, string>();
                List<SysSQLString> listSql = new List<SysSQLString>();
                listSql.Add(sql);
                //如果是审核不通过，则将没有提交或审核状态的单据设为正常
                if (frmVerify.auditStatus == DataSources.EnumAuditStatus.NOTAUDIT)
                {
                    #region 将没有提交或审核状态的单据设为正常
                    if (orderType == DataSources.EnumOrderType.RECEIVABLE)
                    {
                        SysSQLString receivableSql = new SysSQLString();
                        receivableSql.cmdType = CommandType.Text;
                        receivableSql.sqlString = string.Format(@"update tb_parts_sale_billing set is_lock=@is_lock where sale_billing_id in (select documents_id from tb_balance_documents where order_id in ({0})) and  exists (
select a.documents_id from tb_balance_documents a inner join tb_bill_receivable b on a.order_id=b.payable_single_id
where order_status in ('1','2') and a.documents_id =sale_billing_id
union all
select b.order_id from tb_account_verification a inner join tb_verificationn_documents b on a.account_verification_id=b.account_verification_id
where a.order_status in ('1','2') and b.order_id=sale_billing_id
);
update tb_parts_sale_order set is_lock=@is_lock where sale_order_id in (select documents_id from tb_balance_documents where order_id in ({0})) and  exists (
select a.documents_id from tb_balance_documents a inner join tb_bill_receivable b on a.order_id=b.payable_single_id
where order_status in ('1','2') and a.documents_id =sale_order_id
union all
select b.order_id from tb_account_verification a inner join tb_verificationn_documents b on a.account_verification_id=b.account_verification_id
where a.order_status in ('1','2') and b.order_id=sale_order_id
);
update tb_maintain_settlement_info set is_lock=@is_lock where settlement_id in (select documents_id from tb_balance_documents where order_id in ({0})) and  exists (
select a.documents_id from tb_balance_documents a inner join tb_bill_receivable b on a.order_id=b.payable_single_id
where order_status in ('1','2') and a.documents_id =settlement_id
union all
select b.order_id from tb_account_verification a inner join tb_verificationn_documents b on a.account_verification_id=b.account_verification_id
where a.order_status in ('1','2') and b.order_id=settlement_id
);
update tb_maintain_three_guaranty_settlement set is_lock=@is_lock where st_id in (select documents_id from tb_balance_documents where order_id in ({0})) and  exists (
select a.documents_id from tb_balance_documents a inner join tb_bill_receivable b on a.order_id=b.payable_single_id
where order_status in ('1','2') and a.documents_id =st_id
union all
select b.order_id from tb_account_verification a inner join tb_verificationn_documents b on a.account_verification_id=b.account_verification_id
where a.order_status in ('1','2') and b.order_id=st_id
);", files);
                        receivableSql.Param = new Dictionary<string, string>();
                        receivableSql.Param.Add("is_lock", ((int)DataSources.EnumImportStaus.OPEN).ToString());
                        //receivableSql.Param.Add("@id", files);
                        listSql.Add(receivableSql);
                    }
                    else
                    {
                        SysSQLString purchaseBillingSql = new SysSQLString();
                        purchaseBillingSql.cmdType = CommandType.Text;
                        purchaseBillingSql.sqlString = string.Format(@"update tb_parts_purchase_billing set is_lock=@is_lock where purchase_billing_id in (select documents_id from tb_balance_documents where order_id in ({0})) and  exists (
select a.documents_id from tb_balance_documents a inner join tb_bill_receivable b on a.order_id=b.payable_single_id
where order_status in ('1','2') and a.documents_id =purchase_billing_id
union all
select b.order_id from tb_account_verification a inner join tb_verificationn_documents b on a.account_verification_id=b.account_verification_id
where a.order_status in ('1','2') and b.order_id=purchase_billing_id
);
                                                  update tb_parts_purchase_order set is_lock=@is_lock where order_id in (select documents_id from tb_balance_documents where order_id in ({0})) and  exists (
select a.documents_id from tb_balance_documents a inner join tb_bill_receivable b on a.order_id=b.payable_single_id
where order_status in ('1','2') and a.documents_id =order_id
union all
select b.order_id from tb_account_verification a inner join tb_verificationn_documents b on a.account_verification_id=b.account_verification_id
where a.order_status in ('1','2') and b.order_id=order_id
);  ", files);
                        purchaseBillingSql.Param = new Dictionary<string, string>();
                        purchaseBillingSql.Param.Add("is_lock", ((int)DataSources.EnumImportStaus.OPEN).ToString());
                        //purchaseBillingSql.Param.Add("@id", files);
                        listSql.Add(purchaseBillingSql);
                    }
                    #endregion
                    //审核失败需重新计算已结算/预收付金额
                    foreach (DataGridViewRow dgvr in dgvBillReceivable.Rows)
                    {
                        if (Convert.ToBoolean(dgvr.Cells[colCheck.Name].EditedFormattedValue))
                        {
                            int strOrderType = Convert.ToInt32(dgvr.Cells[colOrderType.Name].Tag);
                            string order_id = dgvr.Cells[colPayableSingleID.Name].Value.ToString();
                            //如果是应收付,则计算已结算金额
                            if (strOrderType == 1)
                            {
                                Financial.DocumentSettlementByBill(orderType, order_id, listSql);
                            }
                            //预收付,则计算预收付金额
                            else if (strOrderType == 0)
                            {
                                Financial.DocumentAdvanceByBill(orderType, order_id, listSql);
                            }
                        }
                    }
                }
                if (DBHelper.BatchExeSQLStringMultiByTrans("审核应收应付", listSql))
                {
                    MessageBoxEx.Show("审核成功！");
                    BindData();
                }
                else
                {
                    MessageBoxEx.Show("审核失败！");
                }
            }
        }
        //删除
        void DeleteData()
        {
            dgvBillReceivable.EndEdit();
            List<string> listField = new List<string>();
            foreach (DataGridViewRow dgvr in dgvBillReceivable.Rows)
            {
                object isCheck = dgvr.Cells["colCheck"].Value;
                if (isCheck != null && (bool)isCheck)
                {
                    listField.Add(dgvr.Cells["colPayableSingleID"].Value.ToString());
                }
            }
            if (listField.Count == 0)
            {
                MessageBoxEx.Show("请选择要删除的数据!");
                return;
            }
            if (MessageBoxEx.Show("是否要删除当前数据？", "提示", MessageBoxButtons.OKCancel) != DialogResult.OK)
            {
                return;
            }
            Dictionary<string, string> dic = new Dictionary<string, string>();
            dic.Add("enable_flag", ((int)DataSources.EnumEnableFlag.DELETED).ToString());
            if (DBHelper.BatchUpdateDataByIn("批量删除应收应付", "tb_bill_receivable", dic, "payable_single_id", listField.ToArray()))
            {
                MessageBoxEx.Show("删除成功！");
                BindData();
            }
            else
            {
                MessageBoxEx.Show("删除失败！");
            }
        }

        //新增
        void AddData()
        {
            UCReceivableAdd add = new UCReceivableAdd(WindowStatus.Add, null, this, this.orderType);
            this.addUserControl(add, string.Format("{0}-新增", Title), "UCReceivableAdd" + orderType.ToString(), this.Tag.ToString(), this.Name);
        }

        // 编辑数据
        private void EditData(WindowStatus status)
        {
            if (status != WindowStatus.Edit && status != WindowStatus.Copy)
            {
                return;
            }
            string title = "编辑";
            string menuId = "UCReceivableEdit";
            if (status == WindowStatus.Copy)
            {
                title = "复制";
                menuId = "UCReceivableCopy";
            }
            if (dgvBillReceivable.CurrentRow == null)
            {
                MessageBoxEx.Show(string.Format("请选择要{0}的数据!", title));
                return;
            }
            string id = ID;
            if (string.IsNullOrEmpty(id))
            {
                return;
            }

            UCReceivableAdd add = new UCReceivableAdd(status, id, this, this.orderType);
            base.addUserControl(add, string.Format("{0}-{1}", Title, title), menuId + orderType.ToString() + id, this.Tag.ToString(), this.Name);
        }

        // 预览数据
        private void ViewData()
        {
            if (dgvBillReceivable.CurrentRow == null)
            {
                MessageBoxEx.Show("请选择要预览的数据!");
                return;
            }
            string id = ID;
            if (string.IsNullOrEmpty(id))
            {
                return;
            }
            UCReceivableAdd view = new UCReceivableAdd(WindowStatus.View, id, this, orderType);
            base.addUserControl(view, Title + "-预览", "UCReceivableView" + orderType.ToString() + id, this.Tag.ToString(), this.Name);
        }
        #endregion
        /// <summary>
        /// 选择，设置工具栏状态
        /// </summary>
        void SetSelectedStatus()
        {
            listIDs.Clear();
            //已选择状态列表
            List<string> listFiles = new List<string>();
            foreach (DataGridViewRow dgvr in dgvBillReceivable.Rows)
            {
                if (Convert.ToBoolean(dgvr.Cells[colCheck.Name].EditedFormattedValue))
                {
                    listFiles.Add(dgvr.Cells[colOrderStatus.Name].Tag.ToString());
                    listIDs.Add(dgvr.Cells[colPayableSingleID.Name].Value.ToString());
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
                tsmiCopyF.Enabled = true;
            }
            else
            {
                btnCopy.Enabled = false;
                tsmiCopyF.Enabled = false;
            }
            //编辑按钮，只选择一个并且是草稿或未通过状态，可以编辑
            if (listFiles.Count == 1 && (listFiles.Contains(draftStr) || listFiles.Contains(noAuditStr)))
            {
                btnEdit.Enabled = true;
                tsmiEditF.Enabled = true;
            }
            else
            {
                btnEdit.Enabled = false;
                tsmiEditF.Enabled = false;
            }
            //判断”审核“按钮是否可用
            if (listFiles.Contains(auditStr) || listFiles.Contains(draftStr) || listFiles.Contains(noAuditStr) || listFiles.Contains(invalid))
            {
                btnVerify.Enabled = false;
                tsmiVerifyF.Enabled = false;
            }
            else
            {
                btnVerify.Enabled = true;
                tsmiVerifyF.Enabled = true;
            }
            //包含已审核、已提交、已作废状态，提交、删除按钮不可用
            if (listFiles.Contains(auditStr) || listFiles.Contains(submitStr) || listFiles.Contains(invalid))
            {
                btnSubmit.Enabled = false;
                btnDelete.Enabled = false;
                tsmiSubmitF.Enabled = false;
                tsmiDeleteF.Enabled = false;
            }
            else
            {
                btnSubmit.Enabled = true;
                btnDelete.Enabled = true;
                tsmiSubmitF.Enabled = true;
                tsmiDeleteF.Enabled = true;
            }

            if (listFiles.Contains(invalid))
            {

            }

        }
        #endregion

        #region 查询面板事件
        //清空
        private void btnClear_Click(object sender, EventArgs e)
        {
            ClearSearch();
        }
        //查询
        private void btnSearch_Click(object sender, EventArgs e)
        {
            BindData();
        }
        //选择公司
        private void cboCompany_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cboCompany.SelectedValue == null)
            {
                return;
            }
            CommonFuncCall.BindDepartment(cboOrgId, cboCompany.SelectedValue.ToString(), "全部");
        }
        //选择部门
        private void cboOrgId_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cboOrgId.SelectedValue == null)
            {
                return;
            }
            CommonFuncCall.BindHandle(cboHandle, cboOrgId.SelectedValue.ToString(), "全部");
        }

        //分页，页码改变，重新绑定
        private void page_PageIndexChanged(object sender, EventArgs e)
        {
            BindData();
        }
        #endregion

        #region 查询面板方法
        /// <summary>
        /// 清空查询面板
        /// </summary>
        void ClearSearch()
        {
            ClearSearch(pnlSearch);
            dtInterval.StartDate = new DateTime(DateTime.Now.Year, DateTime.Now.Month, 1).ToString(dtInterval.customFormat);
            dtInterval.EndDate = DateTime.Now.ToString(dtInterval.customFormat);
        }

        /// <summary>
        /// 获取查询条件
        /// </summary>
        /// <returns></returns>
        string GetWhere()
        {
            StringBuilder sbWhere = new StringBuilder();
            sbWhere.AppendFormat("enable_flag='{0}'", (int)DataSources.EnumEnableFlag.USING);//查询标记未删除的数据
            sbWhere.AppendFormat(" and order_type='{0}'", (int)orderType);//单据类型
            string payment_type = CommonCtrl.IsNullToString(cboOrderType.SelectedValue);//收付款类型
            if (payment_type.Length > 0)
            {
                sbWhere.AppendFormat(" and payment_type='{0}'", payment_type);
            }
            string orderNum = txtOrderNum.Caption.Trim();//单号
            if (orderNum.Length > 0)
            {
                sbWhere.AppendFormat(" and order_num like '%{0}%'", orderNum);
            }
            string custCode = txtCustCode.Text.Trim();//往来单位编码
            if (custCode.Length > 0)
            {
                sbWhere.AppendFormat(" and cust_code like '%{0}%'", custCode);
            }
            string custName = txtCustName.Caption.Trim();//往来单位名称
            if (custName.Length > 0)
            {
                sbWhere.AppendFormat(" and cust_name like '%{0}%'", custName);
            }
            string comId = CommonCtrl.IsNullToString(cboCompany.SelectedValue);//公司
            if (comId.Length > 0)
            {
                sbWhere.AppendFormat(" and com_id='{0}'", comId);
            }
            string orgId = CommonCtrl.IsNullToString(cboOrgId.SelectedValue);//部门
            if (orgId.Length > 0)
            {
                sbWhere.AppendFormat(" and org_id='{0}'", orgId);
            }
            string handle = CommonCtrl.IsNullToString(cboHandle.SelectedValue);//经办人
            if (handle.Length > 0)
            {
                sbWhere.AppendFormat(" and handle='{0}'", handle);
            }
            if (!isSearch)
            {
                string orderStatus = CommonCtrl.IsNullToString(cboOrderStatus.SelectedValue);//单据状态
                if (orderStatus.Length > 0)
                {
                    sbWhere.AppendFormat(" and order_status='{0}'", orderStatus);
                }
            }
            else
            {
                sbWhere.Append(" and order_status ='2'");
            }
            if (!string.IsNullOrEmpty(dtInterval.StartDate) && !string.IsNullOrEmpty(dtInterval.EndDate))
            {
                sbWhere.AppendFormat(" and create_time between {0} and {1}", Common.LocalDateTimeToUtcLong(DateTime.Parse(dtInterval.StartDate).Date),
                   Common.LocalDateTimeToUtcLong(DateTime.Parse(dtInterval.EndDate).Date.AddDays(1).AddMilliseconds(-1)));
            }
            return sbWhere.ToString();
        }
        /// <summary>
        /// 绑定数据
        /// </summary>
        void BindData()
        {
            string strWhere = GetWhere();
            int recordCount = 0;
            isPreview = false;
            dtData = DBHelper.GetTableByPage("查询应收应付", "tb_bill_receivable", "*", strWhere, "", "create_time desc", page.PageIndex, page.PageSize, out recordCount);
            dgvBillReceivable.RowCount = 0;
            foreach (DataRow dr in dtData.Rows)
            {
                DataGridViewRow dgvr = dgvBillReceivable.Rows[dgvBillReceivable.Rows.Add()];
                dgvr.Cells[colPayableSingleID.Name].Value = dr["payable_single_id"];
                if (orderType == DataSources.EnumOrderType.RECEIVABLE)
                {
                    dgvr.Cells[colOrderType.Name].Value = DataSources.GetDescription(typeof(DataSources.EnumReceivableType), dr["payment_type"]);//单据类型
                }
                else
                {
                    dgvr.Cells[colOrderType.Name].Value = DataSources.GetDescription(typeof(DataSources.EnumPaymentType), dr["payment_type"]);//单据类型
                }
                dgvr.Cells[colOrderType.Name].Tag = dr["payment_type"];
                dgvr.Cells[colOrderNum.Name].Value = CommonCtrl.IsNullToString(dr["order_num"]);//单号
                dgvr.Cells[colOrderDate.Name].Value = Common.UtcLongToLocalDateTime(dr["order_date"], "yyyy-MM-dd");
                dgvr.Cells[colClientCode.Name].Value = CommonCtrl.IsNullToString(dr["cust_code"]);//客户编号
                dgvr.Cells[colClientName.Name].Value = CommonCtrl.IsNullToString(dr["cust_name"]);//客户名称
                dgvr.Cells["colDealingsBalance"].Value = Common.CurrencyFormat((dr["cash_money"]));//收款金额
                dgvr.Cells[colOrgId.Name].Value = CommonCtrl.IsNullToString(dr["org_name"]);//部门
                dgvr.Cells[colHandle.Name].Value = CommonCtrl.IsNullToString(dr["handle_name"]);//经办人
                dgvr.Cells[colOperator.Name].Value = CommonCtrl.IsNullToString(dr["operator_name"]);//操作人
                dgvr.Cells[colRemark.Name].Value = CommonCtrl.IsNullToString(dr["remark"]);//备注
                string status = CommonCtrl.IsNullToString(dr["order_status"]);//
                if (status.Length > 0)
                {
                    dgvr.Cells[colOrderStatus.Name].Value = DataSources.GetDescription(typeof(DataSources.EnumAuditStatus), int.Parse(status));//单据状态
                }
                dgvr.Cells[colOrderStatus.Name].Tag = dr["order_status"];
            }
            if (dgvBillReceivable.RowCount > 0)
            {
                dgvBillReceivable.CurrentCell = dgvBillReceivable.Rows[0].Cells[colOrderNum.Name];
            }
            page.RecordCount = recordCount;
            page.SetBtnState();
        }
        /// <summary>
        /// 绑定数据后，定位到指定单击
        /// </summary>
        /// <param name="orderID">指定单据ID</param>
        public void BindData(string orderID)
        {
            BindData();
            if (string.IsNullOrEmpty(orderID))
            {
                return;
            }
            foreach (DataGridViewRow dgvr in dgvBillReceivable.Rows)
            {
                if (CommonCtrl.IsNullToString(dgvr.Cells[colPayableSingleID.Name].Value) == orderID)
                {
                    dgvBillReceivable.CurrentCell = dgvr.Cells[colOrderType.Name];
                    break;
                }
            }
        }

        //绑定查询控件
        void BindSearch()
        {
            CommonFuncCall.BindCompany(cboCompany, "全部");
            BindOrderType();
            if (!isSearch)
            {
                DataSources.BindComBoxDataEnum(cboOrderStatus, typeof(DataSources.EnumAuditStatus), true);
            }
            else
            {
                lblStatus.Visible = false;
                cboOrderStatus.Visible = false;
            }
        }

        /// <summary>
        /// 绑定单据类型
        /// </summary>
        void BindOrderType()
        {
            if (orderType == DataSources.EnumOrderType.RECEIVABLE)
            {
                DataSources.BindComBoxDataEnum(cboOrderType, typeof(DataSources.EnumReceivableType), true);
            }
            else
            {
                DataSources.BindComBoxDataEnum(cboOrderType, typeof(DataSources.EnumPaymentType), true);
            }
        }

        /// <summary>
        /// 设置显示
        /// </summary>
        void SetLable()
        {
            if (orderType == DataSources.EnumOrderType.PAYMENT)
            {
                lblCustCode.Text = "供应商编码：";
                lblCustName.Text = "供应商名称：";
                colClientCode.HeaderText = "供应商编码";
                colClientName.HeaderText = "供应商名称";
                colDealingsBalance.HeaderText = "付款金额";
            }
        }
        #endregion

        #region DataGridView事件
        //双击行，查看行
        private void dgvBillReceivable_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            ViewData();
        }
        //点击选择复选框
        private void dgvBillReceivable_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {
            if (dgvBillReceivable.CurrentCell == null)
            {
                return;
            }
            //点击选择框
            if (dgvBillReceivable.CurrentCell.OwningColumn.Name == colCheck.Name)
            {
                SetSelectedStatus();
            }
        }

        //选择器
        private void txtCustCode_ChooserClick(object sender, EventArgs e)
        {
            //应收选择客户
            if (orderType == DataSources.EnumOrderType.RECEIVABLE)
            {
                frmCustomerInfo frm = new frmCustomerInfo();
                if (frm.ShowDialog() == DialogResult.OK)
                {
                    txtCustCode.Text = frm.strCustomerNo;
                    txtCustName.Caption = frm.strCustomerName;
                }
            }
            else if (orderType == DataSources.EnumOrderType.PAYMENT)//应付选择供应商
            {
                frmSupplier frmSupp = new frmSupplier();
                if (frmSupp.ShowDialog() == DialogResult.OK)
                {
                    txtCustCode.Text = frmSupp.supperCode;
                    txtCustName.Caption = frmSupp.supperName;
                }
            }
        }
        //单击选择单行
        private void dgvBillReceivable_CellMouseClick(object sender, DataGridViewCellMouseEventArgs e)
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
            foreach (DataGridViewRow dgvr in dgvBillReceivable.Rows)
            {
                object check = dgvr.Cells[colCheck.Name].EditedFormattedValue;
                if (check != null && (bool)check)
                {
                    dgvr.Cells[colCheck.Name].Value = false;
                }
            }
            //选择当前行
            dgvBillReceivable.Rows[e.RowIndex].Cells[colCheck.Name].Value = true;
            SetSelectedStatus();
        }
        //全选
        void dgvBillReceivable_HeadCheckChanged()
        {
            SetSelectedStatus();
        }
        #endregion

    }
}
