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
	std::shared_ptr<MyClass[]> var;
	var = std::make_shared<MyClass[]>(10);

	for (size_t i = 0; i < 10; i++)
	{
		var[i].n = i;
		std::cout << var[i].n << std::endl;
	}
}