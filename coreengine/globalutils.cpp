#include "globalutils.h"

#include "game/gamemap.h"
#include "coreengine/console.h"
#include "coreengine/interpreter.h"
#include <QFileInfo>
#include <QDirIterator>

GlobalUtils GlobalUtils::m_pInstace = GlobalUtils();

QRandomGenerator GlobalUtils::randGenerator;
bool GlobalUtils::m_useSeed{false};
quint32 GlobalUtils::m_seed = 0;

GlobalUtils::GlobalUtils()
{
    Interpreter::setCppOwnerShip(this);
    quint32 seedValue = QRandomGenerator::global()->bounded(0u, std::numeric_limits<quint32>::max());
    randGenerator.seed(seedValue);
}

void GlobalUtils::seed(quint32 seed)
{
    m_seed = seed;
    randGenerator.seed(seed);
    Console::print("Seeding with " + QString::number(m_seed), Console::eDEBUG);
}

quint32 GlobalUtils::getSeed()
{
    return m_seed;
}

qint32 GlobalUtils::randInt(qint32 low, qint32 high)
{
    if (high <= low)
    {
        return low;
    }
    if (m_useSeed)
    {
        return randGenerator.bounded(low, high + 1);
    }
    else
    {
        return randIntBase(low, high);
    }
}


qint32 GlobalUtils::randIntBase(qint32 low, qint32 high)
{
    if (high <= low)
    {
        return low;
    }
    return QRandomGenerator::global()->bounded(low, high + 1);
}

qint32 GlobalUtils::roundUp(float value)
{
    qint32 roundDown = static_cast<qint32>(value);
    // little cheat
    if (static_cast<float>(roundDown) == value)
    {
        return roundDown;
    }
    else
    {
        if (value > 0)
        {
            return roundDown + 1;
        }
        else
        {
            return roundDown;
        }
    }
}

qint32 GlobalUtils::roundDown(float value)
{
    qint32 roundDown = static_cast<qint32>(value);
    return roundDown;
}

QmlVectorPoint* GlobalUtils::getCircle(qint32 min, qint32 max)
{
    QmlVectorPoint* ret = new QmlVectorPoint();
    qint32 x2 = 0;
    qint32 y2 = 0;

        for (qint32 currentRadius = min; currentRadius <= max; currentRadius++)
        {
            x2 = -currentRadius;
            y2 = 0;
            if (currentRadius == 0)
            {
                ret->append(QPoint(0, 0));
            }
            else
            {
                for (qint32 i = 0; i < currentRadius; i++)
                {
                    x2 += 1;
                    y2 += 1;
                    ret->append(QPoint(x2, y2));
                }
                for (qint32 i = 0; i < currentRadius; i++)
                {
                    x2 += 1;
                    y2 -= 1;
                    ret->append(QPoint(x2, y2));
                }
                for (qint32 i = 0; i < currentRadius; i++)
                {
                    x2 -= 1;
                    y2 -= 1;
                    ret->append(QPoint(x2, y2));
                }
                for (qint32 i = 0; i < currentRadius; i++)
                {
                    x2 -= 1;
                    y2 += 1;
                    ret->append(QPoint(x2, y2));
                }
            }
        }
    return ret;
}

QmlVectorPoint* GlobalUtils::getShotFields(qint32 min, qint32 max, qint32 xDirection, qint32 yDirection)
{
    QmlVectorPoint* ret = new QmlVectorPoint();
    for (qint32 i = min; i <= max; i++)
    {
        if (xDirection > 0)
        {
            for (qint32 y = -i + 1; y < i; y++)
            {
                ret->append(QPoint(i, y));
            }
        }
        else if (xDirection < 0)
        {
            for (qint32 y = -i + 1; y < i; y++)
            {
                ret->append(QPoint(-i, y));
            }
        }

        if (yDirection > 0)
        {
            for (qint32 x = -i + 1; x < i; x++)
            {
                ret->append(QPoint(x, i));
            }
        }
        else if (yDirection < 0)
        {
            for (qint32 x = -i + 1; x < i; x++)
            {
                ret->append(QPoint(x, -i));
            }
        }
    }
    return ret;
}

void GlobalUtils::setImageSize(qint32 value)
{
    GameMap::setImagesize(value);
}

qint32 GlobalUtils::getDistance(QPoint p1, QPoint p2)
{
    return qAbs(p1.x() - p2.x()) + qAbs(p1.y() - p2.y());
}

QmlVectorPoint* GlobalUtils::getEmptyPointArray()
{
    return new QmlVectorPoint();
}

bool GlobalUtils::isEven(qint32 value)
{
    float val = value/2.0f;
    qint32 val2 = value/2;
    if(val == val2)
    {
        return true;
    }
    else
    {
        return false;
    }
}


bool GlobalUtils::getUseSeed()
{
    return m_useSeed;
}

void GlobalUtils::setUseSeed(bool useSeed)
{
    m_useSeed = useSeed;
}

QStringList GlobalUtils::getFiles(QString folder, QStringList filter)
{
    QStringList ret;
    QDirIterator dirIter(folder, filter, QDir::Files, QDirIterator::Subdirectories);
    while (dirIter.hasNext())
    {
        dirIter.next();
        QString file = dirIter.fileInfo().absoluteFilePath();
        file.replace(QCoreApplication::applicationDirPath() + "/", "");
        file.replace(QCoreApplication::applicationDirPath(), "");
        file.replace(folder + "/", "");
        file.replace(folder, "");
        ret.append(file);
    }
    return ret;
}
