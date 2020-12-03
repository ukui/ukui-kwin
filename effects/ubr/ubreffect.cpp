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

#include "ubreffect.h"
#include "xatom-helper.h"
#include "ubrtexturehelper.h"
#include "shaderhelper.h"

#include <kwineffects.h>
#include <kwinglutils.h>
#include <kwinglplatform.h>

#include <QPainterPath>
#include <QX11Info>
#include <QDebug>

#include <QMetaMethod>

#include <QVector4D>

#define UNITY_BORDER_RADIUS 6

static QList<KWin::EffectWindow *> maximizedWindows;

typedef void (*ToplevelSetDepth)(void *, int);

static ToplevelSetDepth setDepthfunc = nullptr;

UBREffect::UBREffect(QObject *parent, const QVariantList &args)
{
    // try get Toplevel::setDepth(), for resolving ubr effect has black corners of opaque window.
    setDepthfunc = (ToplevelSetDepth) QLibrary::resolve("ukui-kwin.so." + qApp->applicationVersion(), "_ZN4KWin8Toplevel8setDepthEi");

    m_ubrShader = ShaderHelper::getShader();
    if (!m_ubrShader->isValid()) {
        m_ubrShader = KWin::ShaderManager::instance()->generateCustomShader(KWin::ShaderTrait::MapTexture|KWin::ShaderTrait::Modulate|KWin::ShaderTrait::AdjustSaturation);
    }

    auto effectsManager = KWin::effects;

    for (auto window : effectsManager->stackingOrder()) {
        bool isUKUIDecoration = XAtomHelper::getInstance()->isUKUIDecorationWindow(window);
        window->setData(IsUKUIDecoration, isUKUIDecoration);

        if (!window->data(UnityBorderRadius).isValid()) {
            auto ubr = XAtomHelper::getInstance()->getWindowBorderRadius(window);
            if (ubr.topLeft <= 0) {
                ubr.topLeft = UNITY_BORDER_RADIUS;
            }
            if (ubr.topRight <= 0) {
                ubr.topRight = UNITY_BORDER_RADIUS;
            }
            if (ubr.bottomLeft <= 0) {
                ubr.bottomLeft = UNITY_BORDER_RADIUS;
            }
            if (ubr.bottomRight <= 0) {
                ubr.bottomRight = UNITY_BORDER_RADIUS;
            }
            window->setData(UnityBorderRadius, QVector4D(ubr.topLeft, ubr.topRight, ubr.bottomLeft, ubr.bottomRight));
        }

        bool isCSD = XAtomHelper::getInstance()->isWindowDecorateBorderOnly(window);
        window->setData(IsCSD, isCSD);
    }

    connect(effectsManager, &KWin::EffectsHandler::windowAdded, this, [=](KWin::EffectWindow *window){
        bool isUKUIDecoration = XAtomHelper::getInstance()->isUKUIDecorationWindow(window);
        window->setData(IsUKUIDecoration, isUKUIDecoration);

        auto ubr = XAtomHelper::getInstance()->getWindowBorderRadius(window);
        if (ubr.topLeft <= 0) {
            ubr.topLeft = UNITY_BORDER_RADIUS;
        }
        if (ubr.topRight <= 0) {
            ubr.topRight = UNITY_BORDER_RADIUS;
        }
        if (ubr.bottomLeft <= 0) {
            ubr.bottomLeft = UNITY_BORDER_RADIUS;
        }
        if (ubr.bottomRight <= 0) {
            ubr.bottomRight = UNITY_BORDER_RADIUS;
        }
        window->setData(UnityBorderRadius, QVector4D(ubr.topLeft, ubr.topRight, ubr.bottomLeft, ubr.bottomRight));

        bool isCSD = XAtomHelper::getInstance()->isWindowDecorateBorderOnly(window);
        window->setData(IsCSD, isCSD);
    });

    connect(effectsManager, &KWin::EffectsHandler::windowDeleted, this, [=](KWin::EffectWindow *window){
        maximizedWindows.removeOne(window);

        window->setData(IgnoreUBR, QVariant());
        window->setData(UnityBorderRadius, QVariant());
        window->setData(IsCSD, QVariant());
        window->setData(IsUKUIDecoration, QVariant());
    });
}

UBREffect::~UBREffect()
{
    //ShaderHelper::releaseShaders();

    // clear the dirty texture.
    // NOTE: do not comment this code.
    UBRTextureHelper::getInstance()->release();

    for (auto window : KWin::effects->stackingOrder()) {
        window->setData(IgnoreUBR, QVariant());
        //window->setData(UnityBorderRadius, QVariant());
    }
}

bool UBREffect::supported()
{
    bool supported = KWin::effects->isOpenGLCompositing() && KWin::GLRenderTarget::supported() && KWin::GLRenderTarget::blitSupported();
    return supported;
}

bool UBREffect::enabledByDefault()
{
    return true;
}

void UBREffect::prePaintScreen(KWin::ScreenPrePaintData &data, int time)
{
    KWin::Effect::prePaintScreen(data, time);
}

void UBREffect::prePaintWindow(KWin::EffectWindow *w, KWin::WindowPrePaintData &data, int time)
{
    return KWin::Effect::prePaintWindow(w, data, time);
}

