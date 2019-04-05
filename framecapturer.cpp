#include "framecapturer.h"

void ShowError(QString error)
{
    QMessageBox msg;
    msg.setText(error);
    msg.exec();

    QApplication::exit(-1); //abort();
}

uint FrameCapturer::getLenght()
{
    return static_cast<uint>(this->height*this->width*4);
}

uint FrameCapturer::getHeight()
{
    return static_cast<uint>(this->height);
}

uint FrameCapturer::getWidth()
{
    return static_cast<uint>(this->width);
}

void FrameCapturer::captureStateInit(int x, int y, int w, int h)
{
    if (this->dxgi_factory != nullptr)dxgi_factory->Release();
    if (this->device != nullptr)this->device->Release();
    if (this->device_context != nullptr)this->device_context->Release();
    if (this->output_duplication != nullptr)this->output_duplication->Release();

    HRESULT hr = CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void**>(&dxgi_factory));

    if (FAILED(hr))
    {
        ShowError(QStringLiteral("Error create dxgi factory: %1").arg(hr));
    }

    D3D_FEATURE_LEVEL supported_feature_levels[] =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    D3D_FEATURE_LEVEL fl;

    POINT pt;

    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, supported_feature_levels, LEN(supported_feature_levels), D3D11_SDK_VERSION, &this->device, &fl, &this->device_context);

    if (FAILED(hr))
    {
        ShowError(QStringLiteral("Error create d3d11 device: %1").arg(hr));

    }

    this->x = x;
    this->y = y;
    this->h = h;
    this->w = w;
    this->captured_display_left = 0;
    this->captured_display_top = 0;
    this->captured_display_right = 0;
    this->captured_display_bottom = 0;

    this->output_duplication = nullptr;
    this->capture_texture = nullptr;
    this->region_copy_texture = nullptr;
    this->region_copy_surface = nullptr;

    pt.x = (x + w) / 2;
    pt.y = (y + h) / 2;

    this->capture_window = WindowFromPoint(pt);

    this->capture_window_left = x;
    this->capture_window_top = y;
    this->capture_window_right = w;
    this->capture_window_bottom = h;

    // find the display that has the window on it.
    IDXGIAdapter1 *adapter;
    for (uint adapter_index = 0; dxgi_factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND; adapter_index++)
    {
        // enumerate outputs
        IDXGIOutput *output;
        for (uint output_index = 0; adapter->EnumOutputs(output_index, &output) != DXGI_ERROR_NOT_FOUND; output_index++)
        {
            DXGI_OUTPUT_DESC output_desc;
            output->GetDesc(&output_desc);
            if (output_desc.AttachedToDesktop)
            {
                if (output_desc.DesktopCoordinates.left <= this->capture_window_left && output_desc.DesktopCoordinates.right >= this->capture_window_right && output_desc.DesktopCoordinates.top <= this->capture_window_top && output_desc.DesktopCoordinates.bottom >= this->capture_window_bottom)
                {
                    this->captured_display_left = output_desc.DesktopCoordinates.left;
                    this->captured_display_right = output_desc.DesktopCoordinates.right;
                    this->captured_display_bottom = output_desc.DesktopCoordinates.bottom;
                    this->captured_display_top = output_desc.DesktopCoordinates.top;

                    IDXGIOutput1 *output1 = static_cast<IDXGIOutput1*>(output);
                    hr = output1->DuplicateOutput(static_cast<IUnknown *>(this->device), &this->output_duplication);
                    if (FAILED(hr))
                    {
                        captureStateInit(x,y,w,h);
                        //ShowError(QStringLiteral("Output Duplication Failed: %1").arg(hr));
                    }
                }
            }
            output->Release();
        }
        adapter->Release();
    }

    this->width = this->capture_window_right - this->capture_window_left;
    this->height = this->capture_window_bottom - this->capture_window_top;
}

