/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Popup.cpp
 *  The nModules Project
 *
 *  Represents a popup box.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "../nShared/LiteStep.h"
#include <strsafe.h>
#include "../nShared/Debugging.h"
#include "Popup.hpp"
#include "../nShared/LSModule.hpp"
#include "../nShared/MonitorInfo.hpp"
#include "FolderItem.hpp"


Popup::Popup(LPCSTR title, LPCSTR bang, LPCSTR prefix) : Drawable(prefix) {
    WCHAR buf[MAX_LINE_LENGTH];

    if (bang != NULL) {
        this->bang = _strdup(bang);
    }
    else {
        this->bang = NULL;
    }
    this->openChild = NULL;
    this->owner = NULL;

    this->itemSpacing = settings->GetInt("ItemSpacing", 2);
    this->maxWidth = settings->GetInt("MaxWidth", 300);
    this->noIcons = settings->GetBool("NoIcons", false);
    this->expandLeft = settings->GetBool("ExpandLeft", false);
    this->confineToMonitor = settings->GetBool("ConfineToMonitor", false);
    this->confineToWorkArea = settings->GetBool("ConfineToWorkArea", false);
    mChildOffsetX = settings->GetInt("ChildOffsetX", 0);
    mChildOffsetY = settings->GetInt("ChildOffsetY", 0);
    settings->GetOffsetRect("PaddingLeft", "PaddingTop", "PaddingRight", "PaddingBottom", &this->padding, 5, 5, 5, 5);

    DrawableSettings defaultSettings;
    defaultSettings.alwaysOnTop = true;
    defaultSettings.width = 200;
    MultiByteToWideChar(CP_ACP, 0, title, (int)strlen(title)+1, buf, MAX_LINE_LENGTH);

    StateSettings defaultState;
    defaultState.backgroundBrush.color = 0x440000FF;
    defaultState.textRotation = -45.0f;
    defaultState.fontSize = 32.0f;
    defaultState.textAlign = DWRITE_TEXT_ALIGNMENT_CENTER;
    defaultState.textVerticalAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;

    this->window->Initialize(&defaultSettings, &defaultState);
    this->window->SetText(buf);

    SetParent(this->window->GetWindowHandle(), NULL);
    SetWindowLongPtr(this->window->GetWindowHandle(), GWL_EXSTYLE, GetWindowLongPtr(this->window->GetWindowHandle(), GWL_EXSTYLE) & ~WS_EX_NOACTIVATE);
    this->sized = false;
    this->mouseOver = false;
    this->childItem = NULL;
}


Popup::~Popup() {
    for (vector<PopupItem*>::const_iterator iter = this->items.begin(); iter != this->items.end(); iter++) {
        delete *iter;
    }
    this->items.clear();
    if (this->bang != NULL) {
        free((LPVOID)this->bang);
    }
}


void Popup::AddItem(PopupItem* item) {
    this->items.push_back(item);
    this->sized = false;
    if (this->window->IsVisible()) {
        Size();
        this->window->Repaint();
    }
}


void Popup::RemoveItem(PopupItem* /* item */) {
}


void Popup::CloseChild(bool closing) {
    if (this->openChild != NULL) {
        if (!closing) {
            SetFocus(this->window->GetWindowHandle());
            SetActiveWindow(this->window->GetWindowHandle());
        }

        this->openChild->owner = NULL;
        ((nPopup::FolderItem*)this->childItem)->ClosingPopup();
        this->openChild->Close();
        this->childItem = NULL;
        this->openChild = NULL;
    }
}


void Popup::OpenChild(Popup* child, LPRECT position, PopupItem* childItem) {
    if (child != this->openChild) {
        CloseChild();
        this->openChild = child;
        this->childItem = childItem;
        this->openChild->expandLeft = this->expandLeft;
        position->top -= this->padding.top + mChildOffsetY;
        position->bottom -= this->padding.top;
        position->left -= this->padding.left + mChildOffsetX;
        position->right += this->padding.right + mChildOffsetX;
        this->openChild->Show(position, this);
    }
}


LPCSTR Popup::GetBang() {
    return this->bang;
}


bool Popup::CheckFocus(HWND newActive, __int8 direction) {
    if (this->window->GetWindowHandle() == newActive || this->mouseOver)
        return true;
    return direction & 1 && this->owner && this->owner->CheckFocus(newActive, 1)
        || direction & 2 && this->openChild && this->openChild->CheckFocus(newActive, 2);
}


void Popup::Close() {
    this->window->Hide();
    this->expandLeft = settings->GetBool("ExpandLeft", false);
    CloseChild(true);
    PostClose();
    this->mouseOver = false;
    if (this->owner != NULL) {
        this->owner->openChild = NULL;
        ((nPopup::FolderItem*)this->owner->childItem)->ClosingPopup();
        this->owner->childItem = NULL;
        Popup* owner = this->owner;
        this->owner = NULL;
        owner->Close();
    }
}


