#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4

// bool trace_floor(vec3 ro, vec3 rd, out vec3 p, out float t)
// {
//     t = -ro.y / rd.y;
//     if (t > 0.0)
//     {
//         p = ro + rd*t;
//         return true;
//     }
//     else
//     {
//         return false;
//     }
// }

// float march_dist(vec3 p)
// {
//     return length(p - vec3(0.5, 0.0, -1.4)) - 0.4;
// }

// bool march(vec3 ro, vec3 rd, out vec3 p, out float t)
// {
//     t = 0.0;
//     for (int i = 0; i < 64; i++)
//     {
//         p = ro + rd * t;
//         float d = march_dist(p);
//         if (d < 0.01)
//         {
//             return true;
//         }
//         t += d;
//     }
//     return false;
// }

// vec3 shade(vec3 ro, vec3 rd)
// {
//     vec3 p;
//     float t1, t2;

//     bool hit1 = march(ro, rd, p, t1); if (!hit1) t1 = 999.0;
//     bool hit2 = trace_floor(ro, rd, p, t2); if (!hit2) t2 = 999.0;

//     vec3 c = vec3(0.0);
//     if (t1 < t2)
//     {
//         c = vec3(p.y*5.0);
//         //c = vec3(1.0, 0.3, 0.1);
//         //float fog = smoothstep(0.0, 5.0, dot(p-ro, p-ro));
//       //c *= 1.0 - fog;
//     }
//     else if (t2 < t1)
//     {
//         c = vec3(1.0, 1.0, 1.0);
//     }

//     return c;
// }

// void mainImage( out vec4 fragColor, in vec2 fragCoord )
// {
//     vec3 forward = vec3(0.0, 0.0, -1.0);
//     vec3 right = vec3(1.0, 0.0, 0.0);
//     vec3 up = cross(right, forward);
//     float u = -1.0 + 2.0*fragCoord.x / iResolution.x;
//     float v = -1.0 + 2.0*fragCoord.y / iResolution.x;
//     vec3 rd = normalize(1.18*forward + u*right + v*up);
//     vec3 ro = vec3(0.0, 0.5, 0.0);
//     fragColor.rgb = pow(shade(ro, rd), vec3(1.0/2.2));
//     fragColor.a = 1.0;
// }

void init()
{

}

void tick(Input io, float t, float dt)
{
    glViewport(0, 0, io.window_width, io.window_height);
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    ImGui::NewFrame();
    persist float lightColor[4];
    persist float attenuation;
    ImGui::Begin("Diffuse Shader");
    ImGui::ColorEdit4("lightColor", lightColor);
    ImGui::SliderFloat("attenuation", &attenuation, 1.0f, 16.0f);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
