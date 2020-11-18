/*
 * KWin Style UKUI
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "shaderhelper.h"

#include <QMap>
#include <kwinglplatform.h>

static QMap<ShaderHelper::Direction, KWin::GLShader *> shaders;

KWin::GLShader *ShaderHelper::getShader()
{
    // copy from kwinglutils.cpp
    QByteArray source;
    QTextStream stream(&source);

    KWin::GLPlatform * const gl = KWin::GLPlatform::instance();
    QByteArray varying, output, textureLookup;

    // caculate corner coord relatived with topleft corner, used for corner texture sampling.
    QByteArray cornerCoord = "    vec2 cornerCoord = vec2(texcoordC.x * scale.x, texcoordC.y * scale.y);\n";
    QByteArray cornerCoordTR = "    vec2 cornerCoord = vec2((1 - texcoordC.x) * scale1.x, texcoordC.y * scale1.y);\n";
    QByteArray cornerCoordBL = "    vec2 cornerCoord = vec2(texcoordC.x * scale2.x, (1 - texcoordC.y) * scale2.y);\n";
    QByteArray cornerCoordBR = "    vec2 cornerCoord = vec2((1 - texcoordC.x) * scale3.x, (1 - texcoordC.y) * scale3.y);\n";

    if (!gl->isGLES()) {
        const bool glsl_140 = gl->glslVersion() >= KWin::kVersionNumber(1, 40);

        if (glsl_140)
            stream << "#version 140\n\n";

        varying       = glsl_140 ? QByteArrayLiteral("in")         : QByteArrayLiteral("varying");
        textureLookup = glsl_140 ? QByteArrayLiteral("texture")    : QByteArrayLiteral("texture2D");
        output        = glsl_140 ? QByteArrayLiteral("fragColor")  : QByteArrayLiteral("gl_FragColor");
    } else {
        const bool glsl_es_300 = KWin::GLPlatform::instance()->glslVersion() >= KWin::kVersionNumber(3, 0);

        if (glsl_es_300)
            stream << "#version 300 es\n\n";

        // From the GLSL ES specification:
        //
        //     "The fragment language has no default precision qualifier for floating point types."
        stream << "precision highp float;\n\n";

        varying       = glsl_es_300 ? QByteArrayLiteral("in")         : QByteArrayLiteral("varying");
        textureLookup = glsl_es_300 ? QByteArrayLiteral("texture")    : QByteArrayLiteral("texture2D");
        output        = glsl_es_300 ? QByteArrayLiteral("fragColor")  : QByteArrayLiteral("gl_FragColor");
    }

    KWin::ShaderTraits traits;

    traits |= KWin::ShaderTrait::MapTexture;
    traits |= KWin::ShaderTrait::Modulate;
    traits |= KWin::ShaderTrait::AdjustSaturation;

    if (traits & KWin::ShaderTrait::MapTexture) {
        stream << "uniform sampler2D sampler;\n";

        // custom texture
        stream << "uniform sampler2D topleft;\n";
        stream << "uniform sampler2D topright;\n";
        stream << "uniform sampler2D bottomleft;\n";
        stream << "uniform sampler2D bottomright;\n";

        // scale
        stream << "uniform vec2 scale;\n";
        stream << "uniform vec2 scale1;\n";
        stream << "uniform vec2 scale2;\n";
        stream << "uniform vec2 scale3;\n";

        if (traits & KWin::ShaderTrait::Modulate)
            stream << "uniform vec4 modulation;\n";
        if (traits & KWin::ShaderTrait::AdjustSaturation)
            stream << "uniform float saturation;\n";

        stream << "\n" << varying << " vec2 texcoord0;\n";

    } else if (traits & KWin::ShaderTrait::UniformColor)
        stream << "uniform vec4 geometryColor;\n";

    if (traits & KWin::ShaderTrait::ClampTexture) {
        stream << "uniform vec4 textureClamp;\n";
    }    

    if (output != QByteArrayLiteral("gl_FragColor"))
        stream << "\nout vec4 " << output << ";\n";

    stream << "\nvoid main(void)\n{\n";
    if (traits & KWin::ShaderTrait::MapTexture) {
        stream << "vec2 texcoordC = texcoord0;\n";


        stream << "    " << "vec4 var;\n";
        stream << "if (texcoordC.x < 0.5) {\n"
                  "    if (texcoordC.y < 0.5) {\n"
                  "        vec2 cornerCoord = vec2(texcoordC.x * scale.x, texcoordC.y * scale.y);\n"
                  "        var = " << textureLookup << "(topleft, cornerCoord);\n"
                  "    } else {\n"
                  "        vec2 cornerCoordBL = vec2(texcoordC.x * scale2.x, (1 - texcoordC.y) * scale2.y);\n"
                  "        var = " << textureLookup << "(bottomleft, cornerCoordBL);\n"
                  "    }\n"
                  "} else {\n"
                  "    if (texcoordC.y < 0.5) {\n"
                  "        vec2 cornerCoordTR = vec2((1 - texcoordC.x) * scale1.x, texcoordC.y * scale1.y);\n"
                  "        var = " << textureLookup << "(topright, cornerCoordTR);\n"
                  "    } else {\n"
                  "        vec2 cornerCoordBR = vec2((1 - texcoordC.x) * scale3.x, (1 - texcoordC.y) * scale3.y);\n"
                  "        var = " << textureLookup << "(bottomright, cornerCoordBR);\n"
                  "    }\n"
                  "}\n";

        stream << "    vec4 texel = " << textureLookup << "(sampler, texcoordC);\n";
        if (traits & KWin::ShaderTrait::Modulate)
            stream << "    texel *= modulation;\n";
        if (traits & KWin::ShaderTrait::AdjustSaturation)
            stream << "    texel.rgb = mix(vec3(dot(texel.rgb, vec3(0.2126, 0.7152, 0.0722))), texel.rgb, saturation);\n";

        stream << "    " << output << " = texel * var;\n";
    } else if (traits & KWin::ShaderTrait::UniformColor)
        stream << "    " << output << " = geometryColor;\n";

    stream << "}";
    stream.flush();

    auto shader = KWin::ShaderManager::instance()->generateCustomShader(traits, QByteArray(), source);
    //shaders.insert(direction, shader);
    return shader;
}

void ShaderHelper::releaseShaders()
{
//    for (auto shader : shaders) {
//        delete shader;
//    }

//    shaders.clear();

    //KWin::ShaderManager::instance()->cleanup();
}

ShaderHelper::ShaderHelper(QObject *parent) : QObject(parent)
{

}
