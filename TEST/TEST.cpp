#include "pch.h"
#include "CppUnitTest.h"
#include "..\alch_win\init.hpp"
#include "alch_win_unit-tests.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TEST {

	TEST_CLASS(TEST) {
	public:
		/**
		 * USAGE
		 * Simply go to the menu bar "Test -> Test Explorer -> Run All Tests in View"
		 *
		 * HOW TO ADD ADDITIONAL TESTS
		 * 1.	Create a new TEST_METHOD(<test-name>)
		 * 2.	Duplicate a function in alch_win_unit-tests.hpp, change the name to suit the new test.
		 * 3.	Fill in the args container with the exact arguments you would use to call the function.
		 * 4.	Open a pwsh window (not wezterm, it has a bug with detecting stream type) and cd to the debug build dir.
		 * 5.	Run this command, filling in the arguments & text file name:
					".\alch.exe arguments > ..\TEST\expected-test-name.txt"
				Make sure the output is correct by checking the file contents!
		 * 6.	Replace the file::read target in your new function with the name of the expected output file.
		 * 7.	Make sure you're calling the new function in the new TEST_METHOD.
		 * Thats it!
		 */
		TEST_METHOD(Test_Search)
		{
			Assert::AreEqual(test_search(), 0);
		}
		TEST_METHOD(Test_Build)
		{
			Assert::AreEqual(test_build(), 0);
		}
		TEST_METHOD(Test_SmartSearch)
		{
			Assert::AreEqual(test_ssearch(), 0);
		}
		TEST_METHOD(Test_ListAll)
		{
			Assert::AreEqual(test_list_all(), 0);
		}
		TEST_METHOD(Test_SearchExact)
		{
			Assert::AreEqual(test_search_exact(), 0);
		}
		TEST_METHOD(Test_SearchVerbose)
		{
			Assert::AreEqual(test_search_verbose(), 0);
		}
	};
}
