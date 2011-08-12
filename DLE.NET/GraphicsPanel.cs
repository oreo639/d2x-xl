using System.Windows.Forms;
using System.Drawing;

namespace DLE.NET
{
    public class GraphicsPanel : Panel
    {
        public GraphicsPanel() : base()
        {
        SetStyle (ControlStyles.DoubleBuffer, true);
        SetStyle (ControlStyles.AllPaintingInWmPaint, true);
        SetStyle (ControlStyles.UserPaint, true);       
        }

        protected override void OnPaintBackground(PaintEventArgs pe)
        {
            return;
        }

        protected override void  OnPaint(PaintEventArgs e)
        {
            using (Graphics gfx = e.Graphics)
            {
            }
            base.OnPaint (e);
        }
    }
}
