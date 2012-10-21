// VERTEX SHADER

// notes:
// to maintain compatibility, the version
// preprocessor call needs to be added to the
// beginning of this file by the (cpu) compiler:
//
// "#version 100" for OpenGL ES 2 and
// "#version 120" (or higher) for desktop OpenGL

#ifdef GL_ES
    // vertex shader defaults for types are:
    // precision highp float;
    // precision highp int;
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
varying mediump vec4 vColor;
varying mediump vec2 vPosition;
varying mediump float vPointSize;

// uniforms
uniform vec4 Color;        // vertex color
uniform vec3 ViewDirn;     // camera eye in world space

void main()
{
   // set color -- set opacity based on
   // distance to the camera eye
   vColor = Color;

   // offset
   vec3 offset_vec = ViewDirn*1500000.0;        // not sure why i need this
   vec3 offset_pt = gl_Vertex.xyz - offset_vec;

   //
   if(dot(offset_pt.xyz,offset_vec) < 0.0)   {
      vColor.w = 0.0;
   }

   vPointSize = 50;
   gl_PointSize = vPointSize;
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

   // send screen coordinates to fragment shader
   vPosition = vec2(gl_Position.x/gl_Position.w,
                    gl_Position.y/gl_Position.w);
}
