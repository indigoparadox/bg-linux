/*
 *  oneit_lite.c, tiny one-process init replacement.
 *
 *  Almost all of this code is derived from either oneit.c 
 *  from toybox 0.1.0 or from cttyhack.c from busybox 1.10.1.
 *
 *
 *  Copyright (C) 2005, 2007 by Rob Landley <rob@landley.net>.
 *  Copyright (C) 2007 Denys Vlasenko <vda.linux@googlemail.com>
 *  Copyright (C) 2011 Bodo Giannone <Bodo@Giannone.de>
 *
 *
 *  Version History:
 *
 *  (c) 01 Jan 2010: version 1.0 by Bodo Giannone <Bodo@Giannone.de>
 *
 *
 *  It does compile also with uClibc (a very small libc). 
 +  http://www.uclibc.org
 *
 *  Of course You can compress the binary with UPX,
 *  The Ultimate Packer for eXecutables.
 *  http://upx.sourceforge.net
 *
 **************************************************************************
 *                                                                        *
 *  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program; if not, write to the Free Software           *
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.             *
 *                                                                        *
 **************************************************************************/

/*
 * TODO:
 *
 */

/*
 *  usage: oneit_lite command [...]
 *
 *  A simple init program that runs a single supplied command line with a
 *  controlling tty (so CTRL-C can kill it).
 *
 *  The oneit_lite command runs the supplied command line as a child process 
 *  (because PID 1 has signals blocked), attached to /dev/tty1, in its
 *  own session.  Then oneit reaps zombies until the child exits, at
 *  which point it powers off the system.
 *
 */

// The minimum amount of work necessary to get ctrl-c and such to work is:
//
// - Fork a child (PID 1 is special: can't exit, has various signals blocked).
// - Do a setsid() (so we have our own session).
// - In the child, attach stdio to /dev/ttyX (/dev/console is special)
// - Exec the rest of the command line.
//
// PID 1 then reaps zombies until the child process it spawned exits, at which
// point it calls sync() and reboot().  I could stick a kill -1 in there.


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/reboot.h>
#include <unistd.h>


int main(int argc, char **argv)
{
  int i;
  pid_t pid;
  char console[sizeof(int)*3 + 16];

  /* name of this program (argv[0]) */
  static char *progname;
  progname = argv[0];

  // Help message
  if (!*++argv) {
        printf("Usage: %s  <program> [<parameters>]\n", progname);
        exit(1);
  }

  // Create a new child process.
  pid = vfork();
  if (pid) {

    // pid 1 just reaps zombies until it gets its child, then halts the system.
    while (pid!=wait(&i));
    sync();

	// PID 1 can't call reboot() because it kills the task that calls it,
	// which causes the kernel to panic before the actual poweroff happens.
	if (!vfork()) reboot(RB_POWER_OFF);
	sleep(5);
	_exit(1);
  }

  // Redirect stdio to /dev/tty0, with new session ID, so ctrl-c works.
  setsid();
  strcpy(console,"/dev/tty1"); 
  for (i=0; i<3; i++) {
    close(i);
    int fd = open(console, O_RDWR, 0);
  }

  // Can't xexec() here, because we vforked so we don't want to error_exit().
  execvp(argv[0], argv);
  printf("cannot exec '%s'\n", argv[0]);
  _exit(127);
}
