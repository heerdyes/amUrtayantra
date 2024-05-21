OF_GLSL_SHADER_HEADER

uniform sampler2D tex0;
uniform float u_time;
uniform vec2 u_res;

varying vec2 texCoordVarying;

void main() {
  vec2 st = gl_FragCoord.xy / u_res;
  float rr = 0.2 * exp(-0.1 * u_time);
  float cx = 0.5 + rr * cos(u_time);
  float cy = 0.5 + rr * sin(u_time);
  float pi = 3.14159;

  float d = sqrt(pow(st.x - cx, 2.0) + pow(st.y - cy, 2.0));
  float h = st.x;
  float j = st.y;
  float k = (1.0 - st.x) * (1.0 - st.y);
  float mu = 32.0 * d * h * u_time;
  float nu = 32.0 * d * j * u_time;
  float la = 32.0 * d * k * u_time;

  float z = exp(-2.0 * d) * sin(2.0 * pi * mu);
  float q = exp(-2.0 * d) * sin(2.0 * pi * nu);
  float s = exp(-2.0 * d) * sin(2.0 * pi * la);
  float r = (23.0 / 256.0) * q;
  float g = (202.0 / 256.0) * z;
  float b = (232.0 / 256.0) * s;
  vec4 ga = vec4(r, g, b, 1.0);
  //
  float kc=0.40 + 0.20*sin(2.0 * pi * 0.112 * u_time);
  //
  vec4 cam=texture2D(tex0, texCoordVarying/u_res);
  //
  gl_FragColor = cam*kc + ga*(1.0-kc);
}
