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
uniform float uFocalLength = 5.0f;
 
in vec2 texcoord;
out vec4 outColor;

void main()
{
	vec2 texOffset = gl_FragCoord.xy * uPixelSize;
	
	float depth = texture(uTexDepth, texOffset).r;

	float myCoc = abs(uBias * uFocalLength * (uFocus - depth)  / (depth * (uFocus - uFocalLength)));
	myCoc = clamp(myCoc, 0.0, uBlurClamp);
			
	vec3 myColor  = vec3(0, 0, 0);

	vec2 points[180] = {
		vec2(-0.04, 0.62),
		vec2(-0.12, 0.62),
		vec2(-0.18, 0.62),
		vec2(-0.25, 0.62),
		vec2(-0.32, 0.62),
		vec2(-0.04, 0.54),
		vec2(-0.14, 0.55),
		vec2(-0.22, 0.54),
		vec2(-0.32, 0.54),
		vec2(-0.38, 0.52),
		vec2(-0.40, 0.44),
		vec2(-0.33, 0.44),
		vec2(-0.22, 0.44),
		vec2(-0.11, 0.44),
		vec2(-0.02, 0.44),
		vec2(-0.44, 0.38),
		vec2(-0.38, 0.36),
		vec2(-0.24, 0.36),
		vec2(-0.13, 0.36),
		vec2(-0.02, 0.36),
		vec2(-0.49, 0.27),
		vec2(-0.29, 0.27),
		vec2(-0.19, 0.26),
		vec2(-0.09, 0.27),
		vec2(-0.02, 0.27),
		vec2(-0.53, 0.2),
		vec2(-0.46, 0.17),
		vec2(-0.32, 0.17),
		vec2(-0.22, 0.17),
		vec2(-0.11, 0.17),
		vec2(-0.02, 0.14),
		vec2(-0.02, 0.07),
		vec2(-0.01, 0.01),
		vec2(-0.07, 0.01),
		vec2(-0.12, 0.1),
		vec2(-0.22, 0.1),
		vec2(-0.33, 0.1),
		vec2(-0.47, 0.1),
		vec2(-0.58, 0.1),
		vec2(-0.64, 0.01),
		vec2(-0.54, 0.01),
		vec2(-0.43, 0.01),
		vec2(-0.35, 0.01),
		vec2(-0.26, 0.01),
		vec2(-0.16, 0.01),
		vec2(-0.04,-0.62),
		vec2(-0.12,-0.62),
		vec2(-0.18,-0.62),
		vec2(-0.25,-0.62),
		vec2(-0.32,-0.62),
		vec2(-0.04,-0.54),
		vec2(-0.14,-0.55),
		vec2(-0.22,-0.54),
		vec2(-0.32,-0.54),
		vec2(-0.38,-0.52),
		vec2(-0.40,-0.44),
		vec2(-0.33,-0.44),
		vec2(-0.22,-0.44),
		vec2(-0.11,-0.44),
		vec2(-0.02,-0.44),
		vec2(-0.44,-0.38),
		vec2(-0.38,-0.36),
		vec2(-0.24,-0.36),
		vec2(-0.13,-0.36),
		vec2(-0.02,-0.36),
		vec2(-0.49,-0.27),
		vec2(-0.29,-0.27),
		vec2(-0.19,-0.26),
		vec2(-0.09,-0.27),
		vec2(-0.02,-0.27),
		vec2(-0.53,-0.2),
		vec2(-0.46,-0.17),
		vec2(-0.32,-0.17),
		vec2(-0.22,-0.17),
		vec2(-0.11,-0.17),
		vec2(-0.02,-0.14),
		vec2(-0.02,-0.07),
		vec2(-0.01,-0.01),
		vec2(-0.07,-0.01),
		vec2(-0.12,-0.1),
		vec2(-0.22,-0.1),
		vec2(-0.33,-0.1),
		vec2(-0.47,-0.1),
		vec2(-0.58,-0.1),
		vec2(-0.64,-0.01),
		vec2(-0.54,-0.01),
		vec2(-0.43,-0.01),
		vec2(-0.35,-0.01),
		vec2(-0.26,-0.01),
		vec2(-0.16,-0.01),
		vec2(0.04, 0.62),
		vec2(0.12, 0.62),
		vec2(0.18, 0.62),
		vec2(0.25, 0.62),
		vec2(0.32, 0.62),
		vec2(0.04, 0.54),
		vec2(0.14, 0.55),
		vec2(0.22, 0.54),
		vec2(0.32, 0.54),
		vec2(0.38, 0.52),
		vec2(0.40, 0.44),
		vec2(0.33, 0.44),
		vec2(0.22, 0.44),
		vec2(0.11, 0.44),
		vec2(0.02, 0.44),
		vec2(0.44, 0.38),
		vec2(0.38, 0.36),
		vec2(0.24, 0.36),
		vec2(0.13, 0.36),
		vec2(0.02, 0.36),
		vec2(0.49, 0.27),
		vec2(0.29, 0.27),
		vec2(0.19, 0.26),
		vec2(0.09, 0.27),
		vec2(0.02, 0.27),
		vec2(0.53, 0.2),
		vec2(0.46, 0.17),
		vec2(0.32, 0.17),
		vec2(0.22, 0.17),
		vec2(0.11, 0.17),
		vec2(0.02, 0.14),
		vec2(0.02, 0.07),
		vec2(0.01, 0.01),
		vec2(0.07, 0.01),
		vec2(0.12, 0.1),
		vec2(0.22, 0.1),
		vec2(0.33, 0.1),
		vec2(0.47, 0.1),
		vec2(0.58, 0.1),
		vec2(0.64, 0.01),
		vec2(0.54, 0.01),
		vec2(0.43, 0.01),
		vec2(0.35, 0.01),
		vec2(0.26, 0.01),
		vec2(0.16, 0.01),
		vec2(0.04,-0.62),
		vec2(0.12,-0.62),
		vec2(0.18,-0.62),
		vec2(0.25,-0.62),
		vec2(0.32,-0.62),
		vec2(0.04,-0.54),
		vec2(0.14,-0.55),
		vec2(0.22,-0.54),
		vec2(0.32,-0.54),
		vec2(0.38,-0.52),
		vec2(0.40,-0.44),
		vec2(0.33,-0.44),
		vec2(0.22,-0.44),
		vec2(0.11,-0.44),
		vec2(0.02,-0.44),
		vec2(0.44,-0.38),
		vec2(0.38,-0.36),
		vec2(0.24,-0.36),
		vec2(0.13,-0.36),
		vec2(0.02,-0.36),
		vec2(0.49,-0.27),
		vec2(0.29,-0.27),
		vec2(0.19,-0.26),
		vec2(0.09,-0.27),
		vec2(0.02,-0.27),
		vec2(0.53,-0.2),
		vec2(0.46,-0.17),
		vec2(0.32,-0.17),
		vec2(0.22,-0.17),
		vec2(0.11,-0.17),
		vec2(0.02,-0.14),
		vec2(0.02,-0.07),
		vec2(0.01,-0.01),
		vec2(0.07,-0.01),
		vec2(0.12,-0.1),
		vec2(0.22,-0.1),
		vec2(0.33,-0.1),
		vec2(0.47,-0.1),
		vec2(0.58,-0.1),
		vec2(0.64,-0.01),
		vec2(0.54,-0.01),
		vec2(0.43,-0.01),
		vec2(0.35,-0.01),
		vec2(0.26,-0.01),
		vec2(0.16,-0.01),
	};

	for(int i=0; i<180; i++)
	{
        vec2 myPoint = points[i];
        myPoint *= myCoc;
        myColor  += texture(uTexScene, texOffset + myPoint).rgb;
    }

    myColor .rgb *= 1.0 / 180;

	outColor = vec4(myColor , 1.0);	              
}