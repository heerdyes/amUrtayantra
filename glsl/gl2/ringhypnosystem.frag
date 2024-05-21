OF_GLSL_SHADER_HEADER

uniform sampler2D tex0;
uniform float u_time;
uniform vec2 u_res;

varying vec2 texCoordVarying;

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
  
  float h = 0.5 + 0.1 * sin(u_time);
  float j = 0.5 + 0.1 * sin(u_time*1.025);
  float k = 0.5 + 0.1 * sin(u_time*1.050);

  // frequency
  float mu = 32.0 * d * h;
  float nu = 32.0 * d * j;
  float la = 32.0 * d * k;
  
  float z = exp(-5.0 * d) * pow(sin(2.0 * pi * mu), 2.0);
  float q = exp(-5.0 * d) * pow(sin(2.0 * pi * nu), 2.0);
  float s = exp(-5.0 * d) * pow(sin(2.0 * pi * la), 2.0);
  
  float r = (250.0 / 256.0) * q;
  float g = (250.0 / 256.0) * z;
  float b = (250.0 / 256.0) * s;
  
  vec4 ga = vec4(r, g, b, 1.0);
  //
  float kc=0.40 + 0.20*sin(2.0 * pi * 0.112 * u_time);
  //
  vec4 cam=texture2D(tex0, texCoordVarying/u_res);
  //
  gl_FragColor = cam*kc + ga*(1.0-kc);
}
