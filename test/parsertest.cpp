#include "libcmdline/cmdline.h"

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace Catch::Matchers;

TEST_CASE("Parsing arguments", "[parser]")
{
	cmdline::Parser parser;
	auto& qwerty = parser.addArgument("qwerty", "default");
	auto& asdfgh = parser.addArgument("asdfgh");

	REQUIRE(qwerty.value == "default");
	REQUIRE(asdfgh.value == "");

	auto result = parser.parse({"Test application", "Something", "blabla" });
	INFO(result.errorStr());
	REQUIRE(result);
	REQUIRE(qwerty.value == "Something");
	REQUIRE(asdfgh.value == "blabla");
}

TEST_CASE("Parsing required arguments", "[parser]")
{
	cmdline::Parser parser;
	auto& qwerty = parser.addArgument("qwerty");
	auto& asdfgh = parser.addArgument("asdfgh");

	REQUIRE(!parser.parse({"Test application", "Something" }));
	REQUIRE(parser.parse({"Test application", "Something", "blabla" }));
}

TEST_CASE("Parsing required arguments with defaults", "[parser]")
{
	cmdline::Parser parser;
	auto& qwerty = parser.addArgument("qwerty");
	auto& asdfgh = parser.addArgument("asdfgh", "whatever");

	REQUIRE(!parser.parse({"Test application" }));

	REQUIRE(parser.parse({"Test application", "Something" }));
	REQUIRE(qwerty.value == "Something");
	REQUIRE(asdfgh.value == "whatever");
	
	REQUIRE(parser.parse({"Test application", "Something", "blabla" }));
	REQUIRE(qwerty.value == "Something");
	REQUIRE(asdfgh.value == "blabla");
}

TEST_CASE("Parsing options", "[parser]")
{
	cmdline::Parser parser;
	auto& qwerty = parser.addOption("qwerty", 'q', "default");
	auto& asdfgh = parser.addOption("asdfgh");

	REQUIRE(qwerty.value == "default");
	REQUIRE(asdfgh.value == "");

	auto result = parser.parse({"Test application"});
	INFO(result.errorStr());
	REQUIRE(result);
	REQUIRE(qwerty.value == "default");
	REQUIRE(asdfgh.value == "");

	result = parser.parse({"Test application", "--qwerty=42" });
	INFO(result.errorStr());
	REQUIRE(result);
	REQUIRE(qwerty.value == "42");	
	REQUIRE(asdfgh.value == "");
	
	result = parser.parse({"Test application", "--qwerty=43", "--asdfgh=1337" });
	INFO(result.errorStr());
	REQUIRE(result);
	REQUIRE(qwerty.value == "43");
	REQUIRE(asdfgh.value == "1337");

	qwerty.value = ""; // Parsing doesn't reset values if option is not given

	result = parser.parse({"Test application", "--asdfgh=1338" });
	INFO(result.errorStr());
	REQUIRE(result);
	REQUIRE(qwerty.value == "");	
	REQUIRE(asdfgh.value == "1338");
}

TEST_CASE("Parsing options with space", "[parser]")
{
	cmdline::Parser parser;
	auto& qwerty = parser.addOption("qwerty");

	auto result = parser.parse({"Test application", "--qwerty", "1337"});
	INFO(result.errorStr());
	REQUIRE(result);
	REQUIRE(qwerty.value == "1337");
}

TEST_CASE("Parsing options with short syntax", "[parser]")
{
	cmdline::Parser parser;
	auto& qwerty = parser.addOption("qwerty", 'q', "default");

	auto result = parser.parse({"Test application", "-q42"});
	INFO(result.errorStr());
	REQUIRE(result);
	REQUIRE(qwerty.value == "42");
	
	result = parser.parse({"Test application", "-q", "1337"});
	INFO(result.errorStr());
	REQUIRE(result);
	REQUIRE(qwerty.value == "1337");

	result = parser.parse({"Test application", "-q=1234"});
	INFO(result.errorStr());
	REQUIRE(result);
	REQUIRE(qwerty.value == "1234");
}

TEST_CASE("Parsing conditional arguments", "[parser]")
{
	cmdline::Parser parser;
	auto& sw1 = parser.addSwitch("switch");
	auto& arg1 = parser.addArgument("arg").setPred(cmdline::enableWhenSwitchIsSet(sw1));
	
	auto res = parser.parse({"appname", "arg"});
	INFO(res.errorStr());
	REQUIRE(res == false);

	res = parser.parse({"appname", "--switch", "arg"});
	INFO(res.errorStr());
	REQUIRE(res);
}

TEST_CASE("Nonexistent arguments", "[parser]")
{
	cmdline::Parser parser;
	parser.addArgument("aaa");
	auto res = parser.parse({"appname", "arg1", "arg2"});

	REQUIRE_THAT(res.errorStr(), ContainsSubstring("2 positional arguments"));
}

TEST_CASE("Missing arguments", "[parser]")
{
	cmdline::Parser parser;
	parser.addArgument("aaa");
	auto res = parser.parse({"appname"});

	REQUIRE_THAT(res.errorStr(), ContainsSubstring("aaa is required"));
}

TEST_CASE("Optional argument", "[parser]")
{
	cmdline::Parser parser;
	parser.addArgument("aaa", "", cmdline::Req::optional);
	auto res = parser.parse({"appname"});

	REQUIRE(res == true);
}

TEST_CASE("Mixed required and optional argument", "[parser]")
{
	cmdline::Parser parser;
	parser.addArgument("aaa", "");
	parser.addArgument("bbb", "", cmdline::Req::optional);
	auto res = parser.parse({"appname", "aaa"});

	INFO(res.errorStr());
	REQUIRE(res == true);
}

TEST_CASE("Defaults", "[parser]")
{
	cmdline::Parser parser;
	auto& arg = parser.addArgument("aaa", "default val");
	auto res = parser.parse({"appname"});

	REQUIRE(res);
	REQUIRE(arg.value == "default val");
}

TEST_CASE("Command validity", "[parser]")
{
	cmdline::Parser parser;
	parser.addArgument("aaa", "", cmdline::Req::optional);
	parser.addArgument("bbb", "");

	auto res = parser.validateCommand();

	REQUIRE_THAT(res.errorStr(), ContainsSubstring("\"bbb\" cannot be optional"));
}
