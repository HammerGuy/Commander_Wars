var Constructor = function()
{
    this.init = function (building)
    {
        building.setHp(100);
        building.setAlwaysVisble(true);
    };
    this.loadSprites = function(building, neutral, map)
    {
        building.loadSprite("nest_center_laser_middle", false, 400, Qt.point(0, building.getImageSize()));
    };
    this.getBaseIncome = function()
    {
        return 0;
    };
    this.getDefense = function()
    {
        return 0;
    };
    this.getMiniMapIcon = function()
    {
        return "minimap_blackholebuilding";
    };
    this.getName = function()
    {
        return qsTr("Nest");
    };
    this.getDescription = function()
    {
        return qsTr("Nest of Dark Matter. Currently only a visual.");
    };
    this.usesMapLayer = function()
    {
        return true;
    };

    this.onDestroyed = function(building, map)
    {
        // called when the terrain is destroyed and replacing of this terrain starts
        var x = building.getX();
        var y = building.getY();
        var animation = GameAnimationFactory.createAnimation(map, x, y);
        animation.addSprite("explosion+land", -map.getImageSize() / 2, -map.getImageSize(), 0, 2);
        animation.addScreenshake(30, 0.95, 1000, 200);
        animation.setSound("explosion+land.wav");
        map.getTerrain(x, y).loadBuilding("ZNEST_CENTER_LASER_MIDDLE_DESTROYED");
    };
    this.getActionTargetFields = function(building)
    {
        var targets = globals.getEmptyPointArray();
        // laser to not fire infinitly but the range is still fucking huge :)
        for (var i = 1; i < 20; i++)
        {
            targets.append(Qt.point(0, i));
        }
        return targets;
    };
    this.getDamage = function(building, unit)
    {
        return 8;
    };
}

Constructor.prototype = BUILDING;
var ZNEST_CENTER_LASER_MIDDLE = new Constructor();
