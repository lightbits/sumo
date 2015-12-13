#ifndef SM_CAMERA_HEADER_INCLUDE
#define SM_CAMERA_HEADER_INCLUDE

mat4 camera_holdclick(Input io, float dt, float move_speed = 1.0f);

#ifdef USE_NEW_MATH
mat4 camera_fps(quat *q, vec3 *p,
                Input io, float dt,
                float move_speed = 1.0f,
                float sensitivity = 3.0f);
#endif

#endif // SM_CAMERA_HEADER_INCLUDE
#ifdef SM_CAMERA_IMPLEMENTATION

#ifdef USE_NEW_MATH
// return: The view transformation matrix you would use in your vertex shader
// q: Unit quaternion describing camera frame orientation relative world axes
// p: Position vector of camera frame origin relative world origin
// In screenspace, the camera frame is defined as follows:
// +x: Points to the left of the window
// +y: Points upwards
// +z: Points into the screen
// The view frame is a 180 degree rotation about y of the camera frame.
mat4 camera_fps(quat *q, vec3 *p,
                Input io, float dt,
                float movespeed,
                float sensitivity)
{
    SDL_SetRelativeMouseMode(SDL_TRUE);

    float kw = 2.0f * PI / (0.1f * WINDOW_WIDTH);
    float wy = -kw * io.mouse.rel.x * sensitivity;
    float wx = kw * io.mouse.rel.y * sensitivity;

    float vx = 0.0f;
    float vy = 0.0f;
    float vz = 0.0f;
    if (io_key_down(D))          vx = -1.0f;
    else if (io_key_down(A))     vx = +1.0f;
    if (io_key_down(W))          vz = +1.0f;
    else if (io_key_down(S))     vz = -1.0f;
    if (io_key_down(SPACE))      vy = +1.0f;
    else if (io_key_down(LCTRL)) vy = -1.0f;

    // Integrate each axis by itself, to avoid numerical
    // issues by integrating the combined axis (which drifts
    // if the angular velocity is large). By instead integrating
    // x and y strictly seperately, we avoid off-axis rotation.
    quat dq = 0.5f * m_quat_mul(m_vec4(0, wy, 0, 0), *q);
    *q += dq * dt;
    dq = 0.5f * m_quat_mul(*q, m_vec4(wx, 0, 0, 0));
    *q += dq * dt;
    *q = m_normalize(*q);

    mat3 R = m_quat_to_so3(*q);
    vec3 dp = R*m_vec3(vx, vy, vz);
    *p += dp * dt * movespeed;

    R.a1 *= -1.0f;
    R.a3 *= -1.0f;
    mat3 T = m_transpose(R);
    return m_se3(T, -T*(*p));
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
