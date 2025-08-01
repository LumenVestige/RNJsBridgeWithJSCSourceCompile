/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Portions Copyright (c) 2010 Motorola Mobility, Inc.  All rights reserved.
 * Copyright (C) 2011 Igalia S.L.
 * Copyright (C) 2013 Gustavo Noronha Silva <gns@gnome.org>.
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
#include "WebKitWebViewBase.h"

#include "APIPageConfiguration.h"
#include "AcceleratedBackingStore.h"
#include "DrawingAreaProxyCoordinatedGraphics.h"
#include "InputMethodFilter.h"
#include "KeyBindingTranslator.h"
#include "NativeWebKeyboardEvent.h"
#include "NativeWebMouseEvent.h"
#include "NativeWebWheelEvent.h"
#include "PageClientImpl.h"
#include "ViewGestureController.h"
#include "WebEventFactory.h"
#include "WebInspectorProxy.h"
#include "WebKit2Initialize.h"
#include "WebKitEmojiChooser.h"
#include "WebKitWebViewAccessible.h"
#include "WebKitWebViewBasePrivate.h"
#include "WebPageGroup.h"
#include "WebPageProxy.h"
#include "WebPreferences.h"
#include "WebProcessPool.h"
#include "WebUserContentControllerProxy.h"
#include <WebCore/ActivityState.h>
#include <WebCore/CairoUtilities.h>
#include <WebCore/GUniquePtrGtk.h>
#include <WebCore/GtkUtilities.h>
#include <WebCore/NotImplemented.h>
#include <WebCore/PasteboardHelper.h>
#include <WebCore/PlatformDisplay.h>
#include <WebCore/RefPtrCairo.h>
#include <WebCore/Region.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n-lib.h>
#include <memory>
#include <pal/system/SleepDisabler.h>
#include <wtf/Compiler.h>
#include <wtf/HashMap.h>
#include <wtf/glib/GRefPtr.h>
#include <wtf/glib/RunLoopSourcePriority.h>
#include <wtf/glib/WTFGType.h>
#include <wtf/text/CString.h>

#if ENABLE(FULLSCREEN_API)
#include "WebFullScreenManagerProxy.h"
#endif

#if PLATFORM(X11)
#include <gdk/gdkx.h>
#endif

using namespace WebKit;
using namespace WebCore;

struct ClickCounter {
public:
    void reset()
    {
        currentClickCount = 0;
        previousClickPoint = IntPoint();
        previousClickTime = 0;
        previousClickButton = 0;
    }

    int currentClickCountForGdkButtonEvent(GdkEvent* event)
    {
        int doubleClickDistance = 250;
        int doubleClickTime = 5;
        g_object_get(gtk_settings_get_for_screen(gdk_event_get_screen(event)),
            "gtk-double-click-distance", &doubleClickDistance, "gtk-double-click-time", &doubleClickTime, nullptr);

        // GTK+ only counts up to triple clicks, but WebCore wants to know about
        // quadruple clicks, quintuple clicks, ad infinitum. Here, we replicate the
        // GDK logic for counting clicks.
        guint32 eventTime = gdk_event_get_time(event);
        if (!eventTime) {
            // Real events always have a non-zero time, but events synthesized
            // by the WTR do not and we must calculate a time manually. This time
            // is not calculated in the WTR, because GTK+ does not work well with
            // anything other than GDK_CURRENT_TIME on synthesized events.
            GTimeVal timeValue;
            g_get_current_time(&timeValue);
            eventTime = (timeValue.tv_sec * 1000) + (timeValue.tv_usec / 1000);
        }

        GdkEventType type;
        guint button;
        double x, y;
        gdk_event_get_coords(event, &x, &y);
        gdk_event_get_button(event, &button);
        type = gdk_event_get_event_type(event);

        if ((type == GDK_2BUTTON_PRESS || type == GDK_3BUTTON_PRESS)
            || ((std::abs(x - previousClickPoint.x()) < doubleClickDistance)
                && (std::abs(y - previousClickPoint.y()) < doubleClickDistance)
                && (eventTime - previousClickTime < static_cast<unsigned>(doubleClickTime))
                && (button == previousClickButton)))
            currentClickCount++;
        else
            currentClickCount = 1;

        previousClickPoint = IntPoint(x, y);
        previousClickButton = button;
        previousClickTime = eventTime;

        return currentClickCount;
    }

private:
    int currentClickCount;
    IntPoint previousClickPoint;
    unsigned previousClickButton;
    int previousClickTime;
};

typedef HashMap<GtkWidget*, IntRect> WebKitWebViewChildrenMap;
typedef HashMap<uint32_t, GUniquePtr<GdkEvent>> TouchEventsMap;

struct _WebKitWebViewBasePrivate {
    _WebKitWebViewBasePrivate()
        : updateActivityStateTimer(RunLoop::main(), this, &_WebKitWebViewBasePrivate::updateActivityStateTimerFired)
#if GTK_CHECK_VERSION(3, 24, 0)
        , releaseEmojiChooserTimer(RunLoop::main(), this, &_WebKitWebViewBasePrivate::releaseEmojiChooserTimerFired)
#endif
    {
#if GTK_CHECK_VERSION(3, 24, 0)
        releaseEmojiChooserTimer.setPriority(RunLoopSourcePriority::ReleaseUnusedResourcesTimer);
#endif
    }

    void updateActivityStateTimerFired()
    {
        if (!pageProxy)
            return;
        pageProxy->activityStateDidChange(activityStateFlagsToUpdate);
        activityStateFlagsToUpdate = { };
    }

#if GTK_CHECK_VERSION(3, 24, 0)
    void releaseEmojiChooserTimerFired()
    {
        if (emojiChooser) {
            gtk_widget_destroy(emojiChooser);
            emojiChooser = nullptr;
        }
    }
#endif

    WebKitWebViewChildrenMap children;
    std::unique_ptr<PageClientImpl> pageClient;
    RefPtr<WebPageProxy> pageProxy;
    bool shouldForwardNextKeyEvent { false };
    bool shouldForwardNextWheelEvent { false };
    ClickCounter clickCounter;
    CString tooltipText;
    IntRect tooltipArea;
    GRefPtr<AtkObject> accessible;
    GtkWidget* dialog { nullptr };
    GtkWidget* inspectorView { nullptr };
    AttachmentSide inspectorAttachmentSide { AttachmentSide::Bottom };
    unsigned inspectorViewSize { 0 };
    GUniquePtr<GdkEvent> contextMenuEvent;
    WebContextMenuProxyGtk* activeContextMenuProxy { nullptr };
    InputMethodFilter inputMethodFilter;
    KeyBindingTranslator keyBindingTranslator;
    TouchEventsMap touchEvents;
    IntSize contentsSize;

    GtkWindow* toplevelOnScreenWindow { nullptr };
    unsigned long toplevelFocusInEventID { 0 };
    unsigned long toplevelFocusOutEventID { 0 };
    unsigned long toplevelWindowStateEventID { 0 };
    unsigned long toplevelWindowRealizedID { 0 };
    unsigned long themeChangedID { 0 };
    unsigned long applicationPreferDarkThemeID { 0 };

    // View State.
    OptionSet<ActivityState::Flag> activityState;
    OptionSet<ActivityState::Flag> activityStateFlagsToUpdate;
    RunLoop::Timer<WebKitWebViewBasePrivate> updateActivityStateTimer;

#if ENABLE(FULLSCREEN_API)
    bool fullScreenModeActive { false };
    std::unique_ptr<PAL::SleepDisabler> sleepDisabler;
#endif

    std::unique_ptr<AcceleratedBackingStore> acceleratedBackingStore;

#if ENABLE(DRAG_SUPPORT)
    std::unique_ptr<DragAndDropHandler> dragAndDropHandler;
#endif

    std::unique_ptr<GestureController> gestureController;
    std::unique_ptr<ViewGestureController> viewGestureController;
    bool isBackForwardNavigationGestureEnabled { false };

#if GTK_CHECK_VERSION(3, 24, 0)
    GtkWidget* emojiChooser;
    CompletionHandler<void(String)> emojiChooserCompletionHandler;
    RunLoop::Timer<WebKitWebViewBasePrivate> releaseEmojiChooserTimer;
#endif
};

WEBKIT_DEFINE_TYPE(WebKitWebViewBase, webkit_web_view_base, GTK_TYPE_CONTAINER)

static void webkitWebViewBaseScheduleUpdateActivityState(WebKitWebViewBase* webViewBase, OptionSet<ActivityState::Flag> flagsToUpdate)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    priv->activityStateFlagsToUpdate.add(flagsToUpdate);
    if (priv->updateActivityStateTimer.isActive())
        return;

    priv->updateActivityStateTimer.startOneShot(0_s);
}

static gboolean toplevelWindowFocusInEvent(GtkWidget* widget, GdkEventFocus*, WebKitWebViewBase* webViewBase)
{
    // Spurious focus in events can occur when the window is hidden.
    if (!gtk_widget_get_visible(widget))
        return FALSE;

    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if (priv->activityState & ActivityState::WindowIsActive)
        return FALSE;

    priv->activityState.add(ActivityState::WindowIsActive);
    webkitWebViewBaseScheduleUpdateActivityState(webViewBase, ActivityState::WindowIsActive);

    return FALSE;
}

static gboolean toplevelWindowFocusOutEvent(GtkWidget*, GdkEventFocus*, WebKitWebViewBase* webViewBase)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if (!(priv->activityState & ActivityState::WindowIsActive))
        return FALSE;

    priv->activityState.remove(ActivityState::WindowIsActive);
    webkitWebViewBaseScheduleUpdateActivityState(webViewBase, ActivityState::WindowIsActive);

    return FALSE;
}

