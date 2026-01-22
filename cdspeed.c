/*
 * Copyright (c) 2026 0l3d <0l3dgit@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include <sys/ioctl.h>
#include <sys/scsiio.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CD_SPEED_UNIT 176

/*
 * Set the read speed of an optical drive using SCSI SET CD SPEED (0xBB).
 */
int
main(int argc, char *argv[])
{
	struct scsireq 	req;
	int 		fd       , factor, speed_kb;

	if (argc != 3) {
		fprintf(stderr, "usage: %s device speed_factor\n", getprogname());
		fprintf(stderr, "example: %s /dev/rcd0c 4\n", getprogname());
		return 1;
	}
	factor = atoi(argv[2]);
	speed_kb = factor * CD_SPEED_UNIT;

	if ((fd = open(argv[1], O_RDWR | O_EXCL)) == -1)
		err(1, "open: %s", argv[1]);

	memset(&req, 0, sizeof(req));

	/*
	 * CDB (Command Descriptor Block) for SET CD SPEED:
	 * cmd[0]: Opcode 0xBB
	 * cmd[2-3]: Logical Unit Read Speed (Big Endian)
	 */
	req.cmd[0] = 0xBB;
	req.cmd[2] = (speed_kb >> 8) & 0xff;
	req.cmd[3] = speed_kb & 0xff;
	req.cmdlen = 12;
	req.timeout = 30000;
	req.flags = SCCMD_ESCAPE;

	if (ioctl(fd, SCIOCCOMMAND, &req) == -1)
		err(1, "ioctl: SCIOCCOMMAND");

	if (factor == 0)
		printf("Drive speed reset to default maximum.\n");
	else
		printf("Drive speed set to %dx (%d KB/s).\n", factor, speed_kb);

	close(fd);
	return 0;
}
