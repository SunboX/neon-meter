#pragma once

#include <stdint.h>

static constexpr uint8_t kFooterLoaderDotCount = 3;
static constexpr uint8_t kFooterLoaderFrameCount = 4;
static constexpr uint8_t kFooterLoaderActiveOpacity = 210;
static constexpr uint8_t kFooterLoaderInactiveOpacity = 55;

/**
 * Returns the opacity for one footer loader dot in a given animation frame.
 */
uint8_t footerLoaderDotOpacity(uint8_t frame, uint8_t dotIndex);
