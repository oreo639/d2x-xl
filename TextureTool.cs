using System.Drawing;
using System.Windows.Forms;

namespace DLE.NET
{
    public partial class TextureTool : ToolWindow
    {
        public TextureTool()
        {
            InitializeComponent();

            // arrange the light tick panel buttons on a circle
            Point minPos = new Point(30000, 30000);
            Point maxPos = new Point(-30000, -30000);

            // first get the bounding box (requires the radio buttons 1,9,17 and 25 (those on the X and Y axis) 
            // to be at their exact position)
            foreach (Control control in lightTickPanel.Controls)
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

            foreach (Control control in lightTickPanel.Controls)
            {
                if ((control is Panel) && (control.Name.Length > 9) && (control.Name.Substring(0, 9) == "tickPanel"))
                {
                    Point pos = center;
                    pos.X = center.X + (int)(xRad * (float)System.Math.Cos((float)control.TabIndex * scale));
                    pos.Y = center.Y + (int)(yRad * (float)System.Math.Sin((float)control.TabIndex * scale));
                    control.Location = pos;
                }
            }
        }
    }
}