void Popup::Show() {
    POINT pt;
    GetCursorPos(&pt);
    Show(pt.x, pt.y);
}


void Popup::Show(int x, int y) {
    RECT r;
    r.left = x - 1; r.right = x + 1;
    r.top = y - 1; r.bottom = y + 1;
    Show(&r);
}


void Popup::Size() {
    // Work out the desired item 
    int itemWidth = settings->GetInt("Width", 200) - this->padding.left - this->padding.right;
    int maxItemWidth = this->maxWidth - this->padding.left - this->padding.right;
    int height = this->padding.top;
    int width;
    for (vector<PopupItem*>::const_iterator iter = this->items.begin(); iter != this->items.end(); iter++) {
        (*iter)->Position(this->padding.left, height);
        height += (*iter)->GetHeight() + this->itemSpacing;
        itemWidth = max(itemWidth, (*iter)->GetDesiredWidth(maxItemWidth));
    }
    width = itemWidth + this->padding.left + this->padding.right;
    height += this->padding.bottom - this->itemSpacing;
    MonitorInfo* monInfo = this->window->GetMonitorInformation();

    // We've excceeded the max height, split the popup into columns.
    if (height > monInfo->m_virtualDesktop.height) {
        int columns = (height - this->padding.top - this->padding.bottom)/(monInfo->m_virtualDesktop.height - this->padding.top - this->padding.bottom) + 1;
        int columnWidth = width;
        width = columnWidth * columns + this->itemSpacing*(columns - 1);
        height = this->padding.top;
        int column = 0;
        int rowHeight = 0;
        for (vector<PopupItem*>::const_iterator iter = this->items.begin(); iter != this->items.end(); iter++) {
            (*iter)->Position(this->padding.left + (columnWidth + this->itemSpacing) * column, height);
            rowHeight = max((*iter)->GetHeight() + this->itemSpacing, rowHeight);
            column++;
            if (column == columns) {
                height += rowHeight;
                rowHeight = 0;
                column = 0;
            }
        }
        if (column != 0) {
            height += rowHeight;
        }
        height += this->padding.bottom - this->itemSpacing;
    }

    // Size all items properly
    for (vector<PopupItem*>::const_iterator iter = this->items.begin(); iter != this->items.end(); iter++) {
        (*iter)->SetWidth(itemWidth);
    }

    // Size the main window
    this->window->Resize(width, height);
    this->sized = true;
}


void Popup::Show(LPRECT position, Popup* owner) {
    this->owner = owner;
    SetParent(this->window->GetWindowHandle(), NULL);
    PreShow();

    MonitorInfo* monInfo = this->window->GetMonitorInformation();

    if (!this->sized) {
        Size();
    }

    int x, y;
    RECT limits = monInfo->m_virtualDesktop.rect;

    if (this->confineToMonitor && this->confineToWorkArea) {
        limits = monInfo->m_monitors.at(monInfo->MonitorFromRECT(position)).workArea;
    }
    else if (this->confineToMonitor) {
        limits = monInfo->m_monitors.at(monInfo->MonitorFromRECT(position)).rect;
    }
    else if (this->confineToWorkArea) {
        // TODO::Shouldn't confine to the monitor here.
        limits = monInfo->m_monitors.at(monInfo->MonitorFromRECT(position)).workArea;
    }


    if (this->expandLeft) {
        x = position->left - this->window->GetDrawingSettings()->width;
        if (x < limits.left) {
            this->expandLeft = false;
            x = this->owner ? position->right : limits.left;
        }
    }
    else {
        x = position->right;
        if (x > limits.right - this->window->GetDrawingSettings()->width) {
            this->expandLeft = true;
            x = (this->owner ? position->left : limits.right) - this->window->GetDrawingSettings()->width;
        }
    }

    y = max(limits.top, min(limits.bottom - this->window->GetDrawingSettings()->height, position->top));

    this->window->Move(x, y);

    this->window->Show();
    SetWindowPos(this->window->GetWindowHandle(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetFocus(this->window->GetWindowHandle());
    SetActiveWindow(this->window->GetWindowHandle());
}


LRESULT Popup::HandleMessage(HWND window, UINT msg, WPARAM wParam, LPARAM lParam, LPVOID) {
    switch (msg) {
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE && this->window->IsVisible()) {
            if (!CheckFocus((HWND)lParam, 3)) {
                Close();
            }
        }
        return 0;

    case WM_MOUSEMOVE:
        this->mouseOver = true;
        return 0;

    case WM_MOUSELEAVE:
        this->mouseOver = false;
        return 0;

    default:
        return DefWindowProc(window, msg, wParam, lParam);
    }
}
