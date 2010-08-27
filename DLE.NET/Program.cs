using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace DLE.NET
{
    public static class DLE
    {
        public static GameMine theMine = new GameMine();

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