static gboolean toplevelWindowStateEvent(GtkWidget*, GdkEventWindowState* event, WebKitWebViewBase* webViewBase)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if (!(event->changed_mask & GDK_WINDOW_STATE_ICONIFIED))
        return FALSE;

    bool visible = !(event->new_window_state & GDK_WINDOW_STATE_ICONIFIED);
    if ((visible && priv->activityState & ActivityState::IsVisible) || (!visible && !(priv->activityState & ActivityState::IsVisible)))
        return FALSE;

    if (visible)
        priv->activityState.add(ActivityState::IsVisible);
    else
        priv->activityState.remove(ActivityState::IsVisible);
    webkitWebViewBaseScheduleUpdateActivityState(webViewBase, ActivityState::IsVisible);

    return FALSE;
}

static void themeChanged(WebKitWebViewBase* webViewBase)
{
    webViewBase->priv->pageProxy->themeDidChange();
}

static void applicationPreferDarkThemeChanged(WebKitWebViewBase* webViewBase)
{
    webViewBase->priv->pageProxy->effectiveAppearanceDidChange();
}

static void toplevelWindowRealized(WebKitWebViewBase* webViewBase)
{
    gtk_widget_realize(GTK_WIDGET(webViewBase));

    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if (priv->toplevelWindowRealizedID) {
        g_signal_handler_disconnect(priv->toplevelOnScreenWindow, priv->toplevelWindowRealizedID);
        priv->toplevelWindowRealizedID = 0;
    }
}

static void webkitWebViewBaseSetToplevelOnScreenWindow(WebKitWebViewBase* webViewBase, GtkWindow* window)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if (priv->toplevelOnScreenWindow == window)
        return;

    if (priv->toplevelFocusInEventID) {
        g_signal_handler_disconnect(priv->toplevelOnScreenWindow, priv->toplevelFocusInEventID);
        priv->toplevelFocusInEventID = 0;
    }
    if (priv->toplevelFocusOutEventID) {
        g_signal_handler_disconnect(priv->toplevelOnScreenWindow, priv->toplevelFocusOutEventID);
        priv->toplevelFocusOutEventID = 0;
    }
    if (priv->toplevelWindowStateEventID) {
        g_signal_handler_disconnect(priv->toplevelOnScreenWindow, priv->toplevelWindowStateEventID);
        priv->toplevelWindowStateEventID = 0;
    }
    if (priv->toplevelWindowRealizedID) {
        g_signal_handler_disconnect(priv->toplevelOnScreenWindow, priv->toplevelWindowRealizedID);
        priv->toplevelWindowRealizedID = 0;
    }
    if (priv->themeChangedID || priv->applicationPreferDarkThemeID) {
        auto* settings = gtk_widget_get_settings(GTK_WIDGET(priv->toplevelOnScreenWindow));
        if (priv->themeChangedID) {
            g_signal_handler_disconnect(settings, priv->themeChangedID);
            priv->themeChangedID = 0;
        }
        if (priv->applicationPreferDarkThemeID) {
            g_signal_handler_disconnect(settings, priv->applicationPreferDarkThemeID);
            priv->applicationPreferDarkThemeID = 0;
        }
    }

    priv->toplevelOnScreenWindow = window;

    if (!priv->toplevelOnScreenWindow) {
        OptionSet<ActivityState::Flag> flagsToUpdate;
        if (priv->activityState & ActivityState::IsInWindow) {
            priv->activityState.remove(ActivityState::IsInWindow);
            flagsToUpdate.add(ActivityState::IsInWindow);
        }
        if (priv->activityState & ActivityState::WindowIsActive) {
            priv->activityState.remove(ActivityState::WindowIsActive);
            flagsToUpdate.add(ActivityState::IsInWindow);
        }
        if (flagsToUpdate)
            webkitWebViewBaseScheduleUpdateActivityState(webViewBase, flagsToUpdate);

        return;
    }

    priv->toplevelFocusInEventID =
        g_signal_connect(priv->toplevelOnScreenWindow, "focus-in-event",
                         G_CALLBACK(toplevelWindowFocusInEvent), webViewBase);
    priv->toplevelFocusOutEventID =
        g_signal_connect(priv->toplevelOnScreenWindow, "focus-out-event",
                         G_CALLBACK(toplevelWindowFocusOutEvent), webViewBase);
    priv->toplevelWindowStateEventID =
        g_signal_connect(priv->toplevelOnScreenWindow, "window-state-event", G_CALLBACK(toplevelWindowStateEvent), webViewBase);

    auto* settings = gtk_widget_get_settings(GTK_WIDGET(priv->toplevelOnScreenWindow));
    priv->themeChangedID =
        g_signal_connect_swapped(settings, "notify::gtk-theme-name", G_CALLBACK(themeChanged), webViewBase);
    priv->applicationPreferDarkThemeID =
        g_signal_connect_swapped(settings, "notify::gtk-application-prefer-dark-theme", G_CALLBACK(applicationPreferDarkThemeChanged), webViewBase);

    if (gtk_widget_get_realized(GTK_WIDGET(window)))
        gtk_widget_realize(GTK_WIDGET(webViewBase));
    else
        priv->toplevelWindowRealizedID = g_signal_connect_swapped(window, "realize", G_CALLBACK(toplevelWindowRealized), webViewBase);
}

static void webkitWebViewBaseRealize(GtkWidget* widget)
{
    WebKitWebViewBase* webView = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webView->priv;

    gtk_widget_set_realized(widget, TRUE);

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    GdkWindowAttr attributes;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual(widget);
    attributes.event_mask = GDK_VISIBILITY_NOTIFY_MASK
        | GDK_EXPOSURE_MASK
        | GDK_BUTTON_PRESS_MASK
        | GDK_BUTTON_RELEASE_MASK
        | GDK_SCROLL_MASK
        | GDK_SMOOTH_SCROLL_MASK
        | GDK_POINTER_MOTION_MASK
        | GDK_ENTER_NOTIFY_MASK
        | GDK_LEAVE_NOTIFY_MASK
        | GDK_KEY_PRESS_MASK
        | GDK_KEY_RELEASE_MASK
        | GDK_BUTTON_MOTION_MASK
        | GDK_BUTTON1_MOTION_MASK
        | GDK_BUTTON2_MOTION_MASK
        | GDK_BUTTON3_MOTION_MASK
        | GDK_TOUCH_MASK;
    attributes.event_mask |= GDK_TOUCHPAD_GESTURE_MASK;

    gint attributesMask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

    GdkWindow* window = gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributesMask);
    gtk_widget_set_window(widget, window);
    gdk_window_set_user_data(window, widget);

    gtk_im_context_set_client_window(priv->inputMethodFilter.context(), window);

    if (priv->acceleratedBackingStore)
        priv->acceleratedBackingStore->realize();
}

static void webkitWebViewBaseUnrealize(GtkWidget* widget)
{
    WebKitWebViewBase* webView = WEBKIT_WEB_VIEW_BASE(widget);
    gtk_im_context_set_client_window(webView->priv->inputMethodFilter.context(), nullptr);

    if (webView->priv->acceleratedBackingStore)
        webView->priv->acceleratedBackingStore->unrealize();

    GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->unrealize(widget);
}

static bool webkitWebViewChildIsInternalWidget(WebKitWebViewBase* webViewBase, GtkWidget* widget)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    return widget == priv->inspectorView || widget == priv->dialog;
}

static void webkitWebViewBaseContainerAdd(GtkContainer* container, GtkWidget* widget)
{
    WebKitWebViewBase* webView = WEBKIT_WEB_VIEW_BASE(container);
    WebKitWebViewBasePrivate* priv = webView->priv;

    // Internal widgets like the web inspector and authentication dialog have custom
    // allocations so we don't need to add them to our list of children.
    if (!webkitWebViewChildIsInternalWidget(webView, widget)) {
        GtkAllocation childAllocation;
        gtk_widget_get_allocation(widget, &childAllocation);
        priv->children.set(widget, childAllocation);
    }

    gtk_widget_set_parent(widget, GTK_WIDGET(container));
}

void webkitWebViewBaseAddDialog(WebKitWebViewBase* webViewBase, GtkWidget* dialog)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    priv->dialog = dialog;
    gtk_container_add(GTK_CONTAINER(webViewBase), dialog);
    gtk_widget_show(dialog);

    // We need to draw the shadow over the widget.
    gtk_widget_queue_draw(GTK_WIDGET(webViewBase));
}

void webkitWebViewBaseAddWebInspector(WebKitWebViewBase* webViewBase, GtkWidget* inspector, AttachmentSide attachmentSide)
{
    if (webViewBase->priv->inspectorView == inspector && webViewBase->priv->inspectorAttachmentSide == attachmentSide)
        return;

    webViewBase->priv->inspectorAttachmentSide = attachmentSide;

    if (webViewBase->priv->inspectorView == inspector) {
        gtk_widget_queue_resize(GTK_WIDGET(webViewBase));
        return;
    }

    webViewBase->priv->inspectorView = inspector;
    gtk_container_add(GTK_CONTAINER(webViewBase), inspector);
}

static void webkitWebViewBaseContainerRemove(GtkContainer* container, GtkWidget* widget)
{
    WebKitWebViewBase* webView = WEBKIT_WEB_VIEW_BASE(container);
    WebKitWebViewBasePrivate* priv = webView->priv;
    GtkWidget* widgetContainer = GTK_WIDGET(container);

    gboolean wasVisible = gtk_widget_get_visible(widget);
    gtk_widget_unparent(widget);

    if (priv->inspectorView == widget) {
        priv->inspectorView = 0;
        priv->inspectorViewSize = 0;
    } else if (priv->dialog == widget) {
        priv->dialog = nullptr;
        if (gtk_widget_get_visible(widgetContainer))
            gtk_widget_grab_focus(widgetContainer);
    } else {
        ASSERT(priv->children.contains(widget));
        priv->children.remove(widget);
    }
    if (wasVisible && gtk_widget_get_visible(widgetContainer))
        gtk_widget_queue_resize(widgetContainer);
}

