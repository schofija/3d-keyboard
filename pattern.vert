#version 330 compatibility

uniform float	uTime;		// "Time", from Animate( )
out vec2  	vST;		// texture coords

//Per-fragment lighting variables
	out vec3 vN; //normal vec
	out vec3 vL; //vector from point to light
	out vec3 vE; //vector from point to eye

	const vec3 LightPosition = vec3(  0., 5., 5. );

const float PI = 	3.14159265;
const float AMP = 	0.2;		// amplitude
const float W = 	2.;		// frequency

void
main( )
{ 
	vec3 vert = gl_Vertex.xyz;
	vec4 ECposition = gl_ModelViewMatrix * vec4( vert, 1. );
	vN = normalize( gl_NormalMatrix * gl_Normal );				// normal vector
	vL = LightPosition - ECposition.xyz;					// vector from the point to the light position
	vE = vec3( 0., 0., 0. ) - ECposition.xyz;	// vector from the point to the eye position 
		
	gl_Position = gl_ModelViewProjectionMatrix * vec4( vert, 1. );
}

	//vST = gl_MultiTexCoord0.st;
	//vec3 vert = gl_Vertex.xyz;
	//vert.x = ??? something fun of your own design
	//vert.y = ??? something fun of your own design
	//vert.z = ??? something fun of your own design
	//gl_Position = gl_ModelViewProjectionMatrix * vec4( vert, 1. );
