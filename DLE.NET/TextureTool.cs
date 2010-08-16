using System.Drawing;
using System.Windows.Forms;
using System;

namespace DLE.NET
{
    public partial class TextureTool : ToolWindow
    {
        private bool m_bAnimate;
        private bool m_bLight;
        private int m_speed;
        private int m_index;

        private RadioButton[] m_lightTicks = new RadioButton[32];

        public TextureTool()
        {
            InitializeComponent();
            Setup();

            m_bAnimate = false;
            m_speed = 250;
        }

        void Setup()
        {
            // arrange the light tick panel buttons on a circle
            Point minPos = new Point(30000, 30000);
            Point maxPos = new Point(-30000, -30000);

            // first get the bounding box (requires the radio buttons 1,9,17 and 25 (those on the X and Y axis) 
            // to be at their exact position)
            foreach (Control control in effectDisplayGroup.Controls)
            {
                if ((control is Panel) && (control.Name.Length > 9) && (control.Name.Substring(0, 9) == "tickPanel"))
                {
                    if (minPos.X > control.Location.X)
                        minPos.X = control.Location.X;
                    if (maxPos.X < control.Location.X)
                        maxPos.X = control.Location.X;
                    if (minPos.Y > control.Location.Y)
                        minPos.Y = control.Location.Y;
                    if (maxPos.Y < control.Location.Y)
                        maxPos.Y = control.Location.Y;
                }
            }

            // now place the radio buttons on a circle around the center of the bounding box
            float xRad = (float) (maxPos.X - minPos.X + 1) / 2.0f;
            float yRad = (float) (maxPos.Y - minPos.Y + 1) / 2.0f;
            float scale = 2.0f * (float) System.Math.PI / 32.0f;
            Point center = new Point((minPos.X + maxPos.X) / 2, (minPos.Y + maxPos.Y) / 2);

            foreach (Control control in effectDisplayGroup.Controls)
            {
                if ((control is Panel) && (control.Name.Length > 9) && (control.Name.Substring(0, 9) == "tickPanel"))
                {
                    m_lightTicks[control.TabIndex] = control.Controls [0] as RadioButton;
                    Point pos = center;
                    pos.X = center.X + (int)(xRad * (float)System.Math.Cos((float)control.TabIndex * scale));
                    pos.Y = center.Y + (int)(yRad * (float)System.Math.Sin((float)control.TabIndex * scale));
                    control.Location = pos;
                }
            }
        }
        
        // Animate the light display
        private void UpdateTimer()
        {
            float f = (float)m_speed / 1000.0f;
            tickTime.Text = Convert.ToString(f);
            secsPerTick.Value = Math.Max ((int) (secsPerTick.Maximum * f), 1);
            lightTimer.Interval = m_speed;
        }

        private void secsPerTick_Scroll(object sender, System.EventArgs e)
        {
            m_speed = secsPerTick.Value * 1000 / secsPerTick.Maximum;
            UpdateTimer();
        }

        private void tickTime_TextChanged(object sender, System.EventArgs e)
        {
            float f = Convert.ToSingle (tickTime.Text);
            if (f < 0.01f)
                f = 0.01f;
            else if (f > 1.0f)
                f = 1.0f;
            m_speed = (int)(f * 1000);
            tickTime.Text = Convert.ToString(f);
        }

        private void tickTime_Leave(object sender, System.EventArgs e)
        {
            float f = Convert.ToSingle(tickTime.Text);
            if (f < 0.01f)
                f = 0.01f;
            else if (f > 1.0f)
                f = 1.0f;
            m_speed = (int)(f * 1000);
            UpdateTimer();
        }

        private void lightTimer_Tick(object sender, System.EventArgs e)
        {
            m_lightTicks[m_index].Checked = m_bLight;
            m_index = (m_index + 1) % 32;
            if (m_bLight = m_lightTicks[m_index].Checked)
                tickDisplay.BackColor = Color.Gold;
            else
                tickDisplay.BackColor = Color.FromName("GradientActiveCaption");
            m_lightTicks[m_index].Checked = true;
        }

        private void addDynLight_Click(object sender, System.EventArgs e)
        {
            if (!m_bAnimate)
            {
                m_bAnimate = true;
                m_index = 0;
                m_bLight = m_lightTicks[m_index].Checked;
                UpdateTimer();
                lightTimer.Start();
            }
        }

        private void delDynLight_Click(object sender, System.EventArgs e)
        {
            if (m_bAnimate)
            {
                m_bAnimate = false;
                m_lightTicks[m_index].Checked = m_bLight;
                tickDisplay.BackColor = Color.FromName("GradientActiveCaption");
                lightTimer.Stop();
            }
        }
    }
}
