#include "game/gameanimation/gameanimation.h"
#include "game/gameanimation/gameanimationfactory.h"

#include "resource_management/gameanimationmanager.h"
#include "resource_management/fontmanager.h"

#include "coreengine/console.h"
#include "coreengine/mainapp.h"
#include "coreengine/interpreter.h"
#include "coreengine/settings.h"
#include "coreengine/audiothread.h"

GameAnimation::GameAnimation(quint32 frameTime, GameMap* pMap)
    : m_frameTime(frameTime / Settings::getAnimationSpeed()),
      m_pMap{pMap}
{
    setObjectName("GameAnimation");
    Mainapp* pApp = Mainapp::getInstance();
    moveToThread(pApp->getWorkerthread());
    Interpreter::setCppOwnerShip(this);
    if (m_frameTime <= 0)
    {
        m_frameTime = 1;
    }
    connect(this, &GameAnimation::sigFinished, this, &GameAnimation::onFinished, Qt::QueuedConnection);
    connect(this, &GameAnimation::sigStart, this, &GameAnimation::start, Qt::QueuedConnection);
    m_buffer.open(QIODevice::ReadWrite);
    oxygine::Sprite::setVisible(false);
}

void GameAnimation::restart()
{    
    if (m_pMap != nullptr)
    {
        m_stopped = false;
        m_previousAnimation = nullptr;
        for (auto & tween : m_stageTweens)
        {
            oxygine::Stage::getStage()->addTween(tween);
        }
        m_pMap->addChild(spGameAnimation(this));
        start();
    }
}

void GameAnimation::start()
{
    if (!m_started)
    {
        m_started = true;
        setVisible(true);
        m_previousAnimation = nullptr;
        doPreAnimationCall();
        AudioThread* pAudioThread = Mainapp::getInstance()->getAudioThread();
        for (auto & data : m_SoundData)
        {
            pAudioThread->playSound(data.soundFile, data.loops, data.delayMs / Settings::getAnimationSpeed(), data.volume, data.stopOldestSound);
        }
    }
}

void GameAnimation::doPreAnimationCall()
{
    if ((!m_jsPreActionObject.isEmpty()) && (!m_jsPreActionObject.isEmpty()))
    {
        CONSOLE_PRINT("Calling post Animation function " + m_jsPreActionObject + "." + m_jsPreActionFunction, Console::eDEBUG);
        Interpreter* pInterpreter = Interpreter::getInstance();
        QJSValueList args({pInterpreter->newQObject(this),
                           pInterpreter->newQObject(m_pMap)});
        pInterpreter->doFunction(m_jsPreActionObject, m_jsPreActionFunction, args);
    }
}

GameMap *GameAnimation::getMap() const
{
    return m_pMap;
}

void GameAnimation::stop()
{
    for (auto & tween : m_stageTweens)
    {
        tween->complete();
    }
    m_stopped = true;
    setVisible(false);
}

void GameAnimation::setRotation(float angle)
{
    setRotationDegrees(angle);
}

void GameAnimation::queueAnimation(GameAnimation* pGameAnimation)
{
    pGameAnimation->setPreviousAnimation(this);
    m_QueuedAnimations.append(spGameAnimation(pGameAnimation));
    GameAnimationFactory::getInstance()->queueAnimation(pGameAnimation);
}

void GameAnimation::queueAnimationBefore(GameAnimation* pGameAnimation)
{
    if (m_previousAnimation.get() != nullptr)
    {
        // remove ourself from previous animation and add our ancestor
        m_previousAnimation->removeQueuedAnimation(this);
        m_previousAnimation->queueAnimation(pGameAnimation);
    }
    // queue ourself after the given animation
    pGameAnimation->queueAnimation(this);
}

