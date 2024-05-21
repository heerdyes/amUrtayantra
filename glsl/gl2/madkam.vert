OF_GLSL_SHADER_HEADER

varying vec2 texCoordVarying;

void main()
{
    vec2 texcoord = gl_MultiTexCoord0.xy;

    // here we move the texture coordinates
    texCoordVarying = vec2(texcoord.x, texcoord.y);

    // send the vertices to the fragment shader
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
