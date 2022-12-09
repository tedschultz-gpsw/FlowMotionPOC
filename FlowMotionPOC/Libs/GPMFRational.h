/*! @file GPMFRational.h
 *
 *  @brief libGPMF
 *
 *  @version 1.0.0
 *
 *  (C) Copyright 2019 GoPro Inc (http://gopro.com/).
 *
 *  Licensed under either:
 *  - Apache License, Version 2.0, http://www.apache.org/licenses/LICENSE-2.0
 *  - MIT license, http://opensource.org/licenses/MIT
 *  at your option.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <iostream>
#include <string>
#include <exception>
#include <cstdlib>
#include <cstdio>
#include <new>

#ifndef GPMF_RATIONAL_H
#define GPMF_RATIONAL_H

typedef struct GPMF_Rational{
    int32_t num;                            // Numerator
    int32_t den;                            // Denominator
} GPMF_Rational, *GPMF_RationalPtr;

  class GPMFRational
    {
    public:
        int32_t numerator, denominator;
        double quotient;
 
        void simplify();
        GPMFRational doubletofraction(double );
 
        GPMFRational();                             //default constructor
        GPMFRational(int32_t );                     //constructor with only numerator
        GPMFRational(int32_t , int32_t );           //constructor with numerator and denominator
        GPMFRational(int32_t , int32_t , int32_t);  //constructor for whole number rationals
        GPMFRational(double );                      //constructor for converting double int32_to GPMFRational
        GPMFRational(const char *);                 //constructor via string
        GPMFRational(const GPMFRational &);         //from existing GPMFRational type object

        operator double();                          //quotient source
 
        void c_display();                           //display of GPMFRational
        char* c_string();                           //puts in a string format
        double real_quotient();                     //real quotient
        int32_t int_quotient();                     //celling rounded-off integer quotient
        GPMFRational reciprocal();                  //return reciprocal of this object
 
        //////OPERATORS FOR TYPE GPMFRational
        GPMFRational operator*(GPMFRational );      //member arithmetic operators with FRACTIONS
        GPMFRational operator/(GPMFRational );
        GPMFRational operator+(GPMFRational );
        GPMFRational operator-(GPMFRational );
        GPMFRational operator+=(GPMFRational );     //compound assignment with FRACTIONS
        GPMFRational operator-=(GPMFRational );
        GPMFRational operator*=(GPMFRational );
        GPMFRational operator/=(GPMFRational );
        bool operator==(GPMFRational );             //comparison operators with FRACTIONS
        bool operator!=(GPMFRational );
        bool operator< (GPMFRational );
        bool operator<=(GPMFRational );
        bool operator> (GPMFRational );
        bool operator>=(GPMFRational );
 
        //////OPERATORS FOR INTEGERS
        friend GPMFRational operator*(GPMFRational , int32_t ); //non-member arithmetic operators with INTS
        friend GPMFRational operator*(int32_t , GPMFRational );
        friend GPMFRational operator/(GPMFRational , int32_t );
        friend GPMFRational operator/(int32_t , GPMFRational );
        friend GPMFRational operator+(GPMFRational , int32_t );
        friend GPMFRational operator+(int32_t , GPMFRational );
        friend GPMFRational operator-(GPMFRational , int32_t );
        friend GPMFRational operator-(int32_t , GPMFRational );
        friend void operator+=(GPMFRational &, int32_t );   //non-member compound arithmetic with INTS
        friend void operator+=(int32_t &, GPMFRational );
        friend void operator-=(GPMFRational &, int32_t );
        friend void operator-=(int32_t &, GPMFRational );
        friend void operator*=(GPMFRational &, int32_t );
        friend void operator*=(int32_t &, GPMFRational );
        friend void operator/=(GPMFRational &, int32_t );
        friend void operator/=(int32_t &, GPMFRational );
        friend bool operator!=(GPMFRational , int32_t);     //comparison operators with INTS
        friend bool operator==(GPMFRational , int32_t);
        friend bool operator< (GPMFRational , int32_t);
        friend bool operator<=(GPMFRational , int32_t);
        friend bool operator> (GPMFRational , int32_t);
        friend bool operator>=(GPMFRational , int32_t);
 
        //////OPERATORS FOR DOUBLES
        friend GPMFRational operator*(GPMFRational , double );  //non-member arithmetic operators with DOUBLES
        friend GPMFRational operator*(double , GPMFRational );
        friend GPMFRational operator/(GPMFRational , double );
        friend GPMFRational operator/(double , GPMFRational );
        friend GPMFRational operator+(GPMFRational , double );
        friend GPMFRational operator+(double , GPMFRational );
        friend GPMFRational operator-(GPMFRational , double );
        friend GPMFRational operator-(double , GPMFRational );
        friend void operator+=(GPMFRational &, double );        //non-member compound arithmetic with DOUBLES
        friend void operator+=(double &, GPMFRational);
        friend void operator-=(GPMFRational &, double );
        friend void operator-=(double &, GPMFRational);
        friend void operator*=(GPMFRational &, double );
        friend void operator*=(double &, GPMFRational);
        friend void operator/=(GPMFRational &, double );
        friend void operator/=(double &, GPMFRational);
        friend bool operator!=(GPMFRational , double);          //comparison operators with DOUBLES
        friend bool operator==(GPMFRational , double);
        friend bool operator< (GPMFRational , double);
        friend bool operator<=(GPMFRational , double);
        friend bool operator> (GPMFRational , double);
        friend bool operator>=(GPMFRational , double);
 
        ////Increment and Decrement Operators
        GPMFRational operator++();
        GPMFRational operator++(int32_t );
        GPMFRational operator--();
        GPMFRational operator--(int32_t );
 
        ////Assignment Operator
        GPMFRational operator=(GPMFRational );
        GPMFRational operator=(int32_t );
        GPMFRational operator=(double );
        GPMFRational operator=(char *);
 
        ////() Operator for object assignment
        void operator()(int32_t );
        void operator()(int32_t , int32_t );
        void operator()(int32_t , int32_t , int32_t );
        bool operator()(char *);
        void operator()(double );
        
private:
        int32_t float2int(float f);
        GPMF_Rational MakeRational(int32_t num, int32_t den);
        int64_t gpmf_gcd(int64_t a, int64_t b);
        int32_t reduce(int32_t *dst_num, int32_t *dst_den, int64_t num, int64_t den, int64_t max);
        double Rational2Double (GPMF_Rational a);
        void Double2Rational(double d, int64_t max,  GPMF_Rational *ratiional);
        unsigned gcd (unsigned m, unsigned n);
    };
 
#endif /* GPMF_RATIONAL_H */



