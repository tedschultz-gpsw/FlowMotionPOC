
/*! @file GPMFRational.cpp
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
#include "GPMFTypes.h"
#include "GPMFRational.h"
#include "GPMFTypes.h"
#include <math.h>
#include <climits>

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
double Rational2Double (GPMF_Rational a);
union intfloat32 {
    uint32_t i;
    float    f;
};
using namespace std;

GPMFRational::GPMFRational()                                //default constructor
: numerator(0), denominator(1)
{}

GPMFRational::GPMFRational(int32_t num)                     //constructor for numerator only, simple numbers
: numerator(num), denominator(1)
{}

GPMFRational::GPMFRational(int32_t num, int32_t den)        //
: numerator(num)
{
    den!=0 ? denominator=den : denominator=1;
    
    if(numerator < 0 && denominator < 0)
    {
        numerator *= -1;
        denominator *= -1;
    }
    if(denominator < 0)
    {
        numerator *= -1;
        denominator *= -1;
    }
    simplify();
}

GPMFRational::GPMFRational(int32_t whole, int32_t num, int32_t den) //constructor for whole number rationals
{
    den!=0 ? denominator=den : denominator=1;
    
    numerator = (den*whole)+num;
    simplify();
}

GPMFRational::GPMFRational(const char *s)                               //constructor for string input
{
    sscanf(s, "%d%*c%d",&numerator,&denominator);
    if(denominator == 0)
        denominator = 1;
    
    simplify();
}

GPMFRational::GPMFRational(double num)
{
    *this = doubletofraction(num);
}

GPMFRational::GPMFRational(const GPMFRational &f)
{
    numerator = f.numerator;
    denominator = f.denominator;
    simplify();
}

GPMFRational::operator double()                 //convertion function
{
    return (float)numerator / (float)denominator;
    //return quotient;
    
}

void GPMFRational::c_display()                  //display function
{
    simplify();
    if(denominator != 1)
        cout << numerator << "/" << denominator ;
    else
        cout << numerator ;
}

char* GPMFRational::c_string()
{
    char *s = NULL;
    if(denominator == 1)
        sprintf(s, "%d", numerator);
    else
        sprintf(s, "%d/%d", numerator,denominator);
    return s;
}

double GPMFRational::real_quotient()
{
    return quotient;
}

int32_t GPMFRational::int_quotient()
{
    return (int32_t)quotient;
}

GPMFRational GPMFRational::reciprocal()
{
    GPMFRational temp(denominator, numerator);
    return temp;
}

GPMFRational GPMFRational::doubletofraction(double d)
{
#if 1
    GPMF_Rational g;
    Double2Rational(d, 100, &g);
    
    
    GPMFRational temp(g.num,g.den);
    
    //double d2 = temp;
    
    return temp;
#else
    int32_t num = d;
    int32_t den = 1;
    double dnum;
    if( d-(double)num != 0 )
    {
        uint32_t limit = 0;
        do
        {
            d *= 10;
            num = d;
            den *= 10;
            
            dnum = double(num);
            
            limit += 10;
            if(limit > 100)
                break;
        }while(d != dnum);
        GPMFRational temp(num,den);
        return temp;
    }
    GPMFRational temp2(num);
    return temp2;
#endif
}

/*simplification of a GPMFRational*/
inline void GPMFRational::simplify()
{
    return;
//    int32_t i;
//    for(i=2; i<=denominator; i++)
//    {
//        if(numerator%i == 0)
//        {
//            if(denominator%i == 0)
//            {
//                numerator /= i;
//                denominator /= i;
//                i=1;
//            }
//        }
//    }
//    
//    if(denominator < 0)
//    {
//        numerator *= -1;
//        denominator *= -1;
//    }
//    if(numerator<0 && denominator<0)
//    {
//        numerator *= -1;
//        denominator *= -1;
//    }
//    if(denominator == 0)
//        denominator = 1;
//    
//    quotient = (double)numerator / (double)denominator;
}
/*Arithmetic Operators with rationals*/
GPMFRational GPMFRational::operator*(GPMFRational f)
{
    GPMFRational temp;
    temp.numerator = numerator * f.numerator;
    temp.denominator = denominator * f.denominator;
    return temp;
}
GPMFRational GPMFRational::operator/(GPMFRational f)
{
    GPMFRational temp;
    temp.numerator = numerator * f.denominator;
    temp.denominator = denominator * f.numerator;
    return temp;
}
GPMFRational GPMFRational::operator+(GPMFRational f)
{
    GPMFRational temp;
    int32_t dummy1, dummy2, LCM=1;
    int32_t i=2;
    dummy1 = denominator;
    dummy2 = f.denominator;
    do  //loop for LCM
    {
        if(dummy1%i == 0 || dummy2%i == 0)
        {
            if(dummy1%i == 0)
            {   dummy1 /= i;    }
            if(dummy2%i == 0)
            {   dummy2 /= i;    }
            LCM *= i;
            i=1;
        }
        i++;
    }while((dummy1 != 1) || (dummy2 != 1));
    temp.denominator = LCM;
    temp.numerator = ((LCM/denominator)*numerator) + ((LCM/f.denominator)*f.numerator);
    return temp;
}
GPMFRational GPMFRational::operator-(GPMFRational f)
{
    GPMFRational temp;
    int32_t dummy1, dummy2, LCM=1;
    int32_t i=2;
    dummy1 = denominator;
    dummy2 = f.denominator;
    do  //loop for LCM
    {
        if(dummy1%i == 0 || dummy2%i == 0)
        {
            if(dummy1%i == 0)
            {   dummy1 /= i;    }
            if(dummy2%i == 0)
            {   dummy2 /= i;    }
            LCM *= i;
            i=1;
        }
        i++;
    }while((dummy1 != 1) || (dummy2 != 1));
    temp.denominator = LCM;
    temp.numerator = ((LCM/denominator)*numerator) - ((LCM/f.denominator)*f.numerator);
    return temp;
}
/*Compound Assignment via rationals*/
GPMFRational GPMFRational::operator+=(GPMFRational f)
{
    GPMFRational &temp = *this;
    temp = temp + f;
    temp.simplify();
    return (temp);
}
GPMFRational GPMFRational::operator-=(GPMFRational f)
{
    GPMFRational &temp = *this;
    temp = temp - f;
    temp.simplify();
    return (temp);
}
GPMFRational GPMFRational::operator*=(GPMFRational f)
{
    GPMFRational &temp = *this;
    temp = temp * f;
    temp.simplify();
    return (temp);
}
GPMFRational GPMFRational::operator/=(GPMFRational f)
{
    GPMFRational &temp = *this;
    temp = temp / f;
    temp.simplify();
    return (temp);
}
/*Comparision Operators*/
bool GPMFRational::operator==(GPMFRational f)
{
    if(numerator == f.numerator)
        if(denominator == f.denominator)
            return true;
        else return false;
        else return false;
}
bool GPMFRational::operator!=(GPMFRational f)
{
    if(numerator == f.numerator)
        if(denominator == f.denominator)
            return false;
        else return true;
        else return true;
}
bool GPMFRational::operator< (GPMFRational f)
{
    if(denominator == f.denominator)
    {
        if(numerator < f.numerator)
            return true;
        else return false;
    }
    int32_t num1 = numerator * f.denominator ;
    int32_t num2 = f.numerator * denominator;
    if(num1 < num2)
        return true;
    else return false;
}
bool GPMFRational::operator<=(GPMFRational f)
{
    if(denominator == f.denominator)
    {
        if(numerator <= f.numerator)
            return true;
        else return false;
    }
    int32_t num1 = numerator * f.denominator ;
    int32_t num2 = f.numerator * denominator;
    if(num1 <= num2)
        return true;
    else return false;
}
bool GPMFRational::operator> (GPMFRational f)
{
    if(denominator == f.denominator)
    {
        if(numerator > f.numerator)
            return true;
        else return false;
    }
    int32_t num1 = numerator * f.denominator ;
    int32_t num2 = f.numerator * denominator;
    if(num1 > num2)
        return true;
    else return false;
}
bool GPMFRational::operator>=(GPMFRational f)
{
    if(denominator == f.denominator)
    {
        if(numerator >= f.numerator)
            return true;
        else return false;
    }
    int32_t num1 = numerator * f.denominator ;
    int32_t num2 = f.numerator * denominator;
    if(num1 >= num2)
        return true;
    else return false;
}
//non-member comparison operators with integers
bool operator!=(GPMFRational f, int32_t num)
{
    if(f.quotient != (double)num)
        return true;
    else
        return false;
}
bool operator==(GPMFRational f, int32_t num)
{
    if(f.quotient == (double)num)
        return true;
    else
        return false;
}
bool operator<(GPMFRational f, int32_t num)
{
    if(f.quotient < (double)num)
        return true;
    else
        return false;
}
bool operator<=(GPMFRational f, int32_t num)
{
    if(f.quotient <= (double)num)
        return true;
    else
        return false;
}
bool operator>(GPMFRational f, int32_t num)
{
    if(f.quotient > (double)num)
        return true;
    else
        return false;
}
bool operator>=(GPMFRational f, int32_t num)
{
    if(f.quotient >= (double)num)
        return true;
    else
        return false;
}

