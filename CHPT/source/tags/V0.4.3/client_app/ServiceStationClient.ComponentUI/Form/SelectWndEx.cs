using System;
using System.Data;
using System.Drawing;
using System.Windows.Forms;
using System.Threading;
using System.Collections;

namespace ServiceStationClient.ComponentUI
{   
    /// <summary>
    /// �����ˣ�yts
    /// ����ʱ�䣺2014.12.29
    /// </summary>
    public partial class SelectWndEx : Form
    {
        #region  --��Ա����
        private DataTable dtSource;  //����Դ
        private string displayColumnName;  //��ʾ����
        private string valueColumnName;
        private Control aimControl;  //Ŀ��ؼ�

        /// <summary>
        /// ѡ�п��ٲ�ѯ���¼�
        /// </summary>
        /// <param name="selectedText">ѡ������</param>
        public delegate void AfterSeletedHandler(string selectedText);
        public event AfterSeletedHandler AfterSeleted;

        /// <summary>
        /// ѡ�п��ٲ�ѯ���¼�
        /// </summary>
        /// <param name="dr">��������</param>
        public delegate void Multi_AfterSeletedHandler(DataRow dr);
        public event Multi_AfterSeletedHandler Multi_AfterSeleted;

        /// <summary>
        /// ��ȡ����Դ
        /// </summary>
        /// <param name="text">sql���</param>
        public delegate void GetDataSourceHandler(string sqlString);
        public event GetDataSourceHandler GetDataSourced;

        public delegate void UIHandler(DataTable dtDsp);
        private Thread threadHandler; 
        private bool isClickShowSource=false;
        private string[] multiDspColumns;
        private string tableName = string.Empty;
        private int offsetLocY = 0;
        #endregion

        #region --����
        /// <summary>
        /// ��ȡ����������Դ
        /// </summary>
        public DataTable DataSource
        {
            set
            {
                this.dtSource = value;
            }
            get
            {
                return this.dtSource;
            }
        }
        /// <summary>
        /// ���ʱ�Ƿ���ʾȫ��
        /// </summary>
        public bool IsClickDropDown
        {
            get
            {
                return this.isClickShowSource;
            }
            set
            {
                this.isClickShowSource = value;
            }
        }
        public string[] MultiDspColumns
        {
            get
            {
                return this.multiDspColumns;
            }
            set
            {
                this.multiDspColumns = value;
                this.GenerateColumns();
            }
        }
        public int OffsetLocY
        {
            get
            {
                return this.offsetLocY;
            }
            set
            {
                this.offsetLocY = value;
            }
        }
        #endregion

        #region --���캯��
        /// <summary>
        /// 
        /// </summary>
        /// <param name="tableName">Ҫ���������ݱ���</param>
        /// <param name="para_displayColumnName">��ʾ���ֶ�����</param>        
        /// <param name="para_valueColumnName">�����ֶ�</param>
        /// <param name="para_aimCtrl">���ٲ�ѯ���ֶ�</param>
        public SelectWndEx(string tableName, string para_displayColumnName, string para_valueColumnName, Control para_aimCtrl)
        {
            InitializeComponent();
            this.SetDgvStyle();
            this.dgvMain.AutoGenerateColumns = false;
            this.tableName = tableName;
            this.displayColumnName = para_displayColumnName;
            this.valueColumnName = para_valueColumnName;

            this.aimControl = para_aimCtrl;
            this.Column_Display.DataPropertyName = para_valueColumnName;
            this.multiDspColumns = new string[1] { para_valueColumnName };

            this.Size = new Size(1, 1);
            this.Location = new Point(0, -600);
            this.Show();
            this.Hide();

            if (para_aimCtrl != null)
            {
                this.aimControl.TextChanged -= new EventHandler(aimControl_TextChanged);
                this.aimControl.KeyDown -= new KeyEventHandler(aimControl_KeyDown);
                this.aimControl.Disposed -= new EventHandler(aimControl_Disposed);
                this.aimControl.LocationChanged -= new EventHandler(aimControl_LocationChanged);
                this.aimControl.TextChanged += new EventHandler(aimControl_TextChanged);
                this.aimControl.KeyDown += new KeyEventHandler(aimControl_KeyDown);
                this.aimControl.Disposed += new EventHandler(aimControl_Disposed);
                this.aimControl.LocationChanged += new EventHandler(aimControl_LocationChanged);
                this.aimControl.Click += new EventHandler(aimControl_Click);
                this.aimControl.Focus();
            }

        }
        #endregion

