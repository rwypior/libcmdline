#include "libcmdline/cmdline.h"

#include <catch2/catch_all.hpp>

TEST_CASE("Detecting option", "[option]")
{
		REQUIRE(cmdline::Parser::isOption("--option"));
		REQUIRE(cmdline::Parser::isOption("--an-option"));
		REQUIRE(cmdline::Parser::isOption("--option=value"));

		REQUIRE(cmdline::Parser::isOption("option") == false);
		REQUIRE(cmdline::Parser::isOption("this-is--an-option") == false);
}

TEST_CASE("Parsing option name", "[option]")
{
        REQUIRE(cmdline::Parser::getOptionName("--option") == "option");
        REQUIRE(cmdline::Parser::getOptionName("--option=value") == "option");
}

TEST_CASE("Parsing abbreviated option name", "[option]")
{
		REQUIRE(cmdline::Parser::getOptionAbbr("-x") == "x");
		REQUIRE(cmdline::Parser::getOptionAbbr("-xyz") == "xyz");
}

TEST_CASE("Getting name and value", "[option]")
{
		{
			auto p = cmdline::Parser::getNameEqualsValue("--option=value");
			REQUIRE(p.first == "option");
			REQUIRE(p.second == "value");
		}
}
