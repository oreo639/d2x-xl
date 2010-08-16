using System;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;
using System.IO;
using System.Drawing;

namespace DLE.NET
{
    public partial class MainWindow : Form
    {
        private bool m_bSaveLayout = false;
        private DeserializeDockContent m_deserializeDockContent;
        private SegmentTool m_segmentTool;
        private LightTool m_lightTool;
        private TextureTool m_textureTool;
        private TriggerTool m_triggerTool;
        private RenderWindow m_renderWindow;
        private TextureWindow m_textureList;

        const float toolPaneWidth = 360.0f;
        const float texturePaneHeight = 200.0f;

        public MainWindow()
        {
            InitializeComponent();
            m_renderWindow = new RenderWindow();
            m_renderWindow.RightToLeftLayout = false;
            m_textureList = new TextureWindow();
            m_textureList.RightToLeftLayout = false;
            m_textureList.Text = "Textures";
            m_segmentTool = new SegmentTool();
            m_segmentTool.RightToLeftLayout = false;
            m_textureTool = new TextureTool();
            m_textureTool.RightToLeftLayout = false;
            m_triggerTool = new TriggerTool();
            m_triggerTool.RightToLeftLayout = false;
            m_lightTool = new LightTool();
            m_lightTool.RightToLeftLayout = false;
            if (!m_bSaveLayout)
                Setup();
            //dockPanel.DockRightPortion = (float)m_segmentTool.Width / (float)dockPanel.Width;
            dockPanel.DockLeftPortion = toolPaneWidth / ((float)dockPanel.Width - 16.0f);
            dockPanel.DockRightPortion = toolPaneWidth / ((float)dockPanel.Width - 16.0f);
            dockPanel.DockBottomPortion = texturePaneHeight / ((float)dockPanel.Height - 16.0f);
            m_deserializeDockContent = new DeserializeDockContent(GetContentFromPersistString);
        }

        private void Setup()
        {
            m_renderWindow.Show(dockPanel, DockState.Document);
            m_segmentTool.Show(dockPanel, DockState.DockLeft);
            m_textureTool.Show(m_segmentTool.Pane, null);
            m_triggerTool.Show(m_segmentTool.Pane, null);
            m_lightTool.Show(m_segmentTool.Pane, null);
            m_textureList.Show(m_renderWindow.Pane, DockAlignment.Bottom, 200.0f / (float) m_renderWindow.Height);
            //m_textureList.Show(dockPanel, DockState.DockBottom);
        }

        private IDockContent FindDocument(string text)
        {
            if (dockPanel.DocumentStyle == DocumentStyle.SystemMdi)
            {
                foreach (Form form in MdiChildren)
                    if (form.Text == text)
                        return form as IDockContent;

                return null;
            }
            else
            {
                foreach (IDockContent content in dockPanel.Documents)
                    if (content.DockHandler.TabText == text)
                        return content;

                return null;
            }
        }

        private IDockContent GetContentFromPersistString(string persistString)
        {
            if (persistString == typeof(SegmentTool).ToString())
                return m_segmentTool;
            else if (persistString == typeof(LightTool).ToString())
                return m_lightTool;
            else if (persistString == typeof(RenderWindow).ToString())
                return m_renderWindow;
            else if (persistString == typeof(TextureWindow).ToString())
                return m_textureList;
            else
                return null;
        }

        private void MainWindow_Load(object sender, EventArgs e)
        {
            string configFile = Path.Combine(Path.GetDirectoryName(Application.ExecutablePath), "DockPanel.config");

            if (m_bSaveLayout)
            {
                if (File.Exists(configFile))
                    dockPanel.LoadFromXml(configFile, m_deserializeDockContent);
                if (dockPanel.Contents.Count == 0)
                    Setup();
            }
         }

        private void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            string configFile = Path.Combine(Path.GetDirectoryName(Application.ExecutablePath), "DockPanel.config");
            if (m_bSaveLayout)
                dockPanel.SaveAsXml(configFile);
            else if (File.Exists(configFile))
                File.Delete(configFile);
        }
    }
}
