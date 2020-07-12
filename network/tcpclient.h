#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "network/NetworkInterface.h"

#include "network/rxtask.h"
#include "network/txtask.h"

class QTcpSocket;

class TCPClient : public NetworkInterface
{
    Q_OBJECT
public:
    TCPClient();
    virtual ~TCPClient();

public slots:
    virtual void connectTCP(QString adress, quint16 port) override;
    virtual void disconnectTCP() override;
    virtual QVector<quint64> getConnectedSockets() override;
    virtual void changeThread(quint64 socketID, QThread* pThread) override;
protected slots:
    void connected();
private:
    spRxTask pRXTask;
    spTxTask pTXTask;
    QTcpSocket* pSocket;
};

#endif // TCPCLIENT_H
