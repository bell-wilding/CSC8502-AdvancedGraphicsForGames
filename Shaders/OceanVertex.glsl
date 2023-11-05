#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

uniform float amplitude;
uniform float speed;
uniform float wavelength;
uniform vec2 direction;

uniform float time;

const float PI = 3.141592653589793238;

in vec3 position;
in vec3 normal;
in vec4 tangent;
in vec2 texCoord;

out Vertex {
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} OUT;

void main(void) {
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

	OUT.texCoord	= (textureMatrix * vec4(texCoord, 0.0, 1.0)).xy;

	vec3 wNormal	= normalize(normalMatrix * normalize(normal));
	vec3 wTangent	= normalize(normalMatrix * normalize(tangent.xyz));
	
	OUT.normal		= wNormal;
	OUT.tangent		= wTangent;
	OUT.binormal	= cross(wTangent, wNormal) * tangent.w;

	vec4 worldPos	= (modelMatrix * vec4(position, 1));
	
	float k = 2 * PI / wavelength;
	vec2 waveDir = normalize(direction);
	float f = k * (dot(waveDir, worldPos.xz) - speed * time);
	worldPos.x += waveDir.x * amplitude * cos(f);
	worldPos.z += waveDir.y * amplitude * cos(f);
	worldPos.y += amplitude * sin(f);

	OUT.worldPos	= worldPos.xyz;

	gl_Position		= (projMatrix * viewMatrix) * worldPos;
}