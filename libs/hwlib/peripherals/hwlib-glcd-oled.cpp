// ==========================================================================
//
// File      : hwlib.cpp
// Part of   : C++ hwlib library for close-to-the-hardware OO programming
// Copyright : wouter@voti.nl 2017-2019
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// ==========================================================================

// The (few) code parts that must be compiled (instead of included)
// are in the header files, enabled only when _HWLIB_ONCE is defined.

#include <cstdint>
//#include "hwlib-xy.hpp"
#include "hwlib.hpp"
//#include "hwlib-glcd-oled.hpp"

const hwlib::xy hwlib::glcd_oled_i2c_128x64_buffered::wsize;
