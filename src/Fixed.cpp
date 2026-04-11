#include "Fixed.hpp"

/**
 * Constructor
 */
Fixed::Fixed(void)
{
    std::cout << "Default Constructor called\n";

    this->_fixed_point = 0;
}

/**
 * Copy Constructor
 * @other: another Fixed object
 */
Fixed::Fixed(const Fixed &other)
{
    std::cout << "Copy Constructor called\n";
    *this = other;
    return;
}

/**
 * Copy Assignment Operator
 * @other: another Fixed object
 * Return: reference to the current object
 */
Fixed &Fixed::operator=(const Fixed &other) {
    std::cout << "Copy assignement operator called\n";

    if (this != &other)
        this->_fixed_point = other.getRawBits();
    return (*this);
}

/**
 * Destructor
 */
Fixed::~Fixed(void)
{
    std::cout << "Destructor called\n";
}

/**
 * Getter for the raw fixed-point value
 * Return: the raw fixed-point value
 */
int Fixed::getRawBits(void) const
{
    std::cout << "getRawBits member function called\n";

    return (this->_fixed_point);
}

/**
 * Setter for the raw fixed-point value
 * @raw: the raw fixed-point value to set
 */
void Fixed::setRawBits(int const raw)
{
    std::cout << "setRawBits member function called\n";

    this->_fixed_point = raw;
}