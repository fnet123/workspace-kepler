﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using HXCPcClient.Chooser;

namespace HXCPcClient.UCForm.BusinessAnalysis.StockReport
{
    public partial class UCWarehouseSummariz : UCReport
    {
        public UCWarehouseSummariz()
            : base("v_warehouse_summariz", "仓库开单汇总表")
        {
            InitializeComponent();
        }

        private void UCWarehouseSummariz_Load(object sender, EventArgs e)
        {
            ReportCommon.BindWarehouseType(cboOrderType);
            CommonFuncCall.BindCompany(cboCompany, "全部");//绑定公司
            CommonFuncCall.BindWarehouse(cboWarehouse, "全部");//绑定仓库

            BindData();
        }
        //选择公司,加载部门
        private void cboCompany_SelectedIndexChanged(object sender, EventArgs e)
        {
            string comID = CommonCtrl.IsNullToString(cboCompany.SelectedValue);
            CommonFuncCall.BindDepartment(cboOrg, comID, "全部");
        }
        //配件选择器
        private void txtcPartsCode_ChooserClick(object sender, EventArgs e)
        {
            frmParts frmPartsChooser = new frmParts();
            if (frmPartsChooser.ShowDialog() == DialogResult.OK)
            {
                txtcPartsCode.Text = frmPartsChooser.PartsCode;
                txtPartsName.Caption = frmPartsChooser.PartsName;
            }
        }
        //配件类别选择器
        private void txtcPartsType_ChooserClick(object sender, EventArgs e)
        {
            frmPartsType frmType = new frmPartsType();
            if (frmType.ShowDialog() == DialogResult.OK)
            {
                txtcPartsType.Text = frmType.TypeName;
            }
        }
        //配件车型选择器
        private void txtcPartsVehicle_ChooserClick(object sender, EventArgs e)
        {
            frmVehicleModels frmVM = new frmVehicleModels();
            if (frmVM.ShowDialog() == DialogResult.OK)
            {
                txtcPartsVehicle.Text = frmVM.VMName;
            }
        }
        //查询
        private void btnSearch_Click(object sender, EventArgs e)
        {
            BindData();
        }

        /// <summary>
        /// 绑定报表数据
        /// </summary>
        void BindData()
        {

        }
    }
}
