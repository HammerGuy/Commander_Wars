#include "ai/coreai.h"
#include "ai/targetedunitpathfindingsystem.h"

#include "game/gameanimation/gameanimationfactory.h"
#include "game/gamemap.h"
#include "game/unit.h"
#include "game/gameaction.h"
#include "game/unitpathfindingsystem.h"
#include "game/player.h"
#include "game/co.h"

#include "menue/gamemenue.h"

#include "coreengine/mainapp.h"
#include "coreengine/console.h"
#include "coreengine/globalutils.h"

#include "resource_management/cospritemanager.h"

#include <QFile>
#include <QSettings>

const QString CoreAI::ACTION_WAIT = "ACTION_WAIT";
const QString CoreAI::ACTION_HOELLIUM_WAIT = "ACTION_HOELLIUM_WAIT";
const QString CoreAI::ACTION_SUPPORTSINGLE = "ACTION_SUPPORTSINGLE";
const QString CoreAI::ACTION_SUPPORTSINGLE_REPAIR = "ACTION_SUPPORTSINGLE_REPAIR";
const QString CoreAI::ACTION_SUPPORTSINGLE_FREEREPAIR = "ACTION_SUPPORTSINGLE_FREEREPAIR";
const QString CoreAI::ACTION_SUPPORTSINGLE_SUPPLY = "ACTION_SUPPORTSINGLE_SUPPLY";
const QString CoreAI::ACTION_SUPPORTALL = "ACTION_SUPPORTALL";
const QString CoreAI::ACTION_SUPPORTALL_RATION = "ACTION_SUPPORTALL_RATION";
const QString CoreAI::ACTION_SUPPORTALL_RATION_MONEY = "ACTION_SUPPORTALL_RATION_MONEY";
const QString CoreAI::ACTION_UNSTEALTH = "ACTION_UNSTEALTH";
const QString CoreAI::ACTION_STEALTH = "ACTION_STEALTH";
const QString CoreAI::ACTION_BUILD_UNITS = "ACTION_BUILD_UNITS";
const QString CoreAI::ACTION_CAPTURE = "ACTION_CAPTURE";
const QString CoreAI::ACTION_MISSILE = "ACTION_MISSILE";
const QString CoreAI::ACTION_PLACE = "ACTION_PLACE";
const QString CoreAI::ACTION_FIRE = "ACTION_FIRE";
const QString CoreAI::ACTION_JOIN = "ACTION_JOIN";
const QString CoreAI::ACTION_UNLOAD = "ACTION_UNLOAD";
const QString CoreAI::ACTION_LOAD = "ACTION_LOAD";
const QString CoreAI::ACTION_NEXT_PLAYER = "ACTION_NEXT_PLAYER";
const QString CoreAI::ACTION_SWAP_COS = "ACTION_SWAP_COS";
const QString CoreAI::ACTION_ACTIVATE_TAGPOWER = "ACTION_ACTIVATE_TAGPOWER";
const QString CoreAI::ACTION_ACTIVATE_POWER_CO_0 = "ACTION_ACTIVATE_POWER_CO_0";
const QString CoreAI::ACTION_ACTIVATE_POWER_CO_1 = "ACTION_ACTIVATE_POWER_CO_1";
const QString CoreAI::ACTION_ACTIVATE_SUPERPOWER_CO_0 = "ACTION_ACTIVATE_SUPERPOWER_CO_0";
const QString CoreAI::ACTION_ACTIVATE_SUPERPOWER_CO_1 = "ACTION_ACTIVATE_SUPERPOWER_CO_1";
const QString CoreAI::ACTION_CO_UNIT_0 = "ACTION_CO_UNIT_0";
const QString CoreAI::ACTION_CO_UNIT_1 = "ACTION_CO_UNIT_1";
const QString CoreAI::ACTION_EXPLODE = "ACTION_EXPLODE";
const QString CoreAI::ACTION_FLARE = "ACTION_FLARE";
const QString CoreAI::ACTION_TRAP = "ACTION_TRAP";

const QString CoreAI::UNIT_INFANTRY = "INFANTRY";

CoreAI::CoreAI(GameMap* pMap, GameEnums::AiTypes aiType)
    : BaseGameInputIF(pMap, aiType)
{
    setObjectName("CoreAI");
    Interpreter::setCppOwnerShip(this);
    Mainapp* pApp = Mainapp::getInstance();
    moveToThread(pApp->getWorkerthread());

    // load static information from scripts to improve performance
    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValue erg = pInterpreter->doFunction(ACTION_FLARE, "getMinRange");
    if (erg.isNumber())
    {
        m_flareInfo.minRange = erg.toNumber();
    }
    erg = pInterpreter->doFunction(ACTION_FLARE, "getMaxRange");
    if (erg.isNumber())
    {
        m_flareInfo.maxRange = erg.toNumber();
    }
    erg = pInterpreter->doFunction(ACTION_FLARE, "getUnfogRange");
    if (erg.isNumber())
    {
        m_flareInfo.unfogRange = erg.toNumber();
    }
}

void CoreAI::init()
{
    connect(GameMenue::getInstance().get(), &GameMenue::sigActionPerformed, this, &CoreAI::nextAction, Qt::QueuedConnection);
    connect(this, &CoreAI::performAction, GameMenue::getInstance().get(), &GameMenue::performAction, Qt::QueuedConnection);

    
    if (m_pMap != nullptr)
    {
        qint32 heigth = m_pMap->getMapHeight();
        qint32 width = m_pMap->getMapWidth();
        for (qint32 x = 0; x < width; x++)
        {
            m_MoveCostMap.push_back(std::vector<std::tuple<qint32, bool>>());
            for (qint32 y = 0; y < heigth; y++)
            {
                m_MoveCostMap[x].push_back(std::tuple<qint32, bool>(0, false));
            }
        }
    }
}

void CoreAI::loadIni(QString file)
{
        AI_CONSOLE_PRINT("CoreAI::loadIni " + file, Console::eDEBUG);
        m_iniFiles.append(file);
        QStringList searchFiles;
        if (!file.isEmpty())
        {
            searchFiles.append(QString(oxygine::Resource::RCC_PREFIX_PATH) + "resources/aidata/" + file);
            searchFiles.append("resources/aidata/" + file);
            // make sure to overwrite existing js stuff
            for (qint32 i = 0; i < Settings::getMods().size(); i++)
            {
                searchFiles.append(QString(oxygine::Resource::RCC_PREFIX_PATH) + Settings::getMods().at(i) + "/aidata/" + file);
                searchFiles.append(Settings::getUserPath() + Settings::getMods().at(i) + "/aidata/" + file);
            }
        }
        for (auto & file : searchFiles)
        {
            if (QFile::exists(file))
            {
                readIni(file);
            }
        }
}

void CoreAI::readIni(QString name)
{
    if (QFile::exists(name))
    {
        QSettings settings(name, QSettings::IniFormat);
        AI_CONSOLE_PRINT("NormalAi::readIni status=" + QString::number(settings.status()), Console::eDEBUG);
        QString lastGroup = "";
        for (auto & entry : m_iniData)
        {
            bool ok = false;
            if (entry.m_group != lastGroup)
            {
                if (!lastGroup.isEmpty())
                {
                    settings.endGroup();
                }
                settings.beginGroup(entry.m_group);
                lastGroup = entry.m_group;
            }
            if (settings.contains(entry.m_name))
            {
                *entry.m_value = settings.value(entry.m_name, entry.m_defaultValue).toDouble(&ok);
                if (!ok)
                {
                    *entry.m_value = entry.m_defaultValue;
                }
            }
        }
        settings.endGroup();
    }
}

void CoreAI::saveIni(QString name) const
{
    QSettings settings(name, QSettings::IniFormat);
    AI_CONSOLE_PRINT("NormalAi::saveIni status=" + QString::number(settings.status()), Console::eDEBUG);
    QString lastGroup = "";
    for (auto & entry : m_iniData)
    {
        bool ok = false;
        if (entry.m_group != lastGroup)
        {
            if (!lastGroup.isEmpty())
            {
                settings.endGroup();
            }
            settings.beginGroup(entry.m_group);
            lastGroup = entry.m_group;
        }
        settings.setValue(entry.m_name, *entry.m_value);
    }
    settings.endGroup();
}

void CoreAI::randomizeIni(QString name, float chance, float mutationRate)
{
    for (auto & entry : m_iniData)
    {
        if (GlobalUtils::randFloat(0.0f, 1.0f) < chance)
        {
            if (qAbs(*entry.m_value) <= 0.05f)
            {
                qint32 rand = GlobalUtils::randInt(-1, 1);
                if (rand == 0)
                {
                    *entry.m_value = 0.0f;
                }
                else if (rand > 0)
                {
                    *entry.m_value = 0.075f;
                }
                else if (rand < 0)
                {
                    *entry.m_value = -0.075f;
                }
            }
            else
            {
                qint32 rand = GlobalUtils::randInt(0, 1);
                if (rand == 0)
                {
                    *entry.m_value -= *entry.m_value * mutationRate;
                }
                else
                {
                    *entry.m_value += *entry.m_value * mutationRate;
                }
            }
        }
        if (*entry.m_value < entry.m_minValue)
        {
            *entry.m_value = entry.m_minValue;
        }
        else if (*entry.m_value > entry.m_maxValue)
        {
            *entry.m_value = entry.m_maxValue;
        }
    }
    saveIni(name);
}

void CoreAI::setInitValue(QString name, double newValue)
{
    for (auto & entry : m_iniData)
    {
        if (entry.m_name == name)
        {
            *entry.m_value = newValue;
        }
    }
}

double CoreAI::getInitValue(QString name) const
{
    for (auto & entry : m_iniData)
    {
        if (entry.m_name == name)
        {
            return *entry.m_value;
        }
    }
    return 0.0;
}

void CoreAI::nextAction()
{
    AI_CONSOLE_PRINT("CoreAI::nextAction", Console::eDEBUG);
    // check if it's our turn
    spGameMenue pMenue = GameMenue::getInstance();
    if (!m_processing)
    {
        m_processing = true;
        if (pMenue.get() != nullptr &&
            m_pMap != nullptr &&
            m_pPlayer == m_pMap->getCurrentPlayer() &&
            pMenue->getGameStarted())
        {

            if (!processPredefinedAi())
            {
                AI_CONSOLE_PRINT("Processing ai specific behaviour", Console::eDEBUG);
                // if so execute next action
                process();
            }
        }
        m_processing = false;
    }
    else
    {
        AI_CONSOLE_PRINT("Unexpected nextAction call", Console::eDEBUG);
    }
}

