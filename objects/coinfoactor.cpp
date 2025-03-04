#include "coinfoactor.h"

#include "coreengine/mainapp.h"

#include "resource_management/cospritemanager.h"
#include "resource_management/objectmanager.h"
#include "resource_management/fontmanager.h"
#include "resource_management/cospritemanager.h"
#include "resource_management/unitspritemanager.h"
#include "resource_management/gamemanager.h"
#include "resource_management/coperkmanager.h"

#include "game/gamemap.h"

#include "objects/base/label.h"

#include "wiki/wikidatabase.h"

COInfoActor::COInfoActor(GameMap* pMap, qint32 width)
    : m_pMap(pMap)
{
    setObjectName("COInfoActor");
    COSpriteManager* pCOSpriteManager = COSpriteManager::getInstance();
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    oxygine::TextStyle headerStyle = oxygine::TextStyle(FontManager::getMainFont48());
    headerStyle.color = FontManager::getFontColor();
    headerStyle.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    headerStyle.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    headerStyle.multiline = false;

    oxygine::spTextField pLabel = oxygine::spTextField::create();
    pLabel->setStyle(headerStyle);
    pLabel->setHtmlText(tr("CO Information"));
    pLabel->setPosition(width / 2 - pLabel->getTextRect().getWidth() / 2, 10);
    addChild(pLabel);
    m_pCurrentCO = oxygine::spSprite::create();
    m_pCurrentCO->setScale((Settings::getHeight() - 200) / 352.0f);
    m_pCurrentCO->setSize(208, 352);
    m_pCurrentCO->setPosition(Settings::getWidth() - 120 - m_pCurrentCO->getScaledWidth(), 90);
    addChild(m_pCurrentCO);

    m_pCurrentCoFaction = oxygine::spSprite::create();
    m_pCurrentCoFaction->setPosition(Settings::getWidth() * 0.25f, 100);
    m_pCurrentCoFaction->setScale(2.0f);
    addChild(m_pCurrentCoFaction);

    m_COName = oxygine::spTextField::create();
    m_COName->setStyle(headerStyle);
    m_COName->setY(m_pCurrentCoFaction->getY() + 40);
    addChild(m_COName);

    style.multiline = true;
    m_COBio = oxygine::spTextField::create();
    m_COBio->setStyle(style);
    m_COBio->setWidth(m_pCurrentCO->getX() - 50);
    m_COBio->setPosition(10, m_COName->getY() + 60);
    addChild(m_COBio);

    m_HitSprite = oxygine::spBox9Sprite::create();
    m_HitSprite->setResAnim(pCOSpriteManager->getResAnim("hit"));
    m_HitSprite->setSize(100, 16);
    m_HitSprite->setScale(2.5f);
    m_HitSprite->setPosition(10, 200);
    oxygine::spTextField pTextField = oxygine::spTextField::create();
    pTextField->setPosition(14, 1);
    pTextField->setStyle(style);
    pTextField->setScale(1 / m_HitSprite->getScaleX());
    pTextField->setHtmlText(tr("Hit"));
    pTextField->setY(3);
    m_HitSprite->setWidth(pTextField->getTextRect().getWidth() / m_HitSprite->getScaleX() + 21);
    m_HitSprite->addChild(pTextField);
    addChild(m_HitSprite);
    m_HitText = oxygine::spTextField::create();
    m_HitText->setStyle(style);
    m_HitText->setX(m_HitSprite->getX() + m_HitSprite->getScaledWidth() +10);
    addChild(m_HitText);

    m_MissSprite = oxygine::spBox9Sprite::create();
    m_MissSprite->setResAnim(pCOSpriteManager->getResAnim("miss"));
    m_MissSprite->setSize(100, 16);
    m_MissSprite->setScale(2.5f);
    m_MissSprite->setPosition(10, 300);
    pTextField = oxygine::spTextField::create();
    pTextField->setPosition(14, 1);
    pTextField->setStyle(style);
    pTextField->setScale(1 / m_MissSprite->getScaleX());
    pTextField->setHtmlText(tr("Miss"));
    pTextField->setY(3);
    m_MissSprite->setWidth(pTextField->getTextRect().getWidth() / m_MissSprite->getScaleX()  + 21);
    m_MissSprite->addChild(pTextField);
    addChild(m_MissSprite);
    m_MissText = oxygine::spTextField::create();
    m_MissText->setStyle(style);
    m_MissText->setX(m_MissSprite->getX() + m_MissSprite->getScaledWidth() + 10);
    addChild(m_MissText);

    m_InfoSprite = oxygine::spSprite::create();
    m_InfoSprite->setResAnim(pCOSpriteManager->getResAnim("skill"));
    m_InfoSprite->setScale(2.5f);
    m_InfoSprite->setPosition((Settings::getWidth() - m_pCurrentCO->getScaledWidth()) / 2 - m_InfoSprite->getScaledWidth() / 2, 400);
    addChild(m_InfoSprite);
    m_InfoText = oxygine::spTextField::create();
    m_InfoText->setStyle(style);
    m_InfoText->setWidth(m_pCurrentCO->getX() - 50);
    addChild(m_InfoText);

    m_pCoPowermeter = spCoPowermeter::create(m_pMap, nullptr);
    addChild(m_pCoPowermeter);

    m_PowerSprite = oxygine::spSprite::create();
    m_PowerSprite->setResAnim(pCOSpriteManager->getResAnim("power"));
    m_PowerSprite->setScale(2.5f);
    m_PowerSprite->setPosition(10, 400);
    addChild(m_PowerSprite);
    m_Powername = oxygine::spTextField::create();
    m_Powername->setStyle(style);
    m_Powername->setX(m_PowerSprite->getX() + m_PowerSprite->getScaledWidth() + 10);
    addChild(m_Powername);
    m_PowerDesc = oxygine::spTextField::create();
    m_PowerDesc->setStyle(style);
    m_PowerDesc->setWidth(m_pCurrentCO->getX() - 50);
    m_PowerDesc->setX(10);
    addChild(m_PowerDesc);

    m_SuperPowerSprite = oxygine::spSprite::create();
    m_SuperPowerSprite->setResAnim(pCOSpriteManager->getResAnim("superpower"));
    m_SuperPowerSprite->setScale(2.5f);
    m_SuperPowerSprite->setPosition(10, 400);
    addChild(m_SuperPowerSprite);
    m_SuperPowername = oxygine::spTextField::create();
    m_SuperPowername->setStyle(style);
    m_SuperPowername->setX(m_SuperPowerSprite->getX() + m_SuperPowerSprite->getScaledWidth() + 10);
    addChild(m_SuperPowername);
    m_SuperPowerDesc = oxygine::spTextField::create();
    m_SuperPowerDesc->setStyle(style);
    m_SuperPowerDesc->setWidth(m_pCurrentCO->getX() - 50);
    m_SuperPowerDesc->setX(10);
    addChild(m_SuperPowerDesc);

    m_synergySprite = oxygine::spSprite::create();
    m_synergySprite->setResAnim(pCOSpriteManager->getResAnim("tagCos"));
    m_synergySprite->setScale(2.0f);
    m_synergySprite->setPosition(10, 400);
    addChild(m_synergySprite);
    m_SynergyText = oxygine::spTextField::create();
    m_SynergyText->setStyle(style);
    m_SynergyText->setX(m_synergySprite->getX() + m_synergySprite->getScaledWidth() + 10);
    m_SynergyText->setHtmlText(tr("Synergy"));
    addChild(m_SynergyText);

    m_PerkText = oxygine::spTextField::create();
    m_PerkText->setStyle(style);
    m_PerkText->setHtmlText(tr("Active Perks"));
    m_PerkText->setX(10);
    addChild(m_PerkText);

    m_powerProgress = oxygine::spTextField::create();
    m_powerProgress->setStyle(style);
    m_powerProgress->setX(10);
    addChild(m_powerProgress);

    m_GlobalBoosts = oxygine::spTextField::create();
    m_GlobalBoosts->setStyle(style);
    m_GlobalBoosts->setHtmlText(tr("Global Boosts"));
    m_GlobalBoosts->setX(10);
    addChild(m_GlobalBoosts);

    m_CoBoost = oxygine::spTextField::create();
    m_CoBoost->setStyle(style);
    m_CoBoost->setHtmlText(tr("CO Zone Boosts"));
    m_CoBoost->setX(10);
    addChild(m_CoBoost);
}

