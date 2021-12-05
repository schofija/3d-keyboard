#version 330 compatibility

uniform float	uTime;		// "Time", from Animate( )
in vec2  	vST;		// texture coords
uniform sampler2D uTexUnit;

uniform float   uKa, uKd, uKs;	// coefficients of each type of lighting
uniform vec3  uColor;			// object color
uniform vec3  uSpecularColor;	// light color
uniform float   uShininess; 	// specular exponent

	in vec3 vN; //normal vec
	in vec3 vL; //vector from point to light
	in vec3 vE; //vector from point to eye

void
main( )
{
	vec3 Normal = normalize(vN);
	vec3 Light     = normalize(vL);
	vec3 Eye        = normalize(vE);

	vec3 myColor = uColor;
	
	vec3 ambient = uKa * myColor;

	float d = max( dot(Normal,Light), 0. );       // only do diffuse if the light can see the point
	vec3 diffuse = uKd * d * myColor;

	float s = 0.;
	if( dot(Normal,Light) > 0. )	          // only do specular if the light can see the point
	{
		vec3 ref = normalize(  reflect( -Light, Normal )  );
		s = pow( max( dot(Eye,ref),0. ), uShininess );
	}
	vec3 specular = uKs * s * vec3(.8, .8, .8);
	
	vec3 newcolor = texture(uTexUnit, vST).rgb;
	gl_FragColor = vec4(newcolor + ambient + diffuse + specular, 1.);
	//gl_FragColor = vec4( ambient + diffuse + specular,  1. );

}

	//vec3 myColor = vec3( ??? );
	//if( ??? )
	//{
	//	myColor = vec3( ??? );
	//}
	//gl_FragColor = vec4( myColor,  1. );
	
	//gl_FragColor = vec4(.5, 1., 0., 1.);
