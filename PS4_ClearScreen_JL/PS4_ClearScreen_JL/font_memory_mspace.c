/* SIE CONFIDENTIAL
 * PlayStation(R)4 Programmer Tool Runtime Library Release 05.008.001
 * Copyright (C) 2016 Sony Interactive Entertainment Inc.
 * All Rights Reserved.
 */
#include <stdio.h>
#include <mspace.h>
#include <sce_font.h>
#include "font_memory_mspace.h"

static void*fontMalloc( void* object, uint32_t size );
static void fontFree( void* object, void *p );
static void*fontCalloc( void* object, uint32_t nelem, uint32_t size );
static void*fontRealloc( void* object, void *p, uint32_t size );
static void fontMemoryDestroyCallback( SceFontMemory* fontMemory, void*object, void* destroyArg );

static const SceFontMemoryInterface s_fontLibcMspaceInterface = {
	.Malloc  = fontMalloc,
	.Free    = fontFree,
	.Realloc = fontRealloc,
	.Calloc  = fontCalloc,
	.MspaceCreate  = (SceFontMspaceCreateFunction *) 0,
	.MspaceDestroy = (SceFontMspaceDestroyFunction*) 0,
};

static void*fontMalign256( void* object, uint32_t size );

static const SceFontMemoryInterface s_fontLibcMspaceAligned256Interface = {
	.Malloc  = fontMalign256,
	.Free    = fontFree,
	.Realloc = (SceFontReallocFunction*) 0,
	.Calloc  = (SceFontCallocFunction *) 0,
	.MspaceCreate  = (SceFontMspaceCreateFunction *) 0,
	.MspaceDestroy = (SceFontMspaceDestroyFunction*) 0,
};

static void* fontMalloc( void* object, uint32_t size )
{
	return sceLibcMspaceMalloc( (SceLibcMspace)object, size );
}
static void* fontMalign256( void* object, uint32_t size )
{
	return sceLibcMspaceMemalign( (SceLibcMspace)object, 256, size );
}
static void fontFree( void* object, void *p )
{
	sceLibcMspaceFree( (SceLibcMspace)object, p );
}
static void* fontRealloc( void* object, void *p, uint32_t size )
{
	return sceLibcMspaceRealloc( (SceLibcMspace)object, p, size );
}
static void* fontCalloc( void* object, uint32_t n, uint32_t size )
{
	return sceLibcMspaceCalloc( (SceLibcMspace)object, n, size );
}

//E Create the memory definition for font libraries.
//J フォントライブラリ用のメモリ定義の作成
static SceFontMemory* fontMemoryCreateByMspaceWithInterface( void* memAddress, uint32_t memSizeByte,
                                                             const SceFontMemoryInterface* memInterface )
{
	SceLibcMspace mspace;

	//E Prepare the SceLibcMspace for font processing.
	//J フォント処理用のメモリー領域の準備
	mspace = sceLibcMspaceCreate( "font mspace", memAddress, memSizeByte, 0 );

	if ( mspace ) {
		SceFontMemory* fontMemory;

		//E Assign the SceFontMemory structure.
		//J SceFontMemory構造体を確保
		fontMemory = (SceFontMemory*) sceLibcMspaceCalloc( mspace, 1, sizeof(SceFontMemory) );

		if ( fontMemory ) {
			int ret;

			//E Prepare the memory area for font processing.
			//J フォント処理用のメモリー領域の準備
			ret = sceFontMemoryInit( fontMemory, memAddress, memSizeByte, memInterface, (void*)mspace,
			                         fontMemoryDestroyCallback, (void*)0 );
			if ( ret == SCE_FONT_OK ) {
				//E Success
				//J 成功
				return fontMemory;
			}
			//E Error processing
			//J エラー処理

			//E Release the assigned memory to return.
			//J 確保メモリを解放して戻る。
			sceLibcMspaceFree( mspace, (void*)fontMemory );
		}
		//E Release the SceLibcMspace.
		//J SceLibcMspaceを解放
		sceLibcMspaceDestroy( mspace );
	}
	return (SceFontMemory*)0;
}
SceFontMemory* fontMemoryCreateByMspace( void* memAddress, uint32_t memSizeByte )
{
	return fontMemoryCreateByMspaceWithInterface( memAddress, memSizeByte, &s_fontLibcMspaceInterface );
}
SceFontMemory* fontMemoryCreateByMspaceAligned256( void* memAddress, uint32_t memSizeByte )
{
	return fontMemoryCreateByMspaceWithInterface( memAddress, memSizeByte, &s_fontLibcMspaceAligned256Interface );
}

//E Callback to make sceFontMemoryTerm() call.
//J sceFontMemoryTerm()に呼ばせるコールバック
static void fontMemoryDestroyCallback( SceFontMemory* fontMemory, void*object, void* destroyArg )
{
	if ( fontMemory && object ) {
		SceLibcMspace mspace = (SceLibcMspace)object;
		sceLibcMspaceFree( mspace, (void*)fontMemory );
		sceLibcMspaceDestroy( mspace );
	}
	(void)destroyArg;
	return;
}
