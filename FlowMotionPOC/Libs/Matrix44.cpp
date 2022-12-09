#include "Matrix44.hpp"
#include <memory.h>
#include <math.h>

namespace IJK
{
    const Matrix44 Matrix44::IDENTITY = Matrix44();

    Matrix44::Matrix44()
    {
        data[0][0] = 1.0f;
        data[0][1] = 0.0f;
        data[0][2] = 0.0f;
        data[0][3] = 0.0f;

        data[1][0] = 0.0f;
        data[1][1] = 1.0f;
        data[1][2] = 0.0f;
        data[1][3] = 0.0f;

        data[2][0] = 0.0f;
        data[2][1] = 0.0f;
        data[2][2] = 1.0f;
        data[2][3] = 0.0f;

        data[3][0] = 0.0f;
        data[3][1] = 0.0f;
        data[3][2] = 0.0f;
        data[3][3] = 1.0f;
    }

    Matrix44::Matrix44(const Matrix44& reference)
    {
        data[0][0] = reference[0][0];
        data[0][1] = reference[0][1];
        data[0][2] = reference[0][2];
        data[0][3] = reference[0][3];

        data[1][0] = reference[1][0];
        data[1][1] = reference[1][1];
        data[1][2] = reference[1][2];
        data[1][3] = reference[1][3];

        data[2][0] = reference[2][0];
        data[2][1] = reference[2][1];
        data[2][2] = reference[2][2];
        data[2][3] = reference[2][3];

        data[3][0] = reference[3][0];
        data[3][1] = reference[3][1];
        data[3][2] = reference[3][2];
        data[3][3] = reference[3][3];
    }

    IJK::Vector Matrix44::get_translation() const
    {
        const float *translation = data[3];
        return IJK::Vector(translation[0], translation[1], translation[2]);
    }

    IJK::Quaternion  Matrix44::get_rotation() const
    {
        IJK::Vector scale = get_scale();

        IJK::Quaternion rotation = IJK::QuaternionMakeWithMatrix3(
            data[0][0] / scale.x(), data[0][1] / scale.x(), data[0][2] / scale.x(),
            data[1][0] / scale.y(), data[1][1] / scale.y(), data[1][2] / scale.y(),
            data[2][0] / scale.z(), data[2][1] / scale.z(), data[2][2] / scale.z());

        return rotation;
    }

    IJK::Vector  Matrix44::get_scale() const
    {
        IJK::Vector scale(
            sqrtf(data[0][0] * data[0][0] + data[0][1] * data[0][1] + data[0][2] * data[0][2]),
            sqrtf(data[1][0] * data[1][0] + data[1][1] * data[1][1] + data[1][2] * data[1][2]),
            sqrtf(data[2][0] * data[2][0] + data[2][1] * data[2][1] + data[2][2] * data[2][2]));

        return scale;
    }

    void Matrix44::rotate(const Quaternion& rotation)
    {
        IJK::Quaternion new_rotation = get_rotation() * rotation;
        set_rotation(new_rotation);
    }

    void Matrix44::rotate(const IJK::Vector& axis, float angle)
    {
        IJK::Quaternion new_rotation = get_rotation() * IJK::Quaternion(axis, angle);
        set_rotation(new_rotation);
    }

    void Matrix44::set_rotation(const Quaternion& rotation)
    {
        // get axis and angle information
        Vector axis(2.0f * rotation.x(), 2.0f * rotation.y(), 2.0f * rotation.z());
        float xx = axis.x() * rotation.x();
        float xy = axis.x() * rotation.y();
        float xz = axis.x() * rotation.z();
        float xw = axis.x() * rotation.w();
        float yy = axis.y() * rotation.y();
        float yz = axis.y() * rotation.z();
        float yw = axis.y() * rotation.w();
        float zz = axis.z() * rotation.z();
        float zw = axis.z() * rotation.w();

        // compute rotation
        data[0][0] = 1.0f - yy - zz;
        data[0][1] = xy - zw;
        data[0][2] = xz + yw;

        data[1][0] = xy + zw;
        data[1][1] = 1.0f - xx - zz;
        data[1][2] = yz - xw;

        data[2][0] = xz - yw;
        data[2][1] = yz + xw;
        data[2][2] = 1.0f - xx - yy;
    }

