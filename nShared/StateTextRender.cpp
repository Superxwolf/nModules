/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  StateTextRender.cpp
 *  The nModules Project
 *
 *  Renders a state's text.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "StateTextRender.hpp"
#include "Factories.h"


/// <summary>
/// Constructor
/// </summary>
StateTextRender::StateTextRender(State *state)
    : mRefCount(0)
    , mState(state)
{
}


/// <summary>
/// Destructor
/// </summary>
StateTextRender::~StateTextRender()
{
}


/// <summary>
/// IUnknown::AddRef
/// Increments the reference count for an interface on an object.
/// </summary>
ULONG StateTextRender::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}


/// <summary>
/// IUnknown::Release
/// Decrements the reference count for an interface on an object.
/// </summary>
ULONG StateTextRender::Release()
{
    if (InterlockedDecrement(&mRefCount) == 0)
    {
        delete this;
        return 0;
    }

    return mRefCount;
}


/// <summary>
/// IUnknown::QueryInterface
/// Retrieves pointers to the supported interfaces on an object.
/// </summary>
HRESULT StateTextRender::QueryInterface(REFIID riid, void **ppvObject)
{
    if (ppvObject == nullptr)
    {
        return E_POINTER;
    }

    if (riid == __uuidof(IDWriteTextRenderer))
    {
        *ppvObject = this;
    }
    else if (riid == __uuidof(IDWritePixelSnapping))
    {
        *ppvObject = (IDWritePixelSnapping*)this;
    }
    else if (riid == IID_IUnknown)
    {
        *ppvObject = (IUnknown*)this;
    }
    else
    {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


/// <summary>
/// IDWritePixelSnapping::GetCurrentTransform
/// Returns the current transform applied to the render target.
/// </summary>
HRESULT StateTextRender::GetCurrentTransform(
    LPVOID clientDrawingContext,
    DWRITE_MATRIX * transform)
{
    if (transform == nullptr)
    {
        return E_POINTER;
    }

    ((ID2D1RenderTarget*)clientDrawingContext)->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));

    return S_OK;
}


/// <summary>
/// IDWritePixelSnapping::GetPixelsPerDip
/// This returns the number of pixels per DIP.
/// </summary>
HRESULT StateTextRender::GetPixelsPerDip(
    LPVOID clientDrawingContext,
    PFLOAT pixelsPerDip)
{
    if (pixelsPerDip == nullptr)
    {
        return E_POINTER;
    }

    FLOAT x, y;

    ((ID2D1RenderTarget*)clientDrawingContext)->GetDpi(&x, &y);

    *pixelsPerDip = x / 96.0f;

    return S_OK;
}


/// <summary>
/// IDWritePixelSnapping::IsPixelSnappingDisabled
/// Determines whether pixel snapping is disabled.
/// </summary>
HRESULT StateTextRender::IsPixelSnappingDisabled(
    LPVOID clientDrawingContext,
    LPBOOL isDisabled)
{
    UNREFERENCED_PARAMETER(clientDrawingContext);

    if (isDisabled == nullptr)
    {
        return E_POINTER;
    }

    *isDisabled = FALSE;
    return S_OK;
}


/// <summary>
/// IDWriteTextRenderer::DrawGlyphRun
/// 
/// </summary>
HRESULT StateTextRender::DrawGlyphRun(
        LPVOID clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE  measuringMode,
        const DWRITE_GLYPH_RUN * glyphRun,
        const DWRITE_GLYPH_RUN_DESCRIPTION * glyphRunDescription,
        LPUNKNOWN clientDrawingEffect)
{
    HRESULT hr = S_OK;

    // Get our D2D factory
    ID2D1Factory *d2dFactory;
    ((ID2D1RenderTarget*)clientDrawingContext)->GetFactory(&d2dFactory);

    // Create the path geometry.
    ID2D1PathGeometry *pPathGeometry = nullptr;
    hr = d2dFactory->CreatePathGeometry(&pPathGeometry);

    // Write to the path geometry using the geometry sink.
    ID2D1GeometrySink* sink = nullptr;
    if (SUCCEEDED(hr))
    {
        hr = pPathGeometry->Open(&sink);
    }

    // Get the glyph run outline geometries back from DirectWrite and place them within the geometry sink.
    if (SUCCEEDED(hr))
    {
        hr = glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize,
            glyphRun->glyphIndices,
            glyphRun->glyphAdvances,
            glyphRun->glyphOffsets,
            glyphRun->glyphCount,
            glyphRun->isSideways,
            glyphRun->bidiLevel%2,
            sink
        );
    }

    // Close the geometry sink
    if (SUCCEEDED(hr))
    {
        hr = sink->Close();
    }

    // Initialize a matrix to translate the origin of the glyph run.
    D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
        1.0f, 0.0f,
        0.0f, 1.0f,
        baselineOriginX, baselineOriginY
        );

    // Create the transformed geometry
    ID2D1TransformedGeometry* pTransformedGeometry = nullptr;
    if (SUCCEEDED(hr))
    {
        hr = d2dFactory->CreateTransformedGeometry(
            pPathGeometry,
            &matrix,
            &pTransformedGeometry
            );
    }

    if (SUCCEEDED(hr))
    {
        auto renderTarget = ((ID2D1RenderTarget*) clientDrawingContext);

        //renderTarget->PushLayer(D2D1)

        float strokeWidth = mState->mStateSettings.textStrokeWidth;
        if (strokeWidth != 0.0f)
        {
            // Draw the outline of the glyph run
            renderTarget->DrawGeometry(
                pTransformedGeometry,
                mState->mBrushes[State::BrushType::TextStroke].brush,
                strokeWidth
                );
        }

        // Fill in the glyph run
        /*renderTarget->FillGeometry(
            pTransformedGeometry,
            mState->mBrushes[State::BrushType::Text].brush
            );*/
    }

    d2dFactory->Release();
    SAFERELEASE(pPathGeometry);
    SAFERELEASE(sink);
    SAFERELEASE(pTransformedGeometry);

    return hr;
}


/// <summary>
/// IDWriteTextRenderer::DrawInlineObject
/// 
/// </summary>
HRESULT StateTextRender::DrawInlineObject(
        LPVOID clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject * inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        LPUNKNOWN clientDrawingEffect)
{
    return E_NOTIMPL;
}


/// <summary>
/// IDWriteTextRenderer::DrawStrikethrough
/// 
/// </summary>
HRESULT StateTextRender::DrawStrikethrough(
        LPVOID clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        const DWRITE_STRIKETHROUGH * strikethrough,
        LPUNKNOWN clientDrawingEffect)
{
    return E_NOTIMPL;
}


/// <summary>
/// IDWriteTextRenderer::DrawUnderline
/// 
/// </summary>
HRESULT StateTextRender::DrawUnderline(
        LPVOID clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        const DWRITE_UNDERLINE * underline,
        LPUNKNOWN clientDrawingEffect)
{
    return E_NOTIMPL;
}