//Arithmetic Operators with integers
GPMFRational operator*(GPMFRational f, int32_t num)
{
    GPMFRational temp;
    temp.numerator = f.numerator * num ;
    temp.denominator = f.denominator;
    return temp;
}
GPMFRational operator*(int32_t num, GPMFRational f)
{
    GPMFRational temp;
    temp.numerator = f.numerator * num ;
    temp.denominator = f.denominator;
    return temp;
}
GPMFRational operator/(GPMFRational f, int32_t num)
{
    GPMFRational temp;
    temp.numerator = f.numerator;
    temp.denominator = f.denominator * num ;
    return temp;
}
GPMFRational operator/(int32_t num, GPMFRational f)
{
    GPMFRational temp;
    temp.numerator = f.denominator * num;
    temp.denominator = f.numerator;
    return temp;
}
GPMFRational operator+(GPMFRational f, int32_t num)
{
    if(num!=0)
    {
        GPMFRational temp(num);
        temp = temp + f;
        return temp;
    }
    return (f);
}
GPMFRational operator+(int32_t num, GPMFRational f)
{
    if(num!=0)
    {
        GPMFRational temp(num);
        temp = temp + f;
        return temp;
    }
    return (f);
}
GPMFRational operator-(GPMFRational f, int32_t num)
{
    if(num!=0)
    {
        GPMFRational temp(num);
        temp = f - temp;
        return temp;
    }
    return (f);
}
GPMFRational operator-(int32_t num, GPMFRational f)
{
    if(num!=0)
    {
        GPMFRational temp(num);
        temp = temp - f;
        return temp;
    }
    return (f);
}
//Compound Arithmetic Assignment operator with integers
void operator+=(GPMFRational &f, int32_t num)
{
    if(num!=0)
    {
        GPMFRational temp(num);
        f = temp + f;
        f.simplify();
    }
}
void operator+=(int32_t &num, GPMFRational f)
{
    if(f!=0)
    {
        num = num + f;
    }
}
void operator-=(GPMFRational &f, int32_t num)
{
    if(num!=0)
    {
        GPMFRational temp(num);
        f = f - temp;
        f.simplify();
    }
}
void operator-=(int32_t &num, GPMFRational f)
{
    if(f!=0)
    {
        num = num - f;
    }
}
void operator*=(GPMFRational &f, int32_t num)
{
    if(num!=1)
    {
        GPMFRational temp(num);
        f = temp * f;
        f.simplify();
    }
}
void operator*=(int32_t &num, GPMFRational f)
{
    if(f!=1)
    {
        num = num * f;
    }
}
void operator/=(GPMFRational &f, int32_t num)
{
    if(num!=1)
    {
        GPMFRational temp(num);
        f = f / temp;
        f.simplify();
    }
}
//void operator/=(int32_t num, GPMFRational &f)
//{
//    if(f!=1)
//    {
//        num = num / f;
//    }
//}
//non-member comparision operators with doubles
bool operator!=(GPMFRational f, double d)
{
    if(f.quotient != d)
        return true;
    else
        return false;
}
bool operator==(GPMFRational f, double d)
{
    if(f.quotient == d)
        return true;
    else
        return false;
}
bool operator< (GPMFRational f, double d)
{
    if(f.quotient < d)
        return true;
    else
        return false;
}
bool operator<=(GPMFRational f, double d)
{
    if(f.quotient <= d)
        return true;
    else
        return false;
}
bool operator> (GPMFRational f, double d)
{
    if(f.quotient > d)
        return true;
    else
        return false;
}
bool operator>=(GPMFRational f, double d)
{
    if(f.quotient >= d)
        return true;
    else
        return false;
}
//non-member arithmetic operators with doubles
GPMFRational operator*(GPMFRational f, double num)
{
    GPMFRational op2;
    op2 = op2.doubletofraction(num);
    GPMFRational temp;
    temp = f * op2;
    return temp;
}
GPMFRational operator*(double num , GPMFRational f)
{
    GPMFRational op2;
    op2 = op2.doubletofraction(num);
    GPMFRational temp;
    temp = f * op2;
    return temp;
}
GPMFRational operator/(GPMFRational f, double num )
{
    GPMFRational op2;
    op2 = op2.doubletofraction(num);
    GPMFRational temp;
    temp = f / op2;
    return temp;
}
GPMFRational operator/(double num , GPMFRational f)
{
    GPMFRational op2;
    op2 = op2.doubletofraction(num);
    GPMFRational temp;
    temp = op2 / f;
    return temp;
}
GPMFRational operator+(GPMFRational f, double num )
{
    GPMFRational op2;
    op2 = op2.doubletofraction(num);
    GPMFRational temp;
    temp = f + op2;
    return temp;
}
GPMFRational operator+(double num , GPMFRational f)
{
    GPMFRational op2;
    op2 = op2.doubletofraction(num);
    GPMFRational temp;
    temp = f + op2;
    return temp;
}
GPMFRational operator-(GPMFRational f, double num )
{
    GPMFRational op2;
    op2 = op2.doubletofraction(num);
    GPMFRational temp;
    temp = f - op2;
    return temp;
}
GPMFRational operator-(double num , GPMFRational f)
{
    GPMFRational op2;
    op2 = op2.doubletofraction(num);
    GPMFRational temp;
    temp = op2 - f;
    return temp;
}
void operator+=(GPMFRational &f, double num )   //non-member compound arithmetic with double numS
{
    GPMFRational temp ;
    temp = temp.doubletofraction(num);
    f += temp;
}
void operator+=(double &num, GPMFRational f)
{
    if(f != 0)
    {
        num = num + f;
    }
}
void operator-=(GPMFRational &f, double num )
{
    GPMFRational temp ;
    temp = temp.doubletofraction(num);
    f -= temp;
}
void operator-=(double &num, GPMFRational f)
{
    if(f != 0)
    {
        num = num - f;
    }
}
void operator*=(GPMFRational &f, double num )
{
    GPMFRational temp ;
    temp = temp.doubletofraction(num);
    f *= temp;
}
void operator*=(double &num, GPMFRational f)
{
    if(f != 1)
    {
        num = num * f;
    }
}
void operator/=(GPMFRational &f, double num )
{
    GPMFRational temp ;
    temp = temp.doubletofraction(num);
    f /= temp;
}
void operator/=(double &num, GPMFRational f)
{
    if(f != 1)
    {
        num = num / f;
    }
}

