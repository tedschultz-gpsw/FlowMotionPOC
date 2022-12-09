//
//  Quaternion.hpp
//
//  Created by Kyler on 8/6/19.
//  Copyright © 2019 GoPro. All rights reserved.

#pragma once

#include <stdio.h>
#include <array>
#include "Vector.hpp"

namespace IJK
{
    
    class Quaternion
    {
        float data[4];
    public:
        Quaternion();
        Quaternion(float x, float y, float z, float w);
        Quaternion(Vector axis, float radians);

        float x() const;
        float y() const;
        float z() const;
        float w() const;
        Vector vector();
        Vector rotatedVector(Vector v) const;
        Quaternion conjugated() const;
        float norm() const;
        float magnitude() const;
        Quaternion scale(float s) const;
        Quaternion UnitQuaternion();
        Quaternion toAxeAngle();
        Quaternion inverse() const;
        Quaternion inverted() const;
        Quaternion normalized() const;
        bool isZero();

        void setXYZW(float x, float y, float z, float w);
        void setX(float x);
        void setY(float y);
        void setZ(float z);
        void setW(float w);

        Quaternion operator+(const Quaternion& rotation) const;
        Quaternion& operator+=(const Quaternion& rotation);
        Quaternion operator-(const Quaternion& rotation) const;
        Quaternion& operator-=(const Quaternion& rotation);
        Quaternion operator*(const Quaternion& rotation) const;
        Quaternion& operator*=(const Quaternion& rotation);
        Quaternion operator/(const Quaternion& rotation) const;
        Quaternion& operator/=(const Quaternion& rotation);

        Quaternion operator*(float factor) const;
        Quaternion& operator*=(float factor);
        Quaternion operator/(float factor) const;
        Quaternion& operator/=(float factor);
    };

    float dotProduct(Quaternion p, Quaternion q);
    
    Quaternion negate(Quaternion q);
    Quaternion normalize(Quaternion q);
    Quaternion inverse(Quaternion q);
    float lengthSquared(Quaternion q);
    float length(Quaternion q);
    float angle(Quaternion p, Quaternion q);
    float sinc(float x);
    float recip(float x);
    float rsqrt(float x);

    Quaternion slerpInternal(Quaternion q0, Quaternion q1, float t);
    Quaternion qCatmullRom(Quaternion q00, Quaternion q01, Quaternion q02, Quaternion q03, float t);
    
    Quaternion QuaternionMakeWithMatrix3(float m00, float m01, float m02,
                                         float m10, float m11, float m12,
                                         float m20, float m21, float m22);

    Quaternion fromAxes(IJK::Vector xAxis, IJK::Vector yAxis, IJK::Vector zAxis);
    
    Quaternion QuaternionMakeWithMatrix3AppleOrder(float m00, float m01, float m02,
                                                   float m10, float m11, float m12,
                                                   float m20, float m21, float m22);
    
    std::array<std::array<float, 3>, 3> matrix3x3MakeWithQuaternion(const Quaternion &q);

}

IJK::Quaternion operator*(float factor, const IJK::Quaternion& rotation);
IJK::Quaternion operator/(float factor, const IJK::Quaternion& rotation);
IJK::Quaternion operator-(const IJK::Quaternion& rotation);
