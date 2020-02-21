#version 420 //Version of OpenGL we're using. - 4.2

layout(binding = 0) uniform sampler2D uTexScene;
layout(binding = 1) uniform sampler2D uTexDepth;

layout(std140, binding = 4) uniform Resolution
{
	uniform vec2 uResolution;
	uniform vec2 uPixelSize;
};

uniform float uBlurClamp = 1.05;  // max blur amount
uniform float uBias = 0.06; //aperture - bigger values for shallower depth of field
uniform float uFocus = 0.8f;  // this is the depth to focus at
uniform float uThreshold = 10.0f;
 
in vec2 texcoord;
out vec4 outColor;

void main()
{
	vec2 texOffset = gl_FragCoord.xy * uPixelSize;
	float aspectRatio = uResolution.x/uResolution.y;
	
	float depth = texture(uTexDepth, texOffset).r;
	float factor = abs(depth - uFocus);

	
	vec2 dofblur = vec2(clamp((factor * uBias - uThreshold), 0.0, uBlurClamp), 
	                    clamp((factor * uBias - 0.01), 0.0, uBlurClamp)) * aspectRatio;
			
	vec4 col = vec4(0, 0, 0, 1);
			
	col.rgb += texture(uTexScene, texOffset + vec2(- 4.0 * dofblur.x, 0.0)).rgb * 0.06;
	col.rgb += texture(uTexScene, texOffset + vec2(- 3.0 * dofblur.x, 0.0)).rgb * 0.09;
	col.rgb += texture(uTexScene, texOffset + vec2(- 2.0 * dofblur.x, 0.0)).rgb * 0.12;
	col.rgb += texture(uTexScene, texOffset + vec2(-       dofblur.x, 0.0)).rgb * 0.15;
	col.rgb += texture(uTexScene, texOffset                               ).rgb * 0.16;
	col.rgb += texture(uTexScene, texOffset + vec2(        dofblur.y, 0.0)).rgb * 0.15;
	col.rgb += texture(uTexScene, texOffset + vec2(  2.0 * dofblur.y, 0.0)).rgb * 0.12;
	col.rgb += texture(uTexScene, texOffset + vec2(  3.0 * dofblur.y, 0.0)).rgb * 0.09;
	col.rgb += texture(uTexScene, texOffset + vec2(  4.0 * dofblur.y, 0.0)).rgb * 0.06;
	               
	outColor = col;
}