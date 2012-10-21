// FRAGMENT SHADER

// notes:
// to maintain compatibility, the version
// preprocessor call needs to be added to the
// beginning of this file by the (cpu) compiler:
//
// "#version 100" for OpenGL ES 2 and
// "#version 120" (or higher) for desktop OpenGL

#ifdef GL_ES
    // the fragment shader in ES 2 doesn't have a
    // default precision qualifier for floats so
    // it needs to be explicitly specified
    precision mediump float;

    // note: highp may not be available for float types in
    // the fragment shader -- use the following to set it:
    // #ifdef GL_FRAGMENT_PRECISION_HIGH
    // precision highp float;
    // #else
    // precision mediump float;
    // #endif

    // fragment shader defaults for other types are:
    // precision mediump int;
    // precision lowp sampler2D;
    // precision lowp samplerCube;
#else
    // with default (non ES) OpenGL shaders, precision
    // qualifiers aren't used -- we explicitly set them
    // to be defined as 'nothing' so they are ignored
    #define lowp
    #define mediump
    #define highp
#endif

// varyings
varying vec4 vColor;
varying vec2 vPosition;
varying float vPointSize;

// uniforms

void main()
{
   vec2 uCanvasDims = vec2(800,400);   // todo FIXME

   // get screen cordinates from frag
   float xScreen = (gl_FragCoord.x/uCanvasDims.x-0.5)*2;
   float yScreen = (gl_FragCoord.y/uCanvasDims.y-0.5)*2;
   vec2 pointCoord=vec2((xScreen-vPosition.x)/(vPointSize/uCanvasDims.x),
                        (yScreen-vPosition.y)/(vPointSize/uCanvasDims.y));

   float dist = distance(pointCoord,vec2(0));
//   float alpha = pow(5,-2.0*dist);
   float alpha = pow(1-sqrt(dist),10);
//   float alpha = 1;
   gl_FragColor = vec4(vColor.xyz,vColor.w*alpha);
}
