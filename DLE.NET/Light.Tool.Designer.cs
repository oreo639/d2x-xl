namespace DLEdotNET
{
    partial class LightTool
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(LightTool));
            this.lightTimer = new System.Windows.Forms.Timer(this.components);
            this.button1 = new System.Windows.Forms.Button();
            this.defaultButton = new System.Windows.Forms.Button();
            this.applyButton = new System.Windows.Forms.Button();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.setVertexLight = new System.Windows.Forms.Button();
            this.label14 = new System.Windows.Forms.Label();
            this.vertexLight = new System.Windows.Forms.NumericUpDown();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.label13 = new System.Windows.Forms.Label();
            this.dynLightExceptLava = new System.Windows.Forms.RadioButton();
            this.dynLightExceptStatic = new System.Windows.Forms.RadioButton();
            this.dynLightAllTextures = new System.Windows.Forms.RadioButton();
            this.showOnlyLights = new System.Windows.Forms.CheckBox();
            this.animateDynLight = new System.Windows.Forms.Button();
            this.label12 = new System.Windows.Forms.Label();
            this.dynLightValue = new System.Windows.Forms.NumericUpDown();
            this.dynLightBox = new System.Windows.Forms.CheckBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.dynamicFrameRate = new System.Windows.Forms.TrackBar();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.dynamicRenderDepth = new System.Windows.Forms.TrackBar();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label11 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.createSideLightsBox = new System.Windows.Forms.CheckBox();
            this.dynSegLightLightBox = new System.Windows.Forms.CheckBox();
            this.avgCornerLightBox = new System.Windows.Forms.CheckBox();
            this.scaleLightValue = new System.Windows.Forms.NumericUpDown();
            this.illuminateValue = new System.Windows.Forms.NumericUpDown();
            this.scaleLightBox = new System.Windows.Forms.CheckBox();
            this.illuminateBox = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.staticRenderDepth = new System.Windows.Forms.TrackBar();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.vertexLight)).BeginInit();
            this.groupBox2.SuspendLayout();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dynLightValue)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dynamicFrameRate)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dynamicRenderDepth)).BeginInit();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.scaleLightValue)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.illuminateValue)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.staticRenderDepth)).BeginInit();
            this.SuspendLayout();
            // 
            // lightTimer
            // 
            this.lightTimer.Tick += new System.EventHandler(this.dynLightTimer_Tick);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(86, 640);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(167, 24);
            this.button1.TabIndex = 21;
            this.button1.Text = "Reset brightness and color";
            this.button1.UseVisualStyleBackColor = true;
            // 
            // defaultButton
            // 
            this.defaultButton.Location = new System.Drawing.Point(173, 616);
            this.defaultButton.Name = "defaultButton";
            this.defaultButton.Size = new System.Drawing.Size(80, 24);
            this.defaultButton.TabIndex = 20;
            this.defaultButton.Text = "Default";
            this.defaultButton.UseVisualStyleBackColor = true;
            // 
            // applyButton
            // 
            this.applyButton.Location = new System.Drawing.Point(86, 616);
            this.applyButton.Name = "applyButton";
            this.applyButton.Size = new System.Drawing.Size(80, 24);
            this.applyButton.TabIndex = 19;
            this.applyButton.Text = "Apply";
            this.applyButton.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.setVertexLight);
            this.groupBox3.Controls.Add(this.label14);
            this.groupBox3.Controls.Add(this.vertexLight);
            this.groupBox3.Location = new System.Drawing.Point(12, 550);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(320, 54);
            this.groupBox3.TabIndex = 5;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Vertex light";
            // 
            // setVertexLight
            // 
            this.setVertexLight.Location = new System.Drawing.Point(176, 19);
            this.setVertexLight.Name = "setVertexLight";
            this.setVertexLight.Size = new System.Drawing.Size(60, 20);
            this.setVertexLight.TabIndex = 18;
            this.setVertexLight.Text = "Set";
            this.setVertexLight.UseVisualStyleBackColor = true;
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(138, 23);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(15, 13);
            this.label14.TabIndex = 13;
            this.label14.Text = "%";
            // 
            // vertexLight
            // 
            this.vertexLight.Location = new System.Drawing.Point(76, 19);
            this.vertexLight.Name = "vertexLight";
            this.vertexLight.Size = new System.Drawing.Size(60, 20);
            this.vertexLight.TabIndex = 17;
            this.vertexLight.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.vertexLight.Value = new decimal(new int[] {
            50,
            0,
            0,
            0});
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.panel1);
            this.groupBox2.Controls.Add(this.showOnlyLights);
            this.groupBox2.Controls.Add(this.animateDynLight);
            this.groupBox2.Controls.Add(this.label12);
            this.groupBox2.Controls.Add(this.dynLightValue);
            this.groupBox2.Controls.Add(this.dynLightBox);
            this.groupBox2.Controls.Add(this.label7);
            this.groupBox2.Controls.Add(this.label8);
            this.groupBox2.Controls.Add(this.label9);
            this.groupBox2.Controls.Add(this.dynamicFrameRate);
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.dynamicRenderDepth);
            this.groupBox2.Location = new System.Drawing.Point(12, 236);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(320, 308);
            this.groupBox2.TabIndex = 4;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Exploding && blinking lights";
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.label13);
            this.panel1.Controls.Add(this.dynLightExceptLava);
            this.panel1.Controls.Add(this.dynLightExceptStatic);
            this.panel1.Controls.Add(this.dynLightAllTextures);
            this.panel1.Location = new System.Drawing.Point(74, 64);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(162, 82);
            this.panel1.TabIndex = 19;
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(4, 30);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(36, 13);
            this.label13.TabIndex = 16;
            this.label13.Text = "Apply:";
            this.label13.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // dynLightExceptLava
            // 
            this.dynLightExceptLava.AutoSize = true;
            this.dynLightExceptLava.Location = new System.Drawing.Point(46, 51);
            this.dynLightExceptLava.Name = "dynLightExceptLava";
            this.dynLightExceptLava.Size = new System.Drawing.Size(103, 17);
            this.dynLightExceptLava.TabIndex = 12;
            this.dynLightExceptLava.TabStop = true;
            this.dynLightExceptLava.Text = "not to static lava";
            this.dynLightExceptLava.UseVisualStyleBackColor = true;
            // 
            // dynLightExceptStatic
            // 
            this.dynLightExceptStatic.AutoSize = true;
            this.dynLightExceptStatic.Location = new System.Drawing.Point(46, 28);
            this.dynLightExceptStatic.Name = "dynLightExceptStatic";
            this.dynLightExceptStatic.Size = new System.Drawing.Size(107, 17);
            this.dynLightExceptStatic.TabIndex = 11;
            this.dynLightExceptStatic.TabStop = true;
            this.dynLightExceptStatic.Text = "not to static lights";
            this.dynLightExceptStatic.UseVisualStyleBackColor = true;
            // 
            // dynLightAllTextures
            // 
            this.dynLightAllTextures.AutoSize = true;
            this.dynLightAllTextures.Location = new System.Drawing.Point(46, 5);
            this.dynLightAllTextures.Name = "dynLightAllTextures";
            this.dynLightAllTextures.Size = new System.Drawing.Size(87, 17);
            this.dynLightAllTextures.TabIndex = 10;
            this.dynLightAllTextures.TabStop = true;
            this.dynLightAllTextures.Text = "to all textures";
            this.dynLightAllTextures.UseVisualStyleBackColor = true;
            // 
            // showOnlyLights
            // 
            this.showOnlyLights.AutoSize = true;
            this.showOnlyLights.Location = new System.Drawing.Point(120, 275);
            this.showOnlyLights.Name = "showOnlyLights";
            this.showOnlyLights.Size = new System.Drawing.Size(137, 17);
            this.showOnlyLights.TabIndex = 16;
            this.showOnlyLights.Text = "Only show light sources";
            this.showOnlyLights.UseVisualStyleBackColor = true;
            // 
            // animateDynLight
            // 
            this.animateDynLight.Location = new System.Drawing.Point(39, 271);
            this.animateDynLight.Name = "animateDynLight";
            this.animateDynLight.Size = new System.Drawing.Size(75, 23);
            this.animateDynLight.TabIndex = 15;
            this.animateDynLight.Text = "Animate";
            this.animateDynLight.UseVisualStyleBackColor = true;
            this.animateDynLight.Click += new System.EventHandler(this.animateDynLight_Click);
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(270, 33);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(15, 13);
            this.label12.TabIndex = 12;
            this.label12.Text = "%";
            // 
            // dynLightValue
            // 
            this.dynLightValue.Location = new System.Drawing.Point(214, 29);
            this.dynLightValue.Name = "dynLightValue";
            this.dynLightValue.Size = new System.Drawing.Size(54, 20);
            this.dynLightValue.TabIndex = 9;
            this.dynLightValue.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.dynLightValue.Value = new decimal(new int[] {
            50,
            0,
            0,
            0});
            // 
            // dynLightBox
            // 
            this.dynLightBox.AutoSize = true;
            this.dynLightBox.Location = new System.Drawing.Point(24, 32);
            this.dynLightBox.Name = "dynLightBox";
            this.dynLightBox.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.dynLightBox.Size = new System.Drawing.Size(184, 17);
            this.dynLightBox.TabIndex = 8;
            this.dynLightBox.Text = "Compute exploding/blinking lights";
            this.dynLightBox.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.dynLightBox.UseVisualStyleBackColor = true;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(253, 238);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(25, 13);
            this.label7.TabIndex = 7;
            this.label7.Text = "100";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(117, 238);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(19, 13);
            this.label8.TabIndex = 6;
            this.label8.Text = "10";
            this.label8.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.BackColor = System.Drawing.Color.Transparent;
            this.label9.Location = new System.Drawing.Point(47, 213);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(60, 13);
            this.label9.TabIndex = 5;
            this.label9.Text = "Frame rate:";
            this.label9.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // dynamicFrameRate
            // 
            this.dynamicFrameRate.AutoSize = false;
            this.dynamicFrameRate.LargeChange = 10;
            this.dynamicFrameRate.Location = new System.Drawing.Point(113, 211);
            this.dynamicFrameRate.Maximum = 100;
            this.dynamicFrameRate.Minimum = 10;
            this.dynamicFrameRate.Name = "dynamicFrameRate";
            this.dynamicFrameRate.Size = new System.Drawing.Size(166, 31);
            this.dynamicFrameRate.TabIndex = 14;
            this.dynamicFrameRate.TickFrequency = 10;
            this.dynamicFrameRate.Value = 100;
            this.dynamicFrameRate.Scroll += new System.EventHandler(this.dynamicFrameRate_Scroll);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(257, 189);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(19, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "10";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(121, 189);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(13, 13);
            this.label5.TabIndex = 2;
            this.label5.Text = "1";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(32, 164);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(75, 13);
            this.label6.TabIndex = 1;
            this.label6.Text = "Render depth:";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // dynamicRenderDepth
            // 
            this.dynamicRenderDepth.AutoSize = false;
            this.dynamicRenderDepth.Location = new System.Drawing.Point(113, 162);
            this.dynamicRenderDepth.Minimum = 1;
            this.dynamicRenderDepth.Name = "dynamicRenderDepth";
            this.dynamicRenderDepth.Size = new System.Drawing.Size(166, 31);
            this.dynamicRenderDepth.TabIndex = 13;
            this.dynamicRenderDepth.Value = 1;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label11);
            this.groupBox1.Controls.Add(this.label10);
            this.groupBox1.Controls.Add(this.createSideLightsBox);
            this.groupBox1.Controls.Add(this.dynSegLightLightBox);
            this.groupBox1.Controls.Add(this.avgCornerLightBox);
            this.groupBox1.Controls.Add(this.scaleLightValue);
            this.groupBox1.Controls.Add(this.illuminateValue);
            this.groupBox1.Controls.Add(this.scaleLightBox);
            this.groupBox1.Controls.Add(this.illuminateBox);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.staticRenderDepth);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(320, 218);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Static illumination";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(271, 54);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(15, 13);
            this.label11.TabIndex = 12;
            this.label11.Text = "%";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(271, 30);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(15, 13);
            this.label10.TabIndex = 11;
            this.label10.Text = "%";
            // 
            // createSideLightsBox
            // 
            this.createSideLightsBox.AutoSize = true;
            this.createSideLightsBox.Location = new System.Drawing.Point(39, 132);
            this.createSideLightsBox.Name = "createSideLightsBox";
            this.createSideLightsBox.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.createSideLightsBox.Size = new System.Drawing.Size(169, 17);
            this.createSideLightsBox.TabIndex = 6;
            this.createSideLightsBox.Text = "Create side lights from textures";
            this.createSideLightsBox.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.createSideLightsBox.UseVisualStyleBackColor = true;
            // 
            // dynSegLightLightBox
            // 
            this.dynSegLightLightBox.AutoSize = true;
            this.dynSegLightLightBox.Location = new System.Drawing.Point(33, 109);
            this.dynSegLightLightBox.Name = "dynSegLightLightBox";
            this.dynSegLightLightBox.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.dynSegLightLightBox.Size = new System.Drawing.Size(175, 17);
            this.dynSegLightLightBox.TabIndex = 5;
            this.dynSegLightLightBox.Text = "Compute dynamic segment light";
            this.dynSegLightLightBox.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.dynSegLightLightBox.UseVisualStyleBackColor = true;
            // 
            // avgCornerLightBox
            // 
            this.avgCornerLightBox.AutoSize = true;
            this.avgCornerLightBox.Location = new System.Drawing.Point(43, 86);
            this.avgCornerLightBox.Name = "avgCornerLightBox";
            this.avgCornerLightBox.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.avgCornerLightBox.Size = new System.Drawing.Size(165, 17);
            this.avgCornerLightBox.TabIndex = 4;
            this.avgCornerLightBox.Text = "Compute average corner light";
            this.avgCornerLightBox.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.avgCornerLightBox.UseVisualStyleBackColor = true;
            // 
            // scaleLightValue
            // 
            this.scaleLightValue.Location = new System.Drawing.Point(214, 51);
            this.scaleLightValue.Name = "scaleLightValue";
            this.scaleLightValue.Size = new System.Drawing.Size(54, 20);
            this.scaleLightValue.TabIndex = 3;
            this.scaleLightValue.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.scaleLightValue.Value = new decimal(new int[] {
            50,
            0,
            0,
            0});
            // 
            // illuminateValue
            // 
            this.illuminateValue.Location = new System.Drawing.Point(214, 26);
            this.illuminateValue.Name = "illuminateValue";
            this.illuminateValue.Size = new System.Drawing.Size(54, 20);
            this.illuminateValue.TabIndex = 1;
            this.illuminateValue.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.illuminateValue.Value = new decimal(new int[] {
            50,
            0,
            0,
            0});
            // 
            // scaleLightBox
            // 
            this.scaleLightBox.AutoSize = true;
            this.scaleLightBox.Location = new System.Drawing.Point(69, 52);
            this.scaleLightBox.Name = "scaleLightBox";
            this.scaleLightBox.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.scaleLightBox.Size = new System.Drawing.Size(139, 17);
            this.scaleLightBox.TabIndex = 2;
            this.scaleLightBox.Text = "(Scale light (0% to 200%";
            this.scaleLightBox.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.scaleLightBox.UseVisualStyleBackColor = true;
            // 
            // illuminateBox
            // 
            this.illuminateBox.AutoSize = true;
            this.illuminateBox.Location = new System.Drawing.Point(74, 29);
            this.illuminateBox.Name = "illuminateBox";
            this.illuminateBox.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.illuminateBox.Size = new System.Drawing.Size(134, 17);
            this.illuminateBox.TabIndex = 0;
            this.illuminateBox.Text = "(Illuminate (0% to 200%";
            this.illuminateBox.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.illuminateBox.UseVisualStyleBackColor = true;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(246, 193);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(19, 13);
            this.label3.TabIndex = 3;
            this.label3.Text = "10";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(110, 193);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(13, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "1";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(31, 168);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(75, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Render depth:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // staticRenderDepth
            // 
            this.staticRenderDepth.AutoSize = false;
            this.staticRenderDepth.Location = new System.Drawing.Point(102, 166);
            this.staticRenderDepth.Minimum = 1;
            this.staticRenderDepth.Name = "staticRenderDepth";
            this.staticRenderDepth.Size = new System.Drawing.Size(166, 31);
            this.staticRenderDepth.TabIndex = 7;
            this.staticRenderDepth.Value = 1;
            // 
            // LightTool
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.GradientActiveCaption;
            this.ClientSize = new System.Drawing.Size(344, 842);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.defaultButton);
            this.Controls.Add(this.applyButton);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "LightTool";
            this.Opacity = 0.5D;
            this.Text = "Lights";
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.vertexLight)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dynLightValue)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dynamicFrameRate)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dynamicRenderDepth)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.scaleLightValue)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.illuminateValue)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.staticRenderDepth)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TrackBar staticRenderDepth;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TrackBar dynamicRenderDepth;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TrackBar dynamicFrameRate;
        private System.Windows.Forms.CheckBox scaleLightBox;
        private System.Windows.Forms.CheckBox illuminateBox;
        private System.Windows.Forms.NumericUpDown illuminateValue;
        private System.Windows.Forms.NumericUpDown scaleLightValue;
        private System.Windows.Forms.CheckBox dynSegLightLightBox;
        private System.Windows.Forms.CheckBox avgCornerLightBox;
        private System.Windows.Forms.CheckBox createSideLightsBox;
        private System.Windows.Forms.CheckBox dynLightBox;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.NumericUpDown dynLightValue;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.RadioButton dynLightExceptLava;
        private System.Windows.Forms.RadioButton dynLightExceptStatic;
        private System.Windows.Forms.RadioButton dynLightAllTextures;
        private System.Windows.Forms.CheckBox showOnlyLights;
        private System.Windows.Forms.Button animateDynLight;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Timer lightTimer;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Button setVertexLight;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.NumericUpDown vertexLight;
        private System.Windows.Forms.Button applyButton;
        private System.Windows.Forms.Button defaultButton;
        private System.Windows.Forms.Button button1;
    }
}