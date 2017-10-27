#ifndef ANDROID_A_DRAW_GL_INFO_H
#define ANDROID_A_DRAW_GL_INFO_H

/**
 * Structure used by OpenGLRenderer::callDrawGLFunction() to pass and
 * receive data from OpenGL functors.
 */
struct ADrawGlInfo {
    // Input: tells the draw function what action to perform.
    enum Mode {
        kModeDraw = 0,
        kModeProcess,
        kModeProcessNoContext,
        kModeSync,
    } mode;

    // Input: current clip rect in surface coordinates. Reflects the current state
    // of the OpenGL scissor rect. Both the OpenGL scissor rect and viewport are
    // set by the caller of the draw function and updated during View animations.
    int clipLeft;
    int clipTop;
    int clipRight;
    int clipBottom;

    // Input: current width/height of destination surface.
    int width;
    int height;

    // Input: is the View rendered into an independent layer.
    // If false, the surface is likely to hold to the full screen contents, with
    // the scissor box set by the caller to the actual View location and size.
    // Also the transformation matrix will contain at least a translation to the
    // position of the View to render, plus any other transformations required as
    // part of any ongoing View animation. View translucency (alpha) is ignored,
    // although the framework will set is_layer to true for non-opaque cases.
    // Can be requested via the View.setLayerType(View.LAYER_TYPE_NONE, ...)
    // Android API method.
    //
    // If true, the surface is dedicated to the View and should have its size.
    // The viewport and scissor box are set by the caller to the whole surface.
    // Animation transformations are handled by the caller and not reflected in
    // the provided transformation matrix. Translucency works normally.
    // Can be requested via the View.setLayerType(View.LAYER_TYPE_HARDWARE, ...)
    // Android API method.
    bool isLayer;

    // Input: current transformation matrix in surface pixels.
    // Uses the column-based OpenGL matrix format.
    float transform[16];

    // Output: dirty region to redraw
    float dirtyLeft;
    float dirtyTop;
    float dirtyRight;
    float dirtyBottom;

    /**
     * Values used by OpenGL functors to tell the framework
     * what to do next.
     */
    enum Status {
        // The functor is done
        kStatusDone = 0x0,
        // The functor is requesting a redraw (the clip rect
        // used by the redraw is specified by DrawGlInfo.)
        // The rest of the UI might redraw too.
        kStatusDraw = 0x1,
        // The functor needs to be invoked again but will
        // not redraw. Only the functor is invoked again
        // (unless another functor requests a redraw.)
        kStatusInvoke = 0x2,
        // DisplayList actually issued GL drawing commands.
        // This is used to signal the HardwareRenderer that the
        // buffers should be flipped - otherwise, there were no
        // changes to the buffer, so no need to flip. Some hardware
        // has issues with stale buffer contents when no GL
        // commands are issued.
        kStatusDrew = 0x4
    };
}; // struct ADrawGlInfo

#endif // ANDROID_A_DRAW_GL_INFO_H