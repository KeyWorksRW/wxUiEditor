/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxBitmapBundle implementation that rasterizes SVG with NanoSVG
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <string>
#include <vector>

#include <wx/rawbmp.h>

#include "nano_svg_bundle.h"

// wxWidgets may already have pulled in the NanoSVG headers (wxUSE_NANOSVG). Undefining the include
// guards lets this translation unit compile its own copy of the NanoSVG implementation.
#undef NANOSVG_H
#undef NANOSVGRAST_H

#define NANOSVG_CPLUSPLUS
#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION

// NanoSVG is third-party C code compiled with the project's warning flags. Its own declarations
// trip -Wshadow (and MSVC C4456), which is not something we can or should fix here.
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wshadow"
#elif defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable : 4456)
#endif

#include <nanosvg.h>
#include <nanosvgrast.h>

#if defined(__clang__)
    #pragma clang diagnostic pop
#elif defined(__GNUC__)
    #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
    #pragma warning(pop)
#endif

namespace
{
    class NanoBitmapBundleImpl : public wxBitmapBundleImpl
    {
    public:
        NanoBitmapBundleImpl(NSVGimage* svg_image, const wxSize& default_size) :
            m_svg_image(svg_image),
            m_svg_rasterizer(nsvgCreateRasterizer()),
            m_default_size(default_size)
        {
        }

        ~NanoBitmapBundleImpl() override
        {
            nsvgDeleteRasterizer(m_svg_rasterizer);
            nsvgDelete(m_svg_image);
        }

        wxSize GetDefaultSize() const override { return m_default_size; }

        wxSize GetPreferredBitmapSizeAtScale(double scale) const override
        {
            // We just scale the bitmap to fit the requested size, so
            // we don't really have any preferences.
            return m_default_size * scale;
        }

        wxBitmap GetBitmap(const wxSize& size) override
        {
            if (!m_cached_bitmap.IsOk() || m_cached_bitmap.GetSize() != size)
            {
                m_cached_bitmap = DoRasterize(size);
                ASSERT(m_cached_bitmap.IsOk());
            }

            return m_cached_bitmap;
        }

    private:
        wxBitmap DoRasterize(const wxSize& size)
        {
            std::vector<unsigned char> buffer(static_cast<size_t>(size.x) * size.y * 4);
            const float scale = std::min(static_cast<float>(size.x) / m_svg_image->width,
                                         static_cast<float>(size.y) / m_svg_image->height);
            nsvgRasterize(m_svg_rasterizer, m_svg_image, 0.0F, 0.0F,  // no offset
                          scale, buffer.data(), size.x, size.y,
                          size.x * 4  // stride -- we have no gaps between lines
            );

            wxBitmap bitmap(size, 32);
            wxAlphaPixelData bmpdata(bitmap);
            wxAlphaPixelData::Iterator dst(bmpdata);

            const unsigned char* src = buffer.data();
            for (int y = 0; y < size.y; ++y)
            {
                dst.MoveTo(bmpdata, 0, y);
                for (int x = 0; x < size.x; ++x)
                {
                    const unsigned char alpha = src[3];
                    dst.Red() = src[0] * alpha / 255;
                    dst.Green() = src[1] * alpha / 255;
                    dst.Blue() = src[2] * alpha / 255;
                    dst.Alpha() = alpha;
                    ++dst;
                    src += 4;
                }
            }

            return bitmap;
        }

        NSVGimage* const m_svg_image;
        NSVGrasterizer* const m_svg_rasterizer;
        const wxSize m_default_size;
        wxBitmap m_cached_bitmap;
    };
}  // namespace

wxBitmapBundle CreateNanoSVGBundle(const char* data, size_t size, const wxSize& default_size)
{
    // nsvgParse() modifies the string it parses and needs it to be NUL terminated, so parse a copy.
    std::string copy(data, size);
    NSVGimage* const svg_image = nsvgParse(copy.data(), "px", 96);
    if (!svg_image)
    {
        return wxBitmapBundle();
    }

    // Somewhat unexpectedly, a non-null but empty image is returned even if the data is not SVG at
    // all, e.g. without this check creating a bundle from any random file would "work".
    if (svg_image->width == 0 && svg_image->height == 0 && !svg_image->shapes)
    {
        nsvgDelete(svg_image);
        return wxBitmapBundle();
    }

    return wxBitmapBundle::FromImpl(new NanoBitmapBundleImpl(svg_image, default_size));
}
