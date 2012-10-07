/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  FolderItem.hpp
 *  The nModules Project
 *
 *  A static folder.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

#include "PopupItem.hpp"
#include "Popup.hpp"

class FolderItem : public PopupItem {
public:
    explicit FolderItem(Drawable* parent, LPCSTR title, Popup* popup, LPCSTR customIcon = NULL);
    virtual ~FolderItem();

    LRESULT WINAPI HandleMessage(HWND, UINT, WPARAM, LPARAM);

private:
    Popup* popup;
    LPCSTR title;

    DrawableWindow::STATE hoverState;
};