#ifndef FIXEDTILEDSURFACE_H
#define FIXEDTILEDSURFACE_H

#include <mypaint-config.h>
#include <mypaint-glib-compat.h>
#include <mypaint-tiled-surface.h>
#include <mypaint-brush.h>
#include <QPointF>
#include "smartPointers/stdselfref.h"
#include "pointhelpers.h"
#include "pathoperations.h"
#include "autotilesdata.h"
#include "brushstroke.h"

struct AutoTiledSurface {
    friend struct BrushStroke;
    friend struct BrushStrokeSet;

    ~AutoTiledSurface();
    AutoTiledSurface();
    void loadBitmap(const SkBitmap &src);

    void _free() {
        mypaint_tiled_surface_destroy(&fParent);
    }
    void _startRequest(MyPaintTileRequest * const request);
    void _endRequest(MyPaintTileRequest * const request);

    void paintPressEvent(MyPaintBrush * const brush,
                         const QPointF& pos,
                         const double& dTime,
                         const double& pressure,
                         const double& xtilt,
                         const double& ytilt) {
        mypaint_brush_reset(brush);
        mypaint_brush_new_stroke(brush);

        mypaint_surface_begin_atomic(fMyPaintSurface);
        mypaint_brush_stroke_to(brush,
                                fMyPaintSurface,
                                static_cast<float>(pos.x()),
                                static_cast<float>(pos.y()),
                                static_cast<float>(pressure),
                                static_cast<float>(xtilt),
                                static_cast<float>(ytilt),
                                dTime,
                                static_cast<float>(1.),
                                static_cast<float>(0.));
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(fMyPaintSurface, &roi);
    }

    void paintMoveEvent(MyPaintBrush * const brush,
                        const QPointF &pos,
                        const double& dTime,
                        const double& pressure,
                        const double& xtilt,
                        const double& ytilt) {
        mypaint_surface_begin_atomic(fMyPaintSurface);
        mypaint_brush_stroke_to(brush,
                                fMyPaintSurface,
                                static_cast<float>(pos.x()),
                                static_cast<float>(pos.y()),
                                static_cast<float>(pressure),
                                static_cast<float>(xtilt),
                                static_cast<float>(ytilt),
                                dTime,
                                static_cast<float>(1.),
                                static_cast<float>(0.));
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(fMyPaintSurface, &roi);
    }

    void execute(MyPaintBrush * const brush,
                 BrushStrokeSet& set) {
        set.execute(brush, fMyPaintSurface, 5);
    }

    SkBitmap toBitmap() const {
        return mAutoTilesData.toBitmap();
    }

    QPoint zeroTilePos() const {
        return mAutoTilesData.zeroTilePos();
    }
protected:
    MyPaintTiledSurface fParent;
    MyPaintSurface* fMyPaintSurface = nullptr;

    AutoTilesData mAutoTilesData;
};

#endif // FIXEDTILEDSURFACE_H
