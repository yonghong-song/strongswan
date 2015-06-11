/*
 * Copyright (C) 2015 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "test_suite.h"

#include <config/ike_cfg.h>

static void assert_my_family(int expected, char *my_addr)
{
	ike_cfg_t *cfg;
	int family;

	cfg = ike_cfg_create(IKEV2, FALSE, FALSE, my_addr, 500, "%any", 500,
						 FRAGMENTATION_NO, 0);
	family = ike_cfg_my_family(cfg);
	ck_assert_msg(expected == family, "expected family %d != %d (addr: '%s')",
				  expected, family, my_addr);
	cfg->destroy(cfg);
}

START_TEST(test_my_address_family_empty)
{
	assert_my_family(AF_UNSPEC, "");
}
END_TEST

START_TEST(test_my_address_family_addr)
{
	assert_my_family(AF_INET, "192.168.1.1");
	assert_my_family(AF_INET6, "fec::1");
}
END_TEST

START_TEST(test_my_address_family_multi)
{
	assert_my_family(AF_INET, "192.168.1.1,192.168.2.2");
	assert_my_family(AF_INET6, "fec::1,fec::2");

	assert_my_family(AF_UNSPEC, "192.168.1.1,fec::1");
	assert_my_family(AF_UNSPEC, "fec::1,192.168.1.1");
}
END_TEST

START_TEST(test_my_address_family_any)
{
	assert_my_family(AF_UNSPEC, "%any");

	assert_my_family(AF_INET, "%any4");
	assert_my_family(AF_INET, "0.0.0.0");

	assert_my_family(AF_INET6, "%any6");
	assert_my_family(AF_INET6, "::");

	assert_my_family(AF_INET, "192.168.1.1,%any");
	assert_my_family(AF_INET, "192.168.1.1,%any4");
	assert_my_family(AF_UNSPEC, "192.168.1.1,%any6");

	assert_my_family(AF_INET6, "fec::1,%any");
	assert_my_family(AF_UNSPEC, "fec::1,%any4");
	assert_my_family(AF_INET6, "fec::1,%any6");
}
END_TEST

START_TEST(test_my_address_family_other)
{
	assert_my_family(AF_INET, "192.168.1.0");
	assert_my_family(AF_UNSPEC, "192.168.1.0/24");
	assert_my_family(AF_UNSPEC, "192.168.1.0-192.168.1.10");

	assert_my_family(AF_INET, "192.168.1.0/24,192.168.2.1");
	assert_my_family(AF_INET, "192.168.1.0-192.168.1.10,192.168.2.1");
	assert_my_family(AF_INET6, "192.168.1.0/24,fec::1");
	assert_my_family(AF_INET6, "192.168.1.0-192.168.1.10,fec::1");

	assert_my_family(AF_INET6, "fec::");
	assert_my_family(AF_UNSPEC, "fec::/64");
	assert_my_family(AF_UNSPEC, "fec::1-fec::10");

	assert_my_family(AF_INET6, "fec::/64,fed::1");
	assert_my_family(AF_INET6, "fec::1-fec::10,fec::1");
	assert_my_family(AF_INET, "fec::/64,192.168.1.1");
	assert_my_family(AF_INET, "fec::1-fec::10,192.168.1.1");
}
END_TEST

Suite *ike_cfg_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("ike_cfg");

	tc = tcase_create("ike_cfg_my_address_family");
	tcase_add_test(tc, test_my_address_family_empty);
	tcase_add_test(tc, test_my_address_family_addr);
	tcase_add_test(tc, test_my_address_family_multi);
	tcase_add_test(tc, test_my_address_family_any);
	tcase_add_test(tc, test_my_address_family_other);
	suite_add_tcase(s, tc);

	return s;
}
