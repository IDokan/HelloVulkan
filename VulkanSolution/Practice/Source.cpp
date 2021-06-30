#include <iostream>

#define ARRAYSIZE(ARR) sizeof(ARR) / sizeof(ARR ## [0])

int main()
{
	int arr[32];

	std::cout << ARRAYSIZE(arr) << std::endl;
}