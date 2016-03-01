#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GLSL(src) "#version 150 \n" #src

#if 0
#include <stdio.h>
struct soi_Lexer
{
    char *at;
};

enum soi_TokenType
{
    SOI_TOKEN_EOF=0,
    SOI_TOKEN_INCLUDE,
    SOI_TOKEN_STRING,
    SOI_TOKEN_COMMENT,
    SOI_TOKEN_OTHER
};

struct soi_Token
{
    char *begin;
    char *end;
    soi_TokenType type;
};

static bool
soi_is_whitespace(char c)
{
    return c == ' ' ||
           c == '\n' ||
           c == '\r';
}

static bool
soi_match_string(char *first,
                 char *last_plus_one,
                 char *null_terminated_query)
{
    char *a = first;
    char *b = null_terminated_query;
    while (a != last_plus_one)
    {
        if (*a != *b)
        {
            return false;
        }
        a++;
        b++;
    }

    if (*b != 0)
    {
        return false;
    }

    return true;
}

bool
soi_get_token(soi_Token *token, soi_Lexer *lexer)
{
    while (*(lexer->at) && soi_is_whitespace(*(lexer->at)))
        lexer->at++;
    char *first = lexer->at;
    while (*(lexer->at) && !soi_is_whitespace(*(lexer->at)))
        lexer->at++;
    char *last_plus_one = lexer->at;

    if (first == last_plus_one)
    {
        token->type = SOI_TOKEN_EOF;
        return false;
    }

    else

    if (first[0] == '/' && first[1] == '/')
    {
        while (*(lexer->at) && *(lexer->at) != '\n')
            lexer->at++;
        last_plus_one = lexer->at;
        token->type = SOI_TOKEN_COMMENT;
        token->begin = first+2;
        token->end = last_plus_one-1;
        return true;
    }

    else

    if (first[0] == '/' && first[1] == '*')
    {
        while (*(lexer->at))
        {
            if (lexer->at[0] == '*' && lexer->at[1] == '/')
            {
                break;
            }
            lexer->at++;
        }
        last_plus_one = lexer->at;
        token->type = SOI_TOKEN_COMMENT;
        token->begin = first+2;
        token->end = last_plus_one-1;
        lexer->at += 2;
        return true;
    }

    else

    if (*first == '"')
    {
        lexer->at = first+1;
        while (*(lexer->at) && *(lexer->at) != '"')
            lexer->at++;
        last_plus_one = lexer->at;
        token->type = SOI_TOKEN_STRING;
        token->begin = first+1;
        token->end = last_plus_one-1;
        return true;
    }

    else

    if (soi_match_string(first, last_plus_one, "#include"))
    {
        token->type = SOI_TOKEN_INCLUDE;
        token->begin = first;
        token->end = last_plus_one-1;
        return true;
    }

    else

    {
        token->type = SOI_TOKEN_OTHER;
        token->begin = first;
        token->end = last_plus_one-1;
        return true;
    }
}

static void
soi_preprocess_source(char *source)
{
    static char buffer[1024*1024];
    soi_Lexer lexer = {};
    lexer.at = source;
    soi_Token token = {};
    while (soi_get_token(&token, &lexer))
    {
        if (token.type == SOI_TOKEN_INCLUDE)
        {
            soi_Token path = {};
            soi_get_token(&path, &lexer);
            if (path.type == SOI_TOKEN_STRING)
            {
                printf("#include '%.*s'\n", path.end+1-path.begin, path.begin);
            }
        }
        if (token.type == SOI_TOKEN_COMMENT)
        {
            printf("comment '%.*s'\n", token.end+1-token.begin, token.begin);
        }
    }
}
#endif

char *vs = GLSL(
in vec2 position;
out vec2 v_position;
void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    v_position = position;
}
);

