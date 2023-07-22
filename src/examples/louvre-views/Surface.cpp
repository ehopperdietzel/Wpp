#include "Compositor.h"
#include "LTime.h"
#include "Surface.h"
#include "LCursor.h"
#include "Output.h"
#include <Global.h>
#include <LTextureView.h>
#include <Dock.h>
#include <LAnimation.h>

Surface::Surface(LSurface::Params *params) : LSurface(params)
{
    view = new LSurfaceView(this, G::compositor()->surfacesLayer);
}

Surface::~Surface()
{
    if (minimizeAnim)
        minimizeAnim->stop();

    while (!minimizedViews.empty())
        delete minimizedViews.back();

    if (thumbnailFullsizeView)
        delete thumbnailFullsizeView;

    if (thumbnailFullSizeTex)
        delete thumbnailFullSizeTex;

    if (thumbnailTex)
        delete thumbnailTex;

    delete view;
}

void Surface::mappingChanged()
{
    if (mapped())
    {
        if (firstMap)
        {
            firstMap = false;

            if (toplevel())
            {
                LPoint outputPosG = compositor()->cursor()->output()->pos() + LPoint(0, TOPBAR_HEIGHT);
                LSize outputSizeG = compositor()->cursor()->output()->size() - LSize(0, TOPBAR_HEIGHT);

                setPos(outputPosG + outputSizeG/2 - toplevel()->windowGeometry().size()/2);

                if (pos().x() < outputPosG.x())
                    setX(outputPosG.x());

                if (pos().y() < TOPBAR_HEIGHT)
                    setY(TOPBAR_HEIGHT);

                toplevel()->configure(LToplevelRole::Activated);
            }
        }

        compositor()->repaintAllOutputs();
    }
    else
    {
        view->repaint();
    }
}

void Surface::orderChanged()
{
    Surface *prev = (Surface*)prevSurface();

    if (prev)
        view->insertAfter(prev->view, false);
    else
        view->insertAfter(nullptr, false);
}

void Surface::roleChanged()
{
    if (roleId() == LSurface::Cursor)
    {
        view->enableForceRequestNextFrame(true);
        view->setVisible(false);
        view->setParent(G::compositor()->hiddenCursorsLayer);
    }
}

void Surface::bufferSizeChanged()
{
    view->repaint();
}

void Surface::minimizedChanged()
{
    if (minimized())
    {
        // When a surface is minimized, its children are too, so lets just hide them
        if (!toplevel())
        {
            view->setVisible(false);
            return;
        }

        // Render the surface, all its decorations and subsurfaces into a texture
        thumbnailFullSizeTex = renderThumbnail();

        // Create a smaller scaled version for the dock
        thumbnailTex = thumbnailFullSizeTex->copyB(LSize((DOCK_ITEM_HEIGHT * thumbnailFullSizeTex->sizeB().w()) /thumbnailFullSizeTex->sizeB().h(), DOCK_ITEM_HEIGHT) * view->bufferScale());

        // Create a view for thumbnailFullSizeTex (we only need one)
        thumbnailFullsizeView = new LTextureView(thumbnailFullSizeTex, G::compositor()->surfacesLayer);
        thumbnailFullsizeView->setBufferScale(view->bufferScale());
        thumbnailFullsizeView->enableScaling(true);
        thumbnailFullsizeView->enableParentOpacity(false);
        thumbnailFullsizeView->setPos(rolePos());

        // Hide the surface as we will show thumbnailFullsizeView instead
        view->setVisible(false);

        // We will move the fullsize view to the dock where the cursor is currently at
        DockItem *dstDockItem;

        // Create a dock item for each output dock
        for (Output *o : G::outputs())
        {
            DockItem *minView = new DockItem(this, o->dock);

            if (cursor()->output() == o)
                dstDockItem = minView;
        }

        minimizeAnim = LAnimation::create(300,
        [this, dstDockItem](LAnimation *anim)
        {
            // Transform linear curve to ease out
            Float32 easeOut = 1.f - powf(1.f - anim->value(), 2.f);

            // Animate all docks items
            for (DockItem *item : minimizedViews)
            {
                item->setScalingVector(easeOut);
                item->dock->update();
            }

            // Scale and move fullsize view to the dock
            thumbnailFullsizeView->setScalingVector(1.f - easeOut);
            thumbnailFullsizeView->setPos((dstDockItem->pos() + dstDockItem->size()) * easeOut +
                     rolePos() * (1.f - easeOut));

            return true;
        },
        [this](LAnimation *)
        {
            // Finish docks items animations
            for (DockItem *item : minimizedViews)
            {
                item->setScalingVector(1.f);
                item->enableScaling(false);
                item->dock->update();
            }

            // Hide the resized fullsize view
            thumbnailFullsizeView->setVisible(false);
            minimizeAnim = nullptr;
        });

        minimizeAnim->start();

        if (toplevel())
            toplevel()->configure(0);
    }
    else
    {
        compositor()->raiseSurface(this);
        if (toplevel())
            toplevel()->configure(LToplevelRole::Activated);

        view->setVisible(true);
        view->enableInput(true);
    }
}

