#include <LCursor.h>
#include <LAnimation.h>
#include <LLayerView.h>
#include <LSurfaceView.h>
#include <LPainter.h>
#include <LLog.h>
#include <LOpenGL.h>
#include <LTextureView.h>
#include <LOutputMode.h>
#include <LSeat.h>
#include <LPointerMoveEvent.h>
#include <LUtils.h>
#include <LScreenshotRequest.h>

#include "Global.h"
#include "Output.h"
#include "Compositor.h"
#include "Dock.h"
#include "Topbar.h"
#include "Workspace.h"
#include "Toplevel.h"
#include "Surface.h"
#include "ToplevelView.h"

Output::Output(const void *params) : LOutput(params) {}

void Output::loadWallpaper()
{
    LSize bufferSize;

    if (Louvre::is90Transform(transform()))
    {
        bufferSize.setW(currentMode()->sizeB().h());
        bufferSize.setH(currentMode()->sizeB().w());
    }
    else
    {
        bufferSize = currentMode()->sizeB();
    }

    if (wallpaperView)
    {
        if (wallpaperView->texture())
        {
            if (compositor()->graphicBackendId() != LGraphicBackendDRM || bufferSize == wallpaperView->texture()->sizeB())
            {
                wallpaperView->enableDstSize(true);
                wallpaperView->setDstSize(size());
                return;
            }


            delete wallpaperView->texture();
        }
    }
    else
    {
        wallpaperView = new LTextureView(nullptr, &G::compositor()->backgroundLayer);
        wallpaperView->enableParentOffset(false);
    }

    LTexture *tmpWallpaper = LOpenGL::loadTexture(std::filesystem::path(getenvString("HOME")) / ".config/Louvre/wallpaper.jpg");

    if (!tmpWallpaper)
        tmpWallpaper = G::loadAssetsTexture("wallpaper.png", false);

    if (tmpWallpaper)
    {
        // Clip and scale wallpaper so that it covers the entire screen

        LRect srcB;
        Float32 w = Float32(bufferSize.w() * tmpWallpaper->sizeB().h()) / Float32(bufferSize.h());

        if (w >= tmpWallpaper->sizeB().w())
        {
            srcB.setX(0);
            srcB.setW(tmpWallpaper->sizeB().w());
            srcB.setH((tmpWallpaper->sizeB().w() * bufferSize.h()) / bufferSize.w());
            srcB.setY((tmpWallpaper->sizeB().h() - srcB.h()) / 2);
        }
        else
        {
            srcB.setY(0);
            srcB.setH(tmpWallpaper->sizeB().h());
            srcB.setW((tmpWallpaper->sizeB().h() * bufferSize.w()) / bufferSize.h());
            srcB.setX((tmpWallpaper->sizeB().w() - srcB.w()) / 2);
        }
        wallpaperView->setTexture(tmpWallpaper->copy(bufferSize, srcB));
        wallpaperView->enableDstSize(true);
        wallpaperView->setDstSize(size());
        delete tmpWallpaper;
    }
    else
    {
        wallpaperView->setVisible(false);
    }

    LRegion trans;
    wallpaperView->setTranslucentRegion(&trans);
    wallpaperView->setPos(pos());
}

void Output::setWorkspace(Workspace *ws, UInt32 animMs, Float64 curve, Float64 start)
{
    animStart = start;
    easingCurve = curve;
    workspaceAnim.stop();
    workspaceAnim.setDuration(animMs * DEBUG_ANIM_SPEED);
    currentWorkspace = ws;

    if (currentWorkspace->toplevel && currentWorkspace->toplevel->surf())
        enableVSync(currentWorkspace->toplevel->surf()->preferVSync());
    else
        enableVSync(true);

    topbar->update();

    for (Output *o : G::outputs())
        o->workspaces.front()->stealChildren();

    workspaceAnim.start();
}

void Output::updateWorkspacesPos()
{
    Int32 offset = 0;
    Int32 spacing = 128;

    for (Workspace *ws : workspaces)
    {
        if (ws->nativePos().x() != offset)
        {
            ws->setPos(offset, 0);

            if (ws->toplevel)
            {
                ws->toplevel->configureSize(size());
                ws->toplevel->configureState(ws->toplevel->pending().state);
            }
        }

        offset += size().w() + spacing;
    }
}

