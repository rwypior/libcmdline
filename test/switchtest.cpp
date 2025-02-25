#include "libcmdline/cmdline.h"

#include <catch2/catch_all.hpp>

TEST_CASE("Switch value", "[option]")
{
	cmdline::Switch sw("Switch");
	REQUIRE(sw.on() == false);
	sw.value = "";
	REQUIRE(sw.on() == false);
	sw.value = "whatever";
	REQUIRE(sw.on() == true);
	sw.value = "";
	REQUIRE(sw.on() == false);
	sw.value = "1";
	REQUIRE(sw.on() == true);
}