bool CoreAI::contains(std::vector<QVector3D>& points, const QPoint & point)
{
    for (auto & pos : points)
    {
        if (static_cast<qint32>(pos.x()) == point.x() &&
            static_cast<qint32>(pos.y()) == point.y())
        {
            return true;
        }
    }
    return false;
}

qint32 CoreAI::index(std::vector<QVector3D>& points, const QPoint & point)
{
    for (qint32 i = 0; i < points.size(); i++)
    {
        if (static_cast<qint32>(points[i].x()) == point.x() &&
            static_cast<qint32>(points[i].y()) == point.y())
        {
            return i;
        }
    }
    return -1;
}

bool CoreAI::useCOPower(spQmlVectorUnit & pUnits, spQmlVectorUnit & pEnemyUnits)
{
    AI_CONSOLE_PRINT("CoreAI::useCOPower()", Console::eDEBUG);
    std::vector<float> data;
    data.push_back(-1);
    data.push_back(0);
    data.push_back(-1);
    data.push_back(pUnits->size());
    qint32 repairUnits = 0;
    qint32 indirectUnits = 0;
    qint32 directUnits = 0;
    for (auto pUnit : pUnits->getVector())
    {
        if (pUnit->getHpRounded() < Unit::MAX_UNIT_HP)
        {
            repairUnits++;
        }
        if (pUnit->getBaseMaxRange() > 1)
        {
            indirectUnits++;
        }
        else
        {
            directUnits++;
        }
    }
    data.push_back(repairUnits);
    data.push_back(indirectUnits);
    data.push_back(directUnits);
    data.push_back(pEnemyUnits->size());
    data.push_back(m_pPlayer->getFunds());
    data.push_back(static_cast<float>(turnMode));

    for (quint8 i = 0; i <= 1; i++)
    {
        CO* pCO = m_pPlayer->getCO(i);
        if (pCO != nullptr)
        {
            data[0] = COSpriteManager::getInstance()->getIndex(pCO->getCoID());
            if (pCO->canUseSuperpower())
            {
                data[1] = 2;
            }
            else if (pCO->canUsePower())
            {
                data[1] = 1;
            }
            else
            {
                data[1] = 0;
            }
            data[2] = pCO->getPowerFilled() - pCO->getPowerStars();

            GameEnums::PowerMode result = pCO->getAiUsePower(data[2], pUnits->size(), repairUnits, indirectUnits,
                    directUnits, pEnemyUnits->size(), turnMode);
            if (result == GameEnums::PowerMode_Power)
            {
                spGameAction pAction = spGameAction::create(ACTION_ACTIVATE_POWER_CO_0, m_pMap);
                if (i == 1)
                {
                    pAction->setActionID(ACTION_ACTIVATE_POWER_CO_1);
                }
                if (pAction->canBePerformed())
                {
                    emit performAction(pAction);
                    return true;
                }
            }
            else if (result == GameEnums::PowerMode_Superpower)
            {
                spGameAction pAction = spGameAction::create(ACTION_ACTIVATE_SUPERPOWER_CO_0, m_pMap);
                if (i == 1)
                {
                    pAction->setActionID(ACTION_ACTIVATE_SUPERPOWER_CO_1);
                }
                if (pAction->canBePerformed())
                {
                    pAction->setActionID(ACTION_ACTIVATE_TAGPOWER);
                    if (pAction->canBePerformed())
                    {
                        emit performAction(pAction);
                        return true;
                    }
                    else if (i == 1)
                    {
                        pAction->setActionID(ACTION_ACTIVATE_SUPERPOWER_CO_1);
                    }
                    else
                    {
                        pAction->setActionID(ACTION_ACTIVATE_SUPERPOWER_CO_0);
                    }
                    emit performAction(pAction);
                    return true;
                }
            }
        }
    }
    return false;
}

float CoreAI::calcBuildingDamage(Unit* pUnit, const QPoint & newPosition, Building* pBuilding) const
{
    float counterDamage = 0.0f;
    GameEnums::BuildingTarget targets = pBuilding->getBuildingTargets();
    if (targets == GameEnums::BuildingTarget_All ||
        (targets == GameEnums::BuildingTarget_Enemy && m_pPlayer->isEnemy(pBuilding->getOwner())) ||
        (targets == GameEnums::BuildingTarget_Own && m_pPlayer == pBuilding->getOwner()))
    {
        if (pBuilding->getFireCount() <= 1 &&
            pBuilding->getOwner() != nullptr)
        {
            QPoint pos = newPosition - pBuilding->getActionTargetOffset() - pBuilding->getPosition();
            spQmlVectorPoint pTargets = spQmlVectorPoint(pBuilding->getActionTargetFields());
            if (pTargets.get() != nullptr)
            {
                if (pTargets->contains(pos))
                {
                    float damage = pBuilding->getDamage(pUnit);
                    if (damage > pUnit->getHp())
                    {
                        damage = pBuilding->getHp();
                    }
                    counterDamage = damage / Unit::MAX_UNIT_HP * pUnit->getUnitCosts();
                }
            }
        }
    }
    return counterDamage;
}

void CoreAI::createMovementMap(spQmlVectorBuilding & pBuildings, spQmlVectorBuilding & pEnemyBuildings)
{
    AI_CONSOLE_PRINT("CoreAI::createMovementMap", Console::eDEBUG);
    
    if (m_pMap != nullptr)
    {
        qint32 heigth = m_pMap->getMapHeight();
        qint32 width = m_pMap->getMapWidth();
        for (qint32 x = 0; x < width; x++)
        {
            for (qint32 y = 0; y < heigth; y++)
            {
                if (std::get<1>(m_MoveCostMap[x][y]) == false)
                {
                    std::get<0>(m_MoveCostMap[x][y]) = 0.0f;
                }
            }
        }
        for (auto & pBuilding : pBuildings->getVector())
        {
            float damage = pBuilding->getDamage(nullptr);
            addMovementMap(pBuilding.get(), damage);
        }
        for (auto & pBuilding : pEnemyBuildings->getVector())
        {
            if (pBuilding->getOwner() != nullptr)
            {
                float damage = pBuilding->getDamage(nullptr);
                addMovementMap(pBuilding.get(), damage);
            }
        }
    }
}

void CoreAI::addMovementMap(Building* pBuilding, float damage)
{
    GameEnums::BuildingTarget targets = pBuilding->getBuildingTargets();
    QPoint offset = pBuilding->getPosition() + pBuilding->getActionTargetOffset() ;
    if (targets == GameEnums::BuildingTarget_All ||
        (targets == GameEnums::BuildingTarget_Enemy && m_pPlayer->isEnemy(pBuilding->getOwner())))
    {
        if (pBuilding->getFireCount() <= 1)
        {
            spQmlVectorPoint pTargets = spQmlVectorPoint(pBuilding->getActionTargetFields());
            if (pTargets.get() != nullptr)
            {
                for (auto & target : pTargets->getVector())
                {
                    QPoint point = target + offset;
                    if ((m_MoveCostMap.size() > point.x() && point.x() >= 0) &&
                        (m_MoveCostMap[point.x()].size() > point.y() && point.y() >= 0))
                    {
                        if (std::get<1>(m_MoveCostMap[point.x()][point.y()]) == false)
                        {
                            std::get<0>(m_MoveCostMap[point.x()][point.y()]) += damage;
                        }
                    }
                }
            }
        }
    }
}

void CoreAI::getBestTarget(Unit* pUnit, spGameAction & pAction, UnitPathFindingSystem* pPfs, std::vector<QVector3D>& ret, std::vector<QVector3D>& moveTargetFields, qint32 maxDistance)
{
    pAction->setMovepath(QVector<QPoint>(1, QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())), 0);
    getBestAttacksFromField(pUnit, pAction, ret, moveTargetFields);
    if (pUnit->canMoveAndFire(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())))
    {        
        if (m_pMap != nullptr)
        {
            std::vector<QPoint> targets = pPfs->getAllNodePointsFast(maxDistance);
            for (auto & target : targets)
            {
                if (m_pMap->getTerrain(target.x(), target.y())->getUnit() == nullptr)
                {
                    pAction->setMovepath(QVector<QPoint>(1, target), 0);
                    getBestAttacksFromField(pUnit, pAction, ret, moveTargetFields);
                }
            }
        }
    }
    pAction->setMovepath(QVector<QPoint>(), 0);
}

void CoreAI::getBestAttacksFromField(Unit* pUnit, spGameAction & pAction, std::vector<QVector3D>& ret, std::vector<QVector3D>& moveTargetFields)
{
    
    // much easier case
    // check if the action can be performed here
    if (pAction->canBePerformed())
    {
        spMarkedFieldData pMarkedFieldData = pAction->getMarkedFieldStepData();
        for (auto & target : *pMarkedFieldData->getPoints())
        {
            QRectF damage = calcUnitDamage(pAction, target);
            Terrain* pTerrain = m_pMap->getTerrain(target.x(), target.y());
            Unit* pDef = pTerrain->getUnit();
            if (pDef != nullptr)
            {
                QPointF dmg = calcFundsDamage(damage, pUnit, pDef);
                if (ret.size() == 0)
                {
                    ret.push_back(QVector3D(target.x(), target.y(), dmg.y()));
                    QPoint point = pAction->getActionTarget();
                    moveTargetFields.push_back(QVector3D(point.x(), point.y(), 1));
                }
                else if (static_cast<float>(ret[0].z()) == dmg.y())
                {
                    ret.push_back(QVector3D(target.x(), target.y(), dmg.y()));
                    QPoint point = pAction->getActionTarget();
                    moveTargetFields.push_back(QVector3D(point.x(), point.y(), 1));
                }
                else if (dmg.y() > ret[0].z())
                {
                    ret.clear();
                    moveTargetFields.clear();
                    ret.push_back(QVector3D(target.x(), target.y(), dmg.y()));
                    QPoint point = pAction->getActionTarget();
                    moveTargetFields.push_back(QVector3D(point.x(), point.y(), 1));
                }
            }
            else
            {
                if (isAttackOnTerrainAllowed(pTerrain, damage.x()))
                {
                    if (ret.size() == 0)
                    {
                        ret.push_back(QVector3D(target.x(), target.y(), static_cast<float>(damage.x()) * m_buildingValue));
                        QPoint point = pAction->getActionTarget();
                        moveTargetFields.push_back(QVector3D(point.x(), point.y(), 1));
                    }
                    else if (ret[0].z() == static_cast<float>(damage.x()) * m_buildingValue)
                    {
                        ret.push_back(QVector3D(target.x(), target.y(), static_cast<float>(damage.x()) * m_buildingValue));
                        QPoint point = pAction->getActionTarget();
                        moveTargetFields.push_back(QVector3D(point.x(), point.y(), 1));
                    }
                    else if (static_cast<float>(damage.x()) > ret[0].z())
                    {
                        ret.clear();
                        moveTargetFields.clear();
                        ret.push_back(QVector3D(target.x(), target.y(), static_cast<float>(damage.x()) * m_buildingValue));
                        QPoint point = pAction->getActionTarget();
                        moveTargetFields.push_back(QVector3D(point.x(), point.y(), 1));
                    }
                }
            }
        }
    }
}

