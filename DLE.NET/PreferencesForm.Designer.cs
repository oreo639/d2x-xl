namespace DLEdotNET
{
    partial class PreferencesForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(PreferencesForm));
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.dataBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            this.D1PIGFolder = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.browseForD1PIG = new System.Windows.Forms.Button();
            this.browseForD2PIG = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.D2PIGFOLDER = new System.Windows.Forms.TextBox();
            this.browseForMissions = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.missionFolder = new System.Windows.Forms.TextBox();
            this.changeFolders = new System.Windows.Forms.Button();
            this.restoreFolders = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.groupBox7 = new System.Windows.Forms.GroupBox();
            this.groupBox8 = new System.Windows.Forms.GroupBox();
            this.groupBox9 = new System.Windows.Forms.GroupBox();
            this.groupBox10 = new System.Windows.Forms.GroupBox();
            this.depthPerceptionOff = new System.Windows.Forms.RadioButton();
            this.depthPerceptionLow = new System.Windows.Forms.RadioButton();
            this.depthPerceptionMedium = new System.Windows.Forms.RadioButton();
            this.depthPerceptionHigh = new System.Windows.Forms.RadioButton();
            this.moveRate = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.playerProfile = new System.Windows.Forms.TextBox();
            this.rotateRate12 = new System.Windows.Forms.RadioButton();
            this.rotateRate6 = new System.Windows.Forms.RadioButton();
            this.rotateRate3 = new System.Windows.Forms.RadioButton();
            this.rotateRate1dot5 = new System.Windows.Forms.RadioButton();
            this.rotateRate45 = new System.Windows.Forms.RadioButton();
            this.renderDepthSlider = new System.Windows.Forms.TrackBar();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.renderDepthValue = new System.Windows.Forms.Label();
            this.groupBox11 = new System.Windows.Forms.GroupBox();
            this.mineCenterSymbol = new System.Windows.Forms.ComboBox();
            this.showWalls = new System.Windows.Forms.CheckBox();
            this.showSpecialSegs = new System.Windows.Forms.CheckBox();
            this.showDeltaShading = new System.Windows.Forms.CheckBox();
            this.showShading = new System.Windows.Forms.CheckBox();
            this.showUsedTextures = new System.Windows.Forms.CheckBox();
            this.showSkybox = new System.Windows.Forms.CheckBox();
            this.hideMarkedSegs = new System.Windows.Forms.CheckBox();
            this.showLights = new System.Windows.Forms.CheckBox();
            this.showEffects = new System.Windows.Forms.CheckBox();
            this.showBosses = new System.Windows.Forms.CheckBox();
            this.showHostages = new System.Windows.Forms.CheckBox();
            this.showWeapons = new System.Windows.Forms.CheckBox();
            this.showKeys = new System.Windows.Forms.CheckBox();
            this.showPowerups = new System.Windows.Forms.CheckBox();
            this.showPlayers = new System.Windows.Forms.CheckBox();
            this.showRobots = new System.Windows.Forms.CheckBox();
            this.showGeoAll = new System.Windows.Forms.Button();
            this.showGeoNone = new System.Windows.Forms.Button();
            this.showObjsNone = new System.Windows.Forms.Button();
            this.showObjsAll = new System.Windows.Forms.Button();
            this.expertMode = new System.Windows.Forms.CheckBox();
            this.showSplashScreen = new System.Windows.Forms.CheckBox();
            this.maxUndoCount = new System.Windows.Forms.NumericUpDown();
            this.label7 = new System.Windows.Forms.Label();
            this.useTextureColors = new System.Windows.Forms.CheckBox();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.groupBox6.SuspendLayout();
            this.groupBox7.SuspendLayout();
            this.groupBox8.SuspendLayout();
            this.groupBox9.SuspendLayout();
            this.groupBox10.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.renderDepthSlider)).BeginInit();
            this.groupBox11.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.maxUndoCount)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.restoreFolders);
            this.groupBox1.Controls.Add(this.changeFolders);
            this.groupBox1.Controls.Add(this.browseForMissions);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.missionFolder);
            this.groupBox1.Controls.Add(this.browseForD2PIG);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.D2PIGFOLDER);
            this.groupBox1.Controls.Add(this.browseForD1PIG);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.D1PIGFolder);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(322, 132);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Folders";
            // 
            // dataBrowserDialog
            // 
            this.dataBrowserDialog.RootFolder = System.Environment.SpecialFolder.ApplicationData;
            // 
            // D1PIGFolder
            // 
            this.D1PIGFolder.Location = new System.Drawing.Point(56, 24);
            this.D1PIGFolder.Name = "D1PIGFolder";
            this.D1PIGFolder.Size = new System.Drawing.Size(230, 20);
            this.D1PIGFolder.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(8, 27);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(45, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "D1 PIG:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // browseForD1PIG
            // 
            this.browseForD1PIG.Image = global::DLEdotNET.Properties.Resources.openfile_16x16;
            this.browseForD1PIG.Location = new System.Drawing.Point(292, 22);
            this.browseForD1PIG.Name = "browseForD1PIG";
            this.browseForD1PIG.Size = new System.Drawing.Size(24, 24);
            this.browseForD1PIG.TabIndex = 1;
            this.browseForD1PIG.UseVisualStyleBackColor = true;
            // 
            // browseForD2PIG
            // 
            this.browseForD2PIG.Image = global::DLEdotNET.Properties.Resources.openfile_16x16;
            this.browseForD2PIG.Location = new System.Drawing.Point(292, 48);
            this.browseForD2PIG.Name = "browseForD2PIG";
            this.browseForD2PIG.Size = new System.Drawing.Size(24, 24);
            this.browseForD2PIG.TabIndex = 3;
            this.browseForD2PIG.UseVisualStyleBackColor = true;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(8, 53);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(45, 13);
            this.label2.TabIndex = 25;
            this.label2.Text = "D2 PIG:";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // D2PIGFOLDER
            // 
            this.D2PIGFOLDER.Location = new System.Drawing.Point(56, 50);
            this.D2PIGFOLDER.Name = "D2PIGFOLDER";
            this.D2PIGFOLDER.Size = new System.Drawing.Size(230, 20);
            this.D2PIGFOLDER.TabIndex = 2;
            // 
            // browseForMissions
            // 
            this.browseForMissions.Image = global::DLEdotNET.Properties.Resources.openfile_16x16;
            this.browseForMissions.Location = new System.Drawing.Point(292, 74);
            this.browseForMissions.Name = "browseForMissions";
            this.browseForMissions.Size = new System.Drawing.Size(24, 24);
            this.browseForMissions.TabIndex = 5;
            this.browseForMissions.UseVisualStyleBackColor = true;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(4, 79);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(50, 13);
            this.label3.TabIndex = 28;
            this.label3.Text = "Missions:";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // missionFolder
            // 
            this.missionFolder.Location = new System.Drawing.Point(56, 76);
            this.missionFolder.Name = "missionFolder";
            this.missionFolder.Size = new System.Drawing.Size(230, 20);
            this.missionFolder.TabIndex = 4;
            // 
            // changeFolders
            // 
            this.changeFolders.Location = new System.Drawing.Point(68, 102);
            this.changeFolders.Name = "changeFolders";
            this.changeFolders.Size = new System.Drawing.Size(100, 23);
            this.changeFolders.TabIndex = 6;
            this.changeFolders.Text = "Set folders";
            this.changeFolders.UseVisualStyleBackColor = true;
            // 
            // restoreFolders
            // 
            this.restoreFolders.Location = new System.Drawing.Point(174, 102);
            this.restoreFolders.Name = "restoreFolders";
            this.restoreFolders.Size = new System.Drawing.Size(100, 23);
            this.restoreFolders.TabIndex = 7;
            this.restoreFolders.Text = "Restore folders";
            this.restoreFolders.UseVisualStyleBackColor = true;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.showGeoNone);
            this.groupBox2.Controls.Add(this.showGeoAll);
            this.groupBox2.Controls.Add(this.showUsedTextures);
            this.groupBox2.Controls.Add(this.showSkybox);
            this.groupBox2.Controls.Add(this.hideMarkedSegs);
            this.groupBox2.Controls.Add(this.showLights);
            this.groupBox2.Controls.Add(this.showDeltaShading);
            this.groupBox2.Controls.Add(this.showShading);
            this.groupBox2.Controls.Add(this.showSpecialSegs);
            this.groupBox2.Controls.Add(this.showWalls);
            this.groupBox2.Location = new System.Drawing.Point(12, 150);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(160, 188);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Geometry display";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.showObjsNone);
            this.groupBox3.Controls.Add(this.showObjsAll);
            this.groupBox3.Controls.Add(this.showEffects);
            this.groupBox3.Controls.Add(this.showBosses);
            this.groupBox3.Controls.Add(this.showHostages);
            this.groupBox3.Controls.Add(this.showWeapons);
            this.groupBox3.Controls.Add(this.showKeys);
            this.groupBox3.Controls.Add(this.showPowerups);
            this.groupBox3.Controls.Add(this.showPlayers);
            this.groupBox3.Controls.Add(this.showRobots);
            this.groupBox3.Location = new System.Drawing.Point(175, 150);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(160, 188);
            this.groupBox3.TabIndex = 2;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Object display";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.depthPerceptionHigh);
            this.groupBox4.Controls.Add(this.depthPerceptionMedium);
            this.groupBox4.Controls.Add(this.depthPerceptionLow);
            this.groupBox4.Controls.Add(this.depthPerceptionOff);
            this.groupBox4.Location = new System.Drawing.Point(12, 344);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(103, 106);
            this.groupBox4.TabIndex = 3;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Depth perception";
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.rotateRate45);
            this.groupBox5.Controls.Add(this.rotateRate12);
            this.groupBox5.Controls.Add(this.rotateRate6);
            this.groupBox5.Controls.Add(this.rotateRate3);
            this.groupBox5.Controls.Add(this.rotateRate1dot5);
            this.groupBox5.Location = new System.Drawing.Point(121, 344);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(94, 106);
            this.groupBox5.TabIndex = 4;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Rotate rate";
            // 
            // groupBox6
            // 
            this.groupBox6.Controls.Add(this.label4);
            this.groupBox6.Controls.Add(this.moveRate);
            this.groupBox6.Location = new System.Drawing.Point(221, 344);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(114, 50);
            this.groupBox6.TabIndex = 5;
            this.groupBox6.TabStop = false;
            this.groupBox6.Text = "Move rate";
            // 
            // groupBox7
            // 
            this.groupBox7.Controls.Add(this.playerProfile);
            this.groupBox7.Location = new System.Drawing.Point(221, 401);
            this.groupBox7.Name = "groupBox7";
            this.groupBox7.Size = new System.Drawing.Size(114, 49);
            this.groupBox7.TabIndex = 6;
            this.groupBox7.TabStop = false;
            this.groupBox7.Text = "Player profile";
            // 
            // groupBox8
            // 
            this.groupBox8.Controls.Add(this.label7);
            this.groupBox8.Controls.Add(this.maxUndoCount);
            this.groupBox8.Controls.Add(this.showSplashScreen);
            this.groupBox8.Controls.Add(this.expertMode);
            this.groupBox8.Location = new System.Drawing.Point(13, 636);
            this.groupBox8.Name = "groupBox8";
            this.groupBox8.Size = new System.Drawing.Size(322, 92);
            this.groupBox8.TabIndex = 7;
            this.groupBox8.TabStop = false;
            this.groupBox8.Text = "Expert mode";
            // 
            // groupBox9
            // 
            this.groupBox9.Controls.Add(this.useTextureColors);
            this.groupBox9.Location = new System.Drawing.Point(13, 456);
            this.groupBox9.Name = "groupBox9";
            this.groupBox9.Size = new System.Drawing.Size(322, 40);
            this.groupBox9.TabIndex = 8;
            this.groupBox9.TabStop = false;
            this.groupBox9.Text = "Lighting";
            // 
            // groupBox10
            // 
            this.groupBox10.Controls.Add(this.renderDepthValue);
            this.groupBox10.Controls.Add(this.label6);
            this.groupBox10.Controls.Add(this.label5);
            this.groupBox10.Controls.Add(this.renderDepthSlider);
            this.groupBox10.Location = new System.Drawing.Point(13, 502);
            this.groupBox10.Name = "groupBox10";
            this.groupBox10.Size = new System.Drawing.Size(322, 74);
            this.groupBox10.TabIndex = 9;
            this.groupBox10.TabStop = false;
            this.groupBox10.Text = "Render depth";
            // 
            // depthPerceptionOff
            // 
            this.depthPerceptionOff.AutoSize = true;
            this.depthPerceptionOff.Location = new System.Drawing.Point(8, 19);
            this.depthPerceptionOff.Name = "depthPerceptionOff";
            this.depthPerceptionOff.Size = new System.Drawing.Size(39, 17);
            this.depthPerceptionOff.TabIndex = 28;
            this.depthPerceptionOff.TabStop = true;
            this.depthPerceptionOff.Text = "Off";
            this.depthPerceptionOff.UseVisualStyleBackColor = true;
            // 
            // depthPerceptionLow
            // 
            this.depthPerceptionLow.AutoSize = true;
            this.depthPerceptionLow.Location = new System.Drawing.Point(8, 39);
            this.depthPerceptionLow.Name = "depthPerceptionLow";
            this.depthPerceptionLow.Size = new System.Drawing.Size(45, 17);
            this.depthPerceptionLow.TabIndex = 29;
            this.depthPerceptionLow.TabStop = true;
            this.depthPerceptionLow.Text = "Low";
            this.depthPerceptionLow.UseVisualStyleBackColor = true;
            // 
            // depthPerceptionMedium
            // 
            this.depthPerceptionMedium.AutoSize = true;
            this.depthPerceptionMedium.Location = new System.Drawing.Point(8, 59);
            this.depthPerceptionMedium.Name = "depthPerceptionMedium";
            this.depthPerceptionMedium.Size = new System.Drawing.Size(62, 17);
            this.depthPerceptionMedium.TabIndex = 30;
            this.depthPerceptionMedium.TabStop = true;
            this.depthPerceptionMedium.Text = "Medium";
            this.depthPerceptionMedium.UseVisualStyleBackColor = true;
            // 
            // depthPerceptionHigh
            // 
            this.depthPerceptionHigh.AutoSize = true;
            this.depthPerceptionHigh.Location = new System.Drawing.Point(8, 79);
            this.depthPerceptionHigh.Name = "depthPerceptionHigh";
            this.depthPerceptionHigh.Size = new System.Drawing.Size(47, 17);
            this.depthPerceptionHigh.TabIndex = 31;
            this.depthPerceptionHigh.TabStop = true;
            this.depthPerceptionHigh.Text = "High";
            this.depthPerceptionHigh.UseVisualStyleBackColor = true;
            // 
            // moveRate
            // 
            this.moveRate.Location = new System.Drawing.Point(6, 19);
            this.moveRate.Name = "moveRate";
            this.moveRate.Size = new System.Drawing.Size(71, 20);
            this.moveRate.TabIndex = 36;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(79, 22);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(31, 13);
            this.label4.TabIndex = 1;
            this.label4.Text = "Units";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // playerProfile
            // 
            this.playerProfile.Location = new System.Drawing.Point(6, 19);
            this.playerProfile.Name = "playerProfile";
            this.playerProfile.Size = new System.Drawing.Size(101, 20);
            this.playerProfile.TabIndex = 37;
            // 
            // rotateRate12
            // 
            this.rotateRate12.AutoSize = true;
            this.rotateRate12.Location = new System.Drawing.Point(6, 64);
            this.rotateRate12.Name = "rotateRate12";
            this.rotateRate12.Size = new System.Drawing.Size(50, 17);
            this.rotateRate12.TabIndex = 35;
            this.rotateRate12.TabStop = true;
            this.rotateRate12.Text = "12.5°";
            this.rotateRate12.UseVisualStyleBackColor = true;
            // 
            // rotateRate6
            // 
            this.rotateRate6.AutoSize = true;
            this.rotateRate6.Location = new System.Drawing.Point(6, 49);
            this.rotateRate6.Name = "rotateRate6";
            this.rotateRate6.Size = new System.Drawing.Size(50, 17);
            this.rotateRate6.TabIndex = 34;
            this.rotateRate6.TabStop = true;
            this.rotateRate6.Text = "6.25°";
            this.rotateRate6.UseVisualStyleBackColor = true;
            // 
            // rotateRate3
            // 
            this.rotateRate3.AutoSize = true;
            this.rotateRate3.Location = new System.Drawing.Point(6, 34);
            this.rotateRate3.Name = "rotateRate3";
            this.rotateRate3.Size = new System.Drawing.Size(56, 17);
            this.rotateRate3.TabIndex = 33;
            this.rotateRate3.TabStop = true;
            this.rotateRate3.Text = "3.125°";
            this.rotateRate3.UseVisualStyleBackColor = true;
            // 
            // rotateRate1dot5
            // 
            this.rotateRate1dot5.AutoSize = true;
            this.rotateRate1dot5.Location = new System.Drawing.Point(6, 19);
            this.rotateRate1dot5.Name = "rotateRate1dot5";
            this.rotateRate1dot5.Size = new System.Drawing.Size(62, 17);
            this.rotateRate1dot5.TabIndex = 32;
            this.rotateRate1dot5.TabStop = true;
            this.rotateRate1dot5.Text = "1.5625°";
            this.rotateRate1dot5.UseVisualStyleBackColor = true;
            // 
            // rotateRate45
            // 
            this.rotateRate45.AutoSize = true;
            this.rotateRate45.Location = new System.Drawing.Point(6, 79);
            this.rotateRate45.Name = "rotateRate45";
            this.rotateRate45.Size = new System.Drawing.Size(41, 17);
            this.rotateRate45.TabIndex = 35;
            this.rotateRate45.TabStop = true;
            this.rotateRate45.Text = "45°";
            this.rotateRate45.UseVisualStyleBackColor = true;
            // 
            // renderDepthSlider
            // 
            this.renderDepthSlider.Location = new System.Drawing.Point(23, 19);
            this.renderDepthSlider.Maximum = 100;
            this.renderDepthSlider.Name = "renderDepthSlider";
            this.renderDepthSlider.Size = new System.Drawing.Size(272, 45);
            this.renderDepthSlider.TabIndex = 39;
            this.renderDepthSlider.TickFrequency = 5;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(23, 51);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(23, 13);
            this.label5.TabIndex = 1;
            this.label5.Text = "min";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(271, 51);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(26, 13);
            this.label6.TabIndex = 2;
            this.label6.Text = "max";
            // 
            // renderDepthValue
            // 
            this.renderDepthValue.AutoSize = true;
            this.renderDepthValue.Location = new System.Drawing.Point(298, 21);
            this.renderDepthValue.Name = "renderDepthValue";
            this.renderDepthValue.Size = new System.Drawing.Size(17, 13);
            this.renderDepthValue.TabIndex = 3;
            this.renderDepthValue.Text = "all";
            this.renderDepthValue.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // groupBox11
            // 
            this.groupBox11.Controls.Add(this.mineCenterSymbol);
            this.groupBox11.Location = new System.Drawing.Point(13, 582);
            this.groupBox11.Name = "groupBox11";
            this.groupBox11.Size = new System.Drawing.Size(322, 48);
            this.groupBox11.TabIndex = 10;
            this.groupBox11.TabStop = false;
            this.groupBox11.Text = "Mine center symbol";
            // 
            // mineCenterSymbol
            // 
            this.mineCenterSymbol.FormattingEnabled = true;
            this.mineCenterSymbol.Items.AddRange(new object[] {
            "None",
            "Cross",
            "Globe"});
            this.mineCenterSymbol.Location = new System.Drawing.Point(99, 19);
            this.mineCenterSymbol.Name = "mineCenterSymbol";
            this.mineCenterSymbol.Size = new System.Drawing.Size(128, 21);
            this.mineCenterSymbol.TabIndex = 40;
            // 
            // showWalls
            // 
            this.showWalls.AutoSize = true;
            this.showWalls.Location = new System.Drawing.Point(6, 19);
            this.showWalls.Name = "showWalls";
            this.showWalls.Size = new System.Drawing.Size(52, 17);
            this.showWalls.TabIndex = 8;
            this.showWalls.Text = "Walls";
            this.showWalls.UseVisualStyleBackColor = true;
            // 
            // showSpecialSegs
            // 
            this.showSpecialSegs.AutoSize = true;
            this.showSpecialSegs.Location = new System.Drawing.Point(6, 36);
            this.showSpecialSegs.Name = "showSpecialSegs";
            this.showSpecialSegs.Size = new System.Drawing.Size(109, 17);
            this.showSpecialSegs.TabIndex = 9;
            this.showSpecialSegs.Text = "Special segments";
            this.showSpecialSegs.UseVisualStyleBackColor = true;
            // 
            // showDeltaShading
            // 
            this.showDeltaShading.AutoSize = true;
            this.showDeltaShading.Location = new System.Drawing.Point(6, 87);
            this.showDeltaShading.Name = "showDeltaShading";
            this.showDeltaShading.Size = new System.Drawing.Size(91, 17);
            this.showDeltaShading.TabIndex = 12;
            this.showDeltaShading.Text = "Delta shading";
            this.showDeltaShading.UseVisualStyleBackColor = true;
            // 
            // showShading
            // 
            this.showShading.AutoSize = true;
            this.showShading.Location = new System.Drawing.Point(6, 70);
            this.showShading.Name = "showShading";
            this.showShading.Size = new System.Drawing.Size(65, 17);
            this.showShading.TabIndex = 11;
            this.showShading.Text = "Shading";
            this.showShading.UseVisualStyleBackColor = true;
            // 
            // showUsedTextures
            // 
            this.showUsedTextures.AutoSize = true;
            this.showUsedTextures.Location = new System.Drawing.Point(6, 138);
            this.showUsedTextures.Name = "showUsedTextures";
            this.showUsedTextures.Size = new System.Drawing.Size(91, 17);
            this.showUsedTextures.TabIndex = 15;
            this.showUsedTextures.Text = "Used textures";
            this.showUsedTextures.UseVisualStyleBackColor = true;
            // 
            // showSkybox
            // 
            this.showSkybox.AutoSize = true;
            this.showSkybox.Location = new System.Drawing.Point(6, 121);
            this.showSkybox.Name = "showSkybox";
            this.showSkybox.Size = new System.Drawing.Size(64, 17);
            this.showSkybox.TabIndex = 14;
            this.showSkybox.Text = "Sky box";
            this.showSkybox.UseVisualStyleBackColor = true;
            // 
            // hideMarkedSegs
            // 
            this.hideMarkedSegs.AutoSize = true;
            this.hideMarkedSegs.Location = new System.Drawing.Point(6, 104);
            this.hideMarkedSegs.Name = "hideMarkedSegs";
            this.hideMarkedSegs.Size = new System.Drawing.Size(134, 17);
            this.hideMarkedSegs.TabIndex = 13;
            this.hideMarkedSegs.Text = "Hide marked segments";
            this.hideMarkedSegs.UseVisualStyleBackColor = true;
            // 
            // showLights
            // 
            this.showLights.AutoSize = true;
            this.showLights.Location = new System.Drawing.Point(6, 53);
            this.showLights.Name = "showLights";
            this.showLights.Size = new System.Drawing.Size(54, 17);
            this.showLights.TabIndex = 10;
            this.showLights.Text = "Lights";
            this.showLights.UseVisualStyleBackColor = true;
            // 
            // showEffects
            // 
            this.showEffects.AutoSize = true;
            this.showEffects.Location = new System.Drawing.Point(6, 138);
            this.showEffects.Name = "showEffects";
            this.showEffects.Size = new System.Drawing.Size(59, 17);
            this.showEffects.TabIndex = 25;
            this.showEffects.Text = "Effects";
            this.showEffects.UseVisualStyleBackColor = true;
            // 
            // showBosses
            // 
            this.showBosses.AutoSize = true;
            this.showBosses.Location = new System.Drawing.Point(6, 121);
            this.showBosses.Name = "showBosses";
            this.showBosses.Size = new System.Drawing.Size(108, 17);
            this.showBosses.TabIndex = 24;
            this.showBosses.Text = "Reactor / bosses";
            this.showBosses.UseVisualStyleBackColor = true;
            // 
            // showHostages
            // 
            this.showHostages.AutoSize = true;
            this.showHostages.Location = new System.Drawing.Point(6, 104);
            this.showHostages.Name = "showHostages";
            this.showHostages.Size = new System.Drawing.Size(71, 17);
            this.showHostages.TabIndex = 23;
            this.showHostages.Text = "Hostages";
            this.showHostages.UseVisualStyleBackColor = true;
            // 
            // showWeapons
            // 
            this.showWeapons.AutoSize = true;
            this.showWeapons.Location = new System.Drawing.Point(6, 53);
            this.showWeapons.Name = "showWeapons";
            this.showWeapons.Size = new System.Drawing.Size(72, 17);
            this.showWeapons.TabIndex = 20;
            this.showWeapons.Text = "Weapons";
            this.showWeapons.UseVisualStyleBackColor = true;
            // 
            // showKeys
            // 
            this.showKeys.AutoSize = true;
            this.showKeys.Location = new System.Drawing.Point(6, 87);
            this.showKeys.Name = "showKeys";
            this.showKeys.Size = new System.Drawing.Size(49, 17);
            this.showKeys.TabIndex = 22;
            this.showKeys.Text = "Keys";
            this.showKeys.UseVisualStyleBackColor = true;
            // 
            // showPowerups
            // 
            this.showPowerups.AutoSize = true;
            this.showPowerups.Location = new System.Drawing.Point(6, 70);
            this.showPowerups.Name = "showPowerups";
            this.showPowerups.Size = new System.Drawing.Size(73, 17);
            this.showPowerups.TabIndex = 21;
            this.showPowerups.Text = "Powerups";
            this.showPowerups.UseVisualStyleBackColor = true;
            // 
            // showPlayers
            // 
            this.showPlayers.AutoSize = true;
            this.showPlayers.Location = new System.Drawing.Point(6, 36);
            this.showPlayers.Name = "showPlayers";
            this.showPlayers.Size = new System.Drawing.Size(60, 17);
            this.showPlayers.TabIndex = 19;
            this.showPlayers.Text = "Players";
            this.showPlayers.UseVisualStyleBackColor = true;
            // 
            // showRobots
            // 
            this.showRobots.AutoSize = true;
            this.showRobots.Location = new System.Drawing.Point(6, 19);
            this.showRobots.Name = "showRobots";
            this.showRobots.Size = new System.Drawing.Size(60, 17);
            this.showRobots.TabIndex = 18;
            this.showRobots.Text = "Robots";
            this.showRobots.UseVisualStyleBackColor = true;
            // 
            // showGeoAll
            // 
            this.showGeoAll.Location = new System.Drawing.Point(24, 160);
            this.showGeoAll.Name = "showGeoAll";
            this.showGeoAll.Size = new System.Drawing.Size(65, 22);
            this.showGeoAll.TabIndex = 16;
            this.showGeoAll.Text = "All";
            this.showGeoAll.UseVisualStyleBackColor = true;
            // 
            // showGeoNone
            // 
            this.showGeoNone.Location = new System.Drawing.Point(89, 160);
            this.showGeoNone.Name = "showGeoNone";
            this.showGeoNone.Size = new System.Drawing.Size(66, 22);
            this.showGeoNone.TabIndex = 17;
            this.showGeoNone.Text = "None";
            this.showGeoNone.UseVisualStyleBackColor = true;
            // 
            // showObjsNone
            // 
            this.showObjsNone.Location = new System.Drawing.Point(89, 160);
            this.showObjsNone.Name = "showObjsNone";
            this.showObjsNone.Size = new System.Drawing.Size(66, 22);
            this.showObjsNone.TabIndex = 27;
            this.showObjsNone.Text = "None";
            this.showObjsNone.UseVisualStyleBackColor = true;
            // 
            // showObjsAll
            // 
            this.showObjsAll.Location = new System.Drawing.Point(24, 160);
            this.showObjsAll.Name = "showObjsAll";
            this.showObjsAll.Size = new System.Drawing.Size(65, 22);
            this.showObjsAll.TabIndex = 26;
            this.showObjsAll.Text = "All";
            this.showObjsAll.UseVisualStyleBackColor = true;
            // 
            // expertMode
            // 
            this.expertMode.AutoSize = true;
            this.expertMode.Location = new System.Drawing.Point(53, 19);
            this.expertMode.Name = "expertMode";
            this.expertMode.Size = new System.Drawing.Size(198, 17);
            this.expertMode.TabIndex = 41;
            this.expertMode.Text = "Stop asking questions all the time! ;-)";
            this.expertMode.UseVisualStyleBackColor = true;
            // 
            // showSplashScreen
            // 
            this.showSplashScreen.AutoSize = true;
            this.showSplashScreen.Location = new System.Drawing.Point(53, 42);
            this.showSplashScreen.Name = "showSplashScreen";
            this.showSplashScreen.Size = new System.Drawing.Size(197, 17);
            this.showSplashScreen.TabIndex = 42;
            this.showSplashScreen.Text = "Show splash screen at program start";
            this.showSplashScreen.UseVisualStyleBackColor = true;
            // 
            // maxUndoCount
            // 
            this.maxUndoCount.Location = new System.Drawing.Point(19, 65);
            this.maxUndoCount.Name = "maxUndoCount";
            this.maxUndoCount.Size = new System.Drawing.Size(47, 20);
            this.maxUndoCount.TabIndex = 43;
            this.maxUndoCount.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.maxUndoCount.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(72, 67);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(104, 13);
            this.label7.TabIndex = 3;
            this.label7.Text = "Max. undos (0 - 100)";
            // 
            // useTextureColors
            // 
            this.useTextureColors.AutoSize = true;
            this.useTextureColors.Location = new System.Drawing.Point(53, 19);
            this.useTextureColors.Name = "useTextureColors";
            this.useTextureColors.Size = new System.Drawing.Size(111, 17);
            this.useTextureColors.TabIndex = 38;
            this.useTextureColors.Text = "Use texture colors";
            this.useTextureColors.UseVisualStyleBackColor = true;
            // 
            // PreferencesForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.GradientActiveCaption;
            this.ClientSize = new System.Drawing.Size(344, 812);
            this.Controls.Add(this.groupBox11);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.groupBox10);
            this.Controls.Add(this.groupBox9);
            this.Controls.Add(this.groupBox8);
            this.Controls.Add(this.groupBox7);
            this.Controls.Add(this.groupBox6);
            this.Controls.Add(this.groupBox5);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "PreferencesForm";
            this.Text = "Settings";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.groupBox6.ResumeLayout(false);
            this.groupBox6.PerformLayout();
            this.groupBox7.ResumeLayout(false);
            this.groupBox7.PerformLayout();
            this.groupBox8.ResumeLayout(false);
            this.groupBox8.PerformLayout();
            this.groupBox9.ResumeLayout(false);
            this.groupBox9.PerformLayout();
            this.groupBox10.ResumeLayout(false);
            this.groupBox10.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.renderDepthSlider)).EndInit();
            this.groupBox11.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.maxUndoCount)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.FolderBrowserDialog dataBrowserDialog;
        private System.Windows.Forms.TextBox D1PIGFolder;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button browseForD1PIG;
        private System.Windows.Forms.Button browseForMissions;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox missionFolder;
        private System.Windows.Forms.Button browseForD2PIG;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox D2PIGFOLDER;
        private System.Windows.Forms.Button restoreFolders;
        private System.Windows.Forms.Button changeFolders;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.GroupBox groupBox6;
        private System.Windows.Forms.GroupBox groupBox7;
        private System.Windows.Forms.GroupBox groupBox8;
        private System.Windows.Forms.RadioButton depthPerceptionHigh;
        private System.Windows.Forms.RadioButton depthPerceptionMedium;
        private System.Windows.Forms.RadioButton depthPerceptionLow;
        private System.Windows.Forms.RadioButton depthPerceptionOff;
        private System.Windows.Forms.GroupBox groupBox9;
        private System.Windows.Forms.GroupBox groupBox10;
        private System.Windows.Forms.RadioButton rotateRate45;
        private System.Windows.Forms.RadioButton rotateRate12;
        private System.Windows.Forms.RadioButton rotateRate6;
        private System.Windows.Forms.RadioButton rotateRate3;
        private System.Windows.Forms.RadioButton rotateRate1dot5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox moveRate;
        private System.Windows.Forms.TextBox playerProfile;
        private System.Windows.Forms.Label renderDepthValue;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TrackBar renderDepthSlider;
        private System.Windows.Forms.GroupBox groupBox11;
        private System.Windows.Forms.ComboBox mineCenterSymbol;
        private System.Windows.Forms.CheckBox showUsedTextures;
        private System.Windows.Forms.CheckBox showSkybox;
        private System.Windows.Forms.CheckBox hideMarkedSegs;
        private System.Windows.Forms.CheckBox showLights;
        private System.Windows.Forms.CheckBox showDeltaShading;
        private System.Windows.Forms.CheckBox showShading;
        private System.Windows.Forms.CheckBox showSpecialSegs;
        private System.Windows.Forms.CheckBox showWalls;
        private System.Windows.Forms.CheckBox showEffects;
        private System.Windows.Forms.CheckBox showBosses;
        private System.Windows.Forms.CheckBox showHostages;
        private System.Windows.Forms.CheckBox showWeapons;
        private System.Windows.Forms.CheckBox showKeys;
        private System.Windows.Forms.CheckBox showPowerups;
        private System.Windows.Forms.CheckBox showPlayers;
        private System.Windows.Forms.CheckBox showRobots;
        private System.Windows.Forms.Button showGeoNone;
        private System.Windows.Forms.Button showGeoAll;
        private System.Windows.Forms.Button showObjsNone;
        private System.Windows.Forms.Button showObjsAll;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.NumericUpDown maxUndoCount;
        private System.Windows.Forms.CheckBox showSplashScreen;
        private System.Windows.Forms.CheckBox expertMode;
        private System.Windows.Forms.CheckBox useTextureColors;
    }
}