#include "libcmdline/cmdline.h"

#include <iostream>

// Something
void mytest(int a, char b)
{
}

int main(int argc, char** argv)
{
	cmdline::Parser parser;
	parser.setHelp("This is an example command line program");
	auto& paramTest = parser.addArgument("test").setDescription("Test parameter");
	auto& paramOpt1 = parser.addOption("option", 'o').setDescription("Some optional option");
	auto& paramOpt2 = parser.addOption("a-very-very-long-option-name-that-will-exceed-length", 'x').setDescription("Some description");
	auto& switch1 = parser.addSwitch("mode-1");
	auto& depOpt1 = parser.addOption("dep").setPred(cmdline::enableWhenSwitchIsSet(switch1));

	auto result = parser.parse(argc, argv);

	if (parser.helpRequested())
	{
		std::cout << parser.getHelp();
		return 0;
	}

	if (!result)
	{
		std::cout << result.errorStr() << std::endl;
		return 1;
	}

	std::cout << "Test: " << paramTest.value << std::endl;
	std::cout << "Option: " << paramOpt1.value << std::endl;

	return 0;
}
