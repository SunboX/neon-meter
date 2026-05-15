#pragma once

#include "usage_model.h"

#include <stddef.h>

/**
 * Formats the data-backed usage footer text.
 */
void formatUsageFooter(const UsageData *data, int rateGroup, char *buffer, size_t bufferLength);
