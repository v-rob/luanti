// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Vincent Robinson <robinsonvincent89@gmail.com>

#include "catch.h"
#include "irrlichttypes.h"
#include "util/flex_encoder.h"

#include <string>
#include <string_view>

using namespace std::string_view_literals;

FlexDecoder byteFlex(u8 byte)
{
	static std::string s_str;
	s_str.clear();
	s_str.push_back((char)byte);

	return FlexDecoder(s_str);
}

TEST_CASE("FlexDecoder::FlexDecoder()/data()")
{
	CHECK(FlexDecoder().data() == "");
	CHECK(FlexDecoder("123"sv).data() == "123");
}

TEST_CASE("FlexDecoder::nextBegin()/nextEnd()/nextKey()/nextInt()/nextStr()")
{
	SECTION("every possible byte is recognized as the correct data type")
	{
		CHECK(FlexDecoder().nextEnd());
		CHECK(byteFlex(0b0'0000000).nextEnd());
		CHECK(byteFlex(0b0'1111111).nextBegin());

		for (int byte = 0b0'0000001; byte <= 0b0'1111110; byte++) {
			CHECK(byteFlex(byte).nextKey());
		}
		for (int byte = 0b10'000000; byte <= 0b10'111111; byte++) {
			CHECK(byteFlex(byte).nextInt());
		}
		for (int byte = 0b11'000000; byte <= 0b11'111111; byte++) {
			CHECK(byteFlex(byte).nextStr());
		}
	}

	SECTION("every possible byte is recognized as exactly one data type")
	{
		for (int byte = 0x00; byte <= 0x100; byte++) {
			auto flex = byte == 0x100 ? FlexDecoder() : byteFlex(byte);

			int numTypes = (int)flex.nextBegin() + (int)flex.nextEnd() +
					(int)flex.nextKey() + (int)flex.nextInt() + (int)flex.nextStr();

			CHECK(numTypes == 1);
			CHECK(flex.nextInt() == flex.nextUInt());
		}
	}
}
