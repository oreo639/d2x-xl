namespace DLE.NET
{
    partial class MissionTool
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MissionTool));
            this.dataGroup = new System.Windows.Forms.GroupBox();
            this.labelObjTexture = new System.Windows.Forms.Label();
            this.labelObjAI = new System.Windows.Forms.Label();
            this.labelObjId = new System.Windows.Forms.Label();
            this.labelObjType = new System.Windows.Forms.Label();
            this.textureDisplay = new System.Windows.Forms.Panel();
            this.objectTexture = new System.Windows.Forms.ComboBox();
            this.objectAI = new System.Windows.Forms.ComboBox();
            this.objectId = new System.Windows.Forms.ComboBox();
            this.objectType = new System.Windows.Forms.ComboBox();
            this.locationLabel = new System.Windows.Forms.Label();
            this.multiplayerCheck = new System.Windows.Forms.CheckBox();
            this.sortCheck = new System.Windows.Forms.CheckBox();
            this.objectCountLabel = new System.Windows.Forms.Label();
            this.objectDisplay = new System.Windows.Forms.Panel();
            this.labelObjDesc = new System.Windows.Forms.Label();
            this.objectDescription = new System.Windows.Forms.ComboBox();
            this.spawnGroup = new System.Windows.Forms.GroupBox();
            this.spawnId = new System.Windows.Forms.ComboBox();
            this.spawnDisplay = new System.Windows.Forms.Panel();
            this.labelSpawnType = new System.Windows.Forms.Label();
            this.spawnType = new System.Windows.Forms.ComboBox();
            this.labelSpawnId = new System.Windows.Forms.Label();
            this.spawnQuantity = new System.Windows.Forms.NumericUpDown();
            this.labelSpawnQty = new System.Windows.Forms.Label();
            this.weaponGroup = new System.Windows.Forms.GroupBox();
            this.primaryWeapon = new System.Windows.Forms.ComboBox();
            this.secondaryWeapon = new System.Windows.Forms.ComboBox();
            this.labelPrimaryWeapon = new System.Windows.Forms.Label();
            this.labelSecondaryWeapon = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.explType = new System.Windows.Forms.ComboBox();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.numericUpDown2 = new System.Windows.Forms.NumericUpDown();
            this.dataGroup.SuspendLayout();
            this.spawnGroup.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.spawnQuantity)).BeginInit();
            this.weaponGroup.SuspendLayout();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).BeginInit();
            this.SuspendLayout();
            // 
            // dataGroup
            // 
            this.dataGroup.Controls.Add(this.labelObjTexture);
            this.dataGroup.Controls.Add(this.labelObjAI);
            this.dataGroup.Controls.Add(this.labelObjId);
            this.dataGroup.Controls.Add(this.labelObjType);
            this.dataGroup.Controls.Add(this.textureDisplay);
            this.dataGroup.Controls.Add(this.objectTexture);
            this.dataGroup.Controls.Add(this.objectAI);
            this.dataGroup.Controls.Add(this.objectId);
            this.dataGroup.Controls.Add(this.objectType);
            this.dataGroup.Controls.Add(this.locationLabel);
            this.dataGroup.Controls.Add(this.multiplayerCheck);
            this.dataGroup.Controls.Add(this.sortCheck);
            this.dataGroup.Controls.Add(this.objectCountLabel);
            this.dataGroup.Controls.Add(this.objectDisplay);
            this.dataGroup.Controls.Add(this.labelObjDesc);
            this.dataGroup.Controls.Add(this.objectDescription);
            this.dataGroup.Location = new System.Drawing.Point(12, 12);
            this.dataGroup.Name = "dataGroup";
            this.dataGroup.Size = new System.Drawing.Size(318, 160);
            this.dataGroup.TabIndex = 0;
            this.dataGroup.TabStop = false;
            this.dataGroup.Text = "Object data";
            // 
            // labelObjTexture
            // 
            this.labelObjTexture.AutoSize = true;
            this.labelObjTexture.Location = new System.Drawing.Point(1, 132);
            this.labelObjTexture.Name = "labelObjTexture";
            this.labelObjTexture.Size = new System.Drawing.Size(43, 13);
            this.labelObjTexture.TabIndex = 15;
            this.labelObjTexture.Text = "Texture";
            this.labelObjTexture.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // labelObjAI
            // 
            this.labelObjAI.AutoSize = true;
            this.labelObjAI.Location = new System.Drawing.Point(27, 105);
            this.labelObjAI.Name = "labelObjAI";
            this.labelObjAI.Size = new System.Drawing.Size(17, 13);
            this.labelObjAI.TabIndex = 14;
            this.labelObjAI.Text = "AI";
            this.labelObjAI.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // labelObjId
            // 
            this.labelObjId.AutoSize = true;
            this.labelObjId.Location = new System.Drawing.Point(28, 78);
            this.labelObjId.Name = "labelObjId";
            this.labelObjId.Size = new System.Drawing.Size(16, 13);
            this.labelObjId.TabIndex = 13;
            this.labelObjId.Text = "Id";
            this.labelObjId.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // labelObjType
            // 
            this.labelObjType.AutoSize = true;
            this.labelObjType.Location = new System.Drawing.Point(13, 50);
            this.labelObjType.Name = "labelObjType";
            this.labelObjType.Size = new System.Drawing.Size(31, 13);
            this.labelObjType.TabIndex = 12;
            this.labelObjType.Text = "Type";
            this.labelObjType.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // textureDisplay
            // 
            this.textureDisplay.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.textureDisplay.Location = new System.Drawing.Point(241, 78);
            this.textureDisplay.Name = "textureDisplay";
            this.textureDisplay.Size = new System.Drawing.Size(70, 70);
            this.textureDisplay.TabIndex = 11;
            // 
            // objectTexture
            // 
            this.objectTexture.FormattingEnabled = true;
            this.objectTexture.Location = new System.Drawing.Point(46, 127);
            this.objectTexture.Name = "objectTexture";
            this.objectTexture.Size = new System.Drawing.Size(100, 21);
            this.objectTexture.TabIndex = 10;
            // 
            // objectAI
            // 
            this.objectAI.FormattingEnabled = true;
            this.objectAI.Location = new System.Drawing.Point(46, 100);
            this.objectAI.Name = "objectAI";
            this.objectAI.Size = new System.Drawing.Size(100, 21);
            this.objectAI.TabIndex = 9;
            // 
            // objectId
            // 
            this.objectId.FormattingEnabled = true;
            this.objectId.Location = new System.Drawing.Point(46, 73);
            this.objectId.Name = "objectId";
            this.objectId.Size = new System.Drawing.Size(60, 21);
            this.objectId.TabIndex = 8;
            // 
            // objectType
            // 
            this.objectType.FormattingEnabled = true;
            this.objectType.Location = new System.Drawing.Point(46, 46);
            this.objectType.Name = "objectType";
            this.objectType.Size = new System.Drawing.Size(100, 21);
            this.objectType.TabIndex = 7;
            // 
            // locationLabel
            // 
            this.locationLabel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.locationLabel.Location = new System.Drawing.Point(152, 102);
            this.locationLabel.Name = "locationLabel";
            this.locationLabel.Size = new System.Drawing.Size(80, 46);
            this.locationLabel.TabIndex = 6;
            this.locationLabel.Text = "label2";
            // 
            // multiplayerCheck
            // 
            this.multiplayerCheck.AutoSize = true;
            this.multiplayerCheck.Location = new System.Drawing.Point(236, 41);
            this.multiplayerCheck.Name = "multiplayerCheck";
            this.multiplayerCheck.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.multiplayerCheck.Size = new System.Drawing.Size(76, 17);
            this.multiplayerCheck.TabIndex = 5;
            this.multiplayerCheck.Text = "Multiplayer";
            this.multiplayerCheck.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.multiplayerCheck.UseVisualStyleBackColor = true;
            // 
            // sortCheck
            // 
            this.sortCheck.AutoSize = true;
            this.sortCheck.Location = new System.Drawing.Point(267, 18);
            this.sortCheck.Name = "sortCheck";
            this.sortCheck.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.sortCheck.Size = new System.Drawing.Size(45, 17);
            this.sortCheck.TabIndex = 4;
            this.sortCheck.Text = "Sort";
            this.sortCheck.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.sortCheck.UseVisualStyleBackColor = true;
            // 
            // objectCountLabel
            // 
            this.objectCountLabel.AutoSize = true;
            this.objectCountLabel.Location = new System.Drawing.Point(238, 19);
            this.objectCountLabel.Name = "objectCountLabel";
            this.objectCountLabel.Size = new System.Drawing.Size(20, 13);
            this.objectCountLabel.TabIndex = 3;
            this.objectCountLabel.Text = "#: ";
            // 
            // objectDisplay
            // 
            this.objectDisplay.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.objectDisplay.Location = new System.Drawing.Point(152, 19);
            this.objectDisplay.Name = "objectDisplay";
            this.objectDisplay.Size = new System.Drawing.Size(80, 80);
            this.objectDisplay.TabIndex = 2;
            // 
            // labelObjDesc
            // 
            this.labelObjDesc.AutoSize = true;
            this.labelObjDesc.Location = new System.Drawing.Point(6, 22);
            this.labelObjDesc.Name = "labelObjDesc";
            this.labelObjDesc.Size = new System.Drawing.Size(38, 13);
            this.labelObjDesc.TabIndex = 1;
            this.labelObjDesc.Text = "Object";
            this.labelObjDesc.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // objectDescription
            // 
            this.objectDescription.FormattingEnabled = true;
            this.objectDescription.Location = new System.Drawing.Point(46, 19);
            this.objectDescription.Name = "objectDescription";
            this.objectDescription.Size = new System.Drawing.Size(100, 21);
            this.objectDescription.TabIndex = 0;
            // 
            // spawnGroup
            // 
            this.spawnGroup.Controls.Add(this.labelSpawnQty);
            this.spawnGroup.Controls.Add(this.spawnQuantity);
            this.spawnGroup.Controls.Add(this.labelSpawnId);
            this.spawnGroup.Controls.Add(this.spawnId);
            this.spawnGroup.Controls.Add(this.spawnDisplay);
            this.spawnGroup.Controls.Add(this.labelSpawnType);
            this.spawnGroup.Controls.Add(this.spawnType);
            this.spawnGroup.Location = new System.Drawing.Point(12, 178);
            this.spawnGroup.Name = "spawnGroup";
            this.spawnGroup.Size = new System.Drawing.Size(318, 110);
            this.spawnGroup.TabIndex = 1;
            this.spawnGroup.TabStop = false;
            this.spawnGroup.Text = "This robot spawns";
            // 
            // spawnId
            // 
            this.spawnId.FormattingEnabled = true;
            this.spawnId.Location = new System.Drawing.Point(46, 46);
            this.spawnId.Name = "spawnId";
            this.spawnId.Size = new System.Drawing.Size(60, 21);
            this.spawnId.TabIndex = 8;
            // 
            // spawnDisplay
            // 
            this.spawnDisplay.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.spawnDisplay.Location = new System.Drawing.Point(152, 19);
            this.spawnDisplay.Name = "spawnDisplay";
            this.spawnDisplay.Size = new System.Drawing.Size(80, 80);
            this.spawnDisplay.TabIndex = 2;
            // 
            // labelSpawnType
            // 
            this.labelSpawnType.AutoSize = true;
            this.labelSpawnType.Location = new System.Drawing.Point(13, 22);
            this.labelSpawnType.Name = "labelSpawnType";
            this.labelSpawnType.Size = new System.Drawing.Size(31, 13);
            this.labelSpawnType.TabIndex = 1;
            this.labelSpawnType.Text = "Type";
            this.labelSpawnType.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // spawnType
            // 
            this.spawnType.FormattingEnabled = true;
            this.spawnType.Location = new System.Drawing.Point(46, 19);
            this.spawnType.Name = "spawnType";
            this.spawnType.Size = new System.Drawing.Size(100, 21);
            this.spawnType.TabIndex = 0;
            // 
            // labelSpawnId
            // 
            this.labelSpawnId.AutoSize = true;
            this.labelSpawnId.Location = new System.Drawing.Point(28, 49);
            this.labelSpawnId.Name = "labelSpawnId";
            this.labelSpawnId.Size = new System.Drawing.Size(16, 13);
            this.labelSpawnId.TabIndex = 14;
            this.labelSpawnId.Text = "Id";
            this.labelSpawnId.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // spawnQuantity
            // 
            this.spawnQuantity.Location = new System.Drawing.Point(46, 76);
            this.spawnQuantity.Name = "spawnQuantity";
            this.spawnQuantity.Size = new System.Drawing.Size(60, 20);
            this.spawnQuantity.TabIndex = 15;
            this.spawnQuantity.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // labelSpawnQty
            // 
            this.labelSpawnQty.AutoSize = true;
            this.labelSpawnQty.Location = new System.Drawing.Point(22, 79);
            this.labelSpawnQty.Name = "labelSpawnQty";
            this.labelSpawnQty.Size = new System.Drawing.Size(23, 13);
            this.labelSpawnQty.TabIndex = 16;
            this.labelSpawnQty.Text = "Qty";
            this.labelSpawnQty.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // weaponGroup
            // 
            this.weaponGroup.Controls.Add(this.labelSecondaryWeapon);
            this.weaponGroup.Controls.Add(this.labelPrimaryWeapon);
            this.weaponGroup.Controls.Add(this.secondaryWeapon);
            this.weaponGroup.Controls.Add(this.primaryWeapon);
            this.weaponGroup.Location = new System.Drawing.Point(12, 294);
            this.weaponGroup.Name = "weaponGroup";
            this.weaponGroup.Size = new System.Drawing.Size(154, 108);
            this.weaponGroup.TabIndex = 2;
            this.weaponGroup.TabStop = false;
            this.weaponGroup.Text = "Weapons";
            // 
            // primaryWeapon
            // 
            this.primaryWeapon.FormattingEnabled = true;
            this.primaryWeapon.Location = new System.Drawing.Point(46, 19);
            this.primaryWeapon.Name = "primaryWeapon";
            this.primaryWeapon.Size = new System.Drawing.Size(100, 21);
            this.primaryWeapon.TabIndex = 1;
            // 
            // secondaryWeapon
            // 
            this.secondaryWeapon.FormattingEnabled = true;
            this.secondaryWeapon.Location = new System.Drawing.Point(46, 46);
            this.secondaryWeapon.Name = "secondaryWeapon";
            this.secondaryWeapon.Size = new System.Drawing.Size(100, 21);
            this.secondaryWeapon.TabIndex = 2;
            // 
            // labelPrimaryWeapon
            // 
            this.labelPrimaryWeapon.AutoSize = true;
            this.labelPrimaryWeapon.Location = new System.Drawing.Point(23, 23);
            this.labelPrimaryWeapon.Name = "labelPrimaryWeapon";
            this.labelPrimaryWeapon.Size = new System.Drawing.Size(21, 13);
            this.labelPrimaryWeapon.TabIndex = 15;
            this.labelPrimaryWeapon.Text = "1st";
            this.labelPrimaryWeapon.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // labelSecondaryWeapon
            // 
            this.labelSecondaryWeapon.AutoSize = true;
            this.labelSecondaryWeapon.Location = new System.Drawing.Point(18, 50);
            this.labelSecondaryWeapon.Name = "labelSecondaryWeapon";
            this.labelSecondaryWeapon.Size = new System.Drawing.Size(25, 13);
            this.labelSecondaryWeapon.TabIndex = 16;
            this.labelSecondaryWeapon.Text = "2nd";
            this.labelSecondaryWeapon.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.numericUpDown2);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.numericUpDown1);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.explType);
            this.groupBox1.Location = new System.Drawing.Point(176, 294);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(154, 108);
            this.groupBox1.TabIndex = 3;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Death";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 75);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(31, 13);
            this.label1.TabIndex = 16;
            this.label1.Text = "Type";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // explType
            // 
            this.explType.FormattingEnabled = true;
            this.explType.Location = new System.Drawing.Point(43, 72);
            this.explType.Name = "explType";
            this.explType.Size = new System.Drawing.Size(100, 21);
            this.explType.TabIndex = 2;
            // 
            // numericUpDown1
            // 
            this.numericUpDown1.Location = new System.Drawing.Point(60, 19);
            this.numericUpDown1.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.numericUpDown1.Name = "numericUpDown1";
            this.numericUpDown1.Size = new System.Drawing.Size(60, 20);
            this.numericUpDown1.TabIndex = 17;
            this.numericUpDown1.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.numericUpDown1.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(28, 22);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(30, 13);
            this.label2.TabIndex = 18;
            this.label2.Text = "Rolls";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(11, 46);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(47, 13);
            this.label3.TabIndex = 20;
            this.label3.Text = "Strength";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // numericUpDown2
            // 
            this.numericUpDown2.Location = new System.Drawing.Point(60, 43);
            this.numericUpDown2.Name = "numericUpDown2";
            this.numericUpDown2.Size = new System.Drawing.Size(60, 20);
            this.numericUpDown2.TabIndex = 19;
            this.numericUpDown2.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.numericUpDown2.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
            // 
            // MissionTool
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.GradientActiveCaption;
            this.ClientSize = new System.Drawing.Size(344, 812);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.weaponGroup);
            this.Controls.Add(this.spawnGroup);
            this.Controls.Add(this.dataGroup);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "MissionTool";
            this.Text = "Mission";
            this.dataGroup.ResumeLayout(false);
            this.dataGroup.PerformLayout();
            this.spawnGroup.ResumeLayout(false);
            this.spawnGroup.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.spawnQuantity)).EndInit();
            this.weaponGroup.ResumeLayout(false);
            this.weaponGroup.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox dataGroup;
        private System.Windows.Forms.CheckBox sortCheck;
        private System.Windows.Forms.Label objectCountLabel;
        private System.Windows.Forms.Label labelObjDesc;
        private System.Windows.Forms.ComboBox objectDescription;
        private System.Windows.Forms.CheckBox multiplayerCheck;
        private System.Windows.Forms.Label locationLabel;
        private System.Windows.Forms.ComboBox objectId;
        private System.Windows.Forms.ComboBox objectType;
        private System.Windows.Forms.ComboBox objectAI;
        private System.Windows.Forms.ComboBox objectTexture;
        private System.Windows.Forms.Panel textureDisplay;
        private System.Windows.Forms.Panel objectDisplay;
        private System.Windows.Forms.Label labelObjAI;
        private System.Windows.Forms.Label labelObjId;
        private System.Windows.Forms.Label labelObjType;
        private System.Windows.Forms.GroupBox spawnGroup;
        private System.Windows.Forms.ComboBox spawnId;
        private System.Windows.Forms.Panel spawnDisplay;
        private System.Windows.Forms.Label labelSpawnType;
        private System.Windows.Forms.ComboBox spawnType;
        private System.Windows.Forms.Label labelObjTexture;
        private System.Windows.Forms.Label labelSpawnId;
        private System.Windows.Forms.NumericUpDown spawnQuantity;
        private System.Windows.Forms.Label labelSpawnQty;
        private System.Windows.Forms.GroupBox weaponGroup;
        private System.Windows.Forms.Label labelSecondaryWeapon;
        private System.Windows.Forms.Label labelPrimaryWeapon;
        private System.Windows.Forms.ComboBox secondaryWeapon;
        private System.Windows.Forms.ComboBox primaryWeapon;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox explType;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown numericUpDown2;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown numericUpDown1;
    }
}