#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GLSL(src) "#version 150\n" #src

char *vs = GLSL(
in vec2 position;
out vec2 texel;
void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    texel = vec2(0.5) + 0.5 * position;
}
);

char *fs = GLSL(
in vec2 texel;
uniform sampler2D source;
out vec4 color;
void main()
{
    color = texture(source, texel);
}
);

#define SRC_X 256
#define SRC_Y 256
#define DST_X (SRC_X/4)
#define DST_Y (SRC_Y/4)
#define PALETTE_VALUES 7
#define PALETTE_COUNT 2
u08 src[SRC_X*SRC_Y];
u32 tex0;
u32 tex1;
RenderPass pass;

struct Palette
{
    u32 values[PALETTE_VALUES];
};

Palette palettes[PALETTE_COUNT] = {
//    RESERVED  1         2         3         4         5          CURSOR
    { 0x000000, 0x122D43, 0x424A5F, 0x9A848E, 0xEEBFB6, 0xFFFFE7, 0xFF3300 },
    { 0x000000, 0x681922, 0xA5837C, 0xEBC3BC, 0xFFFDF8, 0xFFFFFF, 0xFF3300 }
};

void PUT(s32 x, s32 y, int value, int palette)
{
    value &= 0x7;
    palette &= 0x1F;
    src[y*SRC_X+x] = (value) | (palette << 3);
}

void GET(s32 x, s32 y, int *value, int *palette)
{
    u08 b = src[y*SRC_X+x];
    if (value)
        *value = b & 0x7;
    if (palette)
        *palette = (b >> 3) & 0x1F;
}

void GET(s32 x, s32 y, u08 *r, u08 *g, u08 *b)
{
    int value, palette;
    GET(x, y, &value, &palette);
    u32 color = palettes[palette].values[value];
    *r = (color >> 16) & 0xFF;
    *g = (color >> 8) & 0xFF;
    *b = (color) & 0xFF;
}

#define BRUSH_X 8
#define BRUSH_Y 8
#define BRUSH_COUNT 5

int brush00[BRUSH_X*BRUSH_Y] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

int brush01[BRUSH_X*BRUSH_Y] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

int brush02[BRUSH_X*BRUSH_Y] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

int brush03[BRUSH_X*BRUSH_Y] = {
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
};

int brush04[BRUSH_X*BRUSH_Y] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

int *brushes[] = {
    brush00, brush01,
    brush02, brush03,
    brush04
};

struct Brush
{
    int value;
    int palette;
    int type;

    int mirror_x_on;
    int mirror_y_on;
    s32 mirror_x;
    s32 mirror_y;
} brush;

void init()
{
    brush.value = 0;
    brush.palette = 0;
    brush.type = 0;
    pass = make_render_pass(vs, fs);
    glGenTextures(1, &tex0);
    glGenTextures(1, &tex1);
}

void clamp(int *x, int min, int max)
{
    if (*x < min) *x = min;
    if (*x > max) *x = max;
}

void PAINT(s32 center_x, s32 center_y, Brush brush)
{
    for (s32 y_brush = 0; y_brush < BRUSH_Y; y_brush++)
    for (s32 x_brush = 0; x_brush < BRUSH_X; x_brush++)
    {
        s32 x_src = center_x - 3 + x_brush;
        s32 y_src = center_y - 3 + y_brush;
        clamp(&x_src, 0, SRC_X-1);
        clamp(&y_src, 0, SRC_Y-1);
        s32 mask = brushes[brush.type][y_brush*BRUSH_X+x_brush];
        if (mask)
        {
            PUT(x_src, y_src, brush.value, brush.palette);
        }
    }
}

