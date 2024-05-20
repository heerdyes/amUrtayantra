OF_GLSL_SHADER_HEADER

uniform float u_time;
uniform vec2 u_res;
uniform sampler2D tex0;

in vec2 texCoordVarying;
out vec4 outputColor;

void main() {
  vec4 camtex = texture(tex0, texCoordVarying);
  vec2 st = gl_FragCoord.xy / u_res;
  //
  float cx = 0.5;
  float cy = 0.5;
  float pi = 3.14159;
  float d = sqrt(pow(st.x - cx, 2.0) + pow(st.y - cy, 2.0));
  float z = exp(-5.0 * d) * sin(32.0 * pi * d * u_time * 5.0 * st.y);
  float r = (23.0 / 256.0) * z;
  float g = (202.0 / 256.0) * z;
  float b = (232.0 / 256.0) * z;
  vec4 ga = vec4(r, g, b, 1.0);
  //
  outputColor = mix(ga, camtex, 0.5);
}
