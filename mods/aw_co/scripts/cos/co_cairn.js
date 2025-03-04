CO_CAIRN.globalRules = true;
CO_CAIRN.init = function(co, map)
{
    co.setPowerStars(0);
    co.setSuperpowerStars(4);
};
CO_CAIRN.getTerrainDefenseModifier = function(co, unit, posX, posY, map)
{
    if (co.getIsCO0() === true)
    {
        if (CO_CAIRN.isWildernessTile(posX, posY, map))
        {
            return CO_CAIRN.coZoneStarBonus;
        }
    }
    return 0;
};
CO_CAIRN.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                      defender, defPosX, defPosY, isDefender, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {
        if (map !== null)
        {
            if (map.onMap(atkPosX, atkPosY))
            {
                var terrain = map.getTerrain(atkPosX, atkPosY);
                var startpower = 0;
                if (terrain.getBuilding() !== null)
                {
                    startpower = -10;
                }
                switch (co.getPowerMode())
                {
                case GameEnums.PowerMode_Tagpower:
                case GameEnums.PowerMode_Superpower:
                    if (CO_CAIRN.isWildernessTile(atkPosX, atkPosY, map))
                    {
                        var terrainDefense = terrain.getDefense(attacker);
                        return terrainDefense * 10 + 10;
                    }
                    else
                    {
                        return 10 + startpower;
                    }
                case GameEnums.PowerMode_Power:
                    return 10 + startpower;
                default:
                    return startpower;
                }
            }
        }
    }
    return 0;
};
CO_CAIRN.getFirerangeModifier = function(co, unit, posX, posY, map)
{
    if (co.getIsCO0() === true)
    {
        if (CO_CAIRN.isWildernessTile(posX, posY, map))
        {
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                break;
            case GameEnums.PowerMode_Power:

                if (unit.getBaseMaxRange() > 1)
                {
                    return 1;
                }
                break;
            default:
                break;
            }
        }
    }
    return 0;
};
CO_CAIRN.getDeffensiveReduction = function(co, attacker, atkPosX, atkPosY,
                                           defender, defPosX, defPosY, isAttacker, action, luckMode, map)
{
    if (co.getIsCO0() === true)
    {
        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Tagpower:
        case GameEnums.PowerMode_Superpower:
            if (CO_CAIRN.isWildernessTile(defPosX, defPosY, map))
            {
                var terrainDefense = map.getTerrain(defPosX, defPosY).getDefense(defender);
                return terrainDefense * 10;
            }
            break;
        case GameEnums.PowerMode_Power:
            break;
        default:
            break;
        }
    }
    return 0;
};
CO_CAIRN.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                       defender, defPosX, defPosY, isAttacker, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {
        if (co.getPowerMode() > GameEnums.PowerMode_Off)
        {
            return 10;
        }
    }
    return 0;
};
CO_CAIRN.getMovementcostModifier = function(co, unit, posX, posY, map)
{
    if (co.getIsCO0() === true)
    {
        if (unit.getOwner() === co.getOwner())
        {
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                break;
            case GameEnums.PowerMode_Power:
                if (CO_CAIRN.isWildernessTile(posX, posY, map))
                {
                    return -1;
                }
                break;
            default:
            }
        }
    }
    return 0;
};
CO_CAIRN.getVisionrangeModifier = function(co, unit, posX, posY, map)
{
    if (co.getIsCO0() === true)
    {
        if (unit.getOwner() === co.getOwner())
        {
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                break;
            case GameEnums.PowerMode_Power:
                if (CO_CAIRN.isWildernessTile(posX, posY, map))
                {
                    return -1;
                }
                break;
            default:
            }
        }
    }
    return 0;
};
CO_CAIRN.postAction = function(co, action, map)
{
    if (co.getIsCO0() === true)
    {
        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Tagpower:
        case GameEnums.PowerMode_Superpower:
            var unit = action.getPerformingUnit();
            if (unit !== null && unit.getHp() > 0)
            {
                var path = action.getMovePath();
                var heal = 0;
                for (var i = 0; i < path.length; ++i)
                {
                    var pos = path[i];
                    if (CO_CAIRN.isWildernessTile(pos.x, pos.y, map))
                    {
                        ++heal;
                    }
                }
                if (heal > 0)
                {
                    unit.setHp(unit.getHpRounded() + heal);
                }
            }
            break;
        case GameEnums.PowerMode_Power:
            break;
        default:
            break;
        }
    }
};
