﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using ServiceStationClient.ComponentUI;
using Model;

namespace HXCPcClient.UCForm.AccessoriesBusiness.PurchaseManagement.YuTongPurchaseOrder
{
    public partial class UserControlShengJi : UserControl
    {
        #region 变量、类声明
        
        #endregion

        #region 初始化窗体
        /// <summary> 初始化窗体
        /// </summary>
        public UserControlShengJi()
        {
            InitializeComponent();
        }
        /// <summary> 窗体加载
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UserControlShengJi_Load(object sender, EventArgs e)
        {
            //绑定紧急程度
            //CommonFuncCall.BindYTEmergencyLevel(ddlemergency_level, true, "请选择");
            CommonFuncCall.BindComBoxDataSource(ddlemergency_level, "emergency_level_yt", "请选择");
            //产品改进通知号
            CommonFuncCall.BindYTProduct_No(ddlproduct_notice_num, "请选择");
            //活动类型
            CommonFuncCall.BindYTActivityType(ddlactivity_type, true, "请选择");
            //要求发货方式
            //CommonFuncCall.BindYTReqdelivery(ddlreq_delivery, true, "请选择");
            CommonFuncCall.BindComBoxDataSource(ddlreq_delivery, "delivery_requirements_yt", "请选择");
            //加载收货人信息
            CommonFuncCall.BindUser(ddlconsignee_code, true, "请选择");
        } 
        #endregion

        #region 方法、函数
        /// <summary> 验证必填项是否填写
        /// </summary>
        /// <returns></returns>
        public bool CheckControlInfo()
        {
            bool ischeck = true;
            if (string.IsNullOrEmpty(ddlemergency_level.SelectedValue.ToString()))
            {
                MessageBoxEx.Show("请选择紧急程度!");
                return false;
            }
            if (string.IsNullOrEmpty(ddlactivity_type.SelectedValue.ToString()))
            {
                MessageBoxEx.Show("请选择活动类型!");
                return false;
            }
            if (string.IsNullOrEmpty(ddlproduct_notice_num.SelectedValue.ToString()))
            {
                MessageBoxEx.Show("请选择产品改进通知号!");
                return false;
            }
            if (string.IsNullOrEmpty(ddlreq_delivery.SelectedValue.ToString()))
            {
                MessageBoxEx.Show("请选择发货方式!");
                return false;
            }
            if (string.IsNullOrEmpty(ddtreq_delivery_time.Value.ToString()))
            {
                MessageBoxEx.Show("请选择发货时间!");
                return false;
            }
            if (string.IsNullOrEmpty(ddlconsignee_code.SelectedValue.ToString()))
            {
                MessageBoxEx.Show("请选择收货人!");
                return false;
            }
            if (string.IsNullOrEmpty(txtconsignee_tel.Caption.Trim()) && string.IsNullOrEmpty(txtconsignee_phone.Caption.Trim()))
            {
                MessageBoxEx.Show("请至少填写一项收货人联系方式!");
                return false;
            }
            if (string.IsNullOrEmpty(txtdelivery_address.Caption.Trim()))
            {
                MessageBoxEx.Show("请填写收货地址!");
                return false;
            }
            if (string.IsNullOrEmpty(txtremark.Caption.Trim()))
            {
                MessageBoxEx.Show("请填写备注!");
                return false;
            }
            if (!txtconsignee_tel.Verify(true))
            { return false; }
            if (!txtconsignee_phone.Verify(true))
            { return false; }
            return ischeck;
        }

        public void LoadControlInfo(tb_parts_purchase_order_2 model)
        {
            CommonFuncCall.SetShowControlValue(this, model, "");
        }

        public void GetControlInfo(tb_parts_purchase_order_2 model)
        {
            if (!string.IsNullOrEmpty(ddtreq_delivery_time.Value))
            {
                ddtreq_delivery_time.Value = Convert.ToDateTime(ddtreq_delivery_time.Value).ToShortDateString() + " 23:59:59";
            }
            CommonFuncCall.SetModelObjectValue(this, model);
            if (!string.IsNullOrEmpty(ddlemergency_level.SelectedValue.ToString()))
            {
                model.emergency_level_name = ddlemergency_level.SelectedItem.ToString();
            }
            if (!string.IsNullOrEmpty(ddlactivity_type.SelectedValue.ToString()))
            {
                model.activity_type_name = ddlactivity_type.SelectedItem.ToString();
            }
            if (!string.IsNullOrEmpty(ddlproduct_notice_num.SelectedValue.ToString()))
            {
                model.product_notice_num_name = ddlproduct_notice_num.SelectedItem.ToString();
            }
            if (!string.IsNullOrEmpty(ddlreq_delivery.SelectedValue.ToString()))
            {
                model.req_delivery_name = ddlreq_delivery.SelectedItem.ToString();
            }
            if (!string.IsNullOrEmpty(ddlconsignee_code.SelectedValue.ToString()))
            {
                model.consignee = ddlconsignee_code.Text.ToString();
            }
        }
        #endregion
    }
}
