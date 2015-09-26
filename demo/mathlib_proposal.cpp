// http://www.reedbeta.com/blog/2013/12/28/on-vector-math-libraries/
// I agree on a lot of the points. I do not like templates in general,
// but for this one case (math) it does make life somewhat better.

template <typename T, int n>
struct Vector
{
    T data[n];
};

template <typename T, int rows, int columns>
struct Matrix
{
    T data[rows*columns];
};

typedef Vector<float, 2> vec2;
typedef Vector<float, 3> vec3;
typedef Vector<float, 4> vec4;
typedef Matrix<float, 2, 2> mat2;
typedef Matrix<float, 3, 3> mat3;
typedef Matrix<float, 4, 4> mat4;

///////////////// Vector Specializations /////////////////
// Specializations for n = 2, 3, 4 so that we can access each
// component of a vector through .a notation, as well as access
// truncated vectors.

template <typename T> struct Vector<T, 2> {
    union {
        T data[2];
        struct { T x, y; };
    };
};

template <typename T> struct Vector<T, 3> {
    union {
        T data[3];
        struct { T x, y, z; };
        struct { T r, g, b; };
        Vector<T, 2> xy;
    };
};

template <typename T> struct Vector<T, 4> {
    union {
        T data[4];
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        Vector<T, 3> xyz;
        Vector<T, 3> rgb;
        Vector<T, 2> xy;
    };
};

///////////////// Matrix Specializations /////////////////
// Specializations for m = n = 2, 3, 4 so that we can access
// each column of a matrix through .a notation.

// The data layout for each matrix is interpreted as column
// order. For example
// | a11 a12 |
// | a21 a22 |
// is stored in memory as [a11, a21, a12, a22].
// The columns of the matrix can be accessed via .a1 or .a2.

template <typename T> struct Matrix<T, 2, 2> {
    union {
        T data[4];
        struct { T a11, a21, a12, a22; };
        struct { Vector<T, 2> a1, a2; };
    };
};

template <typename T> struct Matrix<T, 3, 3> {
    union {
        T data[9];
        struct { T a11, a21, a31,
                   a12, a22, a32,
                   a13, a23, a33; };
        struct { Vector<T, 3> a1, a2, a3; };
    };
};

template <typename T> struct Matrix<T, 4, 4> {
    union {
        T data[16];
        struct { T a11, a21, a31, a41,
                   a12, a22, a32, a42,
                   a13, a23, a33, a43,
                   a14, a24, a34, a44; };
        struct { Vector<T, 4> a1, a2, a3, a4; };
    };
};

//////////// Matrix and vector constructors ////////////
// Convenience functions for constructing matrices and
// vectors, from components or other vectors.

vec2 m_vec2(float x, float y)                   { vec2 result = { x, y       }; return result; }
vec3 m_vec3(float x, float y, float z)          { vec3 result = { x, y, z    }; return result; }
vec4 m_vec4(float x, float y, float z, float w) { vec4 result = { x, y, z, w }; return result; }

template <typename T>
Matrix<T, 2, 2> m_mat2_(Vector<T, 2> a1,
                        Vector<T, 2> a2)
{
    mat2 result = { };
    result.a1 = a1;
    result.a2 = a2;
    return result;
}

template <typename T>
Matrix<T, 3, 3> m_mat3_(Vector<T, 3> a1,
                        Vector<T, 3> a2,
                        Vector<T, 3> a3)
{
    mat3 result = { };
    result.a1 = a1;
    result.a2 = a2;
    result.a3 = a3;
    return result;
}

template <typename T>
Matrix<T, 3, 3> m_mat3_(Matrix<T, 4, 4> m)
{
    mat3 result = { };
    result.a1 = m.a1.xyz;
    result.a2 = m.a2.xyz;
    result.a3 = m.a3.xyz;
    return result;
}

template <typename T>
Matrix<T, 4, 4> m_mat4_(Vector<T, 4> a1,
                        Vector<T, 4> a2,
                        Vector<T, 4> a3,
                        Vector<T, 4> a4)
{
    mat4 result = { };
    result.a1 = a1;
    result.a2 = a2;
    result.a3 = a3;
    result.a4 = a4;
    return result;
}

template <typename T>
Matrix<T, 4, 4> m_mat4_(Matrix<T, 3, 3> m)
{
    mat4 result = m_identity_<T, 4>();
    result.a1.xyz = m.a1;
    result.a2.xyz = m.a2;
    result.a3.xyz = m.a3;
    result.a4.xyz = m.a4;
    return result;
}

#define m_mat2  m_mat2_<float>
#define m_mat2i m_mat2_<int>
#define m_mat2u m_mat2_<unsigned int>

#define m_mat3  m_mat3_<float>
#define m_mat3i m_mat3_<int>
#define m_mat3u m_mat3_<unsigned int>

#define m_mat4  m_mat4_<float>
#define m_mat4i m_mat4_<int>
#define m_mat4u m_mat4_<unsigned int>

