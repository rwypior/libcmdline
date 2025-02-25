#include "libcmdline/cmdline.h"

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace Catch::Matchers;

TEST_CASE("Auto adding help", "[help]")
{
	cmdline::Parser p1;
	REQUIRE(p1.getSwitch("help"));

	cmdline::Parser p2(false);
	REQUIRE(p2.getSwitch("help") == nullptr);
}

TEST_CASE("Requesting help", "[help]")
{
	cmdline::Parser parser;
	parser.addArgument("arg"); // Required argument
	
	auto res = parser.parse({"appname", "--help"});

	REQUIRE(parser.helpRequested());
}

TEST_CASE("Requesting help short syntax", "[help]")
{
	cmdline::Parser parser;
	parser.addArgument("arg"); // Required argument
	
	auto res = parser.parse({"appname", "-?"});

	REQUIRE(parser.helpRequested());
}

TEST_CASE("Help description", "[help]")
{
	cmdline::Parser parser;
	parser.addArgument("arg"); // Required argument
	parser.setHelp("Example description");
	parser.parse({"TestApp"}); // This sets the application name

	auto help = parser.getHelp();

	REQUIRE_THAT(help, StartsWith("Example description"));
	REQUIRE_THAT(help, ContainsSubstring("TestApp [Options] <arg>"));
}

TEST_CASE("Help arguments", "[help]")
{
	cmdline::Parser parser;
	parser.addArgument("arg", "default-value", cmdline::Req::required, "Some desc");
	parser.addArgument("arg-simple");
	parser.addOption("opt", 'o', "1337", cmdline::Req::required, "An option");
	parser.addOption("opt-simple");
	parser.addSwitch("switch", 's', "A switch");
	parser.addSwitch("switch-simple");

	auto help = parser.getHelp();

	REQUIRE_THAT(help, ContainsSubstring("--opt, -o [value]    = An option"));
	REQUIRE_THAT(help, ContainsSubstring("--opt-simple [value]"));
	REQUIRE_THAT(help, ContainsSubstring("--help, -?           = Show help message"));
	REQUIRE_THAT(help, ContainsSubstring("--switch, -s         = A switch"));
	REQUIRE_THAT(help, ContainsSubstring("--switch-simple"));
}