void GameAnimation::removeQueuedAnimation(GameAnimation* pGameAnimation)
{
    qint32 i = 0;
    while (i < m_QueuedAnimations.size())
    {
        if (m_QueuedAnimations[i].get() == pGameAnimation)
        {
            m_QueuedAnimations.removeAt(i);
        }
        else
        {
            i++;
        }
    }
}

void GameAnimation::update(const oxygine::UpdateState& us)
{
    if (!m_stopped)
    {
        oxygine::Sprite::update(us);
    }
    if (!m_started)
    {
        emit sigStart();
    }
}

bool GameAnimation::getStopSoundAtAnimationEnd() const
{
    return m_stopSoundAtAnimationEnd;
}

void GameAnimation::setStopSoundAtAnimationEnd(bool stopSoundAtAnimationEnd)
{
    m_stopSoundAtAnimationEnd = stopSoundAtAnimationEnd;
}

bool GameAnimation::getVisible() const
{
    return oxygine::Sprite::getVisible();
}
void GameAnimation::setVisible(bool vis)
{
    oxygine::Sprite::setVisible(vis);
}

void GameAnimation::setPreviousAnimation(GameAnimation *previousAnimation)
{
    m_previousAnimation = previousAnimation;
}

void GameAnimation::addSprite(QString spriteID, float offsetX, float offsetY, qint32 sleepAfterFinish, float scale, qint32 delay, qint32 loops)
{
    addSprite2(spriteID, offsetX, offsetY, sleepAfterFinish, scale, scale, delay, loops);
}

void GameAnimation::addSprite2(QString spriteID, float offsetX, float offsetY, qint32 sleepAfterFinish, float scaleX, float scaleY, qint32 delay, qint32 loops)
{
    addSprite3(spriteID, offsetX, offsetY, QColor(255, 255, 255), sleepAfterFinish, scaleX, scaleY, delay, 0, loops);
}

void GameAnimation::addSpriteAnimTable(QString spriteID, float offsetX, float offsetY, Player* pPlayer, qint32 sleepAfterFinish, float scaleX, float scaleY, qint32 delay, qint32, GameEnums::Recoloring mode)
{
    GameAnimationManager* pGameAnimationManager = GameAnimationManager::getInstance();
    oxygine::ResAnim* pAnim = pGameAnimationManager->getResAnim(spriteID, oxygine::error_policy::ep_ignore_error);
    if (pAnim != nullptr)
    {
        loadSpriteAnimTable(pAnim, offsetX, offsetY, pPlayer, sleepAfterFinish, scaleX, scaleY, delay, mode);
    }
    else
    {
        CONSOLE_PRINT("Unable to load animation sprite: " + spriteID, Console::eDEBUG);
    }
}

void GameAnimation::addSprite3(QString spriteID, float offsetX, float offsetY, QColor color, qint32 sleepAfterFinish, float scaleX, float scaleY, qint32 delay, qint32 frames, qint32 loops)
{
    GameAnimationManager* pGameAnimationManager = GameAnimationManager::getInstance();
    oxygine::ResAnim* pAnim = pGameAnimationManager->getResAnim(spriteID, oxygine::error_policy::ep_ignore_error);
    if (pAnim != nullptr)
    {
        loadSpriteAnim(pAnim, offsetX, offsetY, color, sleepAfterFinish, scaleX, scaleY, delay, loops);
    }
    else
    {
        QImage img;
        if (QFile::exists(Settings::getUserPath() + spriteID))
        {
            img = QImage(Settings::getUserPath() + spriteID);
        }
        else if (QFile::exists(oxygine::Resource::RCC_PREFIX_PATH + spriteID))
        {
            img = QImage(oxygine::Resource::RCC_PREFIX_PATH + spriteID);
        }
        else
        {
            CONSOLE_PRINT("Unable to load animation sprite: " + spriteID, Console::eDEBUG);
            return;
        }
        oxygine::spSingleResAnim pAnim = oxygine::spSingleResAnim::create();
        Mainapp::getInstance()->loadResAnim(pAnim, img, 1, frames, 1, true);
        m_resAnims.append(pAnim);
        loadSpriteAnim(pAnim.get(), offsetX, offsetY, color, sleepAfterFinish, scaleX, scaleY, delay, loops);
    }
}

