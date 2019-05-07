/* SIE CONFIDENTIAL
 * PlayStation(R)4 Programmer Tool Runtime Library Release 05.008.001
 * Copyright (C) 2016 Sony Interactive Entertainment Inc.
 * All Rights Reserved.
 */
#ifndef FONT_MEMORY_MSPACE_H__
#define FONT_MEMORY_MSPACE_H__

#include <stdint.h>
#include <sce_font.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

SceFontMemory* fontMemoryCreateByMspace( void* memAddress, uint32_t memSizeByte );
SceFontMemory* fontMemoryCreateByMspaceAligned256( void* memAddress, uint32_t memSizeByte );

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* FONT_MEMORY_MSPACE_H__ */
