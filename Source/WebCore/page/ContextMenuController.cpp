/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Igalia S.L
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "ContextMenuController.h"

#if ENABLE(CONTEXT_MENUS)

#include "BackForwardController.h"
#include "Chrome.h"
#include "ContextMenu.h"
#include "ContextMenuClient.h"
#include "ContextMenuItem.h"
#include "ItemSelectedEvent.h"
#include "ContextMenuProvider.h"
#include "Document.h"
#include "DocumentFragment.h"
#include "DocumentLoader.h"
#include "Editor.h"
#include "EditorClient.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventNames.h"
#include "FormState.h"
#include "Frame.h"
#include "FrameLoadRequest.h"
#include "FrameLoader.h"
#include "FrameLoaderClient.h"
#include "FrameSelection.h"
#include "HTMLFormElement.h"
#include "HitTestRequest.h"
#include "HitTestResult.h"
#include "InspectorController.h"
#include "LocalizedStrings.h"
#include "MouseEvent.h"
#include "NavigationAction.h"
#include "Node.h"
#include "Page.h"
#include "RenderLayer.h"
#include "RenderObject.h"
#include "ReplaceSelectionCommand.h"
#include "ResourceRequest.h"
#include "Settings.h"
#include "TextIterator.h"
#include "UserTypingGestureIndicator.h"
#include "WindowFeatures.h"
#include "markup.h"
#include <wtf/unicode/Unicode.h>

using namespace WTF;
using namespace Unicode;

namespace WebCore {

ContextMenuController::ContextMenuController(Page* page, ContextMenuClient* client)
    : m_page(page)
    , m_client(client)
{
    ASSERT_ARG(page, page);
    ASSERT_ARG(client, client);
}

ContextMenuController::~ContextMenuController()
{
    m_client->contextMenuDestroyed();
}

PassOwnPtr<ContextMenuController> ContextMenuController::create(Page* page, ContextMenuClient* client)
{
    return adoptPtr(new ContextMenuController(page, client));
}

void ContextMenuController::clearContextMenu()
{
    m_contextMenu.clear();
    if (m_menuProvider)
        m_menuProvider->contextMenuCleared();
    m_menuProvider = 0;
}

void ContextMenuController::handleContextMenuEvent(Event* event)
{
    m_element = toHTMLElement(event->target()->toNode());

    HTMLElement* self = m_element;
    DeepinMenu* deepinmenu = NULL;
    while (self != NULL) {
        deepinmenu = self->contextMenu();
        if (deepinmenu)
            break;
        self = toHTMLElement(self->parentElement());
    }
    m_element = self;

    if (deepinmenu != NULL) {
        m_contextMenu = deepinmenu->contextMenu();
    }
    if (!m_contextMenu)
        return;
    showContextMenu(event);
}

static PassOwnPtr<ContextMenuItem> separatorItem()
{
    return adoptPtr(new ContextMenuItem(SeparatorType, ContextMenuItemTagNoAction, String()));
}

void ContextMenuController::showContextMenu(Event* event, PassRefPtr<ContextMenuProvider> menuProvider)
{
    m_menuProvider = menuProvider;

    m_contextMenu = createContextMenu(event);
    if (!m_contextMenu) {
        clearContextMenu();
        return;
    }

    m_menuProvider->populateContextMenu(m_contextMenu.get());
    if (m_hitTestResult.isSelected()) {
        appendItem(*separatorItem(), m_contextMenu.get());
    }
    showContextMenu(event);
    event->setDefaultHandled();
}

PassOwnPtr<ContextMenu> ContextMenuController::createContextMenu(Event* event)
{
    if (!event->isMouseEvent())
        return nullptr;

    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);
    HitTestResult result(mouseEvent->absoluteLocation());

    if (Frame* frame = event->target()->toNode()->document()->frame())
        result = frame->eventHandler()->hitTestResultAtPoint(mouseEvent->absoluteLocation(), false);

    if (!result.innerNonSharedNode())
        return nullptr;

    m_hitTestResult = result;

    return adoptPtr(new ContextMenu);
}

void ContextMenuController::showContextMenu(Event* event)
{
#if ENABLE(INSPECTOR)
    //if (m_page->inspectorController()->enabled())
        //addInspectElementItem();
#endif


    PlatformMenuDescription customMenu = m_client->getCustomMenuFromDefaultItems(m_contextMenu.get());
    m_contextMenu->setPlatformDescription(customMenu);

    event->setDefaultHandled();
}

