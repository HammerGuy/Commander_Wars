#pragma once
#include "3rd_party/oxygine-framework/oxygine/oxygine-forwards.h"
#include "3rd_party/oxygine-framework/oxygine/tween/Tween.h"
#include "3rd_party/oxygine-framework/oxygine/EventDispatcher.h"
#include "3rd_party/oxygine-framework/oxygine/TouchEvent.h"
#include "3rd_party/oxygine-framework/oxygine/core/Object.h"
#include "3rd_party/oxygine-framework/oxygine/math/AffineTransform.h"
#include "3rd_party/oxygine-framework/oxygine/math/Rect.h"
#include "3rd_party/oxygine-framework/oxygine/Clock.h"
#include <vector>

namespace oxygine
{
    class TweenOptions
    {
    public:
        explicit TweenOptions(timeMS duration = timeMS(500))
            : m_duration(duration),
              m_delay(timeMS(0)),
              m_ease(QEasingCurve::Linear),
              m_globalEase(QEasingCurve::Linear),
              m_loops(1),
              m_twoSides(false),
              m_detach(false)
        {
        }
        virtual ~TweenOptions() = default;
        TweenOptions& duration(timeMS duration)
        {
            m_duration = duration;
            return *this;
        }
        TweenOptions& delay(timeMS delay)
        {
            m_delay = delay;
            return *this;
        }
        TweenOptions& loops(qint32 loops)
        {
            m_loops = loops;
            return *this;
        }
        TweenOptions& twoSides(bool enabled = true)
        {
            m_twoSides = enabled;
            return *this;
        }
        TweenOptions& ease(QEasingCurve::Type ease)
        {
            m_ease = ease;
            return *this;
        }
        TweenOptions& detach(bool detach_ = true)
        {
            m_detach = detach_;
            return *this;
        }
        TweenOptions& globalEase(QEasingCurve::Type ease)
        {
            m_globalEase = ease;
            return *this;
        }
        TweenOptions& doneCallback(const EventCallback& cb)
        {
            m_callback = cb;
            return *this;
        }

        EventCallback   m_callback;
        timeMS          m_duration;
        timeMS          m_delay;
        QEasingCurve::Type     m_ease;
        QEasingCurve::Type     m_globalEase;
        qint32          m_loops;
        bool            m_twoSides;
        bool            m_detach;
    };

    class Actor;
    using spActor = intrusive_ptr<Actor>;
    class Actor: public EventDispatcher
    {
    public:
        explicit Actor();
        virtual ~Actor();
        using children = std::vector<spActor>;
        using tweens = std::vector<spTween>;

        children& getChildren()
        {
            return m_children;
        }
        oxygine::spActor getFirstChild()
        {
            return m_children.front();
        }
        oxygine::spActor getLastChild()
        {
            return m_children.back();
        }
        tweens& getTweens()
        {
            return m_tweens;
        }

        const Vector2& getAnchor() const
        {
            return m_anchor;
        }
        float getAnchorX() const
        {
            return m_anchor.x;
        }
        float getAnchorY() const
        {
            return m_anchor.y;
        }
        bool getIsAnchorInPixels()
        {
            return (m_flags & flag_anchorInPixels) != 0;
        }
        const Vector2& getPosition() const
        {
            return m_pos;
        }
        float getX() const
        {
            return m_pos.x;
        }
        float getY() const
        {
            return m_pos.y;
        }
        const Vector2& getScale() const
        {
            return m_scale;
        }
        float getScaleX() const
        {
            return m_scale.x;
        }
        float getScaleY() const
        {
            return m_scale.y;
        }
        /**Returns rotation angle in radians*/
        float getRotation() const
        {
            return m_rotation;
        }
        /**Returns rotation angle in degrees*/
        float getRotationDegrees() const
        {
            return m_rotation / M_PI * 180.0f;
        }
        qint32 getPriority() const
        {
            return m_zOrder;
        }
        virtual bool getVisible() const
        {
            return (m_flags & flag_visible) != 0;
        }
        Actor* getParent()
        {
            return m_parent;
        }
        const Actor* getParent() const
        {
            return m_parent;
        }
        const Vector2& getSize() const
        {
            return m_size;
        }
        /**Returns Size*Scale*/
        Vector2 getScaledSize() const
        {
            return m_size.mult(m_scale);
        }
        float getWidth() const;
        float getScaledWidth() const
        {
            return m_size.x * m_scale.x;
        }
        float getHeight() const;
        float getScaledHeight() const
        {
            return m_size.y * m_scale.y;
        }
        unsigned char getAlpha() const;
        const spClock& getClock() const;
        virtual RectF getDestRect() const;
        /**returns touch id if actor is pressed down*/
        pointer_index getPressed(MouseButton b = MouseButton_Touch) const;
        /**returns touch id if actor is moused overred*/
        pointer_index getOvered() const;        
        RenderDelegate* getRenderDelegate()
        {
            return m_rdelegate;
        }

