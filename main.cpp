#include <QApplication>
#include <QObject>
#include <QPixmap>
#include <QImage>
#include <QDialog>
#include <QLabel>

#include "framebroadcast.h"

/*static Frame* CopyFrame(const Frame *incomingFrame)
{
    Frame *frame = new Frame();
    frame->width=incomingFrame->width;
    frame->height=incomingFrame->height;
    frame->lenght=incomingFrame->lenght;
    frame->buffer=new unsigned char[frame->lenght];

    std::memcpy(frame->buffer,incomingFrame->buffer,frame->lenght);
    return frame;
}

static Frame* CopyFrame(const QSharedPointer<Frame> &incomingFrame)
{
    return CopyFrame(incomingFrame.data());
}*/


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDialog *dialog = new QDialog();
    QLabel *label = new QLabel(dialog);

    FrameBroadcast *cast = new FrameBroadcast();
    QObject::connect(cast, &FrameBroadcast::frameCaptured, [=](const QSharedPointer<Frame> &frame) {

        int w = static_cast<int>(frame.data()->width);
        int h = static_cast<int>(frame.data()->height);

        QImage img(frame.data()->buffer,w,h,QImage::Format_RGBA8888);
        label->setPixmap(QPixmap::fromImage(img));
        label->resize(w,h);

        qDebug() << "Update";
    });
    cast->startCapture();

    dialog->show();

    return app.exec();
}
