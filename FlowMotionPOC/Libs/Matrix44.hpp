#pragma once

#include "Vector.hpp"
#include "Quaternion.hpp"
#include <cstdint>

namespace IJK
{
    class Matrix44
    {
    public:
        static const Matrix44 IDENTITY;

        Matrix44();
        Matrix44(const Matrix44& reference);

        IJK::Vector         get_translation     () const;
        IJK::Quaternion     get_rotation        () const;
        IJK::Vector         get_scale           () const;

        inline void         translate           (const Vector& translation);
        inline void         translate           (float translation);
        inline void         translate           (float x, float y, float z);

        void                rotate              (const Quaternion& rotation);
        void                rotate              (const Vector& axis, float angle);

        inline void         scale               (const Vector& factor);
        inline void         scale               (float factor);
        inline void         scale               (float x, float y, float z);

        void                transform           (const Vector& translation, const Quaternion& rotation, const Vector& scale_factor);
        void                transform           (const Matrix44& reference);

        inline void         set_translation     (const Vector& translation);
        inline void         set_translation     (float translation);
        inline void         set_translation     (float x, float y, float z);

        void                set_rotation        (const Quaternion& rotation);
        void                set_rotation        (const Vector& axis, float angle);
        
        void                set_rotation_in_matrix(const Quaternion& rotQuat, float rotMatrix[3][3]);

        inline void         set_scale           (const Vector& scale);
        inline void         set_scale           (float scale);
        inline void         set_scale           (float x, float y, float z);

        Matrix44            inverse             ();

        inline float*       operator[]          (uint32_t index);
        inline const float* operator[]          (uint32_t index) const;

        inline Matrix44     operator*           (const Matrix44& reference) const;
        inline Matrix44&    operator*=          (const Matrix44& reference);

    private:
        float data[4][4];
    };

    void Matrix44::translate(const Vector& translation)
    {
        translate(translation.x(), translation.y(), translation.z());
    }

    void Matrix44::translate(float translation)
    {
        translate(translation, translation, translation);
    }

    void Matrix44::translate(float x, float y, float z)
    {
        data[3][0] += x;
        data[3][1] += y;
        data[3][2] += z;
    }

    void Matrix44::scale(const Vector& factor)
    {
        scale(factor.x(), factor.y(), factor.z());
    }

    void Matrix44::scale(float factor)
    {
        scale(factor, factor, factor);
    }

    void Matrix44::scale(float x, float y, float z)
    {
        data[0][0] *= x;
        data[1][1] *= y;
        data[2][2] *= z;
    }

    void Matrix44::set_translation(const Vector& translation)
    {
        set_translation(translation.x(), translation.y(), translation.z());
    }

    void Matrix44::set_translation(float translation)
    {
        set_translation(translation, translation, translation);
    }

    void Matrix44::set_translation(float x, float y, float z)
    {
        data[3][0] = x;
        data[3][1] = y;
        data[3][2] = z;
    }

    void Matrix44::set_scale(const Vector& scale)
    {
        set_scale(scale.x(), scale.y(), scale.z());
    }

    void Matrix44::set_scale(float scale)
    {
        set_scale(scale, scale, scale);
    }

    void Matrix44::set_scale(float x, float y, float z)
    {
        data[0][0] = x;
        data[1][1] = y;
        data[2][2] = z;
    }

    float*  Matrix44::operator[](uint32_t index)
    {
         return data[index];
    }

    const float*  Matrix44::operator[](uint32_t index) const
    {
         return data[index];
    }

    Matrix44 Matrix44::operator*(const Matrix44& reference) const
    {
        Matrix44 result(*this);
        result.transform(reference);
        return result;
    }

    Matrix44& Matrix44::operator*=(const Matrix44& reference)
    {
        transform(reference);
        return *this;
    }
}
