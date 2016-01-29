#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define GLSL150(src) "#version 150\n" #src

char *VS = GLSL150(
in vec2 position;
out vec2 quadcoord;
void main()
{
    quadcoord = position;
    gl_Position = vec4(position, 0.0, 1.0);
}
);

char *FS = GLSL150(
in vec2 quadcoord;
uniform float lens_width;
uniform float lens_height;
uniform float focal_distance;
uniform float k1;
uniform float k2;
uniform mat4 c_to_w;
uniform sampler2D channel0;

// Debugging visualizations
uniform vec2 vis_point0;
uniform vec2 vis_point1;
uniform vec3 vis_line;

out vec4 f_color;

const float PLANE_SIZE = 3.2;

bool trace(vec3 ro, vec3 rd, out vec2 p)
{
    float t = -ro.z / rd.z;
    if (t >= 0.0)
    {
        p = (ro + rd * t).xy;
        if (abs(p.x) <= PLANE_SIZE &&
            abs(p.y) <= PLANE_SIZE)
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}

void main()
{
    vec2 sample = quadcoord;
    float r = length(sample);
    sample = (1.0 + k1*r*r + k2*r*r*r*r) * sample;

    vec3 film = vec3(sample.x * lens_width/2.0,
                     sample.y * lens_height/2.0, -focal_distance);

    vec3 origin = (c_to_w[3]).xyz;
    vec3 dir = normalize((c_to_w * vec4(film, 1.0)).xyz - origin);

    vec2 plane;
    if (trace(origin, dir, plane))
    {
        vec2 floor_texel = vec2(0.5) + 0.5 * plane / vec2(PLANE_SIZE);
        f_color.rgb = texture(channel0, floor_texel).rgb;
        f_color.rgb *= 1.0 - 0.2*length(quadcoord);

        if (abs(dot(vis_line, vec3(plane.xy, 1.0))) < 0.1)
        {
            f_color.r += 0.5;
        }

        if (length(vis_point0-plane.xy) < 0.1)
        {
            f_color.g += 0.5;
        }

        if (length(vis_point1-plane.xy) < 0.1)
        {
            f_color.g += 0.5;
        }
    }
    else
    {
        f_color.rgb = vec3(0.84, 0.82, 0.75) * (1.0 + 0.1*abs(dir.y));
    }
    f_color.a = 1.0;
}
);

RenderPass pass;
GLuint quad;
GLuint tex;

void init()
{
    pass = make_render_pass(VS, FS);
    quad = make_quad();
    tex = so_load_tex2d("C:/Resources/textures/kitchen-tiles.jpg",
                        0, 0, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
}

vec3 trace_floor(mat3 c_to_w, vec3 ro, vec3 pc)
{
    vec3 rd = c_to_w * m_normalize(pc);
    float t = -ro.z / rd.z;
    return ro + rd * t;
}

typedef vec3 LineEquation;

void test_match_line(vec2 p0, vec2 p1,
                     LineEquation *out_best_match)
{
    LineEquation lines[] = {
        // verticals
        { 1.0f, 0.0f, +2.53f },
        { 1.0f, 0.0f, +1.50f },
        { 1.0f, 0.0f, +0.50f },
        { 1.0f, 0.0f, -0.53f },
        { 1.0f, 0.0f, -1.55f },
        { 1.0f, 0.0f, -2.58f },

        // horizontals
        { 0.0f, 1.0f, +2.65f },
        { 0.0f, 1.0f, +1.60f },
        { 0.0f, 1.0f, +0.59f },
        { 0.0f, 1.0f, -0.41f },
        { 0.0f, 1.0f, -1.45f },
        { 0.0f, 1.0f, -2.46f }
    };

    vec2 n = m_normalize(p1 - p0);
    n = m_vec2(n.y, -n.x);

    int count = array_count(lines);
    float min_e = 0.0f;
    int min_i = -1;
    for (int line_i = 0; line_i < count; line_i++)
    {
        LineEquation line = lines[line_i];
        float e0 = m_abs(m_dot(m_vec3(p0.x, p0.y, 1.0f), line));
        float e1 = m_abs(m_dot(m_vec3(p1.x, p1.y, 1.0f), line));
        float e = e0 + e1;

        if (min_i < 0 || e < min_e)
        {
            min_e = e;
            min_i = line_i;
        }
    }

    LineEquation best_match = lines[min_i];
    *out_best_match = best_match;
}

LineEquation line_from_points(vec2 p0, vec2 p1)
{
    vec2 n = m_normalize(p1 - p0);
    n = m_vec2(n.y, -n.x);
    LineEquation result = { n.x, n.y, -m_dot(n, p0) };
    return result;
}

void tick(Input io, float t, float dt)
{
    persist float focal_distance_mm = 3.67f;
    persist float lens_width = 4.8f * 0.001f;
    persist float lens_height = 3.6f * 0.001f;
    persist float k1 = 0.0f; // Default: 0.1f;
    persist float k2 = 0.0f; // Default: 0.05f;
    persist vec3  camera_p = m_vec3(0.0f, 0.0f, 1.3f);
    persist float camera_phi = 0.0f;
    persist float camera_theta = 0.0f;
    persist float camera_psi = 0.0f;

    float focal_distance = focal_distance_mm * 0.001f;

    persist vec2 vis_point0;
    persist vec2 vis_point1;
    persist vec3 vis_line;
    {
        persist vec2 selected_ndc_0 = m_vec2(-0.6f, 0.0f);
        persist vec2 selected_ndc_1 = m_vec2(-0.6f, -0.5f);
        persist vec3 selected_w_0;
        persist vec3 selected_w_1;

        // Our estimate of the rotation matrix
        mat3 R;
        {
            float n1 = 0.02f*(-1.0f + 2.0f * frand());
            float n2 = 0.02f*(-1.0f + 2.0f * frand());
            float n3 = 0.02f*(-1.0f + 2.0f * frand());
            R = m_mat3(mat_rotate_x(camera_theta+n1)*
                       mat_rotate_y(camera_phi+n2)*
                       mat_rotate_z(camera_psi+n3));
        }

        if (io.mouse.left.down)
        {
            vec2 *modify_ndc = &selected_ndc_1;
            vec3 *modify_w = &selected_w_1;
            if (m_length(selected_ndc_0-io.mouse.ndc) <
                m_length(selected_ndc_1-io.mouse.ndc))
            {
                modify_ndc = &selected_ndc_0;
                modify_w = &selected_w_0;
            }

            float pc_x = io.mouse.ndc.x * lens_width/2.0f;
            float pc_y = io.mouse.ndc.y * lens_height/2.0f;
            float pc_z = -focal_distance;
            vec3 pc = m_vec3(pc_x, pc_y, pc_z);
            vec3 pw = trace_floor(R, m_vec3(0.0f, 0.0f, camera_p.z), pc);

            *modify_ndc = io.mouse.ndc;
            *modify_w = pw;
        }

        vis_point0 = selected_w_0.xy;
        vis_point1 = selected_w_1.xy;

        LineEquation best_match;
        test_match_line(selected_w_0.xy, selected_w_1.xy, &best_match);
        vis_line = best_match;
    }

    // Render everything
    {
        mat3 camera_R = m_mat3(mat_rotate_x(camera_theta)*
                               mat_rotate_y(camera_phi)*
                               mat_rotate_z(camera_psi));
        mat4 c_to_w = m_se3(camera_R, camera_p);

        clearc(1.0f, 1.0f, 1.0f, 1.0f);
        begin(&pass);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindBuffer(GL_ARRAY_BUFFER, quad);
        attribfv("position", 2, 2, 0);
        uniformf("c_to_w", c_to_w);
        uniformf("lens_width", lens_width);
        uniformf("lens_height", lens_height);
        uniformf("focal_distance", focal_distance);
        uniformf("k1", k1);
        uniformf("k2", k2);
        uniformf("vis_point0", vis_point0);
        uniformf("vis_point1", vis_point1);
        uniformf("vis_line", vis_line);
        uniformi("channel0", 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui::NewFrame();
        ImGui::Begin("Camera");
        ImGui::PushItemWidth(0.4f * ImGui::GetWindowWidth());
        ImGui::SliderFloat("focal_distance [mm]", &focal_distance_mm, 1.0f, 10.0f);
        ImGui::PopItemWidth();
        ImGui::SliderFloat("x", &camera_p.x, 0.1f, 3.0f);
        ImGui::SliderFloat("y", &camera_p.y, 0.1f, 3.0f);
        ImGui::SliderFloat("z", &camera_p.z, 0.1f, 3.0f);
        ImGui::SliderAngle("phi", &camera_phi, -180.0f, 180.0f);
        ImGui::SliderAngle("theta", &camera_theta, -180.0f, 180.0f);
        ImGui::SliderAngle("psi", &camera_psi, -180.0f, 180.0f);
        if (ImGui::CollapsingHeader("Brown-Conrady coefficients"))
        {
            ImGui::SliderFloat("k1", &k1, -1.0f, 1.0f);
            ImGui::SliderFloat("k2", &k2, -1.0f, 1.0f);
        }
        ImGui::End();
        ImGui::Render();
    }
}

#include "sumo.cpp"
