using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace DLEdotNET
{
    public partial class DiagnosisTool : ToolWindow
    {
        string[] statisticsItems = new string[] { "segments", "vertices", "robot makers", "fuel centers", "walls", "triggers", 
                                                  "objects", "robots", "hostages", "players", "coop players", "powerups", 
                                                  "weapons", "keys", "reactors", "textures" };
        public DiagnosisTool()
        {
            InitializeComponent();
            Setup();
        }

        void Setup()
        {
            float cx = (float) (statisticsView.Width - 16) / 100.0f;

            statisticsView.Columns.Clear();
            statisticsView.Columns.Add("Item", (int)(43 * cx), HorizontalAlignment.Center);
            statisticsView.Columns.Add("Count", (int)(25 * cx), HorizontalAlignment.Center);
            statisticsView.Columns.Add("Max #", (int)(32 * cx), HorizontalAlignment.Center);
 
            ListViewItem[] items = new ListViewItem[16];
            int nItems = 0;

            statisticsView.Items.Clear();
            foreach (string s in statisticsItems)
            {
                ListViewItem li = new ListViewItem(s, 0);
                li.SubItems.Clear();
                li.Text = s;
                li.SubItems.Add("0");
                li.SubItems.Add("0");
                items[nItems++] = li;
            }
           statisticsView.Items.AddRange(items);
        }

        private void statisticsView_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

    }
}
