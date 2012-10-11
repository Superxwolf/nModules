/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  TrayIcon.hpp
 *  The nModules Project
 *
 *  Declaration of the TrayIcon class.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

#include "../nShared/Drawable.hpp"
#include "../nShared/DrawableWindow.hpp"

class TrayIcon : public Drawable {
public:
    explicit TrayIcon(Drawable* parent, LPLSNOTIFYICONDATA pNID, Settings* parentSettings);
    virtual ~TrayIcon();

    void Reposition(UINT x, UINT y, UINT width, UINT height);
    void Show();

    void LoadSettings(bool isRefresh = false);
    LRESULT WINAPI HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    void SendCallback(UINT message, WPARAM wParam, LPARAM lParam);
    void GetScreenRect(LPRECT rect);

    void SetIcon(HICON icon);

    void HandleAdd(LPLSNOTIFYICONDATA pNID);
    void HandleModify(LPLSNOTIFYICONDATA pNID);
    void HandleSetVersion(LPLSNOTIFYICONDATA pNID);

private:
    //
    int iconSize;
    bool showingTip;
    bool showingBalloon;

    bool showTip; // True if we should show the tooltip

    // Tray data
    WCHAR tip[TRAY_MAX_TIP_LENGTH];
    WCHAR info[TRAY_MAX_INFO_LENGTH];
    WCHAR infoTitle[TRAY_MAX_INFOTITLE_LENGTH];
    DWORD infoFlags;
    UINT version;
    HICON icon;
    HWND callbackWindow;
    UINT callbackID;
    UINT callbackMessage;
    HICON balloonIcon;
};
