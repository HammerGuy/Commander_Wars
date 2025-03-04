CO_EAGLE.getFuelCostModifier = function(co, unit, costs, map)
{
    if (co.getIsCO0() === true)
    {
        if (unit.getUnitType() === GameEnums.UnitType_Air)
        {
            return -2;
        }
    }
    return 0;
};

CO_EAGLE.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                      defender, defPosX, defPosY, isDefender, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {

        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Superpower:
            if (attacker.getUnitType() === GameEnums.UnitType_Air)
            {
                return 30;
            }
            else if (attacker.getUnitType() === GameEnums.UnitType_Naval)
            {
                return 0;
            }
            return 10;
        case GameEnums.PowerMode_Power:
            if (attacker.getUnitType() === GameEnums.UnitType_Air)
            {
                return -30;
            }
            else if (attacker.getUnitType() === GameEnums.UnitType_Naval)
            {
                return -45;
            }
            else
            {
                return -45;
            }
        default:
            if (attacker.getUnitType() === GameEnums.UnitType_Air)
            {
                    return 20;
            }
            break;
        }
        if (attacker.getUnitType() === GameEnums.UnitType_Naval)
        {
            return -10;
        }
    }
    return 0;
};

CO_EAGLE.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
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
