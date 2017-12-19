
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkShadowUtils_DEFINED
#define SkShadowUtils_DEFINED

#include "SkColor.h"
#include "SkPoint3.h"
#include "SkScalar.h"
#include "../private/SkShadowFlags.h"

class SkCanvas;
class SkPath;
class SkResourceCache;

class SK_API SkShadowUtils {
public:
    /**
     * Draw an offset spot shadow and outlining ambient shadow for the given path using a disc
     * light. The shadow may be cached, depending on the path type and canvas matrix. If the
     * matrix is perspective or the path is volatile, it will not be cached.
     *
     * @param canvas  The canvas on which to draw the shadows.
     * @param path  The occluder used to generate the shadows.
     * @param zPlaneParams  Values for the plane function which returns the Z offset of the
     *  occluder from the canvas based on local x and y values (the current matrix is not applied).
     * @param lightPos  The 3D position of the light relative to the canvas plane. This is
     *  independent of the canvas's current matrix.
     * @param lightRadius  The radius of the disc light.
     * @param ambientAlpha  The maximum alpha of the ambient shadow.
     * @param spotAlpha  The maxium alpha of the spot shadow.
     * @param color  The shadow color.
     * @param flags  Options controlling opaque occluder optimizations and shadow appearance. See
     *               SkShadowFlags.
     */
    static void DrawShadow(SkCanvas* canvas, const SkPath& path, const SkPoint3& zPlaneParams,
                           const SkPoint3& lightPos, SkScalar lightRadius, SkScalar ambientAlpha,
                           SkScalar spotAlpha, SkColor color,
                           uint32_t flags = SkShadowFlags::kNone_ShadowFlag);

    /**
    * Draw an offset spot shadow and outlining ambient shadow for the given path using a disc
    * light.
    *
    * Deprecated version with height value (to be removed when Flutter is updated).
    *
    * @param canvas  The canvas on which to draw the shadows.
    * @param path  The occluder used to generate the shadows.
    * @param occluderHeight  The vertical offset of the occluder from the canvas. This is
    *  independent of the canvas's current matrix.
    * @param lightPos  The 3D position of the light relative to the canvas plane. This is
    *  independent of the canvas's current matrix.
    * @param lightRadius  The radius of the disc light.
    * @param ambientAlpha  The maximum alpha of the ambient shadow.
    * @param spotAlpha  The maxium alpha of the spot shadow.
    * @param color  The shadow color.
    * @param flags  Options controlling opaque occluder optimizations and shadow appearance. See
    *               SkShadowFlags.
    */
    static void DrawShadow(SkCanvas* canvas, const SkPath& path, SkScalar occluderHeight,
                           const SkPoint3& lightPos, SkScalar lightRadius, SkScalar ambientAlpha,
                           SkScalar spotAlpha, SkColor color,
                           uint32_t flags = SkShadowFlags::kNone_ShadowFlag) {
        SkPoint3 zPlane = SkPoint3::Make(0, 0, occluderHeight);
        DrawShadow(canvas, path, zPlane, lightPos, lightRadius, ambientAlpha, spotAlpha,
                   color, flags);
    }

    /**
     * Helper routine to compute scale alpha values for one-pass tonal alpha.
     *
     * The final color we want to emulate is generated by rendering a color shadow (C_rgb) using an
     * alpha computed from the color's luminance (C_a), and then a black shadow with alpha (S_a)
     * which is an adjusted value of 'a'.  Assuming SrcOver, a background color of B_rgb, and
     * ignoring edge falloff, this becomes
     *
     *     (C_a - S_a*C_a)*C_rgb + (1 - (S_a + C_a - S_a*C_a))*B_rgb
     *
     * Since we use premultiplied alpha, this means we can scale the color by (C_a - S_a*C_a) and
     * set the alpha to (S_a + C_a - S_a*C_a).
     *
     * @param r  Red value of color
     * @param g  Green value of color
     * @param b  Blue value of color
     * @param a  Alpha value of color
     * @param colorScale  Factor to scale color values by
     * @param tonalAlpha  Value to set alpha to
     */
    static inline void ComputeTonalColorParams(SkScalar r, SkScalar g, SkScalar b, SkScalar a,
                                               SkScalar* colorScale, SkScalar* tonalAlpha) {
        SkScalar max = SkTMax(SkTMax(r, g), b);
        SkScalar min = SkTMin(SkTMin(r, g), b);
        SkScalar luminance = 0.5f*(max + min);

        // We compute a color alpha value based on the luminance of the color, scaled by an
        // adjusted alpha value. We want the following properties to match the UX examples
        // (assuming a = 0.25) and to ensure that we have reasonable results when the color
        // is black and/or the alpha is 0:
        //     f(0, a) = 0
        //     f(luminance, 0) = 0
        //     f(1, 0.25) = .5
        //     f(0.5, 0.25) = .4
        //     f(1, 1) = 1
        // The following functions match this as closely as possible.
        SkScalar alphaAdjust = (2.6f + (-2.66667f + 1.06667f*a)*a)*a;
        SkScalar colorAlpha = (3.544762f + (-4.891428f + 2.3466f*luminance)*luminance)*luminance;
        colorAlpha = SkTPin(alphaAdjust*colorAlpha, 0.0f, 1.0f);

        // Similarly, we set the greyscale alpha based on luminance and alpha so that
        //     f(0, a) = a
        //     f(luminance, 0) = 0
        //     f(1, 0.25) = 0.15
        SkScalar greyscaleAlpha = SkTPin(a*(1 - 0.4f*luminance), 0.0f, 1.0f);

        *colorScale = colorAlpha*(SK_Scalar1 - greyscaleAlpha);
        *tonalAlpha = *colorScale + greyscaleAlpha;
    }

};

#endif