LTexture *Surface::renderThumbnail()
{
    LSceneView tmpView = LSceneView(view->size()*view->bufferScale(), view->bufferScale());
    tmpView.setPos(rolePos());
    LView *prevParent = view->parent();
    view->setParent(&tmpView);

    std::list<LSurfaceView*>tmpChildren;

    Surface *next = this;
    while ((next = (Surface*)next->nextSurface()))
    {
        if (next->parent() == this)
        {
            next->view->enableParentOffset(false);
            next->view->setParent(&tmpView);
            tmpChildren.push_back(next->view);
        }
    }

    view->enableParentOffset(false);
    tmpView.render();
    LTexture *renderedThumbnail = tmpView.texture()->copyB();
    view->enableParentOffset(true);
    view->setParent(prevParent);

    while (!tmpChildren.empty())
    {
        tmpChildren.front()->enableParentOffset(false);
        tmpChildren.front()->setParent(prevParent);
        tmpChildren.pop_front();
    }

    return renderedThumbnail;
}

void Surface::unminimize(DockItem *clickedItem)
{
    // Show the resized fullsize view
    thumbnailFullsizeView->setVisible(true);
    thumbnailFullsizeView->insertAfter(G::compositor()->surfacesLayer->children().back());

    // Setup dock items
    for (DockItem *item : minimizedViews)
    {
        item->enableInput(false);
        item->setOpacity(1.f);
        item->setScalingVector(1.f);
        item->enableScaling(true);
    }

    minimizeAnim = LAnimation::create(300,
    [this, clickedItem](LAnimation *anim)
    {
        // Transform linear curve to ease out
        Float32 exp = powf(anim->value(), 2.f);

        // Animate all docks items
        for (DockItem *item : minimizedViews)
        {
            item->setScalingVector(1.f - exp);
            item->dock->update();
        }

        // Scale and move fullsize view to the dock
        thumbnailFullsizeView->setScalingVector(exp);
        thumbnailFullsizeView->setPos((clickedItem->pos() + clickedItem->size()) * (1.f - exp) +
                 rolePos() * exp);

        return true;
    },
    [this](LAnimation *)
    {
        // Destroy minimized views
        while (!minimizedViews.empty())
        {
            Dock *dock = minimizedViews.back()->dock;
            delete minimizedViews.back();
            dock->update();
        }

        // Destroy the resized fullsize view
        delete thumbnailFullsizeView;
        thumbnailFullsizeView = nullptr;

        // Destroy textures
        delete thumbnailFullSizeTex;
        thumbnailFullSizeTex = nullptr;
        delete thumbnailTex;
        thumbnailTex = nullptr;

        minimizeAnim = nullptr;

        setMinimized(false);
    });

    minimizeAnim->start();
}
