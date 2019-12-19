// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SHADEREFFECTCALLER_H
#define SHADEREFFECTCALLER_H
#include "RasterEffects/rastereffectcaller.h"
#include "shadereffectprogram.h"
#include "../gpurendertools.h"

class ShaderEffectCaller : public RasterEffectCaller {
    e_OBJECT
public:
    ShaderEffectCaller(const ShaderEffectProgram& program);

    void processGpu(QGL33 * const gl,
                    GpuRenderTools& renderTools,
                    GpuRenderData& data);

    QJSEngine& getJSEngine()
    { return mEngine; }

    UniformSpecifiers mUniformSpecifiers;
protected:
    QMargins getMargin(const SkIRect &srcRect);
private:
    void setupProgram(QGL33 * const gl,
                      QJSEngine& mEngine,
                      const GpuRenderData &data);

    QJSEngine mEngine;
    const ShaderEffectProgram mProgram;
};


#endif // SHADEREFFECTCALLER_H
