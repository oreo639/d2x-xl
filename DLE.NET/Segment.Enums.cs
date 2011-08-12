
namespace DLEdotNET
{
    partial class Segment
    {
        public enum Type : byte
        {
            NONE = 0,
            FUELCEN = 1,
            REPAIRCEN = 2,
            CONTROLCEN = 3,
            ROBOTMAKER = 4,
            COUNT_D1 = 5,

            GOAL_BLUE = 5, // Descent 2 only
            GOAL_RED = 6, // Descent 2 only
            WATER = 7,
            LAVA = 8,
            TEAM_BLUE = 9,
            TEAM_RED = 10,
            SPEEDBOOST = 11,
            BLOCKED = 12,
            NODAMAGE = 13,
            SKYBOX = 14,
            EQUIPMAKER = 15, // matcen for powerups
            OUTDOORS = 16,
            COUNT_D2 = 17 // Descent 2 only
        }

        public enum Functions : byte
        {
            NONE = 0,
            FUELCEN = 1,
            REPAIRCEN = 2,
            CONTROLCEN = 3,
            ROBOTMAKER = 4,
            GOAL_BLUE = 5,
            GOAL_RED = 6,
            TEAM_BLUE = 7,
            TEAM_RED = 8,
            SPEEDBOOST = 9,
            SKYBOX = 10,
            EQUIPMAKER = 11,
            COUNT = 12
        }

        public enum Properties : byte
        {
            NONE = 0,
            WATER = 1,
            LAVA = 2,
            BLOCKED = 4,
            NODAMAGE = 8,
            OUTDOORS = 16
        }
    }
}
