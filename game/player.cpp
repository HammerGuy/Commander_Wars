#include "QRandomGenerator"

#include "game/player.h"

#include "game/gamemap.h"

#include "coreengine/mainapp.h"

#include "gameinput/basegameinputif.h"

#include "game/building.h"

#include "game/unit.h"

#include "game/co.h"

#include "menue/gamemenue.h"

Player::Player(quint32 id)
    : playerID(id)
{
    Mainapp* pApp = Mainapp::getInstance();
    QString function = "loadDefaultPlayerColor";
    QJSValueList args;
    QJSValue objArg = pApp->getInterpreter()->newQObject(this);
    args << objArg;
    pApp->getInterpreter()->doFunction("PLAYER", function, args);
}

Player::~Player()
{
    if (m_pBaseGameInput != nullptr)
    {
        delete m_pBaseGameInput;
    }
}

QColor Player::getColor() const
{
    return m_Color;
}

void Player::setColor(QColor color)
{
    m_Color = color;
}

quint32 Player::getPlayerID() const
{
    return playerID;
}

void Player::setPlayerID(const quint32 &value)
{
    playerID = value;
}

QString Player::getArmy()
{
    if (GameMenue::getInstance() != nullptr)
    {
        return playerArmy;
    }
    else
    {
        // editor memu mode
        Mainapp* pApp = Mainapp::getInstance();
        QJSValueList args;
        QJSValue objArg = pApp->getInterpreter()->newQObject(this);
        args << objArg;
        QJSValue ret = pApp->getInterpreter()->doFunction("PLAYER", "getDefaultArmy", args);
        if (ret.isString())
        {
            return ret.toString();
        }
        else
        {
            return "OS";
        }
    }
}

GameEnums::Alliance Player::checkAlliance(Player* pPlayer)
{
    if (pPlayer == this)
    {
        return GameEnums::Alliance_Friend;
    }
    else
    {
        // todo implement real check for alliance
        return GameEnums::Alliance_Enemy;
    }
}

bool Player::isEnemyUnit(Unit* pUnit)
{
    return (checkAlliance(pUnit->getOwner()) == GameEnums::Alliance_Enemy);
}


void Player::setFonds(const qint32 &value)
{
    fonds = value;
    if (GameMenue::getInstance() != nullptr)
    {
        GameMenue::getInstance()->updatePlayerinfo();
    }
}

void Player::addFonds(const qint32 &value)
{
    setFonds(fonds + value);
}

qint32 Player::getFonds() const
{
    return fonds;
}

void Player::earnMoney(float modifier)
{
    GameMap* pMap = GameMap::getInstance();
    for (qint32 y = 0; y < pMap->getMapHeight(); y++)
    {
        for (qint32 x = 0; x < pMap->getMapWidth(); x++)
        {
            spBuilding pBuilding = pMap->getSpTerrain(x, y)->getSpBuilding();
            if (pBuilding.get() != nullptr)
            {
                if (pBuilding->getOwner() == this)
                {
                    quint32 income = static_cast<quint32>(pBuilding->getBaseIncome() * modifier * fondsModifier);
                    // todo modifier income by co's and rules

                    fonds += income;
                }
            }
        }
    }
    setFonds(fonds);
}

qint32 Player::getCostModifier(QString id, qint32 baseCost)
{
    qint32 costModifier = 0;
    if (playerCOs[0].get() != nullptr)
    {
       costModifier += playerCOs[0]->getCostModifier(id, baseCost);
    }
    if (playerCOs[1].get() != nullptr)
    {
       costModifier += playerCOs[1]->getCostModifier(id, baseCost);
    }
    return costModifier;
}

qint32 Player::getCosts(QString id)
{
    Mainapp* pApp = Mainapp::getInstance();
    QJSValue ret = pApp->getInterpreter()->doFunction(id, "getBaseCost");
    qint32 costs = 0;
    if (ret.isNumber())
    {
        costs = ret.toInt();
    }
    costs += getCostModifier(id, costs);
    return costs;
}

void Player::gainPowerstar(qint32 fondsDamage, QPoint position)
{
    if (playerCOs[0].get() != nullptr)
    {
       playerCOs[0]->gainPowerstar(fondsDamage, position);
    }
    if (playerCOs[1].get() != nullptr)
    {
       playerCOs[1]->gainPowerstar(fondsDamage, position);
    }
    if (GameMenue::getInstance() != nullptr)
    {
        GameMenue::getInstance()->updatePlayerinfo();
    }
}

qint32 Player::getMovementpointModifier(Unit* pUnit, QPoint position)
{
    qint32 modifier = 0;
    if (playerCOs[0].get() != nullptr)
    {
       modifier += playerCOs[0]->getMovementpointModifier(pUnit, position);
    }
    if (playerCOs[1].get() != nullptr)
    {
       modifier += playerCOs[1]->getMovementpointModifier(pUnit, position);
    }
    return modifier;
}

