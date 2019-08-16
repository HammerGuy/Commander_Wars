#ifndef SCRIPTCONDITIONUNITDESTROYED_H
#define SCRIPTCONDITIONUNITDESTROYED_H

#include "scriptcondition.h"


class ScriptConditionUnitDestroyed;
typedef oxygine::intrusive_ptr<ScriptConditionUnitDestroyed> spScriptConditionUnitDestroyed;

class ScriptConditionUnitDestroyed : public ScriptCondition
{
    Q_OBJECT
public:
    ScriptConditionUnitDestroyed();

    /**
     * @brief readCondition
     * @param rStream
     */
    virtual void readCondition(QTextStream& rStream) override;
    /**
     * @brief writeCondition
     * @param rStream
     */
    virtual void writeCondition(QTextStream& rStream) override;
    /**
     * @brief writePreCondition
     * @param rStream
     */
    virtual void writePreCondition(QTextStream& rStream) override;
    /**
     * @brief writePostCondition
     * @param rStream
     */
    virtual void writePostCondition(QTextStream& rStream) override;
    /**
     * @brief getDescription
     * @return
     */
    virtual QString getDescription() override
    {
        return tr("Unit Destroyed X: ") + QString::number(m_x) + " Y: " + QString::number(m_y);
    }
    /**
     * @brief showEditConditin
     */
    virtual void showEditCondition(spScriptEditor pScriptEditor) override;
    /**
     * @brief getX
     * @return
     */
    qint32 getX() const;
    /**
     * @brief setX
     * @param x
     */
    void setX(const qint32 &x);
    /**
     * @brief getY
     * @return
     */
    qint32 getY() const;
    /**
     * @brief setY
     * @param y
     */
    void setY(const qint32 &y);

private:
    qint32 m_x{0};
    qint32 m_y{0};
    QString m_executed;
    QString m_unitID;
};

#endif // SCRIPTCONDITIONUNITDESTROYED_H
