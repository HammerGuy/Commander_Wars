var Constructor = function()
{
    this.getTerrainGroup = function()
    {
        return 1;
    };
    this.baseTerrainId = "PLAINS";
    this.loadBaseSprite = function(terrain, map, map)
    {
        __BASEFOREST.loadBase(terrain, "FOREST", "forest_style2", map)
    };
    this.loadOverlaySprite = function(terrain, map, map)
    {
        __BASEFOREST.loadOverlay(terrain, "FOREST", "forest_style2", map);
    };

    this.getTerrainSprites = function()
    {
        return __BASEFOREST.getSprites("forest_style2")
    };
};
Constructor.prototype = __BASEFOREST;
var FOREST = new Constructor();