static void webkitWebViewBaseContainerForall(GtkContainer* container, gboolean includeInternals, GtkCallback callback, gpointer callbackData)
{
    WebKitWebViewBase* webView = WEBKIT_WEB_VIEW_BASE(container);
    WebKitWebViewBasePrivate* priv = webView->priv;

    for (const auto& child : copyToVector(priv->children.keys())) {
        if (priv->children.contains(child))
            (*callback)(child, callbackData);
    }

    if (includeInternals && priv->inspectorView)
        (*callback)(priv->inspectorView, callbackData);

    if (includeInternals && priv->dialog)
        (*callback)(priv->dialog, callbackData);
}

void webkitWebViewBaseChildMoveResize(WebKitWebViewBase* webView, GtkWidget* child, const IntRect& childRect)
{
    const IntRect& geometry = webView->priv->children.get(child);
    if (geometry == childRect)
        return;

    webView->priv->children.set(child, childRect);
    gtk_widget_queue_resize_no_redraw(GTK_WIDGET(webView));
}

#if GTK_CHECK_VERSION(3, 24, 0)
static void webkitWebViewBaseCompleteEmojiChooserRequest(WebKitWebViewBase* webView, const String& text)
{
    if (auto completionHandler = std::exchange(webView->priv->emojiChooserCompletionHandler, nullptr))
        completionHandler(text);
}
#endif

static void webkitWebViewBaseDispose(GObject* gobject)
{
    WebKitWebViewBase* webView = WEBKIT_WEB_VIEW_BASE(gobject);
    webkitWebViewBaseSetToplevelOnScreenWindow(webView, nullptr);
    if (webView->priv->accessible)
        webkitWebViewAccessibleSetWebView(WEBKIT_WEB_VIEW_ACCESSIBLE(webView->priv->accessible.get()), nullptr);
#if GTK_CHECK_VERSION(3, 24, 0)
    webkitWebViewBaseCompleteEmojiChooserRequest(webView, emptyString());
#endif
    webView->priv->pageProxy->close();
    webView->priv->acceleratedBackingStore = nullptr;
    webView->priv->sleepDisabler = nullptr;
    G_OBJECT_CLASS(webkit_web_view_base_parent_class)->dispose(gobject);
}

static void webkitWebViewBaseConstructed(GObject* object)
{
    G_OBJECT_CLASS(webkit_web_view_base_parent_class)->constructed(object);

    GtkWidget* viewWidget = GTK_WIDGET(object);
    gtk_widget_set_can_focus(viewWidget, TRUE);
    gtk_drag_dest_set(viewWidget, static_cast<GtkDestDefaults>(0), nullptr, 0,
        static_cast<GdkDragAction>(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_PRIVATE));
    gtk_drag_dest_set_target_list(viewWidget, PasteboardHelper::singleton().targetList());

    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(object)->priv;
    priv->pageClient = makeUnique<PageClientImpl>(viewWidget);
    priv->dialog = nullptr;
}

static gboolean webkitWebViewBaseDraw(GtkWidget* widget, cairo_t* cr)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    auto* drawingArea = static_cast<DrawingAreaProxyCoordinatedGraphics*>(webViewBase->priv->pageProxy->drawingArea());
    if (!drawingArea)
        return FALSE;

    GdkRectangle clipRect;
    if (!gdk_cairo_get_clip_rectangle(cr, &clipRect))
        return FALSE;

    bool showingNavigationSnapshot = webViewBase->priv->pageProxy->isShowingNavigationGestureSnapshot();
    if (showingNavigationSnapshot)
        cairo_push_group(cr);

    if (drawingArea->isInAcceleratedCompositingMode()) {
        ASSERT(webViewBase->priv->acceleratedBackingStore);
        webViewBase->priv->acceleratedBackingStore->paint(cr, clipRect);
    } else {
        WebCore::Region unpaintedRegion; // This is simply unused.
        drawingArea->paint(cr, clipRect, unpaintedRegion);
    }

    if (showingNavigationSnapshot) {
        RefPtr<cairo_pattern_t> group = adoptRef(cairo_pop_group(cr));
        if (auto* controller = webkitWebViewBaseViewGestureController(webViewBase))
            controller->draw(cr, group.get());
    }

    GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->draw(widget, cr);

    return FALSE;
}

static void webkitWebViewBaseChildAllocate(GtkWidget* child, gpointer userData)
{
    if (!gtk_widget_get_visible(child))
        return;

    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(userData);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    const IntRect& geometry = priv->children.get(child);
    if (geometry.isEmpty())
        return;

    GtkAllocation childAllocation = geometry;
    gtk_widget_size_allocate(child, &childAllocation);
    priv->children.set(child, IntRect());
}

static void webkitWebViewBaseSizeAllocate(GtkWidget* widget, GtkAllocation* allocation)
{
    GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->size_allocate(widget, allocation);

    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    gtk_container_foreach(GTK_CONTAINER(webViewBase), webkitWebViewBaseChildAllocate, webViewBase);

    IntRect viewRect(allocation->x, allocation->y, allocation->width, allocation->height);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if (priv->inspectorView) {
        GtkAllocation childAllocation = viewRect;

        if (priv->inspectorAttachmentSide == AttachmentSide::Bottom) {
            int inspectorViewHeight = std::min(static_cast<int>(priv->inspectorViewSize), allocation->height);
            childAllocation.x = 0;
            childAllocation.y = allocation->height - inspectorViewHeight;
            childAllocation.height = inspectorViewHeight;
            viewRect.setHeight(std::max(allocation->height - inspectorViewHeight, 1));
        } else {
            int inspectorViewWidth = std::min(static_cast<int>(priv->inspectorViewSize), allocation->width);
            childAllocation.y = 0;
            childAllocation.x = allocation->width - inspectorViewWidth;
            childAllocation.width = inspectorViewWidth;
            viewRect.setWidth(std::max(allocation->width - inspectorViewWidth, 1));
        }

        gtk_widget_size_allocate(priv->inspectorView, &childAllocation);
    }

    // The dialogs are centered in the view rect, which means that it
    // never overlaps the web inspector. Thus, we need to calculate the allocation here
    // after calculating the inspector allocation.
    if (priv->dialog) {
        GtkRequisition minimumSize;
        gtk_widget_get_preferred_size(priv->dialog, &minimumSize, nullptr);

        GtkAllocation childAllocation = { 0, 0, std::max(minimumSize.width, viewRect.width()), std::max(minimumSize.height, viewRect.height()) };
        gtk_widget_size_allocate(priv->dialog, &childAllocation);
    }

    if (auto* drawingArea = static_cast<DrawingAreaProxyCoordinatedGraphics*>(priv->pageProxy->drawingArea()))
        drawingArea->setSize(viewRect.size());
}

static void webkitWebViewBaseGetPreferredWidth(GtkWidget* widget, gint* minimumSize, gint* naturalSize)
{
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;
    *minimumSize = 0;
    *naturalSize = priv->contentsSize.width();
}

static void webkitWebViewBaseGetPreferredHeight(GtkWidget* widget, gint* minimumSize, gint* naturalSize)
{
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;
    *minimumSize = 0;
    *naturalSize = priv->contentsSize.height();
}

static void webkitWebViewBaseMap(GtkWidget* widget)
{
    GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->map(widget);

    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    OptionSet<ActivityState::Flag> flagsToUpdate;
    if (!(priv->activityState & ActivityState::IsVisible))
        flagsToUpdate.add(ActivityState::IsVisible);
    if (priv->toplevelOnScreenWindow) {
        if (!(priv->activityState & ActivityState::IsInWindow))
            flagsToUpdate.add(ActivityState::IsInWindow);
        if (gtk_window_is_active(GTK_WINDOW(priv->toplevelOnScreenWindow)) && !(priv->activityState & ActivityState::WindowIsActive))
            flagsToUpdate.add(ActivityState::WindowIsActive);
    }
    if (!flagsToUpdate)
        return;

    priv->activityState.add(flagsToUpdate);
    webkitWebViewBaseScheduleUpdateActivityState(webViewBase, flagsToUpdate);
}

static void webkitWebViewBaseUnmap(GtkWidget* widget)
{
    GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->unmap(widget);

    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if (!(priv->activityState & ActivityState::IsVisible))
        return;

    priv->activityState.remove(ActivityState::IsVisible);
    webkitWebViewBaseScheduleUpdateActivityState(webViewBase, ActivityState::IsVisible);
}

static gboolean webkitWebViewBaseFocusInEvent(GtkWidget* widget, GdkEventFocus* event)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    webkitWebViewBaseSetFocus(webViewBase, true);
    webViewBase->priv->inputMethodFilter.notifyFocusedIn();

    return GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->focus_in_event(widget, event);
}

static gboolean webkitWebViewBaseFocusOutEvent(GtkWidget* widget, GdkEventFocus* event)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    webkitWebViewBaseSetFocus(webViewBase, false);
    webViewBase->priv->inputMethodFilter.notifyFocusedOut();

    return GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->focus_out_event(widget, event);
}

static gboolean webkitWebViewBaseKeyPressEvent(GtkWidget* widget, GdkEventKey* keyEvent)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;

    GdkModifierType state;
    guint keyval;
    gdk_event_get_state(reinterpret_cast<GdkEvent*>(keyEvent), &state);
    gdk_event_get_keyval(reinterpret_cast<GdkEvent*>(keyEvent), &keyval);

#if ENABLE(DEVELOPER_MODE) && OS(LINUX)
    if ((state & GDK_CONTROL_MASK) && (state & GDK_SHIFT_MASK) && keyval == GDK_KEY_G) {
        auto& preferences = priv->pageProxy->preferences();
        preferences.setResourceUsageOverlayVisible(!preferences.resourceUsageOverlayVisible());
        priv->shouldForwardNextKeyEvent = FALSE;
        return GDK_EVENT_STOP;
    }
