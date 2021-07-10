/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2021 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/core/core.hpp>
#include <mango/image/image.hpp>

using namespace mango;
using namespace mango::image;

struct Test
{
    Format dest;
    Format source;
};

const Test tests [] =
{
    { Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 8), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8) },
    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 0), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8) },
    { Format(24, Format::UNORM, Format::RGB, 8, 8, 8, 0), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8) },
    { Format(24, Format::UNORM, Format::BGR, 8, 8, 8, 0), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8) },

    { Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 8), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 0) },
    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 0), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 0) },
    { Format(24, Format::UNORM, Format::RGB, 8, 8, 8, 0), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 0) },
    { Format(24, Format::UNORM, Format::BGR, 8, 8, 8, 0), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 0) },

    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8), Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 8) },
    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 0), Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 8) },
    { Format(24, Format::UNORM, Format::RGB, 8, 8, 8, 0), Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 8) },
    { Format(24, Format::UNORM, Format::BGR, 8, 8, 8, 0), Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 8) },

    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8), Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 0) },
    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 0), Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 0) },
    { Format(24, Format::UNORM, Format::RGB, 8, 8, 8, 0), Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 0) },
    { Format(24, Format::UNORM, Format::BGR, 8, 8, 8, 0), Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 0) },

    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8), Format(24, Format::UNORM, Format::RGB, 8, 8, 8, 0) },
    { Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 8), Format(24, Format::UNORM, Format::RGB, 8, 8, 8, 0) },
    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 0), Format(24, Format::UNORM, Format::RGB, 8, 8, 8, 0) },
    { Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 0), Format(24, Format::UNORM, Format::RGB, 8, 8, 8, 0) },
    { Format(24, Format::UNORM, Format::BGR, 8, 8, 8, 0), Format(24, Format::UNORM, Format::RGB, 8, 8, 8, 0) },

    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8), Format(24, Format::UNORM, Format::BGR, 8, 8, 8, 0) },
    { Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 8), Format(24, Format::UNORM, Format::BGR, 8, 8, 8, 0) },
    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 0), Format(24, Format::UNORM, Format::BGR, 8, 8, 8, 0) },
    { Format(32, Format::UNORM, Format::BGRA, 8, 8, 8, 0), Format(24, Format::UNORM, Format::BGR, 8, 8, 8, 0) },
    { Format(24, Format::UNORM, Format::RGB, 8, 8, 8, 0), Format(24, Format::UNORM, Format::BGR, 8, 8, 8, 0) },

    { Format(8, Format::UNORM, Format::RGBA, 2, 2, 2, 2), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8) },
    { Format(16, Format::UNORM, Format::RGBA, 4, 4, 4, 4), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8) },
    { Format(32, Format::UNORM, Format::RGBA, 6, 6, 6, 8), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8) },
    { Format(8, Format::UNORM, Format::R, 8, 0, 0, 0), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8) },

    { Format(128, Format::FLOAT32, Format::RGBA, 32, 32, 32, 32), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8) },
    { Format(64, Format::FLOAT16, Format::RGBA, 16, 16, 16, 16), Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8) },

    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8), Format(128, Format::FLOAT32, Format::RGBA, 32, 32, 32, 32) },
    { Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8), Format(64, Format::FLOAT16, Format::RGBA, 16, 16, 16, 16) },

    // ...
};

void test(Surface dest, Surface source)
{
    dest.clear(0xff00ff00);

    int width = dest.width;
    int height = dest.height;

    constexpr int s = 337;
    int x = 0;
    int y = 0;

    for (int i = 0; i < sizeof(tests) / sizeof(Test); ++i)
    {
        Bitmap a(width, height, tests[i].source);
        Bitmap b(width, height, tests[i].dest);

        a.blit(x, y, Surface(source, x, y, s, s));
        b.blit(x, y, Surface(a, x, y, s, s));
        dest.blit(x, y, Surface(b, x, y, s, s));

        x += s;
        if (x >= width)
        {
            x = 0;
            y += s;
        }
    }
}

struct Component
{
    int size;
    int offset;
    int channel;
};

static
bool sort_component(Component a, Component b)
{
    return a.offset < b.offset;
}

void print(const Format& format)
{
    Component component[4];

    for (int i = 0; i < 4; ++i)
    {
        component[i].size = format.size[i];
        component[i].offset = format.offset[i];
        component[i].channel = i;
    }

    std::sort(component + 0, component + 3, sort_component);

    const char* name[] =
    {
        "R", "G", "B", "A"
    };

    printf("[%3d ", format.bits);

    int count = 0;

    for (int i = 0; i < 4; ++i)
    {
        Component c = component[i];
        if (c.size)
        {
            printf("%s", name[c.channel]);
            ++count;
        }
    }

    for ( ; count < 4; ++count)
    {
        printf(" ");
    }

    count = 0;

    for (int i = 0; i < 4; ++i)
    {
        Component c = component[i];
        if (c.size)
        {
            printf(" %3d", c.size);
            ++count;
        }
    }

    for ( ; count < 4; ++count)
    {
        printf("    ");
    }

    printf(" ]");
}

void profile(Surface surface)
{
    int width = surface.width;
    int height = surface.height;

    for (int i = 0; i < sizeof(tests) / sizeof(Test); ++i)
    {
        Bitmap a(surface, tests[i].source);
        Bitmap b(width, height, tests[i].dest);

        u64 time0 = Time::us();

        b.blit(0, 0, a);
        u64 time1 = Time::us();

        u64 s = width * height / ((time1 - time0) * 1);
        printf("  %6d us (%4d MP/s)  ", int(time1 - time0), int(s));
        print(b.format);
        printf("   <--   ");
        print(a.format);
        printf("\n");
    }
}

int main()
{
    Bitmap bitmap("conquer.jpg", Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8));
    printf("Image: %d x %d\n", bitmap.width, bitmap.height);

    profile(bitmap);

    Bitmap target(bitmap.width, bitmap.height, bitmap.format);
    test(target, bitmap);

    target.save("result.png");
}
