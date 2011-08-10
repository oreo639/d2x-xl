
namespace DLE.NET
{
    public partial class GameObject
    {
        public int Key { get; set; }

        // ------------------------------------------------------------------------

        short signature;     // reduced size to save memory 
        char        type;          // what type of object this is... robot, weapon, hostage, powerup, fireball 
        char        id;            // which form of object...which powerup, robot, etc. 
        byte        control_type;  // how this object is controlled 
        byte        movement_type; // how this object moves 
        byte        render_type;   //  how this object renders 
        byte        flags;         // misc flags 
        byte        multiplayer;   // object only available in multiplayer games 
        short       segnum;        // segment number containing object 
        FixVector   pos;           // absolute x,y,z coordinate of center of object 
        FixMatrix   orient;        // orientation of object in world 
        int         size;          // 3d size of object - for collision detection 
        int         shields;       // Starts at maximum, when <0, object dies.. 
        FixVector   last_pos;      // where object was last frame 
        char        contains_type; //  Type of object this object contains (eg, spider contains powerup) 
        char        contains_id;   //  ID of object this object contains (eg, id = blue type = key) 
        char        contains_count;// number of objects of type:id this object contains 

        MType       mtype;
        CType       ctype;
        RType       rtype;

        // ------------------------------------------------------------------------

        public GameObject (int key = 0)
        {
            Key = key;
        }

        // ------------------------------------------------------------------------

    }
}