/*INC and DEC Operators*/
GPMFRational GPMFRational::operator++()
{
    *this = *this + 1;
    return (*this);
}
GPMFRational GPMFRational::operator++(int32_t x)
{
    *this = *this + 1;
    return (*this);
}
GPMFRational GPMFRational::operator--()
{
    *this = *this - 1;
    return (*this);
}
GPMFRational GPMFRational::operator--(int32_t x)
{
    *this = *this - 1;
    return (*this);
}

/*Assignment Operator*/
GPMFRational GPMFRational::operator=(const GPMFRational f)  //assignment operator
{
    numerator = f.numerator;
    denominator = f.denominator;
    simplify();
    return (*this);
}
GPMFRational GPMFRational::operator=(int32_t num)
{
    numerator = num;
    denominator = 1;
    return (*this);
}
GPMFRational GPMFRational::operator=(double num)
{
    *this = doubletofraction(num);
    return (*this);
}
GPMFRational GPMFRational::operator=(char *s)
{
    sscanf(s, "%d%*c%d", &numerator,&denominator);
    if(denominator == 0)
        denominator = 1;
    simplify();
    return (*this);
}

/*() Operator for assigning GPMFRational*/
void GPMFRational::operator()(int32_t num)
{ numerator=num; denominator=1; }

