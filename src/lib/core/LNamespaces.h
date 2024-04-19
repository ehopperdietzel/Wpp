#ifndef LNAMESPACES_H
#define LNAMESPACES_H

#include <protocols/Wayland/wayland.h>

#define LOUVRE_MAX_SURFACE_SIZE 10000000
#define LOUVRE_MAX_DMA_PLANES 4
#define LOUVRE_MAX_DAMAGE_RECTS 32

/* Protocols Globals Versions */
#define LOUVRE_FRACTIONAL_SCALE_MANAGER_VERSION 1
#define LOUVRE_WL_COMPOSITOR_VERSION 6
#define LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION 3
#define LOUVRE_WL_SEAT_VERSION 9
#define LOUVRE_WL_OUTPUT_VERSION 4
#define LOUVRE_WL_SUBCOMPOSITOR_VERSION 1
#define LOUVRE_XDG_WM_BASE_VERSION 3
#define LOUVRE_XDG_DECORATION_MANAGER_VERSION 1
#define LOUVRE_XDG_OUTPUT_MANAGER_VERSION 3
#define LOUVRE_PRESENTATION_VERSION 1
#define LOUVRE_LINUX_DMA_BUF_VERSION 5
#define LOUVRE_VIEWPORTER_VERSION 1
#define LOUVRE_GAMMA_CONTROL_MANAGER_VERSION 1
#define LOUVRE_TEARING_CONTROL_MANAGER_VERSION 1
#define LOUVRE_RELATIVE_POINTER_MANAGER_VERSION 1
#define LOUVRE_POINTER_GESTURES_VERSION 3
#define LOUVRE_SESSION_LOCK_MANAGER_VERSION 1
#define LOUVRE_POINTER_CONSTRAINTS_VERSION 1
#define LOUVRE_SCREEN_COPY_MANAGER_VERSION 3
#define LOUVRE_LAYER_SHELL_VERSION 5

#define L_UNUSED(object){(void)object;}

#define CAT(x, y) CAT_(x, y)
#define CAT_(x, y) x ## y