void COInfoActor::showCO(spCO pCO, spPlayer pPlayer)
{
    GameManager* pGameManager = GameManager::getInstance();
    COSpriteManager* pCOSpriteManager = COSpriteManager::getInstance();
    oxygine::ResAnim* pAnim = nullptr;
    QString coid = "";
    m_pMap = nullptr;
    if (pCO.get() != nullptr)
    {
        m_pMap = pCO->getMap();
        coid = pCO->getCoID();
        pAnim = pCO->getResAnim((coid + "+nrm"));
    }
    else if (!coid.isEmpty())
    {
        pAnim = pCOSpriteManager->getResAnim((coid + "+nrm"));        
    }
    m_pCoPowermeter->setMap(m_pMap);
    m_pCurrentCO->setResAnim(pAnim);

    QString coName = "Unknown";
    QString coBio = "Unknown";
    QString coHits = "Unknown";
    QString coMiss = "Unknown";
    QString coDesc = "Unknown";
    QString coPower = "Unknown";
    QString coPowerDesc = "Unknown";
    QString coSuperpower = "Unknown";
    QString coSuperpowerDesc = "Unknown";
    QString powerProgress = "-/-";
    qint32 corange = 0;
    if (pCO.get() != nullptr)
    {
        corange = pCO->getCORange();
    }

    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValue value;
    if (pCO.get() != nullptr)
    {
        QJSValue value = pInterpreter->doFunction(coid, "getName");
        if (value.isString())
        {
            coName = value.toString();
        }
        auto* pResAnim = pGameManager->getResAnim("icon_" + pCO->getCOArmy().toLower(), oxygine::ep_ignore_error);
        m_pCurrentCoFaction->setResAnim(pResAnim);
    }
    m_COName->setHtmlText(coName);
    m_COName->setX((Settings::getWidth() - m_pCurrentCO->getScaledWidth()) / 2 - m_COName->getTextRect().getWidth());
    if (pCO.get() != nullptr)
    {
        coBio = pCO->getBio();
        coBio += pCO->getLongBio();
        coHits = pCO->getHits();
        coMiss = pCO->getMiss();
        coDesc = pCO->getCODescription();
        coDesc += "\n" + pCO->getLongCODescription();
        coPowerDesc = pCO->getPowerDescription();
        coPower = pCO->getPowerName();
        coSuperpowerDesc = pCO->getSuperPowerDescription();
        coSuperpower = pCO->getSuperPowerName();
    }
    m_COBio->setHtmlText(coBio);
    m_COBio->setHeight(m_COBio->getTextRect().getHeight());
    m_HitSprite->setY(m_COBio->getY() + m_COBio->getTextRect().getHeight() + 10);
    m_HitText->setHtmlText(coHits);
    m_HitText->setY(m_HitSprite->getY() + 5);
    m_MissSprite->setY(m_HitSprite->getY() + 50);
    m_MissText->setHtmlText(coMiss);
    m_MissText->setY(m_MissSprite->getY() + 5);
    m_InfoSprite->setY(m_MissSprite->getY() + 50);
    m_InfoText->setHtmlText((coDesc + "\n\n" + tr("CO-Zone-Range: ") + QString::number(corange)));
    m_InfoText->setHeight(m_InfoText->getTextRect().getHeight());
    m_InfoText->setY(m_InfoSprite->getY() + 50);

    m_pCoPowermeter->setCO(pCO.get());
    m_pCoPowermeter->drawPowerMeter();
    m_pCoPowermeter->setY(m_InfoText->getY() + m_InfoText->getTextRect().getHeight() + 20);
    m_pCoPowermeter->setX(20);

    if (pCO.get() != nullptr)
    {
        float starCost = pCO->getStarCost();
        float powerStars = pCO->getPowerFilled();
        powerProgress = QString::number(static_cast<qint32>(powerStars * starCost));
        powerProgress += " / ";
        qint32 powerCost = pCO->getPowerStars();
        if (powerStars > powerCost)
        {
            powerProgress += QString::number(static_cast<qint32>((powerCost + pCO->getSuperpowerStars()) * starCost));
        }
        else
        {
            powerProgress += QString::number(static_cast<qint32>(powerCost * starCost));
        }
    }
    m_powerProgress->setText(tr("Progress: ") + powerProgress);
    m_powerProgress->setY(m_pCoPowermeter->getY() + 40);

    m_PowerSprite->setY(m_powerProgress->getY() + 40);
    m_Powername->setY(m_PowerSprite->getY());
    m_PowerDesc->setY(m_Powername->getY() + 40);
    m_Powername->setHtmlText(coPower);
    m_PowerDesc->setHtmlText(coPowerDesc);
    m_PowerDesc->setHeight(m_PowerDesc->getTextRect().getHeight());

    m_SuperPowerSprite->setY(m_PowerDesc->getY() + m_PowerDesc->getTextRect().getHeight() + 20);
    m_SuperPowername->setY(m_SuperPowerSprite->getY());
    m_SuperPowerDesc->setY(m_SuperPowername->getY() + 40);
    m_SuperPowername->setHtmlText(coSuperpower);
    m_SuperPowerDesc->setHtmlText(coSuperpowerDesc);
    m_SuperPowerDesc->setHeight(m_SuperPowerDesc->getTextRect().getHeight());

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    qint32 y = m_SuperPowerDesc->getY() + m_SuperPowerDesc->getTextRect().getHeight() + 20;
    if (pCO.get() != nullptr)
    {
        m_SynergyText->setY(y);
        m_synergySprite->setY(y);
        y += 40;

        // show co synergys
        for (qint32 i = 0; i < m_SynergyCONames.size(); i++)
        {
            m_SynergyCONames[i]->detach();
            m_SynergyStarActors[i]->detach();
        }
        m_SynergyCONames.clear();
        m_SynergyStarActors.clear();
        GameManager* pGameManager = GameManager::getInstance();
        pAnim = pGameManager->getResAnim("tagaffinitystar");
        for (qint32 i = 0; i < pCOSpriteManager->getCount(); i++)
        {
            QString coid = pCOSpriteManager->getID(i);
            spCO testCO = spCO::create(coid, pPlayer.get(), pPlayer->getMap());

            QJSValueList args({pInterpreter->newQObject(pCO.get()),
                               pInterpreter->newQObject(testCO.get())});
            value = pInterpreter->doFunction("TAGPOWER", "getTagstars", args);
            if (value.isNumber())
            {
                qint32 synergy = value.toInt();
                if (synergy > 0)
                {
                    QString coSynergyName = "";
                    value = pInterpreter->doFunction(coid, "getName");
                    if (value.isString())
                    {
                        coSynergyName = value.toString();
                    }
                    oxygine::spTextField pText = oxygine::spTextField::create();
                    pText->setStyle(style);
                    pText->setHtmlText(coSynergyName);
                    pText->setX(10);
                    pText->setY(y);
                    addChild(pText);
                    m_SynergyCONames.append(pText);
                    oxygine::spActor pActor = oxygine::spActor::create();
                    pActor->setX(220);
                    pActor->setY(y);
                    for (qint32 i2 = 0; i2 < synergy; i2++)
                    {
                        oxygine::spSprite pSprite = oxygine::spSprite::create();
                        pSprite->setResAnim(pAnim);
                        pSprite->setScale(1.5f);
                        pSprite->setX(i2 * pSprite->getScaledWidth() + 3);
                        pActor->addChild(pSprite);
                    }
                    addChild(pActor);
                    m_SynergyStarActors.append(pActor);
                    y += 40;
                }
            }
        }
    }
    else
    {
        m_synergySprite->setY(y);
        m_SynergyText->setY(y);
        y += 40;
    }

    m_PerkText->setPosition(10, y);
    y += 40;
    showPerks(pCO, y);
    y += 50;

    for (qint32 i = 0; i < m_UnitDataActors.size(); i++)
    {
        m_UnitDataActors[i]->detach();
    }
    m_UnitDataActors.clear();

    qint32 x = 10;
    UnitSpriteManager* pUnitSpriteManager = UnitSpriteManager::getInstance();
    QStringList sortedUnits = pUnitSpriteManager->getUnitsSorted();
    m_GlobalBoosts->setPosition(10, y);
    constexpr qint32 textAdvance = 25;
    y += GameMap::getImageSize() * 2 + textAdvance;
    if (pCO.get() != nullptr)
    {
        qint32 customCount = pCO->getCustomUnitGlobalBoostCount();
        if (customCount > 0)
        {
            for (qint32 i = 0; i < customCount; i++)
            {
                showCustomCOBoost(pCO, x, y, i, true);
            }
            x = 10;
            y += GameMap::getImageSize() * 2 + textAdvance;
        }
        if (pCO->showDefaultUnitGlobalBoost())
        {
            for (qint32 i = 0; i < sortedUnits.size(); i++)
            {
                QString unitID = sortedUnits[i];
                spUnit pUnit = spUnit::create(unitID, pPlayer.get(), false, pPlayer->getMap());
                pUnit->setVirtuellX(-2);
                pUnit->setVirtuellY(-2);
                showCOBoost(pUnit, pCO, x, y);
            }
            x = 10;
            y += GameMap::getImageSize() * 2 + textAdvance;
        }
        m_CoBoost->setPosition(10, y);
        y += GameMap::getImageSize() * 2 + textAdvance;
        customCount = pCO->getCustomUnitZoneBoostCount();
        if (customCount > 0)
        {
            for (qint32 i = 0; i < customCount; i++)
            {
                showCustomCOBoost(pCO, x, y, i, false);
            }
            x = 10;
            y += GameMap::getImageSize() * 2 + textAdvance;
        }
        if (pCO->showDefaultUnitZoneBoost())
        {
            for (qint32 i = 0; i < sortedUnits.size(); i++)
            {
                QString unitID = sortedUnits[i];
                spUnit pUnit = spUnit::create(unitID, pPlayer.get(), false, pPlayer->getMap());
                showCOBoost(pUnit, pCO, x, y);
            }
        }
    }
    else
    {
        m_CoBoost->setPosition(10, y);
        y += GameMap::getImageSize() * 2 + textAdvance;
    }
    setHeight(y + 100);
    connect(this, &COInfoActor::sigShowLink, this, &COInfoActor::showLink, Qt::QueuedConnection);    
}

