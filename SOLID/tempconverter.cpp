#include "tempconverter.h"

tempconverter::tempconverter(QObject *parent)
    : QObject{parent}
{}

int tempconverter::celsiusToFahrenheit(int celsius)
{
    auto outputfahrenheit = (celsius * 9.0 / 5.0) + 32;
    return outputfahrenheit;
}

int tempconverter::fahrenheitToCelsius(int fahrenheit)
{
    // Note: Parentheses (f - 32) are required to ensure subtraction happens first.
    auto outputcelsius = (fahrenheit - 32) * 5.0 / 9.0;
    return outputcelsius;
}
