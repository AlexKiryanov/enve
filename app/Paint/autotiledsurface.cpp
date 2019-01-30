#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "colorconversions.h"
#include "autotiledsurface.h"

void autoTiledSurfaceFree(MyPaintSurface *surface) {
    AutoTiledSurface *self = (AutoTiledSurface*)surface;
    self->_free();
}

void autoTiledSurfaceRequestStart(MyPaintTiledSurface *tiled_surface,
                                  MyPaintTileRequest *request) {
    AutoTiledSurface *self = (AutoTiledSurface*)tiled_surface;
    self->_startRequest(request);
}

void autoTiledSurfaceRequestEnd(MyPaintTiledSurface *tiled_surface,
                                MyPaintTileRequest *request) {
    AutoTiledSurface *self = (AutoTiledSurface*)tiled_surface;
    self->_endRequest(request);
}

AutoTiledSurface::AutoTiledSurface() :
    mAutoTilesData(MYPAINT_TILE_SIZE) {
    fMyPaintSurface = &fParent.parent;
    mypaint_tiled_surface_init(&fParent,
                               autoTiledSurfaceRequestStart,
                               autoTiledSurfaceRequestEnd);
    fParent.parent.destroy = autoTiledSurfaceFree;
}

AutoTiledSurface::~AutoTiledSurface() {
    _free();
}

void AutoTiledSurface::loadBitmap(const SkBitmap& src) {
    mAutoTilesData.loadBitmap(src);
}

void AutoTiledSurface::_startRequest(MyPaintTileRequest * const request) {
    //qDebug() << request->tx << request->ty;
    request->buffer = mAutoTilesData.getTileRelToZero(
                request->tx, request->ty);
}

void AutoTiledSurface::_endRequest(MyPaintTileRequest * const request) {
    Q_UNUSED(request);
}
