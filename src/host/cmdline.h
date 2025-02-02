/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- cmdline.h

Abstract:
- This file contains the internal structures and definitions used by command line input and editing.

Author:
- Therese Stowell (ThereseS) 15-Nov-1991

Revision History:
- Mike Griese (migrie) Jan 2018:
    Refactored the history and alias functionality into their own files.
- Michael Niksa (miniksa) May 2018:
    Split apart popup information. Started encapsulating command line things. Removed 0 length buffers.
Notes:
    The input model for the command line editing popups is complex.
    Here is the relevant pseudocode:

    CookedReadWaitRoutine
        if (CookedRead->Popup)
            Status = (*CookedRead->Popup->Callback)();
            if (Status == CONSOLE_STATUS_READ_COMPLETE)
                return STATUS_SUCCESS;
            return Status;

    CookedRead
        if (Command Line Editing Key)
            ProcessCommandLine
        else
            process regular key

    ProcessCommandLine
        if F7
            return Popup

    Popup
        draw popup
        return ProcessCommandListInput

    ProcessCommandListInput
        while (TRUE)
            GetChar
            if (wait)
                return wait
            switch (char)
                .
                .
                .
--*/

#pragma once

#include "input.h"
#include "screenInfo.hpp"
#include "server.h"

#include "history.h"
#include "alias.h"
#include "popup.h"
#include "cookedRead.hpp"


class CommandLine
{
public:
    ~CommandLine();

    static CommandLine& Instance();

    bool IsEditLineEmpty() const;
    void Hide(const bool fUpdateFields);
    void Show();
    bool IsVisible() const noexcept;

    [[nodiscard]]
    NTSTATUS ProcessCommandLine(CookedRead& cookedReadData,
                                _In_ WCHAR wch,
                                const DWORD dwKeyState);

    [[nodiscard]]
    HRESULT StartCommandNumberPopup(CookedRead& cookedReadData);

    bool HasPopup() const noexcept;
    Popup& GetPopup();

    void UpdatePopups(const TextAttribute& NewAttributes,
                      const TextAttribute& NewPopupAttributes,
                      const TextAttribute& OldAttributes,
                      const TextAttribute& OldPopupAttributes);

    void EndCurrentPopup();
    void EndAllPopups();

    void DeletePromptAfterCursor(CookedRead& cookedReadData) noexcept;
    void DeleteFromRightOfCursor(CookedRead& cookedReadData) noexcept;
protected:
    CommandLine();

    // delete these because we don't want to accidentally get copies of the singleton
    CommandLine(CommandLine const&) = delete;
    CommandLine& operator=(CommandLine const&) = delete;

    [[nodiscard]]
    NTSTATUS CommandLine::_startCommandListPopup(CookedRead& cookedReadData);
    [[nodiscard]]
    NTSTATUS CommandLine::_startCopyFromCharPopup(CookedRead& cookedReadData);
    [[nodiscard]]
    NTSTATUS CommandLine::_startCopyToCharPopup(CookedRead& cookedReadData);

    void _processHistoryCycling(CookedRead& cookedReadData, const CommandHistory::SearchDirection searchDirection);
    void _setPromptToOldestCommand(CookedRead& cookedReadData);
    void _setPromptToNewestCommand(CookedRead& cookedReadData);
    COORD _deletePromptBeforeCursor(CookedRead& cookedReadData) noexcept;
    COORD _moveCursorToEndOfPrompt(CookedRead& cookedReadData) noexcept;
    COORD _moveCursorToStartOfPrompt(CookedRead& cookedReadData) noexcept;
    COORD _moveCursorLeftByWord(CookedRead& cookedReadData) noexcept;
    COORD _moveCursorLeft(CookedRead& cookedReadData);
    COORD _moveCursorRightByWord(CookedRead& cookedReadData) noexcept;
    COORD _moveCursorRight(CookedRead& cookedReadData) noexcept;
    void _insertCtrlZ(CookedRead& cookedReadData) noexcept;
    void _deleteCommandHistory(CookedRead& cookedReadData) noexcept;
    void _fillPromptWithPreviousCommandFragment(CookedRead& cookedReadData) noexcept;
    void _cycleMatchingCommandHistoryToPrompt(CookedRead& cookedReadData);


#ifdef UNIT_TESTING
    friend class CommandLineTests;
    friend class CommandNumberPopupTests;
#endif

private:

    std::deque<std::unique_ptr<Popup>> _popups;
    bool _isVisible;
};


void DeleteCommandLine(CookedRead& cookedReadData, const bool fUpdateFields);

void RedrawCommandLine(CookedRead& cookedReadData);

// Values for WriteChars(), WriteCharsLegacy() dwFlags
#define WC_DESTRUCTIVE_BACKSPACE 0x01
#define WC_KEEP_CURSOR_VISIBLE   0x02
#define WC_ECHO                  0x04

// This is no longer necessary. The buffer will always be Unicode. We don't need to perform special work to check if we're in a raster font
// and convert the entire buffer to match (and all insertions).
//#define WC_FALSIFY_UNICODE       0x08

#define WC_LIMIT_BACKSPACE       0x10
#define WC_NONDESTRUCTIVE_TAB    0x20
//#define WC_NEWLINE_SAVE_X        0x40  -  This has been replaced with an output mode flag instead as it's line discipline behavior that may not necessarily be coupled with VT.
#define WC_DELAY_EOL_WRAP        0x80

// Word delimiters
bool IsWordDelim(const WCHAR wch);
bool IsWordDelim(const std::wstring_view charData);

[[nodiscard]]
HRESULT DoSrvSetConsoleTitleW(const std::wstring_view title) noexcept;

bool IsValidStringBuffer(_In_ bool Unicode, _In_reads_bytes_(Size) PVOID Buffer, _In_ ULONG Size, _In_ ULONG Count, ...);

void SetCurrentCommandLine(CookedRead& cookedReadData, _In_ SHORT Index);