#define LPRIVATE_CLASS(class_name) \
    class class_name::CAT(class_name,Private){ \
        public: \
        CAT(class_name,Private)() = default; \
        ~CAT(class_name,Private)() = default; \
        CAT(class_name,Private)(const CAT(class_name,Private)&) = delete; \
        CAT(class_name,Private) &operator=(const CAT(class_name,Private)&) = delete;

#define LPRIVATE_CLASS_NO_COPY(class_name) \
    class class_name::CAT(class_name,Private){ \
        public: \
        CAT(class_name,Private)(const CAT(class_name,Private)&) = delete; \
        CAT(class_name,Private) &operator=(const CAT(class_name,Private)&) = delete;

#define LPRIVATE_IMP(class_name) \
    class CAT(class_name,Private); \
    inline CAT(class_name,Private) *imp() const {return m_imp;}; \
    private: \
    CAT(class_name,Private) *m_imp = nullptr;

#define LPRIVATE_IMP_UNIQUE(class_name) \
    class CAT(class_name,Private); \
    inline CAT(class_name,Private) *imp() const {return m_imp.get();}; \
    private: \
        friend class std::unique_ptr<CAT(class_name,Private)>;\
        std::unique_ptr<CAT(class_name,Private)> m_imp;

#define LPRIVATE_INIT_UNIQUE(class_name) \
    m_imp(std::make_unique<CAT(class_name,Private)>())

#define LCLASS_NO_COPY(class_name) \
    class_name(const class_name&) = delete; \
    class_name &operator=(const class_name&) = delete;

/**
 * @brief Namespaces
 * @page Namespaces
 */
namespace Louvre
{
    // API Classes
    class LObject;
    class LGlobal;
    class LAnimation;
    class LGraphicBackend;
    class LInputBackend;
    class LClient;
    class LCompositor;
    class LOutput;
    class LOutputManager;
    class LOutputMode;
    class LOpenGL;
    class LPositioner;
    class LRegion;
    class LResource;
    class LSessionLockManager;
    class LSurface;
    class LTexture;
    class LScreenshotRequest;

    // Painter
    class LPainter;
    class LRenderBuffer;
    class LFramebuffer;
    class LOutputFramebuffer;
    class LGLFramebuffer;

    // Views
    class LScene;
    class LView;
    class LLayerView;
    class LSurfaceView;
    class LTextureView;
    class LSolidColorView;
    class LSceneView;
    class LSceneTouchPoint;

    // Data
    class LDND;
    class LDNDSession;
    class LClipboard;

    // Surface roles
    class LBaseSurfaceRole;
    class LCursorRole;
    class LDNDIconRole;
    class LToplevelRole;
    class LPopupRole;
    class LSubsurfaceRole;
    class LSessionLockRole;

    // Input related
    class LSeat;
    class LPointer;
    class LKeyboard;
    class LTouch;
    class LCursor;
    class LClientCursor;
    class LXCursor;
    class LInputDevice;
    class LEvent;
    class LInputEvent;

    class LPointerEvent;
    class LPointerEnterEvent;
    class LPointerLeaveEvent;
    class LPointerMoveEvent;
    class LPointerButtonEvent;
    class LPointerScrollEvent;

    class LPointerSwipeBeginEvent;
    class LPointerSwipeUpdateEvent;
    class LPointerSwipeEndEvent;

    class LPointerPinchBeginEvent;
    class LPointerPinchUpdateEvent;
    class LPointerPinchEndEvent;

    class LPointerHoldBeginEvent;
    class LPointerHoldEndEvent;

    class LKeyboardEvent;
    class LKeyboardEnterEvent;
    class LKeyboardLeaveEvent;
    class LKeyboardKeyEvent;
    class LKeyboardModifiersEvent;

    class LTouchEvent;
    class LTouchDownEvent;
    class LTouchMoveEvent;
    class LTouchUpEvent;
    class LTouchFrameEvent;
    class LTouchCancelEvent;
    class LTouchPoint;

    class LToplevelResizeSession;
    class LToplevelMoveSession;

    // Other
    class LDMABuffer;

    // Utils
    class LLog;
    class LTime;
    class LTimer;
    class LLauncher;
    class LGammaTable;
    class LWeakUtils;
    template <class T> class LWeak;
    template <class TA, class TB> class LPointTemplate;
    template <class TA, class TB> class LRectTemplate;
    template <class T> class LBitset;

    // Types
    /// @brief 64 bits unsigned integer
    typedef uint64_t        UInt64;

    /// @brief 64 bits signed integer
    typedef int64_t         Int64;

    /// @brief 32 bits unsigned integer
    typedef uint32_t        UInt32;

    /// @brief 32 bits signed integer
    typedef int32_t         Int32;

    /// @brief 16 bits unsigned integer
    typedef uint16_t        UInt16;

    /// @brief 16 bits signed integer
    typedef int16_t         Int16;

    /// @brief 8 bits unsigned integer
    typedef uint8_t         UInt8;

    /// @brief 8 bits signed integer
    typedef int8_t          Int8;

    /// @brief 8 bits unsigned integer
    typedef unsigned char   UChar8;

    /// @brief 8 bits signed integer
    typedef char            Char8;

    /// @brief 64 bits float
    typedef double          Float64;

    /// @brief 32 bits float
    typedef float           Float32;

    /// @brief 24 bits Wayland float
    typedef wl_fixed_t      Float24;

    /// @brief Unsigned integer capable of holding a pointer
    typedef uintptr_t       UIntPtr;

    /// 2D vector of 32 bits integers
    using LPoint = LPointTemplate<Int32,Float32>;

    /// 2D vector of 32 bits integers
    using LSize = LPoint;

    /// 2D vector of 32 bits floats
    using LPointF = LPointTemplate<Float32,Int32>;

    /// 2D vector of 32 bits floats
    using LSizeF = LPointF;

    /// 4D vector of 32 bits integers
    using LRect = LRectTemplate<Int32,Float32>;

    /// 4D vector of 32 bits floats
    using LRectF = LRectTemplate<Float32,Int32>;

    typedef void* EGLContext;
    typedef void* EGLDisplay;
    typedef unsigned int GLuint;
    typedef unsigned int GLenum;

    /**
     * @brief Structure representing a 2D box.
     *
     * The LBox struct defines a 2D box using four integer coordinates (x1, y1, x2, y2).
     * It is typically used to represent bounding boxes or rectangular regions in 2D space.
     */
    struct LBox
    {
        /// The x-coordinate of the top-left corner of the box.
        Int32 x1;

        /// The y-coordinate of the top-left corner of the box.
        Int32 y1;

        /// The x-coordinate of the bottom-right corner of the box.
        Int32 x2;

        /// The y-coordinate of the bottom-right corner of the box.
        Int32 y2;
    };

    /**
     * @brief Structure representing DMA format and modifier.
     *
     * The LDMAFormat struct contains information about DMA format and modifier.
     * It is used to describe the format and memory layout of DMA planes used for texture generation.
     * Each LDMAFormat instance includes a format value and a modifier value.
     */
    struct LDMAFormat
    {
        /// The format of the DMA plane.
        UInt32 format;

        /// The modifier value specifying the memory layout.
        UInt64 modifier;
    };

    /**
     * @brief Direct Memory Access (DMA) planes.
     *
     * Use this struct to import DMA buffers with LTexture.
     */
    struct LDMAPlanes
    {
        /// Width of the buffer in pixels.
        UInt32 width;

        /// Height of the buffer in pixels.
        UInt32 height;

        /// DRM format of the buffer.
        UInt32 format;

        /// Number of file descriptors.
        UInt32 num_fds = 0;

        /// Array of file descriptors associated with each DMA plane.
        Int32 fds[LOUVRE_MAX_DMA_PLANES] = {-1};

        /// Array of strides for each DMA plane.
        UInt32 strides[LOUVRE_MAX_DMA_PLANES] = {0};

        /// Array of offsets for each DMA plane.
        UInt32 offsets[LOUVRE_MAX_DMA_PLANES] = {0};

        /// Array of modifiers for each DMA plane.
        UInt64 modifiers[LOUVRE_MAX_DMA_PLANES] = {0};
    };

    /**
     * @brief RGB color with floating-point components.
     *
     * The LRGBF struct defines an RGB color with three floating-point components (r, g, b).\n
     * Each component ranges from 0.0 to 1.0.
     */
    struct LRGBF
    {
        /// The red component of the RGB color (range: 0.0 to 1.0).
        Float32 r;

        /// The green component of the RGB color (range: 0.0 to 1.0).
        Float32 g;

        /// The blue component of the RGB color (range: 0.0 to 1.0).
        Float32 b;
    };

    /**
     * @brief RGBA color with floating-point components.
     *
     * The LRGBAF struct defines an RGBA color with four floating-point components (r, g, b, a).\n
     * Each component ranges from 0.0 to 1.0.
     */
    struct LRGBAF
    {
        /// The red component of the RGBA color (range: 0.0 to 1.0).
        Float32 r;

        /// The green component of the RGBA color (range: 0.0 to 1.0).
        Float32 g;

        /// The blue component of the RGBA color (range: 0.0 to 1.0).
        Float32 b;

        /// The alpha component of the RGBA color (range: 0.0 to 1.0).
        Float32 a;
    };

    /**
     * @brief Alpha blending function
     *
     * OpenGL blend function. Refer to the documentation
     * of [glBlendFuncSeparate()](https://docs.gl/es2/glBlendFuncSeparate) for more information.
     */
    struct LBlendFunc
    {
        /// Source RGB factor for blending
        GLenum sRGBFactor;

        /// Destination RGB factor for blending
        GLenum dRGBFactor;

        /// Source alpha factor for blending
        GLenum sAlphaFactor;

        /// Destination alpha factor for blendin
        GLenum dAlphaFactor;
    };

    /**
     * @brief ID values for the graphic backends shipped with Louvre.
     *
     * These IDs are returned by LSeat::graphicBackendId() to identify
     * the currently loaded backend.
     *
     * The range [0, 1000] is reserved for Louvre graphic backends only. If you wish to create
     * your custom graphic backends, use an ID out of that range.
     *
     * @enum LGraphicBackendID
     */
    enum LGraphicBackendID : UInt32
    {
        LGraphicBackendDRM = 0,     ///< ID for the DRM graphic backend.
        LGraphicBackendX11 = 1      ///< ID for the X11 graphic backend.
    };

    /**
     * @brief ID values for the input backends included with Louvre.
     *
     * These IDs are returned by LSeat::inputBackendId() to identify the currently
     * loaded input backend.
     *
     * The range [0, 1000] is reserved exclusively for Louvre's built-in input backends. If
     * you intend to create custom input backends, please use an ID outside of this range.
     *
     * @enum LInputBackendID
     */
    enum LInputBackendID : UInt32
    {
        LInputBackendLibinput = 0, ///< ID for the Libinput input backend.
        LInputBackendX11 = 1       ///< ID for the X11 input backend.
    };

    /**
     * @brief Structure representing a version in the format major.minor.patch-build.
     */
    struct LVersion
    {
        UInt32 major; ///< Major version.
        UInt32 minor; ///< Minor version.
        UInt32 patch; ///< Patch version.
        UInt32 build; ///< Build number.
    };

    /// @cond OMIT

    namespace Protocols
    {
        namespace Wayland
        {
            class GCompositor;
            class GSubcompositor;
            class GSeat;
            class GDataDeviceManager;
            class GOutput;

            class RSurface;
            class RRegion;
            class RPointer;
            class RKeyboard;
            class RTouch;
            class RDataDevice;
            class RDataSource;
            class RDataOffer;
            class RSubsurface;
            class RCallback;
        }

        namespace XdgShell
        {
            class GXdgWmBase;

            class RXdgPositioner;
            class RXdgSurface;
            class RXdgToplevel;
            class RXdgPopup;
        };

        namespace XdgDecoration
        {
            class GXdgDecorationManager;

            class RXdgToplevelDecoration;
        };

        namespace XdgOutput
        {
            class GXdgOutputManager;

            class RXdgOutput;
        }

        namespace PresentationTime
        {
            class GPresentation;

            class RPresentationFeedback;
        };

        namespace LinuxDMABuf
        {
            class GLinuxDMABuf;

            class RLinuxBufferParams;
            class RLinuxDMABufFeedback;
        };

        namespace Viewporter
        {
            class GViewporter;

            class RViewport;
        };

        namespace FractionalScale
        {
            class GFractionalScaleManager;

            class RFractionalScale;
        };

        namespace GammaControl
        {
            class GGammaControlManager;

            class RGammaControl;
        };

        namespace TearingControl
        {
            class GTearingControlManager;

            class RTearingControl;
        };

        namespace PointerGestures
        {
            class GPointerGestures;

            class RGestureSwipe;
            class RGesturePinch;
            class RGestureHold;
        };

        namespace RelativePointer
        {
            class GRelativePointerManager;

            class RRelativePointer;
        };

        namespace SessionLock
        {
            class GSessionLockManager;

            class RSessionLock;
            class RSessionLockSurface;
        };

        namespace ScreenCopy
        {
            class GScreenCopyManager;

            class RScreenCopyFrame;
        };

        namespace PointerConstraints
        {
            class GPointerConstraints;

            class RLockedPointer;
            class RConfinedPointer;
        };

        namespace LayerShell
        {
            class GLayerShell;

            class RLayerSurface;
        }
    }

    /// @endcond

    /**
     * @brief Get the static LCompositor instance.
     *
     * This method provides access to the single LCompositor instance that can exist per process.\n
     * You can also access it from any Louvre object with `object->compositor()`.
     *
     * @return A pointer to the LCompositor instance.
     */
    LCompositor *compositor() noexcept;

    /**
     * @brief Gets the compositor cursor.
     *
     * This method must be accessed within or after the initialized() or cursorInitialized() events.
     * If the cursor has not yet been initialized, this method returns `nullptr`.
     *
     * @return A pointer to the LCursor instance or `nullptr` if not yet initialized.
     */
    LCursor *cursor() noexcept;

    /**
     * @brief Gets the compositor seat.
     *
     * The seat provides access to the LClipboard, LDND, LPointer, LKeyboard, LTouch, and LOutput instances.
     *
     * @return A pointer to the LSeat instance.
     */
    LSeat *seat() noexcept;

    // TODO
    LSessionLockManager *sessionLockManager() noexcept;
};

#endif // LNAMESPACES_H