#endif

    if (priv->dialog)
        return GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->key_press_event(widget, keyEvent);

#if ENABLE(FULLSCREEN_API)
    if (priv->fullScreenModeActive) {
        switch (keyval) {
        case GDK_KEY_Escape:
        case GDK_KEY_f:
        case GDK_KEY_F:
            priv->pageProxy->fullScreenManager()->requestExitFullScreen();
            return GDK_EVENT_STOP;
        default:
            break;
        }
    }
#endif

    // Since WebProcess key event handling is not synchronous, handle the event in two passes.
    // When WebProcess processes the input event, it will call PageClientImpl::doneWithKeyEvent
    // with event handled status which determines whether to pass the input event to parent or not
    // using gtk_main_do_event().
    if (priv->shouldForwardNextKeyEvent) {
        priv->shouldForwardNextKeyEvent = FALSE;
        return GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->key_press_event(widget, keyEvent);
    }

    // We need to copy the event as otherwise it could be destroyed before we reach the lambda body.
    GUniquePtr<GdkEvent> event(gdk_event_copy(reinterpret_cast<GdkEvent*>(keyEvent)));
    priv->inputMethodFilter.filterKeyEvent(keyEvent, [priv, event = WTFMove(event)](const WebCore::CompositionResults& compositionResults, InputMethodFilter::EventFakedForComposition faked) {
        priv->pageProxy->handleKeyboardEvent(NativeWebKeyboardEvent(event.get(), compositionResults, faked,
            !compositionResults.compositionUpdated() ? priv->keyBindingTranslator.commandsForKeyEvent(&event->key) : Vector<String>()));
    });

    return GDK_EVENT_STOP;
}

static gboolean webkitWebViewBaseKeyReleaseEvent(GtkWidget* widget, GdkEventKey* keyEvent)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;

    if (priv->shouldForwardNextKeyEvent) {
        priv->shouldForwardNextKeyEvent = FALSE;
        return GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->key_release_event(widget, keyEvent);
    }

    // We need to copy the event as otherwise it could be destroyed before we reach the lambda body.
    GUniquePtr<GdkEvent> event(gdk_event_copy(reinterpret_cast<GdkEvent*>(keyEvent)));
    priv->inputMethodFilter.filterKeyEvent(keyEvent, [priv, event = WTFMove(event)](const WebCore::CompositionResults& compositionResults, InputMethodFilter::EventFakedForComposition faked) {
        priv->pageProxy->handleKeyboardEvent(NativeWebKeyboardEvent(event.get(), compositionResults, faked, { }));
    });

    return GDK_EVENT_STOP;
}

static void webkitWebViewBaseHandleMouseEvent(WebKitWebViewBase* webViewBase, GdkEvent* event)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    ASSERT(!priv->dialog);

    int clickCount = 0;
    GdkEventType eventType = gdk_event_get_event_type(event);
    switch (eventType) {
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS: {
        // For double and triple clicks GDK sends both a normal button press event
        // and a specific type (like GDK_2BUTTON_PRESS). If we detect a special press
        // coming up, ignore this event as it certainly generated the double or triple
        // click. The consequence of not eating this event is two DOM button press events
        // are generated.
        GUniquePtr<GdkEvent> nextEvent(gdk_event_peek());
        if (nextEvent && (nextEvent->any.type == GDK_2BUTTON_PRESS || nextEvent->any.type == GDK_3BUTTON_PRESS))
            return;

        priv->inputMethodFilter.notifyMouseButtonPress();

        guint button;
        gdk_event_get_button(event, &button);
        // If it's a right click event save it as a possible context menu event.
        if (button == GDK_BUTTON_SECONDARY)
            priv->contextMenuEvent.reset(gdk_event_copy(event));

        clickCount = priv->clickCounter.currentClickCountForGdkButtonEvent(event);
    }
        FALLTHROUGH;
    case GDK_BUTTON_RELEASE:
        gtk_widget_grab_focus(GTK_WIDGET(webViewBase));
        break;
    case GDK_MOTION_NOTIFY:
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
        break;
    default:
        ASSERT_NOT_REACHED();
    }

    priv->pageProxy->handleMouseEvent(NativeWebMouseEvent(event, clickCount));
}

static gboolean webkitWebViewBaseButtonPressEvent(GtkWidget* widget, GdkEventButton* event)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;

    if (priv->dialog)
        return GDK_EVENT_STOP;

    webkitWebViewBaseHandleMouseEvent(webViewBase, reinterpret_cast<GdkEvent*>(event));

    return GDK_EVENT_STOP;
}

static gboolean webkitWebViewBaseButtonReleaseEvent(GtkWidget* widget, GdkEventButton* event)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;

    if (priv->dialog)
        return GDK_EVENT_STOP;

    webkitWebViewBaseHandleMouseEvent(webViewBase, reinterpret_cast<GdkEvent*>(event));

    return GDK_EVENT_STOP;
}

static void webkitWebViewBaseHandleWheelEvent(WebKitWebViewBase* webViewBase, GdkEvent* event, Optional<WebWheelEvent::Phase> phase = WTF::nullopt, Optional<WebWheelEvent::Phase> momentum = WTF::nullopt)
{
    ViewGestureController* controller = webkitWebViewBaseViewGestureController(webViewBase);
    if (controller && controller->isSwipeGestureEnabled() && controller->handleScrollWheelEvent(reinterpret_cast<GdkEventScroll*>(event)))
        return;

    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    ASSERT(!priv->dialog);
    if (phase)
        priv->pageProxy->handleWheelEvent(NativeWebWheelEvent(event, phase.value(), momentum.valueOr(WebWheelEvent::Phase::PhaseNone)));
    else
        priv->pageProxy->handleWheelEvent(NativeWebWheelEvent(event));
}

static gboolean webkitWebViewBaseScrollEvent(GtkWidget* widget, GdkEventScroll* event)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;

    if (std::exchange(priv->shouldForwardNextWheelEvent, false))
        return GDK_EVENT_PROPAGATE;

    if (priv->dialog)
        return GDK_EVENT_PROPAGATE;

    // Shift+Wheel scrolls in the perpendicular direction.
    if (event->state & GDK_SHIFT_MASK) {
        switch (event->direction) {
        case GDK_SCROLL_UP:
            event->direction = GDK_SCROLL_LEFT;
            break;
        case GDK_SCROLL_LEFT:
            event->direction = GDK_SCROLL_UP;
            break;
        case GDK_SCROLL_DOWN:
            event->direction = GDK_SCROLL_RIGHT;
            break;
        case GDK_SCROLL_RIGHT:
            event->direction = GDK_SCROLL_DOWN;
            break;
        case GDK_SCROLL_SMOOTH:
            std::swap(event->delta_x, event->delta_y);
            break;
        }
    }

    webkitWebViewBaseHandleWheelEvent(webViewBase, reinterpret_cast<GdkEvent*>(event));

    return GDK_EVENT_STOP;
}

static gboolean webkitWebViewBasePopupMenu(GtkWidget* widget)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;

    GdkEvent* currentEvent = gtk_get_current_event();
    if (!currentEvent)
        currentEvent = gdk_event_new(GDK_NOTHING);
    priv->contextMenuEvent.reset(currentEvent);
    priv->pageProxy->handleContextMenuKeyEvent();

    return TRUE;
}

static gboolean webkitWebViewBaseMotionNotifyEvent(GtkWidget* widget, GdkEventMotion* event)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;

    if (priv->dialog) {
        auto* widgetClass = GTK_WIDGET_CLASS(webkit_web_view_base_parent_class);
        return widgetClass->motion_notify_event ? widgetClass->motion_notify_event(widget, event) : GDK_EVENT_PROPAGATE;
    }

    webkitWebViewBaseHandleMouseEvent(webViewBase, reinterpret_cast<GdkEvent*>(event));

    return GDK_EVENT_PROPAGATE;
}

static gboolean webkitWebViewBaseCrossingNotifyEvent(GtkWidget* widget, GdkEventCrossing* crossingEvent)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;

    if (priv->dialog)
        return GDK_EVENT_PROPAGATE;

#if ENABLE(DEVELOPER_MODE)
    // Do not send mouse move events to the WebProcess for crossing events during testing.
    // WTR never generates crossing events and they can confuse tests.
    // https://bugs.webkit.org/show_bug.cgi?id=185072.
    if (UNLIKELY(priv->pageProxy->process().processPool().configuration().fullySynchronousModeIsAllowedForTesting()))
        return GDK_EVENT_PROPAGATE;
#endif

    // In the case of crossing events, it's very important the actual coordinates the WebProcess receives, because once the mouse leaves
    // the web view, the WebProcess won't receive more events until the mouse enters again in the web view. So, if the coordinates of the leave
    // event are not accurate, the WebProcess might not know the mouse left the view. This can happen because of double to integer conversion,
    // if the coordinates of the leave event are for example (25.2, -0.9), the WebProcess will receive (25, 0) and any hit test will succeed
    // because those coordinates are inside the web view.
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    double xEvent, yEvent;
    gdk_event_get_coords(reinterpret_cast<GdkEvent*>(crossingEvent), &xEvent, &yEvent);
    double width = allocation.width;
    double height = allocation.height;
    double x = xEvent;
    double y = yEvent;
    if (x < 0 && x > -1)
        x = -1;
    else if (x >= width && x < width + 1)
        x = width + 1;
    if (y < 0 && y > -1)
        y = -1;
    else if (y >= height && y < height + 1)
        y = height + 1;

    GdkEvent* event = reinterpret_cast<GdkEvent*>(crossingEvent);
    GUniquePtr<GdkEvent> copiedEvent;
    if (x != xEvent || y != yEvent) {
        copiedEvent.reset(gdk_event_copy(event));
        copiedEvent->crossing.x = x;
        copiedEvent->crossing.y = y;
    }

    webkitWebViewBaseHandleMouseEvent(webViewBase, copiedEvent ? copiedEvent.get() : event);

    return GDK_EVENT_PROPAGATE;
}

