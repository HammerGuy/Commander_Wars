#include "objects/dialogs/editor/dialograndommap.h"
#include "objects/dialogs/filedialog.h"

#include "coreengine/mainapp.h"
#include "coreengine/globalutils.h"

#include "resource_management/objectmanager.h"
#include "resource_management/fontmanager.h"

#include "game/terrain.h"

DialogRandomMap::DialogRandomMap()
{
    setObjectName("DialogRandomMap");
    Mainapp* pApp = Mainapp::getInstance();
    moveToThread(pApp->getWorkerthread());
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::spBox9Sprite pSpriteBox = oxygine::spBox9Sprite::create();
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("codialog");
    pSpriteBox->setResAnim(pAnim);
    pSpriteBox->setSize(Settings::getWidth(), Settings::getHeight());
    addChild(pSpriteBox);
    pSpriteBox->setPosition(0, 0);
    pSpriteBox->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    setPriority(static_cast<qint32>(Mainapp::ZOrder::Dialogs));

    m_pPanel = spPanel::create(true, QSize(Settings::getWidth() - 60, Settings::getHeight() - 110),
                         QSize(Settings::getWidth() - 60, Settings::getHeight() - 110));
    m_pPanel->setPosition(30, 30);
    pSpriteBox->addChild(m_pPanel);
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();

    float y = 30;
    qint32 width = 250;

    spLabel text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Generator:"));
    text->setPosition(30, y);
    m_pPanel->addItem(text);
    m_GeneratorFile = spTextbox::create(Settings::getWidth() - 300 - width);
    m_GeneratorFile->setTooltipText(tr("Selects the generator script used to generate the random map."));
    m_GeneratorFile->setPosition(text->getX() + width, text->getY());
    m_GeneratorFile->setCurrentText("data/randommaps/commanderwarsgenerator.js");
    m_pPanel->addItem(m_GeneratorFile);
    m_Generator = ObjectManager::createButton(tr("Select"), 130);
    m_Generator->setPosition(m_GeneratorFile->getX() + m_GeneratorFile->getWidth() + 10, y);
    m_Generator->addClickListener([this](oxygine::Event*)
    {
        emit sigShowGeneratorSelection();
    });
    connect(m_GeneratorFile.get(), &Textbox::sigTextChanged, this, &DialogRandomMap::generatorChanged, Qt::QueuedConnection);
    connect(this, &DialogRandomMap::sigShowGeneratorSelection, this, &DialogRandomMap::showGeneratorSelection, Qt::QueuedConnection);
    m_pPanel->addItem(m_Generator);
    y += 40;

    // Label
    text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Name:"));
    text->setPosition(30, y);
    m_pPanel->addItem(text);
    m_MapName = spTextbox::create(Settings::getWidth() - 150 - width);
    m_MapName->setTooltipText(tr("Selects the name of the new map."));
    m_MapName->setPosition(text->getX() + width, text->getY());
    m_MapName->setCurrentText("");
    m_pPanel->addItem(m_MapName);
    y += 40;

    // Label
    text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Author:"));
    text->setPosition(30, y);
    m_pPanel->addItem(text);
    m_MapAuthor = spTextbox::create(Settings::getWidth() - 150 - width);
    m_MapAuthor->setTooltipText(tr("Selects the author of the new map."));
    m_MapAuthor->setPosition(text->getX() + width, text->getY());
    m_MapAuthor->setCurrentText(Settings::getUsername());
    m_pPanel->addItem(m_MapAuthor);
    y += 40;

    // Label
    text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Description:"));
    text->setPosition(30, y);
    m_pPanel->addItem(text);
    m_MapDescription = spTextbox::create(Settings::getWidth() - 150 - width);
    m_MapDescription->setTooltipText(tr("Selects the description for the new map."));
    m_MapDescription->setPosition(text->getX() + width, text->getY());
    m_MapDescription->setCurrentText("");
    m_pPanel->addItem(m_MapDescription);
    y += 40;

    // Label
    text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Width:"));
    text->setPosition(30, 5 + y );
    m_pPanel->addItem(text);
    m_MapWidth = spSpinBox::create(300, 1, 999, SpinBox::Mode::Int);
    m_MapWidth->setTooltipText(tr("Selects the width of the new map."));
    m_MapWidth->setPosition(text->getX() + width, text->getY());
    m_MapWidth->setCurrentValue(20);
    m_pPanel->addItem(m_MapWidth);

    // Label
    y += 40;
    text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Heigth:"));
    text->setPosition(30, 5 + y + text->getHeight());
    m_pPanel->addItem(text);
    m_MapHeigth = spSpinBox::create(300, 1, 999, SpinBox::Mode::Int);
    m_MapHeigth->setTooltipText(tr("Selects the height of the new map."));
    m_MapHeigth->setPosition(text->getX() + width, text->getY());
    m_MapHeigth->setCurrentValue(20);
    m_pPanel->addItem(m_MapHeigth);

    // Label
    y += 40;
    text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Player:"));
    text->setPosition(30, 5 + y + text->getHeight());
    m_pPanel->addItem(text);
    m_MapPlayerCount = spSpinBox::create(300, 2, 40, SpinBox::Mode::Int);
    m_MapPlayerCount->setTooltipText(tr("Selects the amount of players for the new map."));
    m_MapPlayerCount->setPosition(text->getX() + width, text->getY());
    m_MapPlayerCount->setCurrentValue(4);
    connect(m_MapPlayerCount.get(), &SpinBox::sigValueChanged, this, &DialogRandomMap::playerChanged, Qt::QueuedConnection);
    m_pPanel->addItem(m_MapPlayerCount);

    // Label
    y += 40;
    text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Mirrored Map:"));
    text->setPosition(30, 5 + y + text->getHeight());
    m_pPanel->addItem(text);
    m_mirrored = spCheckbox::create();
    m_mirrored->setTooltipText(tr("If the map should be created in a mirrored/rotated base. This requires an even number of players to work."));
    m_mirrored->setChecked(false);
    m_mirrored->setPosition(text->getX() + width, text->getY());
    m_pPanel->addItem(m_mirrored);

    // Label
    y += 40;
    text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Seed:"));
    text->setPosition(30, 5 + y + text->getHeight());
    m_pPanel->addItem(text);
    m_Seed = spSpinBox::create(300, 0, std::numeric_limits<qint32>::max() - 1, SpinBox::Mode::Int);
    m_Seed->setTooltipText(tr("The seed to generate the new map. Using the same map settings with the same seed will generate the same map."));
    m_Seed->setPosition(text->getX() + width, text->getY());
    m_Seed->setCurrentValue(GlobalUtils::randInt(0, std::numeric_limits<qint32>::max() - 1));
    m_pPanel->addItem(m_Seed);

    // Label
    y += 40;
    text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Create Road:"));
    text->setPosition(30, 5 + y + text->getHeight());
    m_pPanel->addItem(text);
    m_CreateRoad = spCheckbox::create();
    m_CreateRoad->setTooltipText(tr("If selected roads are created between the HQ's of the players."));
    m_CreateRoad->setChecked(false);
    m_CreateRoad->setPosition(text->getX() + width, text->getY());
    m_pPanel->addItem(m_CreateRoad);

    // Label
    y += 40;
    text = spLabel::create(width - 10);
    text->setStyle(style);
    text->setHtmlText(tr("Base Size:"));
    text->setPosition(30, 5 + y + text->getHeight());
    m_pPanel->addItem(text);
    m_BaseSize = spSlider::create(Settings::getWidth() - 220 - width, 0, 100);
    m_BaseSize->setCurrentValue(10);
    m_BaseSize->setTooltipText(tr("The percent of randomly placed buildings versus buildings placed near each HQ. A lower percentage places more buildings randomly across the whole map."));
    m_BaseSize->setPosition(text->getX() + width, y);
    m_pPanel->addItem(m_BaseSize);

    // Label
    y += 40;
    m_TerrainChanceLabel = oxygine::spTextField::create();
    m_TerrainChanceLabel->setStyle(style);
    m_TerrainChanceLabel->setHtmlText(tr("Terrain Distribution"));
    m_TerrainChanceLabel->setPosition(30, 5 + y + m_TerrainChanceLabel->getHeight());
    m_pPanel->addItem(m_TerrainChanceLabel);
    y += 40;
    m_BuildingChanceLabel = oxygine::spTextField::create();
    m_BuildingChanceLabel->setStyle(style);
    m_BuildingChanceLabel->setHtmlText(tr("Building Distribution"));
    m_BuildingChanceLabel->setPosition(30, 5 + y + m_BuildingChanceLabel->getHeight());
    m_pPanel->addItem(m_BuildingChanceLabel);
    y += 40;
    m_OwnerDistributionLabel = oxygine::spTextField::create();
    m_OwnerDistributionLabel->setStyle(style);
    m_OwnerDistributionLabel->setHtmlText(tr("Owner Distribution"));
    m_OwnerDistributionLabel->setPosition(30, 5 + y + m_OwnerDistributionLabel->getHeight());
    m_pPanel->addItem(m_OwnerDistributionLabel);
    y += 40;

    // Label
    m_unitCountLabel = spLabel::create(width - 10);
    m_unitCountLabel->setStyle(style);
    m_unitCountLabel->setHtmlText(tr("Unit Count:"));
    m_unitCountLabel->setPosition(30, 5 + y );
    m_pPanel->addItem(m_unitCountLabel);
    m_unitCount = spSpinBox::create(300, 0, 9999, SpinBox::Mode::Int);
    m_unitCount->setTooltipText(tr("Total amount of units that get spawned. If no valid position for a unit is found no unit will be spawned."));
    m_unitCount->setPosition(m_unitCountLabel->getX() + width, m_unitCountLabel->getY());
    m_unitCount->setCurrentValue(0);
    m_pPanel->addItem(m_unitCount);
    y += 40;

    // Label
    m_unitsNearHqLabel = spLabel::create(width - 10);
    m_unitsNearHqLabel->setStyle(style);
    m_unitsNearHqLabel->setHtmlText(tr("Units near HQ:"));
    m_unitsNearHqLabel->setPosition(30, 5 + y );
    m_pPanel->addItem(m_unitsNearHqLabel);
    m_unitsNearHq = spSpinBox::create(300, 0, 100, SpinBox::Mode::Int);
    m_unitsNearHq->setUnit("%");
    m_unitsNearHq->setTooltipText(tr("The percentage of units which are spawned near the HQ of the player."));
    m_unitsNearHq->setPosition(text->getX() + width, text->getY());
    m_unitsNearHq->setCurrentValue(100);
    m_pPanel->addItem(m_unitsNearHq);
    y += 40;
    // Label
    m_unitDistributionLabel = spLabel::create(width - 10);
    m_unitDistributionLabel->setStyle(style);
    m_unitDistributionLabel->setHtmlText(tr("Units spawn mode:"));
    m_unitDistributionLabel->setPosition(30, 5 + y );
    m_pPanel->addItem(m_unitDistributionLabel);
    QStringList items = {tr("Random"), tr("Distributed")};
    m_unitDistributionSelection = spDropDownmenu::create(300, items);
    m_unitDistributionSelection->setTooltipText(tr("Random: Units are spawned at random.\nDistributed: Units are spawned at the given rate."));
    m_unitDistributionSelection->setPosition(text->getX() + width, text->getY());
    m_unitDistributionSelection->setCurrentItem(1);
    m_pPanel->addItem(m_unitDistributionSelection);
    y += 40;
    m_UnitDistributionLabel = oxygine::spTextField::create();
    m_UnitDistributionLabel->setStyle(style);
    m_UnitDistributionLabel->setHtmlText(tr("Unit distribution:"));
    m_UnitDistributionLabel->setPosition(30, 5 + y + m_UnitDistributionLabel->getHeight());
    m_pPanel->addItem(m_UnitDistributionLabel);
    y += 40;
    m_UnitChanceLabel = oxygine::spTextField::create();
    m_UnitChanceLabel->setStyle(style);
    m_UnitChanceLabel->setHtmlText(tr("Unit spawn chance"));
    m_UnitChanceLabel->setPosition(30, 5 + y + m_UnitChanceLabel->getHeight());
    m_pPanel->addItem(m_UnitChanceLabel);
    y += 40;


    m_pPanel->setContentHeigth(y + 40);
    // ok button
    m_OkButton = pObjectManager->createButton(tr("Ok"), 150);
    m_OkButton->setPosition(Settings::getWidth() - m_OkButton->getWidth() - 30, Settings::getHeight() - 30 - m_OkButton->getHeight());
    pSpriteBox->addChild(m_OkButton);


    // cancel button
    m_ExitButton = pObjectManager->createButton(tr("Cancel"), 150);
    m_ExitButton->setPosition(30, Settings::getHeight() - 30 - m_OkButton->getHeight());
    pSpriteBox->addChild(m_ExitButton);
    m_ExitButton->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
    {
        emit sigCancel();
    });
    generatorChanged(m_GeneratorFile->getCurrentText());

    m_OkButton->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
    {
        emit sigFinish();
    });
    connect(m_mirrored.get(), &Checkbox::checkChanged, this, [this](bool)
    {
        checkIfGenerationIsAllowed();
    }, Qt::QueuedConnection);
    connect(m_MapPlayerCount.get(), &SpinBox::sigValueChanged, this, [this](qreal)
    {
        checkIfGenerationIsAllowed();
    }, Qt::QueuedConnection);
    connect(this, &DialogRandomMap::sigCancel, this, &DialogRandomMap::remove, Qt::QueuedConnection);
    connect(this, &DialogRandomMap::sigClose, this, &DialogRandomMap::remove, Qt::QueuedConnection);
    connect(this, &DialogRandomMap::sigFinish, this, &DialogRandomMap::generatorFinished, Qt::QueuedConnection);
}

