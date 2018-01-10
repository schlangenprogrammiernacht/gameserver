#include <iostream>

#include "Vector.h"

int main(void)
{
	Vector a(1, 3);
	Vector b(3, 4);
	Vector c;

	std::cout << "Hello World!" << std::endl;

	std::cout << "A = (" << a.x() << ", " << a.y() << ")" << std::endl;
	std::cout << "B = (" << b.x() << ", " << b.y() << ")" << std::endl;
	std::cout << "C = (" << c.x() << ", " << c.y() << ")" << std::endl;

	c = a + b;

	std::cout << "C = (" << c.x() << ", " << c.y() << ")" << std::endl;

	std::cout << "Distance(A,B) = " << a.distanceTo(b) << std::endl;
	std::cout << "Abs(C) = " << c.abs() << std::endl;

	return 0;
}