        #region --������ʽ
        public void SetDgvStyle()
        {
            Color dgvBackgroundColor1 = Color.White;
            Color dgvAlterRowBackColor = Color.FromArgb(255, 255, 255);   //���б���ɫ
            Color dgvGridColor = Color.FromArgb(195, 198, 202); //DataGridView�߿���ɫ
            Color dgvHeaderForeColor = Color.Black; //DataGridView����������ɫ
            Color dgvHeaderBackColor = Color.FromArgb(225, 231, 237);

            this.dgvMain.AutoGenerateColumns = false;
            this.dgvMain.ColumnHeadersHeightSizeMode = DataGridViewColumnHeadersHeightSizeMode.DisableResizing;
            this.dgvMain.EnableHeadersVisualStyles = false;
            this.dgvMain.GridColor = dgvGridColor;//����ִ���
            this.dgvMain.AlternatingRowsDefaultCellStyle.BackColor = dgvAlterRowBackColor;
            this.dgvMain.RowTemplate.Height = 22;           
            this.dgvMain.ColumnHeadersHeight = 22;            
            this.dgvMain.BackgroundColor = dgvBackgroundColor1;
            this.dgvMain.ColumnHeadersBorderStyle = DataGridViewHeaderBorderStyle.None;
            DataGridViewCellStyle headerStyle = new DataGridViewCellStyle();
            headerStyle.Font = new Font("����", 9);
            headerStyle.ForeColor = dgvHeaderForeColor;
            headerStyle.BackColor = dgvHeaderBackColor;
            headerStyle.SelectionBackColor = dgvHeaderBackColor;
            headerStyle.SelectionForeColor = dgvHeaderForeColor;
            this.dgvMain.ColumnHeadersDefaultCellStyle = headerStyle;

            DataGridViewCellStyle selectedRowStyle = new DataGridViewCellStyle();
            selectedRowStyle.Font = new Font("����", 9);
            selectedRowStyle.ForeColor = Color.Black;
            selectedRowStyle.SelectionForeColor = Color.Black;
            selectedRowStyle.SelectionBackColor = Color.FromArgb(51, 153, 255);
            this.dgvMain.DefaultCellStyle = selectedRowStyle;

            this.dgvMain.AllowUserToAddRows = false;
            this.dgvMain.AllowUserToDeleteRows = false;
            this.dgvMain.AllowUserToResizeColumns = false;
            this.dgvMain.AllowUserToResizeRows = false;
            return;
        }
        #endregion

        #region --Click�¼�
        void aimControl_Click(object sender, EventArgs e)
        {
            if (!this.isClickShowSource)
            {
                return;
            }
            this.Display(this.dtSource);
        }
        #endregion

        #region --PrePaint�¼�
        /// <summary>
        /// PrePaint�¼�
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void dgv_Msg_RowPrePaint(object sender, DataGridViewRowPrePaintEventArgs e)
        {
            e.PaintParts = e.PaintParts ^ DataGridViewPaintParts.Focus;
        }
        #endregion

        #region  --��Ա����
        /// <summary>
        /// �Ե��ڴ�С��λ��
        /// </summary>
        private void AdjustLocAndSize()
        {
            if(this.aimControl==null)
            {
                return;
            }

            int num = this.dgvMain.Rows.Count;
            if (num > 10)
            {
                this.dgvMain.ScrollBars = ScrollBars.Vertical;
                num = 10;
            }
            else
            {
                this.dgvMain.ScrollBars = ScrollBars.None;
            }

            //�����С
            if (this.dgvMain.ColumnHeadersVisible)
            {
                this.Size = new Size(this.aimControl.Width + 4,
                    num * this.dgvMain.RowTemplate.Height + this.dgvMain.ColumnHeadersHeight + 3);
            }
            else
            {
                this.Size = new Size(this.aimControl.Width + 4, num * this.dgvMain.RowTemplate.Height + 3);
            }
            this.dgvMain.Size = new Size(this.aimControl.Width + 2, this.Height - 2);

            //ʵ��λ��
            int loc_x = 0;
            int loc_y = 0;

            //��Ļ�ֱ��� ��С
            Rectangle rect = Screen.PrimaryScreen.Bounds;
            int resolutionX = rect.Width;
            int resolutionY = rect.Height;

            //�ѿؼ��ĵ�ǰ����ת��Ϊ��Ļ����
            Point screenPos = this.aimControl.Parent.PointToScreen(this.aimControl.Location);
            if (screenPos.X < 0)
            {
                loc_x = 0;
            }
            else if (screenPos.X + this.Width > resolutionX)
            {
                loc_x = resolutionX - this.Width;
            }
            else 
            {
                loc_x = screenPos.X;
            }


            if (screenPos.Y < 0)
            {
                loc_y = 0;
            }
            else if (screenPos.Y + this.Height + this.aimControl.Height > resolutionY)
            {
                loc_y = screenPos.Y - this.Height;
            }
            else
            {
                loc_y = screenPos.Y + this.aimControl.Height;
                if (this.aimControl.GetType() == typeof(DataGridViewTextBoxEditingControl))
                {
                    loc_y+= 4;
                }
            }

            if (loc_x < 0)
            {
                loc_x = 0;
            }

            if (loc_y < 0)
            {
                loc_y = 0;
            }

            this.Location = new Point(loc_x - 1, loc_y + this.offsetLocY + 1);
        }
        #endregion