        /**return local actor transformation*/
        const AffineTransform& getTransform() const;
        const AffineTransform& getTransformInvert() const;
        /**computes global actor transformation*/
        AffineTransform computeGlobalTransform(Actor* parent = nullptr) const;
        /**computes actor Bounds rectangle. Iterates children*/
        RectF computeBounds(const AffineTransform& transform = AffineTransform()) const;

        /**Sets Anchor. Anchor also called Pivot point. It is "center" for rotation/scale/position. Anchor could be set in Pixels or in Percents (/100).
        Default value is (0,0) - top left corner of Actor
        */
        void setAnchor(const Vector2& anchor);
        void setAnchor(float ax, float ay);
        void setAnchorInPixels(const Vector2& anchor);
        void setAnchorInPixels(float x, float y);
        void setAnchorX(float x);
        void setAnchorY(float y);

        void setPosition(const Vector2& pos);
        void setPosition(float x, float y);
        void setX(float x);
        void setY(float y);

        /**Overwrites transformation matrix. position/scale/rotation would be ignored until you change them*/
        void setTransform(const AffineTransform& tr);
        /** set z order draw priority, from back (low value) to front (high value). Max value is 32000, Min value -32000*/
        void setPriority(qint32 zorder);
        void setScale(float scale);
        void setScale(const Vector2& scale);
        void setScale(float scaleX, float scaleY);
        void setScaleX(float sx);
        void setScaleY(float sy);
        /**Sets rotation angle in radians*/
        void setRotation(float angle);
        /**Sets rotation angle in degrees. Converts internally to radians. (use setRotation!)*/
        void setRotationDegrees(float angle);

        /**Sets Size of Actor. Size doesn't scale contents of Actor. Size only affects event handling and rendering if you change Anchor*/
        void setSize(const Vector2&);
        void setSize(float w, float h);
        virtual void setWidth(float w);
        virtual void setHeight(float h);
        /**Extends actor's clickable area from each side. Affects only to Actor::isOn. Max value is 127, Min Value is -128*/
        void setExtendedClickArea(char add) {m_extendedIsOn = add;}

        void setClock(spClock & clock);
        void setRenderDelegate(RenderDelegate* mat);

        /**Show/Hide actor and children. Invisible Actor doesn't receive Touch events.*/
        virtual void setVisible(bool vis)
        {
            m_flags &= ~flag_visible;
            if (vis)
            {
                m_flags |= flag_visible;
            }
        }
        /**Enable/Disable culling this actor outside of clip area (use it in pair with ClipRectActor)*/
        void setCull(bool enable) {m_flags &= ~flag_cull; if (enable) m_flags |= flag_cull;}
        /**Sets transparency. if alpha is 0 actor and children are completely invisible. Invisible Actor doesn't receive Touch events.*/
        void setAlpha(unsigned char alpha);

