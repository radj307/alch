#include <iostream>
#include <string>
#include <sysapi.h>

int main()
{
	try {
		for ( std::string sec{ }; std::getline(std::cin, sec); std::cout << sec << std::endl ) {

			std::cout << '\n';
		}
		return 0;
	} catch ( std::exception& ex ) {

		return -1;
	} catch ( ... ) {

		return -2;
	}
}