        #region --ѡ���¼�
        private void dgvMain_CellClick(object sender, DataGridViewCellEventArgs e)
        {
            try
            {
                if (e.ColumnIndex < 0 || e.RowIndex < 0)
                {
                    return;
                }

                DataRow bindRow = (this.dgvMain.Rows[e.RowIndex].DataBoundItem as DataRowView).Row;
                if (bindRow == null)
                {
                    return;
                }
                if (!bindRow.Table.Columns.Contains(this.displayColumnName))
                {
                    return;
                }
                if (this.AfterSeleted != null)
                {
                    string display = bindRow[this.displayColumnName].ToString();                  
                    this.AfterSeleted(display);
                }
                if (this.Multi_AfterSeleted != null)
                {
                    this.Multi_AfterSeleted(bindRow);
                }

                this.Hide();
            }
            catch (Exception)
            { }
        }
        #endregion

        #region --����Բ��
        protected override void OnResize(System.EventArgs e)
        {
            this.Region = null;           
        }
        #endregion

        #region  --TextChanged�¼�
        void aimControl_TextChanged(object sender, EventArgs e)
        {
            try
            {
                string text = this.aimControl.Text.Trim();
                if (text.Length == 0)
                {
                    this.Hide();
                    return;
                }

                if (threadHandler != null)
                {
                    threadHandler.Abort();
                }
                if (GetDataSourced != null)
                {
                    string sqlString = string.Format("select top 10 * from {0} where ({1} like '%{2}%') or ({3} like '%{2}%')",
                        this.tableName, this.valueColumnName, text, this.displayColumnName);
                    this.GetDataSourced(sqlString);
                }       
            }
            catch (Exception)
            { }
        }
        #endregion

        #region --����
        public void Search(string text)
        {
            if (this.dtSource == null || text == null || text.Length == 0)
            {
                return;
            }
            foreach (DataRow dr in this.dtSource.Rows)
            {
                if (dr[this.valueColumnName].ToString() == text
                    || dr[this.displayColumnName].ToString() == text)
                {
                    return;
                }
            }
            //string pinYinShort = string.Empty;
            //foreach (DataRow dr in this.dtSource.Rows)
            //{
            //    pinYinShort = ConvertToPy.GetSpellStringSupper(dr[this.valueColumnName].ToString()).ToLower();                 
            //}
            this.Display(this.dtSource);
        }      
        #endregion

        #region --չ��
        private void Display(DataTable dtDsp)
        {
            if (dtDsp == null || dtDsp.Rows.Count <= 0)
            {
                this.Hide();
                this.aimControl.Focus();
                return;
            }

            try
            {

                if (!IsEqual((DataTable)this.dgvMain.DataSource, dtDsp))
                {
                    this.dgvMain.DataSource = dtDsp;
                }
                this.AdjustLocAndSize();
                this.Show();
                this.BringToFront();

                foreach (DataGridViewRow dgvr in this.dgvMain.Rows)
                {
                    dgvr.Selected = false;
                }

                if (this.dgvMain.Rows.Count > 0)
                {
                    this.dgvMain.Rows[0].Selected = true;                    
                    this.dgvMain.CurrentCell = this.dgvMain.Rows[0].Cells[0];
                }

                if (aimControl is System.Windows.Forms.TextBox)
                {
                    System.Windows.Forms.TextBox amTb = aimControl as System.Windows.Forms.TextBox;
                    amTb.SelectionLength = 0;
                    amTb.SelectionStart = amTb.Text.Length;
                }
                else if (aimControl is ComboBox)
                {
                    ComboBox amTb = aimControl as ComboBox;
                    amTb.SelectionLength = 0;
                    amTb.SelectionStart = amTb.Text.Length;
                }
            }
            catch (Exception)
            { }
        }
        #endregion

