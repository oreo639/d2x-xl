
namespace DLE.NET
{
    public partial class GameMine
    {
        int LevelVersion { get; set; }
        GameFileType FileType { get; set; }

        bool IsD1File() { return FileType == GameFileType.RDL; }
        bool IsD2File() { return FileType != GameFileType.RDL; }
        bool IsStdLevel() { return LevelVersion < 9; }
        bool IsD2XLevel() { return LevelVersion >= 9; }
    }
}