void COInfoActor::showCOBoost(spUnit pUnit, spCO pCO, qint32 & x, qint32 & y)
{
    COSpriteManager* pCOSpriteManager = COSpriteManager::getInstance();

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    m_UnitDataActors.append(oxygine::spActor::create());
    qint32 i = m_UnitDataActors.size() - 1;
    m_UnitDataActors[i]->setPosition(x, y);
    QString unitId = pUnit->getUnitID();
    m_UnitDataActors[i]->addClickListener([this, unitId](oxygine::Event*)
    {
        emit sigShowLink(unitId);
    });
    addChild(m_UnitDataActors[i]);

    // gather basic co information
    qint32 offBonus = 0;
    qint32 defBonus = 0;
    qint32 firerangeBonus = 0;
    qint32 movementBonus = 0;
    // not sure which one is better for ingame data
    //        if (m_Ingame)
    //        {
    //            offBonus = pUnit->getBonusOffensive(QPoint(-1, -1), nullptr, QPoint(-1, -1), false);
    //            defBonus = pUnit->getBonusDefensive(QPoint(-1, -1), nullptr, QPoint(-1, -1), false);
    //            firerangeBonus = pUnit->getBonusMaxRange(QPoint(-1, -1));
    //            movementBonus = pUnit->getBonusMovementpoints(QPoint(-1, -1));
    //        }
    //        else
    if (pCO.get() != nullptr)
    {
        offBonus = pCO->getOffensiveBonus(nullptr, pUnit.get(), pUnit->getPosition(), nullptr, pUnit->getPosition(), false, GameEnums::LuckDamageMode_Off);
        defBonus = pCO->getDeffensiveBonus(nullptr, nullptr, pUnit->getPosition(), pUnit.get(), pUnit->getPosition(), false, GameEnums::LuckDamageMode_Off);
        firerangeBonus = pCO->getFirerangeModifier(pUnit.get(), pUnit->getPosition());
        movementBonus = pCO->getMovementpointModifier(pUnit.get(), pUnit->getPosition());
    }

    createStrengthBar(m_UnitDataActors[i], offBonus, 0);
    createStrengthBar(m_UnitDataActors[i], defBonus, GameMap::getImageSize() / 2);

    if (firerangeBonus != 0)
    {
        oxygine::spSprite pSprite = oxygine::spSprite::create();
        pSprite->setResAnim(pCOSpriteManager->getResAnim("atkRange"));
        pSprite->setY(5 +  GameMap::getImageSize());
        pSprite->setScale(2.0f);
        m_UnitDataActors[i]->addChild(pSprite);
        oxygine::spTextField pText = oxygine::spTextField::create();
        pText->setStyle(style);
        if (firerangeBonus > 0)
        {
            pText->setHtmlText(("+" + QString::number(firerangeBonus)));
        }
        else
        {
            pText->setHtmlText((QString::number(firerangeBonus)));
        }
        pText->setPosition(pSprite->getX() + pSprite->getScaledWidth() + 2, pSprite->getY() - 2);
        pText->setScale(0.75f);
        m_UnitDataActors[i]->addChild(pText);
    }
    if (movementBonus != 0)
    {
        oxygine::spSprite pSprite = oxygine::spSprite::create();
        pSprite->setResAnim(pCOSpriteManager->getResAnim("moveRange"));
        pSprite->setPosition(20 +  GameMap::getImageSize(), 5 + GameMap::getImageSize());
        pSprite->setScale(2.0f);
        m_UnitDataActors[i]->addChild(pSprite);
        oxygine::spTextField pText = oxygine::spTextField::create();
        pText->setStyle(style);
        if (movementBonus > 0)
        {
            pText->setHtmlText(("+" + QString::number(movementBonus)));
        }
        else
        {
            pText->setHtmlText((QString::number(movementBonus)));
        }

        pText->setPosition(pSprite->getX() + pSprite->getScaledWidth() + 2, pSprite->getY() - 2);
        pText->setScale(0.75f);
        m_UnitDataActors[i]->addChild(pText);
    }

    m_UnitDataActors[i]->addChild(pUnit);
    if (x + 120 > m_pCurrentCO->getX() - 50)
    {
        x = 10;
        y += GameMap::getImageSize() * 2 + 25;
    }
    else
    {
        x += 100;
    }
}

