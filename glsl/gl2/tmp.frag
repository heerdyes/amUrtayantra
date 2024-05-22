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
  //
  float kc=0.50 + 0.10*sin(2.0 * pi * 0.224 * u_time);
  //
  vec4 cam=texture2D(tex0, texCoordVarying/u_res);
  float mahou=sin(2.0*pi*cam.x*cam.y*u_time * d);
  float hypnote=log(2.0+sin(1.0*pi*u_time*d));
  vec4 recam=vec4(mahou, cam.y, hypnote, cam.w);
  //
  gl_FragColor = recam*kc;
}
