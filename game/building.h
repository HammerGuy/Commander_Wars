#ifndef BUILDING_H
#define BUILDING_H

#include <QObject>
#include <QVector>


#include "3rd_party/oxygine-framework/oxygine-framework.h"
#include "game/GameEnums.h"
#include "coreengine/fileserializable.h"
#include "coreengine/scriptvariables.h"

#include "objects/base/tooltip.h"

class QmlVectorPoint;

class Player;
class Unit;
class Terrain;
class GameAction;
class Weather;
class GameMap;

class Building;
using spBuilding = oxygine::intrusive_ptr<Building>;

class Building : public Tooltip, public FileSerializable
{
    Q_OBJECT
    enum class DrawPriority
    {
        Mask = 0,
        NoneMask,
        Overlay,
        OverlayNoneMask,
    };

public:

    explicit Building(QString BuildingID, GameMap* pMap);

    virtual ~Building() = default;
    /**
     * @brief init
     */
    void init();
    /**
     * @brief updateBuildingSprites updates all sprites of this building
     */
    void updateBuildingSprites(bool neutral);
    /**
     * @brief setTerrain
     * @param pTerrain smart pointer to the terrain this building is placed on
     */
    void setTerrain(Terrain* pTerrain);
    /**
     * @brief canBuildingBePlaced
     * @param terrainID
     * @return if the building can be placed on the given terrain
     */
    bool canBuildingBePlaced(Terrain* pTerrain);
    /**
     * @brief serialize stores the object
     * @param pStream
     */
    virtual void serializeObject(QDataStream& pStream) const override;
    /**
     * @brief deserialize restores the object
     * @param pStream
     */
    virtual void deserializeObject(QDataStream& pStream) override;
    /**
     * @brief deserializer
     * @param pStream
     * @param fast
     */
    void deserializer(QDataStream& pStream, bool fast);
    /**
     * @brief getVersion stream version for serialization
     * @return
     */
    inline virtual qint32 getVersion() const override
    {
        return 5;
    }
    /**
     * @brief getOffset
     * @param pTerrain
     * @return
     */
    QPoint getOffset(Terrain* pTerrain);
    /**
     * @brief isValid
     * @return
     */
    bool isValid();
    /**
     * @brief scaleAndShowOnSingleTile
     */
    void scaleAndShowOnSingleTile();
    /**
     * @brief syncAnimation
     * @param syncTime
     */
    void syncAnimation(oxygine::timeMS syncTime);
    /**
     * @brief Unit::getSortValues
     * @return
     */
    inline const QVector<qint32> &getSortValues() const
    {
        return m_sortValues;
    }
    /**
     * @brief Unit::setSortValues
     * @param newSortValues
     */
    inline void setSortValues(const QVector<qint32> &newSortValues)
    {
        m_sortValues = newSortValues;
    }

public slots:
    /**
     * @brief usesMapLayer
     * @return
     */
    bool usesMapLayer();
    /**
     * @brief getShowInEditor
     * @param unitId
     * @return if the given building should be shown in the editor
     */
    static bool getShowInEditor(QString building);
    /**
     * @brief getImageSize
     * @return the size of an field in pixel
     */
    static qint32 getImageSize();
    /**
     * @brief getPMap
     * @return
     */
    GameMap *getMap() const;
    /**
     * @brief getBuildingName
     * @return
     */
    QString getBuildingName() const;
    /**
     * @brief setBuildingName
     * @param BuildingName
     */
    void setBuildingName(const QString &BuildingName);
    /**
     * @brief getVisionHide
     * @return
     */
    bool getVisionHide();
    /**
     * @brief getVisionHigh
     * @return
     */
    qint32 getVisionHigh() const;
    /**
     * @brief setVisionHigh
     * @param VisionHigh
     */
    void setVisionHigh(qint32 VisionHigh);
    /**
     * @brief getTotalVisionHigh
     * @return
     */
    qint32 getTotalVisionHigh();
    /**
     * @brief getBaseTerrain
     * @return
     */
    QStringList getBaseTerrain();
    /**
     * @brief getNeutralLoaded
     * @return
     */
    bool getNeutralLoaded() const;
    /**
     * @brief getDescription
     * @return
     */
    QString getDescription();
    /**
     * @brief getVision
     * @return
     */
    qint32 getVision();
    /**
     * @brief setAlwaysVisble if true the owner is always visible else the owner gets hidden during fog of war
     * @param value
     */
    bool getAlwaysVisble() const;
    /**
     * @brief setAlwaysVisble if true the owner is always visible else the owner gets hidden during fog of war
     * @param value
     */
    void setAlwaysVisble(bool value);
    /**
     * @brief getName
     * @return
     */
    QString getName();
    /**
     * @brief setOwner changes the owner ship of this building
     * @param pOwner
     */
    void setOwner(Player* pOwner);
    /**
     * @brief getMinimapIcon
     * @return
     */
    QString getMinimapIcon();
    /**
     * @brief setUnitOwner the building gets owned by the owner of this unit
     * @param pUnit
     */
    void setUnitOwner(Unit* pUnit);
    /**
     * @brief loadSprite loads a sprite for this building
     * @param sprite the sprite id
     * @param addPlayerColor true for adding player color to the sprite
     */
    void loadSprite(const QString & sprite, bool addPlayerColor, qint32 frameTime = 400, QPoint pos = QPoint(0, 0));
    /**
     * @brief loadSpriteV2
     * @param spriteID
     * @param mode
     */
    void loadSpriteV2(const QString & spriteID, GameEnums::Recoloring mode, qint32 frameTime = 400, QPoint pos = QPoint(0, 0));
    /**
     * @brief unloadSprites
     */
    void unloadSprites();
    /**
     * @brief updatePlayerColor
     * @param visible
     */
    void updatePlayerColor(bool visible);
    /**
     * @brief getOwnerID
     * @return the player owner index of this building
     */
    qint32 getOwnerID();
    /**
     * @brief getOwner
     * @return the pointer to the owner of this building
     */
    Player* getOwner();
    /**
     * @brief getBuildingID
     * @return the string if of this building
     */
    inline QString getBuildingID() const
    {
        return m_BuildingID;
    }
    /**
     * @brief getX
     * @return  x coordinates of this unit
     */
    qint32 getX() const;
    /**
     * @brief getY
     * @return  y coordinates of this unit
     */
    qint32 getY() const;
    /**
     * @brief getPosition
     * @return
     */
    QPoint getPosition() const
    {
        return QPoint(Building::getX(), Building::getY());
    }