void Player::startOfTurn()
{
    if (playerCOs[0].get() != nullptr)
    {
       playerCOs[0]->setPowerMode(GameEnums::PowerMode_Off);
       playerCOs[0]->startOfTurn();
    }
    if (playerCOs[1].get() != nullptr)
    {
       playerCOs[1]->setPowerMode(GameEnums::PowerMode_Off);
       playerCOs[1]->startOfTurn();
    }
}

QmlVectorUnit* Player::getUnits()
{
    return GameMap::getInstance()->getUnits(this);
}

void Player::updateVisualCORange()
{
    if (playerCOs[0].get() != nullptr)
    {
       if (playerCOs[0]->getCOUnit() != nullptr)
       {
           playerCOs[0]->getCOUnit()->createCORange(playerCOs[0]->getCORange());
       }
    }
    if (playerCOs[1].get() != nullptr)
    {
        if (playerCOs[1]->getCOUnit() != nullptr)
        {
            playerCOs[1]->getCOUnit()->createCORange(playerCOs[1]->getCORange());
        }
    }
}

void Player::setBaseGameInput(BaseGameInputIF *pBaseGameInput)
{
    m_pBaseGameInput = pBaseGameInput;
    m_pBaseGameInput->setPlayer(this);
}

CO* Player::getCO(quint8 id)
{
    if (id <= 1)
    {
        return playerCOs[id].get();
    }
    else
    {
        return nullptr;
    }
}

void Player::setCO(QString coId, quint8 idx)
{
    if (idx <= 1)
    {
        playerCOs[idx] = new CO(coId, this);
    }
}

QPoint Player::getRockettarget(qint32 radius, qint32 damage, float ownUnitValue)
{
    GameMap* pMap = GameMap::getInstance();
    QmlVectorPoint* pPoints = Mainapp::getCircle(0, radius);
    qint32 highestDamage = -1;
    QVector<QPoint> targets;

    for (qint32 x = 0; x < pMap->getMapWidth(); x++)
    {
        for (qint32 y = 0; y < pMap->getMapHeight(); y++)
        {
            qint32 fondsDamage = 0;
            for (qint32 i = 0; i < pPoints->size(); i++)
            {
                qint32 x2 = x + pPoints->at(i).x();
                qint32 y2 = y + pPoints->at(i).y();
                // is there a unit?
                if ((pMap->onMap(x2, y2)) &&
                    (pMap->getTerrain(x2, y2)->getUnit() != nullptr))
                {
                    Unit* pUnit = pMap->getTerrain(x2, y2)->getUnit();
                    float modifier = 1.0f;
                    if (!isEnemyUnit(pUnit))
                    {
                       modifier = -ownUnitValue;
                    }
                    qint32 damagePoints = damage;
                    qint32 hpRounded = pUnit->getHpRounded();
                    if (hpRounded < damage)
                    {
                        damagePoints = hpRounded;
                    }
                    // calc fonds damage
                    fondsDamage += damagePoints / 10.0f * modifier * pUnit->getCosts();
                }
            }
            if (fondsDamage > highestDamage)
            {
                highestDamage = fondsDamage;
                targets.clear();
                targets.append(QPoint(x, y));
            }
            else if ((fondsDamage == highestDamage) && highestDamage >= 0)
            {
                targets.append(QPoint(x, y));
            }
        }
    }
    delete pPoints;

    // create pseudo rand integer (not
    QRandomGenerator randInt(static_cast<quint32>(highestDamage));
    if (targets.size() >= 0)
    {
        return targets[randInt.bounded(0, targets.size())];
    }
    else
    {
        return QPoint(-1, -1);
    }
}

void Player::defineArmy()
{
    if (playerCOs[0].get() != nullptr)
    {
        playerArmy = playerCOs[0]->getCOArmy();
    }
}

float Player::getFondsModifier() const
{
    return fondsModifier;
}

void Player::setFondsModifier(float value)
{
    fondsModifier = value;
}

void Player::serialize(QDataStream& pStream)
{
    pStream << getVersion();
    quint32 color = m_Color.rgb();
    pStream << color;

    pStream << playerID;
    pStream << fonds;
    pStream << fondsModifier;
    pStream << playerArmy;
    if (playerCOs[0].get() == nullptr)
    {
        pStream << false;
    }
    else
    {
        pStream << true;
        playerCOs[0]->serialize(pStream);
    }
    if (playerCOs[1].get() == nullptr)
    {
        pStream << false;
    }
    else
    {
        pStream << true;
        playerCOs[1]->serialize(pStream);
    }
}
void Player::deserialize(QDataStream& pStream)
{
    qint32 version = 0;
    pStream >> version;
    quint32 color;
    pStream >> color;
    m_Color.fromRgb(color);
    if (version > 1)
    {
        pStream >> playerID;
        pStream >> fonds;
        pStream >> fondsModifier;
        pStream >> playerArmy;
        bool hasC0 = false;
        pStream >> hasC0;
        if (hasC0)
        {
            playerCOs[0] = new CO("", this);
            playerCOs[0]->deserialize(pStream);
        }
        bool hasC1 = false;
        pStream >> hasC1;
        if (hasC1)
        {
            playerCOs[1] = new CO("", this);
            playerCOs[1]->deserialize(pStream);
        }
    }
}