void DialogRandomMap::checkIfGenerationIsAllowed()
{
    m_OkButton->setEnabled(!m_mirrored->getChecked() || static_cast<qint32>(m_MapPlayerCount->getCurrentValue()) % 2 == 0);
}

void DialogRandomMap::generatorFinished()
{
    auto* pTerrainChances = m_TerrainChances.get();
    auto* pBuildingChances = m_BuildingChances.get();
    auto* pOwnerDistribution = m_OwnerDistribution.get();
    auto* pUnitChances = m_UnitChances.get();
    auto* pUnitDistribution = m_unitDistribution.get();
    auto* pMapName = m_MapName.get();
    auto* pMapAuthor = m_MapAuthor.get();
    auto* pMapDescription = m_MapDescription.get();
    auto* pMapWidth = m_MapWidth.get();
    auto* pMapHeigth = m_MapHeigth.get();
    auto* pCreateRoad = m_CreateRoad.get();
    auto* pSeed = m_Seed.get();
    auto* pBaseSize = m_BaseSize.get();
    auto* pUnitCount = m_unitCount.get();
    auto* pUnitsNearHq = m_unitsNearHq.get();
    auto* pUnitDistributionSelection = m_unitDistributionSelection.get();
    QVector<std::tuple<QString, float>> terrains;
    for (qint32 i = 0; i < m_TerrainIDs.size(); i++)
    {
        terrains.append(std::tuple<QString, float>(m_TerrainIDs[i], pTerrainChances->getSliderValue(i)));
    }
    QVector<std::tuple<QString, float>> buildings;
    for (qint32 i = 0; i < m_BuildingIDs.size(); i++)
    {
        buildings.append(std::tuple<QString, float>(m_BuildingIDs[i], pBuildingChances->getSliderValue(i)));
    }
    QVector<float> ownedBaseSize;
    qint32 player = m_MapPlayerCount->getCurrentValue();
    for (qint32 i = 0; i < player; i++)
    {
        ownedBaseSize.append(pOwnerDistribution->getSliderValue(i + 1));
    }

    QVector<std::tuple<QString, float>> units;
    for (qint32 i = 0; i < m_UnitIDs.size(); i++)
    {
        units.append(std::tuple<QString, float>(m_UnitIDs[i], pUnitChances->getSliderValue(i)));
    }
    QVector<float> unitDistribution;
    for (qint32 i = 0; i < player; i++)
    {
        unitDistribution.append(pUnitDistribution->getSliderValue(i));
    }

    emit sigFinished(pMapName->getCurrentText(), pMapAuthor->getCurrentText(),
                     pMapDescription->getCurrentText(),
                     static_cast<qint32>(pMapWidth->getCurrentValue()), static_cast<qint32>(pMapHeigth->getCurrentValue()),
                     player, pCreateRoad->getChecked(), static_cast<qint32>(pSeed->getCurrentValue()),
                     terrains, buildings, ownedBaseSize,
                     pBaseSize->getCurrentValue(),
                     units, pUnitCount->getCurrentValue(),
                     pUnitsNearHq->getCurrentValue(),
                     unitDistribution,
                     (pUnitDistributionSelection->getCurrentItem() == 1),
                     m_mirrored->getChecked());
    emit sigClose();
}

