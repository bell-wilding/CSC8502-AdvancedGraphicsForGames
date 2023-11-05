#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTexA;
uniform sampler2D bumpTexB;
uniform samplerCube cubeTex;

uniform float offsetA;
uniform float offsetB;

uniform float time;

uniform vec4 oceanColour;

uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

uniform vec3 cameraPos;

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

void main(void) {
	vec3 incident	= normalize(lightPos - IN.worldPos);
	vec3 viewDir	= normalize(cameraPos - IN.worldPos);
	vec3 halfDir	= normalize(incident + viewDir);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 bumpA  = texture(bumpTexA, (IN.texCoord * 4) + time / offsetA).rgb;
	vec3 bumpB  = texture(bumpTexB, (IN.texCoord * 4) + time / offsetB).rgb;
	vec3 bumpNormal	= normalize(TBN * normalize((bumpA + bumpB) * 2.0 - 1.0));

	vec3 reflectDir = reflect(-viewDir, normalize(IN.normal));
	vec4 reflectTex = texture(cubeTex, reflectDir);

	float lambert		= max(dot(incident, bumpNormal), 0.0f);
	float distance		= length(lightPos - IN.worldPos);
	float attenuation	= clamp(1.0 - distance * distance / (lightRadius * lightRadius), 0.0, 1.0) * lightColour.w;

	float specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
	specFactor = pow(specFactor, 1000.0);

	vec3 surface = oceanColour.rgb * 0.85 * lightColour.rgb;
	fragColour.rgb = reflectTex.rgb * 0.5;
	fragColour.rgb += surface * lambert * attenuation;
	fragColour.rgb += (lightColour.rgb * specFactor * 25) * attenuation * 0.66;
	fragColour.rgb += (surface * 0.5f);
	fragColour.a = oceanColour.a;
}