void tick(Input io, float t, float dt)
{
    float mouse_xf = (0.5f + 0.5f * io.mouse.ndc.x);
    float mouse_yf = (0.5f + 0.5f * io.mouse.ndc.y);
    s32 mouse_x_src = mouse_xf * SRC_X;
    s32 mouse_y_src = mouse_yf * SRC_Y;

    if (io.mouse.left.down)
    {
        PAINT(mouse_x_src, mouse_y_src, brush);
        if (brush.mirror_x_on && brush.mirror_y_on)
        {
            s32 dx = brush.mirror_x - mouse_x_src;
            s32 dy = brush.mirror_y - mouse_y_src;
            PAINT(brush.mirror_x + dx, mouse_y_src, brush);
            PAINT(brush.mirror_x + dx, brush.mirror_y + dy, brush);
            PAINT(mouse_x_src, brush.mirror_y + dy, brush);
        }
        else if (brush.mirror_x_on)
        {
            s32 dx = brush.mirror_x - mouse_x_src;
            PAINT(brush.mirror_x + dx, mouse_y_src, brush);
        }
        else if (brush.mirror_y_on)
        {
            s32 dy = brush.mirror_y - mouse_y_src;
            PAINT(mouse_x_src, brush.mirror_y + dy, brush);
        }
    }

    if (io_key_down(1)) brush.value = 0;
    if (io_key_down(2)) brush.value = 1;
    if (io_key_down(3)) brush.value = 2;
    if (io_key_down(4)) brush.value = 3;
    if (io_key_down(5)) brush.value = 4;
    if (io_key_down(6)) brush.value = 5;

    if (io_key_released(A) && brush.palette > 0) brush.palette--;
    if (io_key_released(S) && brush.palette < PALETTE_COUNT-1) brush.palette++;

    if (io_key_released(C))
    {
        for (s32 i = 0; i < SRC_X*SRC_Y; i++)
            src[i] = 0;
    }

    if (io_key_released(W) && brush.type < BRUSH_COUNT-1) brush.type++;
    if (io_key_released(Q) && brush.type > 0) brush.type--;

    if (io_key_released(X))
    {
        if (brush.mirror_x_on)
        {
            brush.mirror_x_on = 0;
        }
        else
        {
            brush.mirror_x_on = 1;
            brush.mirror_x = mouse_x_src;
        }
    }

    if (io_key_released(Y))
    {
        if (brush.mirror_y_on)
        {
            brush.mirror_y_on = 0;
        }
        else
        {
            brush.mirror_y_on = 1;
            brush.mirror_y = mouse_y_src;
        }
    }

    struct Pixel
    {
        u08 r, g, b;
    };
    u32 transparent[] = {
        0x000000,
        0x000000
    };
    Pixel data[DST_X*DST_Y];
    Pixel *data_ptr = data;
    for (s32 y = 0; y < DST_Y; y++)
    for (s32 x = 0; x < DST_X; x++)
    {
        int sums[PALETTE_COUNT];
        int counts[PALETTE_COUNT];
        for (int i = 0; i < PALETTE_COUNT; i++)
        {
            sums[i] = 0;
            counts[i] = 0;
        }
        for (s32 y_off = 0; y_off < 4; y_off++)
        for (s32 x_off = 0; x_off < 4; x_off++)
        {
            s32 x_src = x*4 + x_off;
            s32 y_src = y*4 + y_off;
            int value, palette;
            GET(x_src, y_src, &value, &palette);
            if (value != 0)
            {
                sums[palette] += value;
                counts[palette]++;
            }
            else
            {
                for (int i = 0; i < PALETTE_COUNT; i++)
                    counts[i]++;
            }
        }

        int dominant_i = 0;
        int dominant_c = counts[0];
        for (int i = 0; i < PALETTE_COUNT; i++)
        {
            if (counts[i] > dominant_c)
            {
                dominant_c = counts[i];
                dominant_i = i;
            }
        }

        float average = sums[dominant_i] / (float)counts[dominant_i];
        int rounded = (int)(average + 0.5f);

        u32 color = palettes[dominant_i].values[rounded];
        if (color == 0)
        {
            int i = ((x % 2) + (y % 2)) % 2;
            color = transparent[i];
        }
        data_ptr->r = (color >> 16) & 0xFF;
        data_ptr->g = (color >> 8) & 0xFF;
        data_ptr->b = (color) & 0xFF;
        data_ptr++;
    }
    glBindTexture(GL_TEXTURE_2D, tex0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, DST_X, DST_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    persist int mode = 0;
    if (io_key_released(SPACE)) mode = (mode + 1) % 2;

    if (mode == 1)
    {
        Pixel tex1_data[SRC_X*SRC_Y];
        Pixel *tex1_data_ptr = tex1_data;
        for (s32 y_src = 0; y_src < SRC_Y; y_src++)
        for (s32 x_src = 0; x_src < SRC_X; x_src++)
        {
            int palette, value;
            GET(x_src, y_src, &value, &palette);
            u32 color = palettes[palette].values[value];
            tex1_data_ptr->r = (color >> 16) & 0xFF;
            tex1_data_ptr->g = (color >> 8) & 0xFF;
            tex1_data_ptr->b = (color) & 0xFF;
            tex1_data_ptr++;
        }

        for (s32 y_brush = 0; y_brush < BRUSH_Y; y_brush++)
        for (s32 x_brush = 0; x_brush < BRUSH_X; x_brush++)
        {
            s32 x_src = mouse_x_src - 3 + x_brush;
            s32 y_src = mouse_y_src - 3 + y_brush;
            clamp(&x_src, 0, SRC_X-1);
            clamp(&y_src, 0, SRC_Y-1);
            s32 mask = brushes[brush.type][y_brush*BRUSH_X+x_brush];
            if (mask)
            {
                Pixel p = { 230, 30, 0 };
                tex1_data[y_src*SRC_X+x_src] = p;
            }
        }

        glBindTexture(GL_TEXTURE_2D, tex1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SRC_X, SRC_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, tex1_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);
        clearc(0, 0, 0, 1);
        begin(&pass);
        glBindTexture(GL_TEXTURE_2D, tex1);
        so_draw_fullscreen_quad();
    }
    else
    {
        struct RGBA {
            u08 r, g, b, a;
        };
        RGBA tex1_data[SRC_X*SRC_Y];
        RGBA *tex1_data_ptr = tex1_data;
        for (s32 y_src = 0; y_src < SRC_Y; y_src++)
        for (s32 x_src = 0; x_src < SRC_X; x_src++)
        {
            tex1_data_ptr->r = 0;
            tex1_data_ptr->g = 0;
            tex1_data_ptr->b = 0;
            tex1_data_ptr->a = 0;
            tex1_data_ptr++;
        }

        for (s32 y_brush = 0; y_brush < BRUSH_Y; y_brush++)
        for (s32 x_brush = 0; x_brush < BRUSH_X; x_brush++)
        {
            s32 x_src = mouse_x_src - 3 + x_brush;
            s32 y_src = mouse_y_src - 3 + y_brush;
            clamp(&x_src, 0, SRC_X-1);
            clamp(&y_src, 0, SRC_Y-1);
            s32 mask = brushes[brush.type][y_brush*BRUSH_X+x_brush];
            if (mask)
            {
                RGBA p = { 230, 30, 0, 255 };
                tex1_data[y_src*SRC_X+x_src] = p;
            }
        }

        glBindTexture(GL_TEXTURE_2D, tex1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SRC_X, SRC_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex1_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);
        clearc(0, 0, 0, 1);
        begin(&pass);
        glBindTexture(GL_TEXTURE_2D, tex0);
        so_draw_fullscreen_quad();

        glBindTexture(GL_TEXTURE_2D, tex1);
        so_draw_fullscreen_quad();
    }
}

#include "sumo.cpp"
