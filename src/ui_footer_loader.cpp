#include "ui_footer_loader.h"

/** Returns a one-active-dot loader frame with a blank pause before wraparound. */
uint8_t footerLoaderDotOpacity(uint8_t frame, uint8_t dotIndex) {
    uint8_t normalizedFrame = static_cast<uint8_t>(frame % kFooterLoaderFrameCount);
    if (normalizedFrame >= kFooterLoaderDotCount) return kFooterLoaderInactiveOpacity;
    return dotIndex == normalizedFrame ? kFooterLoaderActiveOpacity : kFooterLoaderInactiveOpacity;
}