        /**By default Actor doesn't has bounds, this will set it to Actor::getDestRect*/
        void setHasOwnBounds(bool enable = true)
        {
            m_flags &= ~flag_actorHasBounds;
            if (enable)
            {
                m_flags |= flag_actorHasBounds;
            }
        }
        /**by default actor with Alpha = 0 not clickable*/
        void setClickableWithZeroAlpha(bool enable)
        {
            m_flags &= ~flag_clickableWithZeroAlpha;
            if (enable)
            {
                m_flags |= flag_clickableWithZeroAlpha;
            }
        }        
        virtual bool isOn(const Vector2& localPosition, float localScale = 1.0f);
        /**Returns true if actor is child or located deeper in current subtree*/
        bool isDescendant(const spActor& actor) const;
        void addChild(spActor actor);
        /**Remove one child*/
        void removeChild(spActor & actor);
        /**Removes all children from Actor*/
        void removeChildren();
        /**detaches actor from parent and returns parent. return NULL If actor doesn't have parent*/
        Actor* detach();
        /**Dispatches an event into the event flow. The event target is the EventDispatcher object upon which the dispatchEvent() method is called.*/
        virtual void dispatchEvent(Event* event) override;

        spTween addTween(spTween);
        template<class TProperty>
        spTween addTween(const TProperty& property, timeMS duration, qint32 loops = 1, bool twoSides = false, timeMS delay = timeMS(0), QEasingCurve::Type ease = QEasingCurve::Linear)
        {
            return addTween(createTween(property, duration, loops, twoSides, delay, ease));
        }
        /**short syntax version of actor->addEventListener(TouchEvent::CLICK, ...);*/
        qint32 addClickListener(const EventCallback& cb)
        {
            return addEventListener(TouchEvent::CLICK, cb);
        }
        /**short syntax version of actor->addEventListener(TouchEvent::TOUCH_DOWN, ...);*/
        qint32 addTouchDownListener(const EventCallback& cb)
        {
            return addEventListener(TouchEvent::TOUCH_DOWN, cb);
        }
        /**short syntax version of actor->addEventListener(TouchEvent::TOUCH_UP, ...);*/
        qint32 addTouchUpListener(const EventCallback& cb)
        {
            return addEventListener(TouchEvent::TOUCH_UP, cb);
        }

        void removeTween(spTween);
        /**remove all tweens and call Tween::complete to them if callComplete == true*/
        void removeTweens(bool callComplete = false);

        /**Updates this actor, children and all tweens.*/
        virtual void update(const UpdateState& us);
        /**Renders this actor and children.*/
        virtual void render(const RenderState& rs);
        virtual void handleEvent(Event* event);
        virtual void doRender(const RenderState&) {}

        //converts position in parent space to local space
        Vector2 parent2local(const Vector2& pos) const;
        //converts local position to parent space
        Vector2 local2parent(const Vector2& pos = Vector2()) const;

        //converts local position to Stage
        Vector2 local2stage(const Vector2& pos = Vector2(), Actor* stage = nullptr) const;
        Vector2 local2stage(float x, float y, Actor* stage = nullptr) const;
        //converts global position (position in Stage space) to local space
        Vector2 stage2local(const Vector2& pos = Vector2(), Actor* stage = nullptr) const;
        Vector2 stage2local(float x, float y, Actor* stage = nullptr) const;

        /**Returns Stage where Actor attached to. Used for multi stage (window) mode*/
        Stage* __getStage();

        void setNotPressed(MouseButton b);

