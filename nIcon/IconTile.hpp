/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  IconTile.hpp
 *  The nModules Project
 *
 *  A single icon in an IconGroup.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

#include "../nShared/Drawable.hpp"
#include "../nShared/Window.hpp"
#include <ShlObj.h>

class IconTile : public Drawable {
public:
    explicit IconTile(Drawable* parent, PCITEMID_CHILD item, IShellFolder2* shellFolder, int width, int height);
    virtual ~IconTile();

public:
    HRESULT CompareID(PCITEMID_CHILD id);
    void SetPosition(int id, int x, int y);
    LRESULT WINAPI HandleMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam, LPVOID);
    HRESULT GetDisplayName(SHGDNF flags, LPWSTR buf, UINT cchBuf);

    //
    PCITEMID_CHILD GetItem();

public:
    //
    int GetPositionID();

    // Hides this icon.
    void Hide();

    // Renames this item.
    void Rename(PCITEMID_CHILD newItem);

    // Updates the icon.
    void UpdateIcon(bool repaint = true);

    // Shows the right-click menu for the icon.
    void ShowContextMenu();

    //
    bool IsSelected();

    //
    bool IsGhosted();
    void SetGhost();
    void ClearGhost();

    //
    void Select(bool repaint = true);
    void Deselect(bool repaint = true);
    bool IsInRect(D2D1_RECT_F rect);

private:
    // Sets the icon.
    void SetIcon();

private:
    // Pointer to the shellfolder this item is in.
    IShellFolder2* mShellFolder;

    // The PID of this icon.
    PITEMID_CHILD mItem;

    //
    int mPositionID;

    //
    int mIconSize;

    //
    float mGhostOpacity;
    bool mGhosted;

    // 
    Window::STATE mHoverState, mSelectedState, mFocusedState;

    //
    Window::OVERLAY mIconOverlay;

    // True if the mouse is currently above the icon.
    bool mMouseOver;

    //
    bool mMouseDown;

    // Where the mouse was when the 
    POINT mMouseDownPosition;
};
