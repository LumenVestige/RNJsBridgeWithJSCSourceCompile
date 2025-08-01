/*
 * Copyright (C) 2012-2018 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#if PLATFORM(IOS_FAMILY)

#import "WKContentView.h"

#if !PLATFORM(WATCHOS) && !PLATFORM(APPLETV)
#import "WKShareSheet.h"
#endif

#import "DragDropInteractionState.h"
#import "EditorState.h"
#import "FocusedElementInformation.h"
#import "GestureTypes.h"
#import "InteractionInformationAtPosition.h"
#import "SyntheticEditingCommandType.h"
#import "TextCheckingController.h"
#import "TransactionID.h"
#import "UIKitSPI.h"
#import "WKActionSheetAssistant.h"
#import "WKAirPlayRoutePicker.h"
#import "WKFileUploadPanel.h"
#import "WKFormPeripheral.h"
#import "WKKeyboardScrollingAnimator.h"
#import "WKShareSheet.h"
#import "WKSyntheticTapGestureRecognizer.h"
#import "WKTouchActionGestureRecognizer.h"
#import "_WKFormInputSession.h"
#import <UIKit/UIView.h>
#import <WebCore/ActivityState.h>
#import <WebCore/Color.h>
#import <WebCore/FloatQuad.h>
#import <wtf/BlockPtr.h>
#import <wtf/CompletionHandler.h>
#import <wtf/Forward.h>
#import <wtf/OptionSet.h>
#import <wtf/Vector.h>
#import <wtf/WeakObjCPtr.h>
#import <wtf/text/WTFString.h>

#if ENABLE(POINTER_EVENTS)
#import <WebCore/PointerID.h>
#endif

namespace API {
class OpenPanelParameters;
}

namespace WTF {
class TextStream;
}

namespace WebCore {
class Color;
class FloatQuad;
class IntSize;
class SelectionRect;
struct PromisedAttachmentInfo;
struct ShareDataWithParsedURL;
enum class DOMPasteAccessResponse : uint8_t;
enum class RouteSharingPolicy : uint8_t;

#if ENABLE(DRAG_SUPPORT)
struct DragItem;
#endif
}

namespace WebKit {
class InputViewUpdateDeferrer;
class NativeWebTouchEvent;
class SmartMagnificationController;
class WebOpenPanelResultListenerProxy;
class WebPageProxy;
struct WebAutocorrectionContext;
}

@class _UILookupGestureRecognizer;
@class _UIHighlightView;
@class _UIWebHighlightLongPressGestureRecognizer;
@class UIHoverGestureRecognizer;
@class UITargetedPreview;
@class WebEvent;
@class WKActionSheetAssistant;
@class WKContextMenuElementInfo;
@class WKDrawingCoordinator;
@class WKFocusedFormControlView;
@class WKFormInputControl;
@class WKFormInputSession;
@class WKInspectorNodeSearchGestureRecognizer;

typedef void (^UIWKAutocorrectionCompletionHandler)(UIWKAutocorrectionRects *rectsForInput);
typedef void (^UIWKAutocorrectionContextHandler)(UIWKAutocorrectionContext *autocorrectionContext);
typedef void (^UIWKDictationContextHandler)(NSString *selectedText, NSString *beforeText, NSString *afterText);
typedef void (^UIWKSelectionCompletionHandler)(void);
typedef void (^UIWKSelectionWithDirectionCompletionHandler)(BOOL selectionEndIsMoving);

typedef BlockPtr<void(WebKit::InteractionInformationAtPosition)> InteractionInformationCallback;
typedef std::pair<WebKit::InteractionInformationRequest, InteractionInformationCallback> InteractionInformationRequestAndCallback;

#define FOR_EACH_WKCONTENTVIEW_ACTION(M) \
    M(_addShortcut) \
    M(_define) \
    M(_lookup) \
    M(_promptForReplace) \
    M(_share) \
    M(_showTextStyleOptions) \
    M(_transliterateChinese) \
    M(_nextAccessoryTab) \
    M(_previousAccessoryTab) \
    M(copy) \
    M(cut) \
    M(paste) \
    M(replace) \
    M(select) \
    M(selectAll) \
    M(toggleBoldface) \
    M(toggleItalics) \
    M(toggleUnderline) \
    M(increaseSize) \
    M(decreaseSize) \
    M(pasteAndMatchStyle) \
    M(makeTextWritingDirectionNatural) \
    M(makeTextWritingDirectionLeftToRight) \
    M(makeTextWritingDirectionRightToLeft)

#define FOR_EACH_PRIVATE_WKCONTENTVIEW_ACTION(M) \
    M(_alignCenter) \
    M(_alignJustified) \
    M(_alignLeft) \
    M(_alignRight) \
    M(_indent) \
    M(_outdent) \
    M(_toggleStrikeThrough) \
    M(_insertOrderedList) \
    M(_insertUnorderedList) \
    M(_insertNestedOrderedList) \
    M(_insertNestedUnorderedList) \
    M(_increaseListLevel) \
    M(_decreaseListLevel) \
    M(_changeListType) \
    M(_pasteAsQuotation) \
    M(_pasteAndMatchStyle)

namespace WebKit {

enum SuppressSelectionAssistantReason : uint8_t {
    EditableRootIsTransparentOrFullyClipped = 1 << 0,
    FocusedElementIsTooSmall = 1 << 1,
    InteractionIsHappening = 1 << 2
};

struct WKSelectionDrawingInfo {
    enum class SelectionType { None, Plugin, Range };
    WKSelectionDrawingInfo();
    explicit WKSelectionDrawingInfo(const EditorState&);
    SelectionType type;
    WebCore::IntRect caretRect;
    Vector<WebCore::SelectionRect> selectionRects;
    WebCore::IntRect selectionClipRect;
};

WTF::TextStream& operator<<(WTF::TextStream&, const WKSelectionDrawingInfo&);

struct WKAutoCorrectionData {
    RetainPtr<UIFont> font;
    CGRect textFirstRect;
    CGRect textLastRect;
};

}

@class WKFocusedElementInfo;
@protocol WKFormControl;

@interface WKFormInputSession : NSObject <_WKFormInputSession>

- (instancetype)initWithContentView:(WKContentView *)view focusedElementInfo:(WKFocusedElementInfo *)elementInfo requiresStrongPasswordAssistance:(BOOL)requiresStrongPasswordAssistance;
- (void)endEditing;
- (void)invalidate;

@end

@interface WKContentView () {
    RetainPtr<UIWebTouchEventsGestureRecognizer> _touchEventGestureRecognizer;

    BOOL _canSendTouchEventsAsynchronously;
#if ENABLE(POINTER_EVENTS)
    BOOL _preventsPanningInXAxis;
    BOOL _preventsPanningInYAxis;
#endif

    RetainPtr<WKSyntheticTapGestureRecognizer> _singleTapGestureRecognizer;
    RetainPtr<_UIWebHighlightLongPressGestureRecognizer> _highlightLongPressGestureRecognizer;
    RetainPtr<UILongPressGestureRecognizer> _longPressGestureRecognizer;
    RetainPtr<WKSyntheticTapGestureRecognizer> _doubleTapGestureRecognizer;
    RetainPtr<UITapGestureRecognizer> _nonBlockingDoubleTapGestureRecognizer;
    RetainPtr<UITapGestureRecognizer> _doubleTapGestureRecognizerForDoubleClick;
    RetainPtr<UITapGestureRecognizer> _twoFingerDoubleTapGestureRecognizer;
    RetainPtr<UITapGestureRecognizer> _twoFingerSingleTapGestureRecognizer;
    RetainPtr<UITapGestureRecognizer> _stylusSingleTapGestureRecognizer;
    RetainPtr<WKInspectorNodeSearchGestureRecognizer> _inspectorNodeSearchGestureRecognizer;

#if ENABLE(POINTER_EVENTS)
    RetainPtr<WKTouchActionGestureRecognizer> _touchActionGestureRecognizer;
#endif

#if PLATFORM(MACCATALYST)
    RetainPtr<UIHoverGestureRecognizer> _hoverGestureRecognizer;
    RetainPtr<_UILookupGestureRecognizer> _lookupGestureRecognizer;
    CGPoint _lastHoverLocation;
#endif

    RetainPtr<UIWKTextInteractionAssistant> _textSelectionAssistant;
    OptionSet<WebKit::SuppressSelectionAssistantReason> _suppressSelectionAssistantReasons;

    RetainPtr<UITextInputTraits> _traits;
    RetainPtr<UIWebFormAccessory> _formAccessoryView;
    RetainPtr<_UIHighlightView> _highlightView;
    RetainPtr<UIView> _interactionViewsContainerView;
    RetainPtr<UIView> _contextMenuHintContainerView;
    RetainPtr<NSString> _markedText;
    RetainPtr<WKActionSheetAssistant> _actionSheetAssistant;
#if ENABLE(AIRPLAY_PICKER)
    RetainPtr<WKAirPlayRoutePicker> _airPlayRoutePicker;
#endif
    RetainPtr<WKFormInputSession> _formInputSession;
    RetainPtr<WKFileUploadPanel> _fileUploadPanel;
#if !PLATFORM(WATCHOS) && !PLATFORM(APPLETV)
    RetainPtr<WKShareSheet> _shareSheet;
#endif
    RetainPtr<UIGestureRecognizer> _previewGestureRecognizer;
    RetainPtr<UIGestureRecognizer> _previewSecondaryGestureRecognizer;
    Vector<bool> _focusStateStack;
#if HAVE(LINK_PREVIEW)
#if USE(UICONTEXTMENU)
    RetainPtr<UIContextMenuInteraction> _contextMenuInteraction;
    RetainPtr<WKContextMenuElementInfo> _contextMenuElementInfo;
    BOOL _showLinkPreviews;
    RetainPtr<UIViewController> _contextMenuLegacyPreviewController;
    RetainPtr<UIMenu> _contextMenuLegacyMenu;
    BOOL _contextMenuHasRequestedLegacyData;
    BOOL _contextMenuActionProviderDelegateNeedsOverride;
    RetainPtr<UITargetedPreview> _contextMenuInteractionTargetedPreview;
#endif
    RetainPtr<UIPreviewItemController> _previewItemController;
#endif

    std::unique_ptr<WebKit::SmartMagnificationController> _smartMagnificationController;

    WeakObjCPtr<id <UITextInputDelegate>> _inputDelegate;

    uint64_t _latestTapID;
    struct TapHighlightInformation {
        BOOL nodeHasBuiltInClickHandling { false };
        WebCore::Color color;
        Vector<WebCore::FloatQuad> quads;
        WebCore::IntSize topLeftRadius;
        WebCore::IntSize topRightRadius;
        WebCore::IntSize bottomLeftRadius;
        WebCore::IntSize bottomRightRadius;
    };
    TapHighlightInformation _tapHighlightInformation;

    WebKit::WKAutoCorrectionData _autocorrectionData;
    WebKit::InteractionInformationAtPosition _positionInformation;
    WebKit::FocusedElementInformation _focusedElementInformation;
    RetainPtr<NSObject<WKFormPeripheral>> _inputPeripheral;
#if !USE(UIKIT_KEYBOARD_ADDITIONS)
    RetainPtr<UIEvent> _uiEventBeingResent;
#endif
    BlockPtr<void(::WebEvent *, BOOL)> _keyWebEventHandler;

    CGPoint _lastInteractionLocation;
    WebKit::TransactionID _layerTreeTransactionIdAtLastTouchStart;

    WebKit::WKSelectionDrawingInfo _lastSelectionDrawingInfo;

    Optional<WebKit::InteractionInformationRequest> _outstandingPositionInformationRequest;

    uint64_t _positionInformationCallbackDepth;
    Vector<Optional<InteractionInformationRequestAndCallback>> _pendingPositionInformationHandlers;
    
    std::unique_ptr<WebKit::InputViewUpdateDeferrer> _inputViewUpdateDeferrer;

    RetainPtr<WKKeyboardScrollViewAnimator> _keyboardScrollingAnimator;

#if ENABLE(DATALIST_ELEMENT)
    RetainPtr<UIView <WKFormControl>> _dataListTextSuggestionsInputView;
    RetainPtr<NSArray<UITextSuggestion *>> _dataListTextSuggestions;
#endif

#if HAVE(PENCILKIT)
    RetainPtr<WKDrawingCoordinator> _drawingCoordinator;
#endif

    BOOL _isEditable;
    BOOL _showingTextStyleOptions;
    BOOL _hasValidPositionInformation;
    BOOL _isTapHighlightIDValid;
    BOOL _potentialTapInProgress;
    BOOL _isDoubleTapPending;
    BOOL _longPressCanClick;
    BOOL _hasTapHighlightForPotentialTap;
    BOOL _selectionNeedsUpdate;
    BOOL _shouldRestoreSelection;
    BOOL _usingGestureForSelection;
    BOOL _inspectorNodeSearchEnabled;
    BOOL _isChangingFocusUsingAccessoryTab;
    BOOL _didAccessoryTabInitiateFocus;
    BOOL _isExpectingFastSingleTapCommit;
    BOOL _showDebugTapHighlightsForFastClicking;

#if ENABLE(POINTER_EVENTS)
    WebCore::PointerID m_commitPotentialTapPointerId;
#endif

    BOOL _keyboardDidRequestDismissal;

#if USE(UIKIT_KEYBOARD_ADDITIONS)
    BOOL _candidateViewNeedsUpdate;
    BOOL _seenHardwareKeyDownInNonEditableElement;
#endif

    BOOL _becomingFirstResponder;
    BOOL _resigningFirstResponder;
    BOOL _needsDeferredEndScrollingSelectionUpdate;
    BOOL _isChangingFocus;
    BOOL _isFocusingElementWithKeyboard;
    BOOL _isBlurringFocusedElement;

    BOOL _focusRequiresStrongPasswordAssistance;
    BOOL _waitingForEditDragSnapshot;

    BOOL _hasSetUpInteractions;
    NSUInteger _ignoreSelectionCommandFadeCount;
    NSInteger _suppressNonEditableSingleTapTextInteractionCount;
    CompletionHandler<void(WebCore::DOMPasteAccessResponse)> _domPasteRequestHandler;
    BlockPtr<void(UIWKAutocorrectionContext *)> _pendingAutocorrectionContextHandler;

    RetainPtr<NSDictionary> _additionalContextForStrongPasswordAssistance;

#if ENABLE(DATA_INTERACTION)
    WebKit::DragDropInteractionState _dragDropInteractionState;
    RetainPtr<UIDragInteraction> _dragInteraction;
    RetainPtr<UIDropInteraction> _dropInteraction;
    BOOL _shouldRestoreCalloutBarAfterDrop;
    RetainPtr<UIView> _visibleContentViewSnapshot;
    RetainPtr<UIView> _unselectedContentSnapshot;
    RetainPtr<_UITextDragCaretView> _editDropCaretView;
    BlockPtr<void()> _actionToPerformAfterReceivingEditDragSnapshot;
#endif

#if PLATFORM(WATCHOS)
    RetainPtr<WKFocusedFormControlView> _focusedFormControlView;
    RetainPtr<UIViewController> _presentedFullScreenInputViewController;
    RetainPtr<UINavigationController> _inputNavigationViewControllerForFullScreenInputs;

    BOOL _shouldRestoreFirstResponderStatusAfterLosingFocus;
    BlockPtr<void()> _activeFocusedStateRetainBlock;
#endif

#if ENABLE(PLATFORM_DRIVEN_TEXT_CHECKING)
    std::unique_ptr<WebKit::TextCheckingController> _textCheckingController;
#endif

    Vector<BlockPtr<void()>> _actionsToPerformAfterResettingSingleTapGestureRecognizer;
}

@end

@interface WKContentView (WKInteraction) <UIGestureRecognizerDelegate, UITextAutoscrolling, UITextInputMultiDocument, UITextInputPrivate, UIWebFormAccessoryDelegate, UIWebTouchEventsGestureRecognizerDelegate, UIWKInteractionViewProtocol, WKActionSheetAssistantDelegate, WKFileUploadPanelDelegate, WKKeyboardScrollViewAnimatorDelegate
#if !PLATFORM(WATCHOS) && !PLATFORM(APPLETV)
    , WKShareSheetDelegate
#endif
#if ENABLE(DATA_INTERACTION)
    , UIDragInteractionDelegate, UIDropInteractionDelegate
#endif
#if PLATFORM(IOS_FAMILY) && ENABLE(POINTER_EVENTS)
    , WKTouchActionGestureRecognizerDelegate
#endif
>

@property (nonatomic, readonly) CGPoint lastInteractionLocation;
@property (nonatomic, readonly) BOOL isEditable;
@property (nonatomic, readonly) BOOL shouldHideSelectionWhenScrolling;
@property (nonatomic, readonly) BOOL shouldIgnoreKeyboardWillHideNotification;
@property (nonatomic, readonly) const WebKit::InteractionInformationAtPosition& positionInformation;
@property (nonatomic, readonly) const WebKit::WKAutoCorrectionData& autocorrectionData;
@property (nonatomic, readonly) const WebKit::FocusedElementInformation& focusedElementInformation;
@property (nonatomic, readonly) UIWebFormAccessory *formAccessoryView;
@property (nonatomic, readonly) UITextInputAssistantItem *inputAssistantItemForWebView;
@property (nonatomic, readonly) UIView *inputViewForWebView;
@property (nonatomic, readonly) UIView *inputAccessoryViewForWebView;
#if ENABLE(POINTER_EVENTS)
@property (nonatomic, readonly) BOOL preventsPanningInXAxis;
@property (nonatomic, readonly) BOOL preventsPanningInYAxis;
#endif

#if ENABLE(DATALIST_ELEMENT)
@property (nonatomic, strong) UIView <WKFormControl> *dataListTextSuggestionsInputView;
@property (nonatomic, strong) NSArray<UITextSuggestion *> *dataListTextSuggestions;
#endif

- (void)setupInteraction;
- (void)cleanupInteraction;

- (void)scrollViewWillStartPanOrPinchGesture;

- (BOOL)canBecomeFirstResponderForWebView;
- (BOOL)becomeFirstResponderForWebView;
- (BOOL)resignFirstResponderForWebView;
- (BOOL)canPerformActionForWebView:(SEL)action withSender:(id)sender;
- (id)targetForActionForWebView:(SEL)action withSender:(id)sender;

#if ENABLE(POINTER_EVENTS)
- (void)cancelPointersForGestureRecognizer:(UIGestureRecognizer *)gestureRecognizer;
- (WTF::Optional<unsigned>)activeTouchIdentifierForGestureRecognizer:(UIGestureRecognizer *)gestureRecognizer;
#endif

#define DECLARE_WKCONTENTVIEW_ACTION_FOR_WEB_VIEW(_action) \
    - (void)_action ## ForWebView:(id)sender;
FOR_EACH_WKCONTENTVIEW_ACTION(DECLARE_WKCONTENTVIEW_ACTION_FOR_WEB_VIEW)
FOR_EACH_PRIVATE_WKCONTENTVIEW_ACTION(DECLARE_WKCONTENTVIEW_ACTION_FOR_WEB_VIEW)
#undef DECLARE_WKCONTENTVIEW_ACTION_FOR_WEB_VIEW

- (void)_setFontForWebView:(UIFont *)fontFamily sender:(id)sender;
- (void)_setFontSizeForWebView:(CGFloat)fontSize sender:(id)sender;
- (void)_setTextColorForWebView:(UIColor *)color sender:(id)sender;

#if ENABLE(TOUCH_EVENTS)
- (void)_webTouchEvent:(const WebKit::NativeWebTouchEvent&)touchEvent preventsNativeGestures:(BOOL)preventsDefault;
#endif
- (void)_commitPotentialTapFailed;
- (void)_didNotHandleTapAsClick:(const WebCore::IntPoint&)point;
- (void)_didCompleteSyntheticClick;
- (void)_didGetTapHighlightForRequest:(uint64_t)requestID color:(const WebCore::Color&)color quads:(const Vector<WebCore::FloatQuad>&)highlightedQuads topLeftRadius:(const WebCore::IntSize&)topLeftRadius topRightRadius:(const WebCore::IntSize&)topRightRadius bottomLeftRadius:(const WebCore::IntSize&)bottomLeftRadius bottomRightRadius:(const WebCore::IntSize&)bottomRightRadius nodeHasBuiltInClickHandling:(BOOL)nodeHasBuiltInClickHandling;

- (BOOL)_mayDisableDoubleTapGesturesDuringSingleTap;
- (void)_disableDoubleTapGesturesDuringTapIfNecessary:(uint64_t)requestID;
- (void)_handleSmartMagnificationInformationForPotentialTap:(uint64_t)requestID renderRect:(const WebCore::FloatRect&)renderRect fitEntireRect:(BOOL)fitEntireRect viewportMinimumScale:(double)viewportMinimumScale viewportMaximumScale:(double)viewportMaximumScale;
- (void)_elementDidFocus:(const WebKit::FocusedElementInformation&)information userIsInteracting:(BOOL)userIsInteracting blurPreviousNode:(BOOL)blurPreviousNode activityStateChanges:(OptionSet<WebCore::ActivityState::Flag>)activityStateChanges userObject:(NSObject <NSSecureCoding> *)userObject;
- (void)_updateInputContextAfterBlurringAndRefocusingElement;
- (void)_elementDidBlur;
- (void)_hideContextMenuHintContainer;
- (void)_didUpdateInputMode:(WebCore::InputMode)mode;
- (void)_didReceiveEditorStateUpdateAfterFocus;
- (void)_hardwareKeyboardAvailabilityChanged;
- (void)_selectionChanged;
- (void)_updateChangedSelection;
- (BOOL)_interpretKeyEvent:(::WebEvent *)theEvent isCharEvent:(BOOL)isCharEvent;
- (void)_positionInformationDidChange:(const WebKit::InteractionInformationAtPosition&)info;
- (void)_attemptClickAtLocation:(CGPoint)location modifierFlags:(UIKeyModifierFlags)modifierFlags;
- (void)_willStartScrollingOrZooming;
- (void)_didScroll;
- (void)_didEndScrollingOrZooming;
- (void)_scrollingNodeScrollingWillBegin;
- (void)_scrollingNodeScrollingDidEnd;
- (void)_showPlaybackTargetPicker:(BOOL)hasVideo fromRect:(const WebCore::IntRect&)elementRect routeSharingPolicy:(WebCore::RouteSharingPolicy)policy routingContextUID:(NSString *)contextUID;
- (void)_showRunOpenPanel:(API::OpenPanelParameters*)parameters resultListener:(WebKit::WebOpenPanelResultListenerProxy*)listener;
- (void)_showShareSheet:(const WebCore::ShareDataWithParsedURL&)shareData inRect:(WTF::Optional<WebCore::FloatRect>)rect completionHandler:(WTF::CompletionHandler<void(bool)>&&)completionHandler;
- (void)dismissFilePicker;
- (void)_didHandleKeyEvent:(::WebEvent *)event eventWasHandled:(BOOL)eventWasHandled;
- (Vector<WebKit::OptionItem>&) focusedSelectElementOptions;
- (void)_enableInspectorNodeSearch;
- (void)_disableInspectorNodeSearch;
- (void)_becomeFirstResponderWithSelectionMovingForward:(BOOL)selectingForward completionHandler:(void (^)(BOOL didBecomeFirstResponder))completionHandler;
- (void)_setDoubleTapGesturesEnabled:(BOOL)enabled;
#if ENABLE(DATA_DETECTION)
- (NSArray *)_dataDetectionResults;
#endif
- (NSArray<NSValue *> *)_uiTextSelectionRects;
- (void)accessibilityRetrieveSpeakSelectionContent;
- (void)_accessibilityRetrieveRectsEnclosingSelectionOffset:(NSInteger)offset withGranularity:(UITextGranularity)granularity;
- (void)_accessibilityRetrieveRectsAtSelectionOffset:(NSInteger)offset withText:(NSString *)text completionHandler:(void (^)(const Vector<WebCore::SelectionRect>& rects))completionHandler;
- (void)_accessibilityRetrieveRectsAtSelectionOffset:(NSInteger)offset withText:(NSString *)text;
- (void)_accessibilityStoreSelection;
- (void)_accessibilityClearSelection;
- (WKFormInputSession *)_formInputSession;
- (void)_didChangeWebViewEditability;
- (NSDictionary *)dataDetectionContextForPositionInformation:(WebKit::InteractionInformationAtPosition)positionInformation;

- (void)willFinishIgnoringCalloutBarFadeAfterPerformingAction;

- (BOOL)hasHiddenContentEditable;
- (void)generateSyntheticEditingCommand:(WebKit::SyntheticEditingCommandType)command;

// UIWebFormAccessoryDelegate protocol
- (void)accessoryDone;

- (void)accessoryOpen;

- (void)_requestDOMPasteAccessWithElementRect:(const WebCore::IntRect&)elementRect originIdentifier:(const String&)originIdentifier completionHandler:(CompletionHandler<void(WebCore::DOMPasteAccessResponse)>&&)completionHandler;

@property (nonatomic, readonly) WebKit::InteractionInformationAtPosition currentPositionInformation;
- (void)doAfterPositionInformationUpdate:(void (^)(WebKit::InteractionInformationAtPosition))action forRequest:(WebKit::InteractionInformationRequest)request;
- (BOOL)ensurePositionInformationIsUpToDate:(WebKit::InteractionInformationRequest)request;

#if ENABLE(DATA_INTERACTION)
- (void)_didChangeDragInteractionPolicy;
- (void)_didPerformDragOperation:(BOOL)handled;
- (void)_didHandleDragStartRequest:(BOOL)started;
- (void)_didHandleAdditionalDragItemsRequest:(BOOL)added;
- (void)_startDrag:(RetainPtr<CGImageRef>)image item:(const WebCore::DragItem&)item;
- (void)_willReceiveEditDragSnapshot;
- (void)_didReceiveEditDragSnapshot:(Optional<WebCore::TextIndicatorData>)data;
- (void)_didChangeDragCaretRect:(CGRect)previousRect currentRect:(CGRect)rect;
#endif

- (void)reloadContextViewForPresentedListViewController;

#if ENABLE(DATALIST_ELEMENT)
- (void)updateTextSuggestionsForInputDelegate;
#endif

#if HAVE(PENCILKIT)
- (WKDrawingCoordinator *)_drawingCoordinator;
#endif

- (void)_handleAutocorrectionContext:(const WebKit::WebAutocorrectionContext&)context;

- (void)_didStartProvisionalLoadForMainFrame;
- (void)_didCommitLoadForMainFrame;

@property (nonatomic, readonly) BOOL _shouldUseContextMenus;
@property (nonatomic, readonly) BOOL _shouldAvoidResizingWhenInputViewBoundsChange;
@property (nonatomic, readonly) BOOL _shouldAvoidScrollingWhenFocusedContentIsVisible;

@end

@interface WKContentView (WKTesting)

- (void)_simulateLongPressActionAtLocation:(CGPoint)location;
- (void)_simulateTextEntered:(NSString *)text;
- (void)selectFormAccessoryPickerRow:(NSInteger)rowIndex;
- (void)setTimePickerValueToHour:(NSInteger)hour minute:(NSInteger)minute;
- (NSDictionary *)_contentsOfUserInterfaceItem:(NSString *)userInterfaceItem;
- (void)_doAfterResettingSingleTapGesture:(dispatch_block_t)action;
- (void)_doAfterReceivingEditDragSnapshotForTesting:(dispatch_block_t)action;

@property (nonatomic, readonly) NSString *textContentTypeForTesting;
@property (nonatomic, readonly) NSString *selectFormPopoverTitle;
@property (nonatomic, readonly) NSString *formInputLabel;
@property (nonatomic, readonly) WKFormInputControl *formInputControl;

@end

#if HAVE(LINK_PREVIEW)
#if USE(UICONTEXTMENU)
@interface WKContentView (WKInteractionPreview) <UIContextMenuInteractionDelegate, UIPreviewItemDelegate>
#else
@interface WKContentView (WKInteractionPreview) <UIPreviewItemDelegate>
#endif
- (void)_registerPreview;
- (void)_unregisterPreview;
@end
#endif

@interface WKContentView (WKFileUploadPanel)
+ (Class)_fileUploadPanelClass;
@end

#endif // PLATFORM(IOS_FAMILY)
