namespace DLE.NET
{
    partial class ReactorTool
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager (typeof (ReactorTool));
            this.groupBox2 = new System.Windows.Forms.GroupBox ();
            this.label5 = new System.Windows.Forms.Label ();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown ();
            this.label4 = new System.Windows.Forms.Label ();
            this.label1 = new System.Windows.Forms.Label ();
            this.reactorCountdown = new System.Windows.Forms.NumericUpDown ();
            this.textBox1 = new System.Windows.Forms.TextBox ();
            this.groupBox1 = new System.Windows.Forms.GroupBox ();
            this.label3 = new System.Windows.Forms.Label ();
            this.label2 = new System.Windows.Forms.Label ();
            this.addObjTarget = new System.Windows.Forms.Button ();
            this.addWallTarget = new System.Windows.Forms.Button ();
            this.delTarget = new System.Windows.Forms.Button ();
            this.addTarget = new System.Windows.Forms.Button ();
            this.triggerTarget = new System.Windows.Forms.TextBox ();
            this.reactorTargetPanel = new System.Windows.Forms.Panel ();
            this.reactorTargetList = new System.Windows.Forms.ListBox ();
            this.groupBox2.SuspendLayout ();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit ();
            ((System.ComponentModel.ISupportInitialize)(this.reactorCountdown)).BeginInit ();
            this.groupBox1.SuspendLayout ();
            this.reactorTargetPanel.SuspendLayout ();
            this.SuspendLayout ();
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add (this.label5);
            this.groupBox2.Controls.Add (this.numericUpDown1);
            this.groupBox2.Controls.Add (this.label4);
            this.groupBox2.Controls.Add (this.label1);
            this.groupBox2.Controls.Add (this.reactorCountdown);
            this.groupBox2.Controls.Add (this.textBox1);
            this.groupBox2.Location = new System.Drawing.Point (12, 188);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size (320, 110);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Descent 2 options";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point (34, 48);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size (88, 41);
            this.label5.TabIndex = 5;
            this.label5.Text = "Return segment after secret level:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // numericUpDown1
            // 
            this.numericUpDown1.Location = new System.Drawing.Point (125, 58);
            this.numericUpDown1.Maximum = new decimal (new int [] {
            7999,
            0,
            0,
            0});
            this.numericUpDown1.Name = "numericUpDown1";
            this.numericUpDown1.Size = new System.Drawing.Size (68, 20);
            this.numericUpDown1.TabIndex = 5;
            this.numericUpDown1.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point (196, 36);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size (24, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "sec";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point (58, 36);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size (64, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Countdown:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // reactorCountdown
            // 
            this.reactorCountdown.Location = new System.Drawing.Point (125, 32);
            this.reactorCountdown.Maximum = new decimal (new int [] {
            1000,
            0,
            0,
            0});
            this.reactorCountdown.Name = "reactorCountdown";
            this.reactorCountdown.Size = new System.Drawing.Size (68, 20);
            this.reactorCountdown.TabIndex = 5;
            this.reactorCountdown.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // textBox1
            // 
            this.textBox1.BackColor = System.Drawing.SystemColors.GradientActiveCaption;
            this.textBox1.Enabled = false;
            this.textBox1.Location = new System.Drawing.Point (238, 19);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size (76, 74);
            this.textBox1.TabIndex = 0;
            this.textBox1.Text = "Insane..: x 1\r\nAce.......: x 1.5\r\nHotshot: x 2\r\nRookie.: x 2.5\r\nTrainee: x 3";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add (this.label3);
            this.groupBox1.Controls.Add (this.label2);
            this.groupBox1.Controls.Add (this.addObjTarget);
            this.groupBox1.Controls.Add (this.addWallTarget);
            this.groupBox1.Controls.Add (this.delTarget);
            this.groupBox1.Controls.Add (this.addTarget);
            this.groupBox1.Controls.Add (this.triggerTarget);
            this.groupBox1.Controls.Add (this.reactorTargetPanel);
            this.groupBox1.Location = new System.Drawing.Point (12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size (320, 170);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Triggered when reactor blows up";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point (142, 31);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size (121, 13);
            this.label3.TabIndex = 34;
            this.label3.Text = "Segment, side activated";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point (56, 31);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size (73, 13);
            this.label2.TabIndex = 33;
            this.label2.Text = "Segment,Side";
            // 
            // addObjTarget
            // 
            this.addObjTarget.Location = new System.Drawing.Point (49, 133);
            this.addObjTarget.Name = "addObjTarget";
            this.addObjTarget.Size = new System.Drawing.Size (90, 22);
            this.addObjTarget.TabIndex = 4;
            this.addObjTarget.Text = "Add object";
            this.addObjTarget.UseVisualStyleBackColor = true;
            // 
            // addWallTarget
            // 
            this.addWallTarget.Location = new System.Drawing.Point (49, 111);
            this.addWallTarget.Name = "addWallTarget";
            this.addWallTarget.Size = new System.Drawing.Size (90, 22);
            this.addWallTarget.TabIndex = 3;
            this.addWallTarget.Text = "Add Wall";
            this.addWallTarget.UseVisualStyleBackColor = true;
            // 
            // delTarget
            // 
            this.delTarget.Location = new System.Drawing.Point (49, 89);
            this.delTarget.Name = "delTarget";
            this.delTarget.Size = new System.Drawing.Size (90, 22);
            this.delTarget.TabIndex = 2;
            this.delTarget.Text = "<-- Delete";
            this.delTarget.UseVisualStyleBackColor = true;
            // 
            // addTarget
            // 
            this.addTarget.Location = new System.Drawing.Point (49, 67);
            this.addTarget.Name = "addTarget";
            this.addTarget.Size = new System.Drawing.Size (90, 22);
            this.addTarget.TabIndex = 1;
            this.addTarget.Text = "Add -->";
            this.addTarget.UseVisualStyleBackColor = true;
            // 
            // triggerTarget
            // 
            this.triggerTarget.Location = new System.Drawing.Point (49, 47);
            this.triggerTarget.Name = "triggerTarget";
            this.triggerTarget.Size = new System.Drawing.Size (90, 20);
            this.triggerTarget.TabIndex = 0;
            this.triggerTarget.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // reactorTargetPanel
            // 
            this.reactorTargetPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.reactorTargetPanel.Controls.Add (this.reactorTargetList);
            this.reactorTargetPanel.Location = new System.Drawing.Point (145, 47);
            this.reactorTargetPanel.Name = "reactorTargetPanel";
            this.reactorTargetPanel.Size = new System.Drawing.Size (118, 112);
            this.reactorTargetPanel.TabIndex = 0;
            // 
            // reactorTargetList
            // 
            this.reactorTargetList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.reactorTargetList.FormattingEnabled = true;
            this.reactorTargetList.Location = new System.Drawing.Point (0, 0);
            this.reactorTargetList.Name = "reactorTargetList";
            this.reactorTargetList.Size = new System.Drawing.Size (114, 108);
            this.reactorTargetList.TabIndex = 0;
            // 
            // ReactorTool
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF (6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.GradientActiveCaption;
            this.ClientSize = new System.Drawing.Size (344, 812);
            this.Controls.Add (this.groupBox2);
            this.Controls.Add (this.groupBox1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject ("$this.Icon")));
            this.Name = "ReactorTool";
            this.Text = "Reactor";
            this.groupBox2.ResumeLayout (false);
            this.groupBox2.PerformLayout ();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit ();
            ((System.ComponentModel.ISupportInitialize)(this.reactorCountdown)).EndInit ();
            this.groupBox1.ResumeLayout (false);
            this.groupBox1.PerformLayout ();
            this.reactorTargetPanel.ResumeLayout (false);
            this.ResumeLayout (false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Panel reactorTargetPanel;
        private System.Windows.Forms.ListBox reactorTargetList;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button addObjTarget;
        private System.Windows.Forms.Button addWallTarget;
        private System.Windows.Forms.Button delTarget;
        private System.Windows.Forms.Button addTarget;
        private System.Windows.Forms.TextBox triggerTarget;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown numericUpDown1;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown reactorCountdown;
    }
}