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
        public static TextureManager Textures { get { return textureManager; } }
        public static PaletteManager Palettes { get { return paletteManager; } }
        public static UndoManager Undos { get { return undoManager; } }
        public static VertexManager Vertices { get { return vertexManager; } }
        public static SegmentManager Segments { get { return segmentManager; } }
        public static WallManager Walls { get { return wallManager; } }
        public static ObjectManager Objects { get { return objectManager; } }

        static GameMine theMine = new GameMine ();
        static Settings settings = new Settings ();
        public static TextureManager textureManager = new TextureManager ();
        public static PaletteManager paletteManager = new PaletteManager ();
        public static UndoManager undoManager = new UndoManager ();
        public static VertexManager vertexManager = new VertexManager ();
        public static SegmentManager segmentManager = new SegmentManager ();
        public static WallManager wallManager = new WallManager ();
        public static ObjectManager objectManager = new ObjectManager ();

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