#if ENABLE(TOUCH_EVENTS)
static void appendTouchEvent(Vector<WebPlatformTouchPoint>& touchPoints, const GdkEvent* event, WebPlatformTouchPoint::TouchPointState state)
{
    gdouble x, y;
    gdk_event_get_coords(event, &x, &y);

    gdouble xRoot, yRoot;
    gdk_event_get_root_coords(event, &xRoot, &yRoot);

    uint32_t identifier = GPOINTER_TO_UINT(gdk_event_get_event_sequence(event));
    touchPoints.uncheckedAppend(WebPlatformTouchPoint(identifier, state, IntPoint(xRoot, yRoot), IntPoint(x, y)));
}

static inline WebPlatformTouchPoint::TouchPointState touchPointStateForEvents(const GdkEvent* current, const GdkEvent* event)
{
    if (gdk_event_get_event_sequence(current) != gdk_event_get_event_sequence(event))
        return WebPlatformTouchPoint::TouchStationary;

    switch (current->type) {
    case GDK_TOUCH_UPDATE:
        return WebPlatformTouchPoint::TouchMoved;
    case GDK_TOUCH_BEGIN:
        return WebPlatformTouchPoint::TouchPressed;
    case GDK_TOUCH_END:
        return WebPlatformTouchPoint::TouchReleased;
    case GDK_TOUCH_CANCEL:
        return WebPlatformTouchPoint::TouchCancelled;
    default:
        return WebPlatformTouchPoint::TouchStationary;
    }
}

static void webkitWebViewBaseGetTouchPointsForEvent(WebKitWebViewBase* webViewBase, GdkEvent* event, Vector<WebPlatformTouchPoint>& touchPoints)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    GdkEventType type = gdk_event_get_event_type(event);
    bool touchEnd = (type == GDK_TOUCH_END) || (type == GDK_TOUCH_CANCEL);
    touchPoints.reserveInitialCapacity(touchEnd ? priv->touchEvents.size() + 1 : priv->touchEvents.size());

    for (const auto& it : priv->touchEvents)
        appendTouchEvent(touchPoints, it.value.get(), touchPointStateForEvents(it.value.get(), event));

    // Touch was already removed from the TouchEventsMap, add it here.
    if (touchEnd)
        appendTouchEvent(touchPoints, event, WebPlatformTouchPoint::TouchReleased);
}

static gboolean webkitWebViewBaseTouchEvent(GtkWidget* widget, GdkEventTouch* event)
{
    WebKitWebViewBase* webViewBase = WEBKIT_WEB_VIEW_BASE(widget);
    WebKitWebViewBasePrivate* priv = webViewBase->priv;

    if (priv->dialog)
        return GDK_EVENT_STOP;

    GdkEvent* touchEvent = reinterpret_cast<GdkEvent*>(event);
    uint32_t sequence = GPOINTER_TO_UINT(gdk_event_get_event_sequence(touchEvent));

    GdkEventType type = gdk_event_get_event_type(touchEvent);
    switch (type) {
    case GDK_TOUCH_BEGIN: {
        ASSERT(!priv->touchEvents.contains(sequence));
        GUniquePtr<GdkEvent> event(gdk_event_copy(touchEvent));
        priv->touchEvents.add(sequence, WTFMove(event));
        break;
    }
    case GDK_TOUCH_UPDATE: {
        auto it = priv->touchEvents.find(sequence);
        ASSERT(it != priv->touchEvents.end());
        it->value.reset(gdk_event_copy(touchEvent));
        break;
    }
    case GDK_TOUCH_CANCEL:
        FALLTHROUGH;
    case GDK_TOUCH_END:
        ASSERT(priv->touchEvents.contains(sequence));
        priv->touchEvents.remove(sequence);
        break;
    default:
        break;
    }

    Vector<WebPlatformTouchPoint> touchPoints;
    webkitWebViewBaseGetTouchPointsForEvent(webViewBase, touchEvent, touchPoints);
    priv->pageProxy->handleTouchEvent(NativeWebTouchEvent(reinterpret_cast<GdkEvent*>(event), WTFMove(touchPoints)));

    return GDK_EVENT_STOP;
}
#endif // ENABLE(TOUCH_EVENTS)

class TouchGestureController final : public GestureControllerClient {
    WTF_MAKE_FAST_ALLOCATED;

public:
    explicit TouchGestureController(WebKitWebViewBase* webViewBase)
        : m_webView(webViewBase)
    {
    }

private:
    static GUniquePtr<GdkEvent> createScrollEvent(GdkEventTouch* event, const FloatPoint& point, const FloatPoint& delta, bool isStop = false)
    {
        GUniquePtr<GdkEvent> scrollEvent(gdk_event_new(GDK_SCROLL));
        scrollEvent->scroll.time = event->time;
        scrollEvent->scroll.x = point.x();
        scrollEvent->scroll.y = point.y();
        scrollEvent->scroll.x_root = event->x_root;
        scrollEvent->scroll.y_root = event->y_root;
        scrollEvent->scroll.direction = GDK_SCROLL_SMOOTH;
        scrollEvent->scroll.delta_x = delta.x();
        scrollEvent->scroll.delta_y = delta.y();
        scrollEvent->scroll.state = event->state;
        scrollEvent->scroll.is_stop = isStop;
        scrollEvent->scroll.window = event->window ? GDK_WINDOW(g_object_ref(event->window)) : nullptr;
        auto* touchEvent = reinterpret_cast<GdkEvent*>(event);
        gdk_event_set_screen(scrollEvent.get(), gdk_event_get_screen(touchEvent));
        gdk_event_set_device(scrollEvent.get(), gdk_event_get_device(touchEvent));
        gdk_event_set_source_device(scrollEvent.get(), gdk_event_get_source_device(touchEvent));
        return scrollEvent;
    }

    void simulateMouseClick(GdkEventTouch* event, unsigned button)
    {
        GUniquePtr<GdkEvent> pointerEvent(gdk_event_new(GDK_MOTION_NOTIFY));
        pointerEvent->motion.time = event->time;
        pointerEvent->motion.x = event->x;
        pointerEvent->motion.y = event->y;
        pointerEvent->motion.x_root = event->x_root;
        pointerEvent->motion.y_root = event->y_root;
        pointerEvent->motion.state = event->state;
        pointerEvent->motion.window = event->window ? GDK_WINDOW(g_object_ref(event->window)) : nullptr;
        auto* touchEvent = reinterpret_cast<GdkEvent*>(event);
        gdk_event_set_screen(pointerEvent.get(), gdk_event_get_screen(touchEvent));
        gdk_event_set_device(pointerEvent.get(), gdk_event_get_device(touchEvent));
        gdk_event_set_source_device(pointerEvent.get(), gdk_event_get_source_device(touchEvent));
        webkitWebViewBaseHandleMouseEvent(m_webView, pointerEvent.get());

        pointerEvent.reset(gdk_event_new(GDK_BUTTON_PRESS));
        pointerEvent->button.button = button;
        pointerEvent->button.time = event->time;
        pointerEvent->button.x = event->x;
        pointerEvent->button.y = event->y;
        pointerEvent->button.x_root = event->x_root;
        pointerEvent->button.y_root = event->y_root;
        pointerEvent->button.window = event->window ? GDK_WINDOW(g_object_ref(event->window)) : nullptr;
        gdk_event_set_screen(pointerEvent.get(), gdk_event_get_screen(touchEvent));
        gdk_event_set_device(pointerEvent.get(), gdk_event_get_device(touchEvent));
        gdk_event_set_source_device(pointerEvent.get(), gdk_event_get_source_device(touchEvent));
        webkitWebViewBaseHandleMouseEvent(m_webView, pointerEvent.get());

        pointerEvent->type = GDK_BUTTON_RELEASE;
        webkitWebViewBaseHandleMouseEvent(m_webView, pointerEvent.get());
    }

    void tap(GdkEventTouch* event) final
    {
        simulateMouseClick(event, GDK_BUTTON_PRIMARY);
    }

    void startDrag(GdkEventTouch* event, const FloatPoint& startPoint) final
    {
        GUniquePtr<GdkEvent> scrollEvent = createScrollEvent(event, startPoint, { });
        webkitWebViewBaseHandleWheelEvent(m_webView, scrollEvent.get(), WebWheelEvent::Phase::PhaseBegan);
    }

    void drag(GdkEventTouch* event, const FloatPoint& point, const FloatPoint& delta) final
    {
        GUniquePtr<GdkEvent> scrollEvent = createScrollEvent(event, point, delta);
        webkitWebViewBaseHandleWheelEvent(m_webView, scrollEvent.get(), WebWheelEvent::Phase::PhaseChanged);
    }

    void cancelDrag() final
    {
        if (auto* controller = webkitWebViewBaseViewGestureController(m_webView))
            controller->cancelSwipe();
    }

    void swipe(GdkEventTouch* event, const FloatPoint& velocity) final
    {
        double x, y;
        gdk_event_get_coords(reinterpret_cast<GdkEvent*>(event), &x, &y);
        GUniquePtr<GdkEvent> scrollEvent = createScrollEvent(event, FloatPoint::narrowPrecision(x, y), velocity, true);
        webkitWebViewBaseHandleWheelEvent(m_webView, scrollEvent.get(), WebWheelEvent::Phase::PhaseNone, WebWheelEvent::Phase::PhaseBegan);
    }

    void startZoom(const IntPoint& center, double& initialScale, IntPoint& initialPoint) final
    {
        auto* page = m_webView->priv->pageProxy.get();
        ASSERT(page);
        initialScale = page->pageScaleFactor();
        page->getCenterForZoomGesture(center, initialPoint);
    }