void Output::updateFractionalOversampling()
{
    bool oversampling { dpi() < 200 };
    bool fullscreenOrSubsurface { false };

    if (!usingFractionalScale() || swipingWorkspace || workspaceAnim.running())
        goto checkChange;

    if (currentWorkspace->toplevel)
    {
        if (currentWorkspace->toplevel->decoratedView && currentWorkspace->toplevel->decoratedView->fullscreenTopbarVisibility != 0.f)
        {
            fullscreenOrSubsurface = true;
            goto checkChange;
        }

        for (LSurface *surf : currentWorkspace->toplevel->surf()->children())
        {
            if (surf->toplevel() && surf->toplevel()->current().decorationMode == LToplevelRole::ServerSide && surf->mapped() && !surf->minimized())
            {
                fullscreenOrSubsurface = true;
                goto checkChange;
            }
        }
    }
    else
    {
        for (Surface *surf : G::surfaces())
        {
            if (!surf->mapped() || surf->minimized())
                continue;

            if (surf->subsurface())
            {
                for (LOutput *o : surf->getView()->outputs())
                {
                    if (o == this)
                    {
                        fullscreenOrSubsurface = true;
                        goto checkChange;
                    }
                }
            }
            else if (surf->toplevel())
            {
                if (surf->tl()->decoratedView)
                {
                    for (LOutput *o : surf->getView()->outputs())
                    {
                        if (o == this)
                        {
                            fullscreenOrSubsurface = true;
                            goto checkChange;
                        }
                    }
                }
            }
        }
    }

    checkChange:

    oversampling = oversampling || fullscreenOrSubsurface;

    if (oversampling != fractionalOversamplingEnabled())
    {
        enableFractionalOversampling(oversampling);
        G::scene()->mainView()->damageAll(this);
        topbar->update();
    }
}

void Output::showAllWorkspaces()
{
    for (auto *ws : workspaces)
        ws->show(true);
}

void Output::hideAllWorkspacesExceptCurrent()
{
    for (auto *ws : workspaces)
        if (ws != currentWorkspace)
            ws->show(false);
}

