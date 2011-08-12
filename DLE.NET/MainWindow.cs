using System;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;
using System.IO;
using System.Drawing;

namespace DLEdotNET
{
    public partial class MainWindow : Form
    {
        private bool m_bSaveLayout = false;
        private DeserializeDockContent m_deserializeDockContent;

        private SegmentTool m_segmentTool;
        private LightTool m_lightTool;
        private TextureTool m_textureTool;
        private TriggerTool m_triggerTool;
        private WallTool m_wallTool;
        private ObjectTool m_objectTool;
        private ReactorTool m_reactorTool;
        private MissionTool m_missionTool;
        private EffectTool m_effectTool;
        private DiagnosisTool m_diagnosisTool;
        private PreferencesForm m_prefsForm;
        private TextureFilterTool m_textureFilter;

        private RenderWindow m_renderWindow;
        private TextureWindow m_textureList;

        const float toolPaneWidth = 345.0f;
        const float texturePaneHeight = 200.0f;

        //------------------------------------------------------------------------------

        public MainWindow ()
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
            m_wallTool = new WallTool();
            m_wallTool.RightToLeftLayout = false;
            m_triggerTool = new TriggerTool();
            m_triggerTool.RightToLeftLayout = false;
            m_lightTool = new LightTool();
            m_lightTool.RightToLeftLayout = false;
            m_prefsForm = new PreferencesForm();
            m_prefsForm.RightToLeftLayout = false;
            m_objectTool = new ObjectTool();
            m_objectTool.RightToLeftLayout = false;
            m_reactorTool = new ReactorTool();
            m_reactorTool.RightToLeftLayout = false;
            m_effectTool = new EffectTool();
            m_effectTool.RightToLeftLayout = false;
            m_diagnosisTool = new DiagnosisTool();
            m_diagnosisTool.RightToLeftLayout = false;
            m_missionTool = new MissionTool();
            m_missionTool.RightToLeftLayout = false;
            m_missionTool = new MissionTool();
            m_missionTool.RightToLeftLayout = false;
            m_textureFilter = new TextureFilterTool();
            m_textureFilter.RightToLeftLayout = false;
            if (!m_bSaveLayout)
                Setup();
            //dockPanel.DockRightPortion = (float)m_segmentTool.Width / (float)dockPanel.Width;
            dockPanel.DockLeftPortion = toolPaneWidth / ((float)dockPanel.Width - 16.0f);
            dockPanel.DockRightPortion = toolPaneWidth / ((float)dockPanel.Width - 16.0f);
            dockPanel.DockBottomPortion = texturePaneHeight / ((float)dockPanel.Height - 16.0f);
            m_deserializeDockContent = new DeserializeDockContent(GetContentFromPersistString);
        }

        //------------------------------------------------------------------------------

        private void Setup ()
        {
            m_renderWindow.Show(dockPanel, DockState.Document);
            m_segmentTool.Show(dockPanel, DockState.DockLeft);
            m_lightTool.Show(m_segmentTool.Pane, null);
            m_textureTool.Show(m_segmentTool.Pane, null);
            m_wallTool.Show(m_segmentTool.Pane, null);
            m_triggerTool.Show(m_segmentTool.Pane, null);
            m_effectTool.Show(m_segmentTool.Pane, null);
            m_objectTool.Show(m_segmentTool.Pane, null);
            m_reactorTool.Show(m_segmentTool.Pane, null);
            m_missionTool.Show(m_segmentTool.Pane, null);
            m_diagnosisTool.Show(m_segmentTool.Pane, null);
            m_prefsForm.Show(m_segmentTool.Pane, null);
            m_segmentTool.Show();
            m_textureList.Show(m_renderWindow.Pane, DockAlignment.Bottom, 200.0f / (float)m_renderWindow.Height);
            m_textureFilter.Show(m_textureList.Pane, DockAlignment.Left, (float)265.0f / (float)m_renderWindow.Width);
            //m_textureList.Show(dockPanel, DockState.DockBottom);
        }

        //------------------------------------------------------------------------------

        private IDockContent FindDocument (string text)
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

        //------------------------------------------------------------------------------

        private IDockContent GetContentFromPersistString (string persistString)
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

        //------------------------------------------------------------------------------

        private void MainWindow_Load (object sender, EventArgs e)
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

        //------------------------------------------------------------------------------

        private void MainWindow_Closing (object sender, System.ComponentModel.CancelEventArgs e)
        {
            string configFile = Path.Combine(Path.GetDirectoryName(Application.ExecutablePath), "DockPanel.config");
            if (m_bSaveLayout)
                dockPanel.SaveAsXml(configFile);
            else if (File.Exists(configFile))
                File.Delete(configFile);
        }

        //------------------------------------------------------------------------------

        private void tbLightTool_Click (object sender, EventArgs e)
        {
            m_lightTool.Show();
        }

        private void tbTextureTool_Click(object sender, EventArgs e)
        {
            m_textureTool.Show();
        }

        private void tbSegmentTool_Click(object sender, EventArgs e)
        {
            m_segmentTool.Show();
        }

        private void tbWallTool_Click(object sender, EventArgs e)
        {
            m_wallTool.Show();
        }

        private void tbTriggerTool_Click(object sender, EventArgs e)
        {
            m_triggerTool.Show();
        }

        private void tbEffectTool_Click(object sender, EventArgs e)
        {
            m_effectTool.Show();
        }

        private void tbObjectTool_Click(object sender, EventArgs e)
        {
            m_objectTool.Show();
        }

        private void tbReactorTool_Click(object sender, EventArgs e)
        {
            m_reactorTool.Show();
        }

        private void tbMissionTool_Click(object sender, EventArgs e)
        {
            m_missionTool.Show();
        }

        private void tbPreferences_Click(object sender, EventArgs e)
        {
            m_prefsForm.Show();
        }

        private void tbCheckMine_Click(object sender, EventArgs e)
        {
            m_diagnosisTool.Show();
        }

        private void textureEditToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_textureTool.Show();
        }

        private void pointEditToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_segmentTool.Show();
        }

        private void cubeEditToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_segmentTool.Show();
        }

        private void objectEditToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_objectTool.Show();
        }

        private void effectToolToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_effectTool.Show();
        }

        private void wallToolToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_wallTool.Show();
        }

        private void triggerToolToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_triggerTool.Show();
        }

        private void lightCalculationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_lightTool.Show();
        }

        private void texturealignmentToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_textureTool.Show();
        }

        private void reactorTriggersToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_reactorTool.Show();
        }

        private void texturefiltersToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void curvegeneratorToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void tbTextureFilter_Click(object sender, EventArgs e)
        {
            m_textureFilter.Show();
        }

        private void standardToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void zoomOutToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void inToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void forwardCubeToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

    }
}
