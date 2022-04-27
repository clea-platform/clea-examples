//
// Created by harlem88 on 25/08/20.
//

#include <limits.h>
#include "unity.h"
#include "../include/ddcmp/DDCMPMsg.hpp"

TEST_CASE("Compare two unit_8 array with size ->true", "[ddcmp_compare]"){
    const uint8_t l[] = { 0, 1 };
    const uint8_t r[] = { 0, 1 };
    bool result = DDCMPMsg::CompareMsg(l, r, 2);

    TEST_ASSERT_TRUE(result);
}

TEST_CASE("Compare two unit_8 array with size ->false", "[ddcmp_compare]"){
    const uint8_t l[] = { 0, 3 };
    const uint8_t r[] = { 0, 1 };
    bool result = DDCMPMsg::CompareMsg(l, r, 2);

    TEST_ASSERT_FALSE(result);
}

TEST_CASE("Compare two unit_8 array with size ->false", "[ddcmp_compare]"){
    const uint8_t l[] = { 0, 3 };
    const uint8_t r[] = { 0 };
    bool result = DDCMPMsg::CompareMsg(l, r, 2);

    TEST_ASSERT_FALSE(result);
}