void ContextMenuController::contextMenuItemSelected(ContextMenuItem* item)
{
    int id = item->action() - ContextMenuItemBaseApplicationTag;
    if (id > 0) {
        PassRefPtr<ItemSelectedEvent> event = ItemSelectedEvent::create();
        event->initEvent(eventNames().itemselectedEvent, false, false);
        event->setID(id);
        event->setTitle(item->title());
        event->setTarget(m_element);
        event->setCurrentTarget(m_element);
        event->setEventPhase(Event::AT_TARGET);
        m_element->fireEventListeners(event.get());

        // FIXME: need this?
        m_client->contextMenuItemSelected(item, contextMenu());
        return;
    }
    //if (item->action() >= ContextMenuItemBaseCustomTag) {
        //ASSERT(m_menuProvider);
        //m_menuProvider->contextMenuItemSelected(item);
        //return;
    //}
}

void ContextMenuController::appendItem(ContextMenuItem& menuItem, ContextMenu* parentMenu)
{
    checkOrEnableIfNeeded(menuItem);
    if (parentMenu)
        parentMenu->appendItem(menuItem);
}

void ContextMenuController::createAndAppendFontSubMenu(ContextMenuItem& fontMenuItem)
{
    ContextMenu fontMenu;

#if PLATFORM(MAC)
    ContextMenuItem showFonts(ActionType, ContextMenuItemTagShowFonts, contextMenuItemTagShowFonts());
#endif
    ContextMenuItem bold(CheckableActionType, ContextMenuItemTagBold, contextMenuItemTagBold());
    ContextMenuItem italic(CheckableActionType, ContextMenuItemTagItalic, contextMenuItemTagItalic());
    ContextMenuItem underline(CheckableActionType, ContextMenuItemTagUnderline, contextMenuItemTagUnderline());
    ContextMenuItem outline(ActionType, ContextMenuItemTagOutline, contextMenuItemTagOutline());
#if PLATFORM(MAC)
    ContextMenuItem styles(ActionType, ContextMenuItemTagStyles, contextMenuItemTagStyles());
    ContextMenuItem showColors(ActionType, ContextMenuItemTagShowColors, contextMenuItemTagShowColors());
#endif

#if PLATFORM(MAC)
    appendItem(showFonts, &fontMenu);
#endif
    appendItem(bold, &fontMenu);
    appendItem(italic, &fontMenu);
    appendItem(underline, &fontMenu);
    appendItem(outline, &fontMenu);
#if PLATFORM(MAC)
    appendItem(styles, &fontMenu);
    appendItem(*separatorItem(), &fontMenu);
    appendItem(showColors, &fontMenu);
#endif

    fontMenuItem.setSubMenu(&fontMenu);
}


#if !PLATFORM(GTK)

void ContextMenuController::createAndAppendSpellingAndGrammarSubMenu(ContextMenuItem& spellingAndGrammarMenuItem)
{
    ContextMenu spellingAndGrammarMenu;

    ContextMenuItem showSpellingPanel(ActionType, ContextMenuItemTagShowSpellingPanel, 
        contextMenuItemTagShowSpellingPanel(true));
    ContextMenuItem checkSpelling(ActionType, ContextMenuItemTagCheckSpelling, 
        contextMenuItemTagCheckSpelling());
    ContextMenuItem checkAsYouType(CheckableActionType, ContextMenuItemTagCheckSpellingWhileTyping, 
        contextMenuItemTagCheckSpellingWhileTyping());
    ContextMenuItem grammarWithSpelling(CheckableActionType, ContextMenuItemTagCheckGrammarWithSpelling, 
        contextMenuItemTagCheckGrammarWithSpelling());
#if PLATFORM(MAC) && !defined(BUILDING_ON_LEOPARD)
    ContextMenuItem correctSpelling(CheckableActionType, ContextMenuItemTagCorrectSpellingAutomatically, 
        contextMenuItemTagCorrectSpellingAutomatically());
#endif

    appendItem(showSpellingPanel, &spellingAndGrammarMenu);
    appendItem(checkSpelling, &spellingAndGrammarMenu);
#if PLATFORM(MAC) && !defined(BUILDING_ON_LEOPARD)
    appendItem(*separatorItem(), &spellingAndGrammarMenu);
#endif
    appendItem(checkAsYouType, &spellingAndGrammarMenu);
    appendItem(grammarWithSpelling, &spellingAndGrammarMenu);
#if PLATFORM(MAC) && !defined(BUILDING_ON_LEOPARD)
    appendItem(correctSpelling, &spellingAndGrammarMenu);
#endif

    spellingAndGrammarMenuItem.setSubMenu(&spellingAndGrammarMenu);
}

