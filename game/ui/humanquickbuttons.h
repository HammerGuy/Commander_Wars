#pragma once

#include <QObject>
#include "ui_reader/createdgui.h"

class GameMenue;
class HumanQuickButtons;
using spHumanQuickButtons = oxygine::intrusive_ptr<HumanQuickButtons>;

class HumanQuickButtons : public CreatedGui
{
public:
    explicit HumanQuickButtons(GameMenue* pMenu);
    virtual ~HumanQuickButtons();
};