void GPMFRational::operator()(int32_t num, int32_t den)
{
    numerator=num;
    if(den!=0)
        denominator=den;
    else denominator=1;
    simplify();
}

void GPMFRational::operator()(int32_t whole, int32_t num, int32_t den)
{
    if(den!=0)
        denominator = den;
    else denominator = 1;
    numerator = (den*whole)+num;
    simplify();
}

void GPMFRational::operator()(double num)
{
    *this = doubletofraction(num);
}

bool GPMFRational::operator()(char *s)
{
    uint8_t slashCount = 0;
    char *startOfstring = s;
    do
    {
        if(*s == '/' || *s == '\\' || *s == ',' || *s == ' ')
            slashCount++;
        s++;
    }while(*s);
    s = startOfstring;
    
    if(slashCount == 0)     //simple number: goes to numerator
    {
        double temp = 0;
        sscanf(s, "%lf", &temp);
        numerator = (int)temp;
        if(temp == (double)numerator)
        {
            numerator = temp;
            denominator = 1;
        }
        else
            *this = doubletofraction(temp);
    }
    else if(slashCount == 1)    //fraction in p/q form
    {
        sscanf(s, "%d%*c%d", &numerator, &denominator);
        if(denominator == 0)
            denominator = 1;
    }
    else if(slashCount == 2)    //whole number for w/p/q
    {
        int32_t whole = 0;
        sscanf(s, "%d%*c%d%*c%d", &whole,&numerator,&denominator);
        if(denominator == 0)
            denominator = 1;
        numerator = (denominator*whole) + numerator;
    }
    else
    {
        numerator = 0;
        denominator = 1;
        return false;
    }
    simplify();
    return true;
}



