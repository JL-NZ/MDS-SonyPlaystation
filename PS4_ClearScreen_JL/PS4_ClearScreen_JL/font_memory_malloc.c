/* SIE CONFIDENTIAL
 * PlayStation(R)4 Programmer Tool Runtime Library Release 05.008.001
 * Copyright (C) 2013 Sony Interactive Entertainment Inc.
 * All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sce_font.h>
#include "font_memory_malloc.h"

static void*fontMalloc( void* object, uint32_t size );
static void fontFree( void* object, void *p );
static void*fontCalloc( void* object, uint32_t n, uint32_t size );
static void*fontRealloc( void* object, void *p, uint32_t size );
static void fontMemoryDestroyCallback( SceFontMemory* fontMemory, void*object, void* destroyArg );

static void* fontMalloc( void* object, uint32_t size )
{
	(void)object;
	return malloc( size );
}
static void fontFree( void* object, void *p )
{
	(void)object;
	free( p );
}
static void* fontRealloc( void* object, void *p, uint32_t size )
{
	(void)object;
	return realloc( p, size );
}
static void* fontCalloc( void* object, uint32_t n, uint32_t size )
{
	(void)object;
	return calloc( n, size );
}

static const SceFontMemoryInterface s_fontLibcMallocInterface = {
	.Malloc  = fontMalloc,
	.Free    = fontFree,
	.Realloc = fontRealloc,
	.Calloc  = fontCalloc,
	.MspaceCreate  = (SceFontMspaceCreateFunction *) 0,
	.MspaceDestroy = (SceFontMspaceDestroyFunction*) 0,
};

//E Create the memory definition for font libraries.
//J フォントライブラリ用のメモリ定義の作成
SceFontMemory* fontMemoryCreateByMalloc()
{
	//E Assign the SceFontMemory structure.
	//J SceFontMemory構造体を確保
	SceFontMemory* fontMemory = (SceFontMemory*)calloc( 1, sizeof(SceFontMemory) );

	if ( fontMemory ) {
		int ret;
		//E Prepare the memory area for font processing.
		//J フォント処理用のメモリー領域の準備
		ret = sceFontMemoryInit( fontMemory, (void*)0, 0,
		                         &s_fontLibcMallocInterface, (void*)0,
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
		free( (void*)fontMemory );
	}
	else {
		printf( "Please set required size as \"global variable:size_t sceLibcHeapSize\"\n" );
	}
	return (SceFontMemory*)0;
}

//E Callback to make sceFontMemoryTerm() call.
//J sceFontMemoryTerm()に呼ばせるコールバック
static void fontMemoryDestroyCallback( SceFontMemory* fontMemory, void*object, void* destroyArg )
{
	(void)object;
	(void)destroyArg;

	if ( fontMemory ) {
		free( (void*)fontMemory );
	}
	return;
}
