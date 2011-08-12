namespace DLEdotNET
{
    partial class DiagnosisTool
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager (typeof (DiagnosisTool));
            this.groupBox2 = new System.Windows.Forms.GroupBox ();
            this.panel2 = new System.Windows.Forms.Panel ();
            this.problemReport = new System.Windows.Forms.ListBox ();
            this.showWarnCheck = new System.Windows.Forms.CheckBox ();
            this.autoFixCheck = new System.Windows.Forms.CheckBox ();
            this.clearListBtn = new System.Windows.Forms.Button ();
            this.showProblemBtn = new System.Windows.Forms.Button ();
            this.runCheckBtn = new System.Windows.Forms.Button ();
            this.groupBox1 = new System.Windows.Forms.GroupBox ();
            this.panel1 = new System.Windows.Forms.Panel ();
            this.statisticsView = new System.Windows.Forms.ListView ();
            this.Item = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader ()));
            this.Count = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader ()));
            this.maxCount = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader ()));
            this.groupBox2.SuspendLayout ();
            this.panel2.SuspendLayout ();
            this.groupBox1.SuspendLayout ();
            this.panel1.SuspendLayout ();
            this.SuspendLayout ();
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add (this.panel2);
            this.groupBox2.Location = new System.Drawing.Point (13, 256);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size (328, 544);
            this.groupBox2.TabIndex = 8;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Problem report";
            // 
            // panel2
            // 
            this.panel2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel2.Controls.Add (this.problemReport);
            this.panel2.Location = new System.Drawing.Point (8, 19);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size (311, 513);
            this.panel2.TabIndex = 0;
            // 
            // problemReport
            // 
            this.problemReport.Dock = System.Windows.Forms.DockStyle.Fill;
            this.problemReport.FormattingEnabled = true;
            this.problemReport.Location = new System.Drawing.Point (0, 0);
            this.problemReport.Name = "problemReport";
            this.problemReport.Size = new System.Drawing.Size (307, 509);
            this.problemReport.TabIndex = 7;
            // 
            // showWarnCheck
            // 
            this.showWarnCheck.AutoSize = true;
            this.showWarnCheck.Location = new System.Drawing.Point (244, 159);
            this.showWarnCheck.Name = "showWarnCheck";
            this.showWarnCheck.Size = new System.Drawing.Size (98, 17);
            this.showWarnCheck.TabIndex = 4;
            this.showWarnCheck.Text = "Show warnings";
            this.showWarnCheck.UseVisualStyleBackColor = true;
            // 
            // autoFixCheck
            // 
            this.autoFixCheck.AutoSize = true;
            this.autoFixCheck.Location = new System.Drawing.Point (245, 136);
            this.autoFixCheck.Name = "autoFixCheck";
            this.autoFixCheck.Size = new System.Drawing.Size (87, 17);
            this.autoFixCheck.TabIndex = 3;
            this.autoFixCheck.Text = "Auto fix bugs";
            this.autoFixCheck.UseVisualStyleBackColor = true;
            // 
            // clearListBtn
            // 
            this.clearListBtn.Location = new System.Drawing.Point (245, 89);
            this.clearListBtn.Name = "clearListBtn";
            this.clearListBtn.Size = new System.Drawing.Size (87, 23);
            this.clearListBtn.TabIndex = 2;
            this.clearListBtn.Text = "Clear list";
            this.clearListBtn.UseVisualStyleBackColor = true;
            // 
            // showProblemBtn
            // 
            this.showProblemBtn.Location = new System.Drawing.Point (245, 60);
            this.showProblemBtn.Name = "showProblemBtn";
            this.showProblemBtn.Size = new System.Drawing.Size (87, 23);
            this.showProblemBtn.TabIndex = 1;
            this.showProblemBtn.Text = "Show problem";
            this.showProblemBtn.UseVisualStyleBackColor = true;
            // 
            // runCheckBtn
            // 
            this.runCheckBtn.Location = new System.Drawing.Point (245, 31);
            this.runCheckBtn.Name = "runCheckBtn";
            this.runCheckBtn.Size = new System.Drawing.Size (87, 23);
            this.runCheckBtn.TabIndex = 0;
            this.runCheckBtn.Text = "Check mine";
            this.runCheckBtn.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add (this.panel1);
            this.groupBox1.Location = new System.Drawing.Point (12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size (226, 238);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Statistics";
            // 
            // panel1
            // 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel1.Controls.Add (this.statisticsView);
            this.panel1.Location = new System.Drawing.Point (7, 19);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size (213, 213);
            this.panel1.TabIndex = 0;
            // 
            // statisticsView
            // 
            this.statisticsView.BackColor = System.Drawing.SystemColors.Window;
            this.statisticsView.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.statisticsView.Columns.AddRange (new System.Windows.Forms.ColumnHeader [] {
            this.Item,
            this.Count,
            this.maxCount});
            this.statisticsView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.statisticsView.FullRowSelect = true;
            this.statisticsView.GridLines = true;
            this.statisticsView.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.statisticsView.Location = new System.Drawing.Point (0, 0);
            this.statisticsView.MultiSelect = false;
            this.statisticsView.Name = "statisticsView";
            this.statisticsView.Size = new System.Drawing.Size (209, 209);
            this.statisticsView.TabIndex = 0;
            this.statisticsView.UseCompatibleStateImageBehavior = false;
            this.statisticsView.View = System.Windows.Forms.View.Details;
            // 
            // Item
            // 
            this.Item.Text = "Item";
            // 
            // Count
            // 
            this.Count.Text = "Count";
            // 
            // maxCount
            // 
            this.maxCount.Text = "Max #";
            // 
            // DiagnosisTool
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF (6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.GradientActiveCaption;
            this.ClientSize = new System.Drawing.Size (344, 812);
            this.Controls.Add (this.groupBox2);
            this.Controls.Add (this.showWarnCheck);
            this.Controls.Add (this.autoFixCheck);
            this.Controls.Add (this.clearListBtn);
            this.Controls.Add (this.showProblemBtn);
            this.Controls.Add (this.runCheckBtn);
            this.Controls.Add (this.groupBox1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject ("$this.Icon")));
            this.Name = "DiagnosisTool";
            this.Text = "Diagnosis";
            this.groupBox2.ResumeLayout (false);
            this.panel2.ResumeLayout (false);
            this.groupBox1.ResumeLayout (false);
            this.panel1.ResumeLayout (false);
            this.ResumeLayout (false);
            this.PerformLayout ();

        }

        #endregion

        private System.Windows.Forms.ListView statisticsView;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button runCheckBtn;
        private System.Windows.Forms.Button showProblemBtn;
        private System.Windows.Forms.Button clearListBtn;
        private System.Windows.Forms.CheckBox autoFixCheck;
        private System.Windows.Forms.CheckBox showWarnCheck;
        private System.Windows.Forms.ListBox problemReport;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.ColumnHeader Item;
        private System.Windows.Forms.ColumnHeader Count;
        private System.Windows.Forms.ColumnHeader maxCount;
        private System.Windows.Forms.Panel panel2;
    }
}