    void zoom(double scale, const IntPoint& origin) final
    {
        auto* page = m_webView->priv->pageProxy.get();
        ASSERT(page);

        page->scalePage(scale, origin);
    }

    void longPress(GdkEventTouch* event) final
    {
        simulateMouseClick(event, GDK_BUTTON_SECONDARY);
    }

    WebKitWebViewBase* m_webView;
};

GestureController& webkitWebViewBaseGestureController(WebKitWebViewBase* webViewBase)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if (!priv->gestureController)
        priv->gestureController = makeUnique<GestureController>(GTK_WIDGET(webViewBase), makeUnique<TouchGestureController>(webViewBase));
    return *priv->gestureController;
}

void webkitWebViewBaseSetEnableBackForwardNavigationGesture(WebKitWebViewBase* webViewBase, bool enabled)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;

    priv->isBackForwardNavigationGestureEnabled = enabled;

    if (auto* controller = webkitWebViewBaseViewGestureController(webViewBase))
        controller->setSwipeGestureEnabled(enabled);

    priv->pageProxy->setShouldRecordNavigationSnapshots(enabled);
}

ViewGestureController* webkitWebViewBaseViewGestureController(WebKitWebViewBase* webViewBase)
{
    return webViewBase->priv->viewGestureController.get();
}

bool webkitWebViewBaseBeginBackSwipeForTesting(WebKitWebViewBase* webViewBase)
{
    if (auto* gestureController = webkitWebViewBaseViewGestureController(webViewBase))
        return gestureController->beginSimulatedSwipeInDirectionForTesting(ViewGestureController::SwipeDirection::Back);

    return FALSE;
}

bool webkitWebViewBaseCompleteBackSwipeForTesting(WebKitWebViewBase* webViewBase)
{
    if (auto* gestureController = webkitWebViewBaseViewGestureController(webViewBase))
        return gestureController->completeSimulatedSwipeInDirectionForTesting(ViewGestureController::SwipeDirection::Back);

    return FALSE;
}

static gboolean webkitWebViewBaseQueryTooltip(GtkWidget* widget, gint /* x */, gint /* y */, gboolean keyboardMode, GtkTooltip* tooltip)
{
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;

    if (keyboardMode) {
        // TODO: https://bugs.webkit.org/show_bug.cgi?id=61732.
        notImplemented();
        return FALSE;
    }

    if (priv->tooltipText.length() <= 0)
        return FALSE;

    if (!priv->tooltipArea.isEmpty()) {
        GdkRectangle area = priv->tooltipArea;
        gtk_tooltip_set_tip_area(tooltip, &area);
    } else
        gtk_tooltip_set_tip_area(tooltip, 0);
    gtk_tooltip_set_text(tooltip, priv->tooltipText.data());

    return TRUE;
}

#if ENABLE(DRAG_SUPPORT)
static void webkitWebViewBaseDragDataGet(GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selectionData, guint info, guint /* time */)
{
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;
    ASSERT(priv->dragAndDropHandler);
    priv->dragAndDropHandler->fillDragData(context, selectionData, info);
}

static void webkitWebViewBaseDragEnd(GtkWidget* widget, GdkDragContext* context)
{
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;
    ASSERT(priv->dragAndDropHandler);
    priv->dragAndDropHandler->finishDrag(context);
}

static void webkitWebViewBaseDragDataReceived(GtkWidget* widget, GdkDragContext* context, gint /* x */, gint /* y */, GtkSelectionData* selectionData, guint info, guint time)
{
    webkitWebViewBaseDragAndDropHandler(WEBKIT_WEB_VIEW_BASE(widget)).dragEntered(context, selectionData, info, time);
}
#endif // ENABLE(DRAG_SUPPORT)

static gboolean webkitWebViewBaseEvent(GtkWidget* widget, GdkEvent* event)
{
    if (gdk_event_get_event_type(event) == GDK_TOUCHPAD_PINCH)
        webkitWebViewBaseGestureController(WEBKIT_WEB_VIEW_BASE(widget)).handleEvent(event);
    return GDK_EVENT_PROPAGATE;
}

static AtkObject* webkitWebViewBaseGetAccessible(GtkWidget* widget)
{
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;
    if (!priv->accessible) {
        // Create the accessible object and associate it to the widget.
        priv->accessible = adoptGRef(ATK_OBJECT(webkitWebViewAccessibleNew(widget)));

        // Set the parent to not break bottom-up navigation.
        if (auto* parentWidget = gtk_widget_get_parent(widget)) {
            if (auto* axParent = gtk_widget_get_accessible(parentWidget))
                atk_object_set_parent(priv->accessible.get(), axParent);
        }
    }

    return priv->accessible.get();
}

#if ENABLE(DRAG_SUPPORT)
static gboolean webkitWebViewBaseDragMotion(GtkWidget* widget, GdkDragContext* context, gint x, gint y, guint time)
{
    webkitWebViewBaseDragAndDropHandler(WEBKIT_WEB_VIEW_BASE(widget)).dragMotion(context, IntPoint(x, y), time);
    return TRUE;
}

static void webkitWebViewBaseDragLeave(GtkWidget* widget, GdkDragContext* context, guint /* time */)
{
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;
    ASSERT(priv->dragAndDropHandler);
    priv->dragAndDropHandler->dragLeave(context);
}

static gboolean webkitWebViewBaseDragDrop(GtkWidget* widget, GdkDragContext* context, gint x, gint y, guint time)
{
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;
    ASSERT(priv->dragAndDropHandler);
    return priv->dragAndDropHandler->drop(context, IntPoint(x, y), time);
}
#endif // ENABLE(DRAG_SUPPORT)

static void webkitWebViewBaseHierarchyChanged(GtkWidget* widget, GtkWidget* oldToplevel)
{
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;
    if (widgetIsOnscreenToplevelWindow(oldToplevel) && GTK_WINDOW(oldToplevel) == priv->toplevelOnScreenWindow) {
        webkitWebViewBaseSetToplevelOnScreenWindow(WEBKIT_WEB_VIEW_BASE(widget), nullptr);
        return;
    }

    if (!oldToplevel) {
        GtkWidget* toplevel = gtk_widget_get_toplevel(widget);
        if (widgetIsOnscreenToplevelWindow(toplevel))
            webkitWebViewBaseSetToplevelOnScreenWindow(WEBKIT_WEB_VIEW_BASE(widget), GTK_WINDOW(toplevel));
    }
}

static gboolean webkitWebViewBaseFocus(GtkWidget* widget, GtkDirectionType direction)
{
    // If a dialog is active, we need to forward focus events there. This
    // ensures that you can tab between elements in the box.
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;
    if (priv->dialog) {
        gboolean returnValue;
        g_signal_emit_by_name(priv->dialog, "focus", direction, &returnValue);
        return returnValue;
    }

    return GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->focus(widget, direction);
}

static void webkitWebViewBaseDestroy(GtkWidget* widget)
{
    WebKitWebViewBasePrivate* priv = WEBKIT_WEB_VIEW_BASE(widget)->priv;
    if (priv->dialog)
        gtk_widget_destroy(priv->dialog);

    GTK_WIDGET_CLASS(webkit_web_view_base_parent_class)->destroy(widget);
}

static void webkit_web_view_base_class_init(WebKitWebViewBaseClass* webkitWebViewBaseClass)
{
    GtkWidgetClass* widgetClass = GTK_WIDGET_CLASS(webkitWebViewBaseClass);
    widgetClass->realize = webkitWebViewBaseRealize;
    widgetClass->unrealize = webkitWebViewBaseUnrealize;
    widgetClass->draw = webkitWebViewBaseDraw;
    widgetClass->size_allocate = webkitWebViewBaseSizeAllocate;
    widgetClass->get_preferred_width = webkitWebViewBaseGetPreferredWidth;
    widgetClass->get_preferred_height = webkitWebViewBaseGetPreferredHeight;
    widgetClass->map = webkitWebViewBaseMap;
    widgetClass->unmap = webkitWebViewBaseUnmap;
    widgetClass->focus = webkitWebViewBaseFocus;
    widgetClass->focus_in_event = webkitWebViewBaseFocusInEvent;
    widgetClass->focus_out_event = webkitWebViewBaseFocusOutEvent;
    widgetClass->key_press_event = webkitWebViewBaseKeyPressEvent;
    widgetClass->key_release_event = webkitWebViewBaseKeyReleaseEvent;
    widgetClass->button_press_event = webkitWebViewBaseButtonPressEvent;
    widgetClass->button_release_event = webkitWebViewBaseButtonReleaseEvent;
    widgetClass->scroll_event = webkitWebViewBaseScrollEvent;
    widgetClass->popup_menu = webkitWebViewBasePopupMenu;
    widgetClass->motion_notify_event = webkitWebViewBaseMotionNotifyEvent;
    widgetClass->enter_notify_event = webkitWebViewBaseCrossingNotifyEvent;
    widgetClass->leave_notify_event = webkitWebViewBaseCrossingNotifyEvent;
#if ENABLE(TOUCH_EVENTS)
    widgetClass->touch_event = webkitWebViewBaseTouchEvent;
#endif
    widgetClass->query_tooltip = webkitWebViewBaseQueryTooltip;
#if ENABLE(DRAG_SUPPORT)
    widgetClass->drag_end = webkitWebViewBaseDragEnd;
    widgetClass->drag_data_get = webkitWebViewBaseDragDataGet;
    widgetClass->drag_motion = webkitWebViewBaseDragMotion;
    widgetClass->drag_leave = webkitWebViewBaseDragLeave;
    widgetClass->drag_drop = webkitWebViewBaseDragDrop;
    widgetClass->drag_data_received = webkitWebViewBaseDragDataReceived;
#endif // ENABLE(DRAG_SUPPORT)
    widgetClass->event = webkitWebViewBaseEvent;
    widgetClass->get_accessible = webkitWebViewBaseGetAccessible;
    widgetClass->hierarchy_changed = webkitWebViewBaseHierarchyChanged;
    widgetClass->destroy = webkitWebViewBaseDestroy;

    GObjectClass* gobjectClass = G_OBJECT_CLASS(webkitWebViewBaseClass);
    gobjectClass->constructed = webkitWebViewBaseConstructed;
    gobjectClass->dispose = webkitWebViewBaseDispose;

    GtkContainerClass* containerClass = GTK_CONTAINER_CLASS(webkitWebViewBaseClass);
    containerClass->add = webkitWebViewBaseContainerAdd;
    containerClass->remove = webkitWebViewBaseContainerRemove;
    containerClass->forall = webkitWebViewBaseContainerForall;

    // Before creating a WebKitWebViewBasePriv we need to be sure that WebKit is started.
    // Usually starting a context triggers InitializeWebKit2, but in case
    // we create a view without asking before for a default_context we get a crash.
    WebKit::InitializeWebKit2();
}

