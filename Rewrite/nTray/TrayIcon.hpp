#pragma once

#include "IconData.h"

#include "../nCoreApi/IImagePainter.hpp"
#include "../nCoreApi/IPane.hpp"
#include "../nCoreApi/IStatePainter.hpp"

#include "../nUtilities/lsapi.h"

class TrayIcon : public IMessageHandler {
public:
  TrayIcon(IPane *parent, IStatePainter *painter, IconData &data);
  ~TrayIcon();

  // IMessageHandler
public:
  LRESULT APICALL HandleMessage(HWND, UINT message, WPARAM, LPARAM, NPARAM) override;

public:
  bool GetScreenRect(LPRECT rect);
  void Modify(LPLSNOTIFYICONDATA);
  void Position(const NRECT &position);
  void SendCallback(UINT message, WPARAM wParam, LPARAM lParam);
  void Show();

private:
  IPane *mPane;
  IconData &mData;

  IStatePainter * const mStatePainter;
  IImagePainter *mIconPainter;
};
