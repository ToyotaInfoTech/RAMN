#ifndef __FORCE_INCLUDE_H
#define __FORCE_INCLUDE_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// Stub out __attribute__ for custom sections that don't exist on Linux
#define __attribute__(x)

#include "main.h"
#include "cmsis_os.h"
#include "stream_buffer.h"

#endif
