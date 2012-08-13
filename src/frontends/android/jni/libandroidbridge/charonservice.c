/*
 * Copyright (C) 2012 Tobias Brunner
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

#include <string.h>
#include <android/log.h>
#include <jni.h>

#include <hydra.h>
#include <ipsec.h>
#include <daemon.h>
#include <library.h>

#define JNI_PACKAGE org_strongswan_android

#define JNI_METHOD_PP(pack, klass, name, ret, ...) \
	ret Java_##pack##_##klass##_##name(JNIEnv *env, jobject this, ##__VA_ARGS__)

#define JNI_METHOD_P(pack, klass, name, ret, ...) \
	JNI_METHOD_PP(pack, klass, name, ret, ##__VA_ARGS__)

#define JNI_METHOD(klass, name, ret, ...) \
	JNI_METHOD_P(JNI_PACKAGE, klass, name, ret, ##__VA_ARGS__)

/**
 * hook in library for debugging messages
 */
extern void (*dbg) (debug_t group, level_t level, char *fmt, ...);

/**
 * Logging hook for library logs, using android specific logging
 */
static void dbg_android(debug_t group, level_t level, char *fmt, ...)
{
	va_list args;

	if (level <= 4)
	{
		char sgroup[16], buffer[8192];
		char *current = buffer, *next;
		snprintf(sgroup, sizeof(sgroup), "%N", debug_names, group);
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		while (current)
		{	/* log each line separately */
			next = strchr(current, '\n');
			if (next)
			{
				*(next++) = '\0';
			}
			__android_log_print(ANDROID_LOG_INFO, "charon", "00[%s] %s\n",
								sgroup, current);
			current = next;
		}
	}
}

/**
 * Initialize charon and the libraries via JNI
 */
JNI_METHOD(CharonVpnService, initializeCharon, void)
{
	/* logging for library during initialization, as we have no bus yet */
	dbg = dbg_android;

	/* initialize library */
	if (!library_init(NULL))
	{
		library_deinit();
		return;
	}

	if (!libhydra_init("charon"))
	{
		libhydra_deinit();
		library_deinit();
		return;
	}

	if (!libipsec_init())
	{
		libipsec_deinit();
		libhydra_deinit();
		library_deinit();
		return;
	}

	if (!libcharon_init("charon") ||
		!charon->initialize(charon, PLUGINS))
	{
		libcharon_deinit();
		libipsec_deinit();
		libhydra_deinit();
		library_deinit();
		return;
	}

	/* start daemon (i.e. the threads in the thread-pool) */
	charon->start(charon);
}

/**
 * Initialize charon and the libraries via JNI
 */
JNI_METHOD(CharonVpnService, deinitializeCharon, void)
{
	libcharon_deinit();
	libipsec_deinit();
	libhydra_deinit();
	library_deinit();
}
