using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    class UndoData
    {
        public enum UndoFlag : uint
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


       	UndoItem<Vertex>			m_vertices;
	    UndoItem<Segment>			m_segments;
        //UndoItem<MatCenter>			m_robotMakers;
        //UndoItem<MatCenter>			m_equipMakers;
	    UndoItem<Wall>				m_walls;
        //UndoItem<Door>				m_doors;
	    UndoItem<Trigger>[]			m_triggers = new UndoItem<Trigger> [2];
        //UndoItem<ReactorTrigger>	m_reactorTriggers;
	    UndoItem<GameObject>		m_objects;
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

	    void Backup (int dataFlags)
        {
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

        }

        // ------------------------------------------------------------------------

	    void Restore ()
        {
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
