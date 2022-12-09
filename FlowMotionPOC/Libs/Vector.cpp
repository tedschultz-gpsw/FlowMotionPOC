//
//  Quaternion.cpp
//
//  Created by Kyler on 8/6/19.
//  Copyright © 2019 GoPro. All rights reserved.

#include "Vector.hpp"
#include <math.h>

using namespace IJK;

Vector::Vector()
{
    data[0] = data[1] = data[2] = 0;
}

Vector::Vector(float x, float y, float z)
{
    data[0] = x;
    data[1] = y;
    data[2] = z;
}

float Vector::x() const
{
    return data[0];
}

float Vector::y() const
{
    return data[1];
}

float Vector::z() const
{
    return data[2];
}

void Vector::setXYZ(float x, float y, float z)
{
    data[0] = x;
    data[1] = y;
    data[2] = z;
}

void Vector::setX(float x)
{
    data[0] = x;
}

void Vector::setY(float y)
{
    data[1] = y;
}

void Vector::setZ(float z)
{
    data[2] = z;
}

Vector Vector::normalized() const
{
    float lengthSquared = data[0] * data[0] + data[1] * data[1] + data[2] * data[2];
    
    if (lengthSquared == 0) {
        return Vector(0,0,0);
    }
    return Vector(data[0] * (float)(1/sqrt(lengthSquared)),
                  data[1] * (float)(1/sqrt(lengthSquared)),
                  data[2] * (float)(1/sqrt(lengthSquared)));
}

float Vector::length()
{
    float lengthSquared = data[0] * data[0] + data[1] * data[1] + data[2] * data[2];
    return sqrtf(lengthSquared);
}

bool Vector::isZero() {
    auto result = (data[0] == 0.0 && data[1] == 0.0 && data[2] == 0.0);
    return result;
}

Vector Vector::operator-(const IJK::Vector& v) const
{
    return IJK::Vector(data[0]-v.x(), data[1]-v.y(), data[2]-v.z());
}

Vector IJK::crossProduct(Vector v1, Vector v2)
{
    return Vector((v1.y() * v2.z()) - (v1.z() * v2.y()),
                  (v1.z() * v2.x()) - (v1.x() * v2.z()),
                  (v1.x() * v2.y()) - (v1.y() * v2.x()));
}

float IJK::dotProduct(Vector v1, Vector v2)
{
    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}

Vector IJK::operator-(const Vector& v)
{
    return Vector(-v.x(), -v.y(), -v.z());
}