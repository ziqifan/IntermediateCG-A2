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

	vec2 points[61] = {
		vec2(-0.2, 1),
		vec2(   0, 1),
		vec2( 0.2, 1),

		vec2(-0.2, 0.8),
		vec2(   0, 0.8),
		vec2( 0.2, 0.8),

		vec2(-0.5, 0.7),
		vec2(-0.4, 0.6),
		vec2(-0.2, 0.6),
		vec2(   0, 0.6),
		vec2( 0.2, 0.6),
		vec2( 0.4, 0.6),
		vec2( 0.5, 0.7),

		vec2(-0.6, 0.4),
		vec2(-0.4, 0.4),
		vec2(-0.2, 0.4),
		vec2(   0, 0.4),
		vec2( 0.2, 0.4),
		vec2( 0.4, 0.4),
		vec2( 0.6, 0.4),

		vec2(-0.6, 0.2),
		vec2(-0.4, 0.2),
		vec2(-0.2, 0.2),
		vec2(   0, 0.2),
		vec2( 0.2, 0.2),
		vec2( 0.4, 0.2),
		vec2( 0.6, 0.2),

		vec2(-0.6, 0.0),
		vec2(-0.4, 0.0),
		vec2(-0.2, 0.0),
		vec2(   0, 0.0),
		vec2( 0.2, 0.0),
		vec2( 0.4, 0.0),
		vec2( 0.6, 0.0),

		vec2(-0.6,-0.2),
		vec2(-0.4,-0.2),
		vec2(-0.2,-0.2),
		vec2(   0,-0.2),
		vec2( 0.2,-0.2),
		vec2( 0.4,-0.2),
		vec2( 0.6,-0.2),

		vec2(-0.6,-0.4),
		vec2(-0.4,-0.4),
		vec2(-0.2,-0.4),
		vec2(   0,-0.4),
		vec2( 0.2,-0.4),
		vec2( 0.4,-0.4),
		vec2( 0.6,-0.4),

		vec2(-0.5,-0.7),
		vec2(-0.4,-0.6),
		vec2(-0.2,-0.6),
		vec2(   0,-0.6),
		vec2( 0.2,-0.6),
		vec2( 0.4,-0.6),
		vec2( 0.5,-0.7),

		vec2(-0.2,-0.8),
		vec2(   0,-0.8),
		vec2( 0.2,-0.8),

		vec2(-0.2,-1),
		vec2(   0,-1),
		vec2( 0.2,-1),
	};

	for(int i=0; i<61; i++)
	{
        vec2 myPoint = points[i];
        myPoint *= myCoc;
        myColor += texture(uTexScene, texOffset + myPoint).rgb;
    }

    myColor.rgb *= 1.0 / 61;

	outColor = vec4(myColor, 1.0);

	              
}