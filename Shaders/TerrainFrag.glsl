#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

uniform sampler2D mudDiffuse;
uniform sampler2D mudBump;
uniform sampler2D sandDiffuse;
uniform sampler2D sandBump;
uniform sampler2D rockDiffuse;
uniform sampler2D rockBump;
uniform sampler2D grassDiffuse;
uniform sampler2D grassBump;

uniform float minHeight;
uniform float maxHeight;
uniform float rockThreshold;

uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightDir;
uniform vec3 lightPos;
uniform float lightRadius;

in Vertex {
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

float inverseLerp(float a, float b, float value) {
	return clamp((value - a) / (b - a), 0.0, 1.0);
}

void main(void) {
	vec4 diffuse = vec4(1, 1, 1, 1);
	vec3 bumpNormal = vec3(1, 1, 1);
	float height = inverseLerp(minHeight, maxHeight, IN.worldPos.y);

	float drawStrength = inverseLerp(-0/2, 0/2, height-0);
	diffuse = diffuse * (1 - drawStrength) + texture(mudDiffuse, IN.texCoord) * drawStrength;
	bumpNormal = bumpNormal * (1 - drawStrength) + texture(mudBump, IN.texCoord).rgb * drawStrength;

	drawStrength = inverseLerp(-0.1/2, 0.1/2, height-0.3);
	diffuse = diffuse * (1 - drawStrength) + texture(sandDiffuse, IN.texCoord) * drawStrength;
	bumpNormal = bumpNormal * (1 - drawStrength) + texture(sandBump, IN.texCoord).rgb * drawStrength;

	drawStrength = inverseLerp(-0.15/2, 0.15/2, height-0.75);
	diffuse = diffuse * (1 - drawStrength) + texture(grassDiffuse, IN.texCoord * 2) * drawStrength;
	bumpNormal = bumpNormal * (1 - drawStrength) + texture(grassBump, IN.texCoord * 2).rgb * drawStrength;

	float verticality = (dot(IN.normal, vec3(1, 0, 1)) + 1)/2;
	
	if (verticality >= rockThreshold) {
		drawStrength = inverseLerp(-0.05/2, 0.05/2, height-0.75) * verticality;
		diffuse = diffuse * (1 - drawStrength) + texture(rockDiffuse, IN.texCoord) * drawStrength;
		bumpNormal = bumpNormal * (1 - drawStrength) + texture(rockBump, IN.texCoord).rgb * drawStrength;
	}

	vec3 incident	= normalize(lightPos - IN.worldPos);
	vec3 viewDir	= normalize(cameraPos - IN.worldPos);
	vec3 halfDir	= normalize(incident + viewDir);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	bumpNormal		 = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));

	float diffuseFac = dot(bumpNormal, -incident);

	float lambert		= max(dot(incident, bumpNormal), 0.0f);
	float distance		= length(lightPos - IN.worldPos);
	float attenuation	= clamp(1.0 - distance * distance / (lightRadius * lightRadius), 0.0, 1.0) * lightColour.w;

	float specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
	specFactor = pow(specFactor, 5.0);

	vec3 surface = (diffuse.rgb * 0.65 * lightColour.rgb);
	fragColour.rgb = surface * lambert * attenuation;
	fragColour.rgb += (lightColour.rgb * specFactor) * 0.33;
	fragColour.rgb += surface * 0.45f;
	fragColour.a = diffuse.a;
}