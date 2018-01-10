#include <iostream>

#include "Vector.h"
#include "Field.h"

int main(void)
{
	Vector a(1, 3);
	Vector b(3, 4);
	Vector c, d;

	Field f(5, 5);

	std::cout << "Hello World!" << std::endl;

	std::cout << "A = (" << a.x() << ", " << a.y() << ")" << std::endl;
	std::cout << "B = (" << b.x() << ", " << b.y() << ")" << std::endl;
	std::cout << "C = (" << c.x() << ", " << c.y() << ")" << std::endl;

	c = a + b;

	std::cout << "C = (" << c.x() << ", " << c.y() << ")" << std::endl;

	std::cout << "Distance(A,B) = " << a.distanceTo(b) << std::endl;
	std::cout << "Abs(C) = " << c.abs() << std::endl;

	d = f.wrapCoords(c);

	std::cout << "D = (" << d.x() << ", " << d.y() << ")" << std::endl;

	d = f.unwrapCoords(a, c);

	std::cout << "D = (" << d.x() << ", " << d.y() << ")" << std::endl;

	return 0;
}
