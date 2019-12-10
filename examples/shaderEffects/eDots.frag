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

#version 330 core
layout(location = 0) out vec4 fragColor;

in vec2 texCoord;
layout(pixel_center_integer) in vec4 gl_FragCoord;

uniform sampler2D texture;
uniform vec2 eGlobalPos;
uniform float dotDistance;
uniform float dotRadius;

void main(void) {
    bool inDot;
    float mixAlpha;

    vec2 transformedCoord = eGlobalPos + gl_FragCoord.xy;

    vec2 dotID = floor(transformedCoord/(2.f*(dotRadius + dotDistance)));
    vec2 posInDot = transformedCoord - dotID*2.f*(dotDistance + dotRadius);
    vec2 posRelDotCenter = posInDot - dotRadius;
    float distToCenter = sqrt(posRelDotCenter.x*posRelDotCenter.x +
                              posRelDotCenter.y*posRelDotCenter.y);
    float distToEdge = distToCenter - sqrt(dotRadius*dotRadius);
    if(distToEdge > 0.f) {
        inDot = false;
    } else {
        distToEdge = abs(distToEdge);
        inDot = true;
        if(distToEdge < 1.f) {
            mixAlpha = distToEdge;
        } else {
            mixAlpha = 1.f;
        }
    }
    if(inDot) {
        vec4 texCol = texture2D(texture, texCoord);
        fragColor =  vec4(mixAlpha*texCol.rgb, mixAlpha*texCol.a);
    } else {
        fragColor =  vec4(0.f, 0.f, 0.f, 0.f);
    }
}
