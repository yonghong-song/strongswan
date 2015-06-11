/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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

#include "utils.h"

#include <unistd.h>
#include <limits.h>
#ifndef WIN32
# include <signal.h>
#endif

#if defined(__linux__) && defined(HAVE_SYS_SYSCALL_H)
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <ctype.h>
# include <sys/syscall.h>
/* This is from the kernel sources.  We limit the length of directory names to
 * 256 as we only use it to enumerate FDs. */
struct linux_dirent64 {
	u_int64_t d_ino;
	int64_t d_off;
	unsigned short	d_reclen;
	unsigned char d_type;
	char d_name[256];
};
#endif /* defined(__linux__) && defined(HAVE_SYS_SYSCALL_H) */

#include <library.h>
#include <collections/enumerator.h>

#ifdef WIN32

#include <threading/mutex.h>
#include <threading/condvar.h>

/**
 * Flag to indicate signaled wait_sigint()
 */
static bool sigint_signaled = FALSE;

/**
 * Condvar to wait in wait_sigint()
 */
static condvar_t *sigint_cond;

/**
 * Mutex to check signaling()
 */
static mutex_t *sigint_mutex;

/**
 * Control handler to catch ^C
 */
static BOOL WINAPI handler(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
			sigint_mutex->lock(sigint_mutex);
			sigint_signaled = TRUE;
			sigint_cond->signal(sigint_cond);
			sigint_mutex->unlock(sigint_mutex);
			return TRUE;
		default:
			return FALSE;
	}
}

/**
 * Windows variant
 */
void wait_sigint()
{
	SetConsoleCtrlHandler(handler, TRUE);

	sigint_mutex = mutex_create(MUTEX_TYPE_DEFAULT);
	sigint_cond = condvar_create(CONDVAR_TYPE_DEFAULT);

	sigint_mutex->lock(sigint_mutex);
	while (!sigint_signaled)
	{
		sigint_cond->wait(sigint_cond, sigint_mutex);
	}
	sigint_mutex->unlock(sigint_mutex);

	sigint_mutex->destroy(sigint_mutex);
	sigint_cond->destroy(sigint_cond);
}

#else /* !WIN32 */

/**
 * Unix variant
 */
void wait_sigint()
{
	sigset_t set;
	int sig;

	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);

	sigprocmask(SIG_BLOCK, &set, NULL);
	sigwait(&set, &sig);
}

#endif

#ifndef HAVE_CLOSEFROM
/**
 * Described in header.
 */
void closefrom(int lowfd)
{
	int maxfd, fd, len;

	/* try to close only open file descriptors on Linux... */
#if defined(__linux__) && defined(HAVE_SYS_SYSCALL_H)
	/* By directly using a syscall we avoid any calls that might be unsafe after
	 * fork() (e.g. malloc()). */
	char buffer[sizeof(struct linux_dirent64)];
	struct linux_dirent64 *entry;
	int dirfd, offset;

	dirfd = open("/proc/self/fd", O_RDONLY);
	if (dirfd != -1)
	{
		while ((len = syscall(SYS_getdents64, dirfd, buffer,
							  sizeof(buffer))) > 0)
		{
			for (offset = 0; offset < len; offset += entry->d_reclen)
			{
				entry = (struct linux_dirent64*)(buffer + offset);
				if (!isdigit(entry->d_name[0]))
				{
					continue;
				}
				fd = atoi(entry->d_name);
				if (fd != dirfd && fd >= lowfd)
				{
					close(fd);
				}
			}
		}
		close(dirfd);
		return;
	}
#else /* !defined(__linux__) || !defined(HAVE_SYS_SYSCALL_H) */
	/* This makes our closefrom() implementation potentially unsafe to call
	 * after fork() due to the allocations (even directly calling opendir()
	 * requires one).  Depends on how the malloc() implementation handles such
	 * situations. */
	char fd_dir[PATH_MAX];

	len = snprintf(fd_dir, sizeof(fd_dir), "/proc/%u/fd", getpid());
	if (len > 0 && len < sizeof(fd_dir) && access(fd_dir, F_OK) == 0)
	{
		enumerator_t *enumerator = enumerator_create_directory(fd_dir);
		if (enumerator)
		{
			char *rel;
			while (enumerator->enumerate(enumerator, &rel, NULL, NULL))
			{
				fd = atoi(rel);
				if (fd >= lowfd)
				{
					close(fd);
				}
			}
			enumerator->destroy(enumerator);
			return;
		}
	}
#endif /* defined(__linux__) && defined(HAVE_SYS_SYSCALL_H) */

	/* ...fall back to closing all fds otherwise */
#ifdef WIN32
	maxfd = _getmaxstdio();
#else
	maxfd = (int)sysconf(_SC_OPEN_MAX);
#endif
	if (maxfd < 0)
	{
		maxfd = 256;
	}
	for (fd = lowfd; fd < maxfd; fd++)
	{
		close(fd);
	}
}
#endif /* HAVE_CLOSEFROM */

/**
 * return null
 */
void *return_null()
{
	return NULL;
}

/**
 * returns TRUE
 */
bool return_true()
{
	return TRUE;
}

/**
 * returns FALSE
 */
bool return_false()
{
	return FALSE;
}

/**
 * nop operation
 */
void nop()
{
}

/**
 * See header
 */
void utils_init()
{
#ifdef WIN32
	windows_init();
#endif
	atomics_init();
	strerror_init();
}

/**
 * See header
 */
void utils_deinit()
{
#ifdef WIN32
	windows_deinit();
#endif
	atomics_deinit();
	strerror_deinit();
}