void UBREffect::drawWindow(KWin::EffectWindow *w, int mask, const QRegion &region, KWin::WindowPaintData &data)
{
    if (!w->data(IsUKUIDecoration).toBool()) {
        return KWin::Effect::drawWindow(w, mask, region, data);
    }

    if (!KWin::effects->isOpenGLCompositing()) {
        return KWin::Effect::drawWindow(w, mask, region, data);
    }

    if (w->data(IgnoreUBR).isValid()) {
        return KWin::Effect::drawWindow(w, mask, region, data);
    }

    if (!w->isManaged()) {
        return KWin::Effect::drawWindow(w, mask, region, data);
    }

    if (!w->isPaintingEnabled() || ((mask & PAINT_WINDOW_LANCZOS))) {
        return KWin::Effect::drawWindow(w, mask, region, data);
    }

    if (w->windowType() != NET::WindowType::Normal && !w->isDialog()  && !w->isUtility()) {
        return KWin::Effect::drawWindow(w, mask, region, data);
    }

    if ((mask & PAINT_WINDOW_TRANSFORMED) && !(mask & PAINT_SCREEN_TRANSFORMED)) {
        //return KWin::Effect::drawWindow(w, mask, region, data);
    }

    if (maximizedWindows.contains(w)) {
        return KWin::Effect::drawWindow(w, mask, region, data);
    }

    if (!w->hasAlpha()) {
        //return KWin::Effect::drawWindow(w, mask, region, data);
        if (setDepthfunc) {
            setDepthfunc(w->parent(), 32);
        }
    }

    auto ubr = qvariant_cast<QVector4D>(w->data(UnityBorderRadius));

    KWin::WindowPaintData tmpData = data;

//    KWin::WindowQuadList windowQuads;
//    KWin::WindowQuadList shadowQuads;
//    KWin::WindowQuadList decorationQuads;

//    for (auto quad : tmpData.quads) {
//        switch (quad.type()) {
//        case KWin::WindowQuadContents: {
//            windowQuads<<quad;
//            break;
//        }
//        case KWin::WindowQuadDecoration: {
//            decorationQuads<<quad;
//            break;
//        }
//        case KWin::WindowQuadError: {
//            break;
//        }
//        case KWin::WindowQuadShadow:
//        case KWin::WindowQuadShadowTop:
//        case KWin::WindowQuadShadowTopRight:
//        case KWin::WindowQuadShadowRight:
//        case KWin::WindowQuadShadowBottomRight:
//        case KWin::WindowQuadShadowBottom:
//        case KWin::WindowQuadShadowBottomLeft:
//        case KWin::WindowQuadShadowLeft:
//        case KWin::WindowQuadShadowTopLeft: {
//            shadowQuads<<quad;
//        }
//        default: {
//            break;
//        }
//        }
//    }

    // draw window shadow
//    tmpData.quads = shadowQuads;
//    KWin::Effect::drawWindow(w, mask, region, tmpData);

//    // draw decoration
//    tmpData.quads = decorationQuads;
//    KWin::Effect::drawWindow(w, mask, region, tmpData);

//    // draw normal area

//    tmpData.quads = windowQuads;

//    tmpData.quads.clear();
//    tmpData.quads<<decorationQuads<<windowQuads;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    auto textureTopLeft = UBRTextureHelper::getInstance()->getTexture(ubr.x());
    glActiveTexture(GL_TEXTURE10);
    textureTopLeft->bind();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glActiveTexture(GL_TEXTURE0);

    auto textureTopRight = UBRTextureHelper::getInstance()->getTexture(ubr.y());
    glActiveTexture(GL_TEXTURE11);
    textureTopRight->bind();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glActiveTexture(GL_TEXTURE0);

    auto textureBottomLeft = UBRTextureHelper::getInstance()->getTexture(ubr.z());
    glActiveTexture(GL_TEXTURE12);
    textureBottomLeft->bind();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glActiveTexture(GL_TEXTURE0);

    auto textureBottomRight = UBRTextureHelper::getInstance()->getTexture(ubr.w());
    glActiveTexture(GL_TEXTURE13);
    textureBottomRight->bind();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glActiveTexture(GL_TEXTURE0);

    tmpData.shader = m_ubrShader;
    KWin::ShaderManager::instance()->pushShader(m_ubrShader);

    m_ubrShader->setUniform("topleft", 10);
    m_ubrShader->setUniform("scale", QVector2D(w->width()*1.0/textureTopLeft->width(), w->height()*1.0/textureTopLeft->height()));

    m_ubrShader->setUniform("topright", 11);
    m_ubrShader->setUniform("scale1", QVector2D(w->width()*1.0/textureTopRight->width(), w->height()*1.0/textureTopRight->height()));

    m_ubrShader->setUniform("bottomleft", 12);
    m_ubrShader->setUniform("scale2", QVector2D(w->width()*1.0/textureBottomLeft->width(), w->height()*1.0/textureBottomLeft->height()));

    m_ubrShader->setUniform("bottomright", 13);
    m_ubrShader->setUniform("scale3", QVector2D(w->width()*1.0/textureBottomRight->width(), w->height()*1.0/textureBottomRight->height()));

    KWin::Effect::drawWindow(w, mask, region, tmpData);

    KWin::ShaderManager::instance()->popShader();

    glActiveTexture(GL_TEXTURE10);
    textureTopLeft->unbind();
    glActiveTexture(GL_TEXTURE0);

    glActiveTexture(GL_TEXTURE11);
    textureTopRight->unbind();
    glActiveTexture(GL_TEXTURE0);

    glActiveTexture(GL_TEXTURE12);
    textureBottomLeft->unbind();
    glActiveTexture(GL_TEXTURE0);

    glActiveTexture(GL_TEXTURE13);
    textureBottomRight->unbind();
    glActiveTexture(GL_TEXTURE0);

    glDisable(GL_BLEND);

    return;
}
