#version 330 core

uniform sampler2D sceneTex;

uniform int isVertical;
uniform int perform;
uniform int lastPass;
uniform float time;

in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColor;

const float scaleFactors[7] =
	float[](0.006, 0.061, 0.242, 0.383, 0.242, 0.061, 0.006);

const vec4 blueTint = vec4(0.015, 0.694, 0.964, 1);

void main(void) {
	if (perform == 1) {
		fragColor = vec4(0,0,0,1);

		vec2 delta = vec2(0,0);

		if (isVertical == 1) {
			delta = dFdy(IN.texCoord);
		}
		else {
			delta = dFdx(IN.texCoord);
		}
		for (int i = 0; i < 7; i++) {
			vec2 offset = delta * (i - 3);
			vec2 uv = IN.texCoord;
			uv.y += (cos(uv.x * 13.0 + time * 0.75) * 0.003) * lastPass;
			uv.x += (cos(uv.y * 13.0 + time * 0.75) * 0.006) * lastPass;
			vec4 baseColour = texture2D(sceneTex, uv + offset);
			vec4 tmp = mix(baseColour, blueTint, 0.045);
			fragColor += tmp * scaleFactors[i];
		}
	}
}