mat4 camera_holdclick(Input io, float dt, float move_speed)
{
    persist r32 theta = 0.0f;
    persist r32 dtheta = 0.0f;
    persist r32 phi = 0.0f;
    persist r32 dphi = 0.0f;
    persist vec3 pos = vec3(0.0f, 0.0f, 0.0f);
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

    if (io.key.down['a'])
        dx = -1.0f;
    else if (io.key.down['d'])
        dx = +1.0f;
    if (io.key.down['w'])
        dz = -1.0f;
    else if (io.key.down['s'])
        dz = +1.0f;
    if (io.key.down['z'])
        dy = -1.0f;
    else if (io.key.down['x'])
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