void CoreAI::getAttackTargets(Unit* pUnit, spGameAction & pAction, UnitPathFindingSystem* pPfs, std::vector<DamageData>& ret, std::vector<QVector3D>& moveTargetFields, qint32 maxDistance) const
{
    pAction->setMovepath(QVector<QPoint>(1, QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())), 0);
    getAttacksFromField(pUnit, pAction, ret, moveTargetFields);
    if (pUnit->canMoveAndFire(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())))
    {        
        if (m_pMap != nullptr)
        {
            auto targets = pPfs->getAllNodePointsFast(maxDistance);
            for (auto & target : targets)
            {
                Terrain* pTerrain = m_pMap->getTerrain(target.x(), target.y());
                if (pTerrain->getUnit() == nullptr)
                {
                    pAction->setMovepath(QVector<QPoint>(1, target), 0);
                    getAttacksFromField(pUnit, pAction, ret, moveTargetFields);
                }
            }
        }
    }
    pAction->setMovepath(QVector<QPoint>(), 0);
}

void CoreAI::getAttacksFromField(Unit* pUnit, spGameAction & pAction, std::vector<DamageData>& ret, std::vector<QVector3D>& moveTargetFields) const
{
    
    // much easier case
    // check if the action can be performed here
    if (pAction->canBePerformed())
    {
        spMarkedFieldData pMarkedFieldData = pAction->getMarkedFieldStepData();
        const auto & points = *pMarkedFieldData->getPoints();
        for (auto & target : qAsConst(points))
        {
            QRectF damage = calcUnitDamage(pAction, target);
            Terrain* pTerrain = m_pMap->getTerrain(target.x(), target.y());
            Unit* pDef = pTerrain->getUnit();
            if (pDef != nullptr)
            {
                qint32 stealthMalus = 0;
                bool terrainHide = false;
                if (pDef->isStatusStealthedAndInvisible(m_pPlayer, terrainHide))
                {
                    stealthMalus = 4;
                    if (terrainHide)
                    {
                        stealthMalus /= 2;
                    }
                }
                QPointF dmg = calcFundsDamage(damage, pUnit, pDef);
                DamageData data;
                data.x = target.x();
                data.y = target.y();
                data.fundsDamage = dmg.y();
                data.hpDamage = dmg.x();
                ret.push_back(data);
                QPoint point = pAction->getActionTarget();
                moveTargetFields.push_back(QVector3D(point.x(), point.y(), 1 + stealthMalus));
            }
            else
            {
                if (isAttackOnTerrainAllowed(pTerrain, damage.x()))
                {
                    DamageData data;
                    data.x = target.x();
                    data.y = target.y();
                    data.fundsDamage = static_cast<float>(damage.x()) * m_buildingValue;
                    data.hpDamage = damage.x();
                    ret.push_back(data);
                    QPoint point = pAction->getActionTarget();
                    moveTargetFields.push_back(QVector3D(point.x(), point.y(), 1));
                }
            }
        }
    }
}

bool CoreAI::isAttackOnTerrainAllowed(Terrain* pTerrain, float damage) const
{
    if (damage >= m_minTerrainDamage)
    {
        Building* pBuilding = pTerrain->getBuilding();
        if ((m_enableNeutralTerrainAttack && pTerrain->getHp() > 0) ||
            (pBuilding != nullptr && pBuilding->getHp() > 0 && pBuilding->getOwner() != nullptr && pBuilding->getOwner() != m_pPlayer) ||
            (m_enableNeutralTerrainAttack && pBuilding != nullptr && pBuilding->getHp() > 0 && pBuilding->getOwner() == nullptr))
        {
            return true;
        }
    }
    return false;
}

QPointF CoreAI::calcFundsDamage(const QRectF & damage, Unit* pAtk, Unit* pDef) const
{
    float atkDamage = static_cast<float>(damage.x()) / Unit::MAX_UNIT_HP;
    if (atkDamage > pDef->getHp())
    {
        atkDamage = pDef->getHp();
    }
    float fundsDamage = pDef->getUnitCosts() * atkDamage / Unit::MAX_UNIT_HP;
    if (damage.width() >= 0.0)
    {
        float counterDamage = static_cast<float>(damage.width()) / Unit::MAX_UNIT_HP;
        if (counterDamage > pAtk->getHp())
        {
            counterDamage = pAtk->getHp();
        }
        fundsDamage -= pAtk->getUnitCosts() * counterDamage / Unit::MAX_UNIT_HP * m_ownUnitValue;
    }
    return QPointF(atkDamage, fundsDamage);
}

QRectF CoreAI::calcUnitDamage(spGameAction & pAction, const QPoint & target) const
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "calcBattleDamage";
    QJSValueList args({pInterpreter->newQObject(m_pMap),
                       pInterpreter->newQObject(pAction.get()),
                       target.x(),
                       target.y(),
                       static_cast<qint32>(GameEnums::LuckDamageMode_Average)});
    QJSValue erg = pInterpreter->doFunction(ACTION_FIRE, function1, args);
    return erg.toVariant().toRectF();
}

QRectF CoreAI::calcVirtuelUnitDamage(GameMap* pMap,
                                     Unit* pAttacker, float attackerTakenDamage, const QPoint & atkPos, GameEnums::LuckDamageMode luckModeAtk,
                                     Unit* pDefender, float defenderTakenDamage, const QPoint & defPos, GameEnums::LuckDamageMode luckModeDef,
                                     bool ignoreOutOfVisionRange)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "calcBattleDamage3";
    QJSValueList args({pInterpreter->newQObject(pMap),
                       pInterpreter->newQObject(nullptr),
                       pInterpreter->newQObject(pAttacker),
                       attackerTakenDamage,
                       atkPos.x(),
                       atkPos.y(),
                       pInterpreter->newQObject(pDefender),
                       defPos.x(),
                       defPos.y(),
                       defenderTakenDamage,
                       static_cast<qint32>(luckModeAtk),
                       static_cast<qint32>(luckModeDef),
                       ignoreOutOfVisionRange});
    QJSValue erg = pInterpreter->doFunction(ACTION_FIRE, function1, args);
    return erg.toVariant().toRectF();
}

bool CoreAI::moveAwayFromProduction(spQmlVectorUnit & pUnits)
{
    AI_CONSOLE_PRINT("CoreAI::moveAwayFromProduction", Console::eDEBUG);
    m_aiStep = AISteps::moveAway;
    
    for (auto & pUnit : pUnits->getVector())
    {
        Terrain* pTerrain =  pUnit->getTerrain();
        Building* pBuilding = pTerrain->getBuilding();
        // can we use the unit and does it block a production center cause it has nothing to do this turn?
        bool isOnProduction = pBuilding != nullptr &&
                              !m_pPlayer->isEnemy(pBuilding->getOwner()) &&
                              pBuilding->isProductionBuilding();
        qint32 remainingSpace = pUnit->getLoadingPlace() - pUnit->getLoadedUnitCount();
        if (!pUnit->getHasMoved() &&
           (isOnProduction ||
           (pTerrain->isLoadingTile() && remainingSpace == 0)))
        {
            UnitPathFindingSystem turnPfs(m_pMap, pUnit.get());
            turnPfs.explore();
            auto points = turnPfs.getAllNodePointsFast();
            QPoint target(-1 , -1);
            for (auto & point : points)
            {
                Terrain* pNewTerrain = m_pMap->getTerrain(point.x(), point.y());
                if (pNewTerrain->getUnit() == nullptr &&
                    !pNewTerrain->isLoadingTile())
                {
                    if (pNewTerrain->getBuilding() == nullptr)
                    {
                        target = point;
                        break;
                    }
                    else if (!pNewTerrain->getBuilding()->isProductionBuilding())
                    {
                        target = point;
                        break;
                    }
                }
            }
            if (target.x() >= 0 && target.y() >= 0)
            {
                spGameAction pAction = spGameAction::create(ACTION_WAIT, m_pMap);
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                auto path = turnPfs.getPathFast(target.x(), target.y());
                pAction->setMovepath(path, turnPfs.getCosts(path));
                emit performAction(pAction);
                return true;
            }
        }
    }
    return false;
}

void CoreAI::addMenuItemData(spGameAction & pGameAction, const QString & itemID, qint32 cost)
{
    AI_CONSOLE_PRINT("CoreAI::addMenuItemData()", Console::eDEBUG);
    pGameAction->writeDataString(itemID);
    // increase costs and input step
    pGameAction->setCosts(pGameAction->getCosts() + cost);
    pGameAction->setInputStep(pGameAction->getInputStep() + 1);
}

void CoreAI::addSelectedFieldData(spGameAction & pGameAction, const QPoint & point)
{
    AI_CONSOLE_PRINT("CoreAI::addSelectedFieldData()", Console::eDEBUG);
    pGameAction->writeDataInt32(point.x());
    pGameAction->writeDataInt32(point.y());
    pGameAction->setInputStep(pGameAction->getInputStep() + 1);
}