#endif // !PLATFORM(GTK)


#if PLATFORM(MAC)

void ContextMenuController::createAndAppendSpeechSubMenu(ContextMenuItem& speechMenuItem)
{
    ContextMenu speechMenu;

    ContextMenuItem start(ActionType, ContextMenuItemTagStartSpeaking, contextMenuItemTagStartSpeaking());
    ContextMenuItem stop(ActionType, ContextMenuItemTagStopSpeaking, contextMenuItemTagStopSpeaking());

    appendItem(start, &speechMenu);
    appendItem(stop, &speechMenu);

    speechMenuItem.setSubMenu(&speechMenu);
}

#endif
 
#if !PLATFORM(GTK)

void ContextMenuController::createAndAppendWritingDirectionSubMenu(ContextMenuItem& writingDirectionMenuItem)
{
    ContextMenu writingDirectionMenu;

    ContextMenuItem defaultItem(ActionType, ContextMenuItemTagDefaultDirection, 
        contextMenuItemTagDefaultDirection());
    ContextMenuItem ltr(CheckableActionType, ContextMenuItemTagLeftToRight, contextMenuItemTagLeftToRight());
    ContextMenuItem rtl(CheckableActionType, ContextMenuItemTagRightToLeft, contextMenuItemTagRightToLeft());

    appendItem(defaultItem, &writingDirectionMenu);
    appendItem(ltr, &writingDirectionMenu);
    appendItem(rtl, &writingDirectionMenu);

    writingDirectionMenuItem.setSubMenu(&writingDirectionMenu);
}

void ContextMenuController::createAndAppendTextDirectionSubMenu(ContextMenuItem& textDirectionMenuItem)
{
    ContextMenu textDirectionMenu;

    ContextMenuItem defaultItem(ActionType, ContextMenuItemTagTextDirectionDefault, contextMenuItemTagDefaultDirection());
    ContextMenuItem ltr(CheckableActionType, ContextMenuItemTagTextDirectionLeftToRight, contextMenuItemTagLeftToRight());
    ContextMenuItem rtl(CheckableActionType, ContextMenuItemTagTextDirectionRightToLeft, contextMenuItemTagRightToLeft());

    appendItem(defaultItem, &textDirectionMenu);
    appendItem(ltr, &textDirectionMenu);
    appendItem(rtl, &textDirectionMenu);

    textDirectionMenuItem.setSubMenu(&textDirectionMenu);
}

#endif

#if PLATFORM(MAC) && !defined(BUILDING_ON_LEOPARD)

void ContextMenuController::createAndAppendSubstitutionsSubMenu(ContextMenuItem& substitutionsMenuItem)
{
    ContextMenu substitutionsMenu;

    ContextMenuItem showSubstitutions(ActionType, ContextMenuItemTagShowSubstitutions, contextMenuItemTagShowSubstitutions(true));
    ContextMenuItem smartCopyPaste(CheckableActionType, ContextMenuItemTagSmartCopyPaste, contextMenuItemTagSmartCopyPaste());
    ContextMenuItem smartQuotes(CheckableActionType, ContextMenuItemTagSmartQuotes, contextMenuItemTagSmartQuotes());
    ContextMenuItem smartDashes(CheckableActionType, ContextMenuItemTagSmartDashes, contextMenuItemTagSmartDashes());
    ContextMenuItem smartLinks(CheckableActionType, ContextMenuItemTagSmartLinks, contextMenuItemTagSmartLinks());
    ContextMenuItem textReplacement(CheckableActionType, ContextMenuItemTagTextReplacement, contextMenuItemTagTextReplacement());

    appendItem(showSubstitutions, &substitutionsMenu);
    appendItem(*separatorItem(), &substitutionsMenu);
    appendItem(smartCopyPaste, &substitutionsMenu);
    appendItem(smartQuotes, &substitutionsMenu);
    appendItem(smartDashes, &substitutionsMenu);
    appendItem(smartLinks, &substitutionsMenu);
    appendItem(textReplacement, &substitutionsMenu);

    substitutionsMenuItem.setSubMenu(&substitutionsMenu);
}