        bool internalRender(RenderState& rs, const RenderState& parentRS);
        virtual bool getBounds(RectF&) const;
        static Vector2 convert_local2stage(spActor & child, const Vector2& pos, spActor root = spActor());
        static Vector2 convert_local2stage(const Actor* child, const Vector2& pos, const Actor* root = nullptr);
        static Vector2 convert_stage2local(spActor & child, const Vector2& pos, spActor root = spActor());
        static Vector2 convert_stage2local(const Actor* child, const Vector2& pos, const Actor* root = nullptr);
        static RectF getActorTransformedDestRect(Actor* actor, const AffineTransform& tr);
        /*****************************************************************************************/
        // properties for tweens
        /*****************************************************************************************/
        typedef Property2Args<float, Vector2, const Vector2&, Actor, &Actor::getPosition, &Actor::setPosition>  TweenPosition;
        typedef Property<float, float, Actor, &Actor::getX, &Actor::setX>                                       TweenX;
        typedef Property<float, float, Actor, &Actor::getY, &Actor::setY>                                       TweenY;
        typedef Property<float, float, Actor, &Actor::getWidth, &Actor::setWidth>                               TweenWidth;
        typedef Property<float, float, Actor, &Actor::getHeight, &Actor::setHeight>                             TweenHeight;
        typedef Property2Args2<float, Vector2, const Vector2&, const Vector2&, Actor, &Actor::getSize, &Actor::setSize>TweenSize;
        typedef Property<float, float, Actor, &Actor::getRotation, &Actor::setRotation>                         TweenRotation;
        typedef Property<float, float, Actor, &Actor::getRotationDegrees, &Actor::setRotationDegrees>           TweenRotationDegrees;
        typedef Property2Args1Arg<float, Vector2, const Vector2&, Actor, &Actor::getScale, &Actor::setScale>    TweenScale;
        typedef Property2Args1Arg<float, Vector2, const Vector2&, Actor, &Actor::getAnchor, &Actor::setAnchor>  TweenAnchor;
        typedef Property<float, float, Actor, &Actor::getScaleX, &Actor::setScaleX>                             TweenScaleX;
        typedef Property<float, float, Actor, &Actor::getScaleY, &Actor::setScaleY>                             TweenScaleY;
        typedef Property<unsigned char, unsigned char, Actor, &Actor::getAlpha, &Actor::setAlpha>               TweenAlpha;
    protected:
        void added2stage(Stage*);
        void removedFromStage();
        void calcBounds2(RectF& bounds, const AffineTransform& transform) const;
        void calcChildrenBounds(RectF& bounds, const AffineTransform& transform) const;
        static void setParent(Actor* actor, Actor* parent);
        void _onGlobalTouchUpEvent(Event*);
        void _onGlobalTouchMoveEvent(Event*);
        const Vector2& _getSize() const
        {
            return m_size;
        }
        void __setSize(const Vector2&);
        virtual void sizeChanged(const Vector2& size);
        spTween __addTween(spTween tween, bool rel);
        bool prepareRender(RenderState& rs, const RenderState& parentRS);
        bool onScreen(RenderState& rs);
        void completeRender(const RenderState& rs);
        void markTranformDirty();
        void updateTransform() const;
        void internalUpdate(const UpdateState& us);
        /**doUpdate is virtual method for overloading in inherited classes. UpdateState struct has local time of Actor (relative to Clock) and delta time.*/
        virtual void doUpdate(const UpdateState& us);
        void dispatchToParent(Event* event);
        void insertActor(spActor & actor);
        static Vector2 convert_global2local_(const Actor* child, const Actor* parent, Vector2 pos);
        static Vector2 convert_global2local(spActor & child, spActor & parent, const Vector2& pos);
        static Vector2 convert_local2global_(const Actor* child, const Actor* parent, Vector2 pos);
        static Vector2 convert_local2global(spActor & child, spActor & parent, const Vector2& pos);
    protected:
        RenderDelegate* m_rdelegate;
        Stage* m_stage;
        mutable AffineTransform m_transform;
        mutable AffineTransform m_transformInvert;
        enum flags
        {
            flag_anchorInPixels         = 1,
            flag_visible                = 1 << 1,
            flag_transformDirty         = 1 << 2,
            flag_transformInvertDirty   = 1 << 3,
            flag_cull                   = 1 << 4,
            flag_fastTransform          = 1 << 5,
            flag_boundsNoChildren       = 1 << 6,
            flag_actorHasBounds         = 1 << 7,
            flag_clickableWithZeroAlpha = 1 << 8,
            flag_reserved               = 1 << 9,
            flag_last                   = flag_reserved
        };
        mutable quint32 m_flags;
        unsigned char   m_alpha;
        char    m_extendedIsOn;
        spClock m_clock;
        Actor* m_parent;
        tweens m_tweens;
        children m_children;
        union
        {
            struct
            {
                pointer_index m_overred;
                pointer_index m_pressedButton[MouseButton_Num];
            };
            int32_t m_pressedOvered;
        };
    private:
        Vector2 m_pos;
        Vector2 m_anchor;
        Vector2 m_scale;
        Vector2 m_size;
        float  m_rotation;
        qint32 m_zOrder;
        qint32 m_onGlobalTouchUpEvent{-1};
        qint32 m_onGlobalTouchMoveEvent{-1};
        bool m_onScreen{true};

    };

}