void COInfoActor::showCustomCOBoost(spCO pCO, qint32 & x, qint32 & y, qint32 index, bool global)
{
    COSpriteManager* pCOSpriteManager = COSpriteManager::getInstance();

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    m_UnitDataActors.append(oxygine::spActor::create());
    qint32 i = m_UnitDataActors.size() - 1;
    m_UnitDataActors[i]->setPosition(x, y);

    CustomCoBoostInfo info;
    if (global)
    {
        pCO->getCustomUnitGlobalBoost(index, info);
    }
    else
    {
        pCO->getCustomUnitZoneBoost(index, info);
    }
    WikiDatabase* pDatabase = WikiDatabase::getInstance();
    m_UnitDataActors[i]->addChild(pDatabase->getIcon(pCO->getMap(), info.getIconId(), GameMap::defaultImageSize * 2));
    QString wikiLink = info.getLink();
    m_UnitDataActors[i]->addClickListener([this, wikiLink](oxygine::Event*)
    {
        emit sigShowLink(wikiLink);
    });
    addChild(m_UnitDataActors[i]);
    // gather basic co information
    qint32 offBonus = 0;
    qint32 defBonus = 0;
    createStrengthBar(m_UnitDataActors[i], info.getOffensiveBoost(), 0);
    createStrengthBar(m_UnitDataActors[i], info.getDefensiveBoost(), GameMap::getImageSize() / 2);

    auto icons = info.getBonusIcons();
    auto bonus = info.getIconBonus();
    for (qint32 i2 = 0; i2 < icons.size(); ++i2)
    {
        if (i2 < 2)
        {
            oxygine::spSprite pSprite = oxygine::spSprite::create();
            pSprite->setResAnim(pCOSpriteManager->getResAnim(icons[i2]));
            pSprite->setY(5 +  GameMap::getImageSize());
            if (i2 > 0)
            {
                pSprite->setX(20 +  GameMap::getImageSize());
            }
            pSprite->setScale(2.0f);
            m_UnitDataActors[i]->addChild(pSprite);
            oxygine::spTextField pText = oxygine::spTextField::create();
            pText->setStyle(style);
            pText->setHtmlText(bonus[i2]);
            pText->setPosition(pSprite->getX() + pSprite->getScaledWidth() + 2, pSprite->getY() - 2);
            pText->setScale(0.75f);
            m_UnitDataActors[i]->addChild(pText);
        }
        else
        {
            oxygine::handleErrorPolicy(oxygine::error_policy::ep_show_error, "Unable to show bonus icon " + icons[i] + " because it exceeds the maximum of 2");
        }
    }
    if (x + 120 > m_pCurrentCO->getX() - 50)
    {
        x = 10;
        y += GameMap::getImageSize() * 2 + 25;
    }
    else
    {
        x += 100;
    }
}

