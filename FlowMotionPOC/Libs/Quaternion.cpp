//
//  Quaternion.cpp
//
//  Created by Kyler on 8/6/19.
//  Copyright © 2019 GoPro. All rights reserved.

#include "Quaternion.hpp"
#include <math.h>

namespace IJK
{
    Quaternion::Quaternion()
    {
        data[0] = data[1] = data[2] = 0;
        data[3] = 1;
    }

    Quaternion::Quaternion(float x, float y, float z, float w)
    {
        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;
    }

    Quaternion::Quaternion(Vector axis, float radians)
    {

        float t = radians * 0.5f;

        data[0] = axis.x() * sinf(t);
        data[1] = axis.y() * sinf(t);
        data[2] = axis.z() * sinf(t);
        data[3] = cosf(t);

    }

    float Quaternion::x() const
    {
        return data[0];
    }

    float Quaternion::y() const
    {
        return data[1];
    }

    float Quaternion::z() const
    {
        return data[2];
    }

    float Quaternion::w() const
    {
        return data[3];
    }

    void Quaternion::setXYZW(float x, float y, float z, float w)
    {
        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;
    }

    void Quaternion::setX(float x)
    {
        data[0] = x;
    }

    void Quaternion::setY(float y)
    {
        data[1] = y;
    }

    void Quaternion::setZ(float z)
    {
        data[2] = z;
    }

    void Quaternion::setW(float w)
    {
        data[3] = w;
    }

    Vector Quaternion::vector()
    {
        return Vector(data[0], data[1], data[2]);
    }

    Quaternion Quaternion::conjugated() const
    {
        return Quaternion(-data[0], -data[1], -data[2], data[3]);
    }

    float Quaternion::norm() const
    {
        return (data[0]*data[0] + data[1]*data[1] + data[2]*data[2] + data[3]*data[3]);
    }

    float Quaternion::magnitude() const
    {
        return sqrtf(this->norm());

    }
    Quaternion Quaternion::scale(float s) const
    {
        return Quaternion(data[0]*s, data[1]*s, data[2]*s, data[3]*s);
    }

    Quaternion Quaternion::UnitQuaternion()
    {
        return (*this).scale(1/(*this).magnitude());
    }

    Quaternion Quaternion::toAxeAngle()
    {
        Quaternion q = this->UnitQuaternion(); //normalized()
        float norm = sqrtf(q.x() * q.x() + q.y() * q.y() + q.z() * q.z());

        if(norm == 0)
        {
            return Quaternion(1., 0., 0., 0.);
        }
        else
        {
            return Quaternion(q.x()/norm, q.y()/norm, q.z()/norm, 2.0f*acosf(q.w()));
        }
    }

    Quaternion Quaternion::normalized() const
    {
        float lengthSquared = data[0] * data[0] + data[1] * data[1] + data[2] * data[2] + data[3] * data[3];

        if (lengthSquared == 0) {
            return Quaternion(0,0,0,1);
        }
        return Quaternion(*this * (1/sqrt(lengthSquared)));
    }

    Quaternion Quaternion::inverse() const
    {
        return Quaternion(-data[0], -data[1],  -data[2], data[3]);
    }

    Quaternion Quaternion::inverted() const
    {

        float lengthSquared = data[0] * data[0] + data[1] * data[1] + data[2] * data[2] + data[3] * data[3];

        return Quaternion(this->conjugated().x() / lengthSquared,
                        this->conjugated().y() / lengthSquared,
                        this->conjugated().z() / lengthSquared,
                        this->conjugated().w() / lengthSquared);
    }

    Vector Quaternion::rotatedVector(Vector v) const
    {
        return (*this * Quaternion(v.x(), v.y(), v.z(), 0.0) * this->conjugated()).vector();
    }

    bool Quaternion::isZero() {
        auto result = (data[0] == 0.0 && data[1] == 0.0 && data[2] == 0.0 && data[3] == 0.0);
        return result;
    }

    Quaternion Quaternion::operator+(const Quaternion& rotation) const
    {
        Quaternion result(*this);
        result += rotation;
        return result;
    }

    Quaternion& Quaternion::operator+=(const Quaternion& rotation)
    {
        setXYZW(data[0] + rotation.x(), data[1] + rotation.y(), data[2] + rotation.z(), data[3] + rotation.w());
        return *this;
    }

