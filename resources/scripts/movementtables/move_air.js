var Constructor = function()
{
    this.getName = function()
    {
        return qsTr("Air");
    };
    this.getMovementpoints = function(terrain)
    {
        switch (terrain.getID())
        {
            case "PLAINS":
            case "PLAINS_DESTROYED":
            case "PLAINS_PLASMA":
            case "BEACH":
            case "BRIDGE":
            case "DESTROYEDWELD":            
            case "RUIN":
            case "STREET":
            case "AIRPORT":
            case "FACTORY":
            case "HARBOUR":
            case "HQ":
            case "MINE":
            case "PIPESTATION":
            case "RADAR":
            case "TOWER":
            case "TOWN":
            case "SILO":
            case "SILO_ROCKET":
            case "LABOR":
			case "FOREST":
			case "WASTELAND":
            case "SEA":
			case "ROUGH_SEA":
            case "REAF":
            case "MOUNTAIN":
            case "RIVER":
				return 1;
        }
        return -1;
    };
};
Constructor.prototype = MOVEMENTTABLE;
var MOVE_AIR = new Constructor();
