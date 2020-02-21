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
			
	vec3 myColor = vec3(0, 0, 0);

	vec2 points[108] = {
		vec2(0, 0.6),
		vec2(-0.03, 0.56),
		vec2(-0.07, 0.472),
		vec2(-0.01, 0.42),
		vec2(-0.08, 0.39),
		vec2(-0.04, 0.37),
		vec2(-0.12, 0.32),
		vec2(-0.01, 0.25),
		vec2(-0.06, 0.21),
		vec2(-0.15, 0.21),
		vec2(-0.24, 0.21),
		vec2(-0.34, 0.21),
		vec2(-0.43, 0.21),
		vec2(-0.56, 0.21),
		vec2(-0.64, 0.21),
		vec2(-0.56, 0.15),
		vec2(-0.49, 0.17),
		vec2(-0.51, 0.11),
		vec2(-0.42, 0.05),
		vec2(-0.36, 0.14),
		vec2(-0.35, 0.08),
		vec2(-0.29, 0.15),
		vec2(-0.14, 0.15),
		vec2(-0.13, 0.16),
		vec2(-0.22, 0.13),
		vec2(-0.06, 0.11),
		vec2(-0.11, 0.09),
		vec2(-0.28, 0.03),
		vec2(-0.28, 0.05),
		vec2(-0.18, 0),
		vec2(-0.34, 0),
		vec2(-0.02,-0.04),
		vec2(-0.11,-0.04),
		vec2(-0.24,-0.09),
		vec2(-0.13,-0.11),
		vec2(-0.02,-0.14),
		vec2(-0.27,-0.17),
		vec2(-0.19,-0.18),
		vec2(-0.13,-0.2),
		vec2(-0.06,-0.23),
		vec2(-0.11,-0.29),
		vec2(-0.25,-0.27),
		vec2(-0.32,-0.28),
		vec2(-0.09,-0.34),
		vec2(-0.2, -0.32),
		vec2(-0.19,-0.41),
		vec2(-0.32,-0.29),
		vec2(-0.24,-0.38),
		vec2(-0.3, -0.43),
		vec2(-0.35,-0.38),
		vec2(-0.28,-0.48),
		vec2(-0.35,-0.52),
		vec2(-0.37,-0.49),
		vec2(-0.39,-0.57),
		vec2(0, 0.6),
		vec2(0.03, 0.56),
		vec2(0.07, 0.472),
		vec2(0.01, 0.42),
		vec2(0.08, 0.39),
		vec2(0.04, 0.37),
		vec2(0.12, 0.32),
		vec2(0.01, 0.25),
		vec2(0.06, 0.21),
		vec2(0.15, 0.21),
		vec2(0.24, 0.21),
		vec2(0.34, 0.21),
		vec2(0.43, 0.21),
		vec2(0.56, 0.21),
		vec2(0.64, 0.21),
		vec2(0.56, 0.15),
		vec2(0.49, 0.17),
		vec2(0.51, 0.11),
		vec2(0.42, 0.05),
		vec2(0.36, 0.14),
		vec2(0.35, 0.08),
		vec2(0.29, 0.15),
		vec2(0.14, 0.15),
		vec2(0.13, 0.16),
		vec2(0.22, 0.13),
		vec2(0.06, 0.11),
		vec2(0.11, 0.09),
		vec2(0.28, 0.03),
		vec2(0.28, 0.05),
		vec2(0.18, 0),
		vec2(0.34, 0),
		vec2(0.02,-0.04),
		vec2(0.11,-0.04),
		vec2(0.24,-0.09),
		vec2(0.13,-0.11),
		vec2(0.02,-0.14),
		vec2(0.27,-0.17),
		vec2(0.19,-0.18),
		vec2(0.13,-0.2),
		vec2(0.06,-0.23),
		vec2(0.11,-0.29),
		vec2(0.25,-0.27),
		vec2(0.32,-0.28),
		vec2(0.09,-0.34),
		vec2(0.2, -0.32),
		vec2(0.19,-0.41),
		vec2(0.32,-0.29),
		vec2(0.24,-0.38),
		vec2(0.3, -0.43),
		vec2(0.35,-0.38),
		vec2(0.28,-0.48),
		vec2(0.35,-0.52),
		vec2(0.37,-0.49),
		vec2(0.39,-0.57),
	};

	for(int i=0; i<108; i++)
	{
        vec2 myPoint = points[i];
        myPoint *= myCoc;
        myColor += texture(uTexScene, texOffset + myPoint).rgb;
    }

    myColor.rgb *= 1.0 / 108;

	outColor = vec4(myColor, 1.0);

	              
}