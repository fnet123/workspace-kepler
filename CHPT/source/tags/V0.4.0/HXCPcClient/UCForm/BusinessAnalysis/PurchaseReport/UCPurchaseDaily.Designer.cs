﻿namespace HXCPcClient.UCForm.BusinessAnalysis.PurchaseReport
{
    partial class UCPurchaseDaily
    {
        /// <summary> 
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region 组件设计器生成的代码

        /// <summary> 
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UCPurchaseDaily));
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle2 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle3 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle4 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle5 = new System.Windows.Forms.DataGridViewCellStyle();
            this.txtPartsName = new ServiceStationClient.ComponentUI.TextBoxEx();
            this.txtdrawing_num = new ServiceStationClient.ComponentUI.TextBoxEx();
            this.cboorg_id = new ServiceStationClient.ComponentUI.ComboBoxEx(this.components);
            this.cboCompany = new ServiceStationClient.ComponentUI.ComboBoxEx(this.components);
            this.txtparts_brand = new ServiceStationClient.ComponentUI.TextBoxEx();
            this.txtcparts_code = new ServiceStationClient.ComponentUI.TextBox.TextChooser();
            this.label13 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.label14 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.dicreate_time = new ServiceStationClient.ComponentUI.DateInterval();
            this.label9 = new System.Windows.Forms.Label();
            this.dgvReport = new ServiceStationClient.ComponentUI.DataGridViewReport();
            this.Column1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column6 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column7 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column8 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column9 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column10 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.pnlSearch.SuspendLayout();
            this.pnlReport.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dt)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dgvReport)).BeginInit();
            this.SuspendLayout();
            // 
            // pnlSearch
            // 
            this.pnlSearch.Controls.Add(this.txtPartsName);
            this.pnlSearch.Controls.Add(this.txtdrawing_num);
            this.pnlSearch.Controls.Add(this.cboorg_id);
            this.pnlSearch.Controls.Add(this.cboCompany);
            this.pnlSearch.Controls.Add(this.txtparts_brand);
            this.pnlSearch.Controls.Add(this.txtcparts_code);
            this.pnlSearch.Controls.Add(this.label13);
            this.pnlSearch.Controls.Add(this.label10);
            this.pnlSearch.Controls.Add(this.label14);
            this.pnlSearch.Controls.Add(this.label11);
            this.pnlSearch.Controls.Add(this.label12);
            this.pnlSearch.Controls.Add(this.dicreate_time);
            this.pnlSearch.Controls.Add(this.label9);
            this.pnlSearch.Location = new System.Drawing.Point(5, 37);
            this.pnlSearch.Size = new System.Drawing.Size(1067, 93);
            this.pnlSearch.Controls.SetChildIndex(this.btnClear, 0);
            this.pnlSearch.Controls.SetChildIndex(this.btnSearch, 0);
            this.pnlSearch.Controls.SetChildIndex(this.label9, 0);
            this.pnlSearch.Controls.SetChildIndex(this.dicreate_time, 0);
            this.pnlSearch.Controls.SetChildIndex(this.label12, 0);
            this.pnlSearch.Controls.SetChildIndex(this.label11, 0);
            this.pnlSearch.Controls.SetChildIndex(this.label14, 0);
            this.pnlSearch.Controls.SetChildIndex(this.label10, 0);
            this.pnlSearch.Controls.SetChildIndex(this.label13, 0);
            this.pnlSearch.Controls.SetChildIndex(this.txtcparts_code, 0);
            this.pnlSearch.Controls.SetChildIndex(this.txtparts_brand, 0);
            this.pnlSearch.Controls.SetChildIndex(this.cboCompany, 0);
            this.pnlSearch.Controls.SetChildIndex(this.cboorg_id, 0);
            this.pnlSearch.Controls.SetChildIndex(this.txtdrawing_num, 0);
            this.pnlSearch.Controls.SetChildIndex(this.txtPartsName, 0);
            // 
            // btnSearch
            // 
            this.btnSearch.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("btnSearch.BackgroundImage")));
            this.btnSearch.Caption = "";
            this.btnSearch.DownImage = ((System.Drawing.Image)(resources.GetObject("btnSearch.DownImage")));
            this.btnSearch.Location = new System.Drawing.Point(971, 20);
            this.btnSearch.MoveImage = ((System.Drawing.Image)(resources.GetObject("btnSearch.MoveImage")));
            this.btnSearch.NormalImage = ((System.Drawing.Image)(resources.GetObject("btnSearch.NormalImage")));
            this.btnSearch.Size = new System.Drawing.Size(80, 24);
            this.btnSearch.Click += new System.EventHandler(this.btnSearch_Click);
            // 
            // btnClear
            // 
            this.btnClear.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("btnClear.BackgroundImage")));
            this.btnClear.Caption = "";
            this.btnClear.DownImage = ((System.Drawing.Image)(resources.GetObject("btnClear.DownImage")));
            this.btnClear.Location = new System.Drawing.Point(971, 52);
            this.btnClear.MoveImage = ((System.Drawing.Image)(resources.GetObject("btnClear.MoveImage")));
            this.btnClear.NormalImage = ((System.Drawing.Image)(resources.GetObject("btnClear.NormalImage")));
            this.btnClear.Size = new System.Drawing.Size(80, 24);
            // 
            // pnlReport
            // 
            this.pnlReport.Controls.Add(this.dgvReport);
            this.pnlReport.Location = new System.Drawing.Point(5, 137);
            this.pnlReport.Size = new System.Drawing.Size(1067, 404);
            // 
            // txtPartsName
            // 
            this.txtPartsName.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.None;
            this.txtPartsName.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.None;
            this.txtPartsName.BackColor = System.Drawing.Color.Transparent;
            this.txtPartsName.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.txtPartsName.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(166)))), ((int)(((byte)(208)))), ((int)(((byte)(226)))));
            this.txtPartsName.ForeImage = null;
            this.txtPartsName.InputtingVerifyCondition = null;
            this.txtPartsName.Location = new System.Drawing.Point(786, 15);
            this.txtPartsName.MaxLengh = 32767;
            this.txtPartsName.Multiline = false;
            this.txtPartsName.Name = "txtPartsName";
            this.txtPartsName.Radius = 3;
            this.txtPartsName.ReadOnly = false;
            this.txtPartsName.ShadowColor = System.Drawing.Color.FromArgb(((int)(((byte)(175)))), ((int)(((byte)(212)))), ((int)(((byte)(228)))));
            this.txtPartsName.ShowError = false;
            this.txtPartsName.Size = new System.Drawing.Size(121, 23);
            this.txtPartsName.TabIndex = 97;
            this.txtPartsName.Tag = "配件名称";
            this.txtPartsName.UseSystemPasswordChar = false;
            this.txtPartsName.Value = "";
            this.txtPartsName.VerifyCondition = null;
            this.txtPartsName.VerifyType = null;
            this.txtPartsName.VerifyTypeName = null;
            this.txtPartsName.WaterMark = null;
            this.txtPartsName.WaterMarkColor = System.Drawing.Color.Silver;
            // 
            // txtdrawing_num
            // 
            this.txtdrawing_num.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.None;
            this.txtdrawing_num.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.None;
            this.txtdrawing_num.BackColor = System.Drawing.Color.Transparent;
            this.txtdrawing_num.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.txtdrawing_num.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(166)))), ((int)(((byte)(208)))), ((int)(((byte)(226)))));
            this.txtdrawing_num.ForeImage = null;
            this.txtdrawing_num.InputtingVerifyCondition = null;
            this.txtdrawing_num.Location = new System.Drawing.Point(555, 58);
            this.txtdrawing_num.MaxLengh = 32767;
            this.txtdrawing_num.Multiline = false;
            this.txtdrawing_num.Name = "txtdrawing_num";
            this.txtdrawing_num.Radius = 3;
            this.txtdrawing_num.ReadOnly = false;
            this.txtdrawing_num.ShadowColor = System.Drawing.Color.FromArgb(((int)(((byte)(175)))), ((int)(((byte)(212)))), ((int)(((byte)(228)))));
            this.txtdrawing_num.ShowError = false;
            this.txtdrawing_num.Size = new System.Drawing.Size(126, 23);
            this.txtdrawing_num.TabIndex = 98;
            this.txtdrawing_num.Tag = "配件图号";
            this.txtdrawing_num.UseSystemPasswordChar = false;
            this.txtdrawing_num.Value = "";
            this.txtdrawing_num.VerifyCondition = null;
            this.txtdrawing_num.VerifyType = null;
            this.txtdrawing_num.VerifyTypeName = null;
            this.txtdrawing_num.WaterMark = null;
            this.txtdrawing_num.WaterMarkColor = System.Drawing.Color.Silver;
            // 
            // cboorg_id
            // 
            this.cboorg_id.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
            this.cboorg_id.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cboorg_id.FormattingEnabled = true;
            this.cboorg_id.Location = new System.Drawing.Point(322, 15);
            this.cboorg_id.Name = "cboorg_id";
            this.cboorg_id.Size = new System.Drawing.Size(121, 22);
            this.cboorg_id.TabIndex = 101;
            this.cboorg_id.Tag = "org_id";
            // 
            // cboCompany
            // 
            this.cboCompany.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
            this.cboCompany.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cboCompany.FormattingEnabled = true;
            this.cboCompany.Location = new System.Drawing.Point(107, 15);
            this.cboCompany.Name = "cboCompany";
            this.cboCompany.Size = new System.Drawing.Size(121, 22);
            this.cboCompany.TabIndex = 100;
            this.cboCompany.SelectedIndexChanged += new System.EventHandler(this.cboCompany_SelectedIndexChanged);
            // 
            // txtparts_brand
            // 
            this.txtparts_brand.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.None;
            this.txtparts_brand.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.None;
            this.txtparts_brand.BackColor = System.Drawing.Color.Transparent;
            this.txtparts_brand.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.txtparts_brand.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(166)))), ((int)(((byte)(208)))), ((int)(((byte)(226)))));
            this.txtparts_brand.ForeImage = null;
            this.txtparts_brand.InputtingVerifyCondition = null;
            this.txtparts_brand.Location = new System.Drawing.Point(786, 58);
            this.txtparts_brand.MaxLengh = 32767;
            this.txtparts_brand.Multiline = false;
            this.txtparts_brand.Name = "txtparts_brand";
            this.txtparts_brand.Radius = 3;
            this.txtparts_brand.ReadOnly = false;
            this.txtparts_brand.ShadowColor = System.Drawing.Color.FromArgb(((int)(((byte)(175)))), ((int)(((byte)(212)))), ((int)(((byte)(228)))));
            this.txtparts_brand.ShowError = false;
            this.txtparts_brand.Size = new System.Drawing.Size(121, 23);
            this.txtparts_brand.TabIndex = 99;
            this.txtparts_brand.Tag = "品牌";
            this.txtparts_brand.UseSystemPasswordChar = false;
            this.txtparts_brand.Value = "";
            this.txtparts_brand.VerifyCondition = null;
            this.txtparts_brand.VerifyType = null;
            this.txtparts_brand.VerifyTypeName = null;
            this.txtparts_brand.WaterMark = null;
            this.txtparts_brand.WaterMarkColor = System.Drawing.Color.Silver;
            // 
            // txtcparts_code
            // 
            this.txtcparts_code.ChooserTypeImage = ServiceStationClient.ComponentUI.TextBox.ChooserType.Default;
            this.txtcparts_code.Location = new System.Drawing.Point(555, 14);
            this.txtcparts_code.Name = "txtcparts_code";
            this.txtcparts_code.ReadOnly = false;
            this.txtcparts_code.Size = new System.Drawing.Size(121, 24);
            this.txtcparts_code.TabIndex = 96;
            this.txtcparts_code.Tag = "配件编码";
            this.txtcparts_code.ToolTipTitle = "";
            this.txtcparts_code.ChooserClick += new System.EventHandler(this.txtcparts_code_ChooserClick);
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(58, 20);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(41, 12);
            this.label13.TabIndex = 93;
            this.label13.Text = "公司：";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(715, 20);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(65, 12);
            this.label10.TabIndex = 90;
            this.label10.Text = "配件名称：";
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(275, 20);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(41, 12);
            this.label14.TabIndex = 94;
            this.label14.Text = "部门：";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(484, 63);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(65, 12);
            this.label11.TabIndex = 91;
            this.label11.Text = "配件图号：";
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(739, 63);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(41, 12);
            this.label12.TabIndex = 92;
            this.label12.Text = "品牌：";
            // 
            // dicreate_time
            // 
            this.dicreate_time.BackColor = System.Drawing.Color.Transparent;
            this.dicreate_time.EndDate = "2015-01-12";
            this.dicreate_time.Location = new System.Drawing.Point(50, 55);
            this.dicreate_time.Name = "dicreate_time";
            this.dicreate_time.ShowFormat = "yyyy-MM-dd";
            this.dicreate_time.Size = new System.Drawing.Size(411, 28);
            this.dicreate_time.StartDate = "2015-01-01";
            this.dicreate_time.TabIndex = 95;
            this.dicreate_time.Tag = "create_time";
            this.dicreate_time.Text = "日期从：";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(484, 20);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(65, 12);
            this.label9.TabIndex = 89;
            this.label9.Text = "配件编码：";
            // 
            // dgvReport
            // 
            this.dgvReport.AllowUserToAddRows = false;
            this.dgvReport.AllowUserToDeleteRows = false;
            this.dgvReport.AllowUserToResizeRows = false;
            dataGridViewCellStyle1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(241)))), ((int)(((byte)(253)))), ((int)(((byte)(252)))));
            this.dgvReport.AlternatingRowsDefaultCellStyle = dataGridViewCellStyle1;
            this.dgvReport.BackgroundColor = System.Drawing.Color.White;
            this.dgvReport.BorderStyle = System.Windows.Forms.BorderStyle.None;
            dataGridViewCellStyle2.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter;
            dataGridViewCellStyle2.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(240)))), ((int)(((byte)(241)))), ((int)(((byte)(243)))));
            dataGridViewCellStyle2.Font = new System.Drawing.Font("微软雅黑", 9F, System.Drawing.FontStyle.Bold);
            dataGridViewCellStyle2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(51)))), ((int)(((byte)(51)))), ((int)(((byte)(51)))));
            dataGridViewCellStyle2.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle2.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle2.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.dgvReport.ColumnHeadersDefaultCellStyle = dataGridViewCellStyle2;
            this.dgvReport.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgvReport.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Column1,
            this.Column2,
            this.Column3,
            this.Column4,
            this.Column5,
            this.Column6,
            this.Column7,
            this.Column8,
            this.Column9,
            this.Column10});
            dataGridViewCellStyle3.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            dataGridViewCellStyle3.BackColor = System.Drawing.SystemColors.Window;
            dataGridViewCellStyle3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            dataGridViewCellStyle3.ForeColor = System.Drawing.Color.Black;
            dataGridViewCellStyle3.SelectionBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(183)))), ((int)(((byte)(233)))), ((int)(((byte)(156)))));
            dataGridViewCellStyle3.SelectionForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(51)))), ((int)(((byte)(51)))), ((int)(((byte)(51)))));
            dataGridViewCellStyle3.WrapMode = System.Windows.Forms.DataGridViewTriState.False;
            this.dgvReport.DefaultCellStyle = dataGridViewCellStyle3;
            this.dgvReport.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dgvReport.EnableHeadersVisualStyles = false;
            this.dgvReport.GridColor = System.Drawing.Color.FromArgb(((int)(((byte)(221)))), ((int)(((byte)(221)))), ((int)(((byte)(221)))));
            this.dgvReport.Location = new System.Drawing.Point(0, 0);
            this.dgvReport.MergeColumnHeaderBackColor = System.Drawing.SystemColors.Control;
            this.dgvReport.MergeColumnNames = ((System.Collections.Generic.List<string>)(resources.GetObject("dgvReport.MergeColumnNames")));
            this.dgvReport.MultiSelect = false;
            this.dgvReport.Name = "dgvReport";
            this.dgvReport.ReadOnly = true;
            this.dgvReport.RowHeadersBorderStyle = System.Windows.Forms.DataGridViewHeaderBorderStyle.Single;
            dataGridViewCellStyle4.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(236)))), ((int)(((byte)(255)))));
            dataGridViewCellStyle4.SelectionBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(236)))), ((int)(((byte)(255)))));
            this.dgvReport.RowHeadersDefaultCellStyle = dataGridViewCellStyle4;
            this.dgvReport.RowHeadersVisible = false;
            dataGridViewCellStyle5.BackColor = System.Drawing.Color.White;
            dataGridViewCellStyle5.Font = new System.Drawing.Font("宋体", 9F);
            dataGridViewCellStyle5.ForeColor = System.Drawing.Color.Black;
            dataGridViewCellStyle5.SelectionBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(204)))), ((int)(((byte)(234)))), ((int)(((byte)(255)))));
            dataGridViewCellStyle5.SelectionForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(51)))), ((int)(((byte)(51)))), ((int)(((byte)(51)))));
            this.dgvReport.RowsDefaultCellStyle = dataGridViewCellStyle5;
            this.dgvReport.RowTemplate.Height = 23;
            this.dgvReport.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dgvReport.Size = new System.Drawing.Size(1067, 404);
            this.dgvReport.TabIndex = 0;
            this.dgvReport.CellFormatting += new System.Windows.Forms.DataGridViewCellFormattingEventHandler(this.dgvReport_CellFormatting);
            // 
            // Column1
            // 
            this.Column1.DataPropertyName = "配件编码";
            this.Column1.HeaderText = "配件编码";
            this.Column1.Name = "Column1";
            this.Column1.ReadOnly = true;
            // 
            // Column2
            // 
            this.Column2.DataPropertyName = "配件名称";
            this.Column2.HeaderText = "配件名称";
            this.Column2.Name = "Column2";
            this.Column2.ReadOnly = true;
            // 
            // Column3
            // 
            this.Column3.DataPropertyName = "配件图号";
            this.Column3.HeaderText = "配件图号";
            this.Column3.Name = "Column3";
            this.Column3.ReadOnly = true;
            this.Column3.Width = 80;
            // 
            // Column4
            // 
            this.Column4.DataPropertyName = "品牌";
            this.Column4.HeaderText = "品牌";
            this.Column4.Name = "Column4";
            this.Column4.ReadOnly = true;
            this.Column4.Width = 80;
            // 
            // Column5
            // 
            this.Column5.DataPropertyName = "厂商编码";
            this.Column5.HeaderText = "厂商编码";
            this.Column5.Name = "Column5";
            this.Column5.ReadOnly = true;
            this.Column5.Width = 80;
            // 
            // Column6
            // 
            this.Column6.DataPropertyName = "单位";
            this.Column6.HeaderText = "单位";
            this.Column6.Name = "Column6";
            this.Column6.ReadOnly = true;
            this.Column6.Width = 50;
            // 
            // Column7
            // 
            this.Column7.DataPropertyName = "数量";
            this.Column7.HeaderText = "数量";
            this.Column7.Name = "Column7";
            this.Column7.ReadOnly = true;
            this.Column7.Width = 60;
            // 
            // Column8
            // 
            this.Column8.DataPropertyName = "货款";
            this.Column8.HeaderText = "货款";
            this.Column8.Name = "Column8";
            this.Column8.ReadOnly = true;
            this.Column8.Width = 60;
            // 
            // Column9
            // 
            this.Column9.DataPropertyName = "税额";
            this.Column9.HeaderText = "税额";
            this.Column9.Name = "Column9";
            this.Column9.ReadOnly = true;
            this.Column9.Width = 60;
            // 
            // Column10
            // 
            this.Column10.DataPropertyName = "金额";
            this.Column10.HeaderText = "金额";
            this.Column10.Name = "Column10";
            this.Column10.ReadOnly = true;
            this.Column10.Width = 60;
            // 
            // UCPurchaseDaily
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Name = "UCPurchaseDaily";
            this.Load += new System.EventHandler(this.UCPurchaseDaily_Load);
            this.pnlSearch.ResumeLayout(false);
            this.pnlSearch.PerformLayout();
            this.pnlReport.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dt)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dgvReport)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private ServiceStationClient.ComponentUI.TextBoxEx txtPartsName;
        private ServiceStationClient.ComponentUI.TextBoxEx txtdrawing_num;
        private ServiceStationClient.ComponentUI.ComboBoxEx cboorg_id;
        private ServiceStationClient.ComponentUI.ComboBoxEx cboCompany;
        private ServiceStationClient.ComponentUI.TextBoxEx txtparts_brand;
        private ServiceStationClient.ComponentUI.TextBox.TextChooser txtcparts_code;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label12;
        private ServiceStationClient.ComponentUI.DateInterval dicreate_time;
        private System.Windows.Forms.Label label9;
        private ServiceStationClient.ComponentUI.DataGridViewReport dgvReport;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column1;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column2;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column3;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column4;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column5;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column6;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column7;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column8;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column9;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column10;
    }
}