void GameAnimation::addBox(QString spriteID, float offsetX, float offsetY, qint32 width, qint32 heigth,  qint32 sleepAfterFinish, QColor color)
{
    oxygine::spBox9Sprite pBox = oxygine::spBox9Sprite::create();
    oxygine::spTweenQueue queuedAnim = oxygine::spTweenQueue::create();
    GameAnimationManager* pGameAnimationManager = GameAnimationManager::getInstance();
    oxygine::ResAnim* pAnim = pGameAnimationManager->getResAnim(spriteID, oxygine::error_policy::ep_ignore_error);
    pBox->setResAnim(pAnim);
    pBox->setSize(width, heigth);
    setSize(width, heigth);
    if (sleepAfterFinish > 0)
    {
        oxygine::spTween tween1 = oxygine::createTween(TweenWait(), oxygine::timeMS(static_cast<qint64>(sleepAfterFinish / Settings::getAnimationSpeed())), 1);
        queuedAnim->add(tween1);
    }
    pBox->addTween(queuedAnim);
    if (color != Qt::white)
    {
        pBox->setColor(color);
    }
    addChild(pBox);
    pBox->setPosition(offsetX, offsetY);
    if(!m_finishQueued)
    {
        m_finishQueued = true;
        queuedAnim->setDoneCallback([this](oxygine::Event *)->void
        {
            emitFinished();
        });

    }
}

void GameAnimation::loadSpriteAnimTable(oxygine::ResAnim* pAnim, float offsetX, float offsetY, Player* pPlayer, qint32 sleepAfterFinish, float scaleX, float scaleY, qint32 delay, GameEnums::Recoloring mode)
{
    if (pAnim != nullptr)
    {
        oxygine::spSprite pSprite = oxygine::spSprite::create();
        oxygine::spTweenQueue queuedAnim = oxygine::spTweenQueue::create();
        oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), oxygine::timeMS(pAnim->getTotalFrames() * m_frameTime), 1, false, oxygine::timeMS(static_cast<qint64>(delay / Settings::getAnimationSpeed())));
        queuedAnim->add(tween);
        if (sleepAfterFinish > 0)
        {
            oxygine::spTween tween1 = oxygine::createTween(TweenWait(), oxygine::timeMS(static_cast<qint64>(sleepAfterFinish / Settings::getAnimationSpeed())), 1);
            queuedAnim->add(tween1);
        }
        pSprite->setScaleX(scaleX);
        pSprite->setScaleY(scaleY);
        pSprite->addTween(queuedAnim);
        bool matrixMode = mode == GameEnums::Recoloring_Matrix;
        if (pPlayer != nullptr)
        {
            pSprite->setColorTable(pPlayer->getColorTableAnim(), matrixMode);
        }
        else
        {
            pSprite->setColorTable(Player::getNeutralTableAnim(), matrixMode);
        }
        addChild(pSprite);
        pSprite->setPosition(offsetX, offsetY);
        if(!m_finishQueued)
        {
            m_finishQueued = true;
            queuedAnim->setDoneCallback([this](oxygine::Event *)->void
            {
                emitFinished();
            });
        }
    }
}

