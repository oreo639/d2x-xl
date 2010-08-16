using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace DockingControls
{
    public partial class DockingManager : UserControl
    {
        public enum Sides
        {
            left, right, top, bottom
        }

        private ToolStrip[] buttonStrips;

        public DockingManager()
        {
            InitializeComponent();
        }

        public void CreateAllElements()
        {
            // For testing reasons we set the visibility to true
            // in the DockingManager, it will be false as they are
            // not to be seen when created
            CreateButtonStrip(DockStyle.Left, true);
            CreateButtonStrip(DockStyle.Right, true);
            CreateButtonStrip(DockStyle.Top, true);
            CreateButtonStrip(DockStyle.Bottom, true);
            // here other needed elements will be created too
        }

        private void CreateButtonStrip(DockStyle pos, bool visible)
        {
            ToolStrip buttonStrip = CreateButtonStrip(pos);
            if (buttonStrip != null)
            {
                buttonStrip.Visible = visible;
            }
        }

        private ToolStrip CreateButtonStrip(DockStyle pos)
        {
            ToolStrip buttonStrip = new ToolStrip();
            buttonStrip.AutoSize = false;
            buttonStrip.GripStyle = ToolStripGripStyle.Hidden;
            if (pos == DockStyle.Left || pos == DockStyle.Right)
            {
                // we need a vertical toolstrip 
                buttonStrip.LayoutStyle = ToolStripLayoutStyle.
                VerticalStackWithOverflow;
                buttonStrip.TextDirection =
                   ToolStripTextDirection.Vertical90;
                buttonStrip.Size = new Size(24, 309);
                buttonStrip.Location = new Point(0, 49);
            }
            else
            {
                // Top or bottom are horizontal Toolstrips
                buttonStrip.LayoutStyle = ToolStripLayoutStyle.HorizontalStackWithOverflow;
                buttonStrip.TextDirection = ToolStripTextDirection.Horizontal;
                buttonStrip.Size = new Size(309, 24);
                buttonStrip.Location = new Point(0, 49);
            }
            buttonStrip.RenderMode = ToolStripRenderMode.ManagerRenderMode;
            // lets sign the controls so we can access them easier
            switch (pos)
            {
                case DockStyle.Left:
                    buttonStrip.Text = "Left";
                    buttonStrip.Name = "bsLeft";
                    break;
                case DockStyle.Right:
                    buttonStrip.Text = "Right";
                    buttonStrip.Name = "bsRight";
                    break;
                case DockStyle.Top:
                    buttonStrip.Text = "Top";
                    buttonStrip.Name = "bsTop";
                    break;
                case DockStyle.Bottom:
                    buttonStrip.Text = "Bottom";
                    buttonStrip.Name = "bsBottom";
                    break;
            }
            if (Parent != null)
            {
                // now we are docking the control;
                buttonStrip.Dock = pos;
                Parent.Controls.Add(buttonStrip);
            }
            return buttonStrip;
        }
    }
}