void Output::initializeGL()
{
    workspaceAnim.setDuration(400);
    workspaceAnim.setOnUpdateCallback(
        [this](LAnimation *anim)
        {
            repaint();
            showAllWorkspaces();

            if (swipingWorkspace)
            {
                anim->stop();
                return;
            }

            // Hide non visible workspaces
            for (Workspace *ws : workspaces)
                ws->setVisible(LRect(ws->pos() + pos(), size()).intersects(rect()));

            const Float64 ease { 1.0 - pow(animStart + (1.0 - animStart) * anim->value(), easingCurve) };
            workspaceOffset = workspaceOffset * ease + Float64( - currentWorkspace->nativePos().x()) * (1.0 - ease);
            workspacesContainer->setPos(workspaceOffset, 0);

            for (Output *o : G::outputs())
                for (Workspace *workspace : o->workspaces)
                    workspace->clipChildren();

            if (animatedFullscreenToplevel)
            {
                Toplevel *tl = animatedFullscreenToplevel;

                if (tl->destructorCalled || tl->quickUnfullscreen)
                {
                    anim->stop();
                    return;
                }

                tl->surface()->requestNextFrame(false);

                // Current fullscreen size
                LSize cSize;

                if (tl->fullscreen())
                {
                    Float32 val = 1.f - pow(1.0 - anim->value(), 4.0);
                    Float32 inv = 1.f - val;
                    tl->animView.enableSrcRect(false);
                    tl->animView.setVisible(true);
                    tl->animScene->render();
                    tl->animView.setTexture(tl->animScene->texture());
                    tl->animView.setPos((pos() * val) + (tl->prevBoundingRect.pos() * (inv)));
                    cSize = (tl->fullscreenOutput->size() * val) + (tl->prevBoundingRect.size() * (inv));
                    tl->animView.setDstSize(cSize);
                    tl->animView.setOpacity(val);

                    tl->capture.setPos(tl->animView.pos());
                    tl->capture.setDstSize(cSize);
                    LRegion transRegion = tl->captureTransRegion;
                    LSizeF transRegionScale = LSizeF(cSize) / LSizeF(tl->prevBoundingRect.size());
                    transRegion.multiply(transRegionScale.x(), transRegionScale.y());
                    tl->capture.setTranslucentRegion(&transRegion);
                }
                else
                {
                    Float32 val = 1.f - pow(1.f - anim->value(), 2.f);
                    Float32 inv = 1.f - val;
                    tl->animScene->setPos(pos());
                    LPoint animPos = (pos() * inv) + (tl->prevBoundingRect.pos() * val);
                    tl->surf()->setPos(0);
                    LBox box = tl->surf()->getView()->boundingBox();
                    LSize boxSize = LSize(box.x2 - box.x1, box.y2 - box.y1);
                    cSize = (size() * inv) + (boxSize * val);

                    tl->capture.setOpacity(inv);
                    tl->capture.setPos(animPos);
                    tl->capture.setDstSize(cSize);

                    if (tl->decoratedView)
                        tl->surf()->setPos(LPoint() - (LPoint(box.x1, box.y1) - tl->animScene->nativePos()));
                    else
                        tl->surf()->setPos(tl->windowGeometry().pos());

                    tl->animScene->render();
                    LRegion transReg;
                    transReg = *tl->animScene->translucentRegion();
                    transReg.offset(LPoint() - tl->animScene->pos());
                    tl->animView.setTexture(tl->animScene->texture());
                    tl->animView.enableSrcRect(true);
                    tl->animView.setSrcRect(LRectF(0, boxSize * tl->animScene->bufferScale()));
                    tl->animView.enableDstSize(true);
                    tl->animView.enableParentOffset(false);
                    tl->animView.setPos(animPos);
                    tl->animView.setDstSize(cSize);

                    LSizeF regScale = LSizeF(cSize) / LSizeF(boxSize);
                    transReg.multiply(regScale.x(), regScale.y());
                    tl->animView.setTranslucentRegion(&transReg);

                    tl->configureSize(tl->prevRect.size());
                    tl->configureState(LToplevelRole::Activated);
                }

                if (tl->decoratedView)
                    tl->decoratedView->updateGeometry();
            }
        });

    workspaceAnim.setOnFinishCallback(
        [this](LAnimation *)
        {
            hideAllWorkspacesExceptCurrent();

            if (currentWorkspace->toplevel)
            {
                Toplevel *tl = currentWorkspace->toplevel;

                tl->blackFullscreenBackground.setVisible(false);

                if (tl->capture.texture())
                    delete tl->capture.texture();

                tl->animView.setTexture(nullptr);

                if (tl->animScene)
                {
                    delete tl->animScene;
                    tl->animScene = nullptr;
                }

                if (tl->destructorCalled || tl->quickUnfullscreen)
                    goto returnChildren;

                seat()->pointer()->setFocus(tl->surface());
                seat()->keyboard()->setFocus(tl->surface());
                tl->configureState(tl->pending().state | LToplevelRole::Activated);
            }

            if (animatedFullscreenToplevel)
            {
                Toplevel *tl = animatedFullscreenToplevel;

                tl->blackFullscreenBackground.setVisible(false);

                if (tl->capture.texture())
                    delete tl->capture.texture();

                tl->animView.setTexture(nullptr);

                if (tl->animScene)
                {
                    delete tl->animScene;
                    tl->animScene = nullptr;
                }

                if (tl->destructorCalled || tl->quickUnfullscreen)
                    goto returnChildren;

                if (tl->fullscreen())
                {
                    tl->surf()->setPos(pos().x(), 0);
                    G::reparentWithSubsurfaces(tl->surf(), &tl->fullscreenWorkspace->surfaces);
                    currentWorkspace->clipChildren();
                }
                else
                {
                    tl->surf()->setPos(tl->prevRect.pos());
                    G::reparentWithSubsurfaces(tl->surf(), &workspaces.front()->surfaces, false);
                    G::repositionNonVisibleToplevelChildren(this, tl->surf());
                    tl->surf()->getView()->setVisible(true);
                    tl->surf()->raise();
                    delete tl->fullscreenWorkspace;
                    tl->fullscreenWorkspace = nullptr;
                }
                animatedFullscreenToplevel = nullptr;

                if (tl->decoratedView)
                    tl->decoratedView->updateGeometry();
            }

            returnChildren:
            for (Output *o : G::outputs())
                if (!o->swipingWorkspace)
                    o->currentWorkspace->returnChildren();

            updateWorkspacesPos();
            G::scene()->mainView()->damageAll(this);
            repaint();
        });

    workspacesContainer = new LLayerView(&G::compositor()->workspacesLayer);
    workspacesContainer->enableParentOffset(false);
    workspacesContainer->setPos(0, 0);
    currentWorkspace = new Workspace(this);

    new Topbar(this);
    new Dock(this);
    loadWallpaper();

    updateWorkspacesPos();
    topbar->update();
    dock->update();
    wallpaperView->setPos(pos());
    wallpaperView->enablePointerEvents(true);
    wallpaperView->enableBlockPointer(true);
    wallpaperView->setUserData(WallpaperType);
    G::compositor()->scene.handleInitializeGL(this);
}

