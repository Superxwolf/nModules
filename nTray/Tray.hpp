/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Tray.hpp
 *  The nModules Project
 *
 *  Declaration of the Tray class.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

#include <map>
#include "TrayIcon.hpp"
#include "../nShared/DrawableWindow.hpp"
#include "../nShared/Settings.hpp"
#include "../nShared/LayoutSettings.hpp"

using std::map;

class Tray : public Drawable {
public:
    explicit Tray(LPCSTR);
    virtual ~Tray();

    void LoadSettings(bool = false);
    TrayIcon* AddIcon(LPLSNOTIFYICONDATA);
    void RemoveIcon(TrayIcon*);
    LRESULT WINAPI HandleMessage(HWND, UINT, WPARAM, LPARAM);
    void Relayout();
    vector<TrayIcon*>::const_iterator FindIcon(TrayIcon* NID);
    void InitCompleted();

private:
    //
    LayoutSettings* layoutSettings;

    // The tray icons
    vector<TrayIcon*> icons;

    // The size of the icons
    int iconSize;
};
