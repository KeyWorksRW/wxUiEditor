// miniz_crc32_wrapper.c — delegates miniz CRC-32 to libdeflate
//
// When miniz is built with USE_EXTERNAL_MZCRC, it declares (but does not
// define) mz_crc32().  This file provides that definition by forwarding to
// libdeflate's hardware-accelerated CRC-32, so we get a single CRC-32
// implementation in the final binary.

#include <libdeflate.h>
#include <miniz.h>

// libdeflate_crc32 takes u32 (uint32_t). On all supported platforms
// unsigned long (mz_ulong) is 32 bits, so the cast is safe.
mz_ulong mz_crc32(mz_ulong crc, const mz_uint8* ptr, size_t buf_len)
{
    return (mz_ulong)libdeflate_crc32((unsigned int)crc, (const void*)ptr, buf_len);
}