void GameAnimation::loadSpriteAnim(oxygine::ResAnim* pAnim, float offsetX, float offsetY, QColor color, qint32 sleepAfterFinish, float scaleX, float scaleY, qint32 delay, qint32 loops)
{
    oxygine::spSprite pSprite = oxygine::spSprite::create();
    oxygine::spTweenQueue queuedAnim = oxygine::spTweenQueue::create();
    qint32 totalFrames = 0;
    if (pAnim != nullptr)
    {
        totalFrames = pAnim->getTotalFrames();
    }
    oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), oxygine::timeMS(totalFrames * m_frameTime), loops, false, oxygine::timeMS(static_cast<qint64>(delay / Settings::getAnimationSpeed())));
    queuedAnim->add(tween);
    if (sleepAfterFinish > 0)
    {
        oxygine::spTween tween1 = oxygine::createTween(TweenWait(), oxygine::timeMS(static_cast<qint64>(sleepAfterFinish / Settings::getAnimationSpeed())), 1);
        queuedAnim->add(tween1);
    }
    if (pAnim != nullptr)
    {
        setSize(pAnim->getSize());
    }
    pSprite->setScaleX(scaleX);
    pSprite->setScaleY(scaleY);
    pSprite->addTween(queuedAnim);
    if (color != Qt::white)
    {
        pSprite->setColor(color);
    }
    addChild(pSprite);
    pSprite->setPosition(offsetX, offsetY);
    if(!m_finishQueued)
    {
        m_finishQueued = true;
        queuedAnim->setDoneCallback([this](oxygine::Event *)->void
        {
            emitFinished();
        });

    }
}

qint32 GameAnimation::addText(QString text, float offsetX, float offsetY, float scale, QColor color, qint32 priority)
{
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont72());
    style.color = color;
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;
    oxygine::spTextField pTextfield = oxygine::spTextField::create();
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(text);
    pTextfield->setPosition(offsetX, offsetY);
    pTextfield->setScale(scale * 24.0f / 72.0f);
    pTextfield->setPriority(priority);
    pTextfield->setWidth(pTextfield->getTextRect().getWidth() * pTextfield->getScaleX());
    pTextfield->setHeight(40);
    addChild(pTextfield);
    return pTextfield->getTextRect().getWidth() * pTextfield->getScaleX();
}

bool GameAnimation::onFinished(bool skipping)
{
    m_skipping |= skipping;
    GameAnimationFactory::removeAnimationFromQueue(spGameAnimation(this));
    if (m_skipping == skipping)
    {
        if (!m_started)
        {
            doPreAnimationCall();
        }
        else
        {
            for (auto & data : m_SoundData)
            {
                if (m_stopSoundAtAnimationEnd || skipping || data.loops < 0)
                {
                    Mainapp::getInstance()->getAudioThread()->stopSound(data.soundFile);
                }
            }
        }
        GameAnimationFactory* pGameAnimationFactory = GameAnimationFactory::getInstance();
        for (qint32 i = 0; i < m_QueuedAnimations.size(); i++)
        {
            pGameAnimationFactory->startQueuedAnimation(m_QueuedAnimations[i].get());
        }
        m_QueuedAnimations.clear();
        if ((!m_jsPostActionObject.isEmpty()) && (!m_jsPostActionObject.isEmpty()))
        {
            CONSOLE_PRINT("Calling post Animation function " + m_jsPostActionObject + "." + m_jsPostActionFunction, Console::eDEBUG);
            Interpreter* pInterpreter = Interpreter::getInstance();
            QJSValueList args({pInterpreter->newQObject(this),
                               pInterpreter->newQObject(m_pMap)});
            pInterpreter->doFunction(m_jsPostActionObject, m_jsPostActionFunction, args);
        }
    }
    for (auto & tween : m_stageTweens)
    {
        tween->complete();
    }
    m_stageTweens.clear();
    GameAnimationFactory::removeAnimation(this, m_skipping, false);
    if (!skipping && GameAnimationFactory::getAnimationCount() > 0)
    {
        GameAnimationFactory::skipAllAnimations();
        if (GameAnimationFactory::getAnimationCount() == 0)
        {
            emit GameAnimationFactory::getInstance()->animationsFinished();
        }
    }
    return true;
}