        #region  --Ŀ��ؼ��¼�
        void aimControl_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Down)
            {
                if (this.Visible == true)
                {
                    this.Activate();
                    this.dgvMain.Focus();
                    if (this.dgvMain.Rows.Count > 0)
                    {
                        this.dgvMain.Rows[0].Selected = true;
                        this.dgvMain.CurrentCell = this.dgvMain.Rows[0].Cells[0];
                    }
                    e.Handled = true;
                }
            }
        }
        /// <summary>
        /// ����
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void aimControl_Disposed(object sender, EventArgs e)
        {
            this.Close();
        }
        /// <summary>
        /// λ��
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void aimControl_LocationChanged(object sender, EventArgs e)
        {
            this.AdjustLocAndSize();
        }
        #endregion

        #region --DataGrideView KeyPress�¼�
        private void dgvMain_KeyPress(object sender, KeyPressEventArgs e)
        {
            Keys key = (Keys)e.KeyChar;
            int keyValue = e.KeyChar;
            if (this.dgvMain.SelectedRows.Count >0 && key == Keys.Enter)
            {
                try
                {
                    int index = this.dgvMain.SelectedRows[0].Index;
                              
                    DataRow bindRow = (this.dgvMain.Rows[index].DataBoundItem as DataRowView).Row;
                    if (bindRow == null)
                    {
                        return;
                    }

                    if (!bindRow.Table.Columns.Contains(this.displayColumnName))
                    {
                        return;
                    }
                    if (this.AfterSeleted != null)
                    {
                        string display = bindRow[this.displayColumnName].ToString();
                        this.AfterSeleted(display);
                    }
                    if (this.Multi_AfterSeleted != null)
                    {
                        this.Multi_AfterSeleted(bindRow);
                    }

                    this.Hide();
                    e.Handled = true;
                }
                catch (Exception)
                { }
            }
            else if (key == Keys.Back)
            {
                //this.aimControl.Focus();
                if (aimControl is System.Windows.Forms.TextBox)
                {
                    System.Windows.Forms.TextBox amTb = aimControl as System.Windows.Forms.TextBox;

                    if (amTb.Text != string.Empty)
                    {
                        amTb.Text = amTb.Text.Substring(0, amTb.Text.Length - 1);
                        amTb.SelectionLength = 0;
                        amTb.SelectionStart = amTb.Text.Length;
                    }
                }
                else if (aimControl is ComboBox)
                {
                    ComboBox amTb = aimControl as ComboBox;
                    if (amTb.Text != string.Empty)
                    {
                        amTb.Text = amTb.Text.Substring(0, amTb.Text.Length - 1);
                        amTb.SelectionLength = 0;
                        amTb.SelectionStart = amTb.Text.Length;
                    }
                }
            }
            else if (key == Keys.Down)
            {
                if (this.dgvMain.Rows.Count > 0 && this.dgvMain.SelectedRows.Count <= 0)
                {
                    this.dgvMain.Rows[0].Selected = true;
                    this.dgvMain.CurrentCell = this.dgvMain.Rows[0].Cells[0];
                }
            }
            else if (key != Keys.Down && key != Keys.Up && key != Keys.Left && key != Keys.Right 
                && key != Keys.Back && key != Keys.Escape && key != Keys.CapsLock && key != Keys.Shift && key != Keys.Tab)
            {
                this.aimControl.Focus();
                if (aimControl is System.Windows.Forms.TextBox)
                {
                    System.Windows.Forms.TextBox amTb = aimControl as System.Windows.Forms.TextBox;

                    char keyChar = e.KeyChar;
                    if (keyValue > 31)
                    {
                        amTb.Text += keyChar.ToString().ToLower();
                    }
                    amTb.SelectionLength = 0;
                    amTb.SelectionStart = amTb.Text.Length;
                }
                else if (aimControl is ComboBox)
                {
                    char keyChar = e.KeyChar;
                    ComboBox amTb = aimControl as ComboBox;
                    if (keyValue > 31)
                    {
                        amTb.Text += keyChar.ToString().ToLower();
                    }
                    amTb.SelectionLength = 0;
                    amTb.SelectionStart = amTb.Text.Length;
                }
            }
        }
        #endregion

        #region --Timer
        private void timerAutoHide_Tick(object sender, EventArgs e)
        {
            if (this.aimControl != null && this.aimControl.Focused == false && this.dgvMain.Focused == false)
            {
                this.Hide();
            }
        }

        private void SelectWndEx_VisibleChanged(object sender, EventArgs e)
        {
            if (this.Visible == true)
            {
                this.timerAutoHide.Enabled = true;
            }
            else
            {
                this.timerAutoHide.Enabled = false;
            }
        }
        #endregion

        #region --DeActivate
        private void SelectWndEx_Deactivate(object sender, EventArgs e)
        {
            if (!this.aimControl.Focused)
            {
                this.Hide();
            }
        }
        #endregion

        #region --������
        private void GenerateColumns()
        {
            if (this.multiDspColumns == null)
            {
                return;
            }

            int index = 0;
            this.dgvMain.ColumnHeadersVisible = false;
            foreach (string s in this.multiDspColumns)
            {
                if (index == 0)
                {
                    this.Column_Display.DataPropertyName = s;
                }
                else
                {
                    DataGridViewTextBoxColumn dgvc = new DataGridViewTextBoxColumn();
                    dgvc.Visible = true;
                    dgvc.HeaderText = "";
                    dgvc.DataPropertyName = s;
                    this.dgvMain.Columns.Add(dgvc);
                }
                index++;
            }
        }
        #endregion

        #region --�ж�����DataTable�Ƿ����
        private bool IsEqual(DataTable dt1, DataTable dt2)
        {
            if (dt1 == null || dt2 == null)
            {
                return false;
            }

            if (dt1.Rows.Count != dt2.Rows.Count)
            {
                return false;
            }

            for(int i=0;i<dt1.Rows.Count;i++)
            {
                foreach (DataColumn dc in dt1.Columns)
                {
                    if (dt1.Rows[i][dc.ColumnName] != dt2.Rows[i][dc.ColumnName])
                    {
                        return false;
                    }
                }
            }
            return true;
        }
        #endregion
    }

    public sealed class DataGridViewExt : DataGridView
    {
        private int currentIndex = 0;

        public DataGridViewExt()
        {
            this.SetDgvStyle();
        }

        #region --������ʽ
        public void SetDgvStyle()
        {
            Color dgvBackgroundColor1 = Color.White;
            Color dgvAlterRowBackColor = Color.FromArgb(255, 255, 255);   //���б���ɫ
            Color dgvGridColor = Color.FromArgb(195, 198, 202); //DataGridView�߿���ɫ
            Color dgvHeaderForeColor = Color.Black; //DataGridView����������ɫ
            Color dgvHeaderBackColor = Color.FromArgb(225, 231, 237);

            this.AutoGenerateColumns = false;
            this.ColumnHeadersHeightSizeMode = DataGridViewColumnHeadersHeightSizeMode.DisableResizing;
            this.EnableHeadersVisualStyles = false;
            this.GridColor = dgvGridColor;//����ִ���
            this.AlternatingRowsDefaultCellStyle.BackColor = dgvAlterRowBackColor;
            this.RowTemplate.Height = 22;
            this.ColumnHeadersHeight = 22;
            this.BackgroundColor = dgvBackgroundColor1;
            this.ColumnHeadersBorderStyle = DataGridViewHeaderBorderStyle.None;
            DataGridViewCellStyle headerStyle = new DataGridViewCellStyle();
            headerStyle.Font = new Font("����", 9);
            headerStyle.ForeColor = dgvHeaderForeColor;
            headerStyle.BackColor = dgvHeaderBackColor;
            headerStyle.SelectionBackColor = dgvHeaderBackColor;
            headerStyle.SelectionForeColor = dgvHeaderForeColor;
            this.ColumnHeadersDefaultCellStyle = headerStyle;

            DataGridViewCellStyle selectedRowStyle = new DataGridViewCellStyle();
            selectedRowStyle.Font = new Font("����", 9);
            selectedRowStyle.ForeColor = Color.Black;
            selectedRowStyle.SelectionForeColor = Color.Black;
            selectedRowStyle.SelectionBackColor = Color.FromArgb(51, 153, 255);
            this.DefaultCellStyle = selectedRowStyle;

            this.AllowUserToAddRows = false;
            this.AllowUserToDeleteRows = false;
            this.AllowUserToResizeColumns = false;
            this.AllowUserToResizeRows = false;
            return;
        }
        #endregion

        #region --��ݼ�
        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            if (keyData == Keys.Enter)
            {
                if (this.CurrentRow != null)
                {
                    this.currentIndex = this.CurrentRow.Index;
                }
                else
                {
                    this.currentIndex = -1;
                }
            }
            return base.ProcessCmdKey(ref msg, keyData);
        }
        #endregion

        protected override void OnKeyPress(KeyPressEventArgs e)
        {
            if (this.currentIndex >= 0 && this.currentIndex < this.Rows.Count)
            {
                this.CurrentCell = this[0, this.currentIndex];
            }
            base.OnKeyPress(e);
        }
    }
}