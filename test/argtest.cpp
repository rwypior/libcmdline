#include "libcmdline/cmdline.h"

#include <catch2/catch_all.hpp>

TEST_CASE("Argument value", "[argument]")
{
	cmdline::Argument arg("Argument 1");
	REQUIRE(arg.value == "");
	REQUIRE(arg.enabled());	
}

TEST_CASE("Argument default value", "[argument]")
{
	cmdline::Argument arg("Argument 1", "Something");
	REQUIRE(arg.value == "Something");
}

TEST_CASE("Argument enable when switch is set", "[argument]")
{
	cmdline::Switch sw1("sw1");
	sw1.value = "";

	cmdline::Argument arg("Argument 1");
	arg.enablePred = cmdline::enableWhenSwitchIsSet(sw1);
	REQUIRE(arg.enabled() == false);

	sw1.value = "1";
	REQUIRE(arg.enabled() == true);
}

TEST_CASE("Argument custom enable predicate", "[argument]")
{
	int myVar = 0;
	cmdline::Argument arg("Argument 1");
	arg.enablePred = [&myVar](){
		return myVar == 42;
	};

	REQUIRE(arg.enabled() == false);

	myVar = 42;
	REQUIRE(arg.enabled() == true);
}