void Output::resizeGL()
{
    G::arrangeOutputs();
    updateWorkspacesPos();
    setWorkspace(currentWorkspace, 1);
    topbar->updateOutputInfo();
    topbar->update();
    dock->update();
    loadWallpaper();
    G::compositor()->scene.handleResizeGL(this);
}

void Output::moveGL()
{
    updateWorkspacesPos();
    topbar->update();
    dock->update();
    wallpaperView->setPos(pos());
    setWorkspace(currentWorkspace, 1);
    G::compositor()->scene.handleMoveGL(this);
}

void Output::paintGL()
{
    // Show black screen during output removal
    if (!G::compositor()->checkUpdateOutputUnplug())
    {
        painter()->clearScreen();
        repaint();
        return;
    }

    if (needsFullRepaint())
        G::scene()->mainView()->damageAll(this);

    // Check pointer events before painting
    if (G::compositor()->updatePointerBeforePaint)
    {
        seat()->pointer()->pointerMoveEvent(LPointerMoveEvent());
        G::compositor()->updatePointerBeforePaint = false;
    }

    updateFractionalOversampling();
    G::scene()->handlePaintGL(this);

    for (auto *screenshotRequest : screenshotRequests())
        screenshotRequest->accept(true);
}

void Output::uninitializeGL()
{
    G::compositor()->outputUnplugHandled = false;

    // Find another output
    Output *aliveOutput = nullptr;

    for (Output *o : G::outputs())
    {
        if (o != this)
        {
            aliveOutput = o;
            break;
        }
    }

    // Unfullscreen toplevels
    while (workspaces.size() != 1)
    {
        Toplevel *tl = workspaces.back()->toplevel;
        tl->surf()->sendOutputEnterEvent(aliveOutput);
        tl->outputUnplugConfigureCount = 0;
        tl->prevStates = LToplevelRole::Activated;
        tl->prevRect.setPos(LPoint(0, TOPBAR_HEIGHT));
        tl->configureSize(tl->prevRect.size());
        tl->configureState(LToplevelRole::Activated);
        tl->quickUnfullscreen = true;
        tl->unsetFullscreen();
        tl->surf()->localOutputPos = tl->prevRect.pos() - pos();
        tl->surf()->localOutputSize = size();
        tl->surf()->outputUnplugHandled = false;
        workspaceAnim.stop();
    }

    workspacesContainer->setPos(0, 0);

    for (Surface *s : G::surfaces())
    {
        if (s->cursorRole() || (s->toplevel() && s->toplevel()->fullscreen()))
            continue;

        Output *intersectedOutput = G::mostIntersectedOuput(s->getView());

        if (intersectedOutput == this)
        {
            s->localOutputPos = s->pos() - pos();
            s->localOutputSize = size();
            s->outputUnplugHandled = false;
        }
        else if (!intersectedOutput)
        {
            s->localOutputPos = LPoint(200, 200);
            s->localOutputSize = size();
            s->outputUnplugHandled = false;
        }

        if (s->minimizedOutput == this)
        {
            s->minimizeAnim.stop();
            s->minimizedOutput = aliveOutput;
            s->minimizeStartRect.setPos(LPoint(rand() % 128, TOPBAR_HEIGHT + (rand() % 128)));
        }
    }

    delete dock;
    dock = nullptr;
    delete topbar;
    topbar = nullptr;

    if (wallpaperView->texture())
        delete wallpaperView->texture();

    delete wallpaperView;
    wallpaperView = nullptr;

    workspaceAnim.stop();

    while (!workspaces.empty())
        delete workspaces.back();

    delete workspacesContainer;
    workspacesContainer = nullptr;

    currentWorkspace = nullptr;
    animatedFullscreenToplevel = nullptr;

    G::compositor()->scene.handleUninitializeGL(this);
}

void Output::setGammaRequest(LClient *client, const LGammaTable *gamma)
{
    L_UNUSED(client);
    setGamma(gamma);
}