    qint32 getHp() const;
    void setHp(const qint32 &Hp);
    /**
     * @brief isEnemyBuilding
     * @param pPlayer
     * @return
     */
    bool isEnemyBuilding(Player* pPlayer);
    /**
     * @brief getFireCount should be 0 for beeing able to fire and 1 for one turn left before firing again. Else the AI may do weird moves
     * @return
     */
    qint32 getFireCount() const;
    /**
     * @brief setFireCount  should be 0 for beeing able to fire and 1 for one turn left before firing again. Else the AI may do weird moves
     * @param value
     */
    void setFireCount(const qint32 &value);
    /**
     * @brief getBaseIncome
     * @return base income generated by this building
     */
    quint32 getBaseIncome() const;
    /**
     * @brief Building::getIncome
     * @return
     */
    qint32 getIncome();
    /**
     * @brief getActionList
     * @return list of actions that can be performed by this building e.g building units.
     */
    QStringList getActionList();
    /**
     * @brief getConstructionList
     * @return
     */
    QStringList getConstructionList();
    /**
     * @brief getRepairTypes
     * @return
     */
    QList<qint32> getRepairTypes();
    /**
     * @brief startOfTurn
     */
    void startOfTurn();
    /**
     * @brief getTerrain
     * @return
     */
    Terrain* getTerrain();
    /**
     * @brief getOffensiveBonus
     * @return
     */
    qint32 getOffensiveBonus();
    /**
     * @brief getCostReduction
     * @return
     */
    qint32 getCostModifier(const QString & id, qint32 baseCost, QPoint position);
    /**
     * @brief getOffensiveFieldBonus
     * @param pAttacker
     * @param atkPosition
     * @param pDefender
     * @param defPosition
     * @param isDefender
     * @return
     */
    qint32 getOffensiveFieldBonus(GameAction* pAction, Unit* pAttacker, QPoint atkPosition,Unit* pDefender,  QPoint defPosition, bool isDefender, GameEnums::LuckDamageMode luckMode);
    /**
     * @brief getDeffensiveFieldBonus
     * @param pAttacker
     * @param atkPosition
     * @param pDefender
     * @param defPosition
     * @param isDefender
     * @return
     */
    qint32 getDeffensiveFieldBonus(GameAction* pAction, Unit* pAttacker, QPoint atkPosition, Unit* pDefender, QPoint defPosition, bool isDefender, GameEnums::LuckDamageMode luckMode);
    /**
     * @brief getDefensiveBonus
     * @return
     */
    qint32 getDefensiveBonus();
    /**
     * @brief getVariables
     * @return
     */
    inline ScriptVariables* getVariables()
    {
        return &m_Variables;
    }
    /**
     * @brief getVision
     * @return
     */
    qint32 getVisionBonus();
    /**
     * @brief getBuildingWidth
     * @return
     */
    qint32 getBuildingWidth();
    /**
     * @brief getBuildingHeigth
     * @return
     */
    qint32 getBuildingHeigth();
    /**
     * @brief getIsAttackable
     * @return
     */
    bool getIsAttackable(qint32 x, qint32 y);
    /**
     * @brief getActionTargetFields
     * @return
     */
    QmlVectorPoint* getActionTargetFields();
    /**
     * @brief getActionTargetOffset
     * @return
     */
    QPoint getActionTargetOffset();
    /**
     * @brief getTerrainAnimationBase
     * @return
     */
    QString getTerrainAnimationBase();
    /**
     * @brief getTerrainAnimationForeground
     * @return
     */
    QString getTerrainAnimationForeground();
    /**
     * @brief getTerrainAnimationBackground
     * @return
     */
    QString getTerrainAnimationBackground();
    /**
     * @brief getTerrainAnimationMoveSpeed
     * @return
     */
    float getTerrainAnimationMoveSpeed();
    /**
     * @brief canRepair
     * @param pUnit
     * @return
     */
    bool canRepair(Unit* pUnit);
    /**
     * @brief isCaptureOrMissileBuilding
     * @return
     */
    bool isCaptureOrMissileBuilding(bool hasSiloTarget);
    /**
     * @brief isCaptureBuilding
     * @return
     */
    bool isCaptureBuilding();
    /**
     * @brief isMissile
     * @return
     */
    bool isMissile();
    /**
     * @brief isProductionBuilding
     * @return
     */
    bool isProductionBuilding();
    /**
     * @brief getDamage
     * @param pUnit
     * @return
     */
    float getDamage(Unit* pUnit);
    /**
     * @brief getBuildingTargets
     * @return
     */
    GameEnums::BuildingTarget getBuildingTargets();
    /**
     * @brief onWeatherChanged
     */
    void onWeatherChanged(Weather* pWeather);
    /**
     * @brief loadWeatherOverlaySpriteV2
     * @param spriteID
     * @param mode
     */
    void loadWeatherOverlaySpriteV2(const QString & spriteID, GameEnums::Recoloring mode, qint32 frameTime = 100);
private:
    QVector<oxygine::spSprite> m_pBuildingSprites;
    QVector<oxygine::spSprite> m_pWeatherOverlaySprites;

    QVector<GameEnums::Recoloring> m_addPlayerColor;
    /**
     * @brief m_BuildingID the id of this building
     */
    QString m_BuildingID;

    QString m_BuildingName;
    /**
     * @brief m_Owner our owner a nullptr means we're a neutral building
     */
    Player* m_pOwner{nullptr};
    /**
     * @brief m_Terrain the terrain at which we are placed
     */
    Terrain* m_pTerrain{nullptr};
    /**
     * @brief m_Hp
     */
    qint32 m_Hp{-1};
    /**
      *
      */
    qint32 m_fireCount{0};

    bool m_alwaysVisble{false};
    bool m_neutralLoaded{false};
    qint32 m_VisionHigh{0};
    ScriptVariables m_Variables;
    GameMap* m_pMap{nullptr};
    /**
     * @brief m_sortValues values sto
     */
    QVector<qint32> m_sortValues;
};

Q_DECLARE_INTERFACE(Building, "Building");

#endif // BUILDING_H
