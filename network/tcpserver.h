#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QVector>
#include "network/NetworkInterface.h"

class QTcpServer;
class QTcpSocket;

#include "network/rxtask.h"
#include "network/txtask.h"

class TCPServer;
typedef oxygine::intrusive_ptr<TCPServer> spTCPServer;

class TCPServer : public NetworkInterface
{
    Q_OBJECT
public:
    TCPServer();
    virtual ~TCPServer();
public slots:
    virtual void connectTCP(QString adress, quint16 port) override;
    virtual void disconnectTCP() override;
    virtual void forwardData(quint64 socketID, QByteArray data, NetworkInterface::NetworkSerives service) override;
    virtual QVector<quint64> getConnectedSockets() override;

    void disconnectSocket();
    void onConnect();
    void disconnectClient(quint64 socketID);
    void pauseListening();
    void continueListening();
    virtual void changeThread(quint64 socketID, QThread* pThread) override;
private:
    QVector<spRxTask> pRXTasks;
    QVector<spTxTask> pTXTasks;
    QVector<QTcpSocket*> pTCPSockets;
    QVector<quint64> m_SocketIDs;
    quint64 m_idCounter = 0;
    QTcpServer* pTCPServer{nullptr};
    bool m_gameServer{false};
};

#endif // TCPSERVER_H
