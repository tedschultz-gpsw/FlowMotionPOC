//
//  Vector.hpp
//
//  Created by Kyler on 8/6/19.
//  Copyright © 2019 GoPro. All rights reserved.

#pragma once

#include <stdio.h>

namespace IJK
{
    
    class Vector
    {
        float data[3];
    public:
        Vector();
        Vector(float x, float y, float z);
        
        float x() const;
        float y() const;
        float z() const;
        
        void setXYZ(float x, float y, float z);
        void setX(float x);
        void setY(float y);
        void setZ(float z);
        
        Vector normalized() const;
        float length();
        bool isZero();

        Vector operator-(const IJK::Vector& v) const;
    };
    Vector operator-(const Vector& v);
    Vector crossProduct(Vector v1, Vector v2);
    float dotProduct(Vector v1, Vector v2);
}
