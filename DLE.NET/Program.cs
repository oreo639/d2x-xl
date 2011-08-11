using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace DLE.NET
{
    public static class DLE
    {
        public static GameMine Mine { get { return theMine; } }
        public static Settings Settings { get { return settings; } }
        public static TextureManager Textures { get { return m_textureManager; } }
        public static PaletteManager Palettes { get { return m_paletteManager; } }
        public static UndoManager Backup { get { return m_undoManager; } }
        public static VertexManager Vertices { get { return m_vertexManager; } }
        public static SegmentManager Segments { get { return m_segmentManager; } }
        public static WallManager Walls { get { return m_wallManager; } }
        public static TriggerManager Triggers { get { return m_triggerManager; } }
        public static ObjectManager Objects { get { return m_objectManager; } }
        public static LightManager Lights { get { return m_lightManager; } }
        public static MineView MineView { get { return m_mineView; } }
        public static Selection Current { get { return m_selections [0]; } }
        public static Selection Other { get { return m_selections [1]; } }
        public static Selection Selection (int i) { return m_selections [i]; }
        
        static GameMine theMine = new GameMine ();
        static Settings settings = new Settings ();
        public static TextureManager m_textureManager = new TextureManager ();
        public static PaletteManager m_paletteManager = new PaletteManager ();
        public static UndoManager m_undoManager = new UndoManager ();
        public static VertexManager m_vertexManager = new VertexManager ();
        public static SegmentManager m_segmentManager = new SegmentManager ();
        public static WallManager m_wallManager = new WallManager ();
        public static TriggerManager m_triggerManager = new TriggerManager ();
        public static ObjectManager m_objectManager = new ObjectManager ();
        public static LightManager m_lightManager = new LightManager ();
        public static Selection [] m_selections = new Selection [2] { new Selection (), new Selection () };
        public static MineView m_mineView = new MineView ();

        static public bool Modified { get; set; }

        public static bool IsD1File { get { return theMine.FileType == GameMine.GameFileType.RDL; } }
        public static bool IsD2File { get { return theMine.FileType != GameMine.GameFileType.RDL; } }
        public static bool IsStdLevel { get { return theMine.LevelVersion < 9; } }
        public static bool IsD2XLevel { get { return theMine.LevelVersion >= 9; } }
        public static int FileType { get { return (int) theMine.FileType; } }
        public static int LevelVersion { get { return theMine.LevelVersion; } }

        public static String[] descentPath = new String [2] { "", "" };

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainWindow());
        }
    }
}
