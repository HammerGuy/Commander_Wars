#ifndef CAMPAIGNMENU_H
#define CAMPAIGNMENU_H

#include "3rd_party/oxygine-framework/oxygine-framework.h"

#include "game/campaign.h"
#include "game/jsData/campaignmapdata.h"

#include "objects/minimap.h"
#include "objects/mapselection.h"
#include "objects/mapselectionview.h"
#include "objects/base/panel.h"

#include "menue/basemenu.h"

class CampaignMenu;
using spCampaignMenu = oxygine::intrusive_ptr<CampaignMenu>;

class CampaignMenu : public Basemenu
{
    Q_OBJECT
public:
    explicit CampaignMenu(spCampaign campaign, bool multiplayer, bool autosaveCampaign = false);
    virtual ~CampaignMenu() = default;
signals:
    void sigExitMenue();
    void sigButtonNext();
    void sigShowSaveCampaign();
    void sigMapSelected(qint32 index, qint32 x, qint32 y);
    void sigFlagAppeared(oxygine::Sprite* pPtrSprite, qint32 map);
    void sigEventPlayed(qint32 event);
    void sigHideMinimap();
    void sigShowMinimap();
public slots:
    // slots for changing the map
    void mapSelectionItemClicked(QString item);
    void mapSelectionItemChanged(QString item);
    void exitMenue();

    // general slots
    void slotButtonNext();
    void showSaveCampaign();
    void saveCampaign(QString file);
protected slots:
    virtual void onEnter() override;
    void mapSelected(qint32 index, qint32 x, qint32 y);
    void flagAppeared(oxygine::Sprite* pPtrSprite, qint32 map);
    void playNextEvent(qint32 event);
    void hideMinimap();
    void showMinimap();
private:
    void autosave();
    void createMapSelection(spCampaign & campaign);
    void createCampaignMapSelection(spCampaign & campaign);
    void focusOnPosition(oxygine::Vector2 position);
private:
    spMapSelectionView m_pMapSelectionView;
    CampaignMapData m_campaignData;
    oxygine::spSingleResAnim m_campaignBackground;
    oxygine::spSprite m_pMapBackground;
    oxygine::spSlidingActor m_pSlidingActor;
    QPoint m_currentMapFlagPosition;
    // buttons
    oxygine::spButton m_pButtonNext;
    oxygine::spButton m_pButtonSave;

    bool m_Multiplayer{false};
};

Q_DECLARE_INTERFACE(CampaignMenu, "CampaignMenu");

#endif // CAMPAIGNMENU_H
