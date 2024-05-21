OF_GLSL_SHADER_HEADER

uniform sampler2D tex0;
uniform float u_time;
uniform vec2 u_res;

in vec2 texCoordVarying;
out vec4 outputColor;

void main() {
  vec2 st = gl_FragCoord.xy / u_res;
  float cx = 0.5;
  float cy = 0.5;
  float pi = 3.14159;
  // stretch correction, aspect ratio magic
  float ar = u_res.x / u_res.y;
  float dx = abs(st.x - cx);
  float dy = (1.0 / ar) * abs(st.y - cy);
  float d = sqrt(pow(dx, 2.0) + pow(dy, 2.0));
  //
  float z = exp(-5.0 * d) * sin(32.0 * pi * d * u_time * 5.0 * st.y);
  float r = (23.0 / 256.0) * z;
  float g = (202.0 / 256.0) * z;
  float b = (232.0 / 256.0) * z;
  vec4 ga = vec4(r, g, b, 1.0);
  //
  float kc=0.40 + 0.20*sin(2.0 * pi * 0.112 * u_time);
  //
  vec4 cam=texture(tex0, texCoordVarying/u_res);
  //
  outputColor = cam*kc + ga*(1.0-kc);
}