    Quaternion Quaternion::operator-(const Quaternion& rotation) const
    {
        Quaternion result(*this);
        result -= rotation;
        return result;
    }

    Quaternion& Quaternion::operator-=(const Quaternion& rotation)
    {
        setXYZW(data[0] - rotation.x(), data[1] - rotation.y(), data[2] - rotation.z(), data[3] - rotation.w());
        return *this;
    }

    Quaternion Quaternion::operator*(const Quaternion& rotation) const
    {
        Quaternion result(*this);
        result *= rotation;
        return result;
    }

    Quaternion& Quaternion::operator*=(const Quaternion& rotation)
    {
        float x =  data[0] * rotation.w() + data[1] * rotation.z() - data[2] * rotation.y() + data[3] * rotation.x();
        float y = -data[0] * rotation.z() + data[1] * rotation.w() + data[2] * rotation.x() + data[3] * rotation.y();
        float z =  data[0] * rotation.y() - data[1] * rotation.x() + data[2] * rotation.w() + data[3] * rotation.z();
        float w = -data[0] * rotation.x() - data[1] * rotation.y() - data[2] * rotation.z() + data[3] * rotation.w();
        setXYZW(x, y, z, w);

        return *this;
    }

    Quaternion Quaternion::operator/(const Quaternion& rotation) const
    {
        Quaternion result(*this);
        result /= rotation;
        return result;
    }

    Quaternion& Quaternion::operator/=(const Quaternion& rotation)
    {
        float x = data[1] * rotation.x() + data[0] * rotation.y() + data[2] * rotation.w() - data[3] * rotation.z();
        float y = data[0] * rotation.z() - data[1] * rotation.w() + data[2] * rotation.x() + data[3] * rotation.y();
        float z = data[0] * rotation.w() + data[1] * rotation.z() - data[2] * rotation.y() + data[3] * rotation.x();
        float w = data[0] * rotation.x() - data[1] * rotation.y() - data[2] * rotation.z() - data[3] * rotation.w();
        setXYZW(x, y, z, w);

        return *this;
    }

    Quaternion Quaternion::operator*(float factor) const
    {
        Quaternion result(*this);
        result *= factor;
        return result;
    }

    Quaternion& Quaternion::operator*=(float factor)
    {
        setXYZW(data[0] * factor, data[1] * factor, data[2] * factor, data[3] * factor);
        return *this;
    }

    Quaternion Quaternion::operator/(float factor) const
    {
        Quaternion result(*this);
        result /= factor;
        return result;
    }

    Quaternion& Quaternion::operator/=(float factor)
    {
        setXYZW(data[0] / factor, data[1] / factor, data[2] / factor, data[3] / factor);
        return *this;
    }

    float dotProduct(Quaternion p, Quaternion q)
    {
        return p.x() * q.x() + p.y() * q.y() + p.z() * q.z() + p.w() * q.w();
    }

    Quaternion negate(Quaternion q)
    {
        return Quaternion(-q.x(), -q.y(), -q.z(), -q.w());
    }

    float lengthSquared(Quaternion q)
    {
        return dotProduct(q, q);
    }

    float length(Quaternion q)
    {
        return sqrtf(lengthSquared(q));
    }

    float angle(Quaternion p, Quaternion q)
    {
        return 2*atan2(length(p - q), length(p + q));
    }

    float sinc(float x)
    {
        if (x == 0) return 1;
        return sin(x)/x;
    }

    float recip(float x)
    {
        return 1.0f/x;
    }

    float rsqrt(float x)
    {
        return 1/sqrt(x);
    }

    Quaternion normalize(Quaternion q)
    {
        float length_squared = lengthSquared(q);
        if (length_squared == 0) {
            return Quaternion(0,0,0,1);
        }
        return Quaternion(q * rsqrt(length_squared));
    }

    Quaternion inverse(Quaternion q)
    {

        return Quaternion(q.conjugated().x() / lengthSquared(q),
                        q.conjugated().y() / lengthSquared(q),
                        q.conjugated().z() / lengthSquared(q),
                        q.conjugated().w() / lengthSquared(q));


    }

    Quaternion slerpInternal(Quaternion q0, Quaternion q1, float t)
    {
        float s = 1 - t;
        float a = angle(q0, q1);
        float r = recip(sinc(a));
        return normalize(Quaternion(sinc(s*a)*r*s*q0 + sinc(t*a)*r*t*q1));
    }