void DialogRandomMap::remove()
{
    detach();
}

void DialogRandomMap::showGeneratorSelection()
{    
    QStringList wildcards;
    wildcards.append("*.js");
    QString path = Settings::getUserPath() + "data/randomMaps";
    spFileDialog fileDialog = spFileDialog::create(path, wildcards, "", false, tr("Load"));
    addChild(fileDialog);
    connect(fileDialog.get(),  &FileDialog::sigFileSelected, this, &DialogRandomMap::generatorChanged, Qt::QueuedConnection);    
}

void DialogRandomMap::DialogRandomMap::generatorChanged(QString filename)
{
    filename =  GlobalUtils::makePathRelative(filename);
    m_GeneratorFile->setCurrentText(filename);
    QFile file(filename);
    if (!file.exists())
    {
        file.setFileName(oxygine::Resource::RCC_PREFIX_PATH + filename);
        filename = oxygine::Resource::RCC_PREFIX_PATH + filename;
    }
    if (file.exists())
    {
        if (m_TerrainChances.get())
        {
            m_TerrainChances->detach();
        }
        if (m_BuildingChances.get())
        {
            m_BuildingChances->detach();
        }
        Interpreter* pInterpreter = Interpreter::getInstance();
        pInterpreter->openScript(filename, false);
        m_TerrainIDs = pInterpreter->doFunction("RANDOMMAPGENERATOR", "getTerrainBases").toVariant().toStringList();
        QList<QVariant> terrainChancesVariant = pInterpreter->doFunction("RANDOMMAPGENERATOR", "getTerrainBaseChances").toVariant().toList();
        m_BuildingIDs = pInterpreter->doFunction("RANDOMMAPGENERATOR", "getBuildingBases").toVariant().toStringList();
        QList<QVariant> buildingChancesVariant = pInterpreter->doFunction("RANDOMMAPGENERATOR", "getBuildingBaseChances").toVariant().toList();
        m_UnitIDs = pInterpreter->doFunction("RANDOMMAPGENERATOR", "getUnitBases").toVariant().toStringList();
        QList<QVariant> unitChancesVariant = pInterpreter->doFunction("RANDOMMAPGENERATOR", "getUnitBaseChances").toVariant().toList();
        QStringList terrainStrings;
        QVector<qint32> terrainChances;
        if (m_TerrainIDs.size() == terrainChancesVariant.size())
        {
            for (qint32 i = 0; i < m_TerrainIDs.size(); i++)
            {
                if (m_TerrainIDs[i] == "Buildings")
                {
                    terrainStrings.append(m_TerrainIDs[i]);
                }
                else
                {
                    spTerrain pTerrain = Terrain::createTerrain(m_TerrainIDs[i], -1, -1, "", nullptr);
                    terrainStrings.append(pTerrain->getTerrainName());
                }
                terrainChances.append(terrainChancesVariant[i].toInt());
            }
        }
        m_TerrainChances = spMultislider::create(terrainStrings, Settings::getWidth() - 150, terrainChances);
        m_TerrainChances->setTooltipText(tr("The percentage of different terrain types distributed."));
        m_TerrainChances->setPosition(30, m_TerrainChanceLabel->getY() + 40);
        m_pPanel->addItem(m_TerrainChances);

        m_BuildingChanceLabel->setY(m_TerrainChances->getY() + 40 * terrainStrings.size());

        QStringList buildingStrings;
        QVector<qint32> buildingChances;
        if (m_BuildingIDs.size() == buildingChancesVariant.size())
        {
            for (qint32 i = 0; i < m_BuildingIDs.size(); i++)
            {
                buildingStrings.append(pInterpreter->doFunction(m_BuildingIDs[i], "getName").toString());
                buildingChances.append(buildingChancesVariant[i].toInt());
            }
        }
        m_BuildingChances = spMultislider::create(buildingStrings, Settings::getWidth() - 150, buildingChances);
        m_BuildingChances->setTooltipText(tr("The percentage of different buildings types distributed."));
        m_BuildingChances->setPosition(30, m_BuildingChanceLabel->getY() + 40);
        m_pPanel->addItem(m_BuildingChances);
        m_OwnerDistributionLabel->setY(m_BuildingChances->getY() + 40 * buildingStrings.size());
        m_UnitChanceValues.clear();
        for (qint32 i = 0; i < m_UnitIDs.size(); i++)
        {
            m_UnitChanceValues.append(unitChancesVariant[i].toInt());
        }
        playerChanged(0);
    }
    
}