void COInfoActor::showPerks(spCO pCO, qint32 & y)
{
    for (qint32 i = 0; i < m_PerkActors.size(); i++)
    {
        m_PerkActors[i]->detach();
    }
    m_PerkActors.clear();
    if (pCO.get() != nullptr)
    {
        oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
        style.color = FontManager::getFontColor();
        style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
        style.hAlign = oxygine::TextStyle::HALIGN_LEFT;

        COPerkManager* pCOPerkManager = COPerkManager::getInstance();
        QStringList perks = pCO->getPerkList();
        qint32 x = 10;
        const qint32 width = 300;
        for (const auto & perk : perks)
        {
            oxygine::spActor perkActor = oxygine::spActor::create();
            qint32 i = pCOPerkManager->getIndex(perk);
            QString name = pCOPerkManager->getName(i);
            QString icon = pCOPerkManager->getIcon(i);
            oxygine::ResAnim* pAnim = pCOPerkManager->getResAnim(icon, oxygine::error_policy::ep_ignore_error);
            oxygine::spSprite pSprite = oxygine::spSprite::create();
            pSprite->setResAnim(pAnim);
            if (pAnim != nullptr)
            {
                pSprite->setScale((GameMap::getImageSize() * 2) / pAnim->getWidth());
            }
            pSprite->setPosition(0, 0);
            perkActor->addChild(pSprite);
            spLabel pLabel = spLabel::create(250);
            pLabel->setStyle(style);
            pLabel->setHtmlText(name);
            pLabel->setPosition(GameMap::getImageSize() * 2 + 10, 10);
            perkActor->addChild(pLabel);
            perkActor->setPosition(x, y);
            addChild(perkActor);
            perkActor->addClickListener([this, perk](oxygine::Event*)
            {
                emit sigShowLink(perk);
            });
            m_PerkActors.append(perkActor);
            x += width;
            if (x + width > m_pCurrentCO->getX() - 50)
            {
                x = 10;
                y += GameMap::getImageSize() * 2 + 10;
            }
        }
    }
}

