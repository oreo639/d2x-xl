using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace DLE.NET
{
    public static class DLE
    {
        public static GameMine theMine = new GameMine();
        public static Settings settings = new Settings ();
        public static TextureManager textureManager = new TextureManager ();

        public static bool IsD1File () { return theMine.FileType == GameMine.GameFileType.RDL; }
        public static bool IsD2File () { return theMine.FileType != GameMine.GameFileType.RDL; }
        public static bool IsStdLevel () { return theMine.LevelVersion < 9; }
        public static bool IsD2XLevel () { return theMine.LevelVersion >= 9; }

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
