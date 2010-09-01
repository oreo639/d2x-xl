namespace DLE.NET
{
    public partial class GameMine
    {
        byte[,] sideVertTable = new byte [6,4] 
        {
	        {7,6,2,3},
	        {0,4,7,3},
	        {0,1,5,4},
	        {2,6,5,1},
	        {4,5,6,7},
	        {3,2,1,0} 
        };
    }
}