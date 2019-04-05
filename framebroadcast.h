#ifndef FRAMEBROADCAST_H
#define FRAMEBROADCAST_H

#include <QObject>
#include <QSharedPointer>
#include <QTimer>

#include "framecapturer.h"

struct Frame
{
    uint height;
    uint width;
    uint lenght;
    unsigned char* buffer;
};

class FrameBroadcast : public QObject, FrameCapturer
{
    Q_OBJECT

private:
    QTimer *timer;

private slots:
    void tick();

public:
    explicit FrameBroadcast(QObject *parent = nullptr);

public slots:
    void startCapture();
    void stopCapture();

signals:
    void frameCaptured(QSharedPointer<Frame> frame);

};

#endif // FRAMEBROADCAST_H
