#include <iostream>
#include <memory>

class MyClass
{
public:
	MyClass();
	~MyClass();

public:
	int n;
	double d;
	float f;
};

MyClass::MyClass()
	:n(0), d(0.0), f(0.f)
{
}

MyClass::~MyClass()
{
}

int main()
{
	std::cout << sizeof(int) << ", " << sizeof(float) << std::endl;
}