int32_t GPMFRational::float2int(float f)
{
    union intfloat32 v;
    v.f = f;
    return (int32_t)v.i;
}

GPMF_Rational GPMFRational::MakeRational(int32_t num, int32_t den)
{
    GPMF_Rational r = { num, den };
    return r;
}

int64_t GPMFRational::gpmf_gcd(int64_t a, int64_t b){
    if(b) return gpmf_gcd(b, a%b);
    else  return a;
}

int32_t GPMFRational::reduce(int32_t *dst_num, int32_t *dst_den,
                         int64_t num, int64_t den, int64_t max)
{
    GPMF_Rational a0 = { 0, 1 }, a1 = { 1, 0 };
    int32_t sign = (num < 0) ^ (den < 0);
    int64_t gcd = gpmf_gcd(fabs(num), fabs(den));
    
    if (gcd) {
        num = fabs(num) / gcd;
        den = fabs(den) / gcd;
    }
    if (num <= max && den <= max) {
        a1 = { static_cast<int32_t>(num), static_cast<int32_t>(den) };
        den = 0;
    }
    
    while (den) {
        int64_t x        = num / den;
        int64_t next_den  = num - den * x;
        int64_t a2n       = x * a1.num + a0.num;
        int64_t a2d       = x * a1.den + a0.den;
        
        if (a2n > max || a2d > max) {
            if (a1.num) x =          (max - a0.num) / a1.num;
            if (a1.den) x = FFMIN(x, (max - a0.den) / a1.den);
            
            if (den * (2 * x * a1.den + a0.den) > num * a1.den)
                a1 = { static_cast<int32_t>(x * a1.num + a0.num), static_cast<int32_t>(x * a1.den + a0.den) };
            break;
        }
        
        a0  = a1;
        a1  = { static_cast<int32_t>(a2n), static_cast<int32_t>(a2d) };
        num = den;
        den = next_den;
    }
    
    *dst_num = sign ? -a1.num : a1.num;
    *dst_den = a1.den;
    
    return den == 0;
}

double Rational2Double (GPMF_Rational a){
    return a.num / (double) a.den;
}

void GPMFRational::Double2Rational(double d, int64_t max,  GPMF_Rational *ratiional)
{
    
    uint32_t exponent;
    int64_t den;
    if (isnan(d))
    {
        ratiional->den = ratiional->num = 0;
        return;
    }
    
    frexp(d, (int *)&exponent);
    exponent = FFMAX(exponent-1, 0);
    den = 1LL << (61 - exponent);
    reduce(&ratiional->num, &ratiional->den, floor(d * den + 0.5), den, INT_MAX);
    return;
}

unsigned GPMFRational::gcd (unsigned m, unsigned n) {
    if (m > n)
        return gcd (m-n, n);
    else if (m < n)
        return gcd (m, n-m);
    else
        return m;
}

