#include <iostream>

int main(const int argc, char* argv[], char* envp[])
{
	try {
		if (argc > 0)
			std::cout << argv[0] << std::endl;
		else std::cout << "No argv[0] found! That should be impossible!" << std::endl;
		for (auto i{ 0u }; envp[i] != nullptr; ++i)
			std::cout << envp[i] << std::endl;
		return 0;
	} catch (...) {
		std::cout << "An exception occurred!" << std::endl;
		return -1;
	}
}