void ContextMenuController::createAndAppendTransformationsSubMenu(ContextMenuItem& transformationsMenuItem)
{
    ContextMenu transformationsMenu;

    ContextMenuItem makeUpperCase(ActionType, ContextMenuItemTagMakeUpperCase, contextMenuItemTagMakeUpperCase());
    ContextMenuItem makeLowerCase(ActionType, ContextMenuItemTagMakeLowerCase, contextMenuItemTagMakeLowerCase());
    ContextMenuItem capitalize(ActionType, ContextMenuItemTagCapitalize, contextMenuItemTagCapitalize());

    appendItem(makeUpperCase, &transformationsMenu);
    appendItem(makeLowerCase, &transformationsMenu);
    appendItem(capitalize, &transformationsMenu);

    transformationsMenuItem.setSubMenu(&transformationsMenu);
}

#endif

static bool selectionContainsPossibleWord(Frame* frame)
{
    // Current algorithm: look for a character that's not just a separator.
    for (TextIterator it(frame->selection()->toNormalizedRange().get()); !it.atEnd(); it.advance()) {
        int length = it.length();
        const UChar* characters = it.characters();
        for (int i = 0; i < length; ++i)
            if (!(category(characters[i]) & (Separator_Space | Separator_Line | Separator_Paragraph)))
                return true;
    }
    return false;
}

#if PLATFORM(MAC)
#if defined(BUILDING_ON_LEOPARD) || defined(BUILDING_ON_SNOW_LEOPARD)
#define INCLUDE_SPOTLIGHT_CONTEXT_MENU_ITEM 1
#else
#define INCLUDE_SPOTLIGHT_CONTEXT_MENU_ITEM 0
#endif
#endif

