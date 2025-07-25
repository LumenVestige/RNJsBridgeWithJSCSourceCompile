/*
 * Copyright (C) 2010-2019 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
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

#include "config.h"
#include "WebPageProxy.h"

#include "APIArray.h"
#include "APIAttachment.h"
#include "APIContextMenuClient.h"
#include "APIFindClient.h"
#include "APIFindMatchesClient.h"
#include "APIFormClient.h"
#include "APIFrameInfo.h"
#include "APIFullscreenClient.h"
#include "APIGeometry.h"
#include "APIHistoryClient.h"
#include "APIHitTestResult.h"
#include "APIIconLoadingClient.h"
#include "APILegacyContextHistoryClient.h"
#include "APILoaderClient.h"
#include "APINavigation.h"
#include "APINavigationAction.h"
#include "APINavigationClient.h"
#include "APINavigationResponse.h"
#include "APIOpenPanelParameters.h"
#include "APIPageConfiguration.h"
#include "APIPolicyClient.h"
#include "APISecurityOrigin.h"
#include "APIUIClient.h"
#include "APIURLRequest.h"
#include "APIWebsitePolicies.h"
#include "AuthenticationChallengeProxy.h"
#include "AuthenticationDecisionListener.h"
#include "DataReference.h"
#include "DownloadProxy.h"
#include "DrawingAreaMessages.h"
#include "DrawingAreaProxy.h"
#include "EventDispatcherMessages.h"
#include "FormDataReference.h"
#include "FrameInfoData.h"
#include "LoadParameters.h"
#include "Logging.h"
#include "NativeWebGestureEvent.h"
#include "NativeWebKeyboardEvent.h"
#include "NativeWebMouseEvent.h"
#include "NativeWebWheelEvent.h"
#include "NavigationActionData.h"
#include "NetworkProcessMessages.h"
#include "NetworkProcessProxy.h"
#include "NotificationPermissionRequest.h"
#include "NotificationPermissionRequestManager.h"
#include "OptionalCallbackID.h"
#include "PageClient.h"
#include "PluginInformation.h"
#include "PluginProcessManager.h"
#include "PrintInfo.h"
#include "ProvisionalPageProxy.h"
#include "SafeBrowsingWarning.h"
#include "ShareSheetCallbackID.h"
#include "SharedBufferDataReference.h"
#include "SyntheticEditingCommandType.h"
#include "TextChecker.h"
#include "TextCheckerState.h"
#include "TextInputContext.h"
#include "UIMessagePortChannelProvider.h"
#include "URLSchemeTaskParameters.h"
#include "UndoOrRedo.h"
#include "UserMediaPermissionRequestProxy.h"
#include "UserMediaProcessManager.h"
#include "WKContextPrivate.h"
#include "WebAutomationSession.h"
#include "WebBackForwardList.h"
#include "WebBackForwardListItem.h"
#include "WebCertificateInfo.h"
#include "WebContextMenuItem.h"
#include "WebContextMenuProxy.h"
#include "WebCoreArgumentCoders.h"
#include "WebEditCommandProxy.h"
#include "WebEvent.h"
#include "WebEventConversion.h"
#include "WebFramePolicyListenerProxy.h"
#include "WebFullScreenManagerProxy.h"
#include "WebFullScreenManagerProxyMessages.h"
#include "WebImage.h"
#include "WebInspectorProxy.h"
#include "WebInspectorUtilities.h"
#include "WebNavigationDataStore.h"
#include "WebNavigationState.h"
#include "WebNotificationManagerProxy.h"
#include "WebOpenPanelResultListenerProxy.h"
#include "WebPageCreationParameters.h"
#include "WebPageDebuggable.h"
#include "WebPageGroup.h"
#include "WebPageGroupData.h"
#include "WebPageInspectorController.h"
#include "WebPageMessages.h"
#include "WebPageProxyMessages.h"
#include "WebPaymentCoordinatorProxy.h"
#include "WebPopupItem.h"
#include "WebPopupMenuProxy.h"
#include "WebPreferences.h"
#include "WebPreferencesKeys.h"
#include "WebProcessMessages.h"
#include "WebProcessPool.h"
#include "WebProcessProxy.h"
#include "WebProtectionSpace.h"
#include "WebResourceLoadStatisticsStore.h"
#include "WebURLSchemeHandler.h"
#include "WebUserContentControllerProxy.h"
#include "WebViewDidMoveToWindowObserver.h"
#include "WebsiteDataStore.h"
#include <WebCore/AdClickAttribution.h>
#include <WebCore/BitmapImage.h>
#include <WebCore/CrossSiteNavigationDataTransfer.h>
#include <WebCore/DOMPasteAccess.h>
#include <WebCore/DeprecatedGlobalSettings.h>
#include <WebCore/DiagnosticLoggingClient.h>
#include <WebCore/DiagnosticLoggingKeys.h>
#include <WebCore/DragController.h>
#include <WebCore/DragData.h>
#include <WebCore/EventNames.h>
#include <WebCore/FloatRect.h>
#include <WebCore/FocusDirection.h>
#include <WebCore/FontAttributeChanges.h>
#include <WebCore/FrameLoader.h>
#include <WebCore/GlobalFrameIdentifier.h>
#include <WebCore/GlobalWindowIdentifier.h>
#include <WebCore/JSDOMBinding.h>
#include <WebCore/JSDOMExceptionHandling.h>
#include <WebCore/LengthBox.h>
#include <WebCore/MIMETypeRegistry.h>
#include <WebCore/MediaStreamRequest.h>
#include <WebCore/PerformanceLoggingClient.h>
#include <WebCore/PlatformEvent.h>
#include <WebCore/PublicSuffix.h>
#include <WebCore/RenderEmbeddedObject.h>
#include <WebCore/ResourceLoadStatistics.h>
#include <WebCore/SSLKeyGenerator.h>
#include <WebCore/SerializedCryptoKeyWrap.h>
#include <WebCore/ShareData.h>
#include <WebCore/SharedBuffer.h>
#include <WebCore/ShouldTreatAsContinuingLoad.h>
#include <WebCore/TextCheckerClient.h>
#include <WebCore/TextIndicator.h>
#include <WebCore/ValidationBubble.h>
#include <WebCore/WindowFeatures.h>
#include <WebCore/WritingDirection.h>
#include <stdio.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/SystemTracing.h>
#include <wtf/URL.h>
#include <wtf/URLParser.h>
#include <wtf/text/StringView.h>
#include <wtf/text/TextStream.h>

#if ENABLE(APPLICATION_MANIFEST)
#include "APIApplicationManifest.h"
#endif

#if ENABLE(ASYNC_SCROLLING) && PLATFORM(COCOA)
#include "RemoteScrollingCoordinatorProxy.h"
#endif

#ifndef NDEBUG
#include <wtf/RefCountedLeakCounter.h>
#endif

#if PLATFORM(COCOA)
#include "AttributedString.h"
#include "InsertTextOptions.h"
#include "RemoteLayerTreeDrawingAreaProxy.h"
#include "RemoteLayerTreeScrollingPerformanceData.h"
#include "TouchBarMenuData.h"
#include "TouchBarMenuItemData.h"
#include "VersionChecks.h"
#include "VideoFullscreenManagerProxy.h"
#include "VideoFullscreenManagerProxyMessages.h"
#include <WebCore/RunLoopObserver.h>
#include <WebCore/TextIndicatorWindow.h>
#include <wtf/MachSendRight.h>
#endif

#if PLATFORM(COCOA) || PLATFORM(GTK)
#include "ViewSnapshotStore.h"
#endif

#if PLATFORM(GTK)
#include "WebSelectionData.h"
#endif

#if USE(CAIRO)
#include <WebCore/CairoUtilities.h>
#endif

#if ENABLE(WIRELESS_PLAYBACK_TARGET) && !PLATFORM(IOS_FAMILY)
#include <WebCore/MediaPlaybackTarget.h>
#include <WebCore/WebMediaSessionManager.h>
#endif

#if ENABLE(MEDIA_SESSION)
#include "WebMediaSessionFocusManager.h"
#include "WebMediaSessionMetadata.h"
#include <WebCore/MediaSessionMetadata.h>
#endif

#if PLATFORM(IOS_FAMILY) || (PLATFORM(MAC) && ENABLE(VIDEO_PRESENTATION_MODE))
#include "PlaybackSessionManagerProxy.h"
#endif

#if ENABLE(WEB_AUTHN)
#include "WebAuthenticatorCoordinatorProxy.h"
#endif

#if ENABLE(REMOTE_INSPECTOR)
#include <JavaScriptCore/RemoteInspector.h>
#endif

#if HAVE(SEC_KEY_PROXY)
#include "SecKeyProxyStore.h"
#endif

#if HAVE(PENCILKIT)
#include "EditableImageController.h"
#endif

#if HAVE(APP_SSO)
#include "SOAuthorizationCoordinator.h"
#endif

// This controls what strategy we use for mouse wheel coalescing.
#define MERGE_WHEEL_EVENTS 1

#define MESSAGE_CHECK(process, assertion) MESSAGE_CHECK_BASE(assertion, process->connection())
#define MESSAGE_CHECK_URL(process, url) MESSAGE_CHECK_BASE(checkURLReceivedFromCurrentOrPreviousWebProcess(process, url), process->connection())

#define RELEASE_LOG_IF_ALLOWED(channel, fmt, ...) RELEASE_LOG_IF(isAlwaysOnLoggingAllowed(), channel, "%p - WebPageProxy::" fmt, this, ##__VA_ARGS__)
#define RELEASE_LOG_ERROR_IF_ALLOWED(channel, fmt, ...) RELEASE_LOG_ERROR_IF(isAlwaysOnLoggingAllowed(), channel, "%p - WebPageProxy::" fmt, this, ##__VA_ARGS__)

// Represents the number of wheel events we can hold in the queue before we start pushing them preemptively.
static const unsigned wheelEventQueueSizeThreshold = 10;

static const Seconds resetRecentCrashCountDelay = 30_s;
static unsigned maximumWebProcessRelaunchAttempts = 1;

namespace WebKit {
using namespace WebCore;

DEFINE_DEBUG_ONLY_GLOBAL(WTF::RefCountedLeakCounter, webPageProxyCounter, ("WebPageProxy"));

class StorageRequests {
    WTF_MAKE_NONCOPYABLE(StorageRequests); WTF_MAKE_FAST_ALLOCATED;
    friend NeverDestroyed<StorageRequests>;
public:
    static StorageRequests& singleton();

    void processOrAppend(CompletionHandler<void()>&& completionHandler)
    {
        if (m_requestsAreBeingProcessed) {
            m_requests.append(WTFMove(completionHandler));
            return;
        }
        m_requestsAreBeingProcessed = true;
        completionHandler();
    }

    void processNextIfAny()
    {
        if (m_requests.isEmpty()) {
            m_requestsAreBeingProcessed = false;
            return;
        }
        m_requests.takeFirst()();
    }

private:
    StorageRequests() { }
    ~StorageRequests() { }

    Deque<CompletionHandler<void()>> m_requests;
    bool m_requestsAreBeingProcessed { false };
};

StorageRequests& StorageRequests::singleton()
{
    static NeverDestroyed<StorageRequests> requests;
    return requests;
}

#if !LOG_DISABLED
static const char* webMouseEventTypeString(WebEvent::Type type)
{
    switch (type) {
    case WebEvent::MouseDown:
        return "MouseDown";
    case WebEvent::MouseUp:
        return "MouseUp";
    case WebEvent::MouseMove:
        return "MouseMove";
    case WebEvent::MouseForceChanged:
        return "MouseForceChanged";
    case WebEvent::MouseForceDown:
        return "MouseForceDown";
    case WebEvent::MouseForceUp:
        return "MouseForceUp";
    default:
        ASSERT_NOT_REACHED();
        return "<unknown>";
    }
}

static const char* webKeyboardEventTypeString(WebEvent::Type type)
{
    switch (type) {
    case WebEvent::KeyDown:
        return "KeyDown";
    case WebEvent::KeyUp:
        return "KeyUp";
    case WebEvent::RawKeyDown:
        return "RawKeyDown";
    case WebEvent::Char:
        return "Char";
    default:
        ASSERT_NOT_REACHED();
        return "<unknown>";
    }
}
#endif // !LOG_DISABLED

class PageClientProtector {
    WTF_MAKE_NONCOPYABLE(PageClientProtector);
public:
    PageClientProtector(PageClient& pageClient)
        : m_pageClient(makeWeakPtr(pageClient))
    {
        m_pageClient->refView();
    }

    ~PageClientProtector()
    {
        ASSERT(m_pageClient);
        m_pageClient->derefView();
    }

private:
    WeakPtr<PageClient> m_pageClient;
};

void WebPageProxy::forMostVisibleWebPageIfAny(PAL::SessionID sessionID, const SecurityOriginData& origin, CompletionHandler<void(WebPageProxy*)>&& completionHandler)
{
    // FIXME: If not finding right away a visible page, we might want to try again for a given period of time when there is a change of visibility.
    WebPageProxy* selectedPage = nullptr;
    WebProcessProxy::forWebPagesWithOrigin(sessionID, origin, [&](auto& page) {
        if (!page.mainFrame())
            return;
        if (page.isViewVisible() && (!selectedPage || !selectedPage->isViewVisible())) {
            selectedPage = &page;
            return;
        }
        if (page.isViewFocused() && (!selectedPage || !selectedPage->isViewFocused())) {
            selectedPage = &page;
            return;
        }
    });
    completionHandler(selectedPage);
}

Ref<WebPageProxy> WebPageProxy::create(PageClient& pageClient, WebProcessProxy& process, Ref<API::PageConfiguration>&& configuration)
{
    return adoptRef(*new WebPageProxy(pageClient, process, WTFMove(configuration)));
}

WebPageProxy::WebPageProxy(PageClient& pageClient, WebProcessProxy& process, Ref<API::PageConfiguration>&& configuration)
    : m_pageID(PageIdentifier::generate())
    , m_pageClient(makeWeakPtr(pageClient))
    , m_configuration(WTFMove(configuration))
    , m_navigationClient(makeUniqueRef<API::NavigationClient>())
    , m_historyClient(makeUniqueRef<API::HistoryClient>())
    , m_iconLoadingClient(makeUnique<API::IconLoadingClient>())
    , m_formClient(makeUnique<API::FormClient>())
    , m_uiClient(makeUnique<API::UIClient>())
    , m_findClient(makeUnique<API::FindClient>())
    , m_findMatchesClient(makeUnique<API::FindMatchesClient>())
#if ENABLE(CONTEXT_MENUS)
    , m_contextMenuClient(makeUnique<API::ContextMenuClient>())
#endif
    , m_navigationState(makeUnique<WebNavigationState>())
    , m_process(process)
    , m_pageGroup(*m_configuration->pageGroup())
    , m_preferences(*m_configuration->preferences())
    , m_userContentController(*m_configuration->userContentController())
    , m_visitedLinkStore(*m_configuration->visitedLinkStore())
    , m_websiteDataStore(m_configuration->websiteDataStore()->websiteDataStore())
    , m_userAgent(standardUserAgent())
    , m_overrideContentSecurityPolicy { m_configuration->overrideContentSecurityPolicy() }
    , m_treatsSHA1CertificatesAsInsecure(m_configuration->treatsSHA1SignedCertificatesAsInsecure())
#if ENABLE(FULLSCREEN_API)
    , m_fullscreenClient(makeUnique<API::FullscreenClient>())
#endif
    , m_geolocationPermissionRequestManager(*this)
    , m_notificationPermissionRequestManager(*this)
#if PLATFORM(IOS_FAMILY)
    , m_alwaysRunsAtForegroundPriority(m_configuration->alwaysRunsAtForegroundPriority())
#endif
    , m_initialCapitalizationEnabled(m_configuration->initialCapitalizationEnabled())
    , m_cpuLimit(m_configuration->cpuLimit())
    , m_backForwardList(WebBackForwardList::create(*this))
    , m_waitsForPaintAfterViewDidMoveToWindow(m_configuration->waitsForPaintAfterViewDidMoveToWindow())
    , m_hasRunningProcess(process.state() != WebProcessProxy::State::Terminated)
    , m_controlledByAutomation(m_configuration->isControlledByAutomation())
#if PLATFORM(COCOA)
    , m_isSmartInsertDeleteEnabled(TextChecker::isSmartInsertDeleteEnabled())
#endif
    , m_pageLoadState(*this)
    , m_configurationPreferenceValues(m_configuration->preferenceValues())
    , m_inspectorController(makeUnique<WebPageInspectorController>(*this))
#if ENABLE(REMOTE_INSPECTOR)
    , m_inspectorDebuggable(makeUnique<WebPageDebuggable>(*this))
#endif
    , m_resetRecentCrashCountTimer(RunLoop::main(), this, &WebPageProxy::resetRecentCrashCount)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "constructor: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (!m_configuration->drawsBackground())
        m_backgroundColor = Color(Color::transparent);

    updateActivityState();
    updateThrottleState();
    updateHiddenPageThrottlingAutoIncreases();
    
#if HAVE(OUT_OF_PROCESS_LAYER_HOSTING)
    m_layerHostingMode = m_activityState & ActivityState::IsInWindow ? WebPageProxy::pageClient().viewLayerHostingMode() : LayerHostingMode::OutOfProcess;
#endif

    platformInitialize();

#ifndef NDEBUG
    webPageProxyCounter.increment();
#endif

    WebProcessPool::statistics().wkPageCount++;

    m_preferences->addPage(*this);
    m_pageGroup->addPage(this);

    m_inspector = WebInspectorProxy::create(this);

    if (hasRunningProcess())
        didAttachToRunningProcess();

    m_process->addMessageReceiver(Messages::WebPageProxy::messageReceiverName(), m_pageID, *this);

#if PLATFORM(IOS_FAMILY)
    DeprecatedGlobalSettings::setDisableScreenSizeOverride(preferencesStore().getBoolValueForKey(WebPreferencesKey::disableScreenSizeOverrideKey()));
#endif

#if PLATFORM(COCOA)
    m_activityStateChangeDispatcher = makeUnique<RunLoopObserver>(static_cast<CFIndex>(RunLoopObserver::WellKnownRunLoopOrders::ActivityStateChange), [this] {
        this->dispatchActivityStateChange();
    });
#endif

#if ENABLE(REMOTE_INSPECTOR)
    m_inspectorDebuggable->setRemoteDebuggingAllowed(true);
    m_inspectorDebuggable->init();
#endif

    createInspectorTargets();
}

WebPageProxy::~WebPageProxy()
{
    RELEASE_LOG_IF_ALLOWED(Loading, "destructor: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    ASSERT(m_process->webPage(m_pageID) != this);
#if !ASSERT_DISABLED
    for (WebPageProxy* page : m_process->pages())
        ASSERT(page != this);
#endif

    setPageLoadStateObserver(nullptr);

    if (!m_isClosed)
        close();

    WebProcessPool::statistics().wkPageCount--;

    if (m_spellDocumentTag)
        TextChecker::closeSpellDocumentWithTag(m_spellDocumentTag.value());

    m_preferences->removePage(*this);
    m_pageGroup->removePage(this);

#ifndef NDEBUG
    webPageProxyCounter.decrement();
#endif
}

// FIXME: Should return a const PageClient& and add a separate non-const
// version of this function, but several PageClient methods will need to become
// const for this to be possible.
PageClient& WebPageProxy::pageClient() const
{
    ASSERT(m_pageClient);
    return *m_pageClient;
}

PAL::SessionID WebPageProxy::sessionID() const
{
    return m_websiteDataStore->sessionID();
}

DrawingAreaProxy* WebPageProxy::provisionalDrawingArea() const
{
    if (m_provisionalPage && m_provisionalPage->drawingArea())
        return m_provisionalPage->drawingArea();
    return drawingArea();
}

const API::PageConfiguration& WebPageProxy::configuration() const
{
    return m_configuration.get();
}

ProcessID WebPageProxy::processIdentifier() const
{
    if (m_isClosed)
        return 0;

    return m_process->processIdentifier();
}

bool WebPageProxy::hasRunningProcess() const
{
    // A page that has been explicitly closed is never valid.
    if (m_isClosed)
        return false;

    return m_hasRunningProcess;
}

void WebPageProxy::notifyProcessPoolToPrewarm()
{
    m_process->processPool().didReachGoodTimeToPrewarm();
}

void WebPageProxy::setPreferences(WebPreferences& preferences)
{
    if (&preferences == m_preferences.ptr())
        return;

    m_preferences->removePage(*this);
    m_preferences = preferences;
    m_preferences->addPage(*this);

    preferencesDidChange();
}
    
void WebPageProxy::setHistoryClient(UniqueRef<API::HistoryClient>&& historyClient)
{
    m_historyClient = WTFMove(historyClient);
}

void WebPageProxy::setNavigationClient(UniqueRef<API::NavigationClient>&& navigationClient)
{
    m_navigationClient = WTFMove(navigationClient);
}

void WebPageProxy::setLoaderClient(std::unique_ptr<API::LoaderClient>&& loaderClient)
{
    m_loaderClient = WTFMove(loaderClient);
}

void WebPageProxy::setPolicyClient(std::unique_ptr<API::PolicyClient>&& policyClient)
{
    m_policyClient = WTFMove(policyClient);
}

void WebPageProxy::setFormClient(std::unique_ptr<API::FormClient>&& formClient)
{
    if (!formClient) {
        m_formClient = makeUnique<API::FormClient>();
        return;
    }

    m_formClient = WTFMove(formClient);
}

void WebPageProxy::setUIClient(std::unique_ptr<API::UIClient>&& uiClient)
{
    if (!uiClient) {
        m_uiClient = makeUnique<API::UIClient>();
        return;
    }

    m_uiClient = WTFMove(uiClient);

    if (hasRunningProcess())
        m_process->send(Messages::WebPage::SetCanRunBeforeUnloadConfirmPanel(m_uiClient->canRunBeforeUnloadConfirmPanel()), m_pageID);

    setCanRunModal(m_uiClient->canRunModal());
    setNeedsFontAttributes(m_uiClient->needsFontAttributes());
}

void WebPageProxy::setIconLoadingClient(std::unique_ptr<API::IconLoadingClient>&& iconLoadingClient)
{
    bool hasClient = iconLoadingClient.get();
    if (!iconLoadingClient)
        m_iconLoadingClient = makeUnique<API::IconLoadingClient>();
    else
        m_iconLoadingClient = WTFMove(iconLoadingClient);

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetUseIconLoadingClient(hasClient), m_pageID);
}

void WebPageProxy::setPageLoadStateObserver(std::unique_ptr<PageLoadState::Observer>&& observer)
{
    if (m_pageLoadStateObserver)
        pageLoadState().removeObserver(*m_pageLoadStateObserver);
    m_pageLoadStateObserver = WTFMove(observer);
    if (m_pageLoadStateObserver)
        pageLoadState().addObserver(*m_pageLoadStateObserver);
}

void WebPageProxy::setFindClient(std::unique_ptr<API::FindClient>&& findClient)
{
    if (!findClient) {
        m_findClient = makeUnique<API::FindClient>();
        return;
    }
    
    m_findClient = WTFMove(findClient);
}

void WebPageProxy::setFindMatchesClient(std::unique_ptr<API::FindMatchesClient>&& findMatchesClient)
{
    if (!findMatchesClient) {
        m_findMatchesClient = makeUnique<API::FindMatchesClient>();
        return;
    }

    m_findMatchesClient = WTFMove(findMatchesClient);
}

void WebPageProxy::setDiagnosticLoggingClient(std::unique_ptr<API::DiagnosticLoggingClient>&& diagnosticLoggingClient)
{
    m_diagnosticLoggingClient = WTFMove(diagnosticLoggingClient);
}

#if ENABLE(CONTEXT_MENUS)
void WebPageProxy::setContextMenuClient(std::unique_ptr<API::ContextMenuClient>&& contextMenuClient)
{
    if (!contextMenuClient) {
        m_contextMenuClient = makeUnique<API::ContextMenuClient>();
        return;
    }

    m_contextMenuClient = WTFMove(contextMenuClient);
}
#endif

void WebPageProxy::setInjectedBundleClient(const WKPageInjectedBundleClientBase* client)
{
    if (!client) {
        m_injectedBundleClient = nullptr;
        return;
    }

    m_injectedBundleClient = makeUnique<WebPageInjectedBundleClient>();
    m_injectedBundleClient->initialize(client);
}

void WebPageProxy::handleMessage(IPC::Connection& connection, const String& messageName, const WebKit::UserData& messageBody)
{
    ASSERT(m_process->connection() == &connection);

    if (!m_injectedBundleClient)
        return;

    m_injectedBundleClient->didReceiveMessageFromInjectedBundle(this, messageName, m_process->transformHandlesToObjects(messageBody.object()).get());
}

void WebPageProxy::handleSynchronousMessage(IPC::Connection& connection, const String& messageName, const UserData& messageBody, CompletionHandler<void(UserData&&)>&& completionHandler)
{
    ASSERT(m_process->connection() == &connection);

    if (!m_injectedBundleClient)
        return completionHandler({ });

    RefPtr<API::Object> returnData;
    m_injectedBundleClient->didReceiveSynchronousMessageFromInjectedBundle(this, messageName, m_process->transformHandlesToObjects(messageBody.object()).get(), [completionHandler = WTFMove(completionHandler), process = m_process.copyRef()] (RefPtr<API::Object>&& returnData) mutable {
        completionHandler(UserData(process->transformObjectsToHandles(returnData.get())));
    });
}

void WebPageProxy::launchProcess(const RegistrableDomain& registrableDomain, ProcessLaunchReason reason)
{
    ASSERT(!m_isClosed);
    ASSERT(!hasRunningProcess());

    RELEASE_LOG_IF_ALLOWED(Loading, "launchProcess: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    m_process->removeWebPage(*this, WebProcessProxy::EndsUsingDataStore::Yes);
    m_process->removeMessageReceiver(Messages::WebPageProxy::messageReceiverName(), m_pageID);

    auto& processPool = m_process->processPool();

    auto* relatedPage = m_configuration->relatedPage();
    if (relatedPage && !relatedPage->isClosed())
        m_process = relatedPage->ensureRunningProcess();
    else
        m_process = processPool.processForRegistrableDomain(m_websiteDataStore.get(), this, registrableDomain);
    m_hasRunningProcess = true;

    m_process->addExistingWebPage(*this, WebProcessProxy::BeginsUsingDataStore::Yes);
    m_process->addMessageReceiver(Messages::WebPageProxy::messageReceiverName(), m_pageID, *this);

    finishAttachingToWebProcess(reason);
}

bool WebPageProxy::suspendCurrentPageIfPossible(API::Navigation& navigation, Optional<FrameIdentifier> mainFrameID, ProcessSwapRequestedByClient processSwapRequestedByClient, ShouldDelayClosingUntilEnteringAcceleratedCompositingMode shouldDelayClosingUntilEnteringAcceleratedCompositingMode)
{
    m_lastSuspendedPage = nullptr;

    if (!mainFrameID)
        return false;

    if (!hasCommittedAnyProvisionalLoads()) {
        RELEASE_LOG_IF_ALLOWED(ProcessSwapping, "suspendCurrentPageIfPossible: Not suspending current page for process pid %i because has not committed any load yet", m_process->processIdentifier());
        return false;
    }

    if (isPageOpenedByDOMShowingInitialEmptyDocument()) {
        RELEASE_LOG_IF_ALLOWED(ProcessSwapping, "suspendCurrentPageIfPossible: Not suspending current page for process pid %i because it is showing the initial empty document", m_process->processIdentifier());
        return false;
    }

    auto* fromItem = navigation.fromItem();

    // If the source and the destination back / forward list items are the same, then this is a client-side redirect. In this case,
    // there is no need to suspend the previous page as there will be no way to get back to it.
    if (fromItem && fromItem == m_backForwardList->currentItem()) {
        RELEASE_LOG_IF_ALLOWED(ProcessSwapping, "suspendCurrentPageIfPossible: Not suspending current page for process pid %i because this is a client-side redirect", m_process->processIdentifier());
        return false;
    }

    if (fromItem && fromItem->url() != pageLoadState().url()) {
        RELEASE_LOG_ERROR_IF_ALLOWED(ProcessSwapping, "suspendCurrentPageIfPossible: Not suspending current page for process pid %i because fromItem's URL does not match the page URL.", m_process->processIdentifier());
        ASSERT_NOT_REACHED();
        return false;
    }

    RELEASE_LOG_IF_ALLOWED(ProcessSwapping, "suspendCurrentPageIfPossible: Suspending current page for process pid %i", m_process->processIdentifier());
    auto suspendedPage = makeUnique<SuspendedPageProxy>(*this, m_process.copyRef(), *mainFrameID, shouldDelayClosingUntilEnteringAcceleratedCompositingMode);

    LOG(ProcessSwapping, "WebPageProxy %" PRIu64 " created suspended page %s for process pid %i, back/forward item %s" PRIu64, pageID().toUInt64(), suspendedPage->loggingString(), m_process->processIdentifier(), fromItem ? fromItem->itemID().logString() : 0);

    // If the client forced a swap then it may not be web-compatible to keep the previous page because other windows may have an opener link to it. We thus close it as soon as we
    // can do so without flashing.
    if (processSwapRequestedByClient == ProcessSwapRequestedByClient::Yes)
        suspendedPage->closeWithoutFlashing();

    if (fromItem && m_preferences->usesPageCache())
        fromItem->setSuspendedPage(suspendedPage.get());

    m_lastSuspendedPage = makeWeakPtr(*suspendedPage);
    m_process->processPool().addSuspendedPage(WTFMove(suspendedPage));
    return true;
}

void WebPageProxy::swapToWebProcess(Ref<WebProcessProxy>&& process, std::unique_ptr<DrawingAreaProxy>&& drawingArea, RefPtr<WebFrameProxy>&& mainFrame)
{
    ASSERT(!m_isClosed);
    RELEASE_LOG_IF_ALLOWED(Loading, "swapToWebProcess: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    m_process = WTFMove(process);
    m_websiteDataStore = m_process->websiteDataStore();

    if (m_logger)
        m_logger->setEnabled(this, isAlwaysOnLoggingAllowed());

    ASSERT(!m_drawingArea);
    setDrawingArea(WTFMove(drawingArea));
    ASSERT(!m_mainFrame);
    m_mainFrame = WTFMove(mainFrame);
    m_hasRunningProcess = true;

    m_process->addExistingWebPage(*this, WebProcessProxy::BeginsUsingDataStore::No);
    m_process->addMessageReceiver(Messages::WebPageProxy::messageReceiverName(), m_pageID, *this);

    finishAttachingToWebProcess(ProcessLaunchReason::ProcessSwap);
}

void WebPageProxy::finishAttachingToWebProcess(ProcessLaunchReason reason)
{
    ASSERT(m_process->state() != AuxiliaryProcessProxy::State::Terminated);

    updateActivityState();
    updateThrottleState();

    didAttachToRunningProcess();

    // In the process-swap case, the ProvisionalPageProxy already took care of initializing the WebPage in the WebProcess.
    if (reason != ProcessLaunchReason::ProcessSwap)
        initializeWebPage();

    m_inspector->updateForNewPageProcess(this);

#if ENABLE(REMOTE_INSPECTOR)
    remoteInspectorInformationDidChange();
#endif

    clearInspectorTargets();
    createInspectorTargets();

    pageClient().didRelaunchProcess();
    m_pageLoadState.didSwapWebProcesses();
    if (reason != ProcessLaunchReason::InitialProcess)
        m_drawingArea->waitForBackingStoreUpdateOnNextPaint();
}

void WebPageProxy::didAttachToRunningProcess()
{
    ASSERT(hasRunningProcess());

#if ENABLE(FULLSCREEN_API)
    ASSERT(!m_fullScreenManager);
    m_fullScreenManager = makeUnique<WebFullScreenManagerProxy>(*this, pageClient().fullScreenManagerProxyClient());
#endif
#if PLATFORM(IOS_FAMILY) && HAVE(AVKIT) || (PLATFORM(MAC) && ENABLE(VIDEO_PRESENTATION_MODE))
    ASSERT(!m_playbackSessionManager);
    m_playbackSessionManager = PlaybackSessionManagerProxy::create(*this);
    ASSERT(!m_videoFullscreenManager);
    m_videoFullscreenManager = VideoFullscreenManagerProxy::create(*this, *m_playbackSessionManager);
#endif

#if ENABLE(APPLE_PAY)
    ASSERT(!m_paymentCoordinator);
    m_paymentCoordinator = makeUnique<WebPaymentCoordinatorProxy>(*this);
#endif

#if USE(SYSTEM_PREVIEW)
    ASSERT(!m_systemPreviewController);
    m_systemPreviewController = makeUnique<SystemPreviewController>(*this);
#endif

#if ENABLE(WEB_AUTHN)
    ASSERT(!m_credentialsMessenger);
    m_credentialsMessenger = makeUnique<WebAuthenticatorCoordinatorProxy>(*this);
#endif

#if HAVE(PENCILKIT)
    ASSERT(!m_editableImageController);
    m_editableImageController = makeUnique<EditableImageController>(*this);
#endif
}

RefPtr<API::Navigation> WebPageProxy::launchProcessForReload()
{
    RELEASE_LOG_IF_ALLOWED(Loading, "launchProcessForReload: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (m_isClosed) {
        RELEASE_LOG_IF_ALLOWED(Loading, "launchProcessForReload: page is closed: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return nullptr;
    }
    
    ASSERT(!hasRunningProcess());
    auto registrableDomain = m_backForwardList->currentItem() ? RegistrableDomain { URL(URL(), m_backForwardList->currentItem()->url()) } : RegistrableDomain { };
    launchProcess(registrableDomain, ProcessLaunchReason::Crash);

    if (!m_backForwardList->currentItem()) {
        RELEASE_LOG_IF_ALLOWED(Loading, "launchProcessForReload: no current item to reload: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return nullptr;
    }

    auto navigation = m_navigationState->createReloadNavigation();

    String url = currentURL();
    if (!url.isEmpty()) {
        auto transaction = m_pageLoadState.transaction();
        m_pageLoadState.setPendingAPIRequest(transaction, { navigation->navigationID(), url });
    }

    // We allow stale content when reloading a WebProcess that's been killed or crashed.
    m_process->send(Messages::WebPage::GoToBackForwardItem(navigation->navigationID(), m_backForwardList->currentItem()->itemID(), FrameLoadType::IndexedBackForward, ShouldTreatAsContinuingLoad::No, WTF::nullopt), m_pageID);
    m_process->responsivenessTimer().start();

    return navigation;
}

RefPtr<API::Navigation> WebPageProxy::launchProcessWithItem(WebBackForwardListItem& item)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "launchProcessWithItem: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (m_isClosed) {
        RELEASE_LOG_IF_ALLOWED(Loading, "launchProcessWithItem: page is closed: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return nullptr;
    }

    ASSERT(!hasRunningProcess());
    launchProcess(RegistrableDomain { URL(URL(), item.url()) }, ProcessLaunchReason::InitialProcess);

    if (&item != m_backForwardList->currentItem())
        m_backForwardList->goToItem(item);

    auto navigation = m_navigationState->createBackForwardNavigation(item, m_backForwardList->currentItem(), FrameLoadType::IndexedBackForward);

    m_process->send(Messages::WebPage::GoToBackForwardItem(navigation->navigationID(), item.itemID(), FrameLoadType::IndexedBackForward, ShouldTreatAsContinuingLoad::No, WTF::nullopt), m_pageID);
    m_process->responsivenessTimer().start();

    return navigation;
}

void WebPageProxy::setDrawingArea(std::unique_ptr<DrawingAreaProxy>&& drawingArea)
{
    m_drawingArea = WTFMove(drawingArea);
    if (!m_drawingArea)
        return;

#if ENABLE(ASYNC_SCROLLING) && PLATFORM(COCOA)
    if (m_drawingArea->type() == DrawingAreaTypeRemoteLayerTree) {
        m_scrollingCoordinatorProxy = makeUnique<RemoteScrollingCoordinatorProxy>(*this);
#if PLATFORM(IOS_FAMILY)
        // On iOS, main frame scrolls are sent in terms of visible rect updates.
        m_scrollingCoordinatorProxy->setPropagatesMainFrameScrolls(false);
#endif
    }
#endif
}

void WebPageProxy::initializeWebPage()
{
    if (!hasRunningProcess())
        return;

    setDrawingArea(pageClient().createDrawingAreaProxy(m_process));
    ASSERT(m_drawingArea);

    process().send(Messages::WebProcess::CreateWebPage(m_pageID, creationParameters(m_process, *m_drawingArea)), 0);

    m_process->addVisitedLinkStoreUser(visitedLinkStore(), m_pageID);
}

void WebPageProxy::close()
{
    if (m_isClosed)
        return;

    RELEASE_LOG_IF_ALLOWED(Loading, "close: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    m_isClosed = true;

    reportPageLoadResult(ResourceError { ResourceError::Type::Cancellation });

    if (m_activePopupMenu)
        m_activePopupMenu->cancelTracking();

    if (m_controlledByAutomation) {
        if (auto* automationSession = process().processPool().automationSession())
            automationSession->willClosePage(*this);
    }

#if ENABLE(CONTEXT_MENUS)
    m_activeContextMenu = nullptr;
#endif

    m_provisionalPage = nullptr;

    m_inspector->invalidate();

    m_backForwardList->pageClosed();
    m_inspectorController->pageClosed();
#if ENABLE(REMOTE_INSPECTOR)
    m_inspectorDebuggable = nullptr;
#endif
    pageClient().pageClosed();

    m_process->disconnectFramesFromPage(this);

    resetState(ResetStateReason::PageInvalidated);

    m_loaderClient = nullptr;
    m_navigationClient = makeUniqueRef<API::NavigationClient>();
    m_policyClient = nullptr;
    m_iconLoadingClient = makeUnique<API::IconLoadingClient>();
    m_formClient = makeUnique<API::FormClient>();
    m_uiClient = makeUnique<API::UIClient>();
    m_findClient = makeUnique<API::FindClient>();
    m_findMatchesClient = makeUnique<API::FindMatchesClient>();
    m_diagnosticLoggingClient = nullptr;
#if ENABLE(CONTEXT_MENUS)
    m_contextMenuClient = makeUnique<API::ContextMenuClient>();
#endif
#if ENABLE(FULLSCREEN_API)
    m_fullscreenClient = makeUnique<API::FullscreenClient>();
#endif

    m_process->processPool().removeAllSuspendedPagesForPage(*this);

    m_process->send(Messages::WebPage::Close(), m_pageID);
    m_process->removeWebPage(*this, WebProcessProxy::EndsUsingDataStore::Yes);
    m_process->removeMessageReceiver(Messages::WebPageProxy::messageReceiverName(), m_pageID);
    m_process->processPool().supplement<WebNotificationManagerProxy>()->clearNotifications(this);

    // Null out related WebPageProxy to avoid leaks.
    m_configuration->setRelatedPage(nullptr);

#if PLATFORM(IOS_FAMILY)
    // Make sure we don't hold a process assertion after getting closed.
    m_activityToken = nullptr;
#endif

    stopAllURLSchemeTasks();
    updatePlayingMediaDidChange(MediaProducer::IsNotPlaying);
}

bool WebPageProxy::tryClose()
{
    if (!hasRunningProcess())
        return true;

    RELEASE_LOG_IF_ALLOWED(Loading, "tryClose: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    // Close without delay if the process allows it. Our goal is to terminate
    // the process, so we check a per-process status bit.
    if (m_process->isSuddenTerminationEnabled())
        return true;

    m_process->send(Messages::WebPage::TryClose(), m_pageID);
    m_process->responsivenessTimer().start();
    return false;
}

void WebPageProxy::maybeInitializeSandboxExtensionHandle(WebProcessProxy& process, const URL& url, const URL& resourceDirectoryURL, SandboxExtension::Handle& sandboxExtensionHandle)
{
    if (!url.isLocalFile())
        return;

    if (!resourceDirectoryURL.isEmpty()) {
        if (process.hasAssumedReadAccessToURL(resourceDirectoryURL))
            return;

        if (SandboxExtension::createHandle(resourceDirectoryURL.fileSystemPath(), SandboxExtension::Type::ReadOnly, sandboxExtensionHandle)) {
            m_process->assumeReadAccessToBaseURL(*this, resourceDirectoryURL);
            return;
        }
    }

    if (process.hasAssumedReadAccessToURL(url))
        return;

    // Inspector resources are in a directory with assumed access.
    ASSERT_WITH_SECURITY_IMPLICATION(!WebKit::isInspectorPage(*this));

#if HAVE(SANDBOX_ISSUE_READ_EXTENSION_TO_PROCESS_BY_PID)
    if (SandboxExtension::createHandleForReadByPid("/", process.processIdentifier(), sandboxExtensionHandle)) {
        willAcquireUniversalFileReadSandboxExtension(process);
        return;
    }
#endif
    if (SandboxExtension::createHandle("/", SandboxExtension::Type::ReadOnly, sandboxExtensionHandle)) {
        willAcquireUniversalFileReadSandboxExtension(process);
        return;
    }

#if PLATFORM(COCOA)
    if (!linkedOnOrAfter(SDKVersion::FirstWithoutUnconditionalUniversalSandboxExtension))
        willAcquireUniversalFileReadSandboxExtension(process);
#endif

    // We failed to issue an universal file read access sandbox, fall back to issuing one for the base URL instead.
    auto baseURL = URL(URL(), url.baseAsString());
    auto basePath = baseURL.fileSystemPath();
    if (!basePath.isNull() && SandboxExtension::createHandle(basePath, SandboxExtension::Type::ReadOnly, sandboxExtensionHandle))
        m_process->assumeReadAccessToBaseURL(*this, baseURL);
}

#if !PLATFORM(COCOA)
void WebPageProxy::addPlatformLoadParameters(LoadParameters&)
{
}
#endif

WebProcessProxy& WebPageProxy::ensureRunningProcess()
{
    if (!hasRunningProcess())
        launchProcess({ }, ProcessLaunchReason::InitialProcess);

    return m_process;
}

RefPtr<API::Navigation> WebPageProxy::loadRequest(ResourceRequest&& request, ShouldOpenExternalURLsPolicy shouldOpenExternalURLsPolicy, API::Object* userData)
{
    if (m_isClosed)
        return nullptr;

    RELEASE_LOG_IF_ALLOWED(Loading, "loadRequest: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (!hasRunningProcess())
        launchProcess(RegistrableDomain { request.url() }, ProcessLaunchReason::InitialProcess);

    auto navigation = m_navigationState->createLoadRequestNavigation(ResourceRequest(request), m_backForwardList->currentItem());
    loadRequestWithNavigationShared(m_process.copyRef(), navigation.get(), WTFMove(request), shouldOpenExternalURLsPolicy, userData, ShouldTreatAsContinuingLoad::No);
    return navigation;
}

void WebPageProxy::loadRequestWithNavigationShared(Ref<WebProcessProxy>&& process, API::Navigation& navigation, ResourceRequest&& request, ShouldOpenExternalURLsPolicy shouldOpenExternalURLsPolicy, API::Object* userData, ShouldTreatAsContinuingLoad shouldTreatAsContinuingLoad, Optional<WebsitePoliciesData>&& websitePolicies)
{
    ASSERT(!m_isClosed);

    RELEASE_LOG_IF_ALLOWED(Loading, "loadRequestWithNavigation: webPID = %i, pageID = %" PRIu64, process->processIdentifier(), m_pageID.toUInt64());

    auto transaction = m_pageLoadState.transaction();

    auto url = request.url();
    if (shouldTreatAsContinuingLoad != ShouldTreatAsContinuingLoad::Yes)
        m_pageLoadState.setPendingAPIRequest(transaction, { navigation.navigationID(), url });

    LoadParameters loadParameters;
    loadParameters.navigationID = navigation.navigationID();
    loadParameters.request = WTFMove(request);
    loadParameters.shouldOpenExternalURLsPolicy = shouldOpenExternalURLsPolicy;
    loadParameters.userData = UserData(process->transformObjectsToHandles(userData).get());
    loadParameters.shouldTreatAsContinuingLoad = shouldTreatAsContinuingLoad == ShouldTreatAsContinuingLoad::Yes;
    loadParameters.websitePolicies = WTFMove(websitePolicies);
    loadParameters.lockHistory = navigation.lockHistory();
    loadParameters.lockBackForwardList = navigation.lockBackForwardList();
    loadParameters.clientRedirectSourceForHistory = navigation.clientRedirectSourceForHistory();
    maybeInitializeSandboxExtensionHandle(process, url, m_pageLoadState.resourceDirectoryURL(), loadParameters.sandboxExtensionHandle);

    addPlatformLoadParameters(loadParameters);

    process->send(Messages::WebPage::LoadRequest(loadParameters), m_pageID);
    process->responsivenessTimer().start();
}

RefPtr<API::Navigation> WebPageProxy::loadFile(const String& fileURLString, const String& resourceDirectoryURLString, API::Object* userData)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "loadFile: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (m_isClosed) {
        RELEASE_LOG_IF_ALLOWED(Loading, "loadFile: page is closed: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return nullptr;
    }

    if (!hasRunningProcess())
        launchProcess({ }, ProcessLaunchReason::InitialProcess);

    URL fileURL = URL(URL(), fileURLString);
    if (!fileURL.isLocalFile()) {
        RELEASE_LOG_IF_ALLOWED(Loading, "loadFile: file is not local: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return nullptr;
    }

    URL resourceDirectoryURL;
    if (resourceDirectoryURLString.isNull())
        resourceDirectoryURL = URL({ }, "file:///"_s);
    else {
        resourceDirectoryURL = URL(URL(), resourceDirectoryURLString);
        if (!resourceDirectoryURL.isLocalFile()) {
            RELEASE_LOG_IF_ALLOWED(Loading, "loadFile: resource URL is not local: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
            return nullptr;
        }
    }

    auto navigation = m_navigationState->createLoadRequestNavigation(ResourceRequest(fileURL), m_backForwardList->currentItem());

    auto transaction = m_pageLoadState.transaction();

    m_pageLoadState.setPendingAPIRequest(transaction, { navigation->navigationID(), fileURLString }, resourceDirectoryURL);

    String resourceDirectoryPath = resourceDirectoryURL.fileSystemPath();

    LoadParameters loadParameters;
    loadParameters.navigationID = navigation->navigationID();
    loadParameters.request = fileURL;
    loadParameters.shouldOpenExternalURLsPolicy = ShouldOpenExternalURLsPolicy::ShouldNotAllow;
    loadParameters.userData = UserData(process().transformObjectsToHandles(userData).get());
#if HAVE(SANDBOX_ISSUE_READ_EXTENSION_TO_PROCESS_BY_PID)
    if (!SandboxExtension::createHandleForReadByPid(resourceDirectoryPath, processIdentifier(), loadParameters.sandboxExtensionHandle))
#endif
    SandboxExtension::createHandle(resourceDirectoryPath, SandboxExtension::Type::ReadOnly, loadParameters.sandboxExtensionHandle);
    addPlatformLoadParameters(loadParameters);

    m_process->assumeReadAccessToBaseURL(*this, resourceDirectoryURL);
    m_process->send(Messages::WebPage::LoadRequest(loadParameters), m_pageID);
    m_process->responsivenessTimer().start();

    return navigation;
}

RefPtr<API::Navigation> WebPageProxy::loadData(const IPC::DataReference& data, const String& MIMEType, const String& encoding, const String& baseURL, API::Object* userData, ShouldOpenExternalURLsPolicy shouldOpenExternalURLsPolicy)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "loadData: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (m_isClosed) {
        RELEASE_LOG_IF_ALLOWED(Loading, "loadData: page is closed: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return nullptr;
    }

    if (!hasRunningProcess())
        launchProcess({ }, ProcessLaunchReason::InitialProcess);

    auto navigation = m_navigationState->createLoadDataNavigation(makeUnique<API::SubstituteData>(data.vector(), MIMEType, encoding, baseURL, userData));
    loadDataWithNavigationShared(m_process.copyRef(), navigation, data, MIMEType, encoding, baseURL, userData, ShouldTreatAsContinuingLoad::No, WTF::nullopt, shouldOpenExternalURLsPolicy);
    return navigation;
}

void WebPageProxy::loadDataWithNavigationShared(Ref<WebProcessProxy>&& process, API::Navigation& navigation, const IPC::DataReference& data, const String& MIMEType, const String& encoding, const String& baseURL, API::Object* userData, ShouldTreatAsContinuingLoad shouldTreatAsContinuingLoad, Optional<WebsitePoliciesData>&& websitePolicies, ShouldOpenExternalURLsPolicy shouldOpenExternalURLsPolicy)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "loadDataWithNavigation: webPID = %i, pageID = %" PRIu64, process->processIdentifier(), m_pageID.toUInt64());

    ASSERT(!m_isClosed);

    auto transaction = m_pageLoadState.transaction();

    m_pageLoadState.setPendingAPIRequest(transaction, { navigation.navigationID(), !baseURL.isEmpty() ? baseURL : WTF::blankURL().string() });

    LoadParameters loadParameters;
    loadParameters.navigationID = navigation.navigationID();
    loadParameters.data = data;
    loadParameters.MIMEType = MIMEType;
    loadParameters.encodingName = encoding;
    loadParameters.baseURLString = baseURL;
    loadParameters.shouldTreatAsContinuingLoad = shouldTreatAsContinuingLoad == ShouldTreatAsContinuingLoad::Yes;
    loadParameters.userData = UserData(process->transformObjectsToHandles(userData).get());
    loadParameters.websitePolicies = WTFMove(websitePolicies);
    loadParameters.shouldOpenExternalURLsPolicy = shouldOpenExternalURLsPolicy;
    addPlatformLoadParameters(loadParameters);

    process->assumeReadAccessToBaseURL(*this, baseURL);
    process->send(Messages::WebPage::LoadData(loadParameters), m_pageID);
    process->responsivenessTimer().start();
}

void WebPageProxy::loadAlternateHTML(const IPC::DataReference& htmlData, const String& encoding, const URL& baseURL, const URL& unreachableURL, API::Object* userData)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "loadAlternateHTML: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    // When the UIProcess is in the process of handling a failing provisional load, do not attempt to
    // start a second alternative HTML load as this will prevent the page load state from being
    // handled properly.
    if (m_isClosed || m_isLoadingAlternateHTMLStringForFailingProvisionalLoad) {
        RELEASE_LOG_IF_ALLOWED(Loading, "loadAlternateHTML: page is closed (or other): webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return;
    }

    if (!m_failingProvisionalLoadURL.isEmpty())
        m_isLoadingAlternateHTMLStringForFailingProvisionalLoad = true;

    if (!hasRunningProcess())
        launchProcess(RegistrableDomain { baseURL }, ProcessLaunchReason::InitialProcess);

    auto transaction = m_pageLoadState.transaction();

    m_pageLoadState.setPendingAPIRequest(transaction, { 0, unreachableURL });
    m_pageLoadState.setUnreachableURL(transaction, unreachableURL);

    if (m_mainFrame)
        m_mainFrame->setUnreachableURL(unreachableURL);

    LoadParameters loadParameters;
    loadParameters.navigationID = 0;
    loadParameters.data = htmlData;
    loadParameters.MIMEType = "text/html"_s;
    loadParameters.encodingName = encoding;
    loadParameters.baseURLString = baseURL;
    loadParameters.unreachableURLString = unreachableURL;
    loadParameters.provisionalLoadErrorURLString = m_failingProvisionalLoadURL;
    loadParameters.userData = UserData(process().transformObjectsToHandles(userData).get());
    addPlatformLoadParameters(loadParameters);

    m_process->assumeReadAccessToBaseURL(*this, baseURL);
    m_process->assumeReadAccessToBaseURL(*this, unreachableURL);
    m_process->send(Messages::WebPage::LoadAlternateHTML(loadParameters), m_pageID);
    m_process->responsivenessTimer().start();
}

void WebPageProxy::loadWebArchiveData(API::Data* webArchiveData, API::Object* userData)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "loadWebArchiveData: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (m_isClosed) {
        RELEASE_LOG_IF_ALLOWED(Loading, "loadWebArchiveData: page is closed: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return;
    }

    if (!hasRunningProcess())
        launchProcess({ }, ProcessLaunchReason::InitialProcess);

    auto transaction = m_pageLoadState.transaction();
    m_pageLoadState.setPendingAPIRequest(transaction, { 0, WTF::blankURL().string() });

    LoadParameters loadParameters;
    loadParameters.navigationID = 0;
    loadParameters.data = webArchiveData->dataReference();
    loadParameters.MIMEType = "application/x-webarchive"_s;
    loadParameters.encodingName = "utf-16"_s;
    loadParameters.userData = UserData(process().transformObjectsToHandles(userData).get());
    addPlatformLoadParameters(loadParameters);

    m_process->send(Messages::WebPage::LoadData(loadParameters), m_pageID);
    m_process->responsivenessTimer().start();
}

void WebPageProxy::navigateToPDFLinkWithSimulatedClick(const String& urlString, IntPoint documentPoint, IntPoint screenPoint)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "navigateToPDFLinkWithSimulatedClick: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (m_isClosed) {
        RELEASE_LOG_IF_ALLOWED(Loading, "navigateToPDFLinkWithSimulatedClick: page is closed: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return;
    }

    if (WTF::protocolIsJavaScript(urlString))
        return;

    if (!hasRunningProcess())
        launchProcess(RegistrableDomain { URL(URL(), urlString) }, ProcessLaunchReason::InitialProcess);

    m_process->send(Messages::WebPage::NavigateToPDFLinkWithSimulatedClick(urlString, documentPoint, screenPoint), m_pageID);
    m_process->responsivenessTimer().start();
}

void WebPageProxy::stopLoading()
{
    RELEASE_LOG_IF_ALLOWED(Loading, "stopLoading: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (!hasRunningProcess()) {
        RELEASE_LOG_IF_ALLOWED(Loading, "navigateToPDFLinkWithSimulatedClick: page is not valid: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return;
    }

    m_process->send(Messages::WebPage::StopLoading(), m_pageID);
    if (m_provisionalPage) {
        m_provisionalPage->cancel();
        m_provisionalPage = nullptr;
    }
    m_process->responsivenessTimer().start();
}

RefPtr<API::Navigation> WebPageProxy::reload(OptionSet<WebCore::ReloadOption> options)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "reload: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    SandboxExtension::Handle sandboxExtensionHandle;

    String url = currentURL();
    if (!url.isEmpty()) {
        // We may not have an extension yet if back/forward list was reinstated after a WebProcess crash or a browser relaunch
        maybeInitializeSandboxExtensionHandle(m_process, URL(URL(), url), currentResourceDirectoryURL(), sandboxExtensionHandle);
    }

    if (!hasRunningProcess())
        return launchProcessForReload();
    
    auto navigation = m_navigationState->createReloadNavigation();

    if (!url.isEmpty()) {
        auto transaction = m_pageLoadState.transaction();
        m_pageLoadState.setPendingAPIRequest(transaction, { navigation->navigationID(), url });
    }

    // Store decision to reload without content blockers on the navigation so that we can later set the corresponding
    // WebsitePolicies flag in WebPageProxy::receivedNavigationPolicyDecision().
    if (options.contains(WebCore::ReloadOption::DisableContentBlockers))
        navigation->setUserContentExtensionsEnabled(false);

    m_process->send(Messages::WebPage::Reload(navigation->navigationID(), options.toRaw(), sandboxExtensionHandle), m_pageID);
    m_process->responsivenessTimer().start();

#if ENABLE(SPEECH_SYNTHESIS)
    resetSpeechSynthesizer();
#endif

    return navigation;
}

void WebPageProxy::recordAutomaticNavigationSnapshot()
{
    if (m_shouldSuppressNextAutomaticNavigationSnapshot)
        return;

    if (WebBackForwardListItem* item = m_backForwardList->currentItem())
        recordNavigationSnapshot(*item);
}

void WebPageProxy::recordNavigationSnapshot(WebBackForwardListItem& item)
{
    if (!m_shouldRecordNavigationSnapshots)
        return;

#if PLATFORM(COCOA) || PLATFORM(GTK)
    ViewSnapshotStore::singleton().recordSnapshot(*this, item);
#else
    UNUSED_PARAM(item);
#endif
}

RefPtr<API::Navigation> WebPageProxy::goForward()
{
    WebBackForwardListItem* forwardItem = m_backForwardList->forwardItem();
    if (!forwardItem)
        return nullptr;

    return goToBackForwardItem(*forwardItem, FrameLoadType::Forward);
}

RefPtr<API::Navigation> WebPageProxy::goBack()
{
    WebBackForwardListItem* backItem = m_backForwardList->backItem();
    if (!backItem)
        return nullptr;

    return goToBackForwardItem(*backItem, FrameLoadType::Back);
}

RefPtr<API::Navigation> WebPageProxy::goToBackForwardItem(WebBackForwardListItem& item)
{
    return goToBackForwardItem(item, FrameLoadType::IndexedBackForward);
}

RefPtr<API::Navigation> WebPageProxy::goToBackForwardItem(WebBackForwardListItem& item, FrameLoadType frameLoadType)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "goToBackForwardItem: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
    LOG(Loading, "WebPageProxy %p goToBackForwardItem to item URL %s", this, item.url().utf8().data());

    if (!hasRunningProcess())
        return launchProcessWithItem(item);

    RefPtr<API::Navigation> navigation;
    if (!m_backForwardList->currentItem()->itemIsInSameDocument(item))
        navigation = m_navigationState->createBackForwardNavigation(item, m_backForwardList->currentItem(), frameLoadType);

    auto transaction = m_pageLoadState.transaction();
    m_pageLoadState.setPendingAPIRequest(transaction, { navigation ? navigation->navigationID() : 0, item.url() });

    m_process->send(Messages::WebPage::GoToBackForwardItem(navigation ? navigation->navigationID() : 0, item.itemID(), frameLoadType, ShouldTreatAsContinuingLoad::No, WTF::nullopt), m_pageID);
    m_process->responsivenessTimer().start();

    return navigation;
}

void WebPageProxy::tryRestoreScrollPosition()
{
    RELEASE_LOG_IF_ALLOWED(Loading, "tryRestoreScrollPosition: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (!hasRunningProcess()) {
        RELEASE_LOG_IF_ALLOWED(Loading, "tryRestoreScrollPosition: page is not valid: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());
        return;
    }

    m_process->send(Messages::WebPage::TryRestoreScrollPosition(), m_pageID);
}

void WebPageProxy::didChangeBackForwardList(WebBackForwardListItem* added, Vector<Ref<WebBackForwardListItem>>&& removed)
{
    PageClientProtector protector(pageClient());

    if (!m_navigationClient->didChangeBackForwardList(*this, added, removed) && m_loaderClient)
        m_loaderClient->didChangeBackForwardList(*this, added, WTFMove(removed));

    auto transaction = m_pageLoadState.transaction();

    m_pageLoadState.setCanGoBack(transaction, m_backForwardList->backItem());
    m_pageLoadState.setCanGoForward(transaction, m_backForwardList->forwardItem());
}

void WebPageProxy::willGoToBackForwardListItem(const BackForwardItemIdentifier& itemID, bool inPageCache)
{
    PageClientProtector protector(pageClient());

    if (auto* item = m_backForwardList->itemForID(itemID))
        m_navigationClient->willGoToBackForwardListItem(*this, *item, inPageCache);
}

bool WebPageProxy::shouldKeepCurrentBackForwardListItemInList(WebBackForwardListItem& item)
{
    PageClientProtector protector(pageClient());

    return !m_loaderClient || m_loaderClient->shouldKeepCurrentBackForwardListItemInList(*this, item);
}

bool WebPageProxy::canShowMIMEType(const String& mimeType)
{
    if (MIMETypeRegistry::canShowMIMEType(mimeType))
        return true;

#if ENABLE(NETSCAPE_PLUGIN_API)
    String newMimeType = mimeType;
    PluginModuleInfo plugin = m_process->processPool().pluginInfoStore().findPlugin(newMimeType, URL());
    if (!plugin.path.isNull() && m_preferences->pluginsEnabled())
        return true;
#endif // ENABLE(NETSCAPE_PLUGIN_API)

#if PLATFORM(COCOA)
    // On Mac, we can show PDFs.
    if (MIMETypeRegistry::isPDFOrPostScriptMIMEType(mimeType) && !WebProcessPool::omitPDFSupport())
        return true;
#endif // PLATFORM(COCOA)

    return false;
}

void WebPageProxy::setControlledByAutomation(bool controlled)
{
    if (m_controlledByAutomation == controlled)
        return;

    m_controlledByAutomation = controlled;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetControlledByAutomation(controlled), m_pageID);
    m_process->processPool().sendToNetworkingProcess(Messages::NetworkProcess::SetSessionIsControlledByAutomation(m_websiteDataStore->sessionID(), m_controlledByAutomation));
}

void WebPageProxy::createInspectorTarget(const String& targetId, Inspector::InspectorTargetType type)
{
    m_inspectorController->createInspectorTarget(targetId, type);
}

void WebPageProxy::destroyInspectorTarget(const String& targetId)
{
    m_inspectorController->destroyInspectorTarget(targetId);
}

void WebPageProxy::sendMessageToInspectorFrontend(const String& targetId, const String& message)
{
    m_inspectorController->sendMessageToInspectorFrontend(targetId, message);
}

#if ENABLE(REMOTE_INSPECTOR)
void WebPageProxy::setIndicating(bool indicating)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetIndicating(indicating), m_pageID);
}

bool WebPageProxy::allowsRemoteInspection() const
{
    return m_inspectorDebuggable->remoteDebuggingAllowed();
}

void WebPageProxy::setAllowsRemoteInspection(bool allow)
{
    m_inspectorDebuggable->setRemoteDebuggingAllowed(allow);
}

String WebPageProxy::remoteInspectionNameOverride() const
{
    return m_inspectorDebuggable->nameOverride();
}

void WebPageProxy::setRemoteInspectionNameOverride(const String& name)
{
    m_inspectorDebuggable->setNameOverride(name);
}

void WebPageProxy::remoteInspectorInformationDidChange()
{
    m_inspectorDebuggable->update();
}
#endif

void WebPageProxy::clearInspectorTargets()
{
    m_inspectorController->clearTargets();
}

void WebPageProxy::createInspectorTargets()
{
    String pageTargetId = makeString("page-", m_pageID.toUInt64());
    m_inspectorController->createInspectorTarget(pageTargetId, Inspector::InspectorTargetType::Page);
}

void WebPageProxy::setBackgroundColor(const Optional<Color>& color)
{
    if (m_backgroundColor == color)
        return;

    m_backgroundColor = color;
    if (hasRunningProcess())
        m_process->send(Messages::WebPage::SetBackgroundColor(color), m_pageID);
}

void WebPageProxy::setTopContentInset(float contentInset)
{
    if (m_topContentInset == contentInset)
        return;

    m_topContentInset = contentInset;

    if (!hasRunningProcess())
        return;
#if PLATFORM(COCOA)
    MachSendRight fence = m_drawingArea->createFence();

    auto fenceAttachment = IPC::Attachment(fence.leakSendRight(), MACH_MSG_TYPE_MOVE_SEND);
    m_process->send(Messages::WebPage::SetTopContentInsetFenced(contentInset, fenceAttachment), m_pageID);
#else
    m_process->send(Messages::WebPage::SetTopContentInset(contentInset), m_pageID);
#endif
}

void WebPageProxy::setUnderlayColor(const Color& color)
{
    if (m_underlayColor == color)
        return;

    m_underlayColor = color;

    if (hasRunningProcess())
        m_process->send(Messages::WebPage::SetUnderlayColor(color), m_pageID);
}

void WebPageProxy::viewWillStartLiveResize()
{
    if (!hasRunningProcess())
        return;

    closeOverlayedViews();
    m_process->send(Messages::WebPage::ViewWillStartLiveResize(), m_pageID);
}

void WebPageProxy::viewWillEndLiveResize()
{
    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::ViewWillEndLiveResize(), m_pageID);
}

void WebPageProxy::setViewNeedsDisplay(const Region& region)
{
    pageClient().setViewNeedsDisplay(region);
}

void WebPageProxy::requestScroll(const FloatPoint& scrollPosition, const IntPoint& scrollOrigin)
{
    pageClient().requestScroll(scrollPosition, scrollOrigin);
}

WebCore::FloatPoint WebPageProxy::viewScrollPosition() const
{
    return pageClient().viewScrollPosition();
}

void WebPageProxy::setSuppressVisibilityUpdates(bool flag)
{
    if (m_suppressVisibilityUpdates == flag)
        return;
    m_suppressVisibilityUpdates = flag;

    if (!m_suppressVisibilityUpdates) {
#if PLATFORM(COCOA)
        m_activityStateChangeDispatcher->schedule();
#else
        dispatchActivityStateChange();
#endif
    }
}

void WebPageProxy::updateActivityState(OptionSet<ActivityState::Flag> flagsToUpdate)
{
    m_activityState.remove(flagsToUpdate);
    if (flagsToUpdate & ActivityState::IsFocused && pageClient().isViewFocused())
        m_activityState.add(ActivityState::IsFocused);
    if (flagsToUpdate & ActivityState::WindowIsActive && pageClient().isViewWindowActive())
        m_activityState.add(ActivityState::WindowIsActive);
    if (flagsToUpdate & ActivityState::IsVisible && pageClient().isViewVisible())
        m_activityState.add(ActivityState::IsVisible);
    if (flagsToUpdate & ActivityState::IsVisibleOrOccluded && pageClient().isViewVisibleOrOccluded())
        m_activityState.add(ActivityState::IsVisibleOrOccluded);
    if (flagsToUpdate & ActivityState::IsInWindow && pageClient().isViewInWindow())
        m_activityState.add(ActivityState::IsInWindow);
    if (flagsToUpdate & ActivityState::IsVisuallyIdle && pageClient().isVisuallyIdle())
        m_activityState.add(ActivityState::IsVisuallyIdle);
    if (flagsToUpdate & ActivityState::IsAudible && m_mediaState & MediaProducer::IsPlayingAudio && !(m_mutedState & MediaProducer::AudioIsMuted))
        m_activityState.add(ActivityState::IsAudible);
    if (flagsToUpdate & ActivityState::IsLoading && m_pageLoadState.isLoading())
        m_activityState.add(ActivityState::IsLoading);
    if (flagsToUpdate & ActivityState::IsCapturingMedia && m_mediaState & (MediaProducer::HasActiveAudioCaptureDevice | MediaProducer::HasActiveVideoCaptureDevice))
        m_activityState.add(ActivityState::IsCapturingMedia);
}

void WebPageProxy::activityStateDidChange(OptionSet<ActivityState::Flag> mayHaveChanged, bool wantsSynchronousReply, ActivityStateChangeDispatchMode dispatchMode)
{
    LOG_WITH_STREAM(ActivityState, stream << "WebPageProxy " << pageID() << " activityStateDidChange - mayHaveChanged " << mayHaveChanged);

    m_potentiallyChangedActivityStateFlags.add(mayHaveChanged);
    m_activityStateChangeWantsSynchronousReply = m_activityStateChangeWantsSynchronousReply || wantsSynchronousReply;

    if (m_suppressVisibilityUpdates && dispatchMode != ActivityStateChangeDispatchMode::Immediate)
        return;

#if PLATFORM(COCOA)
    bool isNewlyInWindow = !isInWindow() && (mayHaveChanged & ActivityState::IsInWindow) && pageClient().isViewInWindow();
    if (dispatchMode == ActivityStateChangeDispatchMode::Immediate || isNewlyInWindow) {
        dispatchActivityStateChange();
        return;
    }
    m_activityStateChangeDispatcher->schedule();
#else
    UNUSED_PARAM(dispatchMode);
    dispatchActivityStateChange();
#endif
}

void WebPageProxy::viewDidLeaveWindow()
{
    closeOverlayedViews();
#if PLATFORM(IOS_FAMILY) && HAVE(AVKIT) || (PLATFORM(MAC) && ENABLE(VIDEO_PRESENTATION_MODE))
    // When leaving the current page, close the video fullscreen.
    if (m_videoFullscreenManager && m_videoFullscreenManager->hasMode(WebCore::HTMLMediaElementEnums::VideoFullscreenModeStandard))
        m_videoFullscreenManager->requestHideAndExitFullscreen();
#endif
}

void WebPageProxy::viewDidEnterWindow()
{
    LayerHostingMode layerHostingMode = pageClient().viewLayerHostingMode();
    if (m_layerHostingMode != layerHostingMode) {
        m_layerHostingMode = layerHostingMode;
        m_process->send(Messages::WebPage::SetLayerHostingMode(layerHostingMode), m_pageID);
    }
}

void WebPageProxy::dispatchActivityStateChange()
{
#if PLATFORM(COCOA)
    m_activityStateChangeDispatcher->invalidate();
#endif

    if (!hasRunningProcess())
        return;

    LOG_WITH_STREAM(ActivityState, stream << "WebPageProxy " << pageID() << " dispatchActivityStateChange - potentiallyChangedActivityStateFlags " << m_potentiallyChangedActivityStateFlags);

    // If the visibility state may have changed, then so may the visually idle & occluded agnostic state.
    if (m_potentiallyChangedActivityStateFlags & ActivityState::IsVisible)
        m_potentiallyChangedActivityStateFlags.add({ ActivityState::IsVisibleOrOccluded, ActivityState::IsVisuallyIdle });

    // Record the prior view state, update the flags that may have changed,
    // and check which flags have actually changed.
    auto previousActivityState = m_activityState;
    updateActivityState(m_potentiallyChangedActivityStateFlags);
    auto changed = m_activityState ^ previousActivityState;

    if (changed)
        LOG_WITH_STREAM(ActivityState, stream << "WebPageProxy " << pageID() << " dispatchActivityStateChange: state changed from " << previousActivityState << " to " << m_activityState);

    if ((changed & ActivityState::WindowIsActive) && isViewWindowActive())
        updateCurrentModifierState();

    if ((m_potentiallyChangedActivityStateFlags & ActivityState::IsVisible) && isViewVisible())
        viewIsBecomingVisible();

    bool isNowInWindow = (changed & ActivityState::IsInWindow) && isInWindow();
    // We always want to wait for the Web process to reply if we've been in-window before and are coming back in-window.
    if (m_viewWasEverInWindow && isNowInWindow) {
        if (m_drawingArea->hasVisibleContent() && m_waitsForPaintAfterViewDidMoveToWindow && !m_shouldSkipWaitingForPaintAfterNextViewDidMoveToWindow)
            m_activityStateChangeWantsSynchronousReply = true;
        m_shouldSkipWaitingForPaintAfterNextViewDidMoveToWindow = false;
    }

    // Don't wait synchronously if the view state is not visible. (This matters in particular on iOS, where a hidden page may be suspended.)
    if (!(m_activityState & ActivityState::IsVisible))
        m_activityStateChangeWantsSynchronousReply = false;

    auto activityStateChangeID = m_activityStateChangeWantsSynchronousReply ? takeNextActivityStateChangeID() : static_cast<ActivityStateChangeID>(ActivityStateChangeAsynchronous);

    if (changed || activityStateChangeID != ActivityStateChangeAsynchronous || !m_nextActivityStateChangeCallbacks.isEmpty())
        m_process->send(Messages::WebPage::SetActivityState(m_activityState, activityStateChangeID, m_nextActivityStateChangeCallbacks), m_pageID);

    m_nextActivityStateChangeCallbacks.clear();

    // This must happen after the SetActivityState message is sent, to ensure the page visibility event can fire.
    updateThrottleState();

#if ENABLE(POINTER_LOCK)
    if (((changed & ActivityState::IsVisible) && !isViewVisible()) || ((changed & ActivityState::WindowIsActive) && !pageClient().isViewWindowActive())
        || ((changed & ActivityState::IsFocused) && !(m_activityState & ActivityState::IsFocused)))
        requestPointerUnlock();
#endif

    if (changed & ActivityState::IsVisible) {
        if (isViewVisible())
            m_visiblePageToken = m_process->visiblePageToken();
        else {
            m_visiblePageToken = nullptr;

            // If we've started the responsiveness timer as part of telling the web process to update the backing store
            // state, it might not send back a reply (since it won't paint anything if the web page is hidden) so we
            // stop the unresponsiveness timer here.
            m_process->responsivenessTimer().stop();
        }
    }

    if (changed & ActivityState::IsInWindow) {
        if (isInWindow())
            viewDidEnterWindow();
        else
            viewDidLeaveWindow();
    }

    updateBackingStoreDiscardableState();

    if (activityStateChangeID != ActivityStateChangeAsynchronous)
        waitForDidUpdateActivityState(activityStateChangeID);

    m_potentiallyChangedActivityStateFlags = { };
    m_activityStateChangeWantsSynchronousReply = false;
    m_viewWasEverInWindow |= isNowInWindow;
}

bool WebPageProxy::isAlwaysOnLoggingAllowed() const
{
    return sessionID().isAlwaysOnLoggingAllowed();
}

void WebPageProxy::updateThrottleState()
{
    bool processSuppressionEnabled = m_preferences->pageVisibilityBasedProcessSuppressionEnabled();

    // If process suppression is not enabled take a token on the process pool to disable suppression of support processes.
    if (!processSuppressionEnabled)
        m_preventProcessSuppressionCount = m_process->processPool().processSuppressionDisabledForPageCount();
    else if (!m_preventProcessSuppressionCount)
        m_preventProcessSuppressionCount = nullptr;

    if (m_activityState & ActivityState::IsVisuallyIdle)
        m_pageIsUserObservableCount = nullptr;
    else if (!m_pageIsUserObservableCount)
        m_pageIsUserObservableCount = m_process->processPool().userObservablePageCount();

#if PLATFORM(IOS_FAMILY)
    bool isCapturingMedia = m_activityState.contains(ActivityState::IsCapturingMedia);
    bool isAudible = m_activityState.contains(ActivityState::IsAudible);
    if (!isViewVisible() && !m_alwaysRunsAtForegroundPriority && !isCapturingMedia && !isAudible) {
        if (m_activityToken) {
            RELEASE_LOG_IF_ALLOWED(ProcessSuspension, "updateThrottleState: UIProcess is releasing a foreground assertion because the view is no longer visible");
            m_activityToken = nullptr;
        }
    } else if (!m_activityToken) {
        if (isViewVisible())
            RELEASE_LOG_IF_ALLOWED(ProcessSuspension, "updateThrottleState: UIProcess is taking a foreground assertion because the view is visible");
        else if (isAudible)
            RELEASE_LOG_IF_ALLOWED(ProcessSuspension, "updateThrottleState: UIProcess is taking a foreground assertion because we are playing audio");
        else if (isCapturingMedia)
            RELEASE_LOG_IF_ALLOWED(ProcessSuspension, "updateThrottleState: UIProcess is taking a foreground assertion because media capture is active");
        else
            RELEASE_LOG_IF_ALLOWED(ProcessSuspension, "updateThrottleState: UIProcess is taking a foreground assertion even though the view is not visible because m_alwaysRunsAtForegroundPriority is true");
        m_activityToken = m_process->throttler().foregroundActivityToken();
    }
#endif
}

void WebPageProxy::updateHiddenPageThrottlingAutoIncreases()
{
    if (!m_preferences->hiddenPageDOMTimerThrottlingAutoIncreases())
        m_hiddenPageDOMTimerThrottlingAutoIncreasesCount = nullptr;
    else if (!m_hiddenPageDOMTimerThrottlingAutoIncreasesCount)
        m_hiddenPageDOMTimerThrottlingAutoIncreasesCount = m_process->processPool().hiddenPageThrottlingAutoIncreasesCount();
}

void WebPageProxy::layerHostingModeDidChange()
{
    LayerHostingMode layerHostingMode = pageClient().viewLayerHostingMode();
    if (m_layerHostingMode == layerHostingMode)
        return;

    m_layerHostingMode = layerHostingMode;

    if (hasRunningProcess())
        m_process->send(Messages::WebPage::SetLayerHostingMode(layerHostingMode), m_pageID);
}

void WebPageProxy::waitForDidUpdateActivityState(ActivityStateChangeID activityStateChangeID)
{
    if (!hasRunningProcess())
        return;

    if (m_process->state() != WebProcessProxy::State::Running)
        return;

    // If we have previously timed out with no response from the WebProcess, don't block the UIProcess again until it starts responding.
    if (m_waitingForDidUpdateActivityState)
        return;

#if PLATFORM(IOS_FAMILY)
    // Hail Mary check. Should not be possible (dispatchActivityStateChange should force async if not visible,
    // and if visible we should be holding an assertion) - but we should never block on a suspended process.
    if (!m_activityToken) {
        ASSERT_NOT_REACHED();
        return;
    }
#endif

    m_waitingForDidUpdateActivityState = true;

    m_drawingArea->waitForDidUpdateActivityState(activityStateChangeID);
}

IntSize WebPageProxy::viewSize() const
{
    return pageClient().viewSize();
}

void WebPageProxy::setInitialFocus(bool forward, bool isKeyboardEventValid, const WebKeyboardEvent& keyboardEvent, WTF::Function<void (CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(CallbackBase::Error::OwnerWasInvalidated);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::SetInitialFocus(forward, isKeyboardEventValid, keyboardEvent, callbackID), m_pageID);
}

void WebPageProxy::clearSelection()
{
    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::ClearSelection(), m_pageID);
}

void WebPageProxy::restoreSelectionInFocusedEditableElement()
{
    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::RestoreSelectionInFocusedEditableElement(), m_pageID);
}

void WebPageProxy::validateCommand(const String& commandName, WTF::Function<void (const String&, bool, int32_t, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(String(), false, 0, CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::ValidateCommand(commandName, callbackID), m_pageID);
}

void WebPageProxy::increaseListLevel()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::IncreaseListLevel(), m_pageID);
}

void WebPageProxy::decreaseListLevel()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::DecreaseListLevel(), m_pageID);
}

void WebPageProxy::changeListType()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::ChangeListType(), m_pageID);
}

void WebPageProxy::setBaseWritingDirection(WritingDirection direction)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetBaseWritingDirection(direction), m_pageID);
}

void WebPageProxy::updateFontAttributesAfterEditorStateChange()
{
    m_cachedFontAttributesAtSelectionStart.reset();

    if (m_editorState.isMissingPostLayoutData)
        return;

    if (auto fontAttributes = m_editorState.postLayoutData().fontAttributes) {
        m_uiClient->didChangeFontAttributes(*fontAttributes);
        m_cachedFontAttributesAtSelectionStart = WTFMove(fontAttributes);
    }
}

void WebPageProxy::setNeedsFontAttributes(bool needsFontAttributes)
{
    if (m_needsFontAttributes == needsFontAttributes)
        return;

    m_needsFontAttributes = needsFontAttributes;

    if (hasRunningProcess())
        m_process->send(Messages::WebPage::SetNeedsFontAttributes(needsFontAttributes), m_pageID);
}

bool WebPageProxy::maintainsInactiveSelection() const
{
    // Regardless of what the client wants to do, keep selections if a local Inspector is open.
    // Otherwise, there is no way to use the console to inspect the state of a selection.
    if (inspector() && inspector()->isVisible())
        return true;

    return m_maintainsInactiveSelection;
}

void WebPageProxy::setMaintainsInactiveSelection(bool newValue)
{
    m_maintainsInactiveSelection = newValue;
}

void WebPageProxy::scheduleFullEditorStateUpdate()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::ScheduleFullEditorStateUpdate(), m_pageID);
}

void WebPageProxy::selectAll()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SelectAll(), m_pageID);
}

void WebPageProxy::executeEditCommand(const String& commandName, const String& argument, WTF::Function<void(CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::ExecuteEditCommandWithCallback(commandName, argument, callbackID), m_pageID);
}
    
void WebPageProxy::executeEditCommand(const String& commandName, const String& argument)
{
    static NeverDestroyed<String> ignoreSpellingCommandName(MAKE_STATIC_STRING_IMPL("ignoreSpelling"));

    if (!hasRunningProcess())
        return;

    if (commandName == ignoreSpellingCommandName)
        ++m_pendingLearnOrIgnoreWordMessageCount;

    m_process->send(Messages::WebPage::ExecuteEditCommand(commandName, argument), m_pageID);
}

void WebPageProxy::requestFontAttributesAtSelectionStart(Function<void(const WebCore::FontAttributes&, CallbackBase::Error)>&& callback)
{
    if (!hasRunningProcess()) {
        callback({ }, CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callback), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::RequestFontAttributesAtSelectionStart(callbackID), m_pageID);
}

void WebPageProxy::fontAttributesCallback(const WebCore::FontAttributes& attributes, CallbackID callbackID)
{
    m_cachedFontAttributesAtSelectionStart = attributes;

    if (auto callback = m_callbacks.take<FontAttributesCallback>(callbackID))
        callback->performCallbackWithReturnValue(attributes);
}

void WebPageProxy::setEditable(bool editable)
{
    if (editable == m_isEditable)
        return;

    m_isEditable = editable;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetEditable(editable), m_pageID);
}
    
void WebPageProxy::setMediaStreamCaptureMuted(bool muted)
{
    if (muted)
        setMuted(m_mutedState | WebCore::MediaProducer::MediaStreamCaptureIsMuted);
    else
        setMuted(m_mutedState & ~WebCore::MediaProducer::MediaStreamCaptureIsMuted);
}

void WebPageProxy::activateMediaStreamCaptureInPage()
{
#if ENABLE(MEDIA_STREAM)
    UserMediaProcessManager::singleton().muteCaptureMediaStreamsExceptIn(*this);
#endif
    setMuted(m_mutedState & ~WebCore::MediaProducer::MediaStreamCaptureIsMuted);
}

#if !PLATFORM(IOS_FAMILY)
void WebPageProxy::didCommitLayerTree(const RemoteLayerTreeTransaction&)
{
}

void WebPageProxy::layerTreeCommitComplete()
{
}
#endif

#if ENABLE(DRAG_SUPPORT)
void WebPageProxy::dragEntered(DragData& dragData, const String& dragStorageName)
{
    performDragControllerAction(DragControllerAction::Entered, dragData, dragStorageName, { }, { });
}

void WebPageProxy::dragUpdated(DragData& dragData, const String& dragStorageName)
{
    performDragControllerAction(DragControllerAction::Updated, dragData, dragStorageName, { }, { });
}

void WebPageProxy::dragExited(DragData& dragData, const String& dragStorageName)
{
    performDragControllerAction(DragControllerAction::Exited, dragData, dragStorageName, { }, { });
}

void WebPageProxy::performDragOperation(DragData& dragData, const String& dragStorageName, SandboxExtension::Handle&& sandboxExtensionHandle, SandboxExtension::HandleArray&& sandboxExtensionsForUpload)
{
    performDragControllerAction(DragControllerAction::PerformDragOperation, dragData, dragStorageName, WTFMove(sandboxExtensionHandle), WTFMove(sandboxExtensionsForUpload));
}

void WebPageProxy::performDragControllerAction(DragControllerAction action, DragData& dragData, const String& dragStorageName, SandboxExtension::Handle&& sandboxExtensionHandle, SandboxExtension::HandleArray&& sandboxExtensionsForUpload)
{
    if (!hasRunningProcess())
        return;
#if PLATFORM(GTK)
    UNUSED_PARAM(dragStorageName);
    UNUSED_PARAM(sandboxExtensionHandle);
    UNUSED_PARAM(sandboxExtensionsForUpload);

    String url = dragData.asURL();
    if (!url.isEmpty())
        m_process->assumeReadAccessToBaseURL(*this, url);

    ASSERT(dragData.platformData());
    WebSelectionData selection(*dragData.platformData());
    m_process->send(Messages::WebPage::PerformDragControllerAction(action, dragData.clientPosition(), dragData.globalPosition(), dragData.draggingSourceOperationMask(), selection, dragData.flags()), m_pageID);
#else
    m_process->send(Messages::WebPage::PerformDragControllerAction(action, dragData, sandboxExtensionHandle, sandboxExtensionsForUpload), m_pageID);
#endif
}

void WebPageProxy::didPerformDragControllerAction(uint64_t dragOperation, WebCore::DragHandlingMethod dragHandlingMethod, bool mouseIsOverFileInput, unsigned numberOfItemsToBeAccepted, const IntRect& insertionRect, const IntRect& editableElementRect)
{
    MESSAGE_CHECK(m_process, dragOperation <= DragOperationDelete);

    m_currentDragOperation = static_cast<DragOperation>(dragOperation);
    m_currentDragHandlingMethod = dragHandlingMethod;
    m_currentDragIsOverFileInput = mouseIsOverFileInput;
    m_currentDragNumberOfFilesToBeAccepted = numberOfItemsToBeAccepted;
    m_currentDragCaretEditableElementRect = editableElementRect;
    setDragCaretRect(insertionRect);
}

#if PLATFORM(GTK)
void WebPageProxy::startDrag(WebSelectionData&& selection, uint64_t dragOperation, const ShareableBitmap::Handle& dragImageHandle)
{
    RefPtr<ShareableBitmap> dragImage = !dragImageHandle.isNull() ? ShareableBitmap::create(dragImageHandle) : nullptr;
    pageClient().startDrag(WTFMove(selection.selectionData), static_cast<WebCore::DragOperation>(dragOperation), WTFMove(dragImage));

    didStartDrag();
}
#endif

void WebPageProxy::dragEnded(const IntPoint& clientPosition, const IntPoint& globalPosition, uint64_t operation)
{
    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::DragEnded(clientPosition, globalPosition, operation), m_pageID);
    setDragCaretRect({ });
}

void WebPageProxy::didPerformDragOperation(bool handled)
{
    pageClient().didPerformDragOperation(handled);
}

void WebPageProxy::didStartDrag()
{
    if (hasRunningProcess())
        m_process->send(Messages::WebPage::DidStartDrag(), m_pageID);
}
    
void WebPageProxy::dragCancelled()
{
    if (hasRunningProcess())
        m_process->send(Messages::WebPage::DragCancelled(), m_pageID);
}

void WebPageProxy::didEndDragging()
{
    resetCurrentDragInformation();
}

void WebPageProxy::resetCurrentDragInformation()
{
    m_currentDragOperation = WebCore::DragOperationNone;
    m_currentDragHandlingMethod = DragHandlingMethod::None;
    m_currentDragIsOverFileInput = false;
    m_currentDragNumberOfFilesToBeAccepted = 0;
    setDragCaretRect({ });
}

#if !ENABLE(DATA_INTERACTION)

void WebPageProxy::setDragCaretRect(const IntRect& dragCaretRect)
{
    m_currentDragCaretRect = dragCaretRect;
}

#endif

#endif // ENABLE(DRAG_SUPPORT)

static bool removeOldRedundantEvent(Deque<NativeWebMouseEvent>& queue, WebEvent::Type incomingEventType)
{
    if (incomingEventType != WebEvent::MouseMove && incomingEventType != WebEvent::MouseForceChanged)
        return false;

    auto it = queue.rbegin();
    auto end = queue.rend();

    // Must not remove the first event in the deque, since it is already being dispatched.
    if (it != end)
        --end;

    for (; it != end; ++it) {
        auto type = it->type();
        if (type == incomingEventType) {
            queue.remove(--it.base());
            return true;
        }
        if (type != WebEvent::MouseMove && type != WebEvent::MouseForceChanged)
            break;
    }
    return false;
}

void WebPageProxy::handleMouseEvent(const NativeWebMouseEvent& event)
{
    if (!hasRunningProcess())
        return;

#if ENABLE(ASYNC_SCROLLING) && PLATFORM(COCOA)
    if (m_scrollingCoordinatorProxy)
        m_scrollingCoordinatorProxy->handleMouseEvent(platform(event));
#endif

    // If we receive multiple mousemove or mouseforcechanged events and the most recent mousemove or mouseforcechanged event
    // (respectively) has not yet been sent to WebProcess for processing, remove the pending mouse event and insert the new
    // event in the queue.
    bool didRemoveEvent = removeOldRedundantEvent(m_mouseEventQueue, event.type());
    m_mouseEventQueue.append(event);

#if LOG_DISABLED
    UNUSED_PARAM(didRemoveEvent);
#else
    LOG(MouseHandling, "UIProcess: %s mouse event %s (queue size %zu)", didRemoveEvent ? "replaced" : "enqueued", webMouseEventTypeString(event.type()), m_mouseEventQueue.size());
#endif

    if (m_mouseEventQueue.size() == 1) // Otherwise, called from DidReceiveEvent message handler.
        processNextQueuedMouseEvent();
}
    
void WebPageProxy::processNextQueuedMouseEvent()
{
    if (!hasRunningProcess())
        return;

    ASSERT(!m_mouseEventQueue.isEmpty());

    const NativeWebMouseEvent& event = m_mouseEventQueue.first();

    if (pageClient().windowIsFrontWindowUnderMouse(event))
        setToolTip(String());

    WebEvent::Type eventType = event.type();
    if (eventType == WebEvent::MouseDown || eventType == WebEvent::MouseForceChanged || eventType == WebEvent::MouseForceDown)
        m_process->responsivenessTimer().startWithLazyStop();
    else if (eventType != WebEvent::MouseMove) {
        // NOTE: This does not start the responsiveness timer because mouse move should not indicate interaction.
        m_process->responsivenessTimer().start();
    }

    LOG(MouseHandling, "UIProcess: sent mouse event %s (queue size %zu)", webMouseEventTypeString(eventType), m_mouseEventQueue.size());
    m_process->send(Messages::WebPage::MouseEvent(event), m_pageID);
}

#if MERGE_WHEEL_EVENTS
static bool canCoalesce(const WebWheelEvent& a, const WebWheelEvent& b)
{
    if (a.position() != b.position())
        return false;
    if (a.globalPosition() != b.globalPosition())
        return false;
    if (a.modifiers() != b.modifiers())
        return false;
    if (a.granularity() != b.granularity())
        return false;
#if PLATFORM(COCOA)
    if (a.phase() != b.phase())
        return false;
    if (a.momentumPhase() != b.momentumPhase())
        return false;
    if (a.hasPreciseScrollingDeltas() != b.hasPreciseScrollingDeltas())
        return false;
#endif

    return true;
}

static WebWheelEvent coalesce(const WebWheelEvent& a, const WebWheelEvent& b)
{
    ASSERT(canCoalesce(a, b));

    FloatSize mergedDelta = a.delta() + b.delta();
    FloatSize mergedWheelTicks = a.wheelTicks() + b.wheelTicks();

#if PLATFORM(COCOA)
    FloatSize mergedUnacceleratedScrollingDelta = a.unacceleratedScrollingDelta() + b.unacceleratedScrollingDelta();

    return WebWheelEvent(WebEvent::Wheel, b.position(), b.globalPosition(), mergedDelta, mergedWheelTicks, b.granularity(), b.directionInvertedFromDevice(), b.phase(), b.momentumPhase(), b.hasPreciseScrollingDeltas(), b.scrollCount(), mergedUnacceleratedScrollingDelta, b.modifiers(), b.timestamp());
#else
    return WebWheelEvent(WebEvent::Wheel, b.position(), b.globalPosition(), mergedDelta, mergedWheelTicks, b.granularity(), b.modifiers(), b.timestamp());
#endif
}
#endif // MERGE_WHEEL_EVENTS

static WebWheelEvent coalescedWheelEvent(Deque<NativeWebWheelEvent>& queue, Vector<NativeWebWheelEvent>& coalescedEvents)
{
    ASSERT(!queue.isEmpty());
    ASSERT(coalescedEvents.isEmpty());

#if MERGE_WHEEL_EVENTS
    NativeWebWheelEvent firstEvent = queue.takeFirst();
    coalescedEvents.append(firstEvent);

    WebWheelEvent event = firstEvent;
    while (!queue.isEmpty() && canCoalesce(event, queue.first())) {
        NativeWebWheelEvent firstEvent = queue.takeFirst();
        coalescedEvents.append(firstEvent);
        event = coalesce(event, firstEvent);
    }

    return event;
#else
    while (!queue.isEmpty())
        coalescedEvents.append(queue.takeFirst());
    return coalescedEvents.last();
#endif
}

void WebPageProxy::handleWheelEvent(const NativeWebWheelEvent& event)
{
#if ENABLE(ASYNC_SCROLLING) && PLATFORM(COCOA)
    if (m_scrollingCoordinatorProxy && m_scrollingCoordinatorProxy->handleWheelEvent(platform(event)))
        return;
#endif

    if (!hasRunningProcess())
        return;

    closeOverlayedViews();

    if (!m_currentlyProcessedWheelEvents.isEmpty()) {
        m_wheelEventQueue.append(event);
        if (!shouldProcessWheelEventNow(event))
            return;
        // The queue has too many wheel events, so push a new event.
    }

    if (!m_wheelEventQueue.isEmpty()) {
        processNextQueuedWheelEvent();
        return;
    }

    auto coalescedWheelEvent = makeUnique<Vector<NativeWebWheelEvent>>();
    coalescedWheelEvent->append(event);
    m_currentlyProcessedWheelEvents.append(WTFMove(coalescedWheelEvent));
    sendWheelEvent(event);
}

void WebPageProxy::processNextQueuedWheelEvent()
{
    auto nextCoalescedEvent = makeUnique<Vector<NativeWebWheelEvent>>();
    WebWheelEvent nextWheelEvent = coalescedWheelEvent(m_wheelEventQueue, *nextCoalescedEvent.get());
    m_currentlyProcessedWheelEvents.append(WTFMove(nextCoalescedEvent));
    sendWheelEvent(nextWheelEvent);
}

void WebPageProxy::sendWheelEvent(const WebWheelEvent& event)
{
    m_process->send(
        Messages::EventDispatcher::WheelEvent(
            m_pageID,
            event,
            shouldUseImplicitRubberBandControl() ? !m_backForwardList->backItem() : rubberBandsAtLeft(),
            shouldUseImplicitRubberBandControl() ? !m_backForwardList->forwardItem() : rubberBandsAtRight(),
            rubberBandsAtTop(),
            rubberBandsAtBottom()
        ), 0);

    // Manually ping the web process to check for responsiveness since our wheel
    // event will dispatch to a non-main thread, which always responds.
    m_process->isResponsiveWithLazyStop();
}

bool WebPageProxy::shouldProcessWheelEventNow(const WebWheelEvent& event) const
{
#if PLATFORM(GTK)
    // Don't queue events representing a non-trivial scrolling phase to
    // avoid having them trapped in the queue, potentially preventing a
    // scrolling session to beginning or end correctly.
    // This is only needed by platforms whose WebWheelEvent has this phase
    // information (Cocoa and GTK+) but Cocoa was fine without it.
    if (event.phase() == WebWheelEvent::Phase::PhaseNone
        || event.phase() == WebWheelEvent::Phase::PhaseChanged
        || event.momentumPhase() == WebWheelEvent::Phase::PhaseNone
        || event.momentumPhase() == WebWheelEvent::Phase::PhaseChanged)
        return true;
#else
    UNUSED_PARAM(event);
#endif
    if (m_wheelEventQueue.size() >= wheelEventQueueSizeThreshold)
        return true;
    return false;
}

void WebPageProxy::handleKeyboardEvent(const NativeWebKeyboardEvent& event)
{
    if (!hasRunningProcess())
        return;
    
    LOG(KeyHandling, "WebPageProxy::handleKeyboardEvent: %s", webKeyboardEventTypeString(event.type()));

    m_keyEventQueue.append(event);

    ResponsivenessTimer& responsivenessTimer = m_process->responsivenessTimer();
    if (event.type() == WebEvent::KeyDown)
        responsivenessTimer.startWithLazyStop();
    else
        responsivenessTimer.start();

    if (m_keyEventQueue.size() == 1) { // Otherwise, sent from DidReceiveEvent message handler.
        LOG(KeyHandling, " UI process: sent keyEvent from handleKeyboardEvent");
        m_process->send(Messages::WebPage::KeyEvent(event), m_pageID);
    }
}

WebPreferencesStore WebPageProxy::preferencesStore() const
{
    if (m_configurationPreferenceValues.isEmpty())
        return m_preferences->store();

    WebPreferencesStore store = m_preferences->store();
    for (const auto& preference : m_configurationPreferenceValues)
        store.m_values.set(preference.key, preference.value);

    return store;
}

#if ENABLE(NETSCAPE_PLUGIN_API)
void WebPageProxy::findPlugin(const String& mimeType, uint32_t processType, const String& urlString, const String& frameURLString, const String& pageURLString, bool allowOnlyApplicationPlugins, Messages::WebPageProxy::FindPlugin::DelayedReply&& reply)
{
    PageClientProtector protector(pageClient());

    MESSAGE_CHECK_URL(m_process, urlString);

    URL pluginURL = URL { URL(), urlString };
    String newMimeType = mimeType.convertToASCIILowercase();

    PluginData::AllowedPluginTypes allowedPluginTypes = allowOnlyApplicationPlugins ? PluginData::OnlyApplicationPlugins : PluginData::AllPlugins;

    URL pageURL = URL { URL(), pageURLString };
    if (!m_process->processPool().pluginInfoStore().isSupportedPlugin(mimeType, pluginURL, frameURLString, pageURL)) {
        reply(0, newMimeType, PluginModuleLoadNormally, { }, true);
        return;
    }

    PluginModuleInfo plugin = m_process->processPool().pluginInfoStore().findPlugin(newMimeType, pluginURL, allowedPluginTypes);
    if (!plugin.path) {
        reply(0, newMimeType, PluginModuleLoadNormally, { }, false);
        return;
    }

    uint32_t pluginLoadPolicy = PluginInfoStore::defaultLoadPolicyForPlugin(plugin);

#if PLATFORM(COCOA)
    auto pluginInformation = createPluginInformationDictionary(plugin, frameURLString, String(), pageURLString, String(), String());
#endif

    auto findPluginCompletion = [processType, reply = WTFMove(reply), newMimeType = WTFMove(newMimeType), plugin = WTFMove(plugin)] (uint32_t pluginLoadPolicy, const String& unavailabilityDescription) mutable {
        PluginProcessSandboxPolicy pluginProcessSandboxPolicy = PluginProcessSandboxPolicyNormal;
        switch (pluginLoadPolicy) {
        case PluginModuleLoadNormally:
            pluginProcessSandboxPolicy = PluginProcessSandboxPolicyNormal;
            break;
        case PluginModuleLoadUnsandboxed:
            pluginProcessSandboxPolicy = PluginProcessSandboxPolicyUnsandboxed;
            break;

        case PluginModuleBlockedForSecurity:
        case PluginModuleBlockedForCompatibility:
            reply(0, newMimeType, pluginLoadPolicy, unavailabilityDescription, false);
            return;
        }

        reply(PluginProcessManager::singleton().pluginProcessToken(plugin, static_cast<PluginProcessType>(processType), pluginProcessSandboxPolicy), newMimeType, pluginLoadPolicy, unavailabilityDescription, false);
    };

#if PLATFORM(COCOA)
    m_navigationClient->decidePolicyForPluginLoad(*this, static_cast<PluginModuleLoadPolicy>(pluginLoadPolicy), pluginInformation.get(), WTFMove(findPluginCompletion));
#else
    findPluginCompletion(pluginLoadPolicy, { });
#endif
}

#endif // ENABLE(NETSCAPE_PLUGIN_API)

#if ENABLE(TOUCH_EVENTS)

static TrackingType mergeTrackingTypes(TrackingType a, TrackingType b)
{
    if (static_cast<uintptr_t>(b) > static_cast<uintptr_t>(a))
        return b;
    return a;
}

void WebPageProxy::updateTouchEventTracking(const WebTouchEvent& touchStartEvent)
{
#if ENABLE(ASYNC_SCROLLING) && PLATFORM(COCOA)
    const EventNames& names = eventNames();
    for (auto& touchPoint : touchStartEvent.touchPoints()) {
        IntPoint location = touchPoint.location();
        auto updateTrackingType = [this, location](TrackingType& trackingType, const AtomString& eventName) {
            if (trackingType == TrackingType::Synchronous)
                return;

            TrackingType trackingTypeForLocation = m_scrollingCoordinatorProxy->eventTrackingTypeForPoint(eventName, location);

            trackingType = mergeTrackingTypes(trackingType, trackingTypeForLocation);
        };
        updateTrackingType(m_touchAndPointerEventTracking.touchForceChangedTracking, names.touchforcechangeEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchStartTracking, names.touchstartEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchMoveTracking, names.touchmoveEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchEndTracking, names.touchendEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchStartTracking, names.pointeroverEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchStartTracking, names.pointerenterEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchStartTracking, names.pointerdownEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchMoveTracking, names.pointermoveEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchEndTracking, names.pointerupEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchEndTracking, names.pointeroutEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchEndTracking, names.pointerleaveEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchStartTracking, names.mousedownEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchMoveTracking, names.mousemoveEvent);
        updateTrackingType(m_touchAndPointerEventTracking.touchEndTracking, names.mouseupEvent);
    }
#else
    UNUSED_PARAM(touchStartEvent);
    m_touchAndPointerEventTracking.touchForceChangedTracking = TrackingType::Synchronous;
    m_touchAndPointerEventTracking.touchStartTracking = TrackingType::Synchronous;
    m_touchAndPointerEventTracking.touchMoveTracking = TrackingType::Synchronous;
    m_touchAndPointerEventTracking.touchEndTracking = TrackingType::Synchronous;
#endif // ENABLE(ASYNC_SCROLLING)
}

TrackingType WebPageProxy::touchEventTrackingType(const WebTouchEvent& touchStartEvent) const
{
    // We send all events if any type is needed, we just do it asynchronously for the types that are not tracked.
    //
    // Touch events define a sequence with strong dependencies. For example, we can expect
    // a TouchMove to only appear after a TouchStart, and the ids of the touch points is consistent between
    // the two.
    //
    // WebCore should not have to set up its state correctly after some events were dismissed.
    // For example, we don't want to send a TouchMoved without a TouchPressed.
    // We send everything, WebCore updates its internal state and dispatch what is needed to the page.
    TrackingType globalTrackingType = m_touchAndPointerEventTracking.isTrackingAnything() ? TrackingType::Asynchronous : TrackingType::NotTracking;

    globalTrackingType = mergeTrackingTypes(globalTrackingType, m_touchAndPointerEventTracking.touchForceChangedTracking);
    for (auto& touchPoint : touchStartEvent.touchPoints()) {
        switch (touchPoint.state()) {
        case WebPlatformTouchPoint::TouchReleased:
            globalTrackingType = mergeTrackingTypes(globalTrackingType, m_touchAndPointerEventTracking.touchEndTracking);
            break;
        case WebPlatformTouchPoint::TouchPressed:
            globalTrackingType = mergeTrackingTypes(globalTrackingType, m_touchAndPointerEventTracking.touchStartTracking);
            break;
        case WebPlatformTouchPoint::TouchMoved:
        case WebPlatformTouchPoint::TouchStationary:
            globalTrackingType = mergeTrackingTypes(globalTrackingType, m_touchAndPointerEventTracking.touchMoveTracking);
            break;
        case WebPlatformTouchPoint::TouchCancelled:
            globalTrackingType = mergeTrackingTypes(globalTrackingType, TrackingType::Asynchronous);
            break;
        }
    }

    return globalTrackingType;
}

#endif

#if ENABLE(MAC_GESTURE_EVENTS)
void WebPageProxy::handleGestureEvent(const NativeWebGestureEvent& event)
{
    if (!hasRunningProcess())
        return;

    m_gestureEventQueue.append(event);
    // FIXME: Consider doing some coalescing here.

    ResponsivenessTimer& responsivenessTimer = m_process->responsivenessTimer();
    if (event.type() == WebEvent::GestureStart || event.type() == WebEvent::GestureChange)
        responsivenessTimer.startWithLazyStop();
    else
        responsivenessTimer.start();

    m_process->send(Messages::EventDispatcher::GestureEvent(m_pageID, event), 0);
}
#endif

#if ENABLE(IOS_TOUCH_EVENTS)
void WebPageProxy::handleTouchEventSynchronously(NativeWebTouchEvent& event)
{
    if (!hasRunningProcess())
        return;

    TraceScope scope(SyncTouchEventStart, SyncTouchEventEnd);

    updateTouchEventTracking(event);

    TrackingType touchEventsTrackingType = touchEventTrackingType(event);
    if (touchEventsTrackingType == TrackingType::NotTracking)
        return;

    if (touchEventsTrackingType == TrackingType::Asynchronous) {
        // We can end up here if a native gesture has not started but the event handlers are passive.
        //
        // The client of WebPageProxy asks the event to be sent synchronously since the touch event
        // can prevent a native gesture.
        // But, here we know that all events handlers that can handle this events are passive.
        // We can use asynchronous dispatch and pretend to the client that the page does nothing with the events.
        event.setCanPreventNativeGestures(false);
        handleTouchEventAsynchronously(event);
        didReceiveEvent(event.type(), false);
        return;
    }

    m_process->responsivenessTimer().start();
    bool handled = false;
    bool replyReceived = m_process->sendSync(Messages::WebPage::TouchEventSync(event), Messages::WebPage::TouchEventSync::Reply(handled), m_pageID, 1_s, IPC::SendSyncOption::ForceDispatchWhenDestinationIsWaitingForUnboundedSyncReply);
    // If the sync request has timed out, we should consider the event handled. The Web Process is too busy to answer any questions, so the default action is also likely to have issues.
    if (!replyReceived)
        handled = true;
    didReceiveEvent(event.type(), handled);
    pageClient().doneWithTouchEvent(event, handled);
    m_process->responsivenessTimer().stop();

    if (event.allTouchPointsAreReleased()) {
        m_touchAndPointerEventTracking.reset();
        didReleaseAllTouchPoints();
    }
}

void WebPageProxy::resetPotentialTapSecurityOrigin()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::ResetPotentialTapSecurityOrigin(), m_pageID);
}

void WebPageProxy::handleTouchEventAsynchronously(const NativeWebTouchEvent& event)
{
    if (!hasRunningProcess())
        return;

    TrackingType touchEventsTrackingType = touchEventTrackingType(event);
    if (touchEventsTrackingType == TrackingType::NotTracking)
        return;

    m_process->send(Messages::EventDispatcher::TouchEvent(m_pageID, event), 0);

    if (event.allTouchPointsAreReleased()) {
        m_touchAndPointerEventTracking.reset();
        didReleaseAllTouchPoints();
    }
}

#elif ENABLE(TOUCH_EVENTS)
void WebPageProxy::handleTouchEvent(const NativeWebTouchEvent& event)
{
    if (!hasRunningProcess())
        return;

    updateTouchEventTracking(event);

    if (touchEventTrackingType(event) == TrackingType::NotTracking)
        return;

    // If the page is suspended, which should be the case during panning, pinching
    // and animation on the page itself (kinetic scrolling, tap to zoom) etc, then
    // we do not send any of the events to the page even if is has listeners.
    if (!m_isPageSuspended) {
        m_touchEventQueue.append(event);
        m_process->responsivenessTimer().start();
        m_process->send(Messages::WebPage::TouchEvent(event), m_pageID);
    } else {
        if (m_touchEventQueue.isEmpty()) {
            bool isEventHandled = false;
            pageClient().doneWithTouchEvent(event, isEventHandled);
        } else {
            // We attach the incoming events to the newest queued event so that all
            // the events are delivered in the correct order when the event is dequed.
            QueuedTouchEvents& lastEvent = m_touchEventQueue.last();
            lastEvent.deferredTouchEvents.append(event);
        }
    }

    if (event.allTouchPointsAreReleased()) {
        m_touchAndPointerEventTracking.reset();
        didReleaseAllTouchPoints();
    }
}
#endif // ENABLE(TOUCH_EVENTS)

#if ENABLE(POINTER_EVENTS)
void WebPageProxy::cancelPointer(WebCore::PointerID pointerId, const WebCore::IntPoint& documentPoint)
{
    m_process->send(Messages::WebPage::CancelPointer(pointerId, documentPoint), m_pageID);
}

void WebPageProxy::touchWithIdentifierWasRemoved(WebCore::PointerID pointerId)
{
    m_process->send(Messages::WebPage::TouchWithIdentifierWasRemoved(pointerId), m_pageID);
}
#endif

void WebPageProxy::scrollBy(ScrollDirection direction, ScrollGranularity granularity)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::ScrollBy(direction, granularity), m_pageID);
}

void WebPageProxy::centerSelectionInVisibleArea()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::CenterSelectionInVisibleArea(), m_pageID);
}

class WebPageProxy::PolicyDecisionSender : public RefCounted<PolicyDecisionSender> {
public:
    using SendFunction = CompletionHandler<void(PolicyCheckIdentifier, PolicyAction, uint64_t newNavigationID, DownloadID, Optional<WebsitePoliciesData>)>;

    static Ref<PolicyDecisionSender> create(PolicyCheckIdentifier identifier, SendFunction&& sendFunction)
    {
        return adoptRef(*new PolicyDecisionSender(identifier, WTFMove(sendFunction)));
    }

    template<typename... Args> void send(Args... args)
    {
        if (m_sendFunction)
            m_sendFunction(m_identifier, std::forward<Args>(args)...);
    }
private:
    PolicyDecisionSender(PolicyCheckIdentifier identifier, SendFunction sendFunction)
        : m_sendFunction(WTFMove(sendFunction))
        , m_identifier(identifier)
        { }

    SendFunction m_sendFunction;
    PolicyCheckIdentifier m_identifier;
};

void WebPageProxy::receivedNavigationPolicyDecision(PolicyAction policyAction, API::Navigation* navigation, ProcessSwapRequestedByClient processSwapRequestedByClient, WebFrameProxy& frame, API::WebsitePolicies* policies, Ref<PolicyDecisionSender>&& sender)
{
    Ref<WebsiteDataStore> websiteDataStore = m_websiteDataStore.copyRef();
    Optional<WebsitePoliciesData> data;
    if (policies) {
        data = policies->data();
        if (policies->websiteDataStore() && &policies->websiteDataStore()->websiteDataStore() != websiteDataStore.ptr()) {
            websiteDataStore = policies->websiteDataStore()->websiteDataStore();
            processSwapRequestedByClient = ProcessSwapRequestedByClient::Yes;
        }
    }

    if (navigation && !navigation->userContentExtensionsEnabled()) {
        if (!data)
            data = WebsitePoliciesData { };
        data->contentBlockersEnabled = false;
    }

#if ENABLE(DEVICE_ORIENTATION)
    if (navigation && (!data || data->deviceOrientationAndMotionAccessState == WebCore::DeviceOrientationOrMotionPermissionState::Prompt)) {
        auto deviceOrientationPermission = websiteDataStore->deviceOrientationAndMotionAccessController().cachedDeviceOrientationPermission(SecurityOriginData::fromURL(navigation->currentRequest().url()));
        if (deviceOrientationPermission != WebCore::DeviceOrientationOrMotionPermissionState::Prompt) {
            if (!data)
                data = WebsitePoliciesData { };
            data->deviceOrientationAndMotionAccessState = deviceOrientationPermission;
        }
    }
#endif

#if PLATFORM(COCOA)
    static const bool forceDownloadFromDownloadAttribute = false;
#else
    static const bool forceDownloadFromDownloadAttribute = true;
#endif
    if (policyAction == PolicyAction::Use && navigation && (navigation->isSystemPreview() || (forceDownloadFromDownloadAttribute && navigation->shouldPerformDownload())))
        policyAction = PolicyAction::Download;

    if (policyAction != PolicyAction::Use || !frame.isMainFrame() || !navigation) {
        receivedPolicyDecision(policyAction, navigation, WTFMove(data), WTFMove(sender));
        return;
    }

    Ref<WebProcessProxy> sourceProcess = process();
    URL sourceURL = URL { URL(), pageLoadState().url() };
    if (auto* provisionalPage = provisionalPageProxy()) {
        if (provisionalPage->navigationID() == navigation->navigationID()) {
            sourceProcess = provisionalPage->process();
            sourceURL = provisionalPage->provisionalURL();
        }
    }

    process().processPool().processForNavigation(*this, *navigation, sourceProcess.copyRef(), sourceURL, processSwapRequestedByClient, WTFMove(websiteDataStore), [this, protectedThis = makeRef(*this), policyAction, navigation = makeRef(*navigation), sourceProcess = sourceProcess.copyRef(),
        data = WTFMove(data), sender = WTFMove(sender), processSwapRequestedByClient] (Ref<WebProcessProxy>&& processForNavigation, SuspendedPageProxy* destinationSuspendedPage, const String& reason) mutable {
        // If the navigation has been destroyed, then no need to proceed.
        if (isClosed() || !navigationState().hasNavigation(navigation->navigationID())) {
            receivedPolicyDecision(policyAction, navigation.ptr(), WTFMove(data), WTFMove(sender));
            return;
        }

        bool shouldProcessSwap = processForNavigation.ptr() != sourceProcess.ptr();
        if (shouldProcessSwap) {
            policyAction = PolicyAction::StopAllLoads;
            RELEASE_LOG_IF_ALLOWED(ProcessSwapping, "decidePolicyForNavigationAction, swapping process %i with process %i for navigation, reason: %{public}s", processIdentifier(), processForNavigation->processIdentifier(), reason.utf8().data());
            LOG(ProcessSwapping, "(ProcessSwapping) Switching from process %i to new process (%i) for navigation %" PRIu64 " '%s'", processIdentifier(), processForNavigation->processIdentifier(), navigation->navigationID(), navigation->loggingString());
        } else
            RELEASE_LOG_IF_ALLOWED(ProcessSwapping, "decidePolicyForNavigationAction: keep using process %i for navigation, reason: %{public}s", processIdentifier(), reason.utf8().data());

        if (shouldProcessSwap) {
            // FIXME: Architecturally we do not currently support multiple WebPage's with the same ID in a given WebProcess.
            // In the case where the destination WebProcess has a SuspendedPageProxy for this WebPage, we should have thrown
            // it away to support WebProcess re-use.
            ASSERT(destinationSuspendedPage || !process().processPool().hasSuspendedPageFor(processForNavigation, *this));

            auto suspendedPage = destinationSuspendedPage ? process().processPool().takeSuspendedPage(*destinationSuspendedPage) : nullptr;
            if (suspendedPage && suspendedPage->pageIsClosedOrClosing())
                suspendedPage = nullptr;

            continueNavigationInNewProcess(navigation, WTFMove(suspendedPage), WTFMove(processForNavigation), processSwapRequestedByClient, WTFMove(data));
        }

        receivedPolicyDecision(policyAction, navigation.ptr(), shouldProcessSwap ? WTF::nullopt : WTFMove(data), WTFMove(sender), shouldProcessSwap ? WillContinueLoadInNewProcess::Yes : WillContinueLoadInNewProcess::No);
    });
}

void WebPageProxy::receivedPolicyDecision(PolicyAction action, API::Navigation* navigation, Optional<WebsitePoliciesData>&& websitePolicies, Ref<PolicyDecisionSender>&& sender, WillContinueLoadInNewProcess willContinueLoadInNewProcess)
{
    if (!hasRunningProcess()) {
        sender->send(PolicyAction::Ignore, 0, DownloadID(), WTF::nullopt);
        return;
    }

    auto transaction = m_pageLoadState.transaction();

    if (action == PolicyAction::Ignore && willContinueLoadInNewProcess == WillContinueLoadInNewProcess::No && navigation && navigation->navigationID() == m_pageLoadState.pendingAPIRequest().navigationID)
        m_pageLoadState.clearPendingAPIRequest(transaction);

    DownloadID downloadID = { };
    if (action == PolicyAction::Download) {
        // Create a download proxy.
        auto& download = m_process->processPool().createDownloadProxy(m_decidePolicyForResponseRequest, this);
        if (navigation) {
            download.setWasUserInitiated(navigation->wasUserInitiated());
            download.setRedirectChain(navigation->takeRedirectChain());
        }

        downloadID = download.downloadID();
        handleDownloadRequest(download);
        m_decidePolicyForResponseRequest = { };
    }

    sender->send(action, navigation ? navigation->navigationID() : 0, downloadID, WTFMove(websitePolicies));
}

void WebPageProxy::commitProvisionalPage(FrameIdentifier frameID, uint64_t navigationID, const String& mimeType, bool frameHasCustomContentProvider, uint32_t frameLoadType, const WebCore::CertificateInfo& certificateInfo, bool containsPluginDocument, Optional<WebCore::HasInsecureContent> forcedHasInsecureContent, const UserData& userData)
{
    ASSERT(m_provisionalPage);
    RELEASE_LOG_IF_ALLOWED(Loading, "commitProvisionalPage: previousPID = %i, newPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_provisionalPage->process().processIdentifier(), m_pageID.toUInt64());

    Optional<FrameIdentifier> mainFrameIDInPreviousProcess = m_mainFrame ? makeOptional(m_mainFrame->frameID()) : WTF::nullopt;

    ASSERT(m_process.ptr() != &m_provisionalPage->process());

    auto shouldDelayClosingUntilEnteringAcceleratedCompositingMode = ShouldDelayClosingUntilEnteringAcceleratedCompositingMode::No;
#if PLATFORM(MAC)
    // On macOS, when not using UI-side compositing, we need to make sure we do not close the page in the previous process until we've
    // entered accelerated compositing for the new page or we will flash on navigation.
    if (drawingArea()->type() == DrawingAreaTypeTiledCoreAnimation)
        shouldDelayClosingUntilEnteringAcceleratedCompositingMode = ShouldDelayClosingUntilEnteringAcceleratedCompositingMode::Yes;
#endif

    processDidTerminate(ProcessTerminationReason::NavigationSwap);

    m_process->removeMessageReceiver(Messages::WebPageProxy::messageReceiverName(), m_pageID);
    auto* navigation = navigationState().navigation(m_provisionalPage->navigationID());
    bool didSuspendPreviousPage = navigation ? suspendCurrentPageIfPossible(*navigation, mainFrameIDInPreviousProcess, m_provisionalPage->processSwapRequestedByClient(), shouldDelayClosingUntilEnteringAcceleratedCompositingMode) : false;
    m_process->removeWebPage(*this, m_websiteDataStore.ptr() == &m_provisionalPage->process().websiteDataStore() ? WebProcessProxy::EndsUsingDataStore::No : WebProcessProxy::EndsUsingDataStore::Yes);

    // There is no way we'll be able to return to the page in the previous page so close it.
    if (!didSuspendPreviousPage)
        m_process->send(Messages::WebPage::Close(), pageID());

    swapToWebProcess(m_provisionalPage->process(), m_provisionalPage->takeDrawingArea(), m_provisionalPage->mainFrame());

#if PLATFORM(COCOA)
    auto accessibilityToken = m_provisionalPage->takeAccessibilityToken();
    if (!accessibilityToken.isEmpty())
        registerWebProcessAccessibilityToken({ accessibilityToken.data(), accessibilityToken.size() });
#endif

    didCommitLoadForFrame(frameID, navigationID, mimeType, frameHasCustomContentProvider, frameLoadType, certificateInfo, containsPluginDocument, forcedHasInsecureContent, userData);

    m_provisionalPage = nullptr;
}

void WebPageProxy::continueNavigationInNewProcess(API::Navigation& navigation, std::unique_ptr<SuspendedPageProxy>&& suspendedPageProxy, Ref<WebProcessProxy>&& newProcess, ProcessSwapRequestedByClient processSwapRequestedByClient, Optional<WebsitePoliciesData>&& websitePolicies)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "continueNavigationInNewProcess: webPID = %i, pageID = %" PRIu64 ", newProcessPID = %i, hasSuspendedPage = %i", m_process->processIdentifier(), m_pageID.toUInt64(), newProcess->processIdentifier(), !!suspendedPageProxy);
    LOG(Loading, "Continuing navigation %" PRIu64 " '%s' in a new web process", navigation.navigationID(), navigation.loggingString());

    if (m_provisionalPage) {
        RELEASE_LOG_IF_ALLOWED(ProcessSwapping, "continueNavigationInNewProcess: There is already a pending provisional load, cancelling it (provisonalNavigationID: %llu, navigationID: %llu)", m_provisionalPage->navigationID(), navigation.navigationID());
        if (m_provisionalPage->navigationID() != navigation.navigationID())
            m_provisionalPage->cancel();
        m_provisionalPage = nullptr;
    }

    m_provisionalPage = makeUnique<ProvisionalPageProxy>(*this, newProcess.copyRef(), WTFMove(suspendedPageProxy), navigation.navigationID(), navigation.currentRequestIsRedirect(), navigation.currentRequest(), processSwapRequestedByClient);

    if (auto* item = navigation.targetItem()) {
        LOG(Loading, "WebPageProxy %p continueNavigationInNewProcess to back item URL %s", this, item->url().utf8().data());

        auto transaction = m_pageLoadState.transaction();
        m_pageLoadState.setPendingAPIRequest(transaction, { navigation.navigationID(), item->url() });

        m_provisionalPage->goToBackForwardItem(navigation, *item, WTFMove(websitePolicies));
        return;
    }

    if (m_backForwardList->currentItem() && (navigation.lockBackForwardList() == LockBackForwardList::Yes || navigation.lockHistory() == LockHistory::Yes)) {
        // If WebCore is supposed to lock the history for this load, then the new process needs to know about the current history item so it can update
        // it instead of creating a new one.
        newProcess->send(Messages::WebPage::SetCurrentHistoryItemForReattach(m_backForwardList->currentItem()->itemState()), m_pageID);
    }

    // FIXME: Work out timing of responding with the last policy delegate, etc
    ASSERT(!navigation.currentRequest().isEmpty());
    if (auto& substituteData = navigation.substituteData())
        m_provisionalPage->loadData(navigation, { substituteData->content.data(), substituteData->content.size() }, substituteData->MIMEType, substituteData->encoding, substituteData->baseURL, substituteData->userData.get(), WTFMove(websitePolicies));
    else
        m_provisionalPage->loadRequest(navigation, ResourceRequest { navigation.currentRequest() }, WebCore::ShouldOpenExternalURLsPolicy::ShouldAllowExternalSchemes, nullptr, WTFMove(websitePolicies));
}

bool WebPageProxy::isPageOpenedByDOMShowingInitialEmptyDocument() const
{
    return openedByDOM() && !hasCommittedAnyProvisionalLoads();
}

// MSVC gives a redeclaration error if noreturn is used on the definition and not the declaration, while
// Cocoa tests segfault if it is moved to the declaration site (even if we move the definition with it!).
#if !COMPILER(MSVC)
NO_RETURN_DUE_TO_ASSERT
#endif
void WebPageProxy::didFailToSuspendAfterProcessSwap()
{
    // Only the SuspendedPageProxy should be getting this call.
    ASSERT_NOT_REACHED();
}

#if !COMPILER(MSVC)
NO_RETURN_DUE_TO_ASSERT
#endif
void WebPageProxy::didSuspendAfterProcessSwap()
{
    // Only the SuspendedPageProxy should be getting this call.
    ASSERT_NOT_REACHED();
}

void WebPageProxy::setUserAgent(String&& userAgent)
{
    if (m_userAgent == userAgent)
        return;
    m_userAgent = WTFMove(userAgent);

#if ENABLE(SERVICE_WORKER)
    // We update the service worker there at the moment to be sure we use values used by actual web pages.
    // FIXME: Refactor this when we have a better User-Agent story.
    process().processPool().updateServiceWorkerUserAgent(m_userAgent);
#endif

    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::SetUserAgent(m_userAgent), m_pageID);
}

void WebPageProxy::setApplicationNameForUserAgent(const String& applicationName)
{
    if (m_applicationNameForUserAgent == applicationName)
        return;

    m_applicationNameForUserAgent = applicationName;
    if (!m_customUserAgent.isEmpty())
        return;

    setUserAgent(standardUserAgent(m_applicationNameForUserAgent));
}

void WebPageProxy::setCustomUserAgent(const String& customUserAgent)
{
    if (m_customUserAgent == customUserAgent)
        return;

    m_customUserAgent = customUserAgent;

    if (m_customUserAgent.isEmpty()) {
        setUserAgent(standardUserAgent(m_applicationNameForUserAgent));
        return;
    }

    setUserAgent(String { m_customUserAgent });
}

void WebPageProxy::resumeActiveDOMObjectsAndAnimations()
{
    if (!hasRunningProcess() || !m_isPageSuspended)
        return;

    m_isPageSuspended = false;

    m_process->send(Messages::WebPage::ResumeActiveDOMObjectsAndAnimations(), m_pageID);
}

void WebPageProxy::suspendActiveDOMObjectsAndAnimations()
{
    if (!hasRunningProcess() || m_isPageSuspended)
        return;

    m_isPageSuspended = true;

    m_process->send(Messages::WebPage::SuspendActiveDOMObjectsAndAnimations(), m_pageID);
}

bool WebPageProxy::supportsTextEncoding() const
{
    // FIXME (118840): We should probably only support this for text documents, not all non-image documents.
    return m_mainFrame && !m_mainFrame->isDisplayingStandaloneImageDocument();
}

void WebPageProxy::setCustomTextEncodingName(const String& encodingName)
{
    if (m_customTextEncodingName == encodingName)
        return;
    m_customTextEncodingName = encodingName;

    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::SetCustomTextEncodingName(encodingName), m_pageID);
}

SessionState WebPageProxy::sessionState(WTF::Function<bool (WebBackForwardListItem&)>&& filter) const
{
    SessionState sessionState;

    sessionState.backForwardListState = m_backForwardList->backForwardListState(WTFMove(filter));

    String provisionalURLString = m_pageLoadState.pendingAPIRequestURL();
    if (provisionalURLString.isEmpty())
        provisionalURLString = m_pageLoadState.provisionalURL();

    if (!provisionalURLString.isEmpty())
        sessionState.provisionalURL = URL(URL(), provisionalURLString);

    sessionState.renderTreeSize = renderTreeSize();
    return sessionState;
}

RefPtr<API::Navigation> WebPageProxy::restoreFromSessionState(SessionState sessionState, bool navigate)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "restoreFromSessionState: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    m_sessionRestorationRenderTreeSize = 0;
    m_hitRenderTreeSizeThreshold = false;

    bool hasBackForwardList = !!sessionState.backForwardListState.currentIndex;

    if (hasBackForwardList) {
        // If there isn't a running process yet the RestoreSession message below is just ignored, and
        // session is restored when the web process is created via creation parameters which is not
        // considered an API request. So, we launch the initial process here before restoring the
        // session to ensure the session is restored in the web process via RestoreSession IPC message
        // which is considered an API request. See https://bugs.webkit.org/show_bug.cgi?id=198561.
        launchInitialProcessIfNecessary();

        m_backForwardList->restoreFromState(WTFMove(sessionState.backForwardListState));
        process().send(Messages::WebPage::RestoreSession(m_backForwardList->itemStates()), m_pageID);

        auto transaction = m_pageLoadState.transaction();
        m_pageLoadState.setCanGoBack(transaction, m_backForwardList->backItem());
        m_pageLoadState.setCanGoForward(transaction, m_backForwardList->forwardItem());

        // The back / forward list was restored from a sessionState so we don't want to snapshot the current
        // page when navigating away. Suppress navigation snapshotting until the next load has committed
        suppressNextAutomaticNavigationSnapshot();
    }

    // FIXME: Navigating should be separate from state restoration.
    if (navigate) {
        m_sessionRestorationRenderTreeSize = sessionState.renderTreeSize;
        if (!m_sessionRestorationRenderTreeSize)
            m_hitRenderTreeSizeThreshold = true; // If we didn't get data on renderTreeSize, just don't fire the milestone.

        if (!sessionState.provisionalURL.isNull())
            return loadRequest(sessionState.provisionalURL);

        if (hasBackForwardList) {
            if (WebBackForwardListItem* item = m_backForwardList->currentItem())
                return goToBackForwardItem(*item);
        }
    }

    return nullptr;
}

bool WebPageProxy::supportsTextZoom() const
{
    // FIXME (118840): This should also return false for standalone media and plug-in documents.
    if (!m_mainFrame || m_mainFrame->isDisplayingStandaloneImageDocument())
        return false;

    return true;
}
 
void WebPageProxy::setTextZoomFactor(double zoomFactor)
{
    if (m_textZoomFactor == zoomFactor)
        return;

    m_textZoomFactor = zoomFactor;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetTextZoomFactor(m_textZoomFactor), m_pageID); 
}

void WebPageProxy::setPageZoomFactor(double zoomFactor)
{
    if (m_pageZoomFactor == zoomFactor)
        return;

    closeOverlayedViews();

    m_pageZoomFactor = zoomFactor;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetPageZoomFactor(m_pageZoomFactor), m_pageID); 
}

void WebPageProxy::setPageAndTextZoomFactors(double pageZoomFactor, double textZoomFactor)
{
    if (m_pageZoomFactor == pageZoomFactor && m_textZoomFactor == textZoomFactor)
        return;

    closeOverlayedViews();

    m_pageZoomFactor = pageZoomFactor;
    m_textZoomFactor = textZoomFactor;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetPageAndTextZoomFactors(m_pageZoomFactor, m_textZoomFactor), m_pageID); 
}

double WebPageProxy::pageZoomFactor() const
{
    // Zoom factor for non-PDF pages persists across page loads. We maintain a separate member variable for PDF
    // zoom which ensures that we don't use the PDF zoom for a normal page.
    if (m_mainFramePluginHandlesPageScaleGesture)
        return m_pluginZoomFactor;
    return m_pageZoomFactor;
}

double WebPageProxy::pageScaleFactor() const
{
    // PDF documents use zoom and scale factors to size themselves appropriately in the window. We store them
    // separately but decide which to return based on the main frame.
    if (m_mainFramePluginHandlesPageScaleGesture)
        return m_pluginScaleFactor;
    return m_pageScaleFactor;
}

void WebPageProxy::scalePage(double scale, const IntPoint& origin)
{
    ASSERT(scale > 0);

    m_pageScaleFactor = scale;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::ScalePage(scale, origin), m_pageID);
}

void WebPageProxy::scalePageInViewCoordinates(double scale, const IntPoint& centerInViewCoordinates)
{
    ASSERT(scale > 0);

    m_pageScaleFactor = scale;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::ScalePageInViewCoordinates(scale, centerInViewCoordinates), m_pageID);
}

void WebPageProxy::scaleView(double scale)
{
    ASSERT(scale > 0);

    m_viewScaleFactor = scale;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::ScaleView(scale), m_pageID);
}

void WebPageProxy::setIntrinsicDeviceScaleFactor(float scaleFactor)
{
    if (m_intrinsicDeviceScaleFactor == scaleFactor)
        return;

    m_intrinsicDeviceScaleFactor = scaleFactor;

    if (m_drawingArea)
        m_drawingArea->deviceScaleFactorDidChange();
}

void WebPageProxy::windowScreenDidChange(PlatformDisplayID displayID)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::WindowScreenDidChange(displayID), m_pageID);
}

float WebPageProxy::deviceScaleFactor() const
{
    return m_customDeviceScaleFactor.valueOr(m_intrinsicDeviceScaleFactor);
}

void WebPageProxy::setCustomDeviceScaleFactor(float customScaleFactor)
{
    // FIXME: Remove this once we bump cairo requirements to support HiDPI.
    // https://bugs.webkit.org/show_bug.cgi?id=133378
#if USE(CAIRO) && !HAVE(CAIRO_SURFACE_SET_DEVICE_SCALE)
    return;
#endif

    if (m_customDeviceScaleFactor && m_customDeviceScaleFactor.value() == customScaleFactor)
        return;

    float oldScaleFactor = deviceScaleFactor();

    // A value of 0 clears the customScaleFactor.
    if (customScaleFactor)
        m_customDeviceScaleFactor = customScaleFactor;
    else
        m_customDeviceScaleFactor = WTF::nullopt;

    if (!hasRunningProcess())
        return;

    if (deviceScaleFactor() != oldScaleFactor)
        m_drawingArea->deviceScaleFactorDidChange();
}

void WebPageProxy::accessibilitySettingsDidChange()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::AccessibilitySettingsDidChange(), m_pageID);
}

void WebPageProxy::setUseFixedLayout(bool fixed)
{
    // This check is fine as the value is initialized in the web
    // process as part of the creation parameters.
    if (fixed == m_useFixedLayout)
        return;

    m_useFixedLayout = fixed;
    if (!fixed)
        m_fixedLayoutSize = IntSize();

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetUseFixedLayout(fixed), m_pageID);
}

void WebPageProxy::setFixedLayoutSize(const IntSize& size)
{
    if (size == m_fixedLayoutSize)
        return;

    m_fixedLayoutSize = size;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetFixedLayoutSize(size), m_pageID);
}

void WebPageProxy::setAlwaysShowsHorizontalScroller(bool alwaysShowsHorizontalScroller)
{
    if (alwaysShowsHorizontalScroller == m_alwaysShowsHorizontalScroller)
        return;

    m_alwaysShowsHorizontalScroller = alwaysShowsHorizontalScroller;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetAlwaysShowsHorizontalScroller(alwaysShowsHorizontalScroller), m_pageID);
}

void WebPageProxy::setAlwaysShowsVerticalScroller(bool alwaysShowsVerticalScroller)
{
    if (alwaysShowsVerticalScroller == m_alwaysShowsVerticalScroller)
        return;

    m_alwaysShowsVerticalScroller = alwaysShowsVerticalScroller;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetAlwaysShowsVerticalScroller(alwaysShowsVerticalScroller), m_pageID);
}

void WebPageProxy::listenForLayoutMilestones(OptionSet<WebCore::LayoutMilestone> milestones)
{    
    if (milestones == m_observedLayoutMilestones)
        return;

    m_observedLayoutMilestones = milestones;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::ListenForLayoutMilestones(milestones), m_pageID);
}

void WebPageProxy::setSuppressScrollbarAnimations(bool suppressAnimations)
{
    if (suppressAnimations == m_suppressScrollbarAnimations)
        return;

    m_suppressScrollbarAnimations = suppressAnimations;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetSuppressScrollbarAnimations(suppressAnimations), m_pageID);
}

bool WebPageProxy::rubberBandsAtLeft() const
{
    return m_rubberBandsAtLeft;
}

void WebPageProxy::setRubberBandsAtLeft(bool rubberBandsAtLeft)
{
    m_rubberBandsAtLeft = rubberBandsAtLeft;
}

bool WebPageProxy::rubberBandsAtRight() const
{
    return m_rubberBandsAtRight;
}

void WebPageProxy::setRubberBandsAtRight(bool rubberBandsAtRight)
{
    m_rubberBandsAtRight = rubberBandsAtRight;
}

bool WebPageProxy::rubberBandsAtTop() const
{
    return m_rubberBandsAtTop;
}

void WebPageProxy::setRubberBandsAtTop(bool rubberBandsAtTop)
{
    m_rubberBandsAtTop = rubberBandsAtTop;
}

bool WebPageProxy::rubberBandsAtBottom() const
{
    return m_rubberBandsAtBottom;
}

void WebPageProxy::setRubberBandsAtBottom(bool rubberBandsAtBottom)
{
    m_rubberBandsAtBottom = rubberBandsAtBottom;
}
    
void WebPageProxy::setEnableVerticalRubberBanding(bool enableVerticalRubberBanding)
{
    if (enableVerticalRubberBanding == m_enableVerticalRubberBanding)
        return;

    m_enableVerticalRubberBanding = enableVerticalRubberBanding;

    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::SetEnableVerticalRubberBanding(enableVerticalRubberBanding), m_pageID);
}
    
bool WebPageProxy::verticalRubberBandingIsEnabled() const
{
    return m_enableVerticalRubberBanding;
}
    
void WebPageProxy::setEnableHorizontalRubberBanding(bool enableHorizontalRubberBanding)
{
    if (enableHorizontalRubberBanding == m_enableHorizontalRubberBanding)
        return;

    m_enableHorizontalRubberBanding = enableHorizontalRubberBanding;

    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::SetEnableHorizontalRubberBanding(enableHorizontalRubberBanding), m_pageID);
}
    
bool WebPageProxy::horizontalRubberBandingIsEnabled() const
{
    return m_enableHorizontalRubberBanding;
}

void WebPageProxy::setBackgroundExtendsBeyondPage(bool backgroundExtendsBeyondPage)
{
    if (backgroundExtendsBeyondPage == m_backgroundExtendsBeyondPage)
        return;

    m_backgroundExtendsBeyondPage = backgroundExtendsBeyondPage;

    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::SetBackgroundExtendsBeyondPage(backgroundExtendsBeyondPage), m_pageID);
}

bool WebPageProxy::backgroundExtendsBeyondPage() const
{
    return m_backgroundExtendsBeyondPage;
}

void WebPageProxy::setPaginationMode(WebCore::Pagination::Mode mode)
{
    if (mode == m_paginationMode)
        return;

    m_paginationMode = mode;

    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::SetPaginationMode(mode), m_pageID);
}

void WebPageProxy::setPaginationBehavesLikeColumns(bool behavesLikeColumns)
{
    if (behavesLikeColumns == m_paginationBehavesLikeColumns)
        return;

    m_paginationBehavesLikeColumns = behavesLikeColumns;

    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::SetPaginationBehavesLikeColumns(behavesLikeColumns), m_pageID);
}

void WebPageProxy::setPageLength(double pageLength)
{
    if (pageLength == m_pageLength)
        return;

    m_pageLength = pageLength;

    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::SetPageLength(pageLength), m_pageID);
}

void WebPageProxy::setGapBetweenPages(double gap)
{
    if (gap == m_gapBetweenPages)
        return;

    m_gapBetweenPages = gap;

    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::SetGapBetweenPages(gap), m_pageID);
}

void WebPageProxy::setPaginationLineGridEnabled(bool lineGridEnabled)
{
    if (lineGridEnabled == m_paginationLineGridEnabled)
        return;
    
    m_paginationLineGridEnabled = lineGridEnabled;
    
    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::SetPaginationLineGridEnabled(lineGridEnabled), m_pageID);
}

void WebPageProxy::pageScaleFactorDidChange(double scaleFactor)
{
    m_pageScaleFactor = scaleFactor;
}

void WebPageProxy::pluginScaleFactorDidChange(double pluginScaleFactor)
{
    m_pluginScaleFactor = pluginScaleFactor;
}

void WebPageProxy::pluginZoomFactorDidChange(double pluginZoomFactor)
{
    m_pluginZoomFactor = pluginZoomFactor;
}

void WebPageProxy::findStringMatches(const String& string, FindOptions options, unsigned maxMatchCount)
{
    if (string.isEmpty()) {
        didFindStringMatches(string, Vector<Vector<WebCore::IntRect>> (), 0);
        return;
    }

    m_process->send(Messages::WebPage::FindStringMatches(string, options, maxMatchCount), m_pageID);
}

void WebPageProxy::findString(const String& string, FindOptions options, unsigned maxMatchCount)
{
    m_process->send(Messages::WebPage::FindString(string, options, maxMatchCount), m_pageID);
}

void WebPageProxy::getImageForFindMatch(int32_t matchIndex)
{
    m_process->send(Messages::WebPage::GetImageForFindMatch(matchIndex), m_pageID);
}

void WebPageProxy::selectFindMatch(int32_t matchIndex)
{
    m_process->send(Messages::WebPage::SelectFindMatch(matchIndex), m_pageID);
}

void WebPageProxy::indicateFindMatch(int32_t matchIndex)
{
    m_process->send(Messages::WebPage::IndicateFindMatch(matchIndex), m_pageID);
}

void WebPageProxy::hideFindUI()
{
    m_process->send(Messages::WebPage::HideFindUI(), m_pageID);
}

void WebPageProxy::countStringMatches(const String& string, FindOptions options, unsigned maxMatchCount)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::CountStringMatches(string, options, maxMatchCount), m_pageID);
}

void WebPageProxy::replaceMatches(Vector<uint32_t>&& matchIndices, const String& replacementText, bool selectionOnly, Function<void(uint64_t, CallbackBase::Error)>&& callback)
{
    if (!hasRunningProcess()) {
        callback(0, CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callback), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::ReplaceMatches(WTFMove(matchIndices), replacementText, selectionOnly, callbackID), m_pageID);
}

void WebPageProxy::launchInitialProcessIfNecessary()
{
    if (&process() == process().processPool().dummyProcessProxy())
        launchProcess({ }, ProcessLaunchReason::InitialProcess);
}

void WebPageProxy::runJavaScriptInMainFrame(const String& script, bool forceUserGesture, WTF::Function<void (API::SerializedScriptValue*, bool hadException, const ExceptionDetails&, CallbackBase::Error)>&& callbackFunction)
{
    runJavaScriptInMainFrameScriptWorld(script, forceUserGesture, WTF::nullopt, WTFMove(callbackFunction));
}

void WebPageProxy::runJavaScriptInMainFrameScriptWorld(const String& script, bool forceUserGesture, const Optional<String>& worldName, WTF::Function<void(API::SerializedScriptValue*, bool hadException, const ExceptionDetails&, CallbackBase::Error)>&& callbackFunction)
{
    // For backward-compatibility support running script in a WebView which has not done any loads yets.
    launchInitialProcessIfNecessary();

    if (!hasRunningProcess()) {
        callbackFunction(nullptr, false, { }, CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::RunJavaScriptInMainFrameScriptWorld(script, forceUserGesture, worldName, callbackID), m_pageID);
}

void WebPageProxy::runJavaScriptInFrame(FrameIdentifier frameID, const String& script, bool forceUserGesture, WTF::Function<void(API::SerializedScriptValue*, bool hadException, const ExceptionDetails&, CallbackBase::Error)>&& callbackFunction)
{
    ASSERT(mainFrame()->frameID() != frameID);
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::RunJavaScriptInFrame(frameID, script, forceUserGesture, callbackID), m_pageID);
}

void WebPageProxy::getRenderTreeExternalRepresentation(WTF::Function<void (const String&, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(String(), CallbackBase::Error::Unknown);
        return;
    }
    
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::GetRenderTreeExternalRepresentation(callbackID), m_pageID);
}

void WebPageProxy::getSourceForFrame(WebFrameProxy* frame, WTF::Function<void (const String&, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(String(), CallbackBase::Error::Unknown);
        return;
    }
    
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_loadDependentStringCallbackIDs.add(callbackID);
    m_process->send(Messages::WebPage::GetSourceForFrame(frame->frameID(), callbackID), m_pageID);
}

void WebPageProxy::getContentsAsString(WTF::Function<void (const String&, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(String(), CallbackBase::Error::Unknown);
        return;
    }
    
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_loadDependentStringCallbackIDs.add(callbackID);
    m_process->send(Messages::WebPage::GetContentsAsString(callbackID), m_pageID);
}

#if PLATFORM(COCOA)
void WebPageProxy::getContentsAsAttributedString(CompletionHandler<void(const AttributedString&)>&& completionHandler)
{
    if (!hasRunningProcess()) {
        completionHandler(AttributedString());
        return;
    }

    m_process->connection()->sendWithAsyncReply(Messages::WebPage::GetContentsAsAttributedString(), WTFMove(completionHandler), m_pageID);
}
#endif

void WebPageProxy::getBytecodeProfile(WTF::Function<void (const String&, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(String(), CallbackBase::Error::Unknown);
        return;
    }
    
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_loadDependentStringCallbackIDs.add(callbackID);
    m_process->send(Messages::WebPage::GetBytecodeProfile(callbackID), m_pageID);
}

void WebPageProxy::getSamplingProfilerOutput(WTF::Function<void (const String&, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(String(), CallbackBase::Error::Unknown);
        return;
    }
    
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_loadDependentStringCallbackIDs.add(callbackID);
    m_process->send(Messages::WebPage::GetSamplingProfilerOutput(callbackID), m_pageID);
}

#if ENABLE(MHTML)
void WebPageProxy::getContentsAsMHTMLData(Function<void (API::Data*, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(nullptr, CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::GetContentsAsMHTMLData(callbackID), m_pageID);
}
#endif

void WebPageProxy::getSelectionOrContentsAsString(WTF::Function<void (const String&, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(String(), CallbackBase::Error::Unknown);
        return;
    }
    
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::GetSelectionOrContentsAsString(callbackID), m_pageID);
}

void WebPageProxy::getSelectionAsWebArchiveData(Function<void (API::Data*, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(nullptr, CallbackBase::Error::Unknown);
        return;
    }
    
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::GetSelectionAsWebArchiveData(callbackID), m_pageID);
}

void WebPageProxy::getMainResourceDataOfFrame(WebFrameProxy* frame, Function<void (API::Data*, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess() || !frame) {
        callbackFunction(nullptr, CallbackBase::Error::Unknown);
        return;
    }
    
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::GetMainResourceDataOfFrame(frame->frameID(), callbackID), m_pageID);
}

void WebPageProxy::getResourceDataFromFrame(WebFrameProxy* frame, API::URL* resourceURL, Function<void (API::Data*, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(nullptr, CallbackBase::Error::Unknown);
        return;
    }
    
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::GetResourceDataFromFrame(frame->frameID(), resourceURL->string(), callbackID), m_pageID);
}

void WebPageProxy::getWebArchiveOfFrame(WebFrameProxy* frame, Function<void (API::Data*, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(nullptr, CallbackBase::Error::Unknown);
        return;
    }
    
    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::GetWebArchiveOfFrame(frame->frameID(), callbackID), m_pageID);
}

void WebPageProxy::forceRepaint(RefPtr<VoidCallback>&& callback)
{
    if (!hasRunningProcess()) {
        // FIXME: If the page is invalid we should not call the callback. It'd be better to just return false from forceRepaint.
        callback->invalidate(CallbackBase::Error::OwnerWasInvalidated);
        return;
    }

    Function<void(CallbackBase::Error)> didForceRepaintCallback = [this, callback = WTFMove(callback)](CallbackBase::Error error) mutable {
        if (error != CallbackBase::Error::None) {
            callback->invalidate(error);
            return;
        }

        if (!hasRunningProcess()) {
            callback->invalidate(CallbackBase::Error::OwnerWasInvalidated);
            return;
        }
    
        callAfterNextPresentationUpdate([callback = WTFMove(callback)](CallbackBase::Error error) {
            if (error != CallbackBase::Error::None) {
                callback->invalidate(error);
                return;
            }

            callback->performCallback();
        });
    };

    auto callbackID = m_callbacks.put(WTFMove(didForceRepaintCallback), m_process->throttler().backgroundActivityToken());
    m_drawingArea->waitForBackingStoreUpdateOnNextPaint();
    m_process->send(Messages::WebPage::ForceRepaint(callbackID), m_pageID); 
}

static OptionSet<IPC::SendOption> printingSendOptions(bool isPerformingDOMPrintOperation)
{
    if (isPerformingDOMPrintOperation)
        return IPC::SendOption::DispatchMessageEvenWhenWaitingForSyncReply;

    return { };
}

void WebPageProxy::preferencesDidChange()
{
    if (!hasRunningProcess())
        return;

    updateThrottleState();
    updateHiddenPageThrottlingAutoIncreases();

    pageClient().preferencesDidChange();

    // FIXME: It probably makes more sense to send individual preference changes.
    // However, WebKitTestRunner depends on getting a preference change notification
    // even if nothing changed in UI process, so that overrides get removed.

    // Preferences need to be updated during synchronous printing to make "print backgrounds" preference work when toggled from a print dialog checkbox.
    m_process->send(Messages::WebPage::PreferencesDidChange(preferencesStore()), m_pageID, printingSendOptions(m_isPerformingDOMPrintOperation));
}

void WebPageProxy::didCreateMainFrame(FrameIdentifier frameID)
{
    // The DecidePolicyForNavigationActionSync IPC is synchronous and may therefore get processed before the DidCreateMainFrame one.
    // When this happens, decidePolicyForNavigationActionSync() calls didCreateMainFrame() and we need to ignore the DidCreateMainFrame
    // IPC when it later gets processed.
    if (m_mainFrame && m_mainFrame->frameID() == frameID)
        return;

    PageClientProtector protector(pageClient());

    MESSAGE_CHECK(m_process, !m_mainFrame);
    MESSAGE_CHECK(m_process, m_process->canCreateFrame(frameID));

    m_mainFrame = WebFrameProxy::create(*this, frameID);

    // Add the frame to the process wide map.
    m_process->frameCreated(frameID, *m_mainFrame);
}

void WebPageProxy::didCreateSubframe(FrameIdentifier frameID)
{
    PageClientProtector protector(pageClient());

    MESSAGE_CHECK(m_process, m_mainFrame);

    // The DecidePolicyForNavigationActionSync IPC is synchronous and may therefore get processed before the DidCreateSubframe one.
    // When this happens, decidePolicyForNavigationActionSync() calls didCreateSubframe() and we need to ignore the DidCreateSubframe
    // IPC when it later gets processed.
    if (m_process->webFrame(frameID))
        return;

    MESSAGE_CHECK(m_process, m_process->canCreateFrame(frameID));
    
    auto subFrame = WebFrameProxy::create(*this, frameID);

    // Add the frame to the process wide map.
    m_process->frameCreated(frameID, subFrame.get());
}

void WebPageProxy::didCreateWindow(FrameIdentifier frameID, GlobalWindowIdentifier&& windowIdentifier)
{
}

double WebPageProxy::estimatedProgress() const
{
    return m_pageLoadState.estimatedProgress();
}

void WebPageProxy::didStartProgress()
{
    ASSERT(!m_isClosed);

    PageClientProtector protector(pageClient());

    auto transaction = m_pageLoadState.transaction();
    m_pageLoadState.didStartProgress(transaction);

    m_pageLoadState.commitChanges();
}

void WebPageProxy::didChangeProgress(double value)
{
    PageClientProtector protector(pageClient());

    auto transaction = m_pageLoadState.transaction();
    m_pageLoadState.didChangeProgress(transaction, value);

    m_pageLoadState.commitChanges();
}

void WebPageProxy::didFinishProgress()
{
    PageClientProtector protector(pageClient());

    auto transaction = m_pageLoadState.transaction();
    m_pageLoadState.didFinishProgress(transaction);

    m_pageLoadState.commitChanges();
}

void WebPageProxy::setNetworkRequestsInProgress(bool networkRequestsInProgress)
{
    auto transaction = m_pageLoadState.transaction();
    m_pageLoadState.setNetworkRequestsInProgress(transaction, networkRequestsInProgress);
}

void WebPageProxy::hasInsecureContent(CompletionHandler<void(WebCore::HasInsecureContent)>&& completionHandler)
{
    completionHandler(m_pageLoadState.committedHasInsecureContent() ? HasInsecureContent::Yes : HasInsecureContent::No);
}

void WebPageProxy::didDestroyNavigation(uint64_t navigationID)
{
    PageClientProtector protector(pageClient());

    // On process-swap, the previous process tries to destroy the navigation but the provisional process is actually taking over the navigation.
    if (m_provisionalPage && m_provisionalPage->navigationID() == navigationID)
        return;

    // FIXME: Message check the navigationID.
    m_navigationState->didDestroyNavigation(navigationID);
}

void WebPageProxy::didStartProvisionalLoadForFrame(FrameIdentifier frameID, uint64_t navigationID, URL&& url, URL&& unreachableURL, const UserData& userData)
{
    didStartProvisionalLoadForFrameShared(m_process.copyRef(), frameID, navigationID, WTFMove(url), WTFMove(unreachableURL), userData);
}

void WebPageProxy::didStartProvisionalLoadForFrameShared(Ref<WebProcessProxy>&& process, FrameIdentifier frameID, uint64_t navigationID, URL&& url, URL&& unreachableURL, const UserData& userData)
{
    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = process->webFrame(frameID);
    MESSAGE_CHECK(process, frame);
    MESSAGE_CHECK_URL(process, url);

    // If the page starts a new main frame provisional load, then cancel any pending one in a provisional process.
    if (frame->isMainFrame() && m_provisionalPage && m_provisionalPage->mainFrame() != frame) {
        m_provisionalPage->cancel();
        m_provisionalPage = nullptr;
    }

    // FIXME: We should message check that navigationID is not zero here, but it's currently zero for some navigations through the page cache.
    RefPtr<API::Navigation> navigation;
    if (frame->isMainFrame() && navigationID)
        navigation = navigationState().navigation(navigationID);

    LOG(Loading, "WebPageProxy %" PRIu64 " in process pid %i didStartProvisionalLoadForFrame to frameID %" PRIu64 ", navigationID %" PRIu64 ", url %s", m_pageID.toUInt64(), process->processIdentifier(), frameID.toUInt64(), navigationID, url.string().utf8().data());
    RELEASE_LOG_IF_ALLOWED(Loading, "didStartProvisionalLoadForFrame: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64, process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64());

    auto transaction = m_pageLoadState.transaction();
    bool fromAlternateHTMLAPI = !unreachableURL.isEmpty() && unreachableURL == m_pageLoadState.pendingAPIRequestURL();
    if (navigation || fromAlternateHTMLAPI)
        m_pageLoadState.clearPendingAPIRequest(transaction);

    if (frame->isMainFrame()) {
        process->didStartProvisionalLoadForMainFrame(url);
        reportPageLoadResult(ResourceError { ResourceError::Type::Cancellation });
        m_pageLoadStart = MonotonicTime::now();
        m_pageLoadState.didStartProvisionalLoad(transaction, url, unreachableURL);
        pageClient().didStartProvisionalLoadForMainFrame();
        closeOverlayedViews();
    }

    frame->setUnreachableURL(unreachableURL);
    frame->didStartProvisionalLoad(url);

    m_pageLoadState.commitChanges();
    if (m_loaderClient)
        m_loaderClient->didStartProvisionalLoadForFrame(*this, *frame, navigation.get(), process->transformHandlesToObjects(userData.object()).get());
    else if (frame->isMainFrame())
        m_navigationClient->didStartProvisionalNavigation(*this, navigation.get(), process->transformHandlesToObjects(userData.object()).get());
}

void WebPageProxy::didExplicitOpenForFrame(FrameIdentifier frameID, URL&& url)
{
    auto* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);
    MESSAGE_CHECK_URL(m_process, url);

    auto transaction = m_pageLoadState.transaction();

    if (frame->isMainFrame())
        m_pageLoadState.didExplicitOpen(transaction, url);

    frame->didExplicitOpen(url);

    m_hasCommittedAnyProvisionalLoads = true;
    m_process->didCommitProvisionalLoad();

    m_pageLoadState.commitChanges();
}

void WebPageProxy::didReceiveServerRedirectForProvisionalLoadForFrame(FrameIdentifier frameID, uint64_t navigationID, ResourceRequest&& request, const UserData& userData)
{
    didReceiveServerRedirectForProvisionalLoadForFrameShared(m_process.copyRef(), frameID, navigationID, WTFMove(request), userData);
}

void WebPageProxy::didReceiveServerRedirectForProvisionalLoadForFrameShared(Ref<WebProcessProxy>&& process, FrameIdentifier frameID, uint64_t navigationID, ResourceRequest&& request, const UserData& userData)
{
    LOG(Loading, "WebPageProxy::didReceiveServerRedirectForProvisionalLoadForFrame to frameID %" PRIu64 ", navigationID %" PRIu64 ", url %s", frameID.toUInt64(), navigationID, request.url().string().utf8().data());
    RELEASE_LOG_IF_ALLOWED(Loading, "didReceiveServerRedirectForProvisionalLoadForFrame: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64, process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64());

    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = process->webFrame(frameID);
    MESSAGE_CHECK(process, frame);
    MESSAGE_CHECK_URL(process, request.url());

    // FIXME: We should message check that navigationID is not zero here, but it's currently zero for some navigations through the page cache.
    RefPtr<API::Navigation> navigation = navigationID ? navigationState().navigation(navigationID) : nullptr;
    if (navigation)
        navigation->appendRedirectionURL(request.url());

    auto transaction = m_pageLoadState.transaction();

    if (frame->isMainFrame())
        m_pageLoadState.didReceiveServerRedirectForProvisionalLoad(transaction, request.url());

    frame->didReceiveServerRedirectForProvisionalLoad(request.url());

    m_pageLoadState.commitChanges();
    if (m_loaderClient)
        m_loaderClient->didReceiveServerRedirectForProvisionalLoadForFrame(*this, *frame, frame->isMainFrame() ? navigation.get() : nullptr, process->transformHandlesToObjects(userData.object()).get());
    else if (frame->isMainFrame())
        m_navigationClient->didReceiveServerRedirectForProvisionalNavigation(*this, navigation.get(), process->transformHandlesToObjects(userData.object()).get());
}

void WebPageProxy::willPerformClientRedirectForFrame(FrameIdentifier frameID, const String& url, double delay, WebCore::LockBackForwardList)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "willPerformClientRedirectForFrame: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64());

    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    if (frame->isMainFrame())
        m_navigationClient->willPerformClientRedirect(*this, url, delay);
}

void WebPageProxy::didCancelClientRedirectForFrame(FrameIdentifier frameID)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "didCancelClientRedirectForFrame: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64());

    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    if (frame->isMainFrame())
        m_navigationClient->didCancelClientRedirect(*this);
}

void WebPageProxy::didChangeProvisionalURLForFrame(FrameIdentifier frameID, uint64_t navigationID, URL&& url)
{
    didChangeProvisionalURLForFrameShared(m_process.copyRef(), frameID, navigationID, WTFMove(url));
}

void WebPageProxy::didChangeProvisionalURLForFrameShared(Ref<WebProcessProxy>&& process, FrameIdentifier frameID, uint64_t, URL&& url)
{
    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = process->webFrame(frameID);
    MESSAGE_CHECK(process, frame);
    MESSAGE_CHECK(process, frame->frameLoadState().state() == FrameLoadState::State::Provisional);
    MESSAGE_CHECK_URL(process, url);

    auto transaction = m_pageLoadState.transaction();

    // Internally, we handle this the same way we handle a server redirect. There are no client callbacks
    // for this, but if this is the main frame, clients may observe a change to the page's URL.
    if (frame->isMainFrame())
        m_pageLoadState.didReceiveServerRedirectForProvisionalLoad(transaction, url);

    frame->didReceiveServerRedirectForProvisionalLoad(url);
}

void WebPageProxy::didFailProvisionalLoadForFrame(FrameIdentifier frameID, const SecurityOriginData& frameSecurityOrigin, uint64_t navigationID, const String& provisionalURL, const ResourceError& error, WillContinueLoading willContinueLoading, const UserData& userData)
{
    if (m_provisionalPage && m_provisionalPage->navigationID() == navigationID) {
        // The load did not fail, it is merely happening in a new provisional process.
        return;
    }

    didFailProvisionalLoadForFrameShared(m_process.copyRef(), frameID, frameSecurityOrigin, navigationID, provisionalURL, error, willContinueLoading, userData);
}

void WebPageProxy::didFailProvisionalLoadForFrameShared(Ref<WebProcessProxy>&& process, FrameIdentifier frameID, const SecurityOriginData& frameSecurityOrigin, uint64_t navigationID, const String& provisionalURL, const ResourceError& error, WillContinueLoading willContinueLoading, const UserData& userData)
{
    LOG(Loading, "(Loading) WebPageProxy %" PRIu64 " in web process pid %i didFailProvisionalLoadForFrame to provisionalURL %s", m_pageID.toUInt64(), process->processIdentifier(), provisionalURL.utf8().data());
    RELEASE_LOG_IF_ALLOWED(Process, "didFailProvisionalLoadForFrame: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64 ", domain = %s, code = %d", process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64(), error.domain().utf8().data(), error.errorCode());

    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = process->webFrame(frameID);
    MESSAGE_CHECK(process, frame);

    if (m_controlledByAutomation) {
        if (auto* automationSession = process->processPool().automationSession())
            automationSession->navigationOccurredForFrame(*frame);
    }

    // FIXME: We should message check that navigationID is not zero here, but it's currently zero for some navigations through the page cache.
    RefPtr<API::Navigation> navigation;
    if (frame->isMainFrame() && navigationID)
        navigation = navigationState().takeNavigation(navigationID);

    auto transaction = m_pageLoadState.transaction();

    if (frame->isMainFrame()) {
        reportPageLoadResult(error);
        m_pageLoadState.didFailProvisionalLoad(transaction);
        pageClient().didFailProvisionalLoadForMainFrame();
    }

    frame->didFailProvisionalLoad();

    m_pageLoadState.commitChanges();

    ASSERT(!m_failingProvisionalLoadURL);
    m_failingProvisionalLoadURL = provisionalURL;

    if (m_loaderClient)
        m_loaderClient->didFailProvisionalLoadWithErrorForFrame(*this, *frame, navigation.get(), error, process->transformHandlesToObjects(userData.object()).get());
    else if (frame->isMainFrame())
        m_navigationClient->didFailProvisionalNavigationWithError(*this, *frame, navigation.get(), error, process->transformHandlesToObjects(userData.object()).get());
    else {
        // FIXME: Get the main frame's current navigation.
        m_navigationClient->didFailProvisionalLoadInSubframeWithError(*this, *frame, frameSecurityOrigin, nullptr, error, process->transformHandlesToObjects(userData.object()).get());
    }

    m_failingProvisionalLoadURL = { };

    // If the provisional page's load fails then we destroy the provisional page.
    if (m_provisionalPage && m_provisionalPage->mainFrame() == frame && willContinueLoading == WillContinueLoading::No)
        m_provisionalPage = nullptr;
}

void WebPageProxy::clearLoadDependentCallbacks()
{
    HashSet<CallbackID> loadDependentStringCallbackIDs = WTFMove(m_loadDependentStringCallbackIDs);
    for (auto& callbackID : loadDependentStringCallbackIDs) {
        if (auto callback = m_callbacks.take<StringCallback>(callbackID))
            callback->invalidate();
    }
}

#if ENABLE(RESOURCE_LOAD_STATISTICS)
static OptionSet<CrossSiteNavigationDataTransfer::Flag> checkIfNavigationContainsDataTransfer(const SecurityOriginData requesterOrigin, const ResourceRequest& currentRequest)
{
    OptionSet<CrossSiteNavigationDataTransfer::Flag> navigationDataTransfer;
    if (requesterOrigin.securityOrigin()->isUnique())
        return navigationDataTransfer;

    auto currentURL = currentRequest.url();
    if (!currentURL.query().isEmpty() || !currentURL.fragmentIdentifier().isEmpty())
        navigationDataTransfer.add(CrossSiteNavigationDataTransfer::Flag::DestinationLinkDecoration);

    URL referrerURL { URL(), currentRequest.httpReferrer() };
    if (!referrerURL.query().isEmpty() || !referrerURL.fragmentIdentifier().isEmpty())
        navigationDataTransfer.add(CrossSiteNavigationDataTransfer::Flag::ReferrerLinkDecoration);

    return navigationDataTransfer;
}
#endif

void WebPageProxy::didCommitLoadForFrame(FrameIdentifier frameID, uint64_t navigationID, const String& mimeType, bool frameHasCustomContentProvider, uint32_t opaqueFrameLoadType, const WebCore::CertificateInfo& certificateInfo, bool containsPluginDocument, Optional<HasInsecureContent> hasInsecureContent, const UserData& userData)
{
    LOG(Loading, "(Loading) WebPageProxy %" PRIu64 " didCommitLoadForFrame in navigation %" PRIu64, m_pageID.toUInt64(), navigationID);
    LOG(BackForward, "(Back/Forward) After load commit, back/forward list is now:%s", m_backForwardList->loggingString());
    RELEASE_LOG_IF_ALLOWED(Loading, "didCommitLoadForFrame: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64());

    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    // FIXME: We should message check that navigationID is not zero here, but it's currently zero for some navigations through the page cache.
    RefPtr<API::Navigation> navigation;
    if (frame->isMainFrame() && navigationID && (navigation = navigationState().navigation(navigationID))) {
#if ENABLE(RESOURCE_LOAD_STATISTICS)
        auto requesterOrigin = navigation->lastNavigationAction().requesterOrigin;
        auto currentRequest = navigation->currentRequest();
        auto navigationDataTransfer = checkIfNavigationContainsDataTransfer(requesterOrigin, currentRequest);
        if (!navigationDataTransfer.isEmpty()) {
            RegistrableDomain currentDomain { currentRequest.url() };
            URL requesterURL { URL(), requesterOrigin.toString() };
            if (!currentDomain.matches(requesterURL))
                m_process->processPool().didCommitCrossSiteLoadWithDataTransfer(m_websiteDataStore->sessionID(), RegistrableDomain { requesterURL }, currentDomain, navigationDataTransfer, m_pageID);
        }
#endif
    }

    m_hasCommittedAnyProvisionalLoads = true;
    m_process->didCommitProvisionalLoad();

#if PLATFORM(IOS_FAMILY)
    if (frame->isMainFrame()) {
        m_hasReceivedLayerTreeTransactionAfterDidCommitLoad = false;
        m_firstLayerTreeTransactionIdAfterDidCommitLoad = downcast<RemoteLayerTreeDrawingAreaProxy>(*drawingArea()).nextLayerTreeTransactionID();
    }
#endif

    auto transaction = m_pageLoadState.transaction();
    Ref<WebCertificateInfo> webCertificateInfo = WebCertificateInfo::create(certificateInfo);
    bool markPageInsecure = hasInsecureContent ? hasInsecureContent.value() == HasInsecureContent::Yes : m_treatsSHA1CertificatesAsInsecure && certificateInfo.containsNonRootSHA1SignedCertificate();

    if (frame->isMainFrame()) {
        m_pageLoadState.didCommitLoad(transaction, webCertificateInfo, markPageInsecure);
        m_shouldSuppressNextAutomaticNavigationSnapshot = false;
    } else if (markPageInsecure)
        m_pageLoadState.didDisplayOrRunInsecureContent(transaction);

#if USE(APPKIT)
    // FIXME (bug 59111): didCommitLoadForFrame comes too late when restoring a page from b/f cache, making us disable secure event mode in password fields.
    // FIXME: A load going on in one frame shouldn't affect text editing in other frames on the page.
    pageClient().resetSecureInputState();
#endif

    clearLoadDependentCallbacks();

    frame->didCommitLoad(mimeType, webCertificateInfo, containsPluginDocument);

    if (navigation && frame->isMainFrame()) {
        if (auto& adClickAttribution = navigation->adClickAttribution()) {
            if (adClickAttribution->destination().matches(frame->url()))
                m_process->processPool().sendToNetworkingProcess(Messages::NetworkProcess::StoreAdClickAttribution(m_websiteDataStore->sessionID(), *adClickAttribution));
        }
    }

    if (frame->isMainFrame()) {
        m_mainFrameHasCustomContentProvider = frameHasCustomContentProvider;

        if (m_mainFrameHasCustomContentProvider) {
            // Always assume that the main frame is pinned here, since the custom representation view will handle
            // any wheel events and dispatch them to the WKView when necessary.
            m_mainFrameIsPinnedToLeftSide = true;
            m_mainFrameIsPinnedToRightSide = true;
            m_mainFrameIsPinnedToTopSide = true;
            m_mainFrameIsPinnedToBottomSide = true;

            m_uiClient->pinnedStateDidChange(*this);
        }
        pageClient().didCommitLoadForMainFrame(mimeType, frameHasCustomContentProvider);
    }

    // Even if WebPage has the default pageScaleFactor (and therefore doesn't reset it),
    // WebPageProxy's cache of the value can get out of sync (e.g. in the case where a
    // plugin is handling page scaling itself) so we should reset it to the default
    // for standard main frame loads.
    if (frame->isMainFrame()) {
        if (static_cast<FrameLoadType>(opaqueFrameLoadType) == FrameLoadType::Standard) {
            m_pageScaleFactor = 1;
            m_pluginScaleFactor = 1;
            m_mainFramePluginHandlesPageScaleGesture = false;
        }
    }

#if ENABLE(POINTER_LOCK)
    if (frame->isMainFrame())
        requestPointerUnlock();
#endif

    m_pageLoadState.commitChanges();
    if (m_loaderClient)
        m_loaderClient->didCommitLoadForFrame(*this, *frame, navigation.get(), m_process->transformHandlesToObjects(userData.object()).get());
    else if (frame->isMainFrame())
        m_navigationClient->didCommitNavigation(*this, navigation.get(), m_process->transformHandlesToObjects(userData.object()).get());

#if ENABLE(ATTACHMENT_ELEMENT)
    if (frame->isMainFrame())
        invalidateAllAttachments();
#endif

#if ENABLE(REMOTE_INSPECTOR)
    if (frame->isMainFrame())
        remoteInspectorInformationDidChange();
#endif
}

void WebPageProxy::didFinishDocumentLoadForFrame(FrameIdentifier frameID, uint64_t navigationID, const UserData& userData)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "didFinishDocumentLoadForFrame: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64());

    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    if (m_controlledByAutomation) {
        if (auto* automationSession = process().processPool().automationSession())
            automationSession->documentLoadedForFrame(*frame);
    }

    // FIXME: We should message check that navigationID is not zero here, but it's currently zero for some navigations through the page cache.
    RefPtr<API::Navigation> navigation;
    if (frame->isMainFrame() && navigationID)
        navigation = navigationState().navigation(navigationID);

    if (frame->isMainFrame())
        m_navigationClient->didFinishDocumentLoad(*this, navigation.get(), m_process->transformHandlesToObjects(userData.object()).get());
}

void WebPageProxy::didFinishLoadForFrame(FrameIdentifier frameID, uint64_t navigationID, const UserData& userData)
{
    LOG(Loading, "WebPageProxy::didFinishLoadForFrame - WebPageProxy %p with navigationID %" PRIu64 " didFinishLoad", this, navigationID);
    RELEASE_LOG_IF_ALLOWED(Loading, "didFinishLoadForFrame: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64());

    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    // FIXME: We should message check that navigationID is not zero here, but it's currently zero for some navigations through the page cache.
    RefPtr<API::Navigation> navigation;
    if (frame->isMainFrame() && navigationID)
        navigation = navigationState().navigation(navigationID);

    auto transaction = m_pageLoadState.transaction();

    bool isMainFrame = frame->isMainFrame();
    if (isMainFrame)
        m_pageLoadState.didFinishLoad(transaction);

    if (m_controlledByAutomation) {
        if (auto* automationSession = process().processPool().automationSession())
            automationSession->navigationOccurredForFrame(*frame);
    }

    frame->didFinishLoad();

    m_pageLoadState.commitChanges();
    if (m_loaderClient)
        m_loaderClient->didFinishLoadForFrame(*this, *frame, navigation.get(), m_process->transformHandlesToObjects(userData.object()).get());
    else if (isMainFrame)
        m_navigationClient->didFinishNavigation(*this, navigation.get(), m_process->transformHandlesToObjects(userData.object()).get());

    if (isMainFrame) {
        reportPageLoadResult();
        pageClient().didFinishLoadForMainFrame();

        resetRecentCrashCountSoon();

        notifyProcessPoolToPrewarm();
    }

    m_isLoadingAlternateHTMLStringForFailingProvisionalLoad = false;
}

void WebPageProxy::didFailLoadForFrame(FrameIdentifier frameID, uint64_t navigationID, const ResourceError& error, const UserData& userData)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "didFailLoadForFrame: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64 ", domain = %s, code = %d", m_process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64(), error.domain().utf8().data(), error.errorCode());

    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    // FIXME: We should message check that navigationID is not zero here, but it's currently zero for some navigations through the page cache.
    RefPtr<API::Navigation> navigation;
    if (frame->isMainFrame() && navigationID)
        navigation = navigationState().navigation(navigationID);

    clearLoadDependentCallbacks();

    auto transaction = m_pageLoadState.transaction();

    bool isMainFrame = frame->isMainFrame();

    if (isMainFrame)
        m_pageLoadState.didFailLoad(transaction);

    if (m_controlledByAutomation) {
        if (auto* automationSession = process().processPool().automationSession())
            automationSession->navigationOccurredForFrame(*frame);
    }

    frame->didFailLoad();

    m_pageLoadState.commitChanges();
    if (m_loaderClient)
        m_loaderClient->didFailLoadWithErrorForFrame(*this, *frame, navigation.get(), error, m_process->transformHandlesToObjects(userData.object()).get());
    else if (frame->isMainFrame())
        m_navigationClient->didFailNavigationWithError(*this, *frame, navigation.get(), error, m_process->transformHandlesToObjects(userData.object()).get());

    if (isMainFrame) {
        reportPageLoadResult(error);
        pageClient().didFailLoadForMainFrame();
    }
}

void WebPageProxy::didSameDocumentNavigationForFrame(FrameIdentifier frameID, uint64_t navigationID, uint32_t opaqueSameDocumentNavigationType, URL&& url, const UserData& userData)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "didSameDocumentNavigationForFrame: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64());

    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);
    MESSAGE_CHECK_URL(m_process, url);

    // FIXME: We should message check that navigationID is not zero here, but it's currently zero for some navigations through the page cache.
    RefPtr<API::Navigation> navigation;
    if (frame->isMainFrame() && navigationID)
        navigation = navigationState().navigation(navigationID);

    auto transaction = m_pageLoadState.transaction();

    bool isMainFrame = frame->isMainFrame();
    if (isMainFrame)
        m_pageLoadState.didSameDocumentNavigation(transaction, url);

    if (m_controlledByAutomation) {
        if (auto* automationSession = process().processPool().automationSession())
            automationSession->navigationOccurredForFrame(*frame);
    }

    m_pageLoadState.clearPendingAPIRequest(transaction);
    frame->didSameDocumentNavigation(url);

    m_pageLoadState.commitChanges();

    SameDocumentNavigationType navigationType = static_cast<SameDocumentNavigationType>(opaqueSameDocumentNavigationType);
    if (isMainFrame)
        m_navigationClient->didSameDocumentNavigation(*this, navigation.get(), navigationType, m_process->transformHandlesToObjects(userData.object()).get());

    if (isMainFrame)
        pageClient().didSameDocumentNavigationForMainFrame(navigationType);
}

void WebPageProxy::didChangeMainDocument(FrameIdentifier frameID)
{
#if ENABLE(MEDIA_STREAM)
    if (m_userMediaPermissionRequestManager)
        m_userMediaPermissionRequestManager->resetAccess(frameID);
#else
    UNUSED_PARAM(frameID);
#endif
    m_isQuotaIncreaseDenied = false;
}

void WebPageProxy::viewIsBecomingVisible()
{
#if ENABLE(MEDIA_STREAM)
    if (m_userMediaPermissionRequestManager)
        m_userMediaPermissionRequestManager->viewIsBecomingVisible();
#endif
}

void WebPageProxy::didReceiveTitleForFrame(FrameIdentifier frameID, const String& title, const UserData& userData)
{
    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    auto transaction = m_pageLoadState.transaction();

    if (frame->isMainFrame())
        m_pageLoadState.setTitle(transaction, title);

    frame->didChangeTitle(title);
    
    m_pageLoadState.commitChanges();

#if ENABLE(REMOTE_INSPECTOR)
    if (frame->isMainFrame())
        remoteInspectorInformationDidChange();
#endif
}

void WebPageProxy::didFirstLayoutForFrame(FrameIdentifier, const UserData& userData)
{
}

void WebPageProxy::didFirstVisuallyNonEmptyLayoutForFrame(FrameIdentifier frameID, const UserData& userData)
{
    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    if (m_loaderClient)
        m_loaderClient->didFirstVisuallyNonEmptyLayoutForFrame(*this, *frame, m_process->transformHandlesToObjects(userData.object()).get());

    if (frame->isMainFrame())
        pageClient().didFirstVisuallyNonEmptyLayoutForMainFrame();
}

void WebPageProxy::didLayoutForCustomContentProvider()
{
    didReachLayoutMilestone({ DidFirstLayout, DidFirstVisuallyNonEmptyLayout, DidHitRelevantRepaintedObjectsAreaThreshold });
}

void WebPageProxy::didReachLayoutMilestone(OptionSet<WebCore::LayoutMilestone> layoutMilestones)
{
    PageClientProtector protector(pageClient());

    if (layoutMilestones.contains(DidFirstVisuallyNonEmptyLayout))
        pageClient().clearSafeBrowsingWarningIfForMainFrameNavigation();
    
    if (m_loaderClient)
        m_loaderClient->didReachLayoutMilestone(*this, layoutMilestones);
    m_navigationClient->renderingProgressDidChange(*this, layoutMilestones);
}

void WebPageProxy::didDisplayInsecureContentForFrame(FrameIdentifier frameID, const UserData& userData)
{
    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    auto transaction = m_pageLoadState.transaction();
    m_pageLoadState.didDisplayOrRunInsecureContent(transaction);
    m_pageLoadState.commitChanges();

    m_navigationClient->didDisplayInsecureContent(*this, m_process->transformHandlesToObjects(userData.object()).get());
}

void WebPageProxy::didRunInsecureContentForFrame(FrameIdentifier frameID, const UserData& userData)
{
    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    auto transaction = m_pageLoadState.transaction();
    m_pageLoadState.didDisplayOrRunInsecureContent(transaction);
    m_pageLoadState.commitChanges();

    m_navigationClient->didRunInsecureContent(*this, m_process->transformHandlesToObjects(userData.object()).get());
}

void WebPageProxy::didDetectXSSForFrame(FrameIdentifier, const UserData&)
{
}

void WebPageProxy::mainFramePluginHandlesPageScaleGestureDidChange(bool mainFramePluginHandlesPageScaleGesture)
{
    m_mainFramePluginHandlesPageScaleGesture = mainFramePluginHandlesPageScaleGesture;
}

void WebPageProxy::frameDidBecomeFrameSet(FrameIdentifier frameID, bool value)
{
    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    frame->setIsFrameSet(value);
    if (frame->isMainFrame())
        m_frameSetLargestFrame = value ? m_mainFrame : 0;
}

#if !PLATFORM(COCOA)
void WebPageProxy::beginSafeBrowsingCheck(const URL&, bool, WebFramePolicyListenerProxy& listener)
{
    listener.didReceiveSafeBrowsingResults({ });
}
#endif

void WebPageProxy::decidePolicyForNavigationActionAsync(FrameIdentifier frameID, WebCore::SecurityOriginData&& frameSecurityOrigin, PolicyCheckIdentifier identifier, uint64_t navigationID,
    NavigationActionData&& navigationActionData, FrameInfoData&& frameInfoData, Optional<WebCore::PageIdentifier> originatingPageID, const WebCore::ResourceRequest& originalRequest, WebCore::ResourceRequest&& request,
    IPC::FormDataReference&& requestBody, WebCore::ResourceResponse&& redirectResponse, const UserData& userData, uint64_t listenerID)
{
    decidePolicyForNavigationActionAsyncShared(m_process.copyRef(), frameID, WTFMove(frameSecurityOrigin), identifier, navigationID, WTFMove(navigationActionData),
        WTFMove(frameInfoData), originatingPageID, originalRequest, WTFMove(request), WTFMove(requestBody), WTFMove(redirectResponse), userData, listenerID);
}

void WebPageProxy::decidePolicyForNavigationActionAsyncShared(Ref<WebProcessProxy>&& process, FrameIdentifier frameID, WebCore::SecurityOriginData&& frameSecurityOrigin,
    WebCore::PolicyCheckIdentifier identifier, uint64_t navigationID, NavigationActionData&& navigationActionData, FrameInfoData&& frameInfoData, Optional<WebCore::PageIdentifier> originatingPageID,
    const WebCore::ResourceRequest& originalRequest, WebCore::ResourceRequest&& request, IPC::FormDataReference&& requestBody, WebCore::ResourceResponse&& redirectResponse,
    const UserData& userData, uint64_t listenerID)
{
    auto* frame = process->webFrame(frameID);
    MESSAGE_CHECK(process, frame);

    auto sender = PolicyDecisionSender::create(identifier, [this, protectedThis = makeRef(*this), frameID, listenerID, process = process.copyRef()] (auto... args) {
        process->send(Messages::WebPage::DidReceivePolicyDecision(frameID, listenerID, args...), m_pageID);
    });

    decidePolicyForNavigationAction(process.copyRef(), *frame, WTFMove(frameSecurityOrigin), navigationID, WTFMove(navigationActionData), WTFMove(frameInfoData), originatingPageID,
        originalRequest, WTFMove(request), WTFMove(requestBody), WTFMove(redirectResponse), userData, WTFMove(sender));
}

void WebPageProxy::decidePolicyForNavigationAction(Ref<WebProcessProxy>&& process, WebFrameProxy& frame, WebCore::SecurityOriginData&& frameSecurityOrigin, uint64_t navigationID,
    NavigationActionData&& navigationActionData, FrameInfoData&& originatingFrameInfoData, Optional<PageIdentifier> originatingPageID, const WebCore::ResourceRequest& originalRequest, WebCore::ResourceRequest&& request,
    IPC::FormDataReference&& requestBody, WebCore::ResourceResponse&& redirectResponse, const UserData& userData, Ref<PolicyDecisionSender>&& sender)
{
    LOG(Loading, "WebPageProxy::decidePolicyForNavigationAction - Original URL %s, current target URL %s", originalRequest.url().string().utf8().data(), request.url().string().utf8().data());

    PageClientProtector protector(pageClient());

    // Make the request whole again as we do not normally encode the request's body when sending it over IPC, for performance reasons.
    request.setHTTPBody(requestBody.takeData());

    auto transaction = m_pageLoadState.transaction();

    bool fromAPI = request.url() == m_pageLoadState.pendingAPIRequestURL();
    if (navigationID && !fromAPI)
        m_pageLoadState.clearPendingAPIRequest(transaction);

    if (!checkURLReceivedFromCurrentOrPreviousWebProcess(process, request.url())) {
        RELEASE_LOG_ERROR_IF_ALLOWED(Process, "Ignoring request to load this main resource because it is outside the sandbox");
        sender->send(PolicyAction::Ignore, 0, DownloadID(), WTF::nullopt);
        return;
    }

    MESSAGE_CHECK_URL(process, originalRequest.url());

    RefPtr<API::Navigation> navigation;
    if (navigationID)
        navigation = m_navigationState->navigation(navigationID);

    // When process-swapping on a redirect, the navigationActionData / originatingFrameInfoData / frameSecurityOrigin provided by the fresh new WebProcess are inaccurate since
    // the new process does not have sufficient information. To address the issue, we restore the information we stored on the NavigationAction during the original request
    // policy decision.
    if (navigationActionData.isRedirect && navigation) {
        navigationActionData = navigation->lastNavigationAction();
        navigationActionData.isRedirect = true;
        originatingFrameInfoData = navigation->originatingFrameInfo();
        frameSecurityOrigin = navigation->destinationFrameSecurityOrigin();
    }

    if (!navigation) {
        if (auto targetBackForwardItemIdentifier = navigationActionData.targetBackForwardItemIdentifier) {
            if (auto* item = m_backForwardList->itemForID(*targetBackForwardItemIdentifier)) {
                auto* fromItem = navigationActionData.sourceBackForwardItemIdentifier ? m_backForwardList->itemForID(*navigationActionData.sourceBackForwardItemIdentifier) : nullptr;
                if (!fromItem)
                    fromItem = m_backForwardList->currentItem();
                navigation = m_navigationState->createBackForwardNavigation(*item, fromItem, FrameLoadType::IndexedBackForward);
            }
        }
        if (!navigation)
            navigation = m_navigationState->createLoadRequestNavigation(ResourceRequest(request), m_backForwardList->currentItem());
    }

    navigationID = navigation->navigationID();

    // Make sure the provisional page always has the latest navigationID.
    if (m_provisionalPage && &m_provisionalPage->process() == process.ptr())
        m_provisionalPage->setNavigationID(navigationID);

    navigation->setCurrentRequest(ResourceRequest(request), process->coreProcessIdentifier());
    navigation->setLastNavigationAction(navigationActionData);
    navigation->setOriginatingFrameInfo(originatingFrameInfoData);
    navigation->setDestinationFrameSecurityOrigin(frameSecurityOrigin);

#if ENABLE(CONTENT_FILTERING)
    if (frame.didHandleContentFilterUnblockNavigation(request))
        return receivedPolicyDecision(PolicyAction::Ignore, m_navigationState->navigation(navigationID), WTF::nullopt, WTFMove(sender));
#endif

    ShouldExpectSafeBrowsingResult shouldExpectSafeBrowsingResult = ShouldExpectSafeBrowsingResult::Yes;
    if (!m_preferences->safeBrowsingEnabled())
        shouldExpectSafeBrowsingResult = ShouldExpectSafeBrowsingResult::No;

    auto listener = makeRef(frame.setUpPolicyListenerProxy([this, protectedThis = makeRef(*this), frame = makeRef(frame), sender = WTFMove(sender), navigation] (PolicyAction policyAction, API::WebsitePolicies* policies, ProcessSwapRequestedByClient processSwapRequestedByClient, RefPtr<SafeBrowsingWarning>&& safeBrowsingWarning) mutable {

        auto completionHandler = [this, protectedThis = protectedThis.copyRef(), frame = frame.copyRef(), sender = WTFMove(sender), navigation, processSwapRequestedByClient, policies = makeRefPtr(policies)] (PolicyAction policyAction) mutable {
            if (frame->isMainFrame()) {
                if (!policies) {
                    if (auto* defaultPolicies = m_configuration->defaultWebsitePolicies())
                        policies = defaultPolicies->copy();
                }
                if (policies)
                    navigation->setEffectiveContentMode(effectiveContentModeAfterAdjustingPolicies(*policies, navigation->currentRequest()));
            }
            receivedNavigationPolicyDecision(policyAction, navigation.get(), processSwapRequestedByClient, frame, policies.get(), WTFMove(sender));
        };

        if (!m_pageClient)
            return completionHandler(policyAction);

        m_pageClient->clearSafeBrowsingWarning();

        if (safeBrowsingWarning) {
            if (frame->isMainFrame() && safeBrowsingWarning->url().isValid()) {
                auto transaction = m_pageLoadState.transaction();
                m_pageLoadState.setPendingAPIRequest(transaction, { navigation->navigationID(), safeBrowsingWarning->url() });
                m_pageLoadState.commitChanges();
            }

            m_pageClient->showSafeBrowsingWarning(*safeBrowsingWarning, [protectedThis = WTFMove(protectedThis), completionHandler = WTFMove(completionHandler), policyAction] (auto&& result) mutable {
                switchOn(result, [&] (const URL& url) {
                    completionHandler(PolicyAction::Ignore);
                    protectedThis->loadRequest({ url });
                }, [&] (ContinueUnsafeLoad continueUnsafeLoad) {
                    switch (continueUnsafeLoad) {
                    case ContinueUnsafeLoad::No:
                        if (!protectedThis->hasCommittedAnyProvisionalLoads())
                            protectedThis->m_uiClient->close(protectedThis.ptr());
                        completionHandler(PolicyAction::Ignore);
                        break;
                    case ContinueUnsafeLoad::Yes:
                        completionHandler(policyAction);
                        break;
                    }
                });
            });
            m_uiClient->didShowSafeBrowsingWarning();
            return;
        }
        completionHandler(policyAction);

    }, shouldExpectSafeBrowsingResult));
    if (shouldExpectSafeBrowsingResult == ShouldExpectSafeBrowsingResult::Yes)
        beginSafeBrowsingCheck(request.url(), frame.isMainFrame(), listener);

    API::Navigation* mainFrameNavigation = frame.isMainFrame() ? navigation.get() : nullptr;
    WebFrameProxy* originatingFrame = originatingFrameInfoData.frameID ? process->webFrame(*originatingFrameInfoData.frameID) : nullptr;

#if ENABLE(RESOURCE_LOAD_STATISTICS)
    if (auto* resourceLoadStatisticsStore = websiteDataStore().resourceLoadStatistics())
        resourceLoadStatisticsStore->logFrameNavigation(frame, URL(URL(), m_pageLoadState.url()), request, redirectResponse.url());
    logFrameNavigation(frame, URL(URL(), m_pageLoadState.url()), request, redirectResponse.url());
#endif

    if (m_policyClient)
        m_policyClient->decidePolicyForNavigationAction(*this, &frame, WTFMove(navigationActionData), originatingFrame, originalRequest, WTFMove(request), WTFMove(listener), process->transformHandlesToObjects(userData.object()).get());
    else {
        auto destinationFrameInfo = API::FrameInfo::create(frame, frameSecurityOrigin.securityOrigin());
        RefPtr<API::FrameInfo> sourceFrameInfo;
        if (!fromAPI && originatingFrame == &frame)
            sourceFrameInfo = destinationFrameInfo.copyRef();
        else if (!fromAPI)
            sourceFrameInfo = API::FrameInfo::create(originatingFrameInfoData, originatingPageID ? process->webPage(*originatingPageID) : nullptr);

        auto userInitiatedActivity = process->userInitiatedActivity(navigationActionData.userGestureTokenIdentifier);
        bool shouldOpenAppLinks = !m_shouldSuppressAppLinksInNextNavigationPolicyDecision
            && destinationFrameInfo->isMainFrame()
            && (m_mainFrame ? !hostsAreEqual(m_mainFrame->url(), request.url()) : false)
            && navigationActionData.navigationType != WebCore::NavigationType::BackForward;

        auto navigationAction = API::NavigationAction::create(WTFMove(navigationActionData), sourceFrameInfo.get(), destinationFrameInfo.ptr(), WTF::nullopt, WTFMove(request), originalRequest.url(), shouldOpenAppLinks, WTFMove(userInitiatedActivity), mainFrameNavigation);

#if HAVE(APP_SSO)
        if (m_shouldSuppressSOAuthorizationInNextNavigationPolicyDecision || m_shouldSuppressSOAuthorizationInAllNavigationPolicyDecision)
            navigationAction->unsetShouldPerformSOAuthorization();
#endif

        m_navigationClient->decidePolicyForNavigationAction(*this, WTFMove(navigationAction), WTFMove(listener), process->transformHandlesToObjects(userData.object()).get());
    }

    m_shouldSuppressAppLinksInNextNavigationPolicyDecision = false;

#if HAVE(APP_SSO)
    m_shouldSuppressSOAuthorizationInNextNavigationPolicyDecision = false;
#endif
}

WebPageProxy* WebPageProxy::nonEphemeralWebPageProxy()
{
    auto processPools = WebProcessPool::allProcessPools();
    if (processPools.isEmpty())
        return nullptr;
    
    auto processPool = processPools[0];
    if (!processPool)
        return nullptr;
    
    for (auto& webProcess : processPool->processes()) {
        for (auto& page : webProcess->pages()) {
            if (page->sessionID().isEphemeral())
                continue;
            return page;
        }
    }
    return nullptr;
}

#if ENABLE(RESOURCE_LOAD_STATISTICS)
void WebPageProxy::logFrameNavigation(const WebFrameProxy& frame, const URL& pageURL, const WebCore::ResourceRequest& request, const URL& redirectURL)
{
    ASSERT(RunLoop::isMain());
    
    auto sourceURL = redirectURL;
    bool isRedirect = !redirectURL.isNull();
    if (!isRedirect) {
        sourceURL = frame.url();
        if (sourceURL.isNull())
            sourceURL = pageURL;
    }
    
    auto& targetURL = request.url();
    
    if (!targetURL.isValid() || !pageURL.isValid())
        return;
    
    auto targetHost = targetURL.host();
    auto mainFrameHost = pageURL.host();
    
    if (targetHost.isEmpty() || mainFrameHost.isEmpty() || targetHost == sourceURL.host())
        return;

    m_process->processPool().sendToNetworkingProcess(Messages::NetworkProcess::LogFrameNavigation(m_websiteDataStore->sessionID(), RegistrableDomain { targetURL }, RegistrableDomain { pageURL }, RegistrableDomain { sourceURL }, isRedirect, frame.isMainFrame()));
}
#endif

void WebPageProxy::decidePolicyForNavigationActionSync(FrameIdentifier frameID, bool isMainFrame, WebCore::SecurityOriginData&& frameSecurityOrigin, PolicyCheckIdentifier identifier,
    uint64_t navigationID, NavigationActionData&& navigationActionData, FrameInfoData&& frameInfoData, Optional<PageIdentifier> originatingPageID,
    const WebCore::ResourceRequest& originalRequest, WebCore::ResourceRequest&& request, IPC::FormDataReference&& requestBody, WebCore::ResourceResponse&& redirectResponse,
    const UserData& userData, Messages::WebPageProxy::DecidePolicyForNavigationActionSync::DelayedReply&& reply)
{
    auto* frame = m_process->webFrame(frameID);
    if (!frame) {
        // This synchronous IPC message was processed before the asynchronous DidCreateMainFrame / DidCreateSubframe one so we do not know about this frameID yet.
        if (isMainFrame)
            didCreateMainFrame(frameID);
        else
            didCreateSubframe(frameID);
    }

    decidePolicyForNavigationActionSyncShared(m_process.copyRef(), frameID, isMainFrame, WTFMove(frameSecurityOrigin), identifier, navigationID, WTFMove(navigationActionData),
        WTFMove(frameInfoData), originatingPageID, originalRequest, WTFMove(request), WTFMove(requestBody), WTFMove(redirectResponse), userData, WTFMove(reply));
}

void WebPageProxy::decidePolicyForNavigationActionSyncShared(Ref<WebProcessProxy>&& process, FrameIdentifier frameID, bool isMainFrame, WebCore::SecurityOriginData&& frameSecurityOrigin, PolicyCheckIdentifier identifier,
    uint64_t navigationID, NavigationActionData&& navigationActionData, FrameInfoData&& frameInfoData, Optional<PageIdentifier> originatingPageID,
    const WebCore::ResourceRequest& originalRequest, WebCore::ResourceRequest&& request, IPC::FormDataReference&& requestBody, WebCore::ResourceResponse&& redirectResponse,
    const UserData& userData, Messages::WebPageProxy::DecidePolicyForNavigationActionSync::DelayedReply&& reply)
{
    auto sender = PolicyDecisionSender::create(identifier, WTFMove(reply));

    auto* frame = process->webFrame(frameID);
    MESSAGE_CHECK(process, frame);

    decidePolicyForNavigationAction(WTFMove(process), *frame, WTFMove(frameSecurityOrigin), navigationID, WTFMove(navigationActionData), WTFMove(frameInfoData),
        originatingPageID, originalRequest, WTFMove(request), WTFMove(requestBody), WTFMove(redirectResponse), userData, sender.copyRef());

    // If the client did not respond synchronously, proceed with the load.
    sender->send(PolicyAction::Use, navigationID, DownloadID(), WTF::nullopt);
}

void WebPageProxy::decidePolicyForNewWindowAction(FrameIdentifier frameID, const SecurityOriginData& frameSecurityOrigin, PolicyCheckIdentifier identifier,
    NavigationActionData&& navigationActionData, ResourceRequest&& request, const String& frameName, uint64_t listenerID, const UserData& userData)
{
    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);
    MESSAGE_CHECK_URL(m_process, request.url());

    auto listener = makeRef(frame->setUpPolicyListenerProxy([this, protectedThis = makeRef(*this), identifier, listenerID, frameID] (PolicyAction policyAction, API::WebsitePolicies*, ProcessSwapRequestedByClient processSwapRequestedByClient, RefPtr<SafeBrowsingWarning>&& safeBrowsingWarning) mutable {
        // FIXME: Assert the API::WebsitePolicies* is nullptr here once clients of WKFramePolicyListenerUseWithPolicies go away.
        RELEASE_ASSERT(processSwapRequestedByClient == ProcessSwapRequestedByClient::No);
        ASSERT_UNUSED(safeBrowsingWarning, !safeBrowsingWarning);

        auto sender = PolicyDecisionSender::create(identifier, [this, protectedThis = WTFMove(protectedThis), frameID, listenerID] (auto... args) {
            m_process->send(Messages::WebPage::DidReceivePolicyDecision(frameID, listenerID, args...), m_pageID);
        });

        receivedPolicyDecision(policyAction, nullptr, WTF::nullopt, WTFMove(sender));
    }, ShouldExpectSafeBrowsingResult::No));

    if (m_policyClient)
        m_policyClient->decidePolicyForNewWindowAction(*this, *frame, navigationActionData, request, frameName, WTFMove(listener), m_process->transformHandlesToObjects(userData.object()).get());
    else {
        RefPtr<API::FrameInfo> sourceFrameInfo;
        if (frame)
            sourceFrameInfo = API::FrameInfo::create(*frame, frameSecurityOrigin.securityOrigin());

        auto userInitiatedActivity = m_process->userInitiatedActivity(navigationActionData.userGestureTokenIdentifier);
        bool shouldOpenAppLinks = m_mainFrame ? !hostsAreEqual(m_mainFrame->url(), request.url()) : false;
        auto navigationAction = API::NavigationAction::create(WTFMove(navigationActionData), sourceFrameInfo.get(), nullptr, frameName, WTFMove(request), URL { }, shouldOpenAppLinks, WTFMove(userInitiatedActivity));

        m_navigationClient->decidePolicyForNavigationAction(*this, navigationAction.get(), WTFMove(listener), m_process->transformHandlesToObjects(userData.object()).get());

    }
        
}

void WebPageProxy::decidePolicyForResponse(FrameIdentifier frameID, const SecurityOriginData& frameSecurityOrigin, PolicyCheckIdentifier identifier,
    uint64_t navigationID, const ResourceResponse& response, const ResourceRequest& request, bool canShowMIMEType, const String& downloadAttribute, uint64_t listenerID, const UserData& userData)
{
    decidePolicyForResponseShared(m_process.copyRef(), frameID, frameSecurityOrigin, identifier, navigationID, response, request, canShowMIMEType, downloadAttribute, listenerID, userData);
}

void WebPageProxy::decidePolicyForResponseShared(Ref<WebProcessProxy>&& process, FrameIdentifier frameID, const SecurityOriginData& frameSecurityOrigin, PolicyCheckIdentifier identifier,
    uint64_t navigationID, const ResourceResponse& response, const ResourceRequest& request, bool canShowMIMEType, const String& downloadAttribute, uint64_t listenerID, const UserData& userData)
{
    PageClientProtector protector(pageClient());

    m_decidePolicyForResponseRequest = request;

    WebFrameProxy* frame = process->webFrame(frameID);
    MESSAGE_CHECK(process, frame);
    MESSAGE_CHECK_URL(process, request.url());
    MESSAGE_CHECK_URL(process, response.url());

    RefPtr<API::Navigation> navigation = navigationID ? m_navigationState->navigation(navigationID) : nullptr;
    auto listener = makeRef(frame->setUpPolicyListenerProxy([this, protectedThis = makeRef(*this), frameID, identifier, listenerID, navigation = WTFMove(navigation),
        process = process.copyRef()] (PolicyAction policyAction, API::WebsitePolicies*, ProcessSwapRequestedByClient processSwapRequestedByClient, RefPtr<SafeBrowsingWarning>&& safeBrowsingWarning) mutable {
        // FIXME: Assert the API::WebsitePolicies* is nullptr here once clients of WKFramePolicyListenerUseWithPolicies go away.
        RELEASE_ASSERT(processSwapRequestedByClient == ProcessSwapRequestedByClient::No);
        ASSERT_UNUSED(safeBrowsingWarning, !safeBrowsingWarning);

        auto sender = PolicyDecisionSender::create(identifier, [this, protectedThis = WTFMove(protectedThis), frameID, listenerID, process = WTFMove(process)] (auto... args) {
            process->send(Messages::WebPage::DidReceivePolicyDecision(frameID, listenerID, args...), m_pageID);
        });
        
        receivedPolicyDecision(policyAction, navigation.get(), WTF::nullopt, WTFMove(sender));
    }, ShouldExpectSafeBrowsingResult::No));

    if (m_policyClient)
        m_policyClient->decidePolicyForResponse(*this, *frame, response, request, canShowMIMEType, WTFMove(listener), process->transformHandlesToObjects(userData.object()).get());
    else {
        auto navigationResponse = API::NavigationResponse::create(API::FrameInfo::create(*frame, frameSecurityOrigin.securityOrigin()).get(), request, response, canShowMIMEType, downloadAttribute);
        m_navigationClient->decidePolicyForNavigationResponse(*this, WTFMove(navigationResponse), WTFMove(listener), process->transformHandlesToObjects(userData.object()).get());
    }
}

void WebPageProxy::unableToImplementPolicy(FrameIdentifier frameID, const ResourceError& error, const UserData& userData)
{
    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    if (!m_policyClient)
        return;
    m_policyClient->unableToImplementPolicy(*this, *frame, error, m_process->transformHandlesToObjects(userData.object()).get());
}

// FormClient

void WebPageProxy::willSubmitForm(FrameIdentifier frameID, FrameIdentifier sourceFrameID, const Vector<std::pair<String, String>>& textFieldValues, uint64_t listenerID, const UserData& userData)
{
    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    WebFrameProxy* sourceFrame = m_process->webFrame(sourceFrameID);
    MESSAGE_CHECK(m_process, sourceFrame);

    m_formClient->willSubmitForm(*this, *frame, *sourceFrame, textFieldValues, m_process->transformHandlesToObjects(userData.object()).get(), [this, protectedThis = makeRef(*this), frameID, listenerID]() {
        m_process->send(Messages::WebPage::ContinueWillSubmitForm(frameID, listenerID), m_pageID);
    });
}

void WebPageProxy::contentRuleListNotification(URL&& url, ContentRuleListResults&& results)
{
    m_navigationClient->contentRuleListNotification(*this, WTFMove(url), WTFMove(results));
}
    
void WebPageProxy::didNavigateWithNavigationData(const WebNavigationDataStore& store, FrameIdentifier frameID)
{
    didNavigateWithNavigationDataShared(m_process.copyRef(), store, frameID);
}

void WebPageProxy::didNavigateWithNavigationDataShared(Ref<WebProcessProxy>&& process, const WebNavigationDataStore& store, FrameIdentifier frameID)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "didNavigateWithNavigationDataShared: webPID = %i, pageID = %" PRIu64, process->processIdentifier(), m_pageID.toUInt64());

    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = process->webFrame(frameID);
    MESSAGE_CHECK(process, frame);
    MESSAGE_CHECK(process, frame->page() == this);

    if (frame->isMainFrame())
        m_historyClient->didNavigateWithNavigationData(*this, store);
    process->processPool().historyClient().didNavigateWithNavigationData(process->processPool(), *this, store, *frame);
}

void WebPageProxy::didPerformClientRedirect(const String& sourceURLString, const String& destinationURLString, FrameIdentifier frameID)
{
    didPerformClientRedirectShared(m_process.copyRef(), sourceURLString, destinationURLString, frameID);
}

void WebPageProxy::didPerformClientRedirectShared(Ref<WebProcessProxy>&& process, const String& sourceURLString, const String& destinationURLString, FrameIdentifier frameID)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "didPerformClientRedirectShared: webPID = %i, pageID = %" PRIu64 ", frameID = %" PRIu64, process->processIdentifier(), m_pageID.toUInt64(), frameID.toUInt64());

    PageClientProtector protector(pageClient());

    if (sourceURLString.isEmpty() || destinationURLString.isEmpty())
        return;
    
    WebFrameProxy* frame = process->webFrame(frameID);
    MESSAGE_CHECK(process, frame);
    MESSAGE_CHECK(process, frame->page() == this);
    MESSAGE_CHECK_URL(process, sourceURLString);
    MESSAGE_CHECK_URL(process, destinationURLString);

    if (frame->isMainFrame()) {
        m_historyClient->didPerformClientRedirect(*this, sourceURLString, destinationURLString);
        m_navigationClient->didPerformClientRedirect(*this, sourceURLString, destinationURLString);
    }
    process->processPool().historyClient().didPerformClientRedirect(process->processPool(), *this, sourceURLString, destinationURLString, *frame);
}

void WebPageProxy::didPerformServerRedirect(const String& sourceURLString, const String& destinationURLString, FrameIdentifier frameID)
{
    didPerformServerRedirectShared(m_process.copyRef(), sourceURLString, destinationURLString, frameID);
}

void WebPageProxy::didPerformServerRedirectShared(Ref<WebProcessProxy>&& process, const String& sourceURLString, const String& destinationURLString, FrameIdentifier frameID)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "didPerformServerRedirect: webPID = %i, pageID = %" PRIu64, process->processIdentifier(), m_pageID.toUInt64());

    PageClientProtector protector(pageClient());

    if (sourceURLString.isEmpty() || destinationURLString.isEmpty())
        return;
    
    WebFrameProxy* frame = process->webFrame(frameID);
    MESSAGE_CHECK(process, frame);
    MESSAGE_CHECK(process, frame->page() == this);

    MESSAGE_CHECK_URL(process, sourceURLString);
    MESSAGE_CHECK_URL(process, destinationURLString);

    if (frame->isMainFrame())
        m_historyClient->didPerformServerRedirect(*this, sourceURLString, destinationURLString);
    process->processPool().historyClient().didPerformServerRedirect(process->processPool(), *this, sourceURLString, destinationURLString, *frame);
}

void WebPageProxy::didUpdateHistoryTitle(const String& title, const String& url, FrameIdentifier frameID)
{
    PageClientProtector protector(pageClient());

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);
    MESSAGE_CHECK(m_process, frame->page() == this);

    MESSAGE_CHECK_URL(m_process, url);

    if (frame->isMainFrame())
        m_historyClient->didUpdateHistoryTitle(*this, title, url);
    process().processPool().historyClient().didUpdateHistoryTitle(process().processPool(), *this, title, url, *frame);
}

// UIClient

using NewPageCallback = CompletionHandler<void(RefPtr<WebPageProxy>&&)>;
using UIClientCallback = Function<void(Ref<API::NavigationAction>&&, NewPageCallback&&)>;
static void trySOAuthorization(Ref<API::NavigationAction>&& navigationAction, WebPageProxy& page, NewPageCallback&& newPageCallback, UIClientCallback&& uiClientCallback)
{
#if HAVE(APP_SSO)
    page.websiteDataStore().soAuthorizationCoordinator().tryAuthorize(WTFMove(navigationAction), page, WTFMove(newPageCallback), WTFMove(uiClientCallback));
#else
    ASSERT_UNUSED(page, page.pageID());
    uiClientCallback(WTFMove(navigationAction), WTFMove(newPageCallback));
#endif
}

void WebPageProxy::createNewPage(const FrameInfoData& originatingFrameInfoData, Optional<PageIdentifier> originatingPageID, ResourceRequest&& request, WindowFeatures&& windowFeatures, NavigationActionData&& navigationActionData, Messages::WebPageProxy::CreateNewPage::DelayedReply&& reply)
{
    MESSAGE_CHECK(m_process, originatingFrameInfoData.frameID);
    MESSAGE_CHECK(m_process, m_process->webFrame(*originatingFrameInfoData.frameID));

    auto originatingFrameInfo = API::FrameInfo::create(originatingFrameInfoData, m_process->webPage(*originatingPageID));
    auto mainFrameURL = m_mainFrame ? m_mainFrame->url() : URL();
    auto completionHandler = [this, protectedThis = makeRef(*this), mainFrameURL, request, reply = WTFMove(reply)] (RefPtr<WebPageProxy> newPage) mutable {
        if (!newPage) {
            reply(WTF::nullopt, WTF::nullopt);
            return;
        }

        newPage->setOpenedByDOM();

        reply(newPage->pageID(), newPage->creationParameters(m_process, *newPage->drawingArea()));

        newPage->m_shouldSuppressAppLinksInNextNavigationPolicyDecision = hostsAreEqual(URL({ }, mainFrameURL), request.url());

#if HAVE(APP_SSO)
        newPage->m_shouldSuppressSOAuthorizationInNextNavigationPolicyDecision = true;
#endif
    };

    auto userInitiatedActivity = m_process->userInitiatedActivity(navigationActionData.userGestureTokenIdentifier);
    bool shouldOpenAppLinks = !hostsAreEqual(originatingFrameInfo->request().url(), request.url());
    auto navigationAction = API::NavigationAction::create(WTFMove(navigationActionData), originatingFrameInfo.ptr(), nullptr, WTF::nullopt, WTFMove(request), URL(), shouldOpenAppLinks, WTFMove(userInitiatedActivity));

    trySOAuthorization(WTFMove(navigationAction), *this, WTFMove(completionHandler), [this, protectedThis = makeRef(*this), windowFeatures = WTFMove(windowFeatures)] (Ref<API::NavigationAction>&& navigationAction, CompletionHandler<void(RefPtr<WebPageProxy>&&)>&& completionHandler) mutable {
        m_uiClient->createNewPage(*this, WTFMove(windowFeatures), WTFMove(navigationAction), WTFMove(completionHandler));
    });
}
    
void WebPageProxy::showPage()
{
    m_uiClient->showPage(this);
}

void WebPageProxy::exitFullscreenImmediately()
{
#if ENABLE(FULLSCREEN_API)
    if (fullScreenManager())
        fullScreenManager()->close();
#endif

#if (PLATFORM(IOS_FAMILY) && HAVE(AVKIT)) || (PLATFORM(MAC) && ENABLE(VIDEO_PRESENTATION_MODE))
    if (videoFullscreenManager())
        videoFullscreenManager()->requestHideAndExitFullscreen();
#endif
}

void WebPageProxy::fullscreenMayReturnToInline()
{
    m_uiClient->fullscreenMayReturnToInline(this);
}

void WebPageProxy::didEnterFullscreen()
{
    m_uiClient->didEnterFullscreen(this);
}

void WebPageProxy::didExitFullscreen()
{
    m_uiClient->didExitFullscreen(this);
}

void WebPageProxy::closePage(bool stopResponsivenessTimer)
{
    if (stopResponsivenessTimer)
        m_process->responsivenessTimer().stop();

    pageClient().clearAllEditCommands();
    m_uiClient->close(this);
}

void WebPageProxy::runJavaScriptAlert(FrameIdentifier frameID, const SecurityOriginData& securityOrigin, const String& message, Messages::WebPageProxy::RunJavaScriptAlert::DelayedReply&& reply)
{
    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

#if PLATFORM(IOS_FAMILY)
    exitFullscreenImmediately();
#endif

    // Since runJavaScriptAlert() can spin a nested run loop we need to turn off the responsiveness timer.
    m_process->responsivenessTimer().stop();

    if (m_controlledByAutomation) {
        if (auto* automationSession = process().processPool().automationSession())
            automationSession->willShowJavaScriptDialog(*this);
    }
    m_uiClient->runJavaScriptAlert(this, message, frame, securityOrigin, WTFMove(reply));
}

void WebPageProxy::runJavaScriptConfirm(FrameIdentifier frameID, const SecurityOriginData& securityOrigin, const String& message, Messages::WebPageProxy::RunJavaScriptConfirm::DelayedReply&& reply)
{
    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

#if PLATFORM(IOS_FAMILY)
    exitFullscreenImmediately();
#endif

    // Since runJavaScriptConfirm() can spin a nested run loop we need to turn off the responsiveness timer.
    m_process->responsivenessTimer().stop();

    if (m_controlledByAutomation) {
        if (auto* automationSession = process().processPool().automationSession())
            automationSession->willShowJavaScriptDialog(*this);
    }

    m_uiClient->runJavaScriptConfirm(this, message, frame, securityOrigin, WTFMove(reply));
}

void WebPageProxy::runJavaScriptPrompt(FrameIdentifier frameID, const SecurityOriginData& securityOrigin, const String& message, const String& defaultValue, Messages::WebPageProxy::RunJavaScriptPrompt::DelayedReply&& reply)
{
    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

#if PLATFORM(IOS_FAMILY)
    exitFullscreenImmediately();
#endif
    // Since runJavaScriptPrompt() can spin a nested run loop we need to turn off the responsiveness timer.
    m_process->responsivenessTimer().stop();

    if (m_controlledByAutomation) {
        if (auto* automationSession = process().processPool().automationSession())
            automationSession->willShowJavaScriptDialog(*this);
    }

    m_uiClient->runJavaScriptPrompt(this, message, defaultValue, frame, securityOrigin, WTFMove(reply));
}

void WebPageProxy::setStatusText(const String& text)
{
    m_uiClient->setStatusText(this, text);
}

void WebPageProxy::mouseDidMoveOverElement(WebHitTestResultData&& hitTestResultData, uint32_t opaqueModifiers, UserData&& userData)
{
    m_lastMouseMoveHitTestResult = API::HitTestResult::create(hitTestResultData);
    auto modifiers = OptionSet<WebEvent::Modifier>::fromRaw(opaqueModifiers);
    m_uiClient->mouseDidMoveOverElement(*this, hitTestResultData, modifiers, m_process->transformHandlesToObjects(userData.object()).get());
}

#if ENABLE(NETSCAPE_PLUGIN_API)
void WebPageProxy::unavailablePluginButtonClicked(uint32_t opaquePluginUnavailabilityReason, const String& mimeType, const String& pluginURLString, const String& pluginspageAttributeURLString, const String& frameURLString, const String& pageURLString)
{
    MESSAGE_CHECK_URL(m_process, pluginURLString);
    MESSAGE_CHECK_URL(m_process, pluginspageAttributeURLString);
    MESSAGE_CHECK_URL(m_process, frameURLString);
    MESSAGE_CHECK_URL(m_process, pageURLString);

    String newMimeType = mimeType;
    PluginModuleInfo plugin = m_process->processPool().pluginInfoStore().findPlugin(newMimeType, URL(URL(), pluginURLString));
    auto pluginInformation = createPluginInformationDictionary(plugin, frameURLString, mimeType, pageURLString, pluginspageAttributeURLString, pluginURLString);

    WKPluginUnavailabilityReason pluginUnavailabilityReason = kWKPluginUnavailabilityReasonPluginMissing;
    switch (static_cast<RenderEmbeddedObject::PluginUnavailabilityReason>(opaquePluginUnavailabilityReason)) {
    case RenderEmbeddedObject::PluginMissing:
        pluginUnavailabilityReason = kWKPluginUnavailabilityReasonPluginMissing;
        break;
    case RenderEmbeddedObject::InsecurePluginVersion:
        pluginUnavailabilityReason = kWKPluginUnavailabilityReasonInsecurePluginVersion;
        break;
    case RenderEmbeddedObject::PluginCrashed:
        pluginUnavailabilityReason = kWKPluginUnavailabilityReasonPluginCrashed;
        break;
    case RenderEmbeddedObject::PluginBlockedByContentSecurityPolicy:
    case RenderEmbeddedObject::UnsupportedPlugin:
    case RenderEmbeddedObject::PluginTooSmall:
        ASSERT_NOT_REACHED();
    }

    m_uiClient->unavailablePluginButtonClicked(*this, pluginUnavailabilityReason, pluginInformation.get());
}
#endif // ENABLE(NETSCAPE_PLUGIN_API)

#if ENABLE(WEBGL)
void WebPageProxy::webGLPolicyForURL(URL&& url, Messages::WebPageProxy::WebGLPolicyForURL::DelayedReply&& reply)
{
    m_navigationClient->webGLLoadPolicy(*this, url, [reply = WTFMove(reply)] (WebGLLoadPolicy policy) mutable {
        reply(static_cast<uint32_t>(policy));
    });
}

void WebPageProxy::resolveWebGLPolicyForURL(URL&& url, Messages::WebPageProxy::ResolveWebGLPolicyForURL::DelayedReply&& reply)
{
    m_navigationClient->resolveWebGLLoadPolicy(*this, url, [reply = WTFMove(reply)] (WebGLLoadPolicy policy) mutable {
        reply(static_cast<uint32_t>(policy));
    });
}
#endif // ENABLE(WEBGL)

void WebPageProxy::setToolbarsAreVisible(bool toolbarsAreVisible)
{
    m_uiClient->setToolbarsAreVisible(*this, toolbarsAreVisible);
}

void WebPageProxy::getToolbarsAreVisible(Messages::WebPageProxy::GetToolbarsAreVisible::DelayedReply&& reply)
{
    m_uiClient->toolbarsAreVisible(*this, WTFMove(reply));
}

void WebPageProxy::setMenuBarIsVisible(bool menuBarIsVisible)
{
    m_uiClient->setMenuBarIsVisible(*this, menuBarIsVisible);
}

void WebPageProxy::getMenuBarIsVisible(Messages::WebPageProxy::GetMenuBarIsVisible::DelayedReply&& reply)
{
    m_uiClient->menuBarIsVisible(*this, WTFMove(reply));
}

void WebPageProxy::setStatusBarIsVisible(bool statusBarIsVisible)
{
    m_uiClient->setStatusBarIsVisible(*this, statusBarIsVisible);
}

void WebPageProxy::getStatusBarIsVisible(Messages::WebPageProxy::GetStatusBarIsVisible::DelayedReply&& reply)
{
    m_uiClient->statusBarIsVisible(*this, WTFMove(reply));
}

void WebPageProxy::setIsResizable(bool isResizable)
{
    m_uiClient->setIsResizable(*this, isResizable);
}

void WebPageProxy::setWindowFrame(const FloatRect& newWindowFrame)
{
    m_uiClient->setWindowFrame(*this, pageClient().convertToDeviceSpace(newWindowFrame));
}

void WebPageProxy::getWindowFrame(Messages::WebPageProxy::GetWindowFrame::DelayedReply&& reply)
{
    m_uiClient->windowFrame(*this, [this, protectedThis = makeRef(*this), reply = WTFMove(reply)] (FloatRect frame) mutable {
        reply(pageClient().convertToUserSpace(frame));
    });
}

void WebPageProxy::getWindowFrameWithCallback(Function<void(FloatRect)>&& completionHandler)
{
    m_uiClient->windowFrame(*this, [this, protectedThis = makeRef(*this), completionHandler = WTFMove(completionHandler)] (FloatRect frame) {
        completionHandler(pageClient().convertToUserSpace(frame));
    });
}

void WebPageProxy::screenToRootView(const IntPoint& screenPoint, Messages::WebPageProxy::ScreenToRootView::DelayedReply&& reply)
{
    reply(pageClient().screenToRootView(screenPoint));
}
    
void WebPageProxy::rootViewToScreen(const IntRect& viewRect, Messages::WebPageProxy::RootViewToScreen::DelayedReply&& reply)
{
    reply(pageClient().rootViewToScreen(viewRect));
}

IntRect WebPageProxy::syncRootViewToScreen(const IntRect& viewRect)
{
    return pageClient().rootViewToScreen(viewRect);
}

void WebPageProxy::accessibilityScreenToRootView(const IntPoint& screenPoint, CompletionHandler<void(IntPoint)>&& completionHandler)
{
    completionHandler(pageClient().accessibilityScreenToRootView(screenPoint));
}

void WebPageProxy::rootViewToAccessibilityScreen(const IntRect& viewRect, CompletionHandler<void(IntRect)>&& completionHandler)
{
    completionHandler(pageClient().rootViewToAccessibilityScreen(viewRect));
}

void WebPageProxy::runBeforeUnloadConfirmPanel(FrameIdentifier frameID, const SecurityOriginData& securityOrigin, const String& message, Messages::WebPageProxy::RunBeforeUnloadConfirmPanel::DelayedReply&& reply)
{
    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    // Per §18 User Prompts in the WebDriver spec, "User prompts that are spawned from beforeunload
    // event handlers, are dismissed implicitly upon navigation or close window, regardless of the
    // defined user prompt handler." So, always allow the unload to proceed if the page is being automated.
    if (m_controlledByAutomation) {
        if (!!process().processPool().automationSession()) {
            reply(true);
            return;
        }
    }

    // Since runBeforeUnloadConfirmPanel() can spin a nested run loop we need to turn off the responsiveness timer.
    m_process->responsivenessTimer().stop();
    m_uiClient->runBeforeUnloadConfirmPanel(this, message, frame, securityOrigin, WTFMove(reply));
}

void WebPageProxy::didChangeViewportProperties(const ViewportAttributes& attr)
{
    pageClient().didChangeViewportProperties(attr);
}

void WebPageProxy::pageDidScroll()
{
    m_uiClient->pageDidScroll(this);

#if PLATFORM(IOS_FAMILY)
    // Do not hide the validation message if the scrolling was caused by the keyboard showing up.
    if (m_isKeyboardAnimatingIn)
        return;
#endif

#if !PLATFORM(IOS_FAMILY)
    closeOverlayedViews();
#endif
}

void WebPageProxy::runOpenPanel(FrameIdentifier frameID, const SecurityOriginData& frameSecurityOrigin, const FileChooserSettings& settings)
{
    if (m_openPanelResultListener) {
        m_openPanelResultListener->invalidate();
        m_openPanelResultListener = nullptr;
    }

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    Ref<API::OpenPanelParameters> parameters = API::OpenPanelParameters::create(settings);
    m_openPanelResultListener = WebOpenPanelResultListenerProxy::create(this);

    if (m_controlledByAutomation) {
        if (auto* automationSession = process().processPool().automationSession())
            automationSession->handleRunOpenPanel(*this, *frame, parameters.get(), *m_openPanelResultListener);

        // Don't show a file chooser, since automation will be unable to interact with it.
        return;
    }

    // Since runOpenPanel() can spin a nested run loop we need to turn off the responsiveness timer.
    m_process->responsivenessTimer().stop();

    if (!m_uiClient->runOpenPanel(this, frame, frameSecurityOrigin, parameters.ptr(), m_openPanelResultListener.get())) {
        if (!pageClient().handleRunOpenPanel(this, frame, parameters.ptr(), m_openPanelResultListener.get()))
            didCancelForOpenPanel();
    }
}

void WebPageProxy::showShareSheet(const ShareDataWithParsedURL& shareData, ShareSheetCallbackID callbackID)
{
    CompletionHandler<void(bool)> completionHandler = [this, protectedThis = makeRef(*this), callbackID] (bool access) {
        m_process->send(Messages::WebPage::DidCompleteShareSheet(access, callbackID), m_pageID);
    };
    
    pageClient().showShareSheet(shareData, WTFMove(completionHandler));
}
    
void WebPageProxy::printFrame(FrameIdentifier frameID, CompletionHandler<void()>&& completionHandler)
{
    ASSERT(!m_isPerformingDOMPrintOperation);
    m_isPerformingDOMPrintOperation = true;

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    m_uiClient->printFrame(*this, *frame);

    endPrinting(); // Send a message synchronously while m_isPerformingDOMPrintOperation is still true.
    m_isPerformingDOMPrintOperation = false;

    completionHandler();
}

void WebPageProxy::setMediaVolume(float volume)
{
    if (volume == m_mediaVolume)
        return;
    
    m_mediaVolume = volume;
    
    if (!hasRunningProcess())
        return;
    
    m_process->send(Messages::WebPage::SetMediaVolume(volume), m_pageID);    
}

void WebPageProxy::setMuted(WebCore::MediaProducer::MutedStateFlags state)
{
    m_mutedState = state;

    if (!hasRunningProcess())
        return;

#if ENABLE(MEDIA_STREAM)
    bool hasMutedCaptureStreams = m_mediaState & WebCore::MediaProducer::MutedCaptureMask;
    if (hasMutedCaptureStreams && !(state & WebCore::MediaProducer::MediaStreamCaptureIsMuted))
        UserMediaProcessManager::singleton().muteCaptureMediaStreamsExceptIn(*this);
#endif

    m_process->send(Messages::WebPage::SetMuted(state), m_pageID);
    activityStateDidChange({ ActivityState::IsAudible, ActivityState::IsCapturingMedia });
}

void WebPageProxy::setMediaCaptureEnabled(bool enabled)
{
    m_mediaCaptureEnabled = enabled;

    if (!hasRunningProcess())
        return;

#if ENABLE(MEDIA_STREAM)
    UserMediaProcessManager::singleton().setCaptureEnabled(enabled);
#endif
}

void WebPageProxy::stopMediaCapture()
{
    if (!hasRunningProcess())
        return;

#if ENABLE(MEDIA_STREAM)
    m_process->send(Messages::WebPage::StopMediaCapture(), m_pageID);
#endif
}

void WebPageProxy::stopAllMediaPlayback()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::StopAllMediaPlayback(), m_pageID);
}

void WebPageProxy::suspendAllMediaPlayback()
{
    if (m_mediaPlaybackIsSuspended)
        return;
    m_mediaPlaybackIsSuspended = true;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SuspendAllMediaPlayback(), m_pageID);
}

void WebPageProxy::resumeAllMediaPlayback()
{
    if (!m_mediaPlaybackIsSuspended)
        return;
    m_mediaPlaybackIsSuspended = false;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::ResumeAllMediaPlayback(), m_pageID);
}

#if ENABLE(MEDIA_SESSION)
void WebPageProxy::handleMediaEvent(MediaEventType eventType)
{
    if (!hasRunningProcess())
        return;
    
    m_process->send(Messages::WebPage::HandleMediaEvent(eventType), m_pageID);
}

void WebPageProxy::setVolumeOfMediaElement(double volume, uint64_t elementID)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetVolumeOfMediaElement(volume, elementID), m_pageID);
}
#endif

void WebPageProxy::setMayStartMediaWhenInWindow(bool mayStartMedia)
{
    if (mayStartMedia == m_mayStartMediaWhenInWindow)
        return;

    m_mayStartMediaWhenInWindow = mayStartMedia;

    if (!hasRunningProcess())
        return;

    process().send(Messages::WebPage::SetMayStartMediaWhenInWindow(mayStartMedia), m_pageID);
}

void WebPageProxy::handleDownloadRequest(DownloadProxy& download)
{
    pageClient().handleDownloadRequest(download);
}

void WebPageProxy::didChangeContentSize(const IntSize& size)
{
    pageClient().didChangeContentSize(size);
}

void WebPageProxy::didChangeIntrinsicContentSize(const IntSize& intrinsicContentSize)
{
#if USE(APPKIT)
    pageClient().intrinsicContentSizeDidChange(intrinsicContentSize);
#endif
}

#if ENABLE(INPUT_TYPE_COLOR)
void WebPageProxy::showColorPicker(const WebCore::Color& initialColor, const IntRect& elementRect, Vector<WebCore::Color>&& suggestions)
{
    m_colorPicker = pageClient().createColorPicker(this, initialColor, elementRect, WTFMove(suggestions));
    m_colorPicker->showColorPicker(initialColor);
}

void WebPageProxy::setColorPickerColor(const WebCore::Color& color)
{
    ASSERT(m_colorPicker);

    m_colorPicker->setSelectedColor(color);
}

void WebPageProxy::endColorPicker()
{
    if (!m_colorPicker)
        return;

    m_colorPicker->endPicker();
}

void WebPageProxy::didChooseColor(const WebCore::Color& color)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::DidChooseColor(color), m_pageID);
}

void WebPageProxy::didEndColorPicker()
{
    m_colorPicker = nullptr;
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::DidEndColorPicker(), m_pageID);
}
#endif

#if ENABLE(DATALIST_ELEMENT)

void WebPageProxy::showDataListSuggestions(WebCore::DataListSuggestionInformation&& info)
{
    if (!m_dataListSuggestionsDropdown)
        m_dataListSuggestionsDropdown = pageClient().createDataListSuggestionsDropdown(*this);

    m_dataListSuggestionsDropdown->show(WTFMove(info));
}

void WebPageProxy::handleKeydownInDataList(const String& key)
{
    if (!m_dataListSuggestionsDropdown)
        return;

    m_dataListSuggestionsDropdown->handleKeydownWithIdentifier(key);
}

void WebPageProxy::endDataListSuggestions()
{
    if (m_dataListSuggestionsDropdown)
        m_dataListSuggestionsDropdown->close();
}

void WebPageProxy::didCloseSuggestions()
{
    if (!m_dataListSuggestionsDropdown)
        return;

    m_dataListSuggestionsDropdown = nullptr;
    m_process->send(Messages::WebPage::DidCloseSuggestions(), m_pageID);
}

void WebPageProxy::didSelectOption(const String& selectedOption)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::DidSelectDataListOption(selectedOption), m_pageID);
}

#endif

WebInspectorProxy* WebPageProxy::inspector() const
{
    if (isClosed())
        return nullptr;
    return m_inspector.get();
}

#if ENABLE(FULLSCREEN_API)
WebFullScreenManagerProxy* WebPageProxy::fullScreenManager()
{
    return m_fullScreenManager.get();
}

void WebPageProxy::setFullscreenClient(std::unique_ptr<API::FullscreenClient>&& client)
{
    if (!client) {
        m_fullscreenClient = makeUnique<API::FullscreenClient>();
        return;
    }

    m_fullscreenClient = WTFMove(client);
}
#endif
    
#if (PLATFORM(IOS_FAMILY) && HAVE(AVKIT)) || (PLATFORM(MAC) && ENABLE(VIDEO_PRESENTATION_MODE))
PlaybackSessionManagerProxy* WebPageProxy::playbackSessionManager()
{
    return m_playbackSessionManager.get();
}

VideoFullscreenManagerProxy* WebPageProxy::videoFullscreenManager()
{
    return m_videoFullscreenManager.get();
}
#endif

#if PLATFORM(IOS_FAMILY)
bool WebPageProxy::allowsMediaDocumentInlinePlayback() const
{
    return m_allowsMediaDocumentInlinePlayback;
}

void WebPageProxy::setAllowsMediaDocumentInlinePlayback(bool allows)
{
    if (m_allowsMediaDocumentInlinePlayback == allows)
        return;
    m_allowsMediaDocumentInlinePlayback = allows;

    m_process->send(Messages::WebPage::SetAllowsMediaDocumentInlinePlayback(allows), m_pageID);
}
#endif

void WebPageProxy::setHasHadSelectionChangesFromUserInteraction(bool hasHadUserSelectionChanges)
{
    m_hasHadSelectionChangesFromUserInteraction = hasHadUserSelectionChanges;
}

void WebPageProxy::setIsTouchBarUpdateSupressedForHiddenContentEditable(bool ignoreTouchBarUpdate)
{
    m_isTouchBarUpdateSupressedForHiddenContentEditable = ignoreTouchBarUpdate;
}

void WebPageProxy::setIsNeverRichlyEditableForTouchBar(bool isNeverRichlyEditable)
{
    m_isNeverRichlyEditableForTouchBar = isNeverRichlyEditable;
}

void WebPageProxy::requestDOMPasteAccess(const WebCore::IntRect& elementRect, const String& originIdentifier, CompletionHandler<void(WebCore::DOMPasteAccessResponse)>&& completionHandler)
{
    m_pageClient->requestDOMPasteAccess(elementRect, originIdentifier, WTFMove(completionHandler));
}

// BackForwardList

void WebPageProxy::backForwardAddItem(BackForwardListItemState&& itemState)
{
    auto item = WebBackForwardListItem::create(WTFMove(itemState), pageID());
    item->setResourceDirectoryURL(currentResourceDirectoryURL());
    m_backForwardList->addItem(WTFMove(item));
}

void WebPageProxy::backForwardGoToItem(const BackForwardItemIdentifier& itemID, CompletionHandler<void(SandboxExtension::Handle&&)>&& completionHandler)
{
    // On process swap, we tell the previous process to ignore the load, which causes it so restore its current back forward item to its previous
    // value. Since the load is really going on in a new provisional process, we want to ignore such requests from the committed process.
    // Any real new load in the committed process would have cleared m_provisionalPage.
    if (m_provisionalPage)
        return completionHandler({ });

    SandboxExtension::Handle sandboxExtensionHandle;
    backForwardGoToItemShared(m_process.copyRef(), itemID, WTFMove(completionHandler));
}

void WebPageProxy::backForwardGoToItemShared(Ref<WebProcessProxy>&& process, const BackForwardItemIdentifier& itemID, CompletionHandler<void(SandboxExtension::Handle&&)>&& completionHandler)
{
    auto* item = m_backForwardList->itemForID(itemID);
    if (!item)
        return completionHandler({ });

    SandboxExtension::Handle sandboxExtensionHandle;
    maybeInitializeSandboxExtensionHandle(process, URL(URL(), item->url()), item->resourceDirectoryURL(),  sandboxExtensionHandle);
    m_backForwardList->goToItem(*item);
    completionHandler(WTFMove(sandboxExtensionHandle));
}

void WebPageProxy::backForwardItemAtIndex(int32_t index, CompletionHandler<void(Optional<BackForwardItemIdentifier>&&)>&& completionHandler)
{
    if (auto* item = m_backForwardList->itemAtIndex(index))
        completionHandler(item->itemID());
    else
        completionHandler(WTF::nullopt);
}

void WebPageProxy::backForwardBackListCount(CompletionHandler<void(uint32_t)>&& completionHandler)
{
    completionHandler(m_backForwardList->backListCount());
}

void WebPageProxy::backForwardForwardListCount(CompletionHandler<void(uint32_t)>&& completionHandler)
{
    completionHandler(m_backForwardList->forwardListCount());
}

void WebPageProxy::compositionWasCanceled()
{
#if PLATFORM(COCOA)
    pageClient().notifyInputContextAboutDiscardedComposition();
#endif
}

// Undo management

void WebPageProxy::registerEditCommandForUndo(WebUndoStepID commandID, const String& label)
{
    registerEditCommand(WebEditCommandProxy::create(commandID, label, *this), UndoOrRedo::Undo);
}
    
void WebPageProxy::registerInsertionUndoGrouping()
{
#if USE(INSERTION_UNDO_GROUPING)
    pageClient().registerInsertionUndoGrouping();
#endif
}

void WebPageProxy::canUndoRedo(UndoOrRedo action, CompletionHandler<void(bool)>&& completionHandler)
{
    completionHandler(pageClient().canUndoRedo(action));
}

void WebPageProxy::executeUndoRedo(UndoOrRedo action, CompletionHandler<void()>&& completionHandler)
{
    pageClient().executeUndoRedo(action);
    completionHandler();
}

void WebPageProxy::clearAllEditCommands()
{
    pageClient().clearAllEditCommands();
}

void WebPageProxy::didCountStringMatches(const String& string, uint32_t matchCount)
{
    m_findClient->didCountStringMatches(this, string, matchCount);
}

void WebPageProxy::didGetImageForFindMatch(const ShareableBitmap::Handle& contentImageHandle, uint32_t matchIndex)
{
    auto bitmap = ShareableBitmap::create(contentImageHandle);
    if (!bitmap) {
        ASSERT_NOT_REACHED();
        return;
    }
    m_findMatchesClient->didGetImageForMatchResult(this, WebImage::create(bitmap.releaseNonNull()).ptr(), matchIndex);
}

void WebPageProxy::setTextIndicator(const TextIndicatorData& indicatorData, uint64_t lifetime)
{
    // FIXME: Make TextIndicatorWindow a platform-independent presentational thing ("TextIndicatorPresentation"?).
#if PLATFORM(COCOA)
    pageClient().setTextIndicator(TextIndicator::create(indicatorData), static_cast<TextIndicatorWindowLifetime>(lifetime));
#else
    ASSERT_NOT_REACHED();
#endif
}

void WebPageProxy::clearTextIndicator()
{
#if PLATFORM(COCOA)
    pageClient().clearTextIndicator(TextIndicatorWindowDismissalAnimation::FadeOut);
#else
    ASSERT_NOT_REACHED();
#endif
}

void WebPageProxy::setTextIndicatorAnimationProgress(float progress)
{
#if PLATFORM(COCOA)
    pageClient().setTextIndicatorAnimationProgress(progress);
#else
    ASSERT_NOT_REACHED();
#endif
}

void WebPageProxy::didFindString(const String& string, const Vector<WebCore::IntRect>& matchRects, uint32_t matchCount, int32_t matchIndex, bool didWrapAround)
{
    m_findClient->didFindString(this, string, matchRects, matchCount, matchIndex, didWrapAround);
}

void WebPageProxy::didFindStringMatches(const String& string, const Vector<Vector<WebCore::IntRect>>& matchRects, int32_t firstIndexAfterSelection)
{
    m_findMatchesClient->didFindStringMatches(this, string, matchRects, firstIndexAfterSelection);
}

void WebPageProxy::didFailToFindString(const String& string)
{
    m_findClient->didFailToFindString(this, string);
}

bool WebPageProxy::sendMessage(std::unique_ptr<IPC::Encoder> encoder, OptionSet<IPC::SendOption> sendOptions)
{
    return m_process->sendMessage(WTFMove(encoder), sendOptions);
}

IPC::Connection* WebPageProxy::messageSenderConnection() const
{
    return m_process->connection();
}

uint64_t WebPageProxy::messageSenderDestinationID() const
{
    return m_pageID.toUInt64();
}

void WebPageProxy::valueChangedForPopupMenu(WebPopupMenuProxy*, int32_t newSelectedIndex)
{
    m_process->send(Messages::WebPage::DidChangeSelectedIndexForActivePopupMenu(newSelectedIndex), m_pageID);
}

void WebPageProxy::setTextFromItemForPopupMenu(WebPopupMenuProxy*, int32_t index)
{
    m_process->send(Messages::WebPage::SetTextForActivePopupMenu(index), m_pageID);
}

bool WebPageProxy::isProcessingKeyboardEvents() const
{
    return !m_keyEventQueue.isEmpty();
}

bool WebPageProxy::isProcessingMouseEvents() const
{
    return !m_mouseEventQueue.isEmpty();
}

NativeWebMouseEvent* WebPageProxy::currentlyProcessedMouseDownEvent()
{
    // <https://bugs.webkit.org/show_bug.cgi?id=57904> We need to keep track of the mouse down event in the case where we
    // display a popup menu for select elements. When the user changes the selected item, we fake a mouseup event by
    // using this stored mousedown event and changing the event type. This trickery happens when WebProcess handles
    // a mousedown event that runs the default handler for HTMLSelectElement, so the triggering mousedown must be the first event.

    if (m_mouseEventQueue.isEmpty())
        return nullptr;
    
    auto& event = m_mouseEventQueue.first();
    if (event.type() != WebEvent::Type::MouseDown)
        return nullptr;

    return &event;
}

void WebPageProxy::postMessageToInjectedBundle(const String& messageName, API::Object* messageBody)
{
    // For backward-compatibility, make sure we launch the initial process if the client asks to post a message to its injected bundle before doing a load.
    launchInitialProcessIfNecessary();

    process().send(Messages::WebPage::PostInjectedBundleMessage(messageName, UserData(process().transformObjectsToHandles(messageBody).get())), m_pageID);
}

#if PLATFORM(GTK)
void WebPageProxy::failedToShowPopupMenu()
{
    m_process->send(Messages::WebPage::FailedToShowPopupMenu(), m_pageID);
}
#endif

void WebPageProxy::showPopupMenu(const IntRect& rect, uint64_t textDirection, const Vector<WebPopupItem>& items, int32_t selectedIndex, const PlatformPopupMenuData& data)
{
    if (m_activePopupMenu) {
        m_activePopupMenu->hidePopupMenu();
        m_activePopupMenu->invalidate();
        m_activePopupMenu = nullptr;
    }

    // If the page is controlled by automation, entering a nested run loop while the menu is open
    // can hang the page / WebDriver test. Since <option> elements are selected via a different
    // code path anyway, just don't show the native popup menu.
    if (auto* automationSession = process().processPool().automationSession()) {
        if (m_controlledByAutomation && automationSession->isSimulatingUserInteraction())
            return;
    }

    m_activePopupMenu = pageClient().createPopupMenuProxy(*this);

    if (!m_activePopupMenu)
        return;

    // Since showPopupMenu() can spin a nested run loop we need to turn off the responsiveness timer.
    m_process->responsivenessTimer().stop();

    // Showing a popup menu runs a nested runloop, which can handle messages that cause |this| to get closed.
    Ref<WebPageProxy> protect(*this);
    m_activePopupMenu->showPopupMenu(rect, static_cast<TextDirection>(textDirection), m_pageScaleFactor, items, data, selectedIndex);
}

void WebPageProxy::hidePopupMenu()
{
    if (!m_activePopupMenu)
        return;

    m_activePopupMenu->hidePopupMenu();
    m_activePopupMenu->invalidate();
    m_activePopupMenu = nullptr;
}

#if ENABLE(CONTEXT_MENUS)
void WebPageProxy::showContextMenu(ContextMenuContextData&& contextMenuContextData, const UserData& userData)
{
    // Showing a context menu runs a nested runloop, which can handle messages that cause |this| to get closed.
    Ref<WebPageProxy> protect(*this);

    // Discard any enqueued mouse events that have been delivered to the UIProcess whilst the WebProcess is still processing the
    // MouseDown event that triggered this ShowContextMenu message. This can happen if we take too long to enter the nested runloop.
    ASSERT(isProcessingMouseEvents());
    while (m_mouseEventQueue.size() > 1)
        m_mouseEventQueue.takeLast();

    m_activeContextMenuContextData = contextMenuContextData;

    m_activeContextMenu = pageClient().createContextMenuProxy(*this, WTFMove(contextMenuContextData), userData);

    // Since showContextMenu() can spin a nested run loop we need to turn off the responsiveness timer.
    m_process->responsivenessTimer().stop();

    // m_activeContextMenu might get cleared if WebPageProxy code is re-entered from the menu runloop or delegates.
    Ref<WebContextMenuProxy> protector(*m_activeContextMenu);
    m_activeContextMenu->show();

    // No matter the result of internalShowContextMenu, always notify the WebProcess that the menu is hidden so it starts handling mouse events again.
    m_process->send(Messages::WebPage::ContextMenuHidden(), m_pageID);
}

void WebPageProxy::contextMenuItemSelected(const WebContextMenuItemData& item)
{
    // Application custom items don't need to round-trip through to WebCore in the WebProcess.
    if (item.action() >= ContextMenuItemBaseApplicationTag) {
        m_contextMenuClient->customContextMenuItemSelected(*this, item);
        return;
    }

#if PLATFORM(COCOA)
    if (item.action() == ContextMenuItemTagSmartCopyPaste) {
        setSmartInsertDeleteEnabled(!isSmartInsertDeleteEnabled());
        return;
    }
    if (item.action() == ContextMenuItemTagSmartQuotes) {
        TextChecker::setAutomaticQuoteSubstitutionEnabled(!TextChecker::state().isAutomaticQuoteSubstitutionEnabled);
        m_process->updateTextCheckerState();
        return;
    }
    if (item.action() == ContextMenuItemTagSmartDashes) {
        TextChecker::setAutomaticDashSubstitutionEnabled(!TextChecker::state().isAutomaticDashSubstitutionEnabled);
        m_process->updateTextCheckerState();
        return;
    }
    if (item.action() == ContextMenuItemTagSmartLinks) {
        TextChecker::setAutomaticLinkDetectionEnabled(!TextChecker::state().isAutomaticLinkDetectionEnabled);
        m_process->updateTextCheckerState();
        return;
    }
    if (item.action() == ContextMenuItemTagTextReplacement) {
        TextChecker::setAutomaticTextReplacementEnabled(!TextChecker::state().isAutomaticTextReplacementEnabled);
        m_process->updateTextCheckerState();
        return;
    }
    if (item.action() == ContextMenuItemTagCorrectSpellingAutomatically) {
        TextChecker::setAutomaticSpellingCorrectionEnabled(!TextChecker::state().isAutomaticSpellingCorrectionEnabled);
        m_process->updateTextCheckerState();
        return;        
    }
    if (item.action() == ContextMenuItemTagShowSubstitutions) {
        TextChecker::toggleSubstitutionsPanelIsShowing();
        return;
    }
#endif
    if (item.action() == ContextMenuItemTagDownloadImageToDisk) {
        m_process->processPool().download(this, URL(URL(), m_activeContextMenuContextData.webHitTestResultData().absoluteImageURL));
        return;    
    }
    if (item.action() == ContextMenuItemTagDownloadLinkToDisk) {
        auto& hitTestResult = m_activeContextMenuContextData.webHitTestResultData();
        m_process->processPool().download(this, URL(URL(), hitTestResult.absoluteLinkURL), hitTestResult.linkSuggestedFilename);
        return;
    }
    if (item.action() == ContextMenuItemTagDownloadMediaToDisk) {
        m_process->processPool().download(this, URL(URL(), m_activeContextMenuContextData.webHitTestResultData().absoluteMediaURL));
        return;
    }
    if (item.action() == ContextMenuItemTagCheckSpellingWhileTyping) {
        TextChecker::setContinuousSpellCheckingEnabled(!TextChecker::state().isContinuousSpellCheckingEnabled);
        m_process->updateTextCheckerState();
        return;
    }
    if (item.action() == ContextMenuItemTagCheckGrammarWithSpelling) {
        TextChecker::setGrammarCheckingEnabled(!TextChecker::state().isGrammarCheckingEnabled);
        m_process->updateTextCheckerState();
        return;
    }
    if (item.action() == ContextMenuItemTagShowSpellingPanel) {
        if (!TextChecker::spellingUIIsShowing())
            advanceToNextMisspelling(true);
        TextChecker::toggleSpellingUIIsShowing();
        return;
    }
    if (item.action() == ContextMenuItemTagLearnSpelling || item.action() == ContextMenuItemTagIgnoreSpelling)
        ++m_pendingLearnOrIgnoreWordMessageCount;

    m_process->send(Messages::WebPage::DidSelectItemFromActiveContextMenu(item), m_pageID);
}

void WebPageProxy::handleContextMenuKeyEvent()
{
    m_process->send(Messages::WebPage::ContextMenuForKeyEvent(), m_pageID);
}
#endif // ENABLE(CONTEXT_MENUS)

#if PLATFORM(IOS_FAMILY)
void WebPageProxy::didChooseFilesForOpenPanelWithDisplayStringAndIcon(const Vector<String>& fileURLs, const String& displayString, const API::Data* iconData)
{
    if (!hasRunningProcess())
        return;

#if ENABLE(SANDBOX_EXTENSIONS)
    SandboxExtension::HandleArray sandboxExtensionHandles;
    sandboxExtensionHandles.allocate(fileURLs.size());
    for (size_t i = 0; i < fileURLs.size(); ++i)
        SandboxExtension::createHandle(fileURLs[i], SandboxExtension::Type::ReadOnly, sandboxExtensionHandles[i]);

    m_process->send(Messages::WebPage::ExtendSandboxForFilesFromOpenPanel(sandboxExtensionHandles), m_pageID);
#endif

    m_process->send(Messages::WebPage::DidChooseFilesForOpenPanelWithDisplayStringAndIcon(fileURLs, displayString, iconData ? iconData->dataReference() : IPC::DataReference()), m_pageID);

    m_openPanelResultListener->invalidate();
    m_openPanelResultListener = nullptr;
}
#endif

void WebPageProxy::didChooseFilesForOpenPanel(const Vector<String>& fileURLs)
{
    if (!hasRunningProcess())
        return;

#if ENABLE(SANDBOX_EXTENSIONS)
    SandboxExtension::HandleArray sandboxExtensionHandles;
    sandboxExtensionHandles.allocate(fileURLs.size());
    for (size_t i = 0; i < fileURLs.size(); ++i) {
        bool createdExtension = SandboxExtension::createHandle(fileURLs[i], SandboxExtension::Type::ReadOnly, sandboxExtensionHandles[i]);
        if (!createdExtension) {
            // This can legitimately fail if a directory containing the file is deleted after the file was chosen.
            // We also have reports of cases where this likely fails for some unknown reason, <rdar://problem/10156710>.
            WTFLogAlways("WebPageProxy::didChooseFilesForOpenPanel: could not create a sandbox extension for '%s'\n", fileURLs[i].utf8().data());
            continue;
        }
    }

    m_process->send(Messages::WebPage::ExtendSandboxForFilesFromOpenPanel(sandboxExtensionHandles), m_pageID);
#endif

    m_process->send(Messages::WebPage::DidChooseFilesForOpenPanel(fileURLs), m_pageID);

    m_openPanelResultListener->invalidate();
    m_openPanelResultListener = nullptr;
}

void WebPageProxy::didCancelForOpenPanel()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::DidCancelForOpenPanel(), m_pageID);
    
    m_openPanelResultListener->invalidate();
    m_openPanelResultListener = nullptr;
}

void WebPageProxy::advanceToNextMisspelling(bool startBeforeSelection)
{
    m_process->send(Messages::WebPage::AdvanceToNextMisspelling(startBeforeSelection), m_pageID);
}

void WebPageProxy::changeSpellingToWord(const String& word)
{
    if (word.isEmpty())
        return;

    m_process->send(Messages::WebPage::ChangeSpellingToWord(word), m_pageID);
}

void WebPageProxy::registerEditCommand(Ref<WebEditCommandProxy>&& commandProxy, UndoOrRedo undoOrRedo)
{
    pageClient().registerEditCommand(WTFMove(commandProxy), undoOrRedo);
}

void WebPageProxy::addEditCommand(WebEditCommandProxy& command)
{
    m_editCommandSet.add(&command);
}

void WebPageProxy::removeEditCommand(WebEditCommandProxy& command)
{
    m_editCommandSet.remove(&command);

    if (!hasRunningProcess())
        return;
    m_process->send(Messages::WebPage::DidRemoveEditCommand(command.commandID()), m_pageID);
}

bool WebPageProxy::canUndo()
{
    return pageClient().canUndoRedo(UndoOrRedo::Undo);
}

bool WebPageProxy::canRedo()
{
    return pageClient().canUndoRedo(UndoOrRedo::Redo);
}

SpellDocumentTag WebPageProxy::spellDocumentTag()
{
    if (!m_spellDocumentTag)
        m_spellDocumentTag = TextChecker::uniqueSpellDocumentTag(this);
    return m_spellDocumentTag.value();
}

#if USE(UNIFIED_TEXT_CHECKING)
void WebPageProxy::checkTextOfParagraph(const String& text, OptionSet<TextCheckingType> checkingTypes, int32_t insertionPoint, CompletionHandler<void(Vector<WebCore::TextCheckingResult>&&)>&& completionHandler)
{
    completionHandler(TextChecker::checkTextOfParagraph(spellDocumentTag(), text, insertionPoint, checkingTypes, m_initialCapitalizationEnabled));
}
#endif

void WebPageProxy::checkSpellingOfString(const String& text, CompletionHandler<void(int32_t misspellingLocation, int32_t misspellingLength)>&& completionHandler)
{
    int32_t misspellingLocation = 0;
    int32_t misspellingLength = 0;
    TextChecker::checkSpellingOfString(spellDocumentTag(), text, misspellingLocation, misspellingLength);
    completionHandler(misspellingLocation, misspellingLength);
}

void WebPageProxy::checkGrammarOfString(const String& text, CompletionHandler<void(Vector<WebCore::GrammarDetail>&&, int32_t badGrammarLocation, int32_t badGrammarLength)>&& completionHandler)
{
    Vector<GrammarDetail> grammarDetails;
    int32_t badGrammarLocation = 0;
    int32_t badGrammarLength = 0;
    TextChecker::checkGrammarOfString(spellDocumentTag(), text, grammarDetails, badGrammarLocation, badGrammarLength);
    completionHandler(WTFMove(grammarDetails), badGrammarLocation, badGrammarLength);
}

void WebPageProxy::spellingUIIsShowing(CompletionHandler<void(bool)>&& completionHandler)
{
    completionHandler(TextChecker::spellingUIIsShowing());
}

void WebPageProxy::updateSpellingUIWithMisspelledWord(const String& misspelledWord)
{
    TextChecker::updateSpellingUIWithMisspelledWord(spellDocumentTag(), misspelledWord);
}

void WebPageProxy::updateSpellingUIWithGrammarString(const String& badGrammarPhrase, const GrammarDetail& grammarDetail)
{
    TextChecker::updateSpellingUIWithGrammarString(spellDocumentTag(), badGrammarPhrase, grammarDetail);
}

void WebPageProxy::getGuessesForWord(const String& word, const String& context, int32_t insertionPoint, CompletionHandler<void(Vector<String>&&)>&& completionHandler)
{
    Vector<String> guesses;
    TextChecker::getGuessesForWord(spellDocumentTag(), word, context, insertionPoint, guesses, m_initialCapitalizationEnabled);
    completionHandler(WTFMove(guesses));
}

void WebPageProxy::learnWord(const String& word)
{
    MESSAGE_CHECK(m_process, m_pendingLearnOrIgnoreWordMessageCount);
    --m_pendingLearnOrIgnoreWordMessageCount;

    TextChecker::learnWord(spellDocumentTag(), word);
}

void WebPageProxy::ignoreWord(const String& word)
{
    MESSAGE_CHECK(m_process, m_pendingLearnOrIgnoreWordMessageCount);
    --m_pendingLearnOrIgnoreWordMessageCount;

    TextChecker::ignoreWord(spellDocumentTag(), word);
}

void WebPageProxy::requestCheckingOfString(uint64_t requestID, const TextCheckingRequestData& request, int32_t insertionPoint)
{
    TextChecker::requestCheckingOfString(TextCheckerCompletion::create(requestID, request, this), insertionPoint);
}

void WebPageProxy::didFinishCheckingText(uint64_t requestID, const Vector<WebCore::TextCheckingResult>& result)
{
    m_process->send(Messages::WebPage::DidFinishCheckingText(requestID, result), m_pageID);
}

void WebPageProxy::didCancelCheckingText(uint64_t requestID)
{
    m_process->send(Messages::WebPage::DidCancelCheckingText(requestID), m_pageID);
}
// Other

void WebPageProxy::setFocus(bool focused)
{
    if (focused)
        m_uiClient->focus(this);
    else
        m_uiClient->unfocus(this);
}

void WebPageProxy::takeFocus(uint32_t direction)
{
    if (m_uiClient->takeFocus(this, (static_cast<FocusDirection>(direction) == FocusDirectionForward) ? kWKFocusDirectionForward : kWKFocusDirectionBackward))
        return;

    pageClient().takeFocus(static_cast<FocusDirection>(direction));
}

void WebPageProxy::setToolTip(const String& toolTip)
{
    String oldToolTip = m_toolTip;
    m_toolTip = toolTip;
    pageClient().toolTipChanged(oldToolTip, m_toolTip);
}

void WebPageProxy::setCursor(const WebCore::Cursor& cursor)
{
    pageClient().setCursor(cursor);
}

void WebPageProxy::setCursorHiddenUntilMouseMoves(bool hiddenUntilMouseMoves)
{
    pageClient().setCursorHiddenUntilMouseMoves(hiddenUntilMouseMoves);
}

void WebPageProxy::didReceiveEvent(uint32_t opaqueType, bool handled)
{
    WebEvent::Type type = static_cast<WebEvent::Type>(opaqueType);

    switch (type) {
    case WebEvent::NoType:
    case WebEvent::MouseMove:
    case WebEvent::Wheel:
        break;

    case WebEvent::MouseDown:
    case WebEvent::MouseUp:
    case WebEvent::MouseForceChanged:
    case WebEvent::MouseForceDown:
    case WebEvent::MouseForceUp:
    case WebEvent::KeyDown:
    case WebEvent::KeyUp:
    case WebEvent::RawKeyDown:
    case WebEvent::Char:
#if ENABLE(TOUCH_EVENTS)
    case WebEvent::TouchStart:
    case WebEvent::TouchMove:
    case WebEvent::TouchEnd:
    case WebEvent::TouchCancel:
#endif
#if ENABLE(MAC_GESTURE_EVENTS)
    case WebEvent::GestureStart:
    case WebEvent::GestureChange:
    case WebEvent::GestureEnd:
#endif
        m_process->responsivenessTimer().stop();
        break;
    }

    switch (type) {
    case WebEvent::NoType:
        break;
    case WebEvent::MouseForceChanged:
    case WebEvent::MouseForceDown:
    case WebEvent::MouseForceUp:
    case WebEvent::MouseMove:
    case WebEvent::MouseDown:
    case WebEvent::MouseUp: {
        LOG(MouseHandling, "WebPageProxy::didReceiveEvent: %s (queue size %zu)", webMouseEventTypeString(type), m_mouseEventQueue.size());

        // Retire the last sent event now that WebProcess is done handling it.
        MESSAGE_CHECK(m_process, !m_mouseEventQueue.isEmpty());
        NativeWebMouseEvent event = m_mouseEventQueue.takeFirst();
        MESSAGE_CHECK(m_process, type == event.type());

        if (!m_mouseEventQueue.isEmpty()) {
            LOG(MouseHandling, " UIProcess: handling a queued mouse event from didReceiveEvent");
            processNextQueuedMouseEvent();
        } else {
            if (auto* automationSession = process().processPool().automationSession())
                automationSession->mouseEventsFlushedForPage(*this);
            pageClient().didFinishProcessingAllPendingMouseEvents();
        }

        break;
    }

    case WebEvent::Wheel: {
        MESSAGE_CHECK(m_process, !m_currentlyProcessedWheelEvents.isEmpty());

        std::unique_ptr<Vector<NativeWebWheelEvent>> oldestCoalescedEvent = m_currentlyProcessedWheelEvents.takeFirst();

        // FIXME: Dispatch additional events to the didNotHandleWheelEvent client function.
        if (!handled) {
            m_uiClient->didNotHandleWheelEvent(this, oldestCoalescedEvent->last());
            pageClient().wheelEventWasNotHandledByWebCore(oldestCoalescedEvent->last());
        }

        if (!m_wheelEventQueue.isEmpty())
            processNextQueuedWheelEvent();
        break;
    }

    case WebEvent::KeyDown:
    case WebEvent::KeyUp:
    case WebEvent::RawKeyDown:
    case WebEvent::Char: {
        LOG(KeyHandling, "WebPageProxy::didReceiveEvent: %s (queue empty %d)", webKeyboardEventTypeString(type), m_keyEventQueue.isEmpty());

        MESSAGE_CHECK(m_process, !m_keyEventQueue.isEmpty());
        NativeWebKeyboardEvent event = m_keyEventQueue.takeFirst();

        MESSAGE_CHECK(m_process, type == event.type());

        bool canProcessMoreKeyEvents = !m_keyEventQueue.isEmpty();
        if (canProcessMoreKeyEvents) {
            LOG(KeyHandling, " UI process: sent keyEvent from didReceiveEvent");
            m_process->send(Messages::WebPage::KeyEvent(m_keyEventQueue.first()), m_pageID);
        }

        // The call to doneWithKeyEvent may close this WebPage.
        // Protect against this being destroyed.
        Ref<WebPageProxy> protect(*this);

        pageClient().doneWithKeyEvent(event, handled);
        if (!handled)
            m_uiClient->didNotHandleKeyEvent(this, event);

        // Notify the session after -[NSApp sendEvent:] has a crack at turning the event into an action.
        if (!canProcessMoreKeyEvents) {
            if (auto* automationSession = process().processPool().automationSession())
                automationSession->keyboardEventsFlushedForPage(*this);
        }
        break;
    }
#if ENABLE(MAC_GESTURE_EVENTS)
    case WebEvent::GestureStart:
    case WebEvent::GestureChange:
    case WebEvent::GestureEnd: {
        MESSAGE_CHECK(m_process, !m_gestureEventQueue.isEmpty());
        NativeWebGestureEvent event = m_gestureEventQueue.takeFirst();

        MESSAGE_CHECK(m_process, type == event.type());

        if (!handled)
            pageClient().gestureEventWasNotHandledByWebCore(event);
        break;
    }
        break;
#endif
#if ENABLE(IOS_TOUCH_EVENTS)
    case WebEvent::TouchStart:
    case WebEvent::TouchMove:
    case WebEvent::TouchEnd:
    case WebEvent::TouchCancel:
        break;
#elif ENABLE(TOUCH_EVENTS)
    case WebEvent::TouchStart:
    case WebEvent::TouchMove:
    case WebEvent::TouchEnd:
    case WebEvent::TouchCancel: {
        MESSAGE_CHECK(m_process, !m_touchEventQueue.isEmpty());
        QueuedTouchEvents queuedEvents = m_touchEventQueue.takeFirst();

        MESSAGE_CHECK(m_process, type == queuedEvents.forwardedEvent.type());

        pageClient().doneWithTouchEvent(queuedEvents.forwardedEvent, handled);
        for (size_t i = 0; i < queuedEvents.deferredTouchEvents.size(); ++i) {
            bool isEventHandled = false;
            pageClient().doneWithTouchEvent(queuedEvents.deferredTouchEvents.at(i), isEventHandled);
        }
        break;
    }
#endif
    }
}

void WebPageProxy::voidCallback(CallbackID callbackID)
{
    auto callback = m_callbacks.take<VoidCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        return;
    }

    callback->performCallback();
}

void WebPageProxy::dataCallback(const IPC::DataReference& dataReference, CallbackID callbackID)
{
    auto callback = m_callbacks.take<DataCallback>(callbackID);
    if (!callback)
        return;

    callback->performCallbackWithReturnValue(API::Data::create(dataReference.data(), dataReference.size()).ptr());
}

void WebPageProxy::imageCallback(const ShareableBitmap::Handle& bitmapHandle, CallbackID callbackID)
{
    auto callback = m_callbacks.take<ImageCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        return;
    }

    callback->performCallbackWithReturnValue(bitmapHandle);
}

void WebPageProxy::stringCallback(const String& resultString, CallbackID callbackID)
{
    auto callback = m_callbacks.take<StringCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        // this can validly happen if a load invalidated the callback, though
        return;
    }

    m_loadDependentStringCallbackIDs.remove(callbackID);

    callback->performCallbackWithReturnValue(resultString.impl());
}

void WebPageProxy::invalidateStringCallback(CallbackID callbackID)
{
    auto callback = m_callbacks.take<StringCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        // this can validly happen if a load invalidated the callback, though
        return;
    }

    m_loadDependentStringCallbackIDs.remove(callbackID);

    callback->invalidate();
}

void WebPageProxy::scriptValueCallback(const IPC::DataReference& dataReference, bool hadException, const ExceptionDetails& details, CallbackID callbackID)
{
    auto callback = m_callbacks.take<ScriptValueCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        return;
    }

    if (dataReference.isEmpty()) {
        callback->performCallbackWithReturnValue(nullptr, hadException, details);
        return;
    }

    Vector<uint8_t> data;
    data.reserveInitialCapacity(dataReference.size());
    data.append(dataReference.data(), dataReference.size());

    callback->performCallbackWithReturnValue(API::SerializedScriptValue::adopt(WTFMove(data)).ptr(), hadException, details);
}

void WebPageProxy::computedPagesCallback(const Vector<IntRect>& pageRects, double totalScaleFactorForPrinting, const FloatBoxExtent& computedPageMargin, CallbackID callbackID)
{
    auto callback = m_callbacks.take<ComputedPagesCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        return;
    }

    callback->performCallbackWithReturnValue(pageRects, totalScaleFactorForPrinting, computedPageMargin);
}

void WebPageProxy::validateCommandCallback(const String& commandName, bool isEnabled, int state, CallbackID callbackID)
{
    auto callback = m_callbacks.take<ValidateCommandCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        return;
    }

    callback->performCallbackWithReturnValue(commandName.impl(), isEnabled, state);
}

void WebPageProxy::unsignedCallback(uint64_t result, CallbackID callbackID)
{
    auto callback = m_callbacks.take<UnsignedCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        // this can validly happen if a load invalidated the callback, though
        return;
    }

    callback->performCallbackWithReturnValue(result);
}

void WebPageProxy::editingRangeCallback(const EditingRange& range, CallbackID callbackID)
{
    MESSAGE_CHECK(m_process, range.isValid());

    auto callback = m_callbacks.take<EditingRangeCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        // this can validly happen if a load invalidated the callback, though
        return;
    }

    callback->performCallbackWithReturnValue(range);
}

void WebPageProxy::editorStateChanged(const EditorState& editorState)
{
    updateEditorState(editorState);
    dispatchDidReceiveEditorStateAfterFocus();
}

#if !PLATFORM(IOS_FAMILY)

void WebPageProxy::dispatchDidReceiveEditorStateAfterFocus()
{
}

#endif

#if ENABLE(APPLICATION_MANIFEST)
void WebPageProxy::applicationManifestCallback(const Optional<WebCore::ApplicationManifest>& manifestOrNull, CallbackID callbackID)
{
    auto callback = m_callbacks.take<ApplicationManifestCallback>(callbackID);
    if (!callback)
        return;

    callback->performCallbackWithReturnValue(manifestOrNull);
}
#endif

#if PLATFORM(COCOA)
void WebPageProxy::machSendRightCallback(const MachSendRight& sendRight, CallbackID callbackID)
{
    auto callback = m_callbacks.take<MachSendRightCallback>(callbackID);
    if (!callback)
        return;

    callback->performCallbackWithReturnValue(sendRight);
}
#endif

inline API::DiagnosticLoggingClient* WebPageProxy::effectiveDiagnosticLoggingClient(ShouldSample shouldSample)
{
    // Diagnostic logging is disabled for ephemeral sessions for privacy reasons.
    if (sessionID().isEphemeral())
        return nullptr;

    return DiagnosticLoggingClient::shouldLogAfterSampling(shouldSample) ? diagnosticLoggingClient() : nullptr;
}

void WebPageProxy::logDiagnosticMessage(const String& message, const String& description, WebCore::ShouldSample shouldSample)
{
    auto* effectiveClient = effectiveDiagnosticLoggingClient(shouldSample);
    if (!effectiveClient)
        return;

    effectiveClient->logDiagnosticMessage(this, message, description);
}

void WebPageProxy::logDiagnosticMessageWithResult(const String& message, const String& description, uint32_t result, WebCore::ShouldSample shouldSample)
{
    auto* effectiveClient = effectiveDiagnosticLoggingClient(shouldSample);
    if (!effectiveClient)
        return;

    effectiveClient->logDiagnosticMessageWithResult(this, message, description, static_cast<WebCore::DiagnosticLoggingResultType>(result));
}

void WebPageProxy::logDiagnosticMessageWithValue(const String& message, const String& description, double value, unsigned significantFigures, ShouldSample shouldSample)
{
    auto* effectiveClient = effectiveDiagnosticLoggingClient(shouldSample);
    if (!effectiveClient)
        return;

    effectiveClient->logDiagnosticMessageWithValue(this, message, description, String::numberToStringFixedPrecision(value, significantFigures));
}

void WebPageProxy::logDiagnosticMessageWithEnhancedPrivacy(const String& message, const String& description, ShouldSample shouldSample)
{
    auto* effectiveClient = effectiveDiagnosticLoggingClient(shouldSample);
    if (!effectiveClient)
        return;

    effectiveClient->logDiagnosticMessageWithEnhancedPrivacy(this, message, description);
}

void WebPageProxy::logDiagnosticMessageWithValueDictionary(const String& message, const String& description, const WebCore::DiagnosticLoggingClient::ValueDictionary& valueDictionary, WebCore::ShouldSample shouldSample)
{
    auto* effectiveClient = effectiveDiagnosticLoggingClient(shouldSample);
    if (!effectiveClient)
        return;

    auto apiDictionary = API::Dictionary::create();

    for (auto& keyValuePair : valueDictionary) {
        apiDictionary->add(keyValuePair.key, WTF::switchOn(keyValuePair.value,
            [](const String& value) -> Ref<Object> { return API::String::create(value); },
            [](uint64_t value) -> Ref<Object> { return API::UInt64::create(value); },
            [](int64_t value) -> Ref<Object> { return API::Int64::create(value); },
            [](bool value) -> Ref<Object> { return API::Boolean::create(value); },
            [](double value) -> Ref<Object> { return API::Double::create(value); }
        ));
    }

    effectiveClient->logDiagnosticMessageWithValueDictionary(this, message, description, WTFMove(apiDictionary));
}

void WebPageProxy::logScrollingEvent(uint32_t eventType, MonotonicTime timestamp, uint64_t data)
{
    PerformanceLoggingClient::ScrollingEvent event = static_cast<PerformanceLoggingClient::ScrollingEvent>(eventType);

    switch (event) {
    case PerformanceLoggingClient::ScrollingEvent::ExposedTilelessArea:
        WTFLogAlways("SCROLLING: Exposed tileless area. Time: %f Unfilled Pixels: %llu\n", timestamp.secondsSinceEpoch().value(), (unsigned long long)data);
        break;
    case PerformanceLoggingClient::ScrollingEvent::FilledTile:
        WTFLogAlways("SCROLLING: Filled visible fresh tile. Time: %f Unfilled Pixels: %llu\n", timestamp.secondsSinceEpoch().value(), (unsigned long long)data);
        break;
    case PerformanceLoggingClient::ScrollingEvent::SwitchedScrollingMode:
        if (data)
            WTFLogAlways("SCROLLING: Switching to main-thread scrolling mode. Time: %f Reason(s): %s\n", timestamp.secondsSinceEpoch().value(), PerformanceLoggingClient::synchronousScrollingReasonsAsString(data).utf8().data());
        else
            WTFLogAlways("SCROLLING: Switching to threaded scrolling mode. Time: %f\n", timestamp.secondsSinceEpoch().value());
        break;
    }
}

void WebPageProxy::rectForCharacterRangeCallback(const IntRect& rect, const EditingRange& actualRange, CallbackID callbackID)
{
    MESSAGE_CHECK(m_process, actualRange.isValid());

    auto callback = m_callbacks.take<RectForCharacterRangeCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        // this can validly happen if a load invalidated the callback, though
        return;
    }

    callback->performCallbackWithReturnValue(rect, actualRange);
}

#if PLATFORM(GTK)
void WebPageProxy::printFinishedCallback(const ResourceError& printError, CallbackID callbackID)
{
    auto callback = m_callbacks.take<PrintFinishedCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        return;
    }

    callback->performCallbackWithReturnValue(API::Error::create(printError).ptr());
}
#endif

void WebPageProxy::focusedFrameChanged(const Optional<FrameIdentifier>& frameID)
{
    if (!frameID) {
        m_focusedFrame = nullptr;
        return;
    }

    WebFrameProxy* frame = m_process->webFrame(*frameID);
    MESSAGE_CHECK(m_process, frame);

    m_focusedFrame = frame;
}

void WebPageProxy::frameSetLargestFrameChanged(const Optional<FrameIdentifier>& frameID)
{
    if (!frameID) {
        m_frameSetLargestFrame = nullptr;
        return;
    }

    WebFrameProxy* frame = m_process->webFrame(*frameID);
    MESSAGE_CHECK(m_process, frame);

    m_frameSetLargestFrame = frame;
}

void WebPageProxy::processDidBecomeUnresponsive()
{
    RELEASE_LOG_IF_ALLOWED(Process, "processDidBecomeUnresponsive: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (!hasRunningProcess())
        return;

    updateBackingStoreDiscardableState();

    m_navigationClient->processDidBecomeUnresponsive(*this);
}

void WebPageProxy::processDidBecomeResponsive()
{
    RELEASE_LOG_IF_ALLOWED(Process, "processDidBecomeResponsive: webPID = %i, pageID = %" PRIu64, m_process->processIdentifier(), m_pageID.toUInt64());

    if (!hasRunningProcess())
        return;
    
    updateBackingStoreDiscardableState();

    m_navigationClient->processDidBecomeResponsive(*this);
}

void WebPageProxy::willChangeProcessIsResponsive()
{
    m_pageLoadState.willChangeProcessIsResponsive();
}

void WebPageProxy::didChangeProcessIsResponsive()
{
    m_pageLoadState.didChangeProcessIsResponsive();
}

String WebPageProxy::currentURL() const
{
    String url = m_pageLoadState.activeURL();
    if (url.isEmpty() && m_backForwardList->currentItem())
        url = m_backForwardList->currentItem()->url();
    return url;
}

URL WebPageProxy::currentResourceDirectoryURL() const
{
    auto resourceDirectoryURL = m_pageLoadState.resourceDirectoryURL();
    if (!resourceDirectoryURL.isEmpty())
        return resourceDirectoryURL;
    if (auto* item = m_backForwardList->currentItem())
        return item->resourceDirectoryURL();
    return { };
}

void WebPageProxy::processDidTerminate(ProcessTerminationReason reason)
{
    if (reason != ProcessTerminationReason::NavigationSwap)
        RELEASE_LOG_IF_ALLOWED(Process, "processDidTerminate: (pid %d), reason %d", processIdentifier(), reason);

    ASSERT(m_hasRunningProcess);

#if PLATFORM(IOS_FAMILY)
    if (m_process->isUnderMemoryPressure()) {
        String domain = WebCore::topPrivatelyControlledDomain(URL({ }, currentURL()).host().toString());
        if (!domain.isEmpty())
            logDiagnosticMessageWithEnhancedPrivacy(WebCore::DiagnosticLoggingKeys::domainCausingJetsamKey(), domain, WebCore::ShouldSample::No);
    }
#endif

    // There is a nested transaction in resetStateAfterProcessExited() that we don't want to commit before the client call.
    PageLoadState::Transaction transaction = m_pageLoadState.transaction();

    resetStateAfterProcessExited(reason);
    stopAllURLSchemeTasks(m_process.ptr());

    // For bringup of process swapping, NavigationSwap termination will not go out to clients.
    // If it does *during* process swapping, and the client triggers a reload, that causes bizarre WebKit re-entry.
    // FIXME: This might have to change
    if (reason != ProcessTerminationReason::NavigationSwap) {
        navigationState().clearAllNavigations();
        dispatchProcessDidTerminate(reason);
    }

    if (m_controlledByAutomation) {
        if (auto* automationSession = process().processPool().automationSession())
            automationSession->terminate();
    }
}

void WebPageProxy::provisionalProcessDidTerminate()
{
    ASSERT(m_provisionalPage);
    m_provisionalPage = nullptr;
}

static bool shouldReloadAfterProcessTermination(ProcessTerminationReason reason)
{
    switch (reason) {
    case ProcessTerminationReason::ExceededMemoryLimit:
    case ProcessTerminationReason::ExceededCPULimit:
    case ProcessTerminationReason::Crash:
        return true;
    case ProcessTerminationReason::NavigationSwap:
    case ProcessTerminationReason::RequestedByClient:
        break;
    }
    return false;
}

void WebPageProxy::dispatchProcessDidTerminate(ProcessTerminationReason reason)
{
    RELEASE_LOG_IF_ALLOWED(Loading, "dispatchProcessDidTerminate: webPID = %i, reason = %d", m_process->processIdentifier(), reason);

    bool handledByClient = false;
    if (m_loaderClient)
        handledByClient = reason != ProcessTerminationReason::RequestedByClient && m_loaderClient->processDidCrash(*this);
    else
        handledByClient = m_navigationClient->processDidTerminate(*this, reason);

    if (!handledByClient && shouldReloadAfterProcessTermination(reason))
        tryReloadAfterProcessTermination();
}

void WebPageProxy::tryReloadAfterProcessTermination()
{
    m_resetRecentCrashCountTimer.stop();

    if (++m_recentCrashCount > maximumWebProcessRelaunchAttempts) {
        RELEASE_LOG_IF_ALLOWED(Process, "tryReloadAfterProcessTermination: process crashed and the client did not handle it, not reloading the page because we reached the maximum number of attempts");
        m_recentCrashCount = 0;
        return;
    }
    RELEASE_LOG_IF_ALLOWED(Process, "tryReloadAfterProcessTermination: process crashed and the client did not handle it, reloading the page");
    reload(ReloadOption::ExpiredOnly);
}

void WebPageProxy::resetRecentCrashCountSoon()
{
    m_resetRecentCrashCountTimer.startOneShot(resetRecentCrashCountDelay);
}

void WebPageProxy::resetRecentCrashCount()
{
    m_recentCrashCount = 0;
}

void WebPageProxy::stopAllURLSchemeTasks(WebProcessProxy* process)
{
    HashSet<WebURLSchemeHandler*> handlers;
    for (auto& handler : m_urlSchemeHandlersByScheme.values())
        handlers.add(handler.ptr());

    for (auto* handler : handlers)
        handler->stopAllTasksForPage(*this, process);
}

#if PLATFORM(IOS_FAMILY)
void WebPageProxy::processWillBecomeSuspended()
{
    if (!hasRunningProcess())
        return;

    m_hasNetworkRequestsOnSuspended = m_pageLoadState.networkRequestsInProgress();
    if (m_hasNetworkRequestsOnSuspended)
        setNetworkRequestsInProgress(false);
}

void WebPageProxy::processWillBecomeForeground()
{
    if (!hasRunningProcess())
        return;

    if (m_hasNetworkRequestsOnSuspended) {
        setNetworkRequestsInProgress(true);
        m_hasNetworkRequestsOnSuspended = false;
    }
}
#endif

void WebPageProxy::resetState(ResetStateReason resetStateReason)
{
    m_mainFrame = nullptr;
    m_focusedFrame = nullptr;
    m_frameSetLargestFrame = nullptr;
    m_lastSuspendedPage = nullptr;

#if PLATFORM(COCOA)
    m_scrollingPerformanceData = nullptr;
#endif

    if (m_drawingArea) {
#if PLATFORM(COCOA)
        if (resetStateReason == ResetStateReason::NavigationSwap && is<RemoteLayerTreeDrawingAreaProxy>(*m_drawingArea)) {
            // Keep layers around in frozen state to avoid flashing during process swaps.
            m_frozenRemoteLayerTreeHost = downcast<RemoteLayerTreeDrawingAreaProxy>(*m_drawingArea).detachRemoteLayerTreeHost();
        }
#endif
        m_drawingArea = nullptr;
    }
    closeOverlayedViews();

    m_inspector->reset();

#if ENABLE(FULLSCREEN_API)
    if (m_fullScreenManager) {
        m_fullScreenManager->close();
        m_fullScreenManager = nullptr;
    }
#endif

    if (m_openPanelResultListener) {
        m_openPanelResultListener->invalidate();
        m_openPanelResultListener = nullptr;
    }

#if ENABLE(TOUCH_EVENTS)
    m_touchAndPointerEventTracking.reset();
#endif

#if ENABLE(GEOLOCATION)
    m_geolocationPermissionRequestManager.invalidateRequests();
#endif

    m_notificationPermissionRequestManager.invalidateRequests();

    m_toolTip = String();

    m_mainFrameHasHorizontalScrollbar = false;
    m_mainFrameHasVerticalScrollbar = false;

    m_mainFrameIsPinnedToLeftSide = true;
    m_mainFrameIsPinnedToRightSide = true;
    m_mainFrameIsPinnedToTopSide = true;
    m_mainFrameIsPinnedToBottomSide = true;

    m_visibleScrollerThumbRect = IntRect();

#if (PLATFORM(IOS_FAMILY) && HAVE(AVKIT)) || (PLATFORM(MAC) && ENABLE(VIDEO_PRESENTATION_MODE))
    if (m_playbackSessionManager) {
        m_playbackSessionManager->invalidate();
        m_playbackSessionManager = nullptr;
    }
    if (m_videoFullscreenManager) {
        m_videoFullscreenManager->invalidate();
        m_videoFullscreenManager = nullptr;
    }
#endif

#if PLATFORM(IOS_FAMILY)
    m_firstLayerTreeTransactionIdAfterDidCommitLoad = { };
    m_lastVisibleContentRectUpdate = { };
    m_hasNetworkRequestsOnSuspended = false;
    m_isKeyboardAnimatingIn = false;
    m_isScrollingOrZooming = false;
#endif

#if ENABLE(WIRELESS_PLAYBACK_TARGET) && !PLATFORM(IOS_FAMILY)
    pageClient().mediaSessionManager().removeAllPlaybackTargetPickerClients(*this);
#endif

#if ENABLE(APPLE_PAY)
    m_paymentCoordinator = nullptr;
#endif

#if USE(SYSTEM_PREVIEW)
    m_systemPreviewController = nullptr;
#endif

#if ENABLE(WEB_AUTHN)
    m_credentialsMessenger = nullptr;
#endif

#if HAVE(PENCILKIT)
    m_editableImageController = nullptr;
#endif

    CallbackBase::Error error;
    switch (resetStateReason) {
    case ResetStateReason::NavigationSwap:
        FALLTHROUGH;
    case ResetStateReason::PageInvalidated:
        error = CallbackBase::Error::OwnerWasInvalidated;
        break;
    case ResetStateReason::WebProcessExited:
        error = CallbackBase::Error::ProcessExited;
        break;
    }

    m_callbacks.invalidate(error);
    m_loadDependentStringCallbackIDs.clear();

    for (auto& editCommand : std::exchange(m_editCommandSet, { }))
        editCommand->invalidate();

    m_activePopupMenu = nullptr;

    updatePlayingMediaDidChange(MediaProducer::IsNotPlaying);
#if ENABLE(MEDIA_STREAM)
    m_userMediaPermissionRequestManager = nullptr;
#endif

#if ENABLE(POINTER_LOCK)
    requestPointerUnlock();
#endif
    
#if ENABLE(SPEECH_SYNTHESIS)
    resetSpeechSynthesizer();
#endif
}

void WebPageProxy::resetStateAfterProcessExited(ProcessTerminationReason terminationReason)
{
    if (!hasRunningProcess())
        return;

    PageClientProtector protector(pageClient());

#if !ASSERT_DISABLED
    // FIXME: It's weird that resetStateAfterProcessExited() is called even though the process is launching.
    if (terminationReason != ProcessTerminationReason::NavigationSwap)
        ASSERT(m_process->state() == WebProcessProxy::State::Launching || m_process->state() == WebProcessProxy::State::Terminated);
#endif

#if PLATFORM(IOS_FAMILY)
    m_waitingForPostLayoutEditorStateUpdateAfterFocusingElement = false;
    m_deferredElementDidFocusArguments = nullptr;
    m_activityToken = nullptr;
#endif

    m_pageIsUserObservableCount = nullptr;
    m_visiblePageToken = nullptr;

    m_hasRunningProcess = false;
    m_isPageSuspended = false;

    m_editorState = EditorState();
    m_cachedFontAttributesAtSelectionStart.reset();

    if (terminationReason == ProcessTerminationReason::NavigationSwap)
        pageClient().processWillSwap();
    else
        pageClient().processDidExit();

    pageClient().clearAllEditCommands();

    auto resetStateReason = terminationReason == ProcessTerminationReason::NavigationSwap ? ResetStateReason::NavigationSwap : ResetStateReason::WebProcessExited;
    resetState(resetStateReason);

    m_pendingLearnOrIgnoreWordMessageCount = 0;

    // Can't expect DidReceiveEvent notifications from a crashed web process.
    m_mouseEventQueue.clear();
    m_keyEventQueue.clear();
    m_wheelEventQueue.clear();
    m_currentlyProcessedWheelEvents.clear();
#if ENABLE(TOUCH_EVENTS) && !ENABLE(IOS_TOUCH_EVENTS)
    m_touchEventQueue.clear();
#endif

#if ENABLE(ATTACHMENT_ELEMENT)
    invalidateAllAttachments();
#endif

    if (terminationReason != ProcessTerminationReason::NavigationSwap) {
        PageLoadState::Transaction transaction = m_pageLoadState.transaction();
        m_pageLoadState.reset(transaction);
    }

    updatePlayingMediaDidChange(MediaProducer::IsNotPlaying);

    // FIXME: <rdar://problem/38676604> In case of process swaps, the old process should gracefully suspend instead of terminating.
    m_process->processTerminated();
}

WebPageCreationParameters WebPageProxy::creationParameters(WebProcessProxy& process, DrawingAreaProxy& drawingArea)
{
    WebPageCreationParameters parameters { process.websiteDataStore().sessionID() };

    parameters.viewSize = pageClient().viewSize();
    parameters.activityState = m_activityState;
    parameters.drawingAreaType = drawingArea.type();
    parameters.drawingAreaIdentifier = drawingArea.identifier();
    parameters.store = preferencesStore();
    parameters.pageGroupData = m_pageGroup->data();
    parameters.isEditable = m_isEditable;
    parameters.underlayColor = m_underlayColor;
    parameters.useFixedLayout = m_useFixedLayout;
    parameters.fixedLayoutSize = m_fixedLayoutSize;
    parameters.alwaysShowsHorizontalScroller = m_alwaysShowsHorizontalScroller;
    parameters.alwaysShowsVerticalScroller = m_alwaysShowsVerticalScroller;
    parameters.suppressScrollbarAnimations = m_suppressScrollbarAnimations;
    parameters.paginationMode = m_paginationMode;
    parameters.paginationBehavesLikeColumns = m_paginationBehavesLikeColumns;
    parameters.pageLength = m_pageLength;
    parameters.gapBetweenPages = m_gapBetweenPages;
    parameters.paginationLineGridEnabled = m_paginationLineGridEnabled;
    parameters.userAgent = userAgent();
    parameters.itemStates = m_backForwardList->itemStates();
    parameters.userContentControllerID = m_userContentController->identifier();
    parameters.visitedLinkTableID = m_visitedLinkStore->identifier();
    parameters.websiteDataStoreID = process.websiteDataStore().identifier();
    parameters.canRunBeforeUnloadConfirmPanel = m_uiClient->canRunBeforeUnloadConfirmPanel();
    parameters.canRunModal = m_canRunModal;
    parameters.deviceScaleFactor = deviceScaleFactor();
    parameters.viewScaleFactor = m_viewScaleFactor;
    parameters.textZoomFactor = m_textZoomFactor;
    parameters.pageZoomFactor = m_pageZoomFactor;
    parameters.topContentInset = m_topContentInset;
    parameters.mediaVolume = m_mediaVolume;
    parameters.muted = m_mutedState;
    parameters.mayStartMediaWhenInWindow = m_mayStartMediaWhenInWindow;
    parameters.mediaPlaybackIsSuspended = m_mediaPlaybackIsSuspended;
    parameters.minimumSizeForAutoLayout = m_minimumSizeForAutoLayout;
    parameters.autoSizingShouldExpandToViewHeight = m_autoSizingShouldExpandToViewHeight;
    parameters.viewportSizeForCSSViewportUnits = m_viewportSizeForCSSViewportUnits;
    parameters.scrollPinningBehavior = m_scrollPinningBehavior;
    if (m_scrollbarOverlayStyle)
        parameters.scrollbarOverlayStyle = m_scrollbarOverlayStyle.value();
    else
        parameters.scrollbarOverlayStyle = WTF::nullopt;
    parameters.backgroundExtendsBeyondPage = m_backgroundExtendsBeyondPage;
    parameters.layerHostingMode = m_layerHostingMode;
    parameters.controlledByAutomation = m_controlledByAutomation;
    parameters.useDarkAppearance = useDarkAppearance();
    parameters.useElevatedUserInterfaceLevel = useElevatedUserInterfaceLevel();
#if PLATFORM(MAC)
    parameters.colorSpace = pageClient().colorSpace();
    parameters.useSystemAppearance = m_useSystemAppearance;
#endif
#if PLATFORM(IOS_FAMILY)
    parameters.screenSize = screenSize();
    parameters.availableScreenSize = availableScreenSize();
    parameters.overrideScreenSize = overrideScreenSize();
    parameters.textAutosizingWidth = textAutosizingWidth();
    parameters.mimeTypesWithCustomContentProviders = pageClient().mimeTypesWithCustomContentProviders();
    parameters.ignoresViewportScaleLimits = m_forceAlwaysUserScalable;
    parameters.viewportConfigurationViewLayoutSize = m_viewportConfigurationViewLayoutSize;
    parameters.viewportConfigurationLayoutSizeScaleFactor = m_viewportConfigurationLayoutSizeScaleFactor;
    parameters.viewportConfigurationMinimumEffectiveDeviceWidth = m_viewportConfigurationMinimumEffectiveDeviceWidth;
    parameters.maximumUnobscuredSize = m_maximumUnobscuredSize;
    parameters.deviceOrientation = m_deviceOrientation;
    parameters.keyboardIsAttached = isInHardwareKeyboardMode();
    parameters.overrideViewportArguments = m_overrideViewportArguments;
    parameters.canShowWhileLocked = m_configuration->canShowWhileLocked();
#endif

#if PLATFORM(MAC)
    parameters.appleMailPaginationQuirkEnabled = appleMailPaginationQuirkEnabled();
#else
    parameters.appleMailPaginationQuirkEnabled = false;
#endif
    
#if PLATFORM(MAC)
    // FIXME: Need to support iOS too, but there is no isAppleMail for iOS.
    parameters.appleMailLinesClampEnabled = appleMailLinesClampEnabled();
#else
    parameters.appleMailLinesClampEnabled = false;
#endif

#if PLATFORM(COCOA)
    parameters.smartInsertDeleteEnabled = m_isSmartInsertDeleteEnabled;
    parameters.additionalSupportedImageTypes = m_configuration->additionalSupportedImageTypes();
#endif
    parameters.shouldScaleViewToFitDocument = m_shouldScaleViewToFitDocument;
    parameters.userInterfaceLayoutDirection = pageClient().userInterfaceLayoutDirection();
    parameters.observedLayoutMilestones = m_observedLayoutMilestones;
    parameters.overrideContentSecurityPolicy = m_overrideContentSecurityPolicy;
    parameters.cpuLimit = m_cpuLimit;

#if USE(WPE_RENDERER)
    parameters.hostFileDescriptor = pageClient().hostFileDescriptor();
#endif

    for (auto& iterator : m_urlSchemeHandlersByScheme)
        parameters.urlSchemeHandlers.set(iterator.key, iterator.value->identifier());

#if ENABLE(WEB_RTC)
    parameters.iceCandidateFilteringEnabled = m_preferences->iceCandidateFilteringEnabled();
#if USE(LIBWEBRTC)
    parameters.enumeratingAllNetworkInterfacesEnabled = m_preferences->enumeratingAllNetworkInterfacesEnabled();
#endif
#endif

#if ENABLE(APPLICATION_MANIFEST)
    parameters.applicationManifest = m_configuration->applicationManifest() ? Optional<WebCore::ApplicationManifest>(m_configuration->applicationManifest()->applicationManifest()) : WTF::nullopt;
#endif

#if ENABLE(SERVICE_WORKER)
    parameters.hasRegisteredServiceWorkers = process.processPool().mayHaveRegisteredServiceWorkers(process.websiteDataStore());
#endif

    parameters.needsFontAttributes = m_needsFontAttributes;
    parameters.backgroundColor = m_backgroundColor;

    process.addWebUserContentControllerProxy(m_userContentController, parameters);

#if PLATFORM(GTK)
    parameters.themeName = pageClient().themeName();
#endif

    return parameters;
}

void WebPageProxy::isJITEnabled(CompletionHandler<void(bool)>&& completionHandler)
{
    m_process->connection()->sendWithAsyncReply(Messages::WebProcess::IsJITEnabled(), WTFMove(completionHandler));
}

void WebPageProxy::enterAcceleratedCompositingMode(const LayerTreeContext& layerTreeContext)
{
#if PLATFORM(MAC)
    ASSERT(m_drawingArea->type() == DrawingAreaTypeTiledCoreAnimation);
#endif
    pageClient().enterAcceleratedCompositingMode(layerTreeContext);

    if (m_lastSuspendedPage)
        m_lastSuspendedPage->pageEnteredAcceleratedCompositingMode();
}

void WebPageProxy::exitAcceleratedCompositingMode()
{
    pageClient().exitAcceleratedCompositingMode();
}

void WebPageProxy::updateAcceleratedCompositingMode(const LayerTreeContext& layerTreeContext)
{
    pageClient().updateAcceleratedCompositingMode(layerTreeContext);
}

void WebPageProxy::backForwardClear()
{
    m_backForwardList->clear();
}

#if ENABLE(GAMEPAD)

void WebPageProxy::gamepadActivity(const Vector<GamepadData>& gamepadDatas, bool shouldMakeGamepadsVisible)
{
    m_process->send(Messages::WebPage::GamepadActivity(gamepadDatas, shouldMakeGamepadsVisible), m_pageID);
}

#endif

void WebPageProxy::didReceiveAuthenticationChallengeProxy(FrameIdentifier, Ref<AuthenticationChallengeProxy>&& authenticationChallenge)
{
    m_navigationClient->didReceiveAuthenticationChallenge(*this, authenticationChallenge.get());
}

void WebPageProxy::exceededDatabaseQuota(FrameIdentifier frameID, const String& originIdentifier, const String& databaseName, const String& displayName, uint64_t currentQuota, uint64_t currentOriginUsage, uint64_t currentDatabaseUsage, uint64_t expectedUsage, Messages::WebPageProxy::ExceededDatabaseQuota::DelayedReply&& reply)
{
    requestStorageSpace(frameID, originIdentifier, databaseName, displayName, currentQuota, currentOriginUsage, currentDatabaseUsage, expectedUsage, [reply = WTFMove(reply)](auto quota) mutable {
        reply(quota);
    });
}

void WebPageProxy::requestStorageSpace(FrameIdentifier frameID, const String& originIdentifier, const String& databaseName, const String& displayName, uint64_t currentQuota, uint64_t currentOriginUsage, uint64_t currentDatabaseUsage, uint64_t expectedUsage, CompletionHandler<void(uint64_t)>&& completionHandler)
{
    RELEASE_LOG_IF_ALLOWED(Storage, "requestStorageSpace for frame %" PRIu64 ", current quota %" PRIu64 " current usage %" PRIu64 " expected usage %" PRIu64, frameID.toUInt64(), currentQuota, currentDatabaseUsage, expectedUsage);

    StorageRequests::singleton().processOrAppend([this, protectedThis = makeRef(*this), pageURL = currentURL(), frameID, originIdentifier, databaseName, displayName, currentQuota, currentOriginUsage, currentDatabaseUsage, expectedUsage, completionHandler = WTFMove(completionHandler)]() mutable {
        this->makeStorageSpaceRequest(frameID, originIdentifier, databaseName, displayName, currentQuota, currentOriginUsage, currentDatabaseUsage, expectedUsage, [this, protectedThis = WTFMove(protectedThis), frameID, pageURL = WTFMove(pageURL), completionHandler = WTFMove(completionHandler), currentQuota](auto quota) mutable {

            RELEASE_LOG_IF_ALLOWED(Storage, "requestStorageSpace response for frame %" PRIu64 ", quota %" PRIu64, frameID.toUInt64(), quota);

            if (quota <= currentQuota && this->currentURL() == pageURL) {
                RELEASE_LOG_IF_ALLOWED(Storage, "storage space increase denied");
                m_isQuotaIncreaseDenied =  true;
            }
            completionHandler(quota);
            StorageRequests::singleton().processNextIfAny();
        });
    });
}

void WebPageProxy::makeStorageSpaceRequest(FrameIdentifier frameID, const String& originIdentifier, const String& databaseName, const String& displayName, uint64_t currentQuota, uint64_t currentOriginUsage, uint64_t currentDatabaseUsage, uint64_t expectedUsage, CompletionHandler<void(uint64_t)>&& completionHandler)
{
    if (m_isQuotaIncreaseDenied) {
        completionHandler(currentQuota);
        return;
    }

    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    auto originData = SecurityOriginData::fromDatabaseIdentifier(originIdentifier);
    if (originData != SecurityOriginData::fromURL(URL { { }, currentURL() })) {
        completionHandler(currentQuota);
        return;
    }

    auto origin = API::SecurityOrigin::create(originData->securityOrigin());
    m_uiClient->exceededDatabaseQuota(this, frame, origin.ptr(), databaseName, displayName, currentQuota, currentOriginUsage, currentDatabaseUsage, expectedUsage, WTFMove(completionHandler));
}

void WebPageProxy::reachedApplicationCacheOriginQuota(const String& originIdentifier, uint64_t currentQuota, uint64_t totalBytesNeeded, Messages::WebPageProxy::ReachedApplicationCacheOriginQuota::DelayedReply&& reply)
{
    Ref<SecurityOrigin> securityOrigin = SecurityOriginData::fromDatabaseIdentifier(originIdentifier)->securityOrigin();
    m_uiClient->reachedApplicationCacheOriginQuota(this, securityOrigin.get(), currentQuota, totalBytesNeeded, WTFMove(reply));
}

void WebPageProxy::requestGeolocationPermissionForFrame(uint64_t geolocationID, FrameIdentifier frameID, String originIdentifier)
{
    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    // FIXME: Geolocation should probably be using toString() as its string representation instead of databaseIdentifier().
    auto origin = API::SecurityOrigin::create(SecurityOriginData::fromDatabaseIdentifier(originIdentifier)->securityOrigin());
    auto request = m_geolocationPermissionRequestManager.createRequest(geolocationID);
    Function<void(bool)> completionHandler = [request = WTFMove(request)](bool allowed) {
        if (allowed)
            request->allow();
        else
            request->deny();
    };

    // FIXME: Once iOS migrates to the new WKUIDelegate SPI, clean this up
    // and make it one UIClient call that calls the completionHandler with false
    // if there is no delegate instead of returning the completionHandler
    // for other code paths to try.
    m_uiClient->decidePolicyForGeolocationPermissionRequest(*this, *frame, origin.get(), completionHandler);
#if PLATFORM(IOS_FAMILY)
    if (completionHandler)
        pageClient().decidePolicyForGeolocationPermissionRequest(*frame, origin.get(), completionHandler);
#endif
    if (completionHandler)
        completionHandler(false);
}

#if ENABLE(MEDIA_STREAM)
UserMediaPermissionRequestManagerProxy& WebPageProxy::userMediaPermissionRequestManager()
{
    if (m_userMediaPermissionRequestManager)
        return *m_userMediaPermissionRequestManager;

    m_userMediaPermissionRequestManager = makeUnique<UserMediaPermissionRequestManagerProxy>(*this);
    return *m_userMediaPermissionRequestManager;
}

void WebPageProxy::setMockCaptureDevicesEnabledOverride(Optional<bool> enabled)
{
    userMediaPermissionRequestManager().setMockCaptureDevicesEnabledOverride(enabled);
}
#endif

void WebPageProxy::requestUserMediaPermissionForFrame(uint64_t userMediaID, FrameIdentifier frameID, const WebCore::SecurityOriginData&  userMediaDocumentOriginData, const WebCore::SecurityOriginData& topLevelDocumentOriginData, WebCore::MediaStreamRequest&& request)
{
#if ENABLE(MEDIA_STREAM)
    MESSAGE_CHECK(m_process, m_process->webFrame(frameID));

    userMediaPermissionRequestManager().requestUserMediaPermissionForFrame(userMediaID, frameID, userMediaDocumentOriginData.securityOrigin(), topLevelDocumentOriginData.securityOrigin(), WTFMove(request));
#else
    UNUSED_PARAM(userMediaID);
    UNUSED_PARAM(frameID);
    UNUSED_PARAM(userMediaDocumentOriginData);
    UNUSED_PARAM(topLevelDocumentOriginData);
    UNUSED_PARAM(request);
#endif
}

void WebPageProxy::enumerateMediaDevicesForFrame(FrameIdentifier frameID, const WebCore::SecurityOriginData& userMediaDocumentOriginData, const WebCore::SecurityOriginData& topLevelDocumentOriginData, CompletionHandler<void(const Vector<CaptureDevice>&, const String&)>&& completionHandler)
{
#if ENABLE(MEDIA_STREAM)
    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    userMediaPermissionRequestManager().enumerateMediaDevicesForFrame(frameID, userMediaDocumentOriginData.securityOrigin(), topLevelDocumentOriginData.securityOrigin(), WTFMove(completionHandler));
#else
    UNUSED_PARAM(frameID);
    UNUSED_PARAM(userMediaDocumentOriginData);
    UNUSED_PARAM(topLevelDocumentOriginData);
    UNUSED_PARAM(completionHandler);
#endif
}

void WebPageProxy::beginMonitoringCaptureDevices()
{
#if ENABLE(MEDIA_STREAM)
    userMediaPermissionRequestManager().syncWithWebCorePrefs();
    UserMediaProcessManager::singleton().beginMonitoringCaptureDevices();
#endif
}

void WebPageProxy::clearUserMediaState()
{
#if ENABLE(MEDIA_STREAM)
    if (m_userMediaPermissionRequestManager)
        m_userMediaPermissionRequestManager->clearCachedState();
#endif
}

#if ENABLE(DEVICE_ORIENTATION)
void WebPageProxy::shouldAllowDeviceOrientationAndMotionAccess(FrameIdentifier frameID, WebCore::SecurityOriginData&& originData, bool mayPrompt, CompletionHandler<void(DeviceOrientationOrMotionPermissionState)>&& completionHandler)
{
    WebFrameProxy* frame = m_process->webFrame(frameID);
    MESSAGE_CHECK(m_process, frame);

    websiteDataStore().deviceOrientationAndMotionAccessController().shouldAllowAccess(*this, *frame, WTFMove(originData), mayPrompt, WTFMove(completionHandler));
}
#endif

void WebPageProxy::requestNotificationPermission(uint64_t requestID, const String& originString)
{
    if (!isRequestIDValid(requestID))
        return;

    auto origin = API::SecurityOrigin::createFromString(originString);
    auto request = m_notificationPermissionRequestManager.createRequest(requestID);
    
    m_uiClient->decidePolicyForNotificationPermissionRequest(*this, origin.get(), [request = WTFMove(request)](bool allowed) {
        if (allowed)
            request->allow();
        else
            request->deny();
    });
}

void WebPageProxy::showNotification(const String& title, const String& body, const String& iconURL, const String& tag, const String& lang, WebCore::NotificationDirection dir, const String& originString, uint64_t notificationID)
{
    m_process->processPool().supplement<WebNotificationManagerProxy>()->show(this, title, body, iconURL, tag, lang, dir, originString, notificationID);
}

void WebPageProxy::cancelNotification(uint64_t notificationID)
{
    m_process->processPool().supplement<WebNotificationManagerProxy>()->cancel(this, notificationID);
}

void WebPageProxy::clearNotifications(const Vector<uint64_t>& notificationIDs)
{
    m_process->processPool().supplement<WebNotificationManagerProxy>()->clearNotifications(this, notificationIDs);
}

void WebPageProxy::didDestroyNotification(uint64_t notificationID)
{
    m_process->processPool().supplement<WebNotificationManagerProxy>()->didDestroyNotification(this, notificationID);
}

float WebPageProxy::headerHeight(WebFrameProxy& frame)
{
    if (frame.isDisplayingPDFDocument())
        return 0;
    return m_uiClient->headerHeight(*this, frame);
}

float WebPageProxy::footerHeight(WebFrameProxy& frame)
{
    if (frame.isDisplayingPDFDocument())
        return 0;
    return m_uiClient->footerHeight(*this, frame);
}

void WebPageProxy::drawHeader(WebFrameProxy& frame, FloatRect&& rect)
{
    if (frame.isDisplayingPDFDocument())
        return;
    m_uiClient->drawHeader(*this, frame, WTFMove(rect));
}

void WebPageProxy::drawFooter(WebFrameProxy& frame, FloatRect&& rect)
{
    if (frame.isDisplayingPDFDocument())
        return;
    m_uiClient->drawFooter(*this, frame, WTFMove(rect));
}

void WebPageProxy::runModal()
{
    // Since runModal() can (and probably will) spin a nested run loop we need to turn off the responsiveness timer.
    m_process->responsivenessTimer().stop();

    // Our Connection's run loop might have more messages waiting to be handled after this RunModal message.
    // To make sure they are handled inside of the nested modal run loop we must first signal the Connection's
    // run loop so we're guaranteed that it has a chance to wake up.
    // See http://webkit.org/b/89590 for more discussion.
    m_process->connection()->wakeUpRunLoop();

    m_uiClient->runModal(*this);
}

void WebPageProxy::notifyScrollerThumbIsVisibleInRect(const IntRect& scrollerThumb)
{
    m_visibleScrollerThumbRect = scrollerThumb;
}

void WebPageProxy::recommendedScrollbarStyleDidChange(int32_t newStyle)
{
#if USE(APPKIT)
    pageClient().recommendedScrollbarStyleDidChange(static_cast<WebCore::ScrollbarStyle>(newStyle));
#else
    UNUSED_PARAM(newStyle);
#endif
}

void WebPageProxy::didChangeScrollbarsForMainFrame(bool hasHorizontalScrollbar, bool hasVerticalScrollbar)
{
    m_mainFrameHasHorizontalScrollbar = hasHorizontalScrollbar;
    m_mainFrameHasVerticalScrollbar = hasVerticalScrollbar;
}

void WebPageProxy::didChangeScrollOffsetPinningForMainFrame(bool pinnedToLeftSide, bool pinnedToRightSide, bool pinnedToTopSide, bool pinnedToBottomSide)
{
    pageClient().pinnedStateWillChange();
    m_mainFrameIsPinnedToLeftSide = pinnedToLeftSide;
    m_mainFrameIsPinnedToRightSide = pinnedToRightSide;
    m_mainFrameIsPinnedToTopSide = pinnedToTopSide;
    m_mainFrameIsPinnedToBottomSide = pinnedToBottomSide;
    pageClient().pinnedStateDidChange();

    m_uiClient->pinnedStateDidChange(*this);
}

void WebPageProxy::didChangePageCount(unsigned pageCount)
{
    m_pageCount = pageCount;
}

void WebPageProxy::pageExtendedBackgroundColorDidChange(const Color& backgroundColor)
{
    m_pageExtendedBackgroundColor = backgroundColor;
}

#if ENABLE(NETSCAPE_PLUGIN_API)
void WebPageProxy::didFailToInitializePlugin(const String& mimeType, const String& frameURLString, const String& pageURLString)
{
    m_navigationClient->didFailToInitializePlugIn(*this, createPluginInformationDictionary(mimeType, frameURLString, pageURLString).get());
}

void WebPageProxy::didBlockInsecurePluginVersion(const String& mimeType, const String& pluginURLString, const String& frameURLString, const String& pageURLString, bool replacementObscured)
{
    String newMimeType = mimeType;
    PluginModuleInfo plugin = m_process->processPool().pluginInfoStore().findPlugin(newMimeType, URL(URL(), pluginURLString));
    auto pluginInformation = createPluginInformationDictionary(plugin, frameURLString, mimeType, pageURLString, String(), String(), replacementObscured);

    m_navigationClient->didBlockInsecurePluginVersion(*this, pluginInformation.get());
}
#endif // ENABLE(NETSCAPE_PLUGIN_API)

bool WebPageProxy::willHandleHorizontalScrollEvents() const
{
    return !m_canShortCircuitHorizontalWheelEvents;
}

void WebPageProxy::updateWebsitePolicies(WebsitePoliciesData&& websitePolicies)
{
    m_process->send(Messages::WebPage::UpdateWebsitePolicies(websitePolicies), m_pageID);
}

void WebPageProxy::didFinishLoadingDataForCustomContentProvider(const String& suggestedFilename, const IPC::DataReference& dataReference)
{
    pageClient().didFinishLoadingDataForCustomContentProvider(suggestedFilename, dataReference);
}

void WebPageProxy::backForwardRemovedItem(const BackForwardItemIdentifier& itemID)
{
    m_process->send(Messages::WebPage::DidRemoveBackForwardItem(itemID), m_pageID);
}

void WebPageProxy::setCanRunModal(bool canRunModal)
{
    // It's only possible to change the state for a WebPage which
    // already qualifies for running modal child web pages, otherwise
    // there's no other possibility than not allowing it.
    m_canRunModal = m_uiClient->canRunModal() && canRunModal;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetCanRunModal(m_canRunModal), m_pageID);
}

bool WebPageProxy::canRunModal()
{
    return hasRunningProcess() ? m_canRunModal : false;
}

void WebPageProxy::beginPrinting(WebFrameProxy* frame, const PrintInfo& printInfo)
{
    if (m_isInPrintingMode)
        return;

    m_isInPrintingMode = true;
    m_process->send(Messages::WebPage::BeginPrinting(frame->frameID(), printInfo), m_pageID, printingSendOptions(m_isPerformingDOMPrintOperation));
}

void WebPageProxy::endPrinting()
{
    if (!m_isInPrintingMode)
        return;

    m_isInPrintingMode = false;
    m_process->send(Messages::WebPage::EndPrinting(), m_pageID, printingSendOptions(m_isPerformingDOMPrintOperation));
}

void WebPageProxy::computePagesForPrinting(WebFrameProxy* frame, const PrintInfo& printInfo, Ref<ComputedPagesCallback>&& callback)
{
    if (!hasRunningProcess()) {
        callback->invalidate();
        return;
    }

    auto callbackID = callback->callbackID();
    m_callbacks.put(WTFMove(callback));
    m_isInPrintingMode = true;
    m_process->send(Messages::WebPage::ComputePagesForPrinting(frame->frameID(), printInfo, callbackID), m_pageID, printingSendOptions(m_isPerformingDOMPrintOperation));
}

#if PLATFORM(COCOA)
void WebPageProxy::drawRectToImage(WebFrameProxy* frame, const PrintInfo& printInfo, const IntRect& rect, const WebCore::IntSize& imageSize, Ref<ImageCallback>&& callback)
{
    if (!hasRunningProcess()) {
        callback->invalidate();
        return;
    }
    
    auto callbackID = callback->callbackID();
    m_callbacks.put(WTFMove(callback));
    m_process->send(Messages::WebPage::DrawRectToImage(frame->frameID(), printInfo, rect, imageSize, callbackID), m_pageID, printingSendOptions(m_isPerformingDOMPrintOperation));
}

void WebPageProxy::drawPagesToPDF(WebFrameProxy* frame, const PrintInfo& printInfo, uint32_t first, uint32_t count, Ref<DataCallback>&& callback)
{
    if (!hasRunningProcess()) {
        callback->invalidate();
        return;
    }
    
    auto callbackID = callback->callbackID();
    m_callbacks.put(WTFMove(callback));
    m_process->send(Messages::WebPage::DrawPagesToPDF(frame->frameID(), printInfo, first, count, callbackID), m_pageID, printingSendOptions(m_isPerformingDOMPrintOperation));
}
#elif PLATFORM(GTK)
void WebPageProxy::drawPagesForPrinting(WebFrameProxy* frame, const PrintInfo& printInfo, Ref<PrintFinishedCallback>&& callback)
{
    if (!hasRunningProcess()) {
        callback->invalidate();
        return;
    }

    auto callbackID = callback->callbackID();
    m_callbacks.put(WTFMove(callback));
    m_isInPrintingMode = true;
    m_process->send(Messages::WebPage::DrawPagesForPrinting(frame->frameID(), printInfo, callbackID), m_pageID, printingSendOptions(m_isPerformingDOMPrintOperation));
}
#endif

void WebPageProxy::updateBackingStoreDiscardableState()
{
    ASSERT(hasRunningProcess());

    if (!m_drawingArea)
        return;

    bool isDiscardable;

    if (!m_process->isResponsive())
        isDiscardable = false;
    else
        isDiscardable = !pageClient().isViewWindowActive() || !isViewVisible();

    m_drawingArea->setBackingStoreIsDiscardable(isDiscardable);
}

void WebPageProxy::saveDataToFileInDownloadsFolder(String&& suggestedFilename, String&& mimeType, URL&& originatingURLString, API::Data& data)
{
    m_uiClient->saveDataToFileInDownloadsFolder(this, suggestedFilename, mimeType, originatingURLString, data);
}

void WebPageProxy::savePDFToFileInDownloadsFolder(String&& suggestedFilename, URL&& originatingURL, const IPC::DataReference& dataReference)
{
    if (!suggestedFilename.endsWithIgnoringASCIICase(".pdf"))
        return;

    saveDataToFileInDownloadsFolder(WTFMove(suggestedFilename), "application/pdf"_s, WTFMove(originatingURL),
        API::Data::create(dataReference.data(), dataReference.size()).get());
}

void WebPageProxy::setMinimumSizeForAutoLayout(const IntSize& size)
{
    if (m_minimumSizeForAutoLayout == size)
        return;

    m_minimumSizeForAutoLayout = size;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetMinimumSizeForAutoLayout(size), m_pageID);
    m_drawingArea->minimumSizeForAutoLayoutDidChange();

#if USE(APPKIT)
    if (m_minimumSizeForAutoLayout.width() <= 0)
        didChangeIntrinsicContentSize(IntSize(-1, -1));
#endif
}

void WebPageProxy::setAutoSizingShouldExpandToViewHeight(bool shouldExpand)
{
    if (m_autoSizingShouldExpandToViewHeight == shouldExpand)
        return;

    m_autoSizingShouldExpandToViewHeight = shouldExpand;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetAutoSizingShouldExpandToViewHeight(shouldExpand), m_pageID);
}

void WebPageProxy::setViewportSizeForCSSViewportUnits(const IntSize& viewportSize)
{
    if (m_viewportSizeForCSSViewportUnits && *m_viewportSizeForCSSViewportUnits == viewportSize)
        return;

    m_viewportSizeForCSSViewportUnits = viewportSize;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetViewportSizeForCSSViewportUnits(viewportSize), m_pageID);
}

#if USE(AUTOMATIC_TEXT_REPLACEMENT)

void WebPageProxy::toggleSmartInsertDelete()
{
    if (TextChecker::isTestingMode())
        TextChecker::setSmartInsertDeleteEnabled(!TextChecker::isSmartInsertDeleteEnabled());
}

void WebPageProxy::toggleAutomaticQuoteSubstitution()
{
    if (TextChecker::isTestingMode())
        TextChecker::setAutomaticQuoteSubstitutionEnabled(!TextChecker::state().isAutomaticQuoteSubstitutionEnabled);
}

void WebPageProxy::toggleAutomaticLinkDetection()
{
    if (TextChecker::isTestingMode())
        TextChecker::setAutomaticLinkDetectionEnabled(!TextChecker::state().isAutomaticLinkDetectionEnabled);
}

void WebPageProxy::toggleAutomaticDashSubstitution()
{
    if (TextChecker::isTestingMode())
        TextChecker::setAutomaticDashSubstitutionEnabled(!TextChecker::state().isAutomaticDashSubstitutionEnabled);
}

void WebPageProxy::toggleAutomaticTextReplacement()
{
    if (TextChecker::isTestingMode())
        TextChecker::setAutomaticTextReplacementEnabled(!TextChecker::state().isAutomaticTextReplacementEnabled);
}

#endif

#if PLATFORM(MAC)

void WebPageProxy::substitutionsPanelIsShowing(CompletionHandler<void(bool)>&& completionHandler)
{
    completionHandler(TextChecker::substitutionsPanelIsShowing());
}

void WebPageProxy::showCorrectionPanel(int32_t panelType, const FloatRect& boundingBoxOfReplacedString, const String& replacedString, const String& replacementString, const Vector<String>& alternativeReplacementStrings)
{
    // FIXME: Make AlternativeTextType an enum class with EnumTraits and serialize it instead of casting to/from an int32_t.
    pageClient().showCorrectionPanel((AlternativeTextType)panelType, boundingBoxOfReplacedString, replacedString, replacementString, alternativeReplacementStrings);
}

void WebPageProxy::dismissCorrectionPanel(int32_t reason)
{
    // FIXME: Make ReasonForDismissingAlternativeText an enum class with EnumTraits and serialize it instead of casting to/from an int32_t.
    pageClient().dismissCorrectionPanel((ReasonForDismissingAlternativeText)reason);
}

void WebPageProxy::dismissCorrectionPanelSoon(int32_t reason, CompletionHandler<void(String)>&& completionHandler)
{
    // FIXME: Make ReasonForDismissingAlternativeText an enum class with EnumTraits and serialize it instead of casting to/from an int32_t.
    completionHandler(pageClient().dismissCorrectionPanelSoon((ReasonForDismissingAlternativeText)reason));
}

void WebPageProxy::recordAutocorrectionResponse(int32_t response, const String& replacedString, const String& replacementString)
{
    // FIXME: Make AutocorrectionResponse an enum class with EnumTraits and serialize it instead of casting to/from an int32_t.
    pageClient().recordAutocorrectionResponse(static_cast<AutocorrectionResponse>(response), replacedString, replacementString);
}

void WebPageProxy::handleAlternativeTextUIResult(const String& result)
{
    if (!isClosed())
        m_process->send(Messages::WebPage::HandleAlternativeTextUIResult(result), m_pageID);
}

#if USE(DICTATION_ALTERNATIVES)
void WebPageProxy::showDictationAlternativeUI(const WebCore::FloatRect& boundingBoxOfDictatedText, uint64_t dictationContext)
{
    pageClient().showDictationAlternativeUI(boundingBoxOfDictatedText, dictationContext);
}

void WebPageProxy::removeDictationAlternatives(uint64_t dictationContext)
{
    pageClient().removeDictationAlternatives(dictationContext);
}

void WebPageProxy::dictationAlternatives(uint64_t dictationContext, CompletionHandler<void(Vector<String>&&)>&& completionHandler)
{
    completionHandler(pageClient().dictationAlternatives(dictationContext));
}
#endif

void WebPageProxy::setEditableElementIsFocused(bool editableElementIsFocused)
{
    pageClient().setEditableElementIsFocused(editableElementIsFocused);
}

#endif // PLATFORM(MAC)

#if PLATFORM(COCOA) || PLATFORM(GTK)
RefPtr<ViewSnapshot> WebPageProxy::takeViewSnapshot()
{
    return pageClient().takeViewSnapshot();
}
#endif

#if PLATFORM(GTK)
void WebPageProxy::setComposition(const String& text, Vector<CompositionUnderline> underlines, uint64_t selectionStart, uint64_t selectionEnd, uint64_t replacementRangeStart, uint64_t replacementRangeEnd)
{
    // FIXME: We need to find out how to proper handle the crashes case.
    if (!hasRunningProcess())
        return;

    process().send(Messages::WebPage::SetComposition(text, underlines, selectionStart, selectionEnd, replacementRangeStart, replacementRangeEnd), m_pageID);
}

void WebPageProxy::confirmComposition(const String& compositionString, int64_t selectionStart, int64_t selectionLength)
{
    if (!hasRunningProcess())
        return;

    process().send(Messages::WebPage::ConfirmComposition(compositionString, selectionStart, selectionLength), m_pageID);
}

void WebPageProxy::cancelComposition()
{
    if (!hasRunningProcess())
        return;

    process().send(Messages::WebPage::CancelComposition(), m_pageID);
}
#endif // PLATFORM(GTK)

void WebPageProxy::didSaveToPageCache()
{
    m_process->didSaveToPageCache();
}

void WebPageProxy::setScrollPinningBehavior(ScrollPinningBehavior pinning)
{
    if (m_scrollPinningBehavior == pinning)
        return;
    
    m_scrollPinningBehavior = pinning;

    if (hasRunningProcess())
        m_process->send(Messages::WebPage::SetScrollPinningBehavior(pinning), m_pageID);
}

void WebPageProxy::setOverlayScrollbarStyle(Optional<WebCore::ScrollbarOverlayStyle> scrollbarStyle)
{
    if (!m_scrollbarOverlayStyle && !scrollbarStyle)
        return;

    if ((m_scrollbarOverlayStyle && scrollbarStyle) && m_scrollbarOverlayStyle.value() == scrollbarStyle.value())
        return;

    m_scrollbarOverlayStyle = scrollbarStyle;

    Optional<uint32_t> scrollbarStyleForMessage;
    if (scrollbarStyle)
        scrollbarStyleForMessage = static_cast<ScrollbarOverlayStyle>(scrollbarStyle.value());

    if (hasRunningProcess())
        m_process->send(Messages::WebPage::SetScrollbarOverlayStyle(scrollbarStyleForMessage), m_pageID);
}

#if ENABLE(WEB_CRYPTO)
void WebPageProxy::wrapCryptoKey(const Vector<uint8_t>& key, CompletionHandler<void(bool, Vector<uint8_t>&&)>&& completionHandler)
{
    PageClientProtector protector(pageClient());

    Vector<uint8_t> masterKey;

    if (auto keyData = m_navigationClient->webCryptoMasterKey(*this))
        masterKey = keyData->dataReference().vector();

    Vector<uint8_t> wrappedKey;
    bool succeeded = wrapSerializedCryptoKey(masterKey, key, wrappedKey);
    completionHandler(succeeded, WTFMove(wrappedKey));
}

void WebPageProxy::unwrapCryptoKey(const Vector<uint8_t>& wrappedKey, CompletionHandler<void(bool, Vector<uint8_t>&&)>&& completionHandler)
{
    PageClientProtector protector(pageClient());

    Vector<uint8_t> masterKey;

    if (auto keyData = m_navigationClient->webCryptoMasterKey(*this))
        masterKey = keyData->dataReference().vector();

    Vector<uint8_t> key;
    bool succeeded = unwrapSerializedCryptoKey(masterKey, wrappedKey, key);
    completionHandler(succeeded, WTFMove(key));
}
#endif

void WebPageProxy::signedPublicKeyAndChallengeString(unsigned keySizeIndex, const String& challengeString, const URL& url, CompletionHandler<void(String)>&& completionHandler)
{
    PageClientProtector protector(pageClient());

    if (auto apiString = m_navigationClient->signedPublicKeyAndChallengeString(*this, keySizeIndex, API::String::create(challengeString), url))
        return completionHandler(apiString->string());
    
    completionHandler({ });
}

void WebPageProxy::addMIMETypeWithCustomContentProvider(const String& mimeType)
{
    m_process->send(Messages::WebPage::AddMIMETypeWithCustomContentProvider(mimeType), m_pageID);
}

void WebPageProxy::changeFontAttributes(WebCore::FontAttributeChanges&& changes)
{
    if (!hasRunningProcess())
        return;

    process().send(Messages::WebPage::ChangeFontAttributes(WTFMove(changes)), m_pageID);
}

void WebPageProxy::changeFont(WebCore::FontChanges&& changes)
{
    if (!hasRunningProcess())
        return;

    process().send(Messages::WebPage::ChangeFont(WTFMove(changes)), m_pageID);
}

#if PLATFORM(COCOA)

void WebPageProxy::setTextAsync(const String& text)
{
    if (hasRunningProcess())
        process().send(Messages::WebPage::SetTextAsync(text), m_pageID);
}

void WebPageProxy::insertTextAsync(const String& text, const EditingRange& replacementRange, InsertTextOptions&& options)
{
    if (!hasRunningProcess())
        return;

    process().send(Messages::WebPage::InsertTextAsync(text, replacementRange, WTFMove(options)), m_pageID);
}

void WebPageProxy::hasMarkedText(CompletionHandler<void(bool)>&& callback)
{
    if (!hasRunningProcess()) {
        callback(false);
        return;
    }
    m_process->connection()->sendWithAsyncReply(Messages::WebPage::HasMarkedText(), WTFMove(callback), m_pageID);
}

void WebPageProxy::getMarkedRangeAsync(WTF::Function<void (EditingRange, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(EditingRange(), CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    process().send(Messages::WebPage::GetMarkedRangeAsync(callbackID), m_pageID);
}

void WebPageProxy::getSelectedRangeAsync(WTF::Function<void (EditingRange, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(EditingRange(), CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    process().send(Messages::WebPage::GetSelectedRangeAsync(callbackID), m_pageID);
}

void WebPageProxy::characterIndexForPointAsync(const WebCore::IntPoint& point, WTF::Function<void (uint64_t, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(0, CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    process().send(Messages::WebPage::CharacterIndexForPointAsync(point, callbackID), m_pageID);
}

void WebPageProxy::firstRectForCharacterRangeAsync(const EditingRange& range, WTF::Function<void (const WebCore::IntRect&, const EditingRange&, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(WebCore::IntRect(), EditingRange(), CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    process().send(Messages::WebPage::FirstRectForCharacterRangeAsync(range, callbackID), m_pageID);
}

void WebPageProxy::setCompositionAsync(const String& text, const Vector<CompositionUnderline>& underlines, const EditingRange& selectionRange, const EditingRange& replacementRange)
{
    if (!hasRunningProcess()) {
        // If this fails, we should call -discardMarkedText on input context to notify the input method.
        // This will happen naturally later, as part of reloading the page.
        return;
    }

    process().send(Messages::WebPage::SetCompositionAsync(text, underlines, selectionRange, replacementRange), m_pageID);
}

void WebPageProxy::confirmCompositionAsync()
{
    if (!hasRunningProcess())
        return;

    process().send(Messages::WebPage::ConfirmCompositionAsync(), m_pageID);
}

void WebPageProxy::setScrollPerformanceDataCollectionEnabled(bool enabled)
{
    if (enabled == m_scrollPerformanceDataCollectionEnabled)
        return;

    m_scrollPerformanceDataCollectionEnabled = enabled;

    if (m_scrollPerformanceDataCollectionEnabled && !m_scrollingPerformanceData)
        m_scrollingPerformanceData = makeUnique<RemoteLayerTreeScrollingPerformanceData>(downcast<RemoteLayerTreeDrawingAreaProxy>(*m_drawingArea));
    else if (!m_scrollPerformanceDataCollectionEnabled)
        m_scrollingPerformanceData = nullptr;
}
#endif

void WebPageProxy::takeSnapshot(IntRect rect, IntSize bitmapSize, SnapshotOptions options, WTF::Function<void (const ShareableBitmap::Handle&, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(ShareableBitmap::Handle(), CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::TakeSnapshot(rect, bitmapSize, options, callbackID), m_pageID);
}

void WebPageProxy::navigationGestureDidBegin()
{
    PageClientProtector protector(pageClient());

    m_isShowingNavigationGestureSnapshot = true;
    pageClient().navigationGestureDidBegin();

    m_navigationClient->didBeginNavigationGesture(*this);
}

void WebPageProxy::navigationGestureWillEnd(bool willNavigate, WebBackForwardListItem& item)
{
    PageClientProtector protector(pageClient());

    pageClient().navigationGestureWillEnd(willNavigate, item);

    m_navigationClient->willEndNavigationGesture(*this, willNavigate, item);
}

void WebPageProxy::navigationGestureDidEnd(bool willNavigate, WebBackForwardListItem& item)
{
    PageClientProtector protector(pageClient());

    pageClient().navigationGestureDidEnd(willNavigate, item);

    m_navigationClient->didEndNavigationGesture(*this, willNavigate, item);
}

void WebPageProxy::navigationGestureDidEnd()
{
    PageClientProtector protector(pageClient());

    pageClient().navigationGestureDidEnd();
}

void WebPageProxy::willRecordNavigationSnapshot(WebBackForwardListItem& item)
{
    PageClientProtector protector(pageClient());

    pageClient().willRecordNavigationSnapshot(item);
}

void WebPageProxy::navigationGestureSnapshotWasRemoved()
{
    m_isShowingNavigationGestureSnapshot = false;

    // The ViewGestureController may call this method on a WebPageProxy whose view has been destroyed. In such case,
    // we need to return early as the pageClient will not be valid below.
    if (m_isClosed)
        return;

    pageClient().didRemoveNavigationGestureSnapshot();

    m_navigationClient->didRemoveNavigationGestureSnapshot(*this);
}

void WebPageProxy::isPlayingMediaDidChange(MediaProducer::MediaStateFlags newState, uint64_t sourceElementID)
{
#if ENABLE(MEDIA_SESSION)
    WebMediaSessionFocusManager* focusManager = process().processPool().supplement<WebMediaSessionFocusManager>();
    ASSERT(focusManager);
    focusManager->updatePlaybackAttributesFromMediaState(this, sourceElementID, newState);
#endif
    if (!m_isClosed)
        updatePlayingMediaDidChange(newState);
}

void WebPageProxy::updatePlayingMediaDidChange(MediaProducer::MediaStateFlags newState)
{
    if (newState == m_mediaState)
        return;

#if ENABLE(MEDIA_STREAM)
    WebCore::MediaProducer::MediaStateFlags oldMediaCaptureState = m_mediaState & WebCore::MediaProducer::MediaCaptureMask;
    WebCore::MediaProducer::MediaStateFlags newMediaCaptureState = newState & WebCore::MediaProducer::MediaCaptureMask;
#endif

    MediaProducer::MediaStateFlags playingMediaMask = MediaProducer::IsPlayingAudio | MediaProducer::IsPlayingVideo;
    MediaProducer::MediaStateFlags oldState = m_mediaState;

    bool playingAudioChanges = (oldState & MediaProducer::IsPlayingAudio) != (newState & MediaProducer::IsPlayingAudio);
    if (playingAudioChanges)
        pageClient().isPlayingAudioWillChange();
    m_mediaState = newState;
    if (playingAudioChanges)
        pageClient().isPlayingAudioDidChange();

#if ENABLE(MEDIA_STREAM)
    if (oldMediaCaptureState != newMediaCaptureState) {
        m_uiClient->mediaCaptureStateDidChange(m_mediaState);
        m_userMediaPermissionRequestManager->captureStateChanged(oldMediaCaptureState, newMediaCaptureState);
    }
#endif

    activityStateDidChange({ ActivityState::IsAudible, ActivityState::IsCapturingMedia });

    playingMediaMask |= WebCore::MediaProducer::MediaCaptureMask;
    if ((oldState & playingMediaMask) != (m_mediaState & playingMediaMask))
        m_uiClient->isPlayingMediaDidChange(*this);

    if ((oldState & MediaProducer::HasAudioOrVideo) != (m_mediaState & MediaProducer::HasAudioOrVideo))
        videoControlsManagerDidChange();

    m_process->webPageMediaStateDidChange(*this);
}

void WebPageProxy::videoControlsManagerDidChange()
{
    pageClient().videoControlsManagerDidChange();
}

bool WebPageProxy::hasActiveVideoForControlsManager() const
{
#if (PLATFORM(IOS_FAMILY) && HAVE(AVKIT)) || (PLATFORM(MAC) && ENABLE(VIDEO_PRESENTATION_MODE))
    return m_playbackSessionManager && m_playbackSessionManager->controlsManagerInterface();
#else
    return false;
#endif
}

void WebPageProxy::requestControlledElementID() const
{
#if (PLATFORM(IOS_FAMILY) && HAVE(AVKIT)) || (PLATFORM(MAC) && ENABLE(VIDEO_PRESENTATION_MODE))
    if (m_playbackSessionManager)
        m_playbackSessionManager->requestControlledElementID();
#endif
}

void WebPageProxy::handleControlledElementIDResponse(const String& identifier) const
{
#if PLATFORM(MAC)
    pageClient().handleControlledElementIDResponse(identifier);
#endif
}

bool WebPageProxy::isPlayingVideoInEnhancedFullscreen() const
{
#if (PLATFORM(IOS_FAMILY) && HAVE(AVKIT)) || (PLATFORM(MAC) && ENABLE(VIDEO_PRESENTATION_MODE))
    return m_videoFullscreenManager && m_videoFullscreenManager->isPlayingVideoInEnhancedFullscreen();
#else
    return false;
#endif
}

#if PLATFORM(COCOA)
void WebPageProxy::requestActiveNowPlayingSessionInfo(Ref<NowPlayingInfoCallback>&& callback)
{
    if (!hasRunningProcess()) {
        callback->invalidate();
        return;
    }

    auto callbackID = callback->callbackID();
    m_callbacks.put(WTFMove(callback));

    m_process->send(Messages::WebPage::RequestActiveNowPlayingSessionInfo(callbackID), m_pageID);
}

void WebPageProxy::nowPlayingInfoCallback(bool hasActiveSession, bool registeredAsNowPlayingApplication, const String& title, double duration, double elapsedTime, uint64_t uniqueIdentifier, CallbackID callbackID)
{
    auto callback = m_callbacks.take<NowPlayingInfoCallback>(callbackID);
    if (!callback) {
        // FIXME: Log error or assert.
        return;
    }

    callback->performCallbackWithReturnValue(hasActiveSession, registeredAsNowPlayingApplication, title, duration, elapsedTime, uniqueIdentifier);
}
#endif

#if ENABLE(MEDIA_SESSION)
void WebPageProxy::hasMediaSessionWithActiveMediaElementsDidChange(bool state)
{
    m_hasMediaSessionWithActiveMediaElements = state;
}

void WebPageProxy::mediaSessionMetadataDidChange(const WebCore::MediaSessionMetadata& metadata)
{
    Ref<WebMediaSessionMetadata> webMetadata = WebMediaSessionMetadata::create(metadata);
    m_uiClient->mediaSessionMetadataDidChange(*this, webMetadata.ptr());
}

void WebPageProxy::focusedContentMediaElementDidChange(uint64_t elementID)
{
    WebMediaSessionFocusManager* focusManager = process().processPool().supplement<WebMediaSessionFocusManager>();
    ASSERT(focusManager);
    focusManager->setFocusedMediaElement(*this, elementID);
}
#endif

void WebPageProxy::handleAutoplayEvent(WebCore::AutoplayEvent event, OptionSet<AutoplayEventFlags> flags)
{
    m_uiClient->handleAutoplayEvent(*this, event, flags);
}

#if PLATFORM(MAC)
void WebPageProxy::performImmediateActionHitTestAtLocation(FloatPoint point)
{
    m_process->send(Messages::WebPage::PerformImmediateActionHitTestAtLocation(point), m_pageID);
}

void WebPageProxy::immediateActionDidUpdate()
{
    m_process->send(Messages::WebPage::ImmediateActionDidUpdate(), m_pageID);
}

void WebPageProxy::immediateActionDidCancel()
{
    m_process->send(Messages::WebPage::ImmediateActionDidCancel(), m_pageID);
}

void WebPageProxy::immediateActionDidComplete()
{
    m_process->send(Messages::WebPage::ImmediateActionDidComplete(), m_pageID);
}

void WebPageProxy::didPerformImmediateActionHitTest(const WebHitTestResultData& result, bool contentPreventsDefault, const UserData& userData)
{
    pageClient().didPerformImmediateActionHitTest(result, contentPreventsDefault, m_process->transformHandlesToObjects(userData.object()).get());
}

NSObject *WebPageProxy::immediateActionAnimationControllerForHitTestResult(RefPtr<API::HitTestResult> hitTestResult, uint64_t type, RefPtr<API::Object> userData)
{
    return pageClient().immediateActionAnimationControllerForHitTestResult(hitTestResult, type, userData);
}

void WebPageProxy::handleAcceptedCandidate(WebCore::TextCheckingResult acceptedCandidate)
{
    m_process->send(Messages::WebPage::HandleAcceptedCandidate(acceptedCandidate), m_pageID);
}

void WebPageProxy::didHandleAcceptedCandidate()
{
    pageClient().didHandleAcceptedCandidate();
}
    
void WebPageProxy::setUseSystemAppearance(bool useSystemAppearance)
{    
    if (useSystemAppearance == m_useSystemAppearance)
        return;
    
    m_useSystemAppearance = useSystemAppearance;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetUseSystemAppearance(useSystemAppearance), m_pageID);
}
    
void WebPageProxy::setHeaderBannerHeightForTesting(int height)
{
    m_process->send(Messages::WebPage::SetHeaderBannerHeightForTesting(height), m_pageID);
}

void WebPageProxy::setFooterBannerHeightForTesting(int height)
{
    m_process->send(Messages::WebPage::SetFooterBannerHeightForTesting(height), m_pageID);
}

#endif

void WebPageProxy::installActivityStateChangeCompletionHandler(Function<void()>&& completionHandler)
{
    if (!hasRunningProcess()) {
        completionHandler();
        return;
    }

    auto voidCallback = VoidCallback::create([completionHandler = WTFMove(completionHandler)] (auto) {
        completionHandler();
    }, m_process->throttler().backgroundActivityToken());
    auto callbackID = m_callbacks.put(WTFMove(voidCallback));
    m_nextActivityStateChangeCallbacks.append(callbackID);
}

void WebPageProxy::imageOrMediaDocumentSizeChanged(const WebCore::IntSize& newSize)
{
    m_uiClient->imageOrMediaDocumentSizeChanged(newSize);
}

void WebPageProxy::setShouldDispatchFakeMouseMoveEvents(bool shouldDispatchFakeMouseMoveEvents)
{
    m_process->send(Messages::WebPage::SetShouldDispatchFakeMouseMoveEvents(shouldDispatchFakeMouseMoveEvents), m_pageID);
}

void WebPageProxy::handleAutoFillButtonClick(const UserData& userData)
{
    m_uiClient->didClickAutoFillButton(*this, m_process->transformHandlesToObjects(userData.object()).get());
}

void WebPageProxy::didResignInputElementStrongPasswordAppearance(const UserData& userData)
{
    m_uiClient->didResignInputElementStrongPasswordAppearance(*this, m_process->transformHandlesToObjects(userData.object()).get());
}

#if ENABLE(WIRELESS_PLAYBACK_TARGET) && !PLATFORM(IOS_FAMILY)
void WebPageProxy::addPlaybackTargetPickerClient(uint64_t contextId)
{
    pageClient().mediaSessionManager().addPlaybackTargetPickerClient(*this, contextId);
}

void WebPageProxy::removePlaybackTargetPickerClient(uint64_t contextId)
{
    pageClient().mediaSessionManager().removePlaybackTargetPickerClient(*this, contextId);
}

void WebPageProxy::showPlaybackTargetPicker(uint64_t contextId, const WebCore::FloatRect& rect, bool hasVideo)
{
    pageClient().mediaSessionManager().showPlaybackTargetPicker(*this, contextId, pageClient().rootViewToScreen(IntRect(rect)), hasVideo, useDarkAppearance());
}

void WebPageProxy::playbackTargetPickerClientStateDidChange(uint64_t contextId, WebCore::MediaProducer::MediaStateFlags state)
{
    pageClient().mediaSessionManager().clientStateDidChange(*this, contextId, state);
}

void WebPageProxy::setMockMediaPlaybackTargetPickerEnabled(bool enabled)
{
    pageClient().mediaSessionManager().setMockMediaPlaybackTargetPickerEnabled(enabled);
}

void WebPageProxy::setMockMediaPlaybackTargetPickerState(const String& name, WebCore::MediaPlaybackTargetContext::State state)
{
    pageClient().mediaSessionManager().setMockMediaPlaybackTargetPickerState(name, state);
}

void WebPageProxy::setPlaybackTarget(uint64_t contextId, Ref<MediaPlaybackTarget>&& target)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::PlaybackTargetSelected(contextId, target->targetContext()), m_pageID);
}

void WebPageProxy::externalOutputDeviceAvailableDidChange(uint64_t contextId, bool available)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::PlaybackTargetAvailabilityDidChange(contextId, available), m_pageID);
}

void WebPageProxy::setShouldPlayToPlaybackTarget(uint64_t contextId, bool shouldPlay)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetShouldPlayToPlaybackTarget(contextId, shouldPlay), m_pageID);
}
#endif

void WebPageProxy::didExceedInactiveMemoryLimitWhileActive()
{
    RELEASE_LOG_IF_ALLOWED(PerformanceLogging, "didExceedInactiveMemoryLimitWhileActive");
    m_uiClient->didExceedBackgroundResourceLimitWhileInForeground(*this, kWKResourceLimitMemory);
}

void WebPageProxy::didExceedBackgroundCPULimitWhileInForeground()
{
    RELEASE_LOG_IF_ALLOWED(PerformanceLogging, "didExceedBackgroundCPULimitWhileInForeground");
    m_uiClient->didExceedBackgroundResourceLimitWhileInForeground(*this, kWKResourceLimitCPU);
}

void WebPageProxy::didChangeBackgroundColor()
{
    pageClient().didChangeBackgroundColor();
}

void WebPageProxy::clearWheelEventTestTrigger()
{
    if (!hasRunningProcess())
        return;
    
    m_process->send(Messages::WebPage::ClearWheelEventTestTrigger(), m_pageID);
}

void WebPageProxy::callAfterNextPresentationUpdate(WTF::Function<void (CallbackBase::Error)>&& callback)
{
    if (!hasRunningProcess() || !m_drawingArea) {
        callback(CallbackBase::Error::OwnerWasInvalidated);
        return;
    }

    m_drawingArea->dispatchAfterEnsuringDrawing(WTFMove(callback));
}

void WebPageProxy::setShouldScaleViewToFitDocument(bool shouldScaleViewToFitDocument)
{
    if (m_shouldScaleViewToFitDocument == shouldScaleViewToFitDocument)
        return;

    m_shouldScaleViewToFitDocument = shouldScaleViewToFitDocument;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetShouldScaleViewToFitDocument(shouldScaleViewToFitDocument), m_pageID);
}

void WebPageProxy::didRestoreScrollPosition()
{
    pageClient().didRestoreScrollPosition();
}

void WebPageProxy::getLoadDecisionForIcon(const WebCore::LinkIcon& icon, CallbackID loadIdentifier)
{
    m_iconLoadingClient->getLoadDecisionForIcon(icon, [this, protectedThis = RefPtr<WebPageProxy>(this), loadIdentifier](WTF::Function<void (API::Data*, CallbackBase::Error)>&& callbackFunction) {
        if (!hasRunningProcess()) {
            if (callbackFunction)
                callbackFunction(nullptr, CallbackBase::Error::Unknown);
            return;
        }

        bool decision = (bool)callbackFunction;
        auto newCallbackIdentifier = decision ? OptionalCallbackID(m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken())) : OptionalCallbackID();

        m_process->send(Messages::WebPage::DidGetLoadDecisionForIcon(decision, loadIdentifier, newCallbackIdentifier), m_pageID);
    });
}

void WebPageProxy::finishedLoadingIcon(CallbackID callbackID, const IPC::DataReference& data)
{
    dataCallback(data, callbackID);
}

void WebPageProxy::setResourceCachingDisabled(bool disabled)
{
    if (m_isResourceCachingDisabled == disabled)
        return;

    m_isResourceCachingDisabled = disabled;

    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetResourceCachingDisabled(disabled), m_pageID);
}

WebCore::UserInterfaceLayoutDirection WebPageProxy::userInterfaceLayoutDirection()
{
    return pageClient().userInterfaceLayoutDirection();
}

void WebPageProxy::setUserInterfaceLayoutDirection(WebCore::UserInterfaceLayoutDirection userInterfaceLayoutDirection)
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::SetUserInterfaceLayoutDirection(static_cast<uint32_t>(userInterfaceLayoutDirection)), m_pageID);
}

void WebPageProxy::hideValidationMessage()
{
#if PLATFORM(COCOA)
    m_validationBubble = nullptr;
#endif
}

// FIXME: Consolidate with dismissContentRelativeChildWindows
void WebPageProxy::closeOverlayedViews()
{
    hideValidationMessage();

#if ENABLE(DATALIST_ELEMENT)
    endDataListSuggestions();
#endif

#if ENABLE(INPUT_TYPE_COLOR)
    endColorPicker();
#endif
}

#if ENABLE(POINTER_LOCK)
void WebPageProxy::requestPointerLock()
{
    ASSERT(!m_isPointerLockPending);
    ASSERT(!m_isPointerLocked);
    m_isPointerLockPending = true;

    if (!isViewVisible() || !(m_activityState & ActivityState::IsFocused)) {
        didDenyPointerLock();
        return;
    }
    m_uiClient->requestPointerLock(this);
}
    
void WebPageProxy::didAllowPointerLock()
{
    ASSERT(m_isPointerLockPending && !m_isPointerLocked);
    m_isPointerLocked = true;
    m_isPointerLockPending = false;
#if PLATFORM(MAC)
    CGDisplayHideCursor(CGMainDisplayID());
    CGAssociateMouseAndMouseCursorPosition(false);
#endif
    m_process->send(Messages::WebPage::DidAcquirePointerLock(), m_pageID);
}
    
void WebPageProxy::didDenyPointerLock()
{
    ASSERT(m_isPointerLockPending && !m_isPointerLocked);
    m_isPointerLockPending = false;
    m_process->send(Messages::WebPage::DidNotAcquirePointerLock(), m_pageID);
}

void WebPageProxy::requestPointerUnlock()
{
    if (m_isPointerLocked) {
#if PLATFORM(MAC)
        CGAssociateMouseAndMouseCursorPosition(true);
        CGDisplayShowCursor(CGMainDisplayID());
#endif
        m_uiClient->didLosePointerLock(this);
        m_process->send(Messages::WebPage::DidLosePointerLock(), m_pageID);
    }

    if (m_isPointerLockPending) {
        m_uiClient->didLosePointerLock(this);
        m_process->send(Messages::WebPage::DidNotAcquirePointerLock(), m_pageID);
    }

    m_isPointerLocked = false;
    m_isPointerLockPending = false;
}
#endif

void WebPageProxy::setURLSchemeHandlerForScheme(Ref<WebURLSchemeHandler>&& handler, const String& scheme)
{
    auto canonicalizedScheme = WTF::URLParser::maybeCanonicalizeScheme(scheme);
    ASSERT(canonicalizedScheme);
    ASSERT(!WTF::URLParser::isSpecialScheme(canonicalizedScheme.value()));

    auto schemeResult = m_urlSchemeHandlersByScheme.add(canonicalizedScheme.value(), handler.get());
    ASSERT_UNUSED(schemeResult, schemeResult.isNewEntry);

    auto identifier = handler->identifier();
    auto identifierResult = m_urlSchemeHandlersByIdentifier.add(identifier, WTFMove(handler));
    ASSERT_UNUSED(identifierResult, identifierResult.isNewEntry);

    m_process->send(Messages::WebPage::RegisterURLSchemeHandler(identifier, canonicalizedScheme.value()), m_pageID);
}

WebURLSchemeHandler* WebPageProxy::urlSchemeHandlerForScheme(const String& scheme)
{
    return scheme.isNull() ? nullptr : m_urlSchemeHandlersByScheme.get(scheme);
}

void WebPageProxy::startURLSchemeTask(URLSchemeTaskParameters&& parameters)
{
    startURLSchemeTaskShared(m_process.copyRef(), WTFMove(parameters));
}

void WebPageProxy::startURLSchemeTaskShared(Ref<WebProcessProxy>&& process, URLSchemeTaskParameters&& parameters)
{
    auto iterator = m_urlSchemeHandlersByIdentifier.find(parameters.handlerIdentifier);
    MESSAGE_CHECK(process, iterator != m_urlSchemeHandlersByIdentifier.end());

    iterator->value->startTask(*this, process, parameters.taskIdentifier, WTFMove(parameters.request), nullptr);
}

void WebPageProxy::stopURLSchemeTask(uint64_t handlerIdentifier, uint64_t taskIdentifier)
{
    auto iterator = m_urlSchemeHandlersByIdentifier.find(handlerIdentifier);
    MESSAGE_CHECK(m_process, iterator != m_urlSchemeHandlersByIdentifier.end());

    iterator->value->stopTask(*this, taskIdentifier);
}

void WebPageProxy::loadSynchronousURLSchemeTask(URLSchemeTaskParameters&& parameters, Messages::WebPageProxy::LoadSynchronousURLSchemeTask::DelayedReply&& reply)
{
    auto iterator = m_urlSchemeHandlersByIdentifier.find(parameters.handlerIdentifier);
    MESSAGE_CHECK(m_process, iterator != m_urlSchemeHandlersByIdentifier.end());

    iterator->value->startTask(*this, m_process, parameters.taskIdentifier, WTFMove(parameters.request), WTFMove(reply));
}

#if ENABLE(RESOURCE_LOAD_STATISTICS)
void WebPageProxy::requestStorageAccessConfirm(const RegistrableDomain& subFrameDomain, const RegistrableDomain& topFrameDomain, FrameIdentifier frameID, CompletionHandler<void(bool)>&& completionHandler)
{
    m_uiClient->requestStorageAccessConfirm(*this, m_process->webFrame(frameID), subFrameDomain, topFrameDomain, WTFMove(completionHandler));
}

void WebPageProxy::didCommitCrossSiteLoadWithDataTransferFromPrevalentResource()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::WasLoadedWithDataTransferFromPrevalentResource(), m_pageID);
}
#endif

bool WebPageProxy::useDarkAppearance() const
{
    return pageClient().effectiveAppearanceIsDark();
}

bool WebPageProxy::useElevatedUserInterfaceLevel() const
{
    return pageClient().effectiveUserInterfaceLevelIsElevated();
}

void WebPageProxy::effectiveAppearanceDidChange()
{
    if (!hasRunningProcess())
        return;

    m_process->send(Messages::WebPage::EffectiveAppearanceDidChange(useDarkAppearance(), useElevatedUserInterfaceLevel()), m_pageID);
}

#if PLATFORM(COCOA)
void WebPageProxy::touchBarMenuDataChanged(const TouchBarMenuData& touchBarMenuData)
{
    m_touchBarMenuData = touchBarMenuData;
}

void WebPageProxy::touchBarMenuItemDataAdded(const TouchBarMenuItemData& touchBarMenuItemData)
{
    m_touchBarMenuData.addMenuItem(touchBarMenuItemData);
}

void WebPageProxy::touchBarMenuItemDataRemoved(const TouchBarMenuItemData& touchBarMenuItemData)
{
    m_touchBarMenuData.removeMenuItem(touchBarMenuItemData);
}
#endif

#if ENABLE(ATTACHMENT_ELEMENT)

RefPtr<API::Attachment> WebPageProxy::attachmentForIdentifier(const String& identifier) const
{
    if (identifier.isEmpty())
        return nullptr;

    return m_attachmentIdentifierToAttachmentMap.get(identifier);
}

void WebPageProxy::insertAttachment(Ref<API::Attachment>&& attachment, Function<void(CallbackBase::Error)>&& callback)
{
    if (!hasRunningProcess()) {
        callback(CallbackBase::Error::OwnerWasInvalidated);
        return;
    }

    auto attachmentIdentifier = attachment->identifier();
    auto callbackID = m_callbacks.put(WTFMove(callback), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::InsertAttachment(attachmentIdentifier, attachment->fileSizeForDisplay(), attachment->fileName(), attachment->contentType(), callbackID), m_pageID);
    m_attachmentIdentifierToAttachmentMap.set(attachmentIdentifier, WTFMove(attachment));
}

void WebPageProxy::updateAttachmentAttributes(const API::Attachment& attachment, Function<void(CallbackBase::Error)>&& callback)
{
    if (!hasRunningProcess()) {
        callback(CallbackBase::Error::OwnerWasInvalidated);
        return;
    }

    IPC::SharedBufferDataReference dataReference;
    if (auto data = attachment.enclosingImageData())
        dataReference = { *data };

    auto callbackID = m_callbacks.put(WTFMove(callback), m_process->throttler().backgroundActivityToken());
    m_process->send(Messages::WebPage::UpdateAttachmentAttributes(attachment.identifier(), attachment.fileSizeForDisplay(), attachment.contentType(), attachment.fileName(), WTFMove(dataReference), callbackID), m_pageID);
}

void WebPageProxy::registerAttachmentIdentifierFromData(const String& identifier, const String& contentType, const String& preferredFileName, const IPC::DataReference& data)
{
    if (attachmentForIdentifier(identifier))
        return;

    auto attachment = ensureAttachment(identifier);
    attachment->setContentType(contentType);
    m_attachmentIdentifierToAttachmentMap.set(identifier, attachment.copyRef());

    platformRegisterAttachment(WTFMove(attachment), preferredFileName, data);
}

void WebPageProxy::registerAttachmentIdentifierFromFilePath(const String& identifier, const String& contentType, const String& filePath)
{
    if (attachmentForIdentifier(identifier))
        return;

    auto attachment = ensureAttachment(identifier);
    attachment->setContentType(contentType);
    attachment->setFilePath(filePath);
    m_attachmentIdentifierToAttachmentMap.set(identifier, attachment.copyRef());

    platformRegisterAttachment(WTFMove(attachment), filePath);
}

void WebPageProxy::registerAttachmentIdentifier(const String& identifier)
{
    if (!attachmentForIdentifier(identifier))
        m_attachmentIdentifierToAttachmentMap.set(identifier, ensureAttachment(identifier));
}

void WebPageProxy::registerAttachmentsFromSerializedData(Vector<WebCore::SerializedAttachmentData>&& data)
{
    for (auto& serializedData : data) {
        auto identifier = WTFMove(serializedData.identifier);
        if (!attachmentForIdentifier(identifier))
            ensureAttachment(identifier)->updateFromSerializedRepresentation(WTFMove(serializedData.data), WTFMove(serializedData.mimeType));
    }
}

void WebPageProxy::cloneAttachmentData(const String& fromIdentifier, const String& toIdentifier)
{
    auto newAttachment = ensureAttachment(toIdentifier);
    auto existingAttachment = attachmentForIdentifier(fromIdentifier);
    if (!existingAttachment) {
        ASSERT_NOT_REACHED();
        return;
    }

    newAttachment->setContentType(existingAttachment->contentType());
    newAttachment->setFilePath(existingAttachment->filePath());

    platformCloneAttachment(existingAttachment.releaseNonNull(), WTFMove(newAttachment));
}

void WebPageProxy::invalidateAllAttachments()
{
    for (auto& attachment : m_attachmentIdentifierToAttachmentMap.values()) {
        if (attachment->insertionState() == API::Attachment::InsertionState::Inserted)
            didRemoveAttachment(attachment.get());
        attachment->invalidate();
    }
    m_attachmentIdentifierToAttachmentMap.clear();
}

void WebPageProxy::serializedAttachmentDataForIdentifiers(const Vector<String>& identifiers, CompletionHandler<void(Vector<WebCore::SerializedAttachmentData>&&)>&& completionHandler)
{
    Vector<WebCore::SerializedAttachmentData> serializedData;
    for (const auto& identifier : identifiers) {
        auto attachment = attachmentForIdentifier(identifier);
        if (!attachment)
            continue;

        auto data = attachment->createSerializedRepresentation();
        if (!data)
            continue;

        serializedData.append({ identifier, attachment->mimeType(), data.releaseNonNull() });
    }
    completionHandler(WTFMove(serializedData));
}

void WebPageProxy::didInvalidateDataForAttachment(API::Attachment& attachment)
{
    pageClient().didInvalidateDataForAttachment(attachment);
}

WebPageProxy::ShouldUpdateAttachmentAttributes WebPageProxy::willUpdateAttachmentAttributes(const API::Attachment& attachment)
{
#if HAVE(PENCILKIT)
    return m_editableImageController->willUpdateAttachmentAttributes(attachment);
#else
    return ShouldUpdateAttachmentAttributes::Yes;
#endif
}

#if !PLATFORM(COCOA)

void WebPageProxy::platformRegisterAttachment(Ref<API::Attachment>&&, const String&, const IPC::DataReference&)
{
}

void WebPageProxy::platformRegisterAttachment(Ref<API::Attachment>&&, const String&)
{
}

void WebPageProxy::platformCloneAttachment(Ref<API::Attachment>&&, Ref<API::Attachment>&&)
{
}

#endif

void WebPageProxy::didInsertAttachmentWithIdentifier(const String& identifier, const String& source, bool hasEnclosingImage)
{
    auto attachment = ensureAttachment(identifier);
    attachment->setHasEnclosingImage(hasEnclosingImage);
    attachment->setInsertionState(API::Attachment::InsertionState::Inserted);
    pageClient().didInsertAttachment(attachment.get(), source);

    if (!attachment->isEmpty() && hasEnclosingImage)
        updateAttachmentAttributes(attachment.get(), [] (auto) { });
}

void WebPageProxy::didRemoveAttachmentWithIdentifier(const String& identifier)
{
    if (auto attachment = attachmentForIdentifier(identifier))
        didRemoveAttachment(*attachment);
}

void WebPageProxy::didRemoveAttachment(API::Attachment& attachment)
{
    attachment.setInsertionState(API::Attachment::InsertionState::NotInserted);
    pageClient().didRemoveAttachment(attachment);
}

Ref<API::Attachment> WebPageProxy::ensureAttachment(const String& identifier)
{
    if (auto existingAttachment = attachmentForIdentifier(identifier))
        return *existingAttachment;

    auto attachment = API::Attachment::create(identifier, *this);
    m_attachmentIdentifierToAttachmentMap.set(identifier, attachment.copyRef());
    return attachment;
}

#endif // ENABLE(ATTACHMENT_ELEMENT)

#if ENABLE(APPLICATION_MANIFEST)
void WebPageProxy::getApplicationManifest(Function<void(const Optional<WebCore::ApplicationManifest>&, CallbackBase::Error)>&& callbackFunction)
{
    if (!hasRunningProcess()) {
        callbackFunction(WTF::nullopt, CallbackBase::Error::Unknown);
        return;
    }

    auto callbackID = m_callbacks.put(WTFMove(callbackFunction), m_process->throttler().backgroundActivityToken());
    m_loadDependentStringCallbackIDs.add(callbackID);
    m_process->send(Messages::WebPage::GetApplicationManifest(callbackID), m_pageID);
}
#endif

namespace {
enum class CompletionCondition {
    Cancellation,
    Error,
    Success,
    Timeout,
};
struct MessageType {
    CompletionCondition condition;
    Seconds seconds;
    String message;
};
}

void WebPageProxy::reportPageLoadResult(const ResourceError& error)
{
    static const NeverDestroyed<Vector<MessageType>> messages(std::initializer_list<MessageType> {
        { CompletionCondition::Cancellation, 2_s, DiagnosticLoggingKeys::canceledLessThan2SecondsKey() },
        { CompletionCondition::Cancellation, 5_s, DiagnosticLoggingKeys::canceledLessThan5SecondsKey() },
        { CompletionCondition::Cancellation, 20_s, DiagnosticLoggingKeys::canceledLessThan20SecondsKey() },
        { CompletionCondition::Cancellation, Seconds::infinity(), DiagnosticLoggingKeys::canceledMoreThan20SecondsKey() },

        { CompletionCondition::Error, 2_s, DiagnosticLoggingKeys::failedLessThan2SecondsKey() },
        { CompletionCondition::Error, 5_s, DiagnosticLoggingKeys::failedLessThan5SecondsKey() },
        { CompletionCondition::Error, 20_s, DiagnosticLoggingKeys::failedLessThan20SecondsKey() },
        { CompletionCondition::Error, Seconds::infinity(), DiagnosticLoggingKeys::failedMoreThan20SecondsKey() },

        { CompletionCondition::Success, 2_s, DiagnosticLoggingKeys::succeededLessThan2SecondsKey() },
        { CompletionCondition::Success, 5_s, DiagnosticLoggingKeys::succeededLessThan5SecondsKey() },
        { CompletionCondition::Success, 20_s, DiagnosticLoggingKeys::succeededLessThan20SecondsKey() },
        { CompletionCondition::Success, Seconds::infinity(), DiagnosticLoggingKeys::succeededMoreThan20SecondsKey() },

        { CompletionCondition::Timeout, Seconds::infinity(), DiagnosticLoggingKeys::timedOutKey() }
        });

    if (!m_pageLoadStart)
        return;

    auto pageLoadTime = MonotonicTime::now() - *m_pageLoadStart;
    m_pageLoadStart = WTF::nullopt;

    CompletionCondition condition { CompletionCondition::Success };
    if (error.isCancellation())
        condition = CompletionCondition::Cancellation;
    else if (error.isTimeout())
        condition = CompletionCondition::Timeout;
    else if (!error.isNull() || error.errorCode())
        condition = CompletionCondition::Error;

    for (auto& messageItem : messages.get()) {
        if (condition == messageItem.condition && pageLoadTime < messageItem.seconds) {
            logDiagnosticMessage(DiagnosticLoggingKeys::telemetryPageLoadKey(), messageItem.message, ShouldSample::No);
            logDiagnosticMessage(DiagnosticLoggingKeys::telemetryPageLoadKey(), DiagnosticLoggingKeys::occurredKey(), ShouldSample::No);
            break;
        }
    }
}

void WebPageProxy::setDefersLoadingForTesting(bool defersLoading)
{
    m_process->send(Messages::WebPage::SetDefersLoading(defersLoading), m_pageID);
}

void WebPageProxy::getIsViewVisible(bool& result)
{
    result = isViewVisible();
}

void WebPageProxy::updateCurrentModifierState()
{
#if PLATFORM(MAC) && ENABLE(WEBPROCESS_WINDOWSERVER_BLOCKING) || PLATFORM(IOS_FAMILY)
    auto modifiers = PlatformKeyboardEvent::currentStateOfModifierKeys();
    m_process->send(Messages::WebPage::UpdateCurrentModifierState(modifiers), m_pageID);
#endif
}

bool WebPageProxy::checkURLReceivedFromCurrentOrPreviousWebProcess(WebProcessProxy& process, const String& urlString)
{
    return checkURLReceivedFromCurrentOrPreviousWebProcess(process, URL(URL(), urlString));
}

bool WebPageProxy::checkURLReceivedFromCurrentOrPreviousWebProcess(WebProcessProxy& process, const URL& url)
{
    if (!url.isLocalFile())
        return true;

    if (m_mayHaveUniversalFileReadSandboxExtension)
        return true;

    String path = url.fileSystemPath();
    auto startsWithURLPath = [&path](const String& visitedPath) {
        return path.startsWith(visitedPath);
    };

    auto localPathsEnd = m_previouslyVisitedPaths.end();
    if (std::find_if(m_previouslyVisitedPaths.begin(), localPathsEnd, startsWithURLPath) != localPathsEnd)
        return true;

    return process.checkURLReceivedFromWebProcess(url);
}

void WebPageProxy::addPreviouslyVisitedPath(const String& path)
{
    m_previouslyVisitedPaths.add(path);
}

void WebPageProxy::willAcquireUniversalFileReadSandboxExtension(WebProcessProxy& process)
{
    m_mayHaveUniversalFileReadSandboxExtension = true;
    process.willAcquireUniversalFileReadSandboxExtension();
}

void WebPageProxy::simulateDeviceOrientationChange(double alpha, double beta, double gamma)
{
    m_process->send(Messages::WebPage::SimulateDeviceOrientationChange(alpha, beta, gamma), m_pageID);
}

#if ENABLE(DATA_DETECTION)

void WebPageProxy::detectDataInAllFrames(WebCore::DataDetectorTypes types, CompletionHandler<void(const DataDetectionResult&)>&& completionHandler)
{
    m_process->connection()->sendWithAsyncReply(Messages::WebPage::DetectDataInAllFrames(static_cast<uint64_t>(types)), WTFMove(completionHandler), m_pageID);
}

void WebPageProxy::removeDataDetectedLinks(CompletionHandler<void(const DataDetectionResult&)>&& completionHandler)
{
    m_process->connection()->sendWithAsyncReply(Messages::WebPage::RemoveDataDetectedLinks(), WTFMove(completionHandler), m_pageID);
}

#endif

void WebPageProxy::dumpAdClickAttribution(CompletionHandler<void(const String&)>&& completionHandler)
{
    if (auto* networkProcess = m_process->processPool().networkProcess()) {
        if (!networkProcess->canSendMessage()) {
            completionHandler(emptyString());
            return;
        }
        networkProcess->sendWithAsyncReply(Messages::NetworkProcess::DumpAdClickAttribution(m_websiteDataStore->sessionID()), WTFMove(completionHandler));
    }
}

void WebPageProxy::clearAdClickAttribution(CompletionHandler<void()>&& completionHandler)
{
    if (auto* networkProcess = m_process->processPool().networkProcess()) {
        if (!networkProcess->canSendMessage()) {
            completionHandler();
            return;
        }
        networkProcess->sendWithAsyncReply(Messages::NetworkProcess::ClearAdClickAttribution(m_websiteDataStore->sessionID()), WTFMove(completionHandler));
    }
}

void WebPageProxy::setAdClickAttributionOverrideTimerForTesting(bool value, CompletionHandler<void()>&& completionHandler)
{
    if (auto* networkProcess = m_process->processPool().networkProcess()) {
        if (!networkProcess->canSendMessage()) {
            completionHandler();
            return;
        }
        networkProcess->sendWithAsyncReply(Messages::NetworkProcess::SetAdClickAttributionOverrideTimerForTesting(m_websiteDataStore->sessionID(), value), WTFMove(completionHandler));
    }
}

void WebPageProxy::setAdClickAttributionConversionURLForTesting(const URL& url, CompletionHandler<void()>&& completionHandler)
{
    if (auto* networkProcess = m_process->processPool().networkProcess()) {
        if (!networkProcess->canSendMessage()) {
            completionHandler();
            return;
        }
        networkProcess->sendWithAsyncReply(Messages::NetworkProcess::SetAdClickAttributionConversionURLForTesting(m_websiteDataStore->sessionID(), url), WTFMove(completionHandler));
    }
}

void WebPageProxy::markAdClickAttributionsAsExpiredForTesting(CompletionHandler<void()>&& completionHandler)
{
    if (auto* networkProcess = m_process->processPool().networkProcess()) {
        if (!networkProcess->canSendMessage()) {
            completionHandler();
            return;
        }
        networkProcess->sendWithAsyncReply(Messages::NetworkProcess::MarkAdClickAttributionsAsExpiredForTesting(m_websiteDataStore->sessionID()), WTFMove(completionHandler));
    }
}

#if ENABLE(SPEECH_SYNTHESIS)

void WebPageProxy::resetSpeechSynthesizer()
{
    if (!m_speechSynthesisData)
        return;
    
    auto& synthesisData = speechSynthesisData();
    synthesisData.speakingFinishedCompletionHandler = nullptr;
    synthesisData.speakingStartedCompletionHandler = nullptr;
    synthesisData.speakingPausedCompletionHandler = nullptr;
    synthesisData.speakingResumedCompletionHandler = nullptr;
    if (synthesisData.synthesizer)
        synthesisData.synthesizer->resetState();
}

WebPageProxy::SpeechSynthesisData& WebPageProxy::speechSynthesisData()
{
    if (!m_speechSynthesisData)
        m_speechSynthesisData = SpeechSynthesisData { makeUnique<PlatformSpeechSynthesizer>(this), nullptr, nullptr, nullptr, nullptr, nullptr };
    return *m_speechSynthesisData;
}

void WebPageProxy::speechSynthesisVoiceList(CompletionHandler<void(Vector<WebSpeechSynthesisVoice>&&)>&& completionHandler)
{
    auto& voiceList = speechSynthesisData().synthesizer->voiceList();
    Vector<WebSpeechSynthesisVoice> result;
    result.reserveInitialCapacity(voiceList.size());
    for (auto& voice : voiceList)
        result.uncheckedAppend(WebSpeechSynthesisVoice { voice->voiceURI(), voice->name(), voice->lang(), voice->localService(), voice->isDefault() });
    completionHandler(WTFMove(result));
}

void WebPageProxy::speechSynthesisSetFinishedCallback(CompletionHandler<void()>&& completionHandler)
{
    speechSynthesisData().speakingFinishedCompletionHandler = WTFMove(completionHandler);
}

void WebPageProxy::speechSynthesisSpeak(const String& text, const String& lang, float volume, float rate, float pitch, MonotonicTime startTime, const String& voiceURI, const String& voiceName, const String& voiceLang, bool localService, bool defaultVoice, CompletionHandler<void()>&& completionHandler)
{
    auto voice = WebCore::PlatformSpeechSynthesisVoice::create(voiceURI, voiceName, voiceLang, localService, defaultVoice);
    auto utterance = WebCore::PlatformSpeechSynthesisUtterance::create(*this);
    utterance->setText(text);
    utterance->setLang(lang);
    utterance->setVolume(volume);
    utterance->setRate(rate);
    utterance->setPitch(pitch);
    utterance->setVoice(&voice.get());

    speechSynthesisData().speakingStartedCompletionHandler = WTFMove(completionHandler);
    speechSynthesisData().utterance = WTFMove(utterance);
    speechSynthesisData().synthesizer->speak(m_speechSynthesisData->utterance.get());
}

void WebPageProxy::speechSynthesisCancel()
{
    speechSynthesisData().synthesizer->cancel();
}

void WebPageProxy::speechSynthesisPause(CompletionHandler<void()>&& completionHandler)
{
    speechSynthesisData().speakingPausedCompletionHandler = WTFMove(completionHandler);
    speechSynthesisData().synthesizer->pause();
}

void WebPageProxy::speechSynthesisResume(CompletionHandler<void()>&& completionHandler)
{
    speechSynthesisData().speakingResumedCompletionHandler = WTFMove(completionHandler);
    speechSynthesisData().synthesizer->resume();
}
#endif // ENABLE(SPEECH_SYNTHESIS)

#if !PLATFORM(IOS_FAMILY)

WebContentMode WebPageProxy::effectiveContentModeAfterAdjustingPolicies(API::WebsitePolicies&, const WebCore::ResourceRequest&)
{
    return WebContentMode::Recommended;
}

#endif // !PLATFORM(IOS_FAMILY)

void WebPageProxy::addObserver(WebViewDidMoveToWindowObserver& observer)
{
    auto result = m_webViewDidMoveToWindowObservers.add(&observer, makeWeakPtr(observer));
    ASSERT_UNUSED(result, result.isNewEntry);
}

void WebPageProxy::removeObserver(WebViewDidMoveToWindowObserver& observer)
{
    auto result = m_webViewDidMoveToWindowObservers.remove(&observer);
    ASSERT_UNUSED(result, result);
}

void WebPageProxy::webViewDidMoveToWindow()
{
    auto observersCopy = m_webViewDidMoveToWindowObservers;
    for (const auto& observer : observersCopy) {
        if (!observer.value)
            continue;
        observer.value->webViewDidMoveToWindow();
    }
}

void WebPageProxy::textInputContextsInRect(WebCore::FloatRect rect, CompletionHandler<void(const Vector<WebKit::TextInputContext>&)>&& completionHandler)
{
    if (!hasRunningProcess()) {
        completionHandler({ });
        return;
    }

    m_process->connection()->sendWithAsyncReply(Messages::WebPage::TextInputContextsInRect(rect), WTFMove(completionHandler), m_pageID);
}

void WebPageProxy::focusTextInputContext(const TextInputContext& context, CompletionHandler<void(bool)>&& completionHandler)
{
    if (!hasRunningProcess()) {
        completionHandler(false);
        return;
    }

    m_process->connection()->sendWithAsyncReply(Messages::WebPage::FocusTextInputContext(context), WTFMove(completionHandler), m_pageID);
}

Logger& WebPageProxy::logger()
{
    if (!m_logger) {
        m_logger = Logger::create(this);
        m_logger->setEnabled(this, isAlwaysOnLoggingAllowed());
    }

    return *m_logger;
}

void WebPageProxy::configureLoggingChannel(const String& channelName, WTFLogChannelState state, WTFLogLevel level)
{
#if !RELEASE_LOG_DISABLED
    auto* channel = getLogChannel(channelName);
    if  (!channel)
        return;

    channel->state = state;
    channel->level = level;
#else
    UNUSED_PARAM(channelName);
    UNUSED_PARAM(state);
    UNUSED_PARAM(level);
#endif
}

#if HAVE(APP_SSO)
void WebPageProxy::decidePolicyForSOAuthorizationLoad(const String& extension, CompletionHandler<void(SOAuthorizationLoadPolicy)>&& completionHandler)
{
    m_navigationClient->decidePolicyForSOAuthorizationLoad(*this, SOAuthorizationLoadPolicy::Allow, extension, WTFMove(completionHandler));
}
#endif

} // namespace WebKit

#undef MERGE_WHEEL_EVENTS
#undef MESSAGE_CHECK
#undef MESSAGE_CHECK_URL
#undef RELEASE_LOG_IF_ALLOWED
