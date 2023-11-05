#version 330 core

uniform sampler2D diffuseTex;
uniform float alphaThreshold;

in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColour;
void main(void) {	
	vec4 colour = texture(diffuseTex, IN.texCoord);
	if (alphaThreshold != 0 && colour.a < alphaThreshold) {
		discard;
	}
	fragColour = colour;
}