    void Matrix44::set_rotation(const Vector& axis, float angle)
    {
        IJK::Quaternion rotation(axis, angle);
        set_rotation(rotation);
    }

    void Matrix44::set_rotation_in_matrix(const Quaternion& rotQuat, float rotMatrix[3][3])
    {
        int i, j;

        set_rotation(rotQuat);
        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < 3; j++)
            {
                rotMatrix[i][j] = data[i][j];
            }
        }
    }

    void Matrix44::transform(const Vector& translation, const Quaternion& rotation, const Vector& scale_factor)
    {
        rotate(rotation);
        scale(scale_factor);
        translate(translation);
    }

    void Matrix44::transform(const Matrix44& reference)
    {
        Matrix44 result(*this);

        data[0][0] =  result[0][0] * reference[0][0];
        data[0][0] += result[0][1] * reference[1][0];
        data[0][0] += result[0][2] * reference[2][0];
        data[0][0] += result[0][3] * reference[3][0];

        data[0][1] =  result[0][0] * reference[0][1];
        data[0][1] += result[0][1] * reference[1][1];
        data[0][1] += result[0][2] * reference[2][1];
        data[0][1] += result[0][3] * reference[3][1];

        data[0][2] =  result[0][0] * reference[0][2];
        data[0][2] += result[0][1] * reference[1][2];
        data[0][2] += result[0][2] * reference[2][2];
        data[0][2] += result[0][3] * reference[3][2];

        data[0][3] =  result[0][0] * reference[0][3];
        data[0][3] += result[0][1] * reference[1][3];
        data[0][3] += result[0][2] * reference[2][3];
        data[0][3] += result[0][3] * reference[3][3];

        data[1][0] =  result[1][0] * reference[0][0];
        data[1][0] += result[1][1] * reference[1][0];
        data[1][0] += result[1][2] * reference[2][0];
        data[1][0] += result[1][3] * reference[3][0];

        data[1][1] =  result[1][0] * reference[0][1];
        data[1][1] += result[1][1] * reference[1][1];
        data[1][1] += result[1][2] * reference[2][1];
        data[1][1] += result[1][3] * reference[3][1];

        data[1][2] =  result[1][0] * reference[0][2];
        data[1][2] += result[1][1] * reference[1][2];
        data[1][2] += result[1][2] * reference[2][2];
        data[1][2] += result[1][3] * reference[3][2];

        data[1][3] =  result[1][0] * reference[0][3];
        data[1][3] += result[1][1] * reference[1][3];
        data[1][3] += result[1][2] * reference[2][3];
        data[1][3] += result[1][3] * reference[3][3];

        data[2][0] =  result[2][0] * reference[0][0];
        data[2][0] += result[2][1] * reference[1][0];
        data[2][0] += result[2][2] * reference[2][0];
        data[2][0] += result[2][3] * reference[3][0];

        data[2][1] =  result[2][0] * reference[0][1];
        data[2][1] += result[2][1] * reference[1][1];
        data[2][1] += result[2][2] * reference[2][1];
        data[2][1] += result[2][3] * reference[3][1];

        data[2][2] =  result[2][0] * reference[0][2];
        data[2][2] += result[2][1] * reference[1][2];
        data[2][2] += result[2][2] * reference[2][2];
        data[2][2] += result[2][3] * reference[3][2];

        data[2][3] =  result[2][0] * reference[0][3];
        data[2][3] += result[2][1] * reference[1][3];
        data[2][3] += result[2][2] * reference[2][3];
        data[2][3] += result[2][3] * reference[3][3];

        data[3][0] =  result[3][0] * reference[0][0];
        data[3][0] += result[3][1] * reference[1][0];
        data[3][0] += result[3][2] * reference[2][0];
        data[3][0] += result[3][3] * reference[3][0];

        data[3][1] =  result[3][0] * reference[0][1];
        data[3][1] += result[3][1] * reference[1][1];
        data[3][1] += result[3][2] * reference[2][1];
        data[3][1] += result[3][3] * reference[3][1];

        data[3][2] =  result[3][0] * reference[0][2];
        data[3][2] += result[3][1] * reference[1][2];
        data[3][2] += result[3][2] * reference[2][2];
        data[3][2] += result[3][3] * reference[3][2];

        data[3][3] =  result[3][0] * reference[0][3];
        data[3][3] += result[3][1] * reference[1][3];
        data[3][3] += result[3][2] * reference[2][3];
        data[3][3] += result[3][3] * reference[3][3];
    }

    Matrix44 Matrix44::inverse()
    {
        Matrix44 inverse;
        inverse[0][0] = data[1][1] * data[2][2] * data[3][3] -
                        data[1][1] * data[2][3] * data[3][2] -
                        data[2][1] * data[1][2] * data[3][3] +
                        data[2][1] * data[1][3] * data[3][2] +
                        data[3][1] * data[1][2] * data[2][3] -
                        data[3][1] * data[1][3] * data[2][2];

        inverse[1][0] = -data[1][0]  * data[2][2] * data[3][3] +
                        data[1][0]  * data[2][3] * data[3][2] +
                        data[2][0]  * data[1][2]  * data[3][3] -
                        data[2][0]  * data[1][3]  * data[3][2] -
                        data[3][0] * data[1][2]  * data[2][3] +
                        data[3][0] * data[1][3]  * data[2][2];

        inverse[2][0] = data[1][0]  * data[2][1] * data[3][3] -
                        data[1][0]  * data[2][3] * data[3][1] -
                        data[2][0]  * data[1][1] * data[3][3] +
                        data[2][0]  * data[1][3] * data[3][1] +
                        data[3][0] * data[1][1] * data[2][3] -
                        data[3][0] * data[1][3] * data[2][1];

        inverse[3][0] = -data[1][0]  * data[2][1] * data[3][2] +
                        data[1][0]  * data[2][2] * data[3][1] +
                        data[2][0]  * data[1][1] * data[3][2] -
                        data[2][0]  * data[1][2] * data[3][1] -
                        data[3][0] * data[1][1] * data[2][2] +
                        data[3][0] * data[1][2] * data[2][1];

        inverse[0][1] = -data[0][1]  * data[2][2] * data[3][3] +
                        data[0][1]  * data[2][3] * data[3][2] +
                        data[2][1]  * data[0][2] * data[3][3] -
                        data[2][1]  * data[0][3] * data[3][2] -
                        data[3][1] * data[0][2] * data[2][3] +
                        data[3][1] * data[0][3] * data[2][2];

        inverse[1][1] = data[0][0]  * data[2][2] * data[3][3] -
                        data[0][0]  * data[2][3] * data[3][2] -
                        data[2][0]  * data[0][2] * data[3][3] +
                        data[2][0]  * data[0][3] * data[3][2] +
                        data[3][0] * data[0][2] * data[2][3] -
                        data[3][0] * data[0][3] * data[2][2];

         inverse[2][1] = -data[0][0]  * data[2][1] * data[3][3] +
                        data[0][0]  * data[2][3] * data[3][1] +
                        data[2][0]  * data[0][1] * data[3][3] -
                        data[2][0]  * data[0][3] * data[3][1] -
                        data[3][0] * data[0][1] * data[2][3] +
                        data[3][0] * data[0][3] * data[2][1];

         inverse[3][1] = data[0][0]  * data[2][1] * data[3][2] -
                        data[0][0]  * data[2][2] * data[3][1] -
                        data[2][0]  * data[0][1] * data[3][2] +
                        data[2][0]  * data[0][2] * data[3][1] +
                        data[3][0] * data[0][1] * data[2][2] -
                        data[3][0] * data[0][2] * data[2][1];

        inverse[0][2] = data[0][1]  * data[1][2] * data[3][3] -
                        data[0][1]  * data[1][3] * data[3][2] -
                        data[1][1]  * data[0][2] * data[3][3] +
                        data[1][1]  * data[0][3] * data[3][2] +
                        data[3][1] * data[0][2] * data[1][3] -
                        data[3][1] * data[0][3] * data[1][2];

        inverse[1][2] = -data[0][0]  * data[1][2] * data[3][3] +
                        data[0][0]  * data[1][3] * data[3][2] +
                        data[1][0]  * data[0][2] * data[3][3] -
                        data[1][0]  * data[0][3] * data[3][2] -
                        data[3][0] * data[0][2] * data[1][3] +
                        data[3][0] * data[0][3] * data[1][2];

        inverse[2][2] = data[0][0]  * data[1][1] * data[3][3] -
                        data[0][0]  * data[1][3] * data[3][1] -
                        data[1][0]  * data[0][1] * data[3][3] +
                        data[1][0]  * data[0][3] * data[3][1] +
                        data[3][0] * data[0][1] * data[1][3] -
                        data[3][0] * data[0][3] * data[1][1];

        inverse[3][2] = -data[0][0]  * data[1][1] * data[3][2] +
                        data[0][0]  * data[1][2] * data[3][1] +
                        data[1][0]  * data[0][1] * data[3][2] -
                        data[1][0]  * data[0][2] * data[3][1] -
                        data[3][0] * data[0][1] * data[1][2] +
                        data[3][0] * data[0][2] * data[1][1];

        inverse[0][3] = -data[0][1] * data[1][2] * data[2][3] +
                        data[0][1] * data[1][3] * data[2][2] +
                        data[1][1] * data[0][2] * data[2][3] -
                        data[1][1] * data[0][3] * data[2][2] -
                        data[2][1] * data[0][2] * data[1][3] +
                        data[2][1] * data[0][3] * data[1][2];

        inverse[1][3] = data[0][0] * data[1][2] * data[2][3] -
                        data[0][0] * data[1][3] * data[2][2] -
                        data[1][0] * data[0][2] * data[2][3] +
                        data[1][0] * data[0][3] * data[2][2] +
                        data[2][0] * data[0][2] * data[1][3] -
                        data[2][0] * data[0][3] * data[1][2];

        inverse[2][3] = -data[0][0] * data[1][1] * data[2][3] +
                        data[0][0] * data[1][3] * data[2][1] +
                        data[1][0] * data[0][1] * data[2][3] -
                        data[1][0] * data[0][3] * data[2][1] -
                        data[2][0] * data[0][1] * data[1][3] +
                        data[2][0] * data[0][3] * data[1][1];

        inverse[3][3] = data[0][0] * data[1][1] * data[2][2] -
                        data[0][0] * data[1][2] * data[2][1] -
                        data[1][0] * data[0][1] * data[2][2] +
                        data[1][0] * data[0][2] * data[2][1] +
                        data[2][0] * data[0][1] * data[1][2] -
                        data[2][0] * data[0][2] * data[1][1];

        float determinant = data[0][0] * inverse[0][0] + data[0][1] * inverse[1][0] + data[0][2] * inverse[2][0] + data[0][3] * inverse[3][0];
        if (determinant == 0)
            return Matrix44::IDENTITY;

        determinant = 1.0f / determinant;
        float* values = inverse[0];
        for (int i = 0; i < 16; ++i)
            values[i] *= determinant;

        return inverse;
    }
}
