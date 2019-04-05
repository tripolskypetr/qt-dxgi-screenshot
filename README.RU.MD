# QPixmap не делает скрин окна веб браузера

Обсуждение на StackOverflow велось [тут][6]

Вопрос:

 >Когда я использую QPixmap::GrabWindow(WId) на окне с веб
 >браузером (например, Chrome), он возвращает мне просто черный
 >экран. Использую следующий код:
 >QScreen *screen = QGuiApplication::primaryScreen();
 >m_pixmap = screen->grabWindow(hW);
 >m_image = m_pixmap.toImage();
 >m_image.save("p.png");
 >p.png - получается просто черная картинка. С другими окнами
 >получается нормально (например, скрин проводника). Как можно
 >сделать скрин окна браузера?

Ответ:

Дело в том, что QScreen::grabWindow использует [Windows GDI][1] для захвата картинки. Это достаточно древнее API, которое используют программы без аппаратного ускорения (рисуются процессором). А хром - ПО не древнее и уже давно рисуется средствами [Windows DXGI][2].

Я уже писал ПО, которое использует эту технологию. Опубликовал пример кода [тут][3]. Собирается компилятором MSVC на библиотеке Qt 5.10, вроде бы без разницы, 2015 или 2017. Моя машина 64 битная, возможно это тоже важно.

![Screenshot](screenshot.png "Screenshot")


Внутри два класса: FrameBroadcast и FrameCapturer. FrameBroadcast запрашивает скриншот с некоторым временным интервалом из FrameCapturer и через сигнал `void frameCaptured(QSharedPointer<Frame> frame);` рассылает подписчикам. QSharedPointer автоматически удаляет память, выделенную под содержимое экрана, как только выходит из области видимости всех обработчиков слота.

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

В main.cpp создается простенькое диалоговое окно, на которое выводится результат захвата. На всякий случай я приложил код, который отвязывает содержимое экрана от QSharedPointer, если разместить все манипуляции в одном слоте не удастся. Он сразу после инклудов и закомментирован.

    #pragma comment(lib,"dxgi.lib")
    #pragma comment(lib,"D3D11.lib")
    #pragma comment(lib,"Shcore.lib")
    #pragma comment(lib,"winmm.lib")
    #pragma comment(lib,"windowscodecs.lib")
    #pragma comment (lib, "user32.lib")
    #pragma comment (lib, "dxguid.lib")

Детально разбирать код нет смысла. Он слишком большой, но переоборудовать под свои нужды труда не составит. Примечательно, что используется "[Автосвязывание][5]" - фишка компилятора Microsoft: необходимые библиотеки сами подтянутся на этапе компиляции (загляните в framecapturer.h)


  [1]: https://ru.wikipedia.org/wiki/GDI
  [2]: https://en.wikipedia.org/wiki/DirectX_Graphics_Infrastructure
  [3]: https://github.com/tripolskypetr/qt-dxgi-screenshot
  [4]: https://i.stack.imgur.com/DWF3Y.png
  [5]: https://ru.wikipedia.org/wiki/%D0%90%D0%B2%D1%82%D0%BE%D1%81%D0%B2%D1%8F%D0%B7%D1%8B%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5
  [6]: https://ru.stackoverflow.com/a/965903/233158