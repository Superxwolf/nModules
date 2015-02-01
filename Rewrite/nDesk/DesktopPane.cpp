#include "DesktopPane.hpp"

#include "../nCoreApi/Core.h"

#include "../nShared/Math.h"

#include "../nUtilities/lsapi.h"
#include "../nUtilities/Macros.h"

#include <ShlObj.h>
#include <wincodec.h>


DesktopPane::DesktopPane()
  : mPane(nullptr)
  , mRenderTarget(nullptr)
{
  PaneInitData initData;
  ZeroMemory(&initData, sizeof(PaneInitData));
  initData.cbSize = sizeof(PaneInitData);
  initData.name = nullptr;
  initData.messageHandler = (IMessageHandler*)this;
  initData.painter = (IPanePainter*)this;
  initData.flags = PaneInitData::DesktopWindow;

  mPane = nCore::CreatePane(&initData);
  mPane->Show();
}


DesktopPane::~DesktopPane() {
  if (mPane != nullptr) {
    mPane->Destroy();
  }
}


LRESULT DesktopPane::HandleMessage(HWND window, UINT msg, WPARAM wParam, LPARAM lParam, NPARAM) {
  switch (msg) {
  case WM_WINDOWPOSCHANGING:
    {
      // Keep the hWnd at the bottom of the window stack
      LPWINDOWPOS c = LPWINDOWPOS(lParam);
      c->hwnd = window;
      c->hwndInsertAfter = HWND_BOTTOM;
      c->flags &= ~SWP_HIDEWINDOW;
      c->flags |= SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_SHOWWINDOW;
      const Display &desktop = nCore::GetDisplays()->GetDesktop();
      c->x = desktop.rect.left;
      c->y = desktop.rect.top;
      c->cx = desktop.width;
      c->cy = desktop.height;
    }
    return 0;
  }

  return DefWindowProc(window, msg, wParam, lParam);
}


LPVOID DesktopPane::AddPane(const IPane *pane) {
  return nullptr;
}


HRESULT DesktopPane::CreateDeviceResources(ID2D1RenderTarget *renderTarget) {
  HRESULT hr = S_OK;
  IDesktopWallpaper *desktopWallpaper;
  hr = LSCoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL, IID_IDesktopWallpaper,
    (void**)&desktopWallpaper);
  if (SUCCEEDED(hr)) {
    const Display &desktop = nCore::GetDisplays()->GetDesktop();

    UINT numMonitors;
    hr = desktopWallpaper->GetMonitorDevicePathCount(&numMonitors);
    if (SUCCEEDED(hr)) {
      mWallpapers.resize(numMonitors);
      for (UINT i = 0; i < numMonitors; ++i) {
        Wallpaper &wallpaper = mWallpapers[i];

        LPWSTR id;
        hr = desktopWallpaper->GetMonitorDevicePathAt(i, &id);
        if (SUCCEEDED(hr)) {
          RECT rect;
          hr = desktopWallpaper->GetMonitorRECT(id, &rect);
          if (SUCCEEDED(hr)) {
            wallpaper.rect.top = (float)(rect.top - desktop.rect.top);
            wallpaper.rect.left = (float)(rect.left - desktop.rect.left);
            wallpaper.rect.right = (float)(rect.right - desktop.rect.left);
            wallpaper.rect.bottom = (float)(rect.bottom - desktop.rect.top);

            LPWSTR file;
            hr = desktopWallpaper->GetWallpaper(id, &file);
            if (SUCCEEDED(hr)) {
              UINT width, height;
              wchar_t expandedPath[MAX_PATH];
              ID2D1BitmapBrush *bitmapBrush;
              ExpandEnvironmentStrings(file, expandedPath, MAX_PATH);
              hr = CreateBitmapBrush(expandedPath, renderTarget, &bitmapBrush, &width, &height);
              if (SUCCEEDED(hr)) {
                wallpaper.brush = (ID2D1Brush*)bitmapBrush;
                D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(
                  (float)wallpaper.rect.left, (float)wallpaper.rect.top);
                D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(
                  float(float(rect.bottom - rect.top) / height),
                  float(float(rect.right - rect.left) / width));
                bitmapBrush->SetTransform(scale * translation);
              }
              CoTaskMemFree(file);
            }
          }
          CoTaskMemFree(id);
        }
        if (FAILED(hr)) {
          break;
        }
      }
    }
    desktopWallpaper->Release();
  }
  return hr;
}


void DesktopPane::DiscardDeviceResources() {
  for (Wallpaper &wallpaper : mWallpapers) {
    SAFERELEASE(wallpaper.brush);
  }
}


bool DesktopPane::DynamicColorChanged(ID2D1RenderTarget*) {
  return false;
}


void DesktopPane::Paint(ID2D1RenderTarget *renderTarget, const D2D1_RECT_F *area, const IPane*,
    LPVOID) const {
  for (const Wallpaper &wallpaper : mWallpapers) {
    D2D1_RECT_F invalidatedArea;
    if (RectIntersection(area, &wallpaper.rect, &invalidatedArea)) {
      renderTarget->FillRectangle(invalidatedArea, wallpaper.brush);
    }
  }
}


void DesktopPane::PositionChanged(const IPane*, LPVOID, D2D1_RECT_F) {
}


void DesktopPane::RemovePane(const IPane*, LPVOID) {
}


void DesktopPane::TextChanged(const IPane*, LPVOID, LPCWSTR) {
}


HRESULT DesktopPane::CreateBitmapBrush(LPCWSTR file, ID2D1RenderTarget *renderTarget,
    ID2D1BitmapBrush **brush, LPUINT widthOut, LPUINT heightOut) {
  HRESULT hr = S_OK;
  IWICImagingFactory *factory = nCore::GetWICFactory();
  IWICBitmapDecoder *decoder;
  hr = factory->CreateDecoderFromFilename(file, nullptr, GENERIC_READ,
    WICDecodeMetadataCacheOnDemand, &decoder);
  if (SUCCEEDED(hr)) {
    IWICBitmapFrameDecode *frame;
    hr = decoder->GetFrame(0, &frame);
    if (SUCCEEDED(hr)) {
      hr = frame->GetSize(widthOut, heightOut);
      if (SUCCEEDED(hr)) {
        IWICFormatConverter *converter;
        hr = factory->CreateFormatConverter(&converter);
        if (SUCCEEDED(hr)) {
          hr = converter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,
            nullptr, 0.0, WICBitmapPaletteTypeMedianCut);
          if (SUCCEEDED(hr)) {
            ID2D1Bitmap *bitmap;
            hr = renderTarget->CreateBitmapFromWicBitmap(converter, &bitmap);
            if (SUCCEEDED(hr)) {
              hr = renderTarget->CreateBitmapBrush(bitmap, brush);
              bitmap->Release();
            }
          }
          converter->Release();
        }
      }
      frame->Release();
    }
    decoder->Release();
  }
  return hr;
}