char *fs = GLSL(
in vec2 v_position;
uniform int steps;
uniform sampler2D sampler0; // t | hit | UNUSED | UNUSED
out vec4 out0; // t | hit | UNUSED | UNUSED
out vec4 out1; // color r g b | UNUSED

float MAP(vec3 p)
{
    return length(p) - 0.5;
}

vec3 NORMAL(vec3 p)
{
    vec2 e = vec2(0.001, 0.0);
    return normalize(vec3(
                     MAP(p + e.xyy) - MAP(p - e.xyy),
                     MAP(p + e.yxy) - MAP(p - e.yxy),
                     MAP(p + e.yyx) - MAP(p - e.yyx)));
}

vec3 STEP(vec3 ro, vec3 rd, float dt)
{
    return ro + rd * dt;
}

vec3 SHADE(vec3 p)
{
    vec3 N = NORMAL(p);
    return vec3(0.5) + 0.5 * N;
}

void main()
{
    vec2 texel = vec2(0.5) + 0.5 * v_position;
    vec3 forward = vec3(0.0, 0.0, -1.0);
    vec3 right = vec3(1.0, 0.0, 0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);

    vec3 ro = vec3(0.0, 0.0, 2.0);
    vec3 rd = normalize(forward*1.4 + v_position.x*right + v_position.y*up);

    vec4 in0 = texture(sampler0, texel);
    float t0 = in0.x;
    float hit0 = in0.y;
    vec3 p0 = ro + rd * t0;
    if (hit0 < 0.5) // no hit yet, take a step
    {
        float t = t0;
        bool hit = false;
        vec3 p = ro;
        for (int i = 0; i < steps; i++)
        {
            p = ro + rd * t;
            float d = MAP(p);
            if (d < 0.01)
            {
                hit = true;
                break;
            }
            t += d;
        }

        if (hit)
        {
            out0.x = t;
            out0.y = 1.0;
            out1.rgb = SHADE(p);
            out1.a = 1.0;
        }
        else
        {
            out0.x = t;
            out0.y = 0.0;
            out1.rgb = vec3(0.0);
            out1.a = 0.0;
        }
    }
    else
    {
        out0.x = t0;
        out0.y = 1.0;
        out1.rgb = SHADE(p0);
        out1.a = 1.0;
    }
}
);

struct Buffers
{
    so_Framebuffer ping;
    so_Framebuffer pong;
} buf;

RenderPass pass;

void debug_draw_texture2D(GLuint texture,
                          vec4 maskr = m_vec4(1,0,0,0),
                          vec4 maskg = m_vec4(0,1,0,0),
                          vec4 maskb = m_vec4(0,0,1,0),
                          vec4 maska = m_vec4(0,0,0,1))
{
    static bool loaded = 0;
    static RenderPass pass;
    if (!loaded)
    {
        pass = load_render_pass("assets/shaders/debug-draw-rt.vs",
                                "assets/shaders/debug-draw-rt.fs");
        loaded = 1;
    }
    begin(&pass);
    clearc(0, 0, 0, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    uniformi("channel", 0);
    uniformf("maskr", maskr);
    uniformf("maskg", maskg);
    uniformf("maskb", maskb);
    uniformf("maska", maska);
    so_draw_fullscreen_quad();
}

void init()
{
    so_Framebuffer fbos[2];
    for (int i = 0; i < 2; i++)
    {
        GLuint color[2];
        GLenum target[2];
        for (int j = 0; j < 2; j++)
        {
            target[j] = GL_TEXTURE_2D;
            color[j] = so_make_tex2d(0,
                          WINDOW_WIDTH, WINDOW_HEIGHT,
                          GL_RGBA32F,
                          GL_RGBA,
                          GL_FLOAT,
                          GL_NEAREST,
                          GL_NEAREST,
                          GL_CLAMP_TO_EDGE,
                          GL_CLAMP_TO_EDGE);
        }
        so_make_fbo(&fbos[i], WINDOW_WIDTH, WINDOW_HEIGHT, 2,
                    color, target, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, fbos[i].fbo);
        glViewport(0, 0, fbos[i].width, fbos[i].height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    buf.ping = fbos[0];
    buf.pong = fbos[1];

    GLuint program = glCreateProgram();
    glBindFragDataLocation(program, 0, "out0");
    glBindFragDataLocation(program, 1, "out1");
    pass = make_render_pass(program, vs, fs);
}

/*

  PING                      PONG
+-------+                 +-------+
|       |                 |       |
| OUT0  | -> STEP PASS -> | OUT0  |
|       |                 |       |
+-------+                 +-------+
POS HIT                   POS HIT
+-------+                 +-------+
|       |                 |       |
| OUT1  |                 | OUT1  |
|       |                 |       |
+-------+                 +-------+
COLOR                     COLOR
*/

void tick(Input io, float t, float dt)
{
    begin(&pass);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, buf.pong.fbo);
        glViewport(0, 0, buf.pong.width, buf.pong.height);
        GLenum draw_buffers[2] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1
        };
        glDrawBuffers(2, draw_buffers);
        clearc(0, 0, 0, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buf.ping.color[0]);
        uniformi("sampler0", 0);
        uniformi("steps", 4);
        so_draw_fullscreen_quad();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    debug_draw_texture2D(buf.pong.color[1]);

    so_Framebuffer temp = buf.ping;
    buf.ping = buf.pong;
    buf.pong = temp;
}

#include "sumo.cpp"