void COInfoActor::createStrengthBar(oxygine::spActor pActor, qint32 bonus, qint32 y)
{
    oxygine::ResAnim* pStartAnim;
    oxygine::ResAnim* pEndAnim;
    COSpriteManager* pCOSpriteManager = COSpriteManager::getInstance();
    qint32 width = 50;
    float divider = 0;
    if (bonus == 0)
    {
        pStartAnim = pCOSpriteManager->getResAnim("normalPowerStart");
        pEndAnim = pCOSpriteManager->getResAnim("normalPowerEnd");
        divider = 1;
    }
    else if (bonus < 0)
    {
        pStartAnim = pCOSpriteManager->getResAnim("lessPowerStart");
        pEndAnim = pCOSpriteManager->getResAnim("lessPowerEnd");
        divider = (100.0f + bonus) / 100.0f;
    }
    else if (bonus > 100)
    {
        while (bonus > 100)
        {
            bonus -= 100;
        }
        pStartAnim = pCOSpriteManager->getResAnim("superStrongPowerStart");
        pEndAnim = pCOSpriteManager->getResAnim("strongPowerEnd");
        divider = bonus / 100.0f;
    }
    else
    {
        pStartAnim = pCOSpriteManager->getResAnim("strongPowerStart");
        pEndAnim = pCOSpriteManager->getResAnim("normalPowerEnd");
        divider = bonus / 100.0f;
    }
    oxygine::spBox9Sprite pStartBox = oxygine::spBox9Sprite::create();
    pStartBox->setResAnim(pStartAnim);
    pStartBox->setSize(static_cast<qint32>(width * divider) + 1, 8);
    pStartBox->setPosition(5 +  GameMap::getImageSize(), y);
    pActor->addChild(pStartBox);

    oxygine::spBox9Sprite pEndBox = oxygine::spBox9Sprite::create();
    pEndBox->setResAnim(pEndAnim);
    pEndBox->setSize(static_cast<qint32>(width * (1.0f - divider)) + 2, 8);
    pEndBox->setPosition(5 +  GameMap::getImageSize() + pStartBox->getWidth(), y);
    pActor->addChild(pEndBox);
}

void COInfoActor::showLink(QString pageID)
{
    
    WikiDatabase* pWikiDatabase = WikiDatabase::getInstance();
    auto entry = pWikiDatabase->getEntry(pageID);
    if (!entry.m_name.isEmpty() &&
        !entry.m_id.isEmpty())
    {
        oxygine::Stage::getStage()->addChild(pWikiDatabase->getPage(entry));
    }
    
}