WebKitWebViewBase* webkitWebViewBaseCreate(const API::PageConfiguration& configuration)
{
    WebKitWebViewBase* webkitWebViewBase = WEBKIT_WEB_VIEW_BASE(g_object_new(WEBKIT_TYPE_WEB_VIEW_BASE, nullptr));
    webkitWebViewBaseCreateWebPage(webkitWebViewBase, configuration.copy());
    return webkitWebViewBase;
}

GtkIMContext* webkitWebViewBaseGetIMContext(WebKitWebViewBase* webkitWebViewBase)
{
    return webkitWebViewBase->priv->inputMethodFilter.context();
}

WebPageProxy* webkitWebViewBaseGetPage(WebKitWebViewBase* webkitWebViewBase)
{
    return webkitWebViewBase->priv->pageProxy.get();
}

static void deviceScaleFactorChanged(WebKitWebViewBase* webkitWebViewBase)
{
    webkitWebViewBase->priv->pageProxy->setIntrinsicDeviceScaleFactor(gtk_widget_get_scale_factor(GTK_WIDGET(webkitWebViewBase)));
}

void webkitWebViewBaseCreateWebPage(WebKitWebViewBase* webkitWebViewBase, Ref<API::PageConfiguration>&& configuration)
{
    WebKitWebViewBasePrivate* priv = webkitWebViewBase->priv;
    WebProcessPool* processPool = configuration->processPool();
    priv->pageProxy = processPool->createWebPage(*priv->pageClient, WTFMove(configuration));
    priv->acceleratedBackingStore = AcceleratedBackingStore::create(*priv->pageProxy);
    priv->pageProxy->initializeWebPage();

    priv->inputMethodFilter.setPage(priv->pageProxy.get());

    // We attach this here, because changes in scale factor are passed directly to the page proxy.
    priv->pageProxy->setIntrinsicDeviceScaleFactor(gtk_widget_get_scale_factor(GTK_WIDGET(webkitWebViewBase)));
    g_signal_connect(webkitWebViewBase, "notify::scale-factor", G_CALLBACK(deviceScaleFactorChanged), nullptr);
}

void webkitWebViewBaseSetTooltipText(WebKitWebViewBase* webViewBase, const char* tooltip)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if (tooltip && tooltip[0] != '\0') {
        priv->tooltipText = tooltip;
        gtk_widget_set_has_tooltip(GTK_WIDGET(webViewBase), TRUE);
    } else {
        priv->tooltipText = "";
        gtk_widget_set_has_tooltip(GTK_WIDGET(webViewBase), FALSE);
    }

    gtk_widget_trigger_tooltip_query(GTK_WIDGET(webViewBase));
}

void webkitWebViewBaseSetTooltipArea(WebKitWebViewBase* webViewBase, const IntRect& tooltipArea)
{
    webViewBase->priv->tooltipArea = tooltipArea;
}

#if ENABLE(DRAG_SUPPORT)
DragAndDropHandler& webkitWebViewBaseDragAndDropHandler(WebKitWebViewBase* webViewBase)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if (!priv->dragAndDropHandler)
        priv->dragAndDropHandler = makeUnique<DragAndDropHandler>(*priv->pageProxy);
    return *priv->dragAndDropHandler;
}
#endif // ENABLE(DRAG_SUPPORT)

void webkitWebViewBaseForwardNextKeyEvent(WebKitWebViewBase* webkitWebViewBase)
{
    webkitWebViewBase->priv->shouldForwardNextKeyEvent = TRUE;
}

void webkitWebViewBaseForwardNextWheelEvent(WebKitWebViewBase* webkitWebViewBase)
{
    webkitWebViewBase->priv->shouldForwardNextWheelEvent = true;
}

void webkitWebViewBaseEnterFullScreen(WebKitWebViewBase* webkitWebViewBase)
{
#if ENABLE(FULLSCREEN_API)
    WebKitWebViewBasePrivate* priv = webkitWebViewBase->priv;
    ASSERT(!priv->fullScreenModeActive);

    WebFullScreenManagerProxy* fullScreenManagerProxy = priv->pageProxy->fullScreenManager();
    fullScreenManagerProxy->willEnterFullScreen();

    GtkWidget* topLevelWindow = gtk_widget_get_toplevel(GTK_WIDGET(webkitWebViewBase));
    if (gtk_widget_is_toplevel(topLevelWindow))
        gtk_window_fullscreen(GTK_WINDOW(topLevelWindow));
    fullScreenManagerProxy->didEnterFullScreen();
    priv->fullScreenModeActive = true;
    priv->sleepDisabler = PAL::SleepDisabler::create(_("Website running in fullscreen mode"), PAL::SleepDisabler::Type::Display);
#endif
}

void webkitWebViewBaseExitFullScreen(WebKitWebViewBase* webkitWebViewBase)
{
#if ENABLE(FULLSCREEN_API)
    WebKitWebViewBasePrivate* priv = webkitWebViewBase->priv;
    ASSERT(priv->fullScreenModeActive);

    WebFullScreenManagerProxy* fullScreenManagerProxy = priv->pageProxy->fullScreenManager();
    fullScreenManagerProxy->willExitFullScreen();

    GtkWidget* topLevelWindow = gtk_widget_get_toplevel(GTK_WIDGET(webkitWebViewBase));
    if (gtk_widget_is_toplevel(topLevelWindow))
        gtk_window_unfullscreen(GTK_WINDOW(topLevelWindow));
    fullScreenManagerProxy->didExitFullScreen();
    priv->fullScreenModeActive = false;
    priv->sleepDisabler = nullptr;
#endif
}

bool webkitWebViewBaseIsFullScreen(WebKitWebViewBase* webkitWebViewBase)
{
#if ENABLE(FULLSCREEN_API)
    return webkitWebViewBase->priv->fullScreenModeActive;
#else
    return false;
#endif
}

void webkitWebViewBaseSetInspectorViewSize(WebKitWebViewBase* webkitWebViewBase, unsigned size)
{
    if (webkitWebViewBase->priv->inspectorViewSize == size)
        return;
    webkitWebViewBase->priv->inspectorViewSize = size;
    if (webkitWebViewBase->priv->inspectorView)
        gtk_widget_queue_resize_no_redraw(GTK_WIDGET(webkitWebViewBase));
}

static void activeContextMenuUnmapped(GtkMenu* menu, WebKitWebViewBase* webViewBase)
{
    if (webViewBase->priv->activeContextMenuProxy && webViewBase->priv->activeContextMenuProxy->gtkMenu() == menu)
        webViewBase->priv->activeContextMenuProxy = nullptr;
}

void webkitWebViewBaseSetActiveContextMenuProxy(WebKitWebViewBase* webkitWebViewBase, WebContextMenuProxyGtk* contextMenuProxy)
{
    webkitWebViewBase->priv->activeContextMenuProxy = contextMenuProxy;
    g_signal_connect_object(contextMenuProxy->gtkMenu(), "unmap", G_CALLBACK(activeContextMenuUnmapped), webkitWebViewBase, static_cast<GConnectFlags>(0));
}

WebContextMenuProxyGtk* webkitWebViewBaseGetActiveContextMenuProxy(WebKitWebViewBase* webkitWebViewBase)
{
    return webkitWebViewBase->priv->activeContextMenuProxy;
}

GdkEvent* webkitWebViewBaseTakeContextMenuEvent(WebKitWebViewBase* webkitWebViewBase)
{
    return webkitWebViewBase->priv->contextMenuEvent.release();
}

void webkitWebViewBaseSetFocus(WebKitWebViewBase* webViewBase, bool focused)
{
    WebKitWebViewBasePrivate* priv = webViewBase->priv;
    if ((focused && priv->activityState & ActivityState::IsFocused) || (!focused && !(priv->activityState & ActivityState::IsFocused)))
        return;

    OptionSet<ActivityState::Flag> flagsToUpdate { ActivityState::IsFocused };
    if (focused) {
        priv->activityState.add(ActivityState::IsFocused);

        // If the view has received the focus and the window is not active
        // mark the current window as active now. This can happen if the
        // toplevel window is a GTK_WINDOW_POPUP and the focus has been
        // set programatically like WebKitTestRunner does, because POPUP
        // can't be focused.
        if (!(priv->activityState & ActivityState::WindowIsActive)) {
            priv->activityState.add(ActivityState::WindowIsActive);
            flagsToUpdate.add(ActivityState::WindowIsActive);
        }
    } else
        priv->activityState.remove(ActivityState::IsFocused);

    webkitWebViewBaseScheduleUpdateActivityState(webViewBase, flagsToUpdate);
}

bool webkitWebViewBaseIsInWindowActive(WebKitWebViewBase* webViewBase)
{
    return webViewBase->priv->activityState.contains(ActivityState::WindowIsActive);
}

bool webkitWebViewBaseIsFocused(WebKitWebViewBase* webViewBase)
{
    return webViewBase->priv->activityState.contains(ActivityState::IsFocused);
}