    Quaternion qCatmullRom(Quaternion q00, Quaternion q01, Quaternion q02, Quaternion q03, float t)
    {
        Quaternion q10 = IJK::slerpInternal(q00, q01, t + 1.0f);
        Quaternion q11 = IJK::slerpInternal(q01, q02, t);
        Quaternion q12 = IJK::slerpInternal(q02, q03, t - 1.0f);
        Quaternion q20 = IJK::slerpInternal(q10, q11, (t + 1.0f)/2.0f);
        Quaternion q21 = IJK::slerpInternal(q11, q12, t/2.0f);
        return slerpInternal(q20, q21, t);
    }

    Quaternion QuaternionMakeWithMatrix3(float m00, float m01, float m02,
                                            float m10, float m11, float m12,
                                            float m20, float m21, float m22)
    {
        float trace = m00 + m11 + m22;

        float s;

        float x;
        float y;
        float z;
        float w;

        if( trace > 0 )
        {
            s = 0.5f / sqrtf(trace + 1.0f);
            w = 0.25f / s;
            x = ( m21 - m12 ) * s;
            y = ( m02 - m20 ) * s;
            z = ( m10 - m01 ) * s;
        }
        else
        {
            if ( m00 > m11 && m00 > m22 )
            {
                s = 2.0f * sqrtf( 1.0f + m00 - m11 - m22);
                w = (m21 - m12 ) / s;
                x = 0.25f * s;
                y = (m01 + m10 ) / s;
                z = (m02 + m20 ) / s;
            }
            else if (m11 > m22)
            {
                s = 2.0f * sqrtf( 1.0f + m11 - m00 - m22);
                w = (m02 - m20 ) / s;
                x = (m01 + m10 ) / s;
                y = 0.25f * s;
                z = (m12 + m21 ) / s;
            }
            else
            {
                s = 2.0f * sqrtf( 1.0f + m22 - m00 - m11 );
                w = (m10 - m01 ) / s;
                x = (m02 + m20 ) / s;
                y = (m12 + m21 ) / s;
                z = 0.25f * s;
            }
        }

        return Quaternion(x, y, z, w);
    }

    Quaternion fromAxes(IJK::Vector xAxis, IJK::Vector yAxis, IJK::Vector zAxis)
    {
        return QuaternionMakeWithMatrix3(xAxis.x(), yAxis.x(), zAxis.x(),
                                        xAxis.y(), yAxis.y(), zAxis.y(),
                                        xAxis.z(), yAxis.z(), zAxis.z());
    }

    Quaternion QuaternionMakeWithMatrix3AppleOrder(float m00, float m01, float m02,
                                                        float m10, float m11, float m12,
                                                        float m20, float m21, float m22)
    {
        return IJK::QuaternionMakeWithMatrix3(m00, m10, m20, m01, m11, m21, m02, m12, m22);
    }

    std::array<std::array<float, 3>, 3> matrix3x3MakeWithQuaternion(const Quaternion &q)
    {
        const Quaternion normalizedQ = IJK::normalize(q);

        float x = normalizedQ.x();
        float y = normalizedQ.y();
        float z = normalizedQ.z();
        float w = normalizedQ.w();

        float _2x = x + x;
        float _2y = y + y;
        float _2z = z + z;
        float _2w = w + w;

        std::array<std::array<float, 3>, 3> result = {{{1.0f - _2y * y - _2z * z, _2x * y - _2w * z, _2x * z + _2w * y},
                                                        {_2x * y + _2w * z, 1.0f - _2x * x - _2z * z, _2y * z - _2w * x},
                                                        {_2x * z - _2w * y, _2y * z + _2w * x, 1.0f - _2x * x - _2y * y}}};
        return result;
    }
}

IJK::Quaternion operator*(float factor, const IJK::Quaternion& rotation)
{
    IJK::Quaternion result(rotation);
    result *= factor;

    return result;
}

IJK::Quaternion operator/(float factor, const IJK::Quaternion& rotation)
{
    IJK::Quaternion result(rotation);
    result /= factor;

    return result;
}

IJK::Quaternion operator-(const IJK::Quaternion& rotation)
{
    return IJK::Quaternion(-rotation.x(), -rotation.y(), -rotation.z(), -rotation.w());
}
