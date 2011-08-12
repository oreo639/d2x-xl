
namespace DLEdotNET
{
    public partial class LightTool : ToolWindow
    {
        bool m_bAnimate;
        int m_frameRate;

        public LightTool()
        {
            InitializeComponent();
            m_bAnimate = false;
            m_frameRate = 10;
            lightTimer.Enabled = false;
        }

        private void UpdateTimer()
        {
            lightTimer.Interval = 1000 / m_frameRate;
        }

        private void animateDynLight_Click(object sender, System.EventArgs e)
        {
            if ((m_bAnimate = !m_bAnimate))
            {
                lightTimer.Start();
                UpdateTimer();
            }
            else
                lightTimer.Stop();
        }

        private void dynamicFrameRate_Scroll(object sender, System.EventArgs e)
        {
            m_frameRate = dynamicFrameRate.Value;
            UpdateTimer();
        }

        private void dynLightTimer_Tick(object sender, System.EventArgs e)
        {

        }
    }
}