bool webkitWebViewBaseIsVisible(WebKitWebViewBase* webViewBase)
{
    return webViewBase->priv->activityState.contains(ActivityState::IsVisible);
}

bool webkitWebViewBaseIsInWindow(WebKitWebViewBase* webViewBase)
{
    return webViewBase->priv->activityState.contains(ActivityState::IsInWindow);
}

void webkitWebViewBaseSetInputMethodState(WebKitWebViewBase* webkitWebViewBase, bool enabled)
{
    webkitWebViewBase->priv->inputMethodFilter.setEnabled(enabled);
}

void webkitWebViewBaseUpdateTextInputState(WebKitWebViewBase* webkitWebViewBase)
{
    const auto& editorState = webkitWebViewBase->priv->pageProxy->editorState();
    if (!editorState.isMissingPostLayoutData)
        webkitWebViewBase->priv->inputMethodFilter.setCursorRect(editorState.postLayoutData().caretRectAtStart);
}

void webkitWebViewBaseSetContentsSize(WebKitWebViewBase* webkitWebViewBase, const IntSize& contentsSize)
{
    WebKitWebViewBasePrivate* priv = webkitWebViewBase->priv;
    if (priv->contentsSize == contentsSize)
        return;
    priv->contentsSize = contentsSize;
}

void webkitWebViewBaseResetClickCounter(WebKitWebViewBase* webkitWebViewBase)
{
    webkitWebViewBase->priv->clickCounter.reset();
}

void webkitWebViewBaseEnterAcceleratedCompositingMode(WebKitWebViewBase* webkitWebViewBase, const LayerTreeContext& layerTreeContext)
{
    ASSERT(webkitWebViewBase->priv->acceleratedBackingStore);
    webkitWebViewBase->priv->acceleratedBackingStore->update(layerTreeContext);
}

void webkitWebViewBaseUpdateAcceleratedCompositingMode(WebKitWebViewBase* webkitWebViewBase, const LayerTreeContext& layerTreeContext)
{
    ASSERT(webkitWebViewBase->priv->acceleratedBackingStore);
    webkitWebViewBase->priv->acceleratedBackingStore->update(layerTreeContext);
}

void webkitWebViewBaseExitAcceleratedCompositingMode(WebKitWebViewBase* webkitWebViewBase)
{
    ASSERT(webkitWebViewBase->priv->acceleratedBackingStore);
    webkitWebViewBase->priv->acceleratedBackingStore->update(LayerTreeContext());
}

bool webkitWebViewBaseMakeGLContextCurrent(WebKitWebViewBase* webkitWebViewBase)
{
    ASSERT(webkitWebViewBase->priv->acceleratedBackingStore);
    return webkitWebViewBase->priv->acceleratedBackingStore->makeContextCurrent();
}

void webkitWebViewBaseWillSwapWebProcess(WebKitWebViewBase* webkitWebViewBase)
{
    WebKitWebViewBasePrivate* priv = webkitWebViewBase->priv;

    if (priv->viewGestureController)
        priv->viewGestureController->disconnectFromProcess();
}

void webkitWebViewBaseDidExitWebProcess(WebKitWebViewBase* webkitWebViewBase)
{
    webkitWebViewBase->priv->viewGestureController = nullptr;
}

void webkitWebViewBaseDidRelaunchWebProcess(WebKitWebViewBase* webkitWebViewBase)
{
    // Queue a resize to ensure the new DrawingAreaProxy is resized.
    gtk_widget_queue_resize_no_redraw(GTK_WIDGET(webkitWebViewBase));

    WebKitWebViewBasePrivate* priv = webkitWebViewBase->priv;
    if (priv->acceleratedBackingStore) {
        auto* drawingArea = static_cast<DrawingAreaProxyCoordinatedGraphics*>(priv->pageProxy->drawingArea());
        priv->acceleratedBackingStore->update(drawingArea->layerTreeContext());
    }

    if (priv->viewGestureController)
        priv->viewGestureController->connectToProcess();
    else {
        priv->viewGestureController = makeUnique<WebKit::ViewGestureController>(*priv->pageProxy);
        priv->viewGestureController->setSwipeGestureEnabled(priv->isBackForwardNavigationGestureEnabled);
    }
}

void webkitWebViewBasePageClosed(WebKitWebViewBase* webkitWebViewBase)
{
    if (webkitWebViewBase->priv->acceleratedBackingStore)
        webkitWebViewBase->priv->acceleratedBackingStore->update({ });
}

RefPtr<WebKit::ViewSnapshot> webkitWebViewBaseTakeViewSnapshot(WebKitWebViewBase* webkitWebViewBase)
{
    WebPageProxy* page = webkitWebViewBase->priv->pageProxy.get();

    IntSize size = page->viewSize();

    float deviceScale = page->deviceScaleFactor();
    size.scale(deviceScale);

    RefPtr<cairo_surface_t> surface = adoptRef(cairo_image_surface_create(CAIRO_FORMAT_RGB24, size.width(), size.height()));
    cairoSurfaceSetDeviceScale(surface.get(), deviceScale, deviceScale);

    RefPtr<cairo_t> cr = adoptRef(cairo_create(surface.get()));
    webkitWebViewBaseDraw(GTK_WIDGET(webkitWebViewBase), cr.get());

    return ViewSnapshot::create(WTFMove(surface));
}

void webkitWebViewBaseDidStartProvisionalLoadForMainFrame(WebKitWebViewBase* webkitWebViewBase)
{
    ViewGestureController* controller = webkitWebViewBaseViewGestureController(webkitWebViewBase);
    if (controller && controller->isSwipeGestureEnabled())
        controller->didStartProvisionalLoadForMainFrame();
}

void webkitWebViewBaseDidFirstVisuallyNonEmptyLayoutForMainFrame(WebKitWebViewBase* webkitWebViewBase)
{
    ViewGestureController* controller = webkitWebViewBaseViewGestureController(webkitWebViewBase);
    if (controller && controller->isSwipeGestureEnabled())
        controller->didFirstVisuallyNonEmptyLayoutForMainFrame();
}

void webkitWebViewBaseDidFinishLoadForMainFrame(WebKitWebViewBase* webkitWebViewBase)
{
    ViewGestureController* controller = webkitWebViewBaseViewGestureController(webkitWebViewBase);
    if (controller && controller->isSwipeGestureEnabled())
        controller->didFinishLoadForMainFrame();
}

void webkitWebViewBaseDidFailLoadForMainFrame(WebKitWebViewBase* webkitWebViewBase)
{
    ViewGestureController* controller = webkitWebViewBaseViewGestureController(webkitWebViewBase);
    if (controller && controller->isSwipeGestureEnabled())
        controller->didFailLoadForMainFrame();
}

void webkitWebViewBaseDidSameDocumentNavigationForMainFrame(WebKitWebViewBase* webkitWebViewBase, SameDocumentNavigationType type)
{
    ViewGestureController* controller = webkitWebViewBaseViewGestureController(webkitWebViewBase);
    if (controller && controller->isSwipeGestureEnabled())
        controller->didSameDocumentNavigationForMainFrame(type);
}

void webkitWebViewBaseDidRestoreScrollPosition(WebKitWebViewBase* webkitWebViewBase)
{
    ViewGestureController* controller = webkitWebViewBaseViewGestureController(webkitWebViewBase);
    if (controller && controller->isSwipeGestureEnabled())
        webkitWebViewBase->priv->viewGestureController->didRestoreScrollPosition();
}

#if GTK_CHECK_VERSION(3, 24, 0)
static void emojiChooserEmojiPicked(WebKitWebViewBase* webkitWebViewBase, const char* text)
{
    webkitWebViewBaseCompleteEmojiChooserRequest(webkitWebViewBase, String::fromUTF8(text));
}

static void emojiChooserClosed(WebKitWebViewBase* webkitWebViewBase)
{
    webkitWebViewBaseCompleteEmojiChooserRequest(webkitWebViewBase, emptyString());
    webkitWebViewBase->priv->releaseEmojiChooserTimer.startOneShot(2_min);
}
#endif

void webkitWebViewBaseShowEmojiChooser(WebKitWebViewBase* webkitWebViewBase, const IntRect& caretRect, CompletionHandler<void(String)>&& completionHandler)
{
#if GTK_CHECK_VERSION(3, 24, 0)
    WebKitWebViewBasePrivate* priv = webkitWebViewBase->priv;
    priv->releaseEmojiChooserTimer.stop();

    if (!priv->emojiChooser) {
        priv->emojiChooser = webkitEmojiChooserNew();
        g_signal_connect_swapped(priv->emojiChooser, "emoji-picked", G_CALLBACK(emojiChooserEmojiPicked), webkitWebViewBase);
        g_signal_connect_swapped(priv->emojiChooser, "closed", G_CALLBACK(emojiChooserClosed), webkitWebViewBase);
        gtk_popover_set_relative_to(GTK_POPOVER(priv->emojiChooser), GTK_WIDGET(webkitWebViewBase));
    }

    priv->emojiChooserCompletionHandler = WTFMove(completionHandler);

    GdkRectangle gdkCaretRect = caretRect;
    gtk_popover_set_pointing_to(GTK_POPOVER(priv->emojiChooser), &gdkCaretRect);
    gtk_popover_popup(GTK_POPOVER(priv->emojiChooser));
#else
    UNUSED_PARAM(webkitWebViewBase);
    UNUSED_PARAM(caretRect);
    completionHandler(emptyString());
#endif
}

#if USE(WPE_RENDERER)
int webkitWebViewBaseRenderHostFileDescriptor(WebKitWebViewBase* webkitWebViewBase)
{
    if (!webkitWebViewBase->priv->acceleratedBackingStore)
        return -1;
    return webkitWebViewBase->priv->acceleratedBackingStore->renderHostFileDescriptor();
}
#endif
