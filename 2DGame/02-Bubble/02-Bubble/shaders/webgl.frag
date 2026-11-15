#version 300 es
precision mediump float;

uniform vec4 color;
uniform sampler2D tex;

in vec2 texCoordFrag;
out vec4 outColor;

void main()
{
	vec4 texColor = texture(tex, texCoordFrag);
	if (texColor.a < 0.5)
		discard;
	outColor = color * texColor;
}