void DialogRandomMap::playerChanged(qreal)
{
    
    if (m_OwnerDistribution.get())
    {
        m_OwnerDistribution->detach();
    }
    QStringList playerStrings;
    QVector<qint32> playerChances;
    playerStrings.append(tr("Neutral"));
    playerChances.append(100);
    qint32 player = m_MapPlayerCount->getCurrentValue();
    for (qint32 i = 0; i < player; i++)
    {
        playerStrings.append(tr("Player ") + QString::number(i + 1));
        playerChances.append(0);
    }
    m_OwnerDistribution = spMultislider::create(playerStrings, Settings::getWidth() - 150, playerChances);
    m_OwnerDistribution->setTooltipText(tr("The percentage of buildings distributed between the players. Note buildings close to an Player HQ may be ignored."));
    m_OwnerDistribution->setPosition(30, m_OwnerDistributionLabel->getY() + 40);
    m_pPanel->addItem(m_OwnerDistribution);
    createUnitChances();
}

void DialogRandomMap::createUnitChances()
{
    if (m_unitDistribution.get())
    {
        m_unitDistribution->detach();
    }
    if (m_UnitChances.get())
    {
        m_UnitChances->detach();
    }
    Interpreter* pInterpreter = Interpreter::getInstance();
    QStringList unitStrings;
    if (m_UnitIDs.size() == m_UnitChanceValues.size())
    {
        for (qint32 i = 0; i < m_UnitIDs.size(); i++)
        {
            unitStrings.append(pInterpreter->doFunction(m_UnitIDs[i], "getName").toString());
        }
    }
    qint32 player = m_MapPlayerCount->getCurrentValue() + 1;
    m_unitCountLabel->setY(m_OwnerDistribution->getY() + 40 * player);
    m_unitCount->setY(m_OwnerDistribution->getY() + 40 * player);
    m_unitsNearHqLabel->setY(m_unitCount->getY() + 40);
    m_unitsNearHq->setY(m_unitCount->getY() + 40);
    m_unitDistributionLabel->setY(m_unitsNearHq->getY() + 40);
    m_unitDistributionSelection->setY(m_unitsNearHq->getY() + 40);
    m_UnitDistributionLabel->setY(m_unitDistributionSelection->getY() + 40);

    QStringList playerStrings;
    QVector<qint32> playerChances;
    for (qint32 i = 0; i < player - 1; i++)
    {
        playerStrings.append(tr("Player ") + QString::number(i + 1));
        playerChances.append(0);
    }
    qint32 y = m_UnitDistributionLabel->getY() + 40;
    m_unitDistribution = spMultislider::create(playerStrings, Settings::getWidth() - 150, playerChances);
    m_unitDistribution->setTooltipText(tr("The percent unit distribution between the players. Note units that can't be spawned still count to the distribution."));
    m_unitDistribution->setPosition(30, y);
    m_pPanel->addItem(m_unitDistribution);

    y = m_unitDistribution->getY() + 40 * (player - 1);
    m_UnitChanceLabel->setY(y);
    m_UnitChances = spMultislider::create(unitStrings, Settings::getWidth() - 150, m_UnitChanceValues);
    m_UnitChances->setTooltipText(tr("The percent distribution between the units that will be spawned. Note units that can't be spawned still count to the distribution."));
    m_UnitChances->setPosition(30, m_UnitChanceLabel->getY() + 40);
    m_pPanel->addItem(m_UnitChances);
    m_pPanel->setContentHeigth(m_UnitChances->getY() + 40 * (m_UnitIDs.size() + 2));
}
