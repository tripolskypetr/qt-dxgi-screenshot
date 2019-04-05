#include "framebroadcast.h"

FrameBroadcast::FrameBroadcast(QObject *parent) : QObject(parent)
{
    timer = new QTimer();
    timer->setInterval(500);
    timer->setTimerType(Qt::TimerType::VeryCoarseTimer);
    timer->setSingleShot(false);

    connect(timer,SIGNAL(timeout()),this,SLOT(tick()));
}

void FrameBroadcast::startCapture()
{
    timer->start();
}

void FrameBroadcast::stopCapture()
{
    timer->stop();
}

static void deleteFrame(Frame *obj)
{
    delete obj->buffer;
    delete obj;
}

void FrameBroadcast::tick()
{
    QSharedPointer<Frame> frame(new Frame,deleteFrame);
    frame->height=getHeight();
    frame->width=getWidth();
    frame->lenght=getLenght();

    while(true)
    {
        frame->buffer=new unsigned char[frame->lenght];

        if(tryCaptureFrame(frame->buffer))
        {
            emit frameCaptured(frame);
            break;
        }
        else
        {
            delete frame->buffer;
        }
    }
}
