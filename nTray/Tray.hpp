/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Tray.hpp
 *  The nModules Project
 *
 *  A systen tray, contains the tray icons.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

#include <map>
#include "TrayIcon.hpp"
#include "../nShared/Window.hpp"
#include "../nShared/Settings.hpp"
#include "../nShared/LayoutSettings.hpp"
#include "../nShared/Tooltip.hpp"
#include "../nShared/Balloon.hpp"
#include <strsafe.h>

using std::map;

class Tray : public Drawable
{
public:
    enum class States
    {
        Base = 0,
        Count
    };

public:
    explicit Tray(LPCTSTR);
    virtual ~Tray();

    // Adds an icon to the tray.
    TrayIcon* AddIcon(LiteStep::LPLSNOTIFYICONDATA);

    // Removes the specified icon from this tray, if it exists.
    void RemoveIcon(TrayIcon*);

    // Handles window messages for the tray.
    LRESULT WINAPI HandleMessage(HWND, UINT, WPARAM, LPARAM, LPVOID);

    // Should be called when the initalization phase is done -- repaint the tray.
    void InitCompleted();

    // Shows the trays tooltip.
    void ShowTip(LPCWSTR text, LPRECT position);

    // Hides the trays tooltip.
    void HideTip();

    // Enques a balloon tip for display.
    void EnqueueBalloon(TrayIcon* icon, LPCWSTR infoTitle, LPCWSTR info, DWORD infoFlags, HICON balloonIcon, bool realTime);

private:
    // Dismisses a balloon notification prematurely.
    void DismissBalloon(UINT message);

    // Hides the current balloon, and possible shows the next balloon.
    void ShowNextBalloon();

    // Returns true if we should display the given icon.
    bool WantIcon(LiteStep::LPLSNOTIFYICONDATA NID);

private:
    enum class OverflowAction
    {
        None,
        SizeLeft,
        SizeRight,
        SizeUp,
        SizeDown
    };

    // All data required to display a balloon.
    struct BalloonData
    {
        TrayIcon* icon;
        LPCWSTR infoTitle;
        LPCWSTR info;
        DWORD infoFlags;
        HICON balloonIcon;
    };

    // All data required to determine whether to show or hide a particular icon.
    struct IconID
    {
        IconID(GUID guid)
        {
            type = Type::GUID;
            this->guid = guid;
        }
        IconID(LPCWSTR process)
        {
            type = Type::Process;
            StringCchCopyW(this->process, _countof(this->process), process);
        }

        enum class Type
        {
            GUID,
            Process
        } type;

        union
        {
            GUID guid;
            WCHAR process[MAX_PATH];
        };
    };

private:
    StateRender<States> mStateRender;
    StateRender<TrayIcon::States> mIconStates;

    WindowSettings mIconWindowSettings;

private:
    // Balloons queued up to be displayed.
    std::list<BalloonData> queuedBalloons;

    //
    std::list<IconID> mHiddenIconIDs;

    // Loads .rc settings for this tray.
    void LoadSettings(bool isRefresh = false);

    // Positions all tray icons properly.
    void Relayout();

    // Finds the specified icon in the tray.
    vector<TrayIcon*>::const_iterator FindIcon(TrayIcon* icon);

    // The tray icons.
    vector<TrayIcon*> icons;

    // The size of the icons.
    int mIconSize;

    // Defines how the tray icons should be layed out.
    LayoutSettings mLayoutSettings;

    // The tooltip to show.
    Tooltip* tooltip;

    // The balloon to show.
    Balloon* balloon;

    // True if we should hide balloons.
    bool mHideBalloons;

    //
    bool mNoTooltips;

    // The size that the tray should be when it's not overflowing
    D2D1_SIZE_F mTargetSize;

    // The position where the tray should be when it's not overflowing
    D2D1_POINT_2F mTargetPosition;

    //
    OverflowAction mOverflowAction;

    TCHAR mOnResize[MAX_LINE_LENGTH];

    // Number of milliseconds to show balloons.
    int balloonTime;

    // Fires when we should hide the current balloon, and possibly show the next one.
    UINT_PTR balloonTimer;

    // Standard balloon icons.
    HICON infoIcon, warningIcon, errorIcon;

    // If true, don't play any notification sounds.
    bool noNotificationSounds;

    // The sound to play when showing new notifications.
    WCHAR notificationSound[128];

    // Message sent by the balloon when it is clicked.
    UINT balloonClickedMessage;

    // The active balloon icon.
    TrayIcon* activeBalloonIcon;
};
