# QPixmap doesn't grab web browser window

From [here][6]

Question:

>When I use QPixmap::GrabWindow(WId) with web browser window it returns me just black screen.
>I'm using the following code:
>QScreen *screen = QGuiApplication::primaryScreen();
>m_pixmap = screen->grabWindow(hW);
>m_image = m_pixmap.toImage();
>m_image.save("p.png");
>When I open "p.png" it is just black picture. With other windows this work well.
>How can I take a normal screen of browser?

Answer:

The fact is that QScreen :: grabWindow uses [Windows GDI] [1] to capture the image. This is a rather ancient API that is used by programs without hardware acceleration (drawn by the processor). And chrome - the software is not ancient and has long been drawn by means of [Windows DXGI] [2].

I already wrote software that uses this technology. Published sample code [here] [3]. It is going to be compiled by the MSVC compiler on the Qt 5.10 library, seemingly no difference, 2015 or 2017. My machine is 64 bit, maybe this is also important.

![screenshot](./screenshot.png)

Inside there are two classes: FrameBroadcast and FrameCapturer. FrameBroadcast requests a screenshot with a certain time interval from FrameCapturer and sends the subscriber via the signal `void frameCaptured (QSharedPointer <Frame> frame);` QSharedPointer automatically deletes the memory allocated for the screen contents as soon as it goes out of scope for all slot handlers.

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

In main.cpp, a simple dialog box is created, which displays the result of the capture. Just in case, I attached a code that untie the contents of the screen from QSharedPointer, if it is not possible to place all the manipulations in one slot. It is immediately after the inclusions and commented out.

    #pragma comment(lib,"dxgi.lib")
    #pragma comment(lib,"D3D11.lib")
    #pragma comment(lib,"Shcore.lib")
    #pragma comment(lib,"winmm.lib")
    #pragma comment(lib,"windowscodecs.lib")
    #pragma comment (lib, "user32.lib")
    #pragma comment (lib, "dxguid.lib")

It makes no sense to parse the code in detail. It is too big, but it will not be difficult to retool to fit your needs. It is noteworthy that "[Auto-linking] [5]" is used - the Microsoft compiler feature: the necessary libraries will pull themselves up at compile time (look in framecapturer.h)


  [1]: https://ru.wikipedia.org/wiki/GDI
  [2]: https://en.wikipedia.org/wiki/DirectX_Graphics_Infrastructure
  [3]: https://github.com/tripolskypetr/qt-dxgi-screenshot
  [5]: https://ru.wikipedia.org/wiki/%D0%90%D0%B2%D1%82%D0%BE%D1%81%D0%B2%D1%8F%D0%B7%D1%8B%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5
  [6]: https://stackoverflow.com/a/55535047/7388307
