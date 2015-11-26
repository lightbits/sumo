#include "so_new_math.h"
#include <stdio.h>

template <int n>
void m_printf(Vector<float, n> v)
{
    for (int i = 0; i < n; i++)
        printf("%.1f\n", v.data[i]);
    printf("\n");
}

template <int rows, int cols>
void m_printf(Matrix<float, rows, cols> m)
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            printf("%.1f ", m.data[r + c*rows]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    vec2 v1 = m_vec2(1.0f, 2.0f);
    vec2 v2 = m_vec2(3.0f, 1.0f);

    mat2 m1 = { 1.0f, 2.0f, 3.0f, 4.0f };
    mat2 m2 = m_mat2(v1, v2);

    mat2 m3 = m1 * m2;
    m_printf(m1);
    m_printf(m2);
    m_printf(m3);

    m_printf(v1);

    Matrix<float, 3, 3> a = {};
    Matrix<float, 3, 2> b = {};

    a.data[0] = 2;
    a.data[1] = 3;
    a.data[3] = 4;
    a.data[4] = 5;
    b.data[0] = 1;
    b.data[1] = -1;
    b.data[3] = 1;
    b.data[4] = 2;
    m_printf(a);
    m_printf(b);
    Matrix<float, 3, 2> c = a * b;
    m_printf(c);
    m_printf(m_transpose(c));

    vec3 v3 = m_vec3(1.0f, 2.0f, 3.0f);
    m_printf(v3);
    // m_printf(c * v3);

    return 0;
}
