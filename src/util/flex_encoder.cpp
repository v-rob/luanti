// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Vincent Robinson <robinsonvincent89@gmail.com>

#include "flex_encoder.h"

static constexpr u8 BEGIN_TAG = 0b0'1111111;
static constexpr u8 END_TAG = 0b0'0000000;

bool FlexDecoder::nextBegin() const { return peek() == BEGIN_TAG; }
bool FlexDecoder::nextEnd() const { return peek() == END_TAG; }
bool FlexDecoder::nextKey() const { return peek() > END_TAG && peek() < BEGIN_TAG; }
bool FlexDecoder::nextInt() const { return (peek() & 0b11'000000) == 0b10'000000; }
bool FlexDecoder::nextStr() const { return (peek() & 0b11'000000) == 0b11'000000; }
