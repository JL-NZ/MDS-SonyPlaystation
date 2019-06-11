/* SIE CONFIDENTIAL
PlayStation(R)4 Programmer Tool Runtime Library Release 05.008.001
* Copyright (C) 2011 Sony Interactive Entertainment Inc.
* All Rights Reserved.
*/

#ifndef __VSOUTPUT_H__
#define __VSOUTPUT_H__

struct VS_OUTPUT
{
	float4 Position		: S_POSITION;
	float3 Color		: TEXCOORD0;
	float2 UV			: TEXCOORD1;
};

struct VS_TOON_OUTPUT{
	float4 Position		: S_POSITION;
	float3 Color		: TEXCOORD0;
	float2 UV			: TEXCOORD1;
	float3 FragPos;//		: POSITION1;
	float3 FragNormal;//	: POSITION2;
	float3 LightPos;

};

#endif