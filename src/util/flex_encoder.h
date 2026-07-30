// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Vincent Robinson <robinsonvincent89@gmail.com>

#pragma once

#include "irrlichttypes.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

class FlexEncoder
{
private:
	std::string &m_data;

public:
	FlexEncoder() = default;
	FlexEncoder(std::string &data) : m_data(data) {}

	std::string &data() { return m_data; }
	const std::string &data() const { return m_data; }
};

class FlexDecoder
{
private:
	std::string_view m_data;

public:
	FlexDecoder() = default;
	FlexDecoder(std::string_view data) : m_data(data) {}

	std::string_view data() const { return m_data; }

	bool nextBegin() const;
	bool nextEnd() const;
	bool nextKey() const;
	bool nextInt() const;
	bool nextUInt() const { return nextInt(); }
	bool nextStr() const;

private:
	u8 peek() const { return m_data.empty() ? 0 : (u8)m_data.front(); }
	std::string_view peek(size_t n) const { return m_data.substr(0, n); }
};