std::vector<Unit*> CoreAI::appendLoadingTargets(Unit* pUnit, spQmlVectorUnit & pUnits,
                                            spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings,
                                            bool addCaptureTargets, bool virtualLoading, std::vector<QVector3D>& targets,
                                            bool all)
{
    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);
    
    qint32 width = m_pMap->getMapWidth();
    qint32 heigth = m_pMap->getMapHeight();
    std::vector<Unit*> transportUnits;
    qint32 transporterMovement = pUnit->getMovementpoints(pUnit->Unit::getPosition());
    for (auto & pUnit : pUnits->getVector())
    {
        Unit* pLoadingUnit = pUnit.get();
        if (pLoadingUnit != pUnit && pLoadingUnit->getHp() > 0.0f)
        {
            // can we transport it?
            if (pUnit->canTransportUnit(pLoadingUnit, virtualLoading))
            {
                bool found = false;
                bool canCapture = pLoadingUnit->getActionList().contains(ACTION_CAPTURE);
                qint32 loadingIslandIdx = getIslandIndex(pLoadingUnit);
                qint32 loadingIsland = getIsland(pLoadingUnit);
                QPoint loadingUnitPos = pLoadingUnit->getPosition();
                if (addCaptureTargets && canCapture)
                {
                    // no targets found -> try to speed up those infis
                    found = hasCaptureTarget(pLoadingUnit, canCapture, pEnemyBuildings,
                                             loadingIslandIdx, loadingIsland);
                }
                else
                {
                    found = hasTargets(transporterMovement, pLoadingUnit, canCapture, pEnemyUnits, pEnemyBuildings,
                                       loadingIslandIdx, loadingIsland);
                }
                if (!found)
                {
                    // no targets for this unit :(
                    found = false;
                    qint32 distance = std::numeric_limits<qint32>::max();
                    qint32 targetX;
                    qint32 targetY;
                    for (qint32 x = 0; x < width; x++)
                    {
                        for (qint32 y = 0; y < heigth; y++)
                        {
                            // can be reached by both units and is empty
                            // and not added yet
                            if ((m_IslandMaps[loadingIslandIdx]->getIsland(x, y) == loadingIsland) &&
                                (m_IslandMaps[unitIslandIdx]->getIsland(x, y) == unitIsland) &&
                                ((m_pMap->getTerrain(x, y)->getUnit() == nullptr) ||
                                 (m_pMap->getTerrain(x, y)->getUnit() == pUnit)))
                            {
                                qint32 dist = GlobalUtils::getDistance(loadingUnitPos, QPoint(x, y));
                                if (dist < distance)
                                {
                                    found = true;
                                    distance = dist;
                                    targetX = x;
                                    targetY = y;
                                    if (all)
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                        if (all && found)
                        {
                            break;
                        }
                    }
                    if (found && (virtualLoading || !GlobalUtils::contains(targets, QVector3D(targetX, targetY, 1))))
                    {
                        targets.push_back(QVector3D(targetX, targetY, 1));
                        transportUnits.push_back(pLoadingUnit);
                        if (!all)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    return transportUnits;
}

bool CoreAI::hasTargets(qint32 transporterMovement, Unit* pLoadingUnit, bool canCapture, spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings,
                        qint32 loadingIslandIdx, qint32 loadingIsland)
{
    bool found = false;
    QPoint unitPos(pLoadingUnit->Unit::getX(), pLoadingUnit->Unit::getY());
    float movementPoints = pLoadingUnit->getMovementpoints(unitPos);
    qint32 minMovementDistance = movementPoints * m_minSameIslandDistance;
    bool fastUnit = movementPoints * m_slowUnitSpeed > transporterMovement;
    // check if we have anything to do here :)
    for (auto & pEnemy : pEnemyUnits->getVector())
    {
        qint32 x = pEnemy->Unit::getX();
        qint32 y = pEnemy->Unit::getY();
        if (GlobalUtils::getDistance(QPoint(x, y), unitPos) <= minMovementDistance ||
            fastUnit)
        {
            if (m_IslandMaps[loadingIslandIdx]->getIsland(x, y) == loadingIsland &&
                pLoadingUnit->isAttackable(pEnemy.get(), true))
            {
                // this unit can do stuff skip it
                found = true;
                break;
            }
        }
    }
    if (!found)
    {
        found = hasCaptureTarget(pLoadingUnit, canCapture, pEnemyBuildings,
                                 loadingIslandIdx, loadingIsland);
    }
    return found;
}

bool CoreAI::hasCaptureTarget(Unit* pLoadingUnit, bool canCapture, spQmlVectorBuilding & pEnemyBuildings,
                              qint32 loadingIslandIdx, qint32 loadingIsland)
{
    bool found = false;
    // check for capturing or missiles next
    if (canCapture)
    {
        QPoint unitPos(pLoadingUnit->Unit::getX(), pLoadingUnit->Unit::getY());
        float movementPoints = pLoadingUnit->getMovementpoints(unitPos);
        qint32 minMovementDistance = movementPoints * m_minSameIslandDistance;
        bool missileTarget = hasMissileTarget();
        for (auto & pBuilding : pEnemyBuildings->getVector())
        {
            qint32 x = pBuilding->Building::getX();
            qint32 y = pBuilding->Building::getY();
            if (GlobalUtils::getDistance(QPoint(x, y), unitPos) <= minMovementDistance)
            {
                if (m_IslandMaps[loadingIslandIdx]->getIsland(x, y) == loadingIsland &&
                    pBuilding->isCaptureOrMissileBuilding(missileTarget))
                {
                    // this unit can do stuff skip it
                    found = true;
                    break;
                }
            }
        }
    }
    return found;
}

void CoreAI::appendSupportTargets(const QStringList & actions, Unit* pCurrentUnit, spQmlVectorUnit & pUnits, spQmlVectorUnit & pEnemyUnits, std::vector<QVector3D>& targets)
{
    spQmlVectorPoint unitFields = spQmlVectorPoint(GlobalUtils::getCircle(1, 1));
    
    for (const auto& action : actions)
    {
        if (action.startsWith(ACTION_SUPPORTSINGLE) ||
            action.startsWith(ACTION_SUPPORTALL))
        {
            for (auto & pUnit : pUnits->getVector())
            {
                if (pUnit != pCurrentUnit)
                {
                    for (auto & field : unitFields->getVector())
                    {
                        if (m_pMap->onMap(pUnit->Unit::getX() + field.x(), pUnit->Unit::getY() + field.y()) &&
                            m_pMap->getTerrain(pUnit->Unit::getX() + field.x(), pUnit->Unit::getY() + field.y())->getUnit() == nullptr)
                        {
                            QVector3D point = QVector3D(pUnit->Unit::getX() + field.x(), pUnit->Unit::getY() + field.y(), 2);
                            if (!GlobalUtils::contains(targets, point))
                            {
                                targets.push_back(point);
                            }
                        }
                    }
                }
            }
        }
        else if (action.startsWith(ACTION_PLACE))
        {
            for (auto & pUnit : pEnemyUnits->getVector())
            {
                for (auto & field : unitFields->getVector())
                {
                    if (m_pMap->onMap(pUnit->Unit::getX() + field.x(), pUnit->Unit::getY() + field.y()) &&
                        m_pMap->getTerrain(pUnit->Unit::getX() + field.x(), pUnit->Unit::getY() + field.y())->getUnit() == nullptr)
                    {
                        QVector3D point = QVector3D(pUnit->Unit::getX() + field.x(), pUnit->Unit::getY() + field.y(), 2);
                        if (!GlobalUtils::contains(targets, point))
                        {
                            targets.push_back(point);
                        }
                    }
                }
            }
        }
    }
}

void CoreAI::appendCaptureTargets(const QStringList & actions, Unit* pUnit, spQmlVectorBuilding & pEnemyBuildings, std::vector<QVector3D>& targets)
{
    if (actions.contains(ACTION_CAPTURE) ||
        actions.contains(ACTION_MISSILE))
    {
        bool missileTarget = hasMissileTarget();
        for (auto & pBuilding : pEnemyBuildings->getVector())
        {
            QPoint point(pBuilding->Building::getX(), pBuilding->Building::getY());
            if (pUnit->canMoveOver(pBuilding->Building::getX(), pBuilding->Building::getY()))
            {
                if (pBuilding->isCaptureOrMissileBuilding(missileTarget) &&
                    pBuilding->getTerrain()->getUnit() == nullptr)
                {
                    targets.push_back(QVector3D(pBuilding->Building::getX(), pBuilding->Building::getY(), 1));
                }
            }
        }
    }
}

void CoreAI::appendAttackTargets(Unit* pUnit, spQmlVectorUnit & pEnemyUnits, std::vector<QVector3D>& targets)
{
    
    qint32 firerange = pUnit->getMaxRange(pUnit->getPosition());
    spQmlVectorPoint pTargetFields = spQmlVectorPoint(GlobalUtils::getCircle(firerange, firerange));
    for (auto & pEnemy : pEnemyUnits->getVector())
    {
        if (pUnit->isAttackable(pEnemy.get(), true))
        {
            for (auto & target : pTargetFields->getVector())
            {
                qint32 x = target.x() + pEnemy->Unit::getX();
                qint32 y = target.y() + pEnemy->Unit::getY();
                if (m_pMap->onMap(x, y) &&
                    m_pMap->getTerrain(x, y)->getUnit() == nullptr)
                {
                    if (pUnit->canMoveOver(x, y))
                    {
                        qint32 stealthMalus = 0;
                        bool terrainHide = false;
                        if (pEnemy->isStatusStealthedAndInvisible(m_pPlayer, terrainHide))
                        {
                            stealthMalus = 4;
                            if (terrainHide)
                            {
                                stealthMalus /= 2;
                            }
                        }
                        QVector3D possibleTarget(x, y, 1 + stealthMalus);
                        if (!GlobalUtils::contains(targets, possibleTarget))
                        {
                            targets.push_back(possibleTarget);
                        }
                    }
                }
            }
        }
    }
}

void CoreAI::appendAttackTargetsIgnoreOwnUnits(Unit* pUnit, spQmlVectorUnit & pEnemyUnits, std::vector<QVector3D>& targets)
{
    
    for (auto & pEnemy : pEnemyUnits->getVector())
    {
        if (pUnit->isAttackable(pEnemy.get(), true))
        {
            qint32 firerange = pUnit->getMaxRange(pUnit->getPosition());
            spQmlVectorPoint pTargetFields = spQmlVectorPoint(GlobalUtils::getCircle(firerange, firerange));
            for (auto & target : pTargetFields->getVector())
            {
                qint32 x = target.x() + pEnemy->Unit::getX();
                qint32 y = target.y() + pEnemy->Unit::getY();
                if (m_pMap->onMap(x, y))
                {
                    Unit* pTargetUnit = m_pMap->getTerrain(x, y)->getUnit();
                    if (pUnit->canMoveOver(x, y) &&
                        pTargetUnit != nullptr &&
                        pTargetUnit->getOwner()->checkAlliance(m_pPlayer) == GameEnums::Alliance_Friend)
                    {
                        qint32 stealthMalus = 0;
                        bool terrainHide = false;
                        if (pEnemy->isStatusStealthedAndInvisible(m_pPlayer, terrainHide))
                        {
                            stealthMalus = 6;
                            if (terrainHide)
                            {
                                stealthMalus /= 2;
                            }
                        }
                        QVector3D possibleTarget(x, y, 4 + stealthMalus);
                        if (!GlobalUtils::contains(targets, possibleTarget))
                        {
                            targets.push_back(possibleTarget);
                        }
                    }
                }
            }
        }
    }
}

void CoreAI::appendRepairTargets(Unit* pUnit, spQmlVectorBuilding & pBuildings, std::vector<QVector3D>& targets)
{
    
    for (auto & pBuilding : pBuildings->getVector())
    {
        QPoint point(pBuilding->Building::getX(), pBuilding->Building::getY());
        if (m_pMap->getTerrain(point.x(), point.y())->getUnit() == nullptr &&
            pBuilding->canRepair(pUnit))
        {
            targets.push_back(QVector3D(pBuilding->Building::getX(), pBuilding->Building::getY(), 1));
        }
    }
}

void CoreAI::appendSupplyTargets(Unit* pUnit, spQmlVectorUnit & pUnits, std::vector<QVector3D>& targets)
{
    for (auto & pSupplyUnit : pUnits->getVector())
    {
        if (pSupplyUnit != pUnit)
        {
            if ((pSupplyUnit->hasAmmo1() && static_cast<float>(pSupplyUnit->getAmmo1()) / static_cast<float>(pSupplyUnit->getMaxAmmo1())  < 0.5f) ||
                (pSupplyUnit->hasAmmo2() && static_cast<float>(pSupplyUnit->getAmmo2()) / static_cast<float>(pSupplyUnit->getMaxAmmo2())  < 0.5f) ||
                (pSupplyUnit->getMaxFuel() > 0 && static_cast<float>(pSupplyUnit->getFuel()) / static_cast<float>(pSupplyUnit->getMaxFuel()) < 0.5f))
            {
                targets.push_back(QVector3D(pSupplyUnit->Unit::getX(), pSupplyUnit->Unit::getY(), 1));
            }
        }
    }
}

void CoreAI::appendTransporterTargets(Unit* pUnit, spQmlVectorUnit & pUnits, std::vector<QVector3D>& targets)
{
    for (auto & pTransporterUnit : pUnits->getVector())
    {
        if (pTransporterUnit != pUnit)
        {
            if (pTransporterUnit->canTransportUnit(pUnit))
            {
                targets.push_back(QVector3D(pTransporterUnit->Unit::getX(), pTransporterUnit->Unit::getY(), 1));
            }
        }
    }
}

void CoreAI::appendCaptureTransporterTargets(Unit* pUnit, spQmlVectorUnit & pUnits, spQmlVectorBuilding & pEnemyBuildings, std::vector<QVector3D>& targets)
{
    if (pUnit == nullptr)
    {
        return;
    }
    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);    
    bool missileTarget = hasMissileTarget();
    for (auto & pTransporterUnit : pUnits->getVector())
    {
        if (pTransporterUnit != pUnit)
        {
            // assuming unit transporter only have space for one unit
            if (pTransporterUnit->canTransportUnit(pUnit) &&
                pTransporterUnit->getLoadingPlace() == 1)
            {
                bool goodTransporter = false;
                // check captures on this island
                qint32 transporterIslandIdx = getIslandIndex(pTransporterUnit.get());
                qint32 transporterIsland = getIsland(pTransporterUnit.get());
                if (!hasCaptureTarget(pUnit, true, pEnemyBuildings, transporterIslandIdx, transporterIsland))
                {
                    for (auto & pEnemyBuilding : pEnemyBuildings->getVector())
                    {
                        qint32 x = pEnemyBuilding->Building::getX();
                        qint32 y = pEnemyBuilding->Building::getY();
                        // check if both can move there on this island
                        // so we avoid loading and unloading a unit recursivly cause we think we need to transport it to another island
                        // eventhough it has something to do here
                        if ((m_IslandMaps[unitIslandIdx]->getIsland(x, y) == unitIsland) &&
                            (m_IslandMaps[transporterIslandIdx]->getIsland(x, y) == transporterIsland) &&
                            (m_pMap->getTerrain(x, y)->getUnit() == nullptr) &&
                            (pEnemyBuilding->isCaptureOrMissileBuilding(missileTarget)))
                        {
                            goodTransporter = true;
                            break;
                        }
                    }
                    if (goodTransporter)
                    {

                        targets.push_back(QVector3D(pTransporterUnit->Unit::getX(), pTransporterUnit->Unit::getY(), 1));
                    }
                }
            }
        }
    }
}

void CoreAI::appendNearestUnloadTargets(Unit* pUnit, spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings, std::vector<QVector3D>& targets)
{
    AI_CONSOLE_PRINT("CoreAI::appendNearestUnloadTargets", Console::eDEBUG);
    std::vector<std::vector<qint32>> checkedIslands;
    std::vector<qint32> loadedUnitIslandIdx;

    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);
    std::vector<Unit*> captureUnits;
    for (auto & pLoadedUnit : pUnit->getLoadedUnits())
    {
        if (pLoadedUnit->getActionList().contains(ACTION_CAPTURE))
        {
            captureUnits.push_back(pLoadedUnit.get());
        }
        loadedUnitIslandIdx.push_back(getIslandIndex(pLoadedUnit.get()));
        checkedIslands.push_back(std::vector<qint32>());
    }
    spQmlVectorPoint pUnloadArea = spQmlVectorPoint(GlobalUtils::getCircle(1, 1));
    // check for enemis
    for (auto & pEnemy : pEnemyUnits->getVector())
    {
        for (qint32 i2 = 0; i2 < pUnit->getLoadedUnitCount(); i2++)
        {
            Unit* pLoadedUnit = pUnit->getLoadedUnit(i2);
            qint32 targetIsland = m_IslandMaps[loadedUnitIslandIdx[i2]]->getIsland(pEnemy->Unit::getX(), pEnemy->Unit::getY());
            // check if we could reach the enemy if we would be on his island
            // and we didn't checked this island yet -> improves the speed
            if (targetIsland >= 0 && !GlobalUtils::contains(checkedIslands[i2], targetIsland))
            {
                // could we beat his ass? -> i mean can we attack him
                // if so this is a great island
                if (pLoadedUnit->isAttackable(pEnemy.get(), true) ||
                    pLoadedUnit->getLoadedUnitCount() > 0)
                {
                    checkIslandForUnloading(pUnit, pLoadedUnit, checkedIslands[i2], unitIslandIdx, unitIsland,
                                            loadedUnitIslandIdx[i2], targetIsland, pUnloadArea.get(), targets);
                }
            }
        }
    }
    // check for capturable buildings
    if (captureUnits.size() > 0)
    {
        bool missileTarget = hasMissileTarget();
        for (auto & pEnemyBuilding : pEnemyBuildings->getVector())
        {
            for (qint32 i2 = 0; i2 < captureUnits.size(); i2++)
            {
                Unit* pLoadedUnit = captureUnits[i2];
                qint32 targetIsland = m_IslandMaps[loadedUnitIslandIdx[i2]]->getIsland(pEnemyBuilding->Building::getX(), pEnemyBuilding->Building::getY());
                // check if we could reach the enemy if we would be on his island
                // and we didn't checked this island yet -> improves the speed
                if (targetIsland >= 0 && !GlobalUtils::contains(checkedIslands[i2], targetIsland))
                {
                    if (pEnemyBuilding->isCaptureOrMissileBuilding(missileTarget))
                    {
                        checkIslandForUnloading(pUnit, pLoadedUnit, checkedIslands[i2], unitIslandIdx, unitIsland,
                                                loadedUnitIslandIdx[i2], targetIsland, pUnloadArea.get(), targets);
                    }
                }
            }
        }
    }
}

bool CoreAI::isUnloadTerrain(Unit* pUnit, Terrain* pTerrain)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValueList args({pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(pTerrain),
                      });
    QJSValue ret = pInterpreter->doFunction(ACTION_UNLOAD, "isUnloadTerrain", args);
    if (ret.isBool())
    {
        return ret.toBool();
    }
    return false;
}

void CoreAI::getBestFlareTarget(Unit* pUnit, spGameAction & pAction, UnitPathFindingSystem* pPfs, QPoint& flareTarget, QPoint& moveTargetField)
{
    flareTarget  = QPoint(-1, -1);
    moveTargetField  = QPoint(-1, -1);
    spQmlVectorPoint pUnfogCircle =  spQmlVectorPoint(GlobalUtils::getCircle(0, m_flareInfo.unfogRange));
    spQmlVectorPoint pTargetCircle = spQmlVectorPoint(GlobalUtils::getCircle(m_flareInfo.minRange, m_flareInfo.maxRange));
    pAction->setMovepath(QVector<QPoint>(1, QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())), 0);
    
    auto targets = pPfs->getAllNodePointsFast();
    qint32 score = std::numeric_limits<qint32>::min();
    for (auto & targetPos : targets)
    {
        auto path = pPfs->getPathFast(targetPos.x(), targetPos.y());
        pAction->setMovepath(path, pPfs->getCosts(path));
        if (pAction->canBePerformed())
        {
            Unit* pFieldUnit = m_pMap->getTerrain(targetPos.x(), targetPos.y())->getUnit();
            if (pFieldUnit == nullptr ||
                pFieldUnit == pUnit)
            {
                for (auto & circlePos : pTargetCircle->getVector())
                {
                    QPoint target = circlePos + targetPos;
                    if (m_pMap->onMap(target.x(), target.y()))
                    {
                        qint32 currentScore = getFlareTargetScore(target, target, pUnfogCircle);
                        if (currentScore > 0)
                        {
                            if (score < currentScore)
                            {
                                score = currentScore;
                                flareTarget  = target;
                                moveTargetField  = targetPos;
                            }
                            else if (score == currentScore && GlobalUtils::randInt(0, 10) > 5)
                            {
                                flareTarget  = target;
                                moveTargetField  = targetPos;
                            }
                        }
                    }
                }
            }
        }
    }
    if (score > 0)
    {
        AI_CONSOLE_PRINT("Found flare target with score: " + QString::number(score), Console::eDEBUG);
    }
    pAction->setMovepath(QVector<QPoint>(), 0);
}

qint32 CoreAI::getFlareTargetScore(const QPoint& moveTarget, const QPoint& flareTarget, const spQmlVectorPoint& pUnfogCircle)
{
    
    qint32 score = 0;
    for (auto & fogField : pUnfogCircle->getVector())
    {
        QPoint target = fogField + flareTarget;
        if (m_pMap->onMap(target.x(), target.y()))
        {
            Unit* pFieldUnit = m_pMap->getTerrain(target.x(), target.y())->getUnit();
            if (pFieldUnit != nullptr && m_pPlayer->isEnemyUnit(pFieldUnit))
            {
                if (pFieldUnit->isStealthed(m_pPlayer))
                {
                    // unstealthing stealthed units is more important.
                    // than unfogging plain fields
                    if (pFieldUnit->isStatusStealthed() ||
                        pFieldUnit->hasTerrainHide(m_pPlayer))
                    {
                        score += 2;
                    }
                    else
                    {
                        score++;
                    }
                }
            }
        }
    }
    if (score > 0)
    {
        // the farther away from the flare the better it is usually
        score += GlobalUtils::getDistance(moveTarget, flareTarget) * pUnfogCircle->size() * 3;
    }
    else
    {
        score = std::numeric_limits<qint32>::min();
    }
    return score;
}

void CoreAI::checkIslandForUnloading(Unit* pUnit, Unit* pLoadedUnit, std::vector<qint32>& checkedIslands,
                                     qint32 unitIslandIdx, qint32 unitIsland,
                                     qint32 loadedUnitIslandIdx, qint32 targetIsland,
                                     QmlVectorPoint* pUnloadArea, std::vector<QVector3D>& targets)
{
    
    qint32 width = m_pMap->getMapWidth();
    qint32 heigth = m_pMap->getMapHeight();
    checkedIslands.push_back(loadedUnitIslandIdx);
    for (qint32 x = 0; x < width; x++)
    {
        for (qint32 y = 0; y < heigth; y++)
        {
            // check if this is the same island as we search for
            // check if it's the same island our transporter is on if so we can reach the field
            // the unloading area is also free
            if (m_IslandMaps[loadedUnitIslandIdx]->getIsland(x, y) == targetIsland &&
                m_IslandMaps[unitIslandIdx]->getIsland(x, y) == unitIsland &&
                (m_pMap->getTerrain(x, y)->getUnit() == nullptr ||
                 m_pMap->getTerrain(x, y)->getUnit() == pUnit))
            {
                if (isUnloadTerrain(pUnit, m_pMap->getTerrain(x, y)))
                {
                    // and on top of that we have same free fields to unload the unit
                    for (auto & unloadPos : pUnloadArea->getVector())
                    {
                        qint32 unloadX = x + unloadPos.x();
                        qint32 unloadY = y + unloadPos.y();
                        if (m_pMap->onMap(unloadX, unloadY) &&
                            m_pMap->getTerrain(unloadX, unloadY)->getUnit() == nullptr &&
                            pLoadedUnit->getBaseMovementCosts(unloadX, unloadY, unloadX, unloadY) > 0 &&
                            !GlobalUtils::contains(targets, QVector3D(x, y, 1)) &&
                            pUnit->getBaseMovementCosts(x, y, x, y) > 0)
                        {
                            targets.push_back(QVector3D(x, y, 1));
                            break;
                        }
                    }
                }
            }
        }
    }
}

void CoreAI::appendUnloadTargetsForCapturing(Unit* pUnit, spQmlVectorBuilding & pEnemyBuildings, std::vector<QVector3D>& targets)
{
    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);

    std::vector<Unit*> capturUnits;
    for (auto & pLoadedUnit : pUnit->getLoadedUnits())
    {
        if (pLoadedUnit->getActionList().contains(ACTION_CAPTURE))
        {
            capturUnits.push_back(pLoadedUnit.get());
        }
    }
    if (capturUnits.size() > 0)
    {
        spQmlVectorPoint pUnloadArea = spQmlVectorPoint(GlobalUtils::getCircle(1, 1));
        GameAction testAction(m_pMap);
        testAction.setTargetUnit(capturUnits[0]);
        // store has moved
        bool hasMoved = capturUnits[0]->getHasMoved();
        // simulate a not moved unit for checking if we can capture the building or fire a missile from it.
        capturUnits[0]->setHasMoved(false);
        bool missileTarget = hasMissileTarget();
        for (auto & pBuilding : pEnemyBuildings->getVector())
        {
            QPoint point(pBuilding->Building::getX(), pBuilding->Building::getY());
            if (capturUnits[0]->canMoveOver(point.x(), point.y()))
            {
                // we can capture it :)
                if (pBuilding->isCaptureOrMissileBuilding(missileTarget) &&
                    pBuilding->getTerrain()->getUnit() == nullptr)
                {
                    // check unload fields
                    for (auto & unloadPos : pUnloadArea->getVector())
                    {
                        qint32 x = point.x() + unloadPos.x();
                        qint32 y = point.y() + unloadPos.y();
                        if (m_pMap->onMap(x, y) &&
                            m_pMap->getTerrain(x, y)->getUnit() == nullptr &&
                            !GlobalUtils::contains(targets, QVector3D(x, y, 1)))
                        {
                            if (isUnloadTerrain(pUnit, m_pMap->getTerrain(x, y)))
                            {
                                // we can reach this unload field?
                                if (m_IslandMaps[unitIslandIdx]->getIsland(x, y) == unitIsland)
                                {
                                    for (auto & captureUnit : capturUnits)
                                    {
                                        if (captureUnit->canMoveOver(x, y))
                                        {
                                            targets.push_back(QVector3D(x, y, 1));
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        capturUnits[0]->setHasMoved(hasMoved);
    }
}

void CoreAI::appendUnloadTargetsForAttacking(Unit* pUnit, spQmlVectorUnit & pEnemyUnits, std::vector<QVector3D>& targets, qint32 rangeMultiplier)
{
    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);
    std::vector<Unit*> attackUnits;
    qint32 averageMovepoints = 0;
    for (auto & pLoadedUnit : pUnit->getLoadedUnits())
    {
        if (pLoadedUnit->getActionList().contains(ACTION_FIRE))
        {
            attackUnits.push_back(pLoadedUnit.get());
            averageMovepoints += pLoadedUnit->getBaseMovementPoints();
        }
    }
    if (attackUnits.size() > 0)
    {
        averageMovepoints = averageMovepoints / attackUnits.size();
        spQmlVectorPoint pUnloadArea = spQmlVectorPoint(GlobalUtils::getCircle(1, 1));
        spQmlVectorPoint range = spQmlVectorPoint(GlobalUtils::getCircle(1, averageMovepoints * rangeMultiplier + 1));
        for (auto & pEnemyUnit : pEnemyUnits->getVector())
        {
            qint32 enemyX = pEnemyUnit->Unit::getX();
            qint32 enemyY = pEnemyUnit->Unit::getY();
            std::vector<Unit*> attackingUnits;
            for (auto & pAttacker : attackUnits)
            {
                if (pAttacker->getBaseDamage(pEnemyUnit.get()) > 0)
                {
                    attackingUnits.push_back(pAttacker);
                }
            }
            if (attackingUnits.size() > 0)
            {
                for (auto & rangePos : range->getVector())
                {
                    for (auto & unloadPos : pUnloadArea->getVector())
                    {
                        qint32 x = enemyX + rangePos.x() + unloadPos.x();
                        qint32 y = enemyY + rangePos.y() + unloadPos.y();
                        if (m_pMap->onMap(x, y) &&
                            m_pMap->getTerrain(x, y)->getUnit() == nullptr &&
                            !GlobalUtils::contains(targets, QVector3D(x, y, 1)))
                        {
                            if (isUnloadTerrain(pUnit, m_pMap->getTerrain(x, y)))
                            {
                                // we can reach this unload field?
                                if (m_IslandMaps[unitIslandIdx]->getIsland(x, y) == unitIsland)
                                {
                                    for (auto & pAttacker : attackingUnits)
                                    {
                                        qint32 attackerIslandIdx = getIslandIndex(pAttacker);
                                        if (pAttacker->canMoveOver(x, y) &&
                                            m_IslandMaps[attackerIslandIdx]->getIsland(x, y) ==
                                            m_IslandMaps[attackerIslandIdx]->getIsland(enemyX, enemyY))
                                        {
                                            targets.push_back(QVector3D(x, y, 1));
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void CoreAI::appendTerrainBuildingAttackTargets(Unit* pUnit, spQmlVectorBuilding & pEnemyBuildings, std::vector<QVector3D>& targets)
{
    
    qint32 firerange = pUnit->getMaxRange(pUnit->getPosition());
    spQmlVectorPoint pTargetFields = spQmlVectorPoint(GlobalUtils::getCircle(firerange, firerange));
    for (auto & pBuilding : pEnemyBuildings->getVector())
    {
        if (isAttackOnTerrainAllowed(pBuilding->getTerrain(), pUnit->getEnvironmentDamage(pBuilding->getBuildingID())))
        {
            if (pBuilding->getHp() > 0 &&
                pUnit->isEnvironmentAttackable(pBuilding->getBuildingID()))
            {
                qint32 width = pBuilding->getBuildingWidth();
                qint32 heigth = pBuilding->getBuildingHeigth();
                QPoint pos = pBuilding->getPosition();
                std::vector<QPoint> attackPosition;
                // find all attackable fields
                for (qint32 x = -width; x <= 0; x++)
                {
                    for (qint32 y = -heigth; y <= 0; y++)
                    {
                        if (pBuilding->getIsAttackable(x + pos.x(), y + pos.y()))
                        {
                            attackPosition.push_back(QPoint(pos.x() + x, pos.y() + y));
                        }
                    }
                }
                // find attackable fields
                for (auto & target : pTargetFields->getVector())
                {
                    for (auto & attackPos : attackPosition)
                    {
                        qint32 x = target.x() + attackPos.x();
                        qint32 y = target.y() + attackPos.y();
                        if (m_pMap->onMap(x, y) &&
                            m_pMap->getTerrain(x, y)->getUnit() == nullptr)
                        {
                            if (pUnit->canMoveOver(x, y))
                            {
                                QVector3D possibleTarget(x, y, 2);
                                if (!GlobalUtils::contains(targets, possibleTarget))
                                {
                                    targets.push_back(possibleTarget);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (m_enableNeutralTerrainAttack)
    {
        // find terrains
        qint32 width = m_pMap->getMapWidth();
        qint32 heigth = m_pMap->getMapHeight();
        for (qint32 x = 0; x < width; x++)
        {
            for (qint32 y = 0; y < heigth; y++)
            {
                Terrain* pTerrain = m_pMap->getTerrain(x, y);
                if (isAttackOnTerrainAllowed(pTerrain, pUnit->getEnvironmentDamage(pTerrain->getID())))
                {
                    for (auto & target : pTargetFields->getVector())
                    {
                        qint32 x1 = target.x() + x;
                        qint32 y1 = target.y() + y;
                        if (m_pMap->onMap(x1, y1) &&
                            m_pMap->getTerrain(x1, y1)->getUnit() == nullptr)
                        {
                            if (pUnit->canMoveOver(x1, y1))
                            {
                                QVector3D possibleTarget(x1, y1, 2);
                                if (!GlobalUtils::contains(targets, possibleTarget))
                                {
                                    targets.push_back(possibleTarget);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void CoreAI::rebuildIsland(spQmlVectorUnit & pUnits)
{
    AI_CONSOLE_PRINT("CoreAI::rebuildIsland", Console::eDEBUG);
    // and create one
    for (auto & pUnit : pUnits->getVector())
    {
        if (getIslandIndex(pUnit.get()) < 0)
        {
            createIslandMap(pUnit->getMovementType(), pUnit->getUnitID());
        }
        for (auto & pLoadedUnit : pUnit->getLoadedUnits())
        {
            if (getIslandIndex(pLoadedUnit.get()) < 0)
            {
                createIslandMap(pLoadedUnit->getMovementType(), pLoadedUnit->getUnitID());
            }
        }
    }
}


void CoreAI::sortUnitsFarFromEnemyFirst(std::vector<MoveUnitData> & pUnits, spQmlVectorUnit & pEnemyUnits)
{
    auto & pEnemyUnitsVec = pEnemyUnits->getVector();
    for (auto & pUnitData : pUnits)
    {
        qint32 distance = std::numeric_limits<qint32>::max();
        for (const auto & pEnemy : qAsConst(pEnemyUnitsVec))
        {
            qint32 newDistance = GlobalUtils::getDistance(pEnemy->getPosition(), pUnitData.pUnit->getPosition());
            if (newDistance < distance)
            {
                distance = newDistance;
            }
        }
        pUnitData.distanceToEnemy = distance;
        pUnitData.canCapture = pUnitData.actions.contains(CoreAI::ACTION_CAPTURE);
    }
    std::sort(pUnits.begin(), pUnits.end(), [](const MoveUnitData& lhs, const MoveUnitData& rhs)
    {
        if (lhs.canCapture == rhs.canCapture)
        {
            if (lhs.distanceToEnemy == rhs.distanceToEnemy)
            {
                return lhs.movementPoints < rhs.movementPoints;
            }
            else
            {
                return lhs.distanceToEnemy > rhs.distanceToEnemy;
            }
        }
        else
        {
            return lhs.canCapture < rhs.canCapture;
        }
    });
}

bool CoreAI::needsRefuel(const Unit *pUnit) const
{
    if (pUnit->getMaxFuel() > 0 &&
        pUnit->getFuel() / static_cast<float>(pUnit->getMaxFuel()) < m_fuelResupply)
    {
        return true;
    }
    if (pUnit->getMaxAmmo1() > 0 &&
        pUnit->getAmmo1() / static_cast<float>(pUnit->getMaxAmmo1()) < m_ammoResupply &&
        !pUnit->getWeapon1ID().isEmpty())
    {
        return true;
    }
    if (pUnit->getMaxAmmo2() > 0 &&
        pUnit->getAmmo2() / static_cast<float>(pUnit->getMaxAmmo2()) < m_ammoResupply &&
        !pUnit->getWeapon2ID().isEmpty())
    {
        return true;
    }
    return false;
}

bool CoreAI::isRefuelUnit(const QStringList & actionList) const
{
    return actionList.contains(ACTION_SUPPORTALL_RATION) ||
           actionList.contains(ACTION_SUPPORTALL_RATION_MONEY) ||
           actionList.contains(ACTION_SUPPORTSINGLE_FREEREPAIR) ||
           actionList.contains(ACTION_SUPPORTSINGLE_REPAIR) ||
           actionList.contains(ACTION_SUPPORTSINGLE_SUPPLY);
}

void CoreAI::createIslandMap(const QString & movementType, const QString & unitID)
{
    bool found = false;

    for (auto & island : m_IslandMaps)
    {
        if (island->getMovementType() == movementType)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        m_IslandMaps.push_back(spIslandMap::create(m_pMap, unitID, m_pPlayer));
    }
}

bool CoreAI::onSameIsland(Unit* pUnit1, Unit* pUnit2) const
{
    for (auto & island : m_IslandMaps)
    {
        if (island->getMovementType() == pUnit1->getMovementType())
        {
            if (island->getIsland(pUnit1->Unit::getX(), pUnit1->Unit::getY()) ==
                island->getIsland(pUnit2->Unit::getX(), pUnit2->Unit::getY()))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool CoreAI::onSameIsland(Unit* pUnit1, Building* pBuilding) const
{
    for (auto i = 0; i < m_IslandMaps.size(); i++)
    {
        if (m_IslandMaps[i]->getMovementType() == pUnit1->getMovementType())
        {
            if (m_IslandMaps[i]->getIsland(pUnit1->Unit::getX(), pUnit1->Unit::getY()) ==
                m_IslandMaps[i]->getIsland(pBuilding->Building::getX(), pBuilding->Building::getY()))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool CoreAI::onSameIsland(const QString & movemnetType, qint32 x, qint32 y, qint32 x1, qint32 y1) const
{
    for (auto i = 0; i < m_IslandMaps.size(); i++)
    {
        if (m_IslandMaps[i]->getMovementType() == movemnetType)
        {
            return onSameIsland(i, x, y, x1, y1);
        }
    }
    return false;
}

bool CoreAI::onSameIsland(qint32 islandIdx, qint32 x, qint32 y, qint32 x1, qint32 y1) const
{
    return m_IslandMaps[islandIdx]->sameIsland(x, y, x1, y1);
}

qint32 CoreAI::getIsland(Unit* pUnit) const
{
    for (auto & island : m_IslandMaps)
    {
        if (island->getMovementType() == pUnit->getMovementType())
        {
            return island->getIsland(pUnit->Unit::getX(), pUnit->Unit::getY());
        }
    }
    return -1;
}

qint32 CoreAI::getIslandIndex(Unit* pUnit) const
{
    for (qint32 i = 0; i < m_IslandMaps.size(); i++)
    {
        if (m_IslandMaps[i]->getMovementType() == pUnit->getMovementType())
        {
            return i;
        }
    }
    return -1;
}

void CoreAI::finishTurn()
{
    AI_CONSOLE_PRINT("CoreAI::finishTurn(()", Console::eDEBUG);
    m_usedTransportSystem = false;
    spGameAction pAction = spGameAction::create(ACTION_NEXT_PLAYER, m_pMap);
    CO* pCO0 = m_pPlayer->getCO(0);
    CO* pCO1 = m_pPlayer->getCO(1);
    if (pCO0 != nullptr &&
        pCO0->getPowerMode() == GameEnums::PowerMode_Off)
    {
        pAction->setActionID(ACTION_SWAP_COS);
        if (pAction->canBePerformed())
        {
            float remainingC0 = pCO0->getPowerStars() + pCO0->getSuperpowerStars() - pCO0->getPowerFilled();
            float remainingC1 = pCO1->getPowerStars() + pCO1->getSuperpowerStars() - pCO1->getPowerFilled();
            if (remainingC1 - 0.1f > remainingC0)
            {
                pAction->setActionID(ACTION_SWAP_COS);
            }
        }
        else
        {
            pAction->setActionID(ACTION_NEXT_PLAYER);
        }
    }
    else if (pCO1 != nullptr && pCO0 != nullptr &&
             pCO0->getPowerMode() == GameEnums::PowerMode_Tagpower &&
             pCO1->getPowerMode() == GameEnums::PowerMode_Off)
    {
        pAction->setActionID(ACTION_SWAP_COS);
    }
    else
    {
        pAction->setActionID(ACTION_NEXT_PLAYER);
    }
    emit performAction(pAction);
}

bool CoreAI::useBuilding(spQmlVectorBuilding & pBuildings)
{
    AI_CONSOLE_PRINT("CoreAI::useBuilding", Console::eDEBUG);
    
    for (auto & pBuilding : pBuildings->getVector())
    {
        QStringList actions = pBuilding->getActionList();
        if (actions.size() >= 1 &&
            !actions[0].isEmpty())
        {
            for (auto & action : actions)
            {
                if (action != ACTION_BUILD_UNITS)
                {
                    spGameAction pAction = spGameAction::create(action, m_pMap);
                    pAction->setTarget(QPoint(pBuilding->Building::getX(), pBuilding->Building::getY()));
                    if (pAction->canBePerformed())
                    {
                        if (pAction->isFinalStep())
                        {
                            emit performAction(pAction);
                            return true;
                        }
                        else
                        {
                            while (!pAction->isFinalStep())
                            {
                                if (pAction->getStepInputType() == "FIELD")
                                {
                                    spMarkedFieldData pData = pAction->getMarkedFieldStepData();
                                    QVector<QPoint> & points = *pData->getPoints();
                                    qint32 index = -1;
                                    QPoint target;
                                    qint32 maxValue = std::numeric_limits<qint32>::lowest();
                                    for (qint32 i2 = 0; i2 < points.size(); ++i2)
                                    {
                                        Unit* pUnit = m_pMap->getTerrain(points[i2].x(), points[i2].y())->getUnit();
                                        qint32 unitValue = pUnit->getCoUnitValue();
                                        if (pUnit != nullptr && unitValue > maxValue)
                                        {
                                            maxValue = unitValue;
                                            index = i2;
                                        }
                                    }
                                    if (index < 0)
                                    {
                                        target = points.at(GlobalUtils::randIntBase(0, points.size() -1));
                                    }
                                    else
                                    {
                                        target = points.at(index);
                                    }
                                    addSelectedFieldData(pAction, target);
                                }
                                else if (pAction->getStepInputType() == "MENU")
                                {
                                    spMenuData pData = pAction->getMenuStepData();
                                    if (pData->validData())
                                    {
                                        QStringList items = pData->getActionIDs();
                                        auto enable = pData->getEnabledList();
                                        qint32 i = 0;
                                        while (i < enable.size())
                                        {
                                            if (enable[i])
                                            {
                                                i++;
                                            }
                                            else
                                            {
                                                items.removeAt(i);
                                                enable.removeAt(i);
                                            }
                                        }
                                        qint32 selection = GlobalUtils::randIntBase(0, items.size() - 1);
                                        addMenuItemData(pAction, items[selection], pData->getCostList()[selection]);
                                    }
                                }
                                else
                                {
                                    break;
                                }
                            }
                            if (pAction->isFinalStep())
                            {
                                emit performAction(pAction);
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

float CoreAI::getAiCoUnitMultiplier(CO* pCO, Unit* pUnit)
{
    bool valid = false;
    float multiplier = 0.0f;
    if (pCO != nullptr)
    {
        multiplier = pCO->getAiCoUnitBonus(pUnit, valid);
        if (!valid)
        {
            if (pCO->getOffensiveBonus(nullptr, pUnit, QPoint(-1, -1), nullptr, QPoint(-1, -1), false, GameEnums::LuckDamageMode_Off) > 0 ||
                pCO->getDeffensiveBonus(nullptr, nullptr, QPoint(-1, -1), pUnit, QPoint(-1, -1), false, GameEnums::LuckDamageMode_Off) > 0 ||
                pCO->getFirerangeModifier(pUnit, QPoint(-1, -1)) > 0)
            {
                multiplier = 1.0f;
            }
        }
    }
    return multiplier;
}

float CoreAI::getAiCoBuildRatioModifier()
{
    float multiplier = 1.0f;
    for (qint32 i = 0; i < m_pPlayer->getMaxCoCount(); ++i)
    {
        CO* pCO = m_pPlayer->getCO(i);
        if (pCO != nullptr)
        {
            multiplier *= pCO->getAiCoBuildRatioModifier();
        }
    }
    return multiplier;
}

void CoreAI::GetOwnUnitCounts(std::vector<MoveUnitData> & units, spQmlVectorUnit & pOwnUnits, spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings,
                              UnitCountData & countData)
{
    for (auto & unitData : units)
    {
        Unit* pUnit = unitData.pUnit.get();
        if (unitData.actions.contains(ACTION_CAPTURE))
        {
            countData.infantryUnits++;
        }
        else if (pUnit->hasWeapons())
        {
            if (pUnit->getBaseMaxRange() > 1)
            {
                countData.indirectUnits++;
            }
            else
            {
                countData.directUnits++;
            }
        }
        if (needsRefuel(pUnit))
        {
            countData.supplyNeededUnits++;
        }
        if (isRefuelUnit(unitData.actions))
        {
            countData.supplyUnits++;
        }
        if (pUnit->getLoadingPlace() > 0)
        {
            countData.transporterUnits++;
            std::vector<QVector3D> ret;
            std::vector<Unit*> transportUnits = appendLoadingTargets(pUnit, pOwnUnits, pEnemyUnits, pEnemyBuildings, false, true, ret, true);
            for (auto & pTransporter : transportUnits)
            {
                countData.transportTargets.push_back(std::tuple<Unit*, Unit*>(pUnit, pTransporter));
            }
        }
    }
}

bool CoreAI::buildCOUnit(spQmlVectorUnit & pUnits)
{
    AI_CONSOLE_PRINT("CoreAI::buildCOUnit", Console::eDEBUG);
    spGameAction pAction = spGameAction::create(m_pMap);
    for (quint8 i2 = 0; i2 < 2; i2++)
    {
        if (i2 == 0)
        {
            pAction->setActionID(ACTION_CO_UNIT_0);
        }
        else
        {
            pAction->setActionID(ACTION_CO_UNIT_1);
        }
        CO* pCO = m_pPlayer->getCO(i2);
        qint32 bestScore = 0;
        qint32 unitIdx = -1;
        if (pCO != nullptr &&
            pCO->getCOUnit() == nullptr)
        {
            qint32 active = 0;
            bool expensive = false;
            for (qint32 i = 0; i < pUnits->size(); i++)
            {
                Unit* pUnit = pUnits->at(i);
                if (pUnit->getUnitValue() >= m_coUnitValue && pUnit->getUnitRank() >= GameEnums::UnitRank_None)
                {
                    active++;
                }
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                if (pAction->canBePerformed())
                {
                    if (!pUnit->getHasMoved())
                    {
                        if (pUnit->hasWeapons())
                        {
                            qint32 score = 0;
                            score += pUnit->getUnitValue() * m_coUnitScoreMultiplier * getAiCoUnitMultiplier(pCO, pUnit);
                            if (pUnit->getUnitValue() >= m_coUnitValue)
                            {
                                expensive = true;
                            }
                            score -= m_coUnitRankReduction * pUnit->getUnitRank();
                            if (score > bestScore)
                            {
                                bestScore = score;
                                unitIdx = i;
                            }
                        }
                    }
                }
            }
            if (unitIdx >= 0 && bestScore > m_minCoUnitScore && (active > m_minCoUnitCount || expensive))
            {
                Unit* pUnit = pUnits->at(unitIdx);
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                emit performAction(pAction);
                return true;
            }
        }
    }
    return false;
}

bool CoreAI::canTransportToEnemy(Unit* pUnit, Unit* pLoadedUnit, spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings)
{
    spQmlVectorPoint pUnloadArea = spQmlVectorPoint(GlobalUtils::getCircle(1, 1));
    // check for enemis
    qint32 loadedUnitIslandIdx = getIslandIndex(pLoadedUnit);
    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);
    std::vector<qint32> checkedIslands;
    std::vector<QVector3D> targets;
    for (auto & pEnemy : pEnemyUnits->getVector())
    {
        qint32 targetIsland = m_IslandMaps[loadedUnitIslandIdx]->getIsland(pEnemy->Unit::getX(), pEnemy->Unit::getY());
        // check if we could reach the enemy if we would be on his island
        // and we didn't checked this island yet -> improves the speed
        if (targetIsland >= 0 )
        {
            // could we beat his ass? -> i mean can we attack him
            // if so this is a great island
            if (pLoadedUnit->isAttackable(pEnemy.get(), true))
            {
                checkIslandForUnloading(pUnit, pLoadedUnit, checkedIslands, unitIslandIdx, unitIsland,
                                        loadedUnitIslandIdx, targetIsland, pUnloadArea.get(), targets);
                if (targets.size() > 0)
                {
                    break;
                }
            }
        }
    }
    // check for capturable buildings
    if (pLoadedUnit->getActionList().contains(ACTION_CAPTURE) && targets.size() == 0)
    {
        bool missileTarget = hasMissileTarget();
        for (auto & pEnemyBuilding : pEnemyBuildings->getVector())
        {
            qint32 targetIsland = m_IslandMaps[loadedUnitIslandIdx]->getIsland(pEnemyBuilding->Building::getX(), pEnemyBuilding->Building::getY());
            // check if we could reach the enemy if we would be on his island
            // and we didn't checked this island yet -> improves the speed
            if (targetIsland >= 0 )
            {
                if (pEnemyBuilding->isCaptureOrMissileBuilding(missileTarget))
                {
                    checkIslandForUnloading(pUnit, pLoadedUnit, checkedIslands, unitIslandIdx, unitIsland,
                                            loadedUnitIslandIdx, targetIsland, pUnloadArea.get(), targets);
                    if (targets.size() > 0)
                    {
                        break;
                    }
                }
            }
        }
    }
    if (targets.size() > 0)
    {
        return true;
    }
    return false;
}

bool CoreAI::isMoveableTile(Building* pBuilding) const
{
    return pBuilding == nullptr || pBuilding->getOwner() == nullptr ||
            pBuilding->getOwner()->isEnemy(m_pPlayer) ||
            !pBuilding->isProductionBuilding();
}

void CoreAI::serializeObject(QDataStream& stream) const
{    
    AI_CONSOLE_PRINT("storing core ai", Console::eDEBUG);
    stream << getVersion();
    stream << m_enableNeutralTerrainAttack;
    stream << static_cast<qint32>(m_BuildingChanceModifier.size());
    auto keys = m_BuildingChanceModifier.keys();
    for (auto & key : keys)
    {
        stream << key;
        stream << m_BuildingChanceModifier[key];
    }
    stream << static_cast<qint32>(m_MoveCostMap.size());
    for (auto & moveCostMap : m_MoveCostMap)
    {
        stream << static_cast<qint32>(moveCostMap.size());
        for (auto & costs : moveCostMap)
        {
            stream << std::get<0>(costs);
            stream << std::get<1>(costs);
        }
    }
    stream << static_cast<qint32>(m_iniFiles.size());
    for (auto & iniFile : m_iniFiles)
    {
        stream << iniFile;
    }
    stream << static_cast<qint32>(m_iniData.size());
    for (const auto & item : qAsConst(m_iniData))
    {
        stream << item.m_name;
        stream << *item.m_value;
    }
}

void CoreAI::deserializeObject(QDataStream& stream)
{
    AI_CONSOLE_PRINT("reading core ai", Console::eDEBUG);
    qint32 version;
    stream >> version;
    deserializeObjectVersion(stream, version);
}

void CoreAI::deserializeObjectVersion(QDataStream &stream, qint32 version)
{
    if (version > 1)
    {
        stream >> m_enableNeutralTerrainAttack;
    }
    if (version > 2)
    {
        qint32 size = 0;
        stream >> size;
        m_BuildingChanceModifier.clear();
        for (qint32 i = 0; i < size; i++)
        {
            QString unitID;
            float value = 1.0f;
            stream >> unitID;
            stream >> value;
            m_BuildingChanceModifier.insert(unitID, value);
        }
    }
    if (version > 3)
    {
        qint32 size = 0;
        stream >> size;
        m_MoveCostMap.clear();
        for (qint32 x = 0; x < size; x++)
        {
            qint32 ysize = 0;
            stream >> ysize;
            m_MoveCostMap.push_back(std::vector<std::tuple<qint32, bool>>());
            for (qint32 y = 0; y < ysize; y++)
            {
                m_MoveCostMap[x].push_back(std::tuple<qint32, bool>(0, false));
                stream >> std::get<0>(m_MoveCostMap[x][y]);
                stream >> std::get<1>(m_MoveCostMap[x][y]);
            }
        }
    }
    if (version > 4)
    {
        m_iniFiles.clear();
        qint32 size = 0;
        stream >> size;
        for (qint32 i = 0; i < size; i++)
        {
            QString list;
            stream >> list;
            loadIni(list);
        }
    }
    if (version > 5)
    {
        qint32 size = 0;
        stream >> size;
        for (qint32 i = 0; i < size; ++i)
        {
            QString name;
            stream >> name;
            double value;
            stream >> value;
            for (auto & item : m_iniData)
            {
                if (item.m_name == name)
                {
                    *item.m_value = value;
                    break;
                }
            }
        }
    }
    if (version == 7)
    {
        QString dummy;
        stream >> dummy;
    }
}