void GameAnimation::setSound(QString soundFile, qint32 loops, qint32 delayMs, float volume, bool stopOldestSound)
{
    if (m_SoundData.size() == 0)
    {
        addSound(soundFile, loops, delayMs, volume);
    }
    else
    {
        SoundData data;
        data.soundFile = soundFile;
        data.loops = loops;
        data.volume = volume;
        data.delayMs = delayMs;
        data.stopOldestSound = stopOldestSound;
        m_SoundData[0] = data;
    }
}

void GameAnimation::addSound(QString soundFile, qint32 loops, qint32 delayMs, float volume, bool stopOldestSound)
{
    SoundData data;
    data.soundFile = soundFile;
    data.loops = loops;
    data.volume = volume;
    data.delayMs = delayMs;
    data.stopOldestSound = stopOldestSound;
    m_SoundData.append(data);
}

void GameAnimation::addTweenScale(float endScale, qint32 duration)
{
    oxygine::spTween tween1 = oxygine::createTween(oxygine::Actor::TweenScale(endScale), oxygine::timeMS(static_cast<qint64>(duration / Settings::getAnimationSpeed())));
    addTween(tween1);
}

void GameAnimation::addTweenPosition(QPoint point, qint32 duration)
{
    oxygine::spTween tween1 = oxygine::createTween(oxygine::Actor::TweenPosition(oxygine::Vector2(point.x(), point.y())), oxygine::timeMS(static_cast<qint64>(duration/ Settings::getAnimationSpeed())));
    addTween(tween1);
}

void GameAnimation::addTweenColor(qint32 spriteIdx, QColor startColor, QColor endColor, qint32 duration, bool twoSided, qint32 delay)
{
    if (spriteIdx < m_children.size())
    {
        oxygine::spActor actor = m_children[spriteIdx];
        oxygine::Sprite* sprite = dynamic_cast<oxygine::Sprite*>(actor.get());
        if (sprite != nullptr)
        {
            sprite->setColor(startColor);
            oxygine::Sprite::TweenColor tweenColor(endColor);
            oxygine::spTween tween = oxygine::createTween(tweenColor, oxygine::timeMS(static_cast<qint64>(duration / Settings::getAnimationSpeed())), 1, twoSided, oxygine::timeMS(static_cast<qint64>(delay / Settings::getAnimationSpeed())));
            sprite->addTween(tween);
        }
    }
}

void GameAnimation::addTweenWait(qint32 duration)
{
    oxygine::spTween tween1 = oxygine::createTween(TweenWait(), oxygine::timeMS(static_cast<qint64>(duration / Settings::getAnimationSpeed())), 1);
    addTween(tween1);
    if(!m_finishQueued)
    {
        m_finishQueued = true;
        tween1->setDoneCallback([this](oxygine::Event *)->void
        {
            emitFinished();
        });
    }
}

void GameAnimation::setEndOfAnimationCall(QString postActionObject, QString postActionFunction)
{
    m_jsPostActionObject = postActionObject;
    m_jsPostActionFunction = postActionFunction;
}

void GameAnimation::setStartOfAnimationCall(QString preActionObject, QString preActionFunction)
{
    m_jsPreActionObject = preActionObject;
    m_jsPreActionFunction = preActionFunction;
}

void GameAnimation::emitFinished()
{
    if (!m_skipping)
    {
        emit sigFinished(m_skipping);
    }
}

void GameAnimation::addScreenshake(qint32 startIntensity, float decay, qint32 durationMs, qint32 delayMs, qint32 shakePauseMs)
{
    oxygine::spTween tween = oxygine::createTween(TweenScreenshake(startIntensity, decay / Settings::getAnimationSpeed(), oxygine::timeMS(shakePauseMs)),
                                                  oxygine::timeMS(static_cast<qint64>(durationMs / Settings::getAnimationSpeed())), 1, false, oxygine::timeMS(static_cast<qint64>(delayMs / Settings::getAnimationSpeed())));
    m_stageTweens.append(tween);
    oxygine::Stage::getStage()->addTween(tween);

}
