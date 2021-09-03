#ifndef IOCTL_CAR_CMD_H
#define IOCTL_CAR_CMD_H

#include <linux/ioctl.h>

struct ioctl_info{
       unsigned long size;
       char buf[128];
};
   
enum {
	CMD_LEFT = 3,
	CMD_RIGHT,
	CMD_FORWARD,
	CMD_BACKWARD,
	CMD_STOP,
	CMD_GARBAGE,
};

#define STOP		"STOP\n"
#define LEFT		"LEFT\n"
#define RIGHT		"RGHT\n"
#define FORWARD		"FWRD\n"
#define BACKWARD	"BKWD\n"
#define GARBAGE		"GABG\n"

#define MAX_CMD_LEN	4
#define MAX_CMD_STR_LEN	5

#define	IOCTL_MAGIC     'G'

#define	PI_CMD_LEFT	_IOW(IOCTL_MAGIC, CMD_LEFT,	struct ioctl_info)
#define	PI_CMD_RIGHT	_IOW(IOCTL_MAGIC, CMD_RIGHT,	struct ioctl_info)
#define	PI_CMD_FORWARD	_IOW(IOCTL_MAGIC, CMD_FORWARD,  struct ioctl_info) 
#define	PI_CMD_BACKWARD	_IOW(IOCTL_MAGIC, CMD_BACKWARD, struct ioctl_info)
#define	PI_CMD_STOP	_IOW(IOCTL_MAGIC, CMD_STOP,	struct ioctl_info)
#define	PI_CMD_GARBAGE	_IOW(IOCTL_MAGIC, CMD_GARBAGE,	struct ioctl_info)

#endif

