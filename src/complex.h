#pragma once
#include <utility>
#include <iostream>

class ComplexNumber
{
private:
    std::pair<double, double> m_cx{0, 0};

public:
    ComplexNumber(double real, double imaginary)
    : m_cx{real, imaginary}
    {
    }

    friend ComplexNumber operator*(const ComplexNumber& num1, const ComplexNumber& num2);
    friend ComplexNumber conjugate(const ComplexNumber& num);
    friend std::ostream& operator<<(std::ostream& out, const ComplexNumber& num);

    double getReal() const
    {
        return m_cx.first;
    }

    double getImaginary() const
    {
        return m_cx.second;
    }
};

ComplexNumber operator+(const ComplexNumber& num1, const ComplexNumber& num2)
{
    return {num1.getReal() + num2.getReal(), num1.getImaginary() + num2.getImaginary()};
}

ComplexNumber multiplyByReal(const ComplexNumber& cxnum, const double& real)
{
    return {real * cxnum.getReal(), real * cxnum.getImaginary()};
}

ComplexNumber operator*(const ComplexNumber& num1, const ComplexNumber& num2)
{
    return multiplyByReal(num1, num2.getReal()) 
    + multiplyByReal({-num1.getImaginary(), num1.getReal()}, num2.getImaginary());
}

ComplexNumber conjugate(const ComplexNumber& num)
{
    return {num.m_cx.first, -num.m_cx.second};
}

std::ostream& operator<<(std::ostream& out, const ComplexNumber& num)
{
    out << num.getReal() << "+ i" << num.getImaginary();

    return out;
}

double normsquare(const ComplexNumber& num)
{
    return (num * conjugate(num)).getReal();
}
