using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLEdotNET
{
    public class UndoData
    {
        public enum Flags : int
        {
	        udVertices = 1,
	        udSegments = 2,
	        udMatCenters = 4,
	        udWalls = 8,
	        udDoors = 16,
	        udTriggers = 32,
	        udObjects = 64,
	        udRobots = 128,
	        udVariableLights = 256,
	        udStaticLight = 512,
	        udDynamicLight = 1024,
	        udLight = 0x700,
	        udAll = 0x7FF
        }


       	UndoItem<Vertex>			m_vertices = new UndoItem<Vertex> ();
	    UndoItem<Segment>			m_segments = new UndoItem<Segment> ();
        //UndoItem<MatCenter>			m_robotMakers;
        //UndoItem<MatCenter>			m_equipMakers;
	    UndoItem<Wall>				m_walls = new UndoItem<Wall> ();
        //UndoItem<Door>				m_doors;
	    UndoItem<Trigger>[]			m_triggers = new UndoItem<Trigger> [2];
        //UndoItem<ReactorTrigger>	m_reactorTriggers;
	    UndoItem<GameObject>		m_objects = new UndoItem<GameObject> ();
        //UndoItem<RobotInfo>			m_robotInfo;
        //UndoItem<LightDeltaIndex>	m_deltaIndices;
        //UndoItem<LightDeltaValue>	m_deltaValues;
        //UndoItem<VariableLight>		m_variableLights;
        //UndoItem<FaceColor>			m_faceColors;
        //UndoItem<TextureColor>		m_textureColors;
        //UndoItem<VertexColor>		m_vertexColors;
        //UndoItem<SecretExit>		m_secretExit;
        //UndoItem<ReactorData>		m_reactorData;
        //Selection[]					m_selections = new Selection [2];

        // ------------------------------------------------------------------------

	    bool m_bSelections = false;
	    uint m_nId = 0;

	    uint Id { get {return m_nId; } }

        // ------------------------------------------------------------------------

	    void Backup (Flags dataFlags)
        {
            //if ((dataFlags & UndoFlag.udVertices) != 0)
            //    m_vertices.Backup (vertexManager.Vertex (0));

            //if ((dataFlags & UndoFlag.udSegments) != 0)
            //    m_segments.Backup (segmentManager.Segment (0));

            //if ((dataFlags & UndoFlag.udMatCenters) != 0)
            //{
            //    m_robotMakers.Backup (segmentManager.RobotMaker (0));
            //    m_equipMakers.Backup (segmentManager.EquipMaker (0));
            //}

            //if ((dataFlags & UndoFlag.udWalls) != 0)
            //    m_walls.Backup (wallManager.Wall (0), &wallManager.WallCount ());

            //if ((dataFlags & UndoFlag.udTriggers) != 0)
            //{
            //    m_triggers [0].Backup (triggerManager.Trigger (0, 0));
            //    m_triggers [1].Backup (triggerManager.Trigger (0, 1));
            //    m_reactorTriggers.Backup (triggerManager.ReactorTrigger (0));
            //    m_reactorData.Backup (&triggerManager.ReactorData ());
            //}

            //if ((dataFlags & UndoFlag.udObjects) != 0)
            //{
            //    m_objects.Backup (objectManager.Object (0));
            //    m_secretExit.Backup (&objectManager.SecretExit ());
            //}

            //if ((dataFlags & UndoFlag.udRobots) != 0)
            //    m_robotInfo.Backup (robotManager.RobotInfo (0));

            //if ((dataFlags & UndoFlag.udVariableLights) != 0)
            //    m_variableLights.Backup (lightManager.VariableLight (0));

            //if ((dataFlags & UndoFlag.udStaticLight) != 0)
            //{
            //    m_faceColors.Backup (lightManager.FaceColor (0));
            //    m_textureColors.Backup (lightManager.TexColor (0));
            //    m_vertexColors.Backup (lightManager.VertexColor (0)));
            //}

            //if ((dataFlags & UndoFlag.udDynamicLight) != 0)
            //{
            //    m_deltaIndices.Backup (lightManager.LightDeltaIndex (0));
            //    m_deltaValues.Backup (lightManager.LightDeltaValue (0));
            //}

            //if (!m_bSelections)
            //{
            //    m_selections [0].Copy (selections [0]);
            //    m_selections [1].Copy (selections [1]);
            //    m_bSelections = true;
            //}
        }

        // ------------------------------------------------------------------------

	    bool Cleanup ()
        {
            bool bEmpty = true;

            if (m_vertices.Cleanup ()) bEmpty = false;
            if (m_segments.Cleanup ()) bEmpty = false;
            //if (m_robotMakers.Cleanup ()) bEmpty = false;
            //if (m_equipMakers.Cleanup ()) bEmpty = false;
            if (m_walls.Cleanup ()) bEmpty = false;
            //if (m_doors.Cleanup ()) bEmpty = false;
            if (m_triggers [0].Cleanup ()) bEmpty = false;
            if (m_triggers [1].Cleanup ()) bEmpty = false;
            //if (m_reactorTriggers.Cleanup ()) bEmpty = false;
            //if (m_reactorData.Cleanup ()) bEmpty = false;
            if (m_objects.Cleanup ()) bEmpty = false;
            //if (m_secretExit.Cleanup ()) bEmpty = false;
            //if (m_robotInfo.Cleanup ()) bEmpty = false;
            //if (m_deltaIndices.Cleanup ()) bEmpty = false;
            //if (m_deltaValues.Cleanup ()) bEmpty = false;
            //if (m_variableLights.Cleanup ()) bEmpty = false;
            //if (m_faceColors.Cleanup ()) bEmpty = false;
            //if (m_textureColors.Cleanup ()) bEmpty = false;
            //if (m_vertexColors.Cleanup ()) bEmpty = false;
            return bEmpty;
        }

        // ------------------------------------------------------------------------

	    void Restore ()
        {
            m_vertices.Restore ();
            m_segments.Restore ();
            //m_robotMakers.Restore ();
            //m_equipMakers.Restore ();
            m_walls.Restore ();
            m_triggers [0].Restore ();
            //m_reactorData.Restore ();
            m_triggers [1].Restore ();
            m_objects.Restore ();
            //m_secretExit.Restore ();
            //m_robotInfo.Restore ();
            //m_variableLights.Restore ();
            //m_faceColors.Restore ();
            //m_textureColors.Restore ();
            //m_vertexColors.Restore ();
            //m_deltaIndices.Restore ();
            //m_deltaValues.Restore ();
            //selections [0].Copy (m_selections [0]);
            //selections [1].Copy (m_selections [1]);
        }

        // ------------------------------------------------------------------------

	    void Destroy ()
        {
            m_vertices.Destroy ();
            m_segments.Destroy ();
            //m_robotMakers.Destroy ();
            //m_equipMakers.Destroy ();
            m_walls.Destroy ();
            //m_doors.Destroy ();
            //m_triggers [0].Destroy ();
            //m_triggers [1].Destroy ();
            //m_reactorTriggers.Destroy ();
            m_objects.Destroy ();
            //m_robotInfo.Destroy ();
            //m_deltaIndices.Destroy ();
            //m_deltaValues.Destroy ();
            //m_variableLights.Destroy ();
            //m_faceColors.Destroy ();
            //m_textureColors.Destroy ();
            //m_vertexColors.Destroy ();
            //m_secretExit.Destroy ();
            //m_reactorData.Destroy ();
            m_bSelections = false;

        }

        // ------------------------------------------------------------------------

	    void Reset ()
        {
            m_vertices.Reset ();
            m_segments.Reset ();
            //m_robotMakers.Reset ();
            //m_equipMakers.Reset ();
            m_walls.Reset ();
            m_triggers [0].Reset ();
            m_triggers [1].Reset ();
            //m_reactorData.Reset ();
            m_objects.Reset ();
            //m_secretExit.Reset ();
            //m_robotInfo.Reset ();
            //m_variableLights.Reset ();
            //m_faceColors.Reset ();
            //m_textureColors.Reset ();
            //m_vertexColors.Reset ();
            //m_deltaIndices.Reset ();
            //m_deltaValues.Reset ();
            m_bSelections = false;
        }

        // ------------------------------------------------------------------------

	    UndoData () { }

        // ------------------------------------------------------------------------

	    ~UndoData () 
        { 
            Destroy (); 
        }

        // ------------------------------------------------------------------------

    }
}
