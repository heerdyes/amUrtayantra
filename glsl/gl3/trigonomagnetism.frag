OF_GLSL_SHADER_HEADER

uniform sampler2D tex0;
uniform float u_time;
uniform vec2 u_res;

in vec2 texCoordVarying;
out vec4 outputColor;

void main() {
  vec2 st = gl_FragCoord.xy / u_res;
  float cx = 0.5 + 0.2 * sin(u_time);
  float cy = 0.5 + 0.24 * sin(u_time);
  float pi = 3.14159;
  // stretch correction, aspect ratio magic
  float ar = u_res.x / u_res.y;
  float dx = abs(st.x - cx);
  float dy = (1.0 / ar) * abs(st.y - cy);
  float d = sqrt(pow(dx, 2.0) + pow(dy, 2.0));
  //
  float h = tan(pi * st.x * st.y);
  float mu = 32.0 * d * h * u_time;
  float z = exp(-2.0 * d) * sin(2.0 * pi * mu);
  float r = (242.0 / 256.0) * z;
  float g = (156.0 / 256.0) * z;
  float b = (33.0 / 256.0) * z;
  vec4 ga = vec4(r, g, b, 1.0);
  //
  float kc=0.40 + 0.20*sin(2.0 * pi * 0.112 * u_time);
  //
  vec4 cam=texture(tex0, texCoordVarying/u_res);
  //
  outputColor = cam*kc + ga*(1.0-kc);
}
