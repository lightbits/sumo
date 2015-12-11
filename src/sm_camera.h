#ifndef SM_CAMERA_HEADER_INCLUDE
#define SM_CAMERA_HEADER_INCLUDE

mat4 camera_holdclick(Input io, float dt, float move_speed = 1.0f);

#ifdef USE_NEW_MATH
mat4 camera_fps(Input io, float dt, float move_speed = 1.0f, float sensitivity = 3.0f);
#endif

#endif // SM_CAMERA_HEADER_INCLUDE
#ifdef SM_CAMERA_IMPLEMENTATION

#ifdef USE_NEW_MATH
mat4 camera_fps(Input io, float dt, float move_speed, float sensitivity)
{
    SDL_SetRelativeMouseMode(SDL_TRUE);

    persist r32 theta = 0.0f;
    persist r32 dtheta = 0.0f;
    persist r32 phi = 0.0f;
    persist r32 dphi = 0.0f;
    persist r32 x = 0.0f;
    persist r32 dx = 0.0f;
    persist r32 y = 0.0f;
    persist r32 dy = 0.0f;
    persist r32 z = 0.0f;
    persist r32 dz = 0.0f;

    r32 ktheta = 2.0f * PI / (0.1f * WINDOW_WIDTH);
    r32 kphi = 2.0f * PI / (0.1f * WINDOW_WIDTH);
    dtheta = ktheta * io.mouse.rel.x * sensitivity;
    dphi = kphi * io.mouse.rel.y * sensitivity;
    theta += dt * dtheta;
    phi += dt * dphi;
    dtheta = exp(-5.0f * dt) * dtheta;
    dphi = exp(-5.0f * dt) * dphi;

    if (io.key.down[SDL_SCANCODE_A])
        dx = -1.0f;
    else if (io.key.down[SDL_SCANCODE_D])
        dx = +1.0f;
    if (io.key.down[SDL_SCANCODE_W])
        dz = -1.0f;
    else if (io.key.down[SDL_SCANCODE_S])
        dz = +1.0f;
    if (io.key.down[SDL_SCANCODE_LCTRL])
        dy = -0.35f;
    else if (io.key.down[SDL_SCANCODE_SPACE])
        dy = +0.35f;
    dx = exp(-5.0f * dt) * dx;
    dy = exp(-5.0f * dt) * dy;
    dz = exp(-5.0f * dt) * dz;

    mat3 R = m_mat3(mat_rotate_y(-theta) * mat_rotate_x(-phi));
    vec3 v = m_vec3(dx, dy, dz);
    vec3 dp = R*v;

    x += dp.x * move_speed * dt;
    y += dp.y * move_speed * dt;
    z += dp.z * move_speed * dt;

    mat3 T = m_transpose(R);
    vec3 p = m_vec3(x, y, z);
    return m_se3(T, -T*p);
}
#endif

mat4 camera_holdclick(Input io, float dt, float move_speed)
{
    persist r32 theta = 0.0f;
    persist r32 dtheta = 0.0f;
    persist r32 phi = 0.0f;
    persist r32 dphi = 0.0f;
    persist r32 x = 0.0f;
    persist r32 dx = 0.0f;
    persist r32 y = 0.0f;
    persist r32 dy = 0.0f;
    persist r32 z = 0.0f;
    persist r32 dz = 0.0f;

    r32 ktheta = 2.0f * PI / (0.1f * WINDOW_WIDTH);
    r32 kphi = (PI / 2.0f) / (0.1f * WINDOW_HEIGHT);
    if (io.mouse.left.down)
    {
        dtheta = ktheta * io.mouse.rel.x;
        dphi = kphi * io.mouse.rel.y;
    }
    theta += dt * dtheta;
    phi += dt * dphi;
    dtheta = exp(-5.0f * dt) * dtheta;
    dphi = exp(-5.0f * dt) * dphi;

    mat4 rotation = mat_rotate_x(-phi) * mat_rotate_y(theta);

    // Use this for the FPS camera!
    // frame * vec3(0, 0, -1) = forward
    // frame * vec3(0, 1, 0) = up
    // frame * vec3(1, 0, 0) = right
    // mat4 frame = mat_rotate_y(-theta) * mat_rotate_x(phi);
    // vec3 forward = vec3(-frame.z.x, -frame.z.y, -frame.z.z);
    // vec3 right = vec3(frame.x.x, frame.x.y, frame.x.z);
    // vec3 up = vec3(frame.y.x, frame.y.y, frame.y.z);

    if (io.key.down[SDL_SCANCODE_A])
        dx = -1.0f;
    else if (io.key.down[SDL_SCANCODE_D])
        dx = +1.0f;
    if (io.key.down[SDL_SCANCODE_W])
        dz = -1.0f;
    else if (io.key.down[SDL_SCANCODE_S])
        dz = +1.0f;
    if (io.key.down[SDL_SCANCODE_LCTRL])
        dy = -1.0f;
    else if (io.key.down[SDL_SCANCODE_SPACE])
        dy = +1.0f;
    dx = exp(-5.0f * dt) * dx;
    dy = exp(-5.0f * dt) * dy;
    dz = exp(-5.0f * dt) * dz;
    x += dx * dt;
    y += dy * dt;
    z += dz * dt;
    mat4 translation = mat_translate(-x, -y, -z);

    return translation * rotation;
}
#endif // SM_CAMERA_IMPLEMENTATION
