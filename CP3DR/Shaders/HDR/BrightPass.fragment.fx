#ifdef OPENGL_DRIVER

uniform sampler2D ColorMapSampler;

void main()
{
	gl_FragColor = texture2D(ColorMapSampler, gl_TexCoord[0].xy);
}

#else

#define POST_PROCESS
#include "Shaders/InternalHandler/Utils.hlsl.fx"

CP3DTexture ColorMapSampler : registerTexture(t0);
SamplerState ColorMapSamplerST : register(s0);

float2 dsOffsets[4];
float brightThreshold;

float4 pixelMain(VS_OUTPUT In) : COLOR0 {
	float4 average = float4(0.0, 0.0, 0.0, 0.0);

	for (int i = 0; i < 4; i++) {
		average += CP3DTex2D(ColorMapSampler, In.TexCoords.xy + float2(dsOffsets[i].x, dsOffsets[i].y), ColorMapSamplerST);
	}

	average *= 0.25;

	float luminance = length(average.rgb);

	if (luminance < brightThreshold) {
		average = float4(0.0, 0.0, 0.0, 1.0);
	}

	return average;
}

#endif