bool FrameCapturer::tryCaptureFrame(unsigned char* copy_to_buffer)
{
    DXGI_OUTDUPL_FRAME_INFO capture_frame_info;
    IDXGIResource *resource;
    HRESULT hr = S_OK;
    hr = this->output_duplication->AcquireNextFrame(0, &capture_frame_info, &resource);
    if (FAILED(hr))
    {
        if (verbosity)qDebug() << "Error: no new frames";
        captureStateInit(this->x, this->y, this->w, this->h);
        return false;
    }

    resource->QueryInterface(IID_ID3D11Texture2D, reinterpret_cast<void**>(&this->capture_texture));
    resource->Release();

    if (this->capture_texture)
    {
        if (verbosity)qDebug()<<"texture captured ok\n";
        D3D11_TEXTURE2D_DESC capture_texture_desc;
        this->capture_texture->GetDesc(&capture_texture_desc);

        D3D11_TEXTURE2D_DESC region_texture_desc;
        ZeroMemory(&region_texture_desc, sizeof(region_texture_desc));

        region_texture_desc.Width = static_cast<uint>(this->width);
        region_texture_desc.Height = static_cast<uint>(this->height);
        region_texture_desc.MipLevels = 1;
        region_texture_desc.ArraySize = 1;
        region_texture_desc.SampleDesc.Count = 1;
        region_texture_desc.SampleDesc.Quality = 0;
        region_texture_desc.Usage = D3D11_USAGE_STAGING;
        region_texture_desc.Format = capture_texture_desc.Format;
        region_texture_desc.BindFlags = 0;
        region_texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        region_texture_desc.MiscFlags = 0;

        hr = this->device->CreateTexture2D(&region_texture_desc, nullptr, &this->region_copy_texture);
        if (FAILED(hr))
        {
            if (verbosity)qDebug()<<"error - CreateTexture2d: "<< hr;
            this->capture_texture->Release();
            captureStateInit(this->x, this->y, this->w, this->h);
            return false;
        }
        this->capture_texture->Release();
        if (verbosity)qDebug()<<"texture created ok\n";
        // copy region of screen to texture
        D3D11_BOX source_region;
        source_region.left = static_cast<uint>(this->capture_window_left);
        source_region.right = static_cast<uint>(this->capture_window_right);
        source_region.top = static_cast<uint>(this->capture_window_top);
        source_region.bottom = static_cast<uint>(this->capture_window_bottom);
        source_region.front = 0;
        source_region.back = 1;
        this->device_context->CopySubresourceRegion(static_cast<ID3D11Resource *>(this->region_copy_texture), 0, 0, 0, 0, static_cast<ID3D11Resource *>(this->capture_texture), 0, &source_region);
        this->region_copy_texture->QueryInterface(IID_IDXGISurface, reinterpret_cast<void**>(&this->region_copy_surface));

        DXGI_MAPPED_RECT rect;
        if (verbosity)qDebug()<<"surface mapped ok\n";
        hr = this->region_copy_surface->Map(&rect, DXGI_MAP_READ);
        if (FAILED(hr))
        {
            if (verbosity)qDebug()<<"error - region_copy_surface: "<< hr;
            this->region_copy_surface->Unmap();
            this->region_copy_surface->Release();
            this->region_copy_texture->Release();
            captureStateInit(this->x, this->y, this->w, this->h);
            return false;
        }

        unsigned char *dest = copy_to_buffer;
        unsigned char *src = rect.pBits;
        int comp = 4;

        for (int row = 0; row < this->height; row++)
        {
            memcpy(dest, src, static_cast<size_t>(this->width * comp));
            dest += this->width * comp;
            src += rect.Pitch;
        }

        if (verbosity)qDebug()<<"surface moved ok\n";

        this->region_copy_surface->Unmap();
        this->region_copy_surface->Release();
        this->output_duplication->ReleaseFrame();
        this->region_copy_texture->Release();
        if (verbosity)qDebug()<<"memory deallocated ok\n";
        return true;
    }
    return false;
}

FrameCapturer::FrameCapturer()
{
    captureStateInit(0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
}