#define m_id2   m_identity_<float, 2>
#define m_id2i  m_identity_<int, 2>
#define m_id2u  m_identity_<unsigned int, 2>

#define m_id3   m_identity_<float, 3>
#define m_id3i  m_identity_<int, 3>
#define m_id3u  m_identity_<unsigned int, 3>

#define m_id4   m_identity_<float, 4>
#define m_id4i  m_identity_<int, 4>
#define m_id4u  m_identity_<unsigned int, 4>

///////////////// Matrix functions /////////////////
template <typename T, int r, int c>
T *m_element(Matrix<T, r, c> *m, int row, int column)
{
    return m->data + row + column * r;
}

template <typename T, int ra, int ca, int cb>
Matrix<T, ra, cb> operator *(Matrix<T, ra, ca> a,
                             Matrix<T, ca, cb> b)
{
    Matrix<T, ra, cb> result = {};
    T *entry = (T*)result.data;
    for (int col = 0; col < cb; col++)
    for (int row = 0; row < ra; row++)
    {
        for (int i = 0; i < ca; i++)
        {
            T x = a.data[i * ra + row];
            T y = b.data[i + col * ca];
            *entry += x * y;
        }
        entry++;
    }
    return result;
}

template <typename T, int r, int c>
Matrix<T, r, c> operator *(Matrix<T, r, c> a, T s)
{
    Matrix<T, r, c> result = {};
    for (int i = 0; i < r*c; i++) result.data[i] = a.data[i] * s;
    return result;
}

template <typename T, int r, int c>
Matrix<T, r, c> operator *(T s, Matrix<T, r, c> a)
{
    Matrix<T, r, c> result = {};
    for (int i = 0; i < r*c; i++) result.data[i] = a.data[i] * s;
    return result;
}

template <typename T, int n>
Matrix<T, n, n> m_identity_()
{
    Matrix<T, n, n> result = { };
    for (unsigned int i = 0; i < n; i++)
        result.data[i*n+i] = 1;
    return result;
}

template <typename T, int r, int c>
Matrix<T, c, r> m_transpose(Matrix<T, r, c> m)
{
    Matrix<T, c, r> result = {};
    for (int row = 0; row < r; row++)
    for (int col = 0; col < c; col++)
    {
        T *a = m_element(&result, col, row);
        T *b = m_element(&m, row, col);
        *a = *b;
    }
    return result;
}

///////////////// Vector functions /////////////////
template <typename T, int n>
Vector<T, n> operator *(Vector<T, n> v, T s)
{
    Vector<T, n> result = {};
    for (int i = 0; i < n; i++) result.data[i] = v.data[i] * s;
    return result;
}

template <typename T, int n>
Vector<T, n> operator *(T s, Vector<T, n> v)
{
    Vector<T, n> result = {};
    for (int i = 0; i < n; i++) result.data[i] = v.data[i] * s;
    return result;
}

template <typename T, int n>
T m_dot(Vector<T, n> a, Vector<T, n> b)
{
    T result = 0;
    for (int i = 0; i < n; i++)
        result += a.data[i] * b.data[i];
    return result;
}

template <int n>
float m_length(Vector<float, n> v)
{
    float = sqrt(m_dot(v, v));
    return float;
}

float m_fast_inv_sqrt(float x)
{
    float xhalf = 0.5f * x;
    int i = *(int*)&x;          // Integer representation of float
    i = 0x5f3759df - (i >> 1);  // Initial guess
    x = *(float*)&i;            // Converted to floating point
    x = x*(1.5f-(xhalf*x*x));   // One round of Newton-Raphson's method
    return x;
}

template <int n>
float m_normalize(Vector<float, n> v)
{
    Vector<float, n> result = v * m_fast_inv_sqrt(m_dot(v, v));
    return result;
}

///////////////// Linear algebra /////////////////
mat3 m_skew(vec3 v)
{
    mat3 result = {0, v.z, -v.y, -v.z, 0, v.x, v.y, -v.x, 0};
    return result;
}

///////////////// GLSL-like stuff ////////////////
template <typename T>
T m_clamp(T x, T low, T high)
{
    if (x < low)       return low;
    else if (x > high) return high;
    else               return x;
}

template <typename T, int n>
Vector<T, n> m_clamp(Vector<T, n> v, Vector<T, n> low, Vector<T, n> high)
{
    Vector<T, n> result = {};
    for (int i = 0; i < n; i++)
        result.data[i] = m_clamp(v.data[i], low.data[i], high.data[i]);
    return result;
}

float m_map(float t0, float t1,
            float t,
            float y0, float y1)
{
    return m_clamp(y0 + (y1 - y0) * (t - t0) / (t1 - t0), y0, y1);
}

float mixf(float lo,
           float hi,
           float t) {
    return lo + (hi - lo) * t;
}

template <int n>
Vector<float, n> m_mix(Vector<float, n> low,
                       Vector<float, n> high,
                       float t)
{
    return lo + (hi - lo) * t;
}

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

    return 0;
}
