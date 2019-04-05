#ifndef FRAMECAPTURER_H
#define FRAMECAPTURER_H

#include <QtDebug>
#include <QMessageBox>
#include <QApplication>

#include <Windows.h>
#include <utilapiset.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi.h>

#include <dxgi.h>
#include <d3d11.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <Psapi.h>

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"Shcore.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"windowscodecs.lib")
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "dxguid.lib")

#define LEN(e) (sizeof(e)/sizeof(e[0]))

class FrameCapturer
{
    ID3D11Device *device=nullptr;
    ID3D11DeviceContext *device_context=nullptr;
    IDXGIOutputDuplication *output_duplication=nullptr;
    IDXGIFactory1 *dxgi_factory = nullptr;
    HWND capture_window;

    int captured_display_left;
    int captured_display_top;
    int captured_display_right;
    int captured_display_bottom;

    int capture_window_left;
    int capture_window_top;
    int capture_window_right;
    int capture_window_bottom;

    ID3D11Texture2D *capture_texture=nullptr;
    ID3D11Texture2D *region_copy_texture=nullptr;
    IDXGISurface *region_copy_surface=nullptr;

    int h;
    int w;
    int x;
    int y;
    int width;
    int height;
    int waitinit;
    int triggerable;
    unsigned int cnt;
    unsigned int *tsz;
    unsigned char **bf;

    /*
     * Было выпелено в угоду православным инклудам!
     *
     * #include <dxgi.h>
     * #include <d3d11.h>
     *
     * Если Microsoft будет выебываться, вернуть на место!
     *
     *   GUID IIDXGISurface = { 0xcafcb56c, 0x6ac3, 0x4889, 0xbf, 0x47, 0x9e, 0x23, 0xbb, 0xd2, 0x60, 0xec}; //IID_IDXGISurface
     *   GUID IIDXGIFactory1 = { 0x770aae78, 0xf26f, 0x4dba, 0xa8, 0x29, 0x25, 0x3c, 0x83, 0xd1, 0xb3, 0x87 }; //IID_IDXGIFactory1
     *   GUID IID3D11Texture2D = { 0x6f15aaf2, 0xd208, 0x4e89, 0x9a, 0xb4, 0x48, 0x95, 0x35, 0xd3, 0x4f, 0x9c }; //IID_ID3D11Texture2D
     */
    bool verbosity = false;

protected:
    FrameCapturer();

    uint getLenght();
    uint getHeight();
    uint getWidth();

    bool tryCaptureFrame(unsigned char* copy_to_buffer);

private:
    void captureStateInit(int x, int y, int w, int h);
};

#endif // FRAMECAPTURER_H