void ContextMenuController::checkOrEnableIfNeeded(ContextMenuItem& item) const
{
    if (item.type() == SeparatorType)
        return;
    
    Frame* frame = m_hitTestResult.innerNonSharedNode()->document()->frame();
    if (!frame)
        return;

    // Custom items already have proper checked and enabled values.
    if (ContextMenuItemBaseCustomTag <= item.action() && item.action() <= ContextMenuItemLastCustomTag)
        return;

    bool shouldEnable = true;
    bool shouldCheck = false; 

    switch (item.action()) {
        case ContextMenuItemTagCheckSpelling:
            shouldEnable = frame->editor()->canEdit();
            break;
        case ContextMenuItemTagDefaultDirection:
            shouldCheck = false;
            shouldEnable = false;
            break;
        case ContextMenuItemTagLeftToRight:
        case ContextMenuItemTagRightToLeft: {
            String direction = item.action() == ContextMenuItemTagLeftToRight ? "ltr" : "rtl";
            shouldCheck = frame->editor()->selectionHasStyle(CSSPropertyDirection, direction) != FalseTriState;
            shouldEnable = true;
            break;
        }
        case ContextMenuItemTagTextDirectionDefault: {
            Editor::Command command = frame->editor()->command("MakeTextWritingDirectionNatural");
            shouldCheck = command.state() == TrueTriState;
            shouldEnable = command.isEnabled();
            break;
        }
        case ContextMenuItemTagTextDirectionLeftToRight: {
            Editor::Command command = frame->editor()->command("MakeTextWritingDirectionLeftToRight");
            shouldCheck = command.state() == TrueTriState;
            shouldEnable = command.isEnabled();
            break;
        }
        case ContextMenuItemTagTextDirectionRightToLeft: {
            Editor::Command command = frame->editor()->command("MakeTextWritingDirectionRightToLeft");
            shouldCheck = command.state() == TrueTriState;
            shouldEnable = command.isEnabled();
            break;
        }
        case ContextMenuItemTagCopy:
            shouldEnable = frame->editor()->canDHTMLCopy() || frame->editor()->canCopy();
            break;
        case ContextMenuItemTagCut:
            shouldEnable = frame->editor()->canDHTMLCut() || frame->editor()->canCut();
            break;
        case ContextMenuItemTagIgnoreSpelling:
        case ContextMenuItemTagLearnSpelling:
            shouldEnable = frame->selection()->isRange();
            break;
        case ContextMenuItemTagPaste:
            shouldEnable = frame->editor()->canDHTMLPaste() || frame->editor()->canPaste();
            break;
#if PLATFORM(GTK)
        case ContextMenuItemTagDelete:
            shouldEnable = frame->editor()->canDelete();
            break;
        case ContextMenuItemTagInputMethods:
        case ContextMenuItemTagUnicode:
            shouldEnable = true;
            break;
#endif
#if PLATFORM(GTK) || PLATFORM(EFL)
        case ContextMenuItemTagSelectAll:
            shouldEnable = true;
            break;
#endif
        case ContextMenuItemTagUnderline: {
            shouldCheck = frame->editor()->selectionHasStyle(CSSPropertyWebkitTextDecorationsInEffect, "underline") != FalseTriState;
            shouldEnable = frame->editor()->canEditRichly();
            break;
        }
        case ContextMenuItemTagLookUpInDictionary:
            shouldEnable = frame->selection()->isRange();
            break;
        case ContextMenuItemTagCheckGrammarWithSpelling:
            if (frame->editor()->isGrammarCheckingEnabled())
                shouldCheck = true;
            shouldEnable = true;
            break;
        case ContextMenuItemTagItalic: {
            shouldCheck = frame->editor()->selectionHasStyle(CSSPropertyFontStyle, "italic") != FalseTriState;
            shouldEnable = frame->editor()->canEditRichly();
            break;
        }
        case ContextMenuItemTagBold: {
            shouldCheck = frame->editor()->selectionHasStyle(CSSPropertyFontWeight, "bold") != FalseTriState;
            shouldEnable = frame->editor()->canEditRichly();
            break;
        }
        case ContextMenuItemTagOutline:
            shouldEnable = false;
            break;
        case ContextMenuItemTagShowSpellingPanel:
            if (frame->editor()->spellingPanelIsShowing())
                item.setTitle(contextMenuItemTagShowSpellingPanel(false));
            else
                item.setTitle(contextMenuItemTagShowSpellingPanel(true));
            shouldEnable = frame->editor()->canEdit();
            break;
        case ContextMenuItemTagNoGuessesFound:
            shouldEnable = false;
            break;
        case ContextMenuItemTagCheckSpellingWhileTyping:
            shouldCheck = frame->editor()->isContinuousSpellCheckingEnabled();
            break;
#if PLATFORM(MAC)
        case ContextMenuItemTagSubstitutionsMenu:
        case ContextMenuItemTagTransformationsMenu:
            break;
        case ContextMenuItemTagShowSubstitutions:
#ifndef BUILDING_ON_LEOPARD
            if (frame->editor()->substitutionsPanelIsShowing())
                item.setTitle(contextMenuItemTagShowSubstitutions(false));
            else
                item.setTitle(contextMenuItemTagShowSubstitutions(true));
            shouldEnable = frame->editor()->canEdit();
#endif
            break;
        case ContextMenuItemTagMakeUpperCase:
        case ContextMenuItemTagMakeLowerCase:
        case ContextMenuItemTagCapitalize:
        case ContextMenuItemTagChangeBack:
            shouldEnable = frame->editor()->canEdit();
            break;
        case ContextMenuItemTagCorrectSpellingAutomatically:
#ifndef BUILDING_ON_LEOPARD
            shouldCheck = frame->editor()->isAutomaticSpellingCorrectionEnabled();
#endif
            break;
        case ContextMenuItemTagSmartCopyPaste:
#ifndef BUILDING_ON_LEOPARD
            shouldCheck = frame->editor()->smartInsertDeleteEnabled();
#endif
            break;
        case ContextMenuItemTagSmartQuotes:
#ifndef BUILDING_ON_LEOPARD
            shouldCheck = frame->editor()->isAutomaticQuoteSubstitutionEnabled();
#endif
            break;
        case ContextMenuItemTagSmartDashes:
#ifndef BUILDING_ON_LEOPARD
            shouldCheck = frame->editor()->isAutomaticDashSubstitutionEnabled();
#endif
            break;
        case ContextMenuItemTagSmartLinks:
#ifndef BUILDING_ON_LEOPARD
            shouldCheck = frame->editor()->isAutomaticLinkDetectionEnabled();
#endif
            break;
        case ContextMenuItemTagTextReplacement:
#ifndef BUILDING_ON_LEOPARD
            shouldCheck = frame->editor()->isAutomaticTextReplacementEnabled();
#endif
            break;
        case ContextMenuItemTagStopSpeaking:
            shouldEnable = client() && client()->isSpeaking();
            break;
#else // PLATFORM(MAC) ends here
        case ContextMenuItemTagStopSpeaking:
            break;
#endif
#if PLATFORM(GTK)
        case ContextMenuItemTagGoBack:
            shouldEnable = frame->page() && frame->page()->backForward()->canGoBackOrForward(-1);
            break;
        case ContextMenuItemTagGoForward:
            shouldEnable = frame->page() && frame->page()->backForward()->canGoBackOrForward(1);
            break;
        case ContextMenuItemTagStop:
            shouldEnable = frame->loader()->documentLoader()->isLoadingInAPISense();
            break;
        case ContextMenuItemTagReload:
            shouldEnable = !frame->loader()->documentLoader()->isLoadingInAPISense();
            break;
        case ContextMenuItemTagFontMenu:
            shouldEnable = frame->editor()->canEditRichly();
            break;
#else
        case ContextMenuItemTagGoBack:
        case ContextMenuItemTagGoForward:
        case ContextMenuItemTagStop:
        case ContextMenuItemTagReload:
        case ContextMenuItemTagFontMenu:
#endif
        case ContextMenuItemTagNoAction:
        case ContextMenuItemTagOpenLinkInNewWindow:
        case ContextMenuItemTagDownloadLinkToDisk:
        case ContextMenuItemTagCopyLinkToClipboard:
        case ContextMenuItemTagOpenImageInNewWindow:
        case ContextMenuItemTagDownloadImageToDisk:
        case ContextMenuItemTagCopyImageToClipboard:
#if PLATFORM(QT) || PLATFORM(GTK) || PLATFORM(EFL)
        case ContextMenuItemTagCopyImageUrlToClipboard:
#endif
            break;
        case ContextMenuItemTagOpenMediaInNewWindow:
            if (m_hitTestResult.mediaIsVideo())
                item.setTitle(contextMenuItemTagOpenVideoInNewWindow());
            else
                item.setTitle(contextMenuItemTagOpenAudioInNewWindow());
            break;
        case ContextMenuItemTagCopyMediaLinkToClipboard:
            if (m_hitTestResult.mediaIsVideo())
                item.setTitle(contextMenuItemTagCopyVideoLinkToClipboard());
            else
                item.setTitle(contextMenuItemTagCopyAudioLinkToClipboard());
            break;
        case ContextMenuItemTagToggleMediaControls:
            shouldCheck = m_hitTestResult.mediaControlsEnabled();
            break;
        case ContextMenuItemTagToggleMediaLoop:
            shouldCheck = m_hitTestResult.mediaLoopEnabled();
            break;
        case ContextMenuItemTagEnterVideoFullscreen:
            shouldEnable = m_hitTestResult.mediaSupportsFullscreen();
            break;
        case ContextMenuItemTagOpenFrameInNewWindow:
        case ContextMenuItemTagSpellingGuess:
        case ContextMenuItemTagOther:
        case ContextMenuItemTagSearchInSpotlight:
        case ContextMenuItemTagSearchWeb:
        case ContextMenuItemTagOpenWithDefaultApplication:
        case ContextMenuItemPDFActualSize:
        case ContextMenuItemPDFZoomIn:
        case ContextMenuItemPDFZoomOut:
        case ContextMenuItemPDFAutoSize:
        case ContextMenuItemPDFSinglePage:
        case ContextMenuItemPDFFacingPages:
        case ContextMenuItemPDFContinuous:
        case ContextMenuItemPDFNextPage:
        case ContextMenuItemPDFPreviousPage:
        case ContextMenuItemTagOpenLink:
        case ContextMenuItemTagIgnoreGrammar:
        case ContextMenuItemTagSpellingMenu:
        case ContextMenuItemTagShowFonts:
        case ContextMenuItemTagStyles:
        case ContextMenuItemTagShowColors:
        case ContextMenuItemTagSpeechMenu:
        case ContextMenuItemTagStartSpeaking:
        case ContextMenuItemTagWritingDirectionMenu:
        case ContextMenuItemTagTextDirectionMenu:
        case ContextMenuItemTagPDFSinglePageScrolling:
        case ContextMenuItemTagPDFFacingPagesScrolling:
#if ENABLE(INSPECTOR)
        case ContextMenuItemTagInspectElement:
#endif
        case ContextMenuItemBaseCustomTag:
        case ContextMenuItemCustomTagNoAction:
        case ContextMenuItemLastCustomTag:
        case ContextMenuItemBaseApplicationTag:
            break;
        case ContextMenuItemTagMediaPlayPause:
            if (m_hitTestResult.mediaPlaying())
                item.setTitle(contextMenuItemTagMediaPause());
            else
                item.setTitle(contextMenuItemTagMediaPlay());
            break;
        case ContextMenuItemTagMediaMute:
            shouldEnable = m_hitTestResult.mediaHasAudio();
            shouldCheck = shouldEnable &&  m_hitTestResult.mediaMuted();
            break;
    }

    item.setChecked(shouldCheck);
    item.setEnabled(shouldEnable);
}

} // namespace WebCore

#endif // ENABLE(CONTEXT_MENUS)
