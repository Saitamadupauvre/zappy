// Adapted from Shadertoy: https://www.shadertoy.com/view/tdjSWR
// (parent: https://www.shadertoy.com/view/XsBSRG)

#version 330

in vec3 fragDir;

uniform float time;
uniform vec2  iResolution;

out vec4 finalColor;

#define iTime time
#define CARS
#define I_MAX 270

void bnot(inout bvec3 a){a.x=!a.x;a.y=!a.y;a.z=!a.z;}

#define vec1 float
bool isOutClosed(vec1 a,vec1 s,vec1 e){return              e-s<abs(a*2.-e-s);}
bool isOutClosed(vec2 a,vec2 s,vec2 e){return any(lessThan(e-s,abs(a*2.-e-s)));}
bool isOutClosed(vec3 a,vec3 s,vec3 e){return any(lessThan(e-s,abs(a*2.-e-s)));}
bool isOutClosed(vec4 a,vec4 s,vec4 e){return any(lessThan(e-s,abs(a*2.-e-s)));}
#define isInClosed(a,b,c) (!isOutClosed(a,b,c))
bool isInOpen(vec1 a,vec1 s,vec1 e){return              abs(a*2.-e-s)<e-s;}
bool isInOpen(vec2 a,vec2 s,vec2 e){return any(lessThan(abs(a*2.-e-s),e-s));}
bool isInOpen(vec3 a,vec3 s,vec3 e){return any(lessThan(abs(a*2.-e-s),e-s));}
bool isInOpen(vec4 a,vec4 s,vec4 e){return any(lessThan(abs(a*2.-e-s),e-s));}
#define isOutOpen(a,b,c) (!isInOpen(a,b,c))

float rand(vec2 n){return fract(sin((n.x*1e2+n.y*1e4+1475.4526)*1e-4)*1e6);}
float noise(vec2 p){p=floor(p*200.);return rand(p);}

#define pmpm vec4(1,-1,1,-1)
vec4 setg(vec2 a,vec2 b){return pmpm*min(a.xxyy*pmpm,b.xxyy*pmpm);}
vec3 polygon(vec2 c,vec4 g,vec3 d,vec3 u
){if(any(lessThan(vec4(c.xy,g.yw),vec4(g.xz,c.xy)))||dot(d,u)<0.)return vec3(101,0,0)
 ;return vec3(length(u),c.xy-g.xz);}
vec3 polygonXY(float z,vec2 a,vec2 b,vec3 u,vec3 d
){vec2 c=u.xy-d.xy*(u.z-z)/d.z;return polygon(c,setg(a,b),d,vec3(c,z)-u);}
vec3 polygonYZ(float x,vec2 a, vec2 b, vec3 u,vec3 d
){vec2 c=u.yz-d.yz*(u.x-x)/d.x;return polygon(c,setg(a,b),d,vec3(x,c)-u);}
vec3 polygonXZ(float y,vec2 a, vec2 b, vec3 u,vec3 d
){vec2 c=u.xz-d.xz*(u.y-y)/d.y;return polygon(c,setg(a,b),d,vec3(c.x,y,c.y)-u);}

vec3 textureWall(vec2 pos, vec2 maxPos, vec2 squarer,float s,float height,float dist,vec3 d,vec3 norm
){float randB=rand(squarer*2.0)
 ;vec3 windowColor=(-0.4+randB*0.8)*vec3(0.3,0.3,0.0)
 +(-0.4+fract(randB*10.0)*0.8)*vec3(0.0,0.0,0.3)+(-0.4+fract(randB*10000.)*0.8)*vec3(0.3,0.0,0.0)
 ;float floorFactor=1.
 ;vec2 windowSize=vec2(0.65,0.35)
 ;vec3 wallColor=s*(0.3+1.4*fract(randB*100.))*vec3(0.1,0.1,0.1)
 +(-0.7+1.4*fract(randB*1000.))*vec3(0.02,0.,0.)
 ;wallColor*=1.3
 ;vec3 color=vec3(0)
 ;vec3 conturColor=wallColor/1.5
 ;if (height<0.51
 ){windowColor += vec3(.3,.3,.0)
  ;windowSize=vec2(.4)
  ;floorFactor=0.;}
 ;if (height<.6){floorFactor=0.;}
 ;if (height>.75)windowColor += vec3(0,0,.3)
 ;windowColor*=1.5
 ;float wsize=0.02
 ;wsize+=-0.007+0.014*fract(randB*75389.9365)
 ;windowSize+= vec2(0.34*fract(randB*45696.9365),0.50*fract(randB*853993.5783))
 ;windowSize/=2.
 ;vec2 contur=vec2(0.0)+(fract(maxPos/2.0/wsize))*wsize
  ;vec2 pc=pos-contur
 ;if (contur.x<wsize)contur.x+=wsize
 ;if (contur.y<wsize)contur.y+=wsize
 ;vec2 winPos=(pc)/wsize/2.0-floor((pc)/wsize/2.0)
 ;float numWin=floor((maxPos-contur)/wsize/2.0).x
 ;vec3 n=floor(numWin*vec3(1,2,3)/4.)
 ;vec2 m=numWin*vec2(1,2)/3.
 ;float w=wsize*2.
 ;bvec3 bo=bvec3(isOutOpen(pc.x  ,w*n.y,w+w*n.y)||isOutOpen(maxPos.x,.5,.6)
                ,isOutOpen(pc.xx ,w*m  ,w+w*m  )||isOutOpen(maxPos.x,.6,.7)
                ,isOutOpen(pc.xxx,w*n  ,w+w*n  )||maxPos.x>.7)
 ;bnot(bo)
 ;if(any(bo))return (.9+.2*noise(pos))*conturColor
 ;if((maxPos.x-pos.x<contur.x)||(maxPos.y-pos.y<contur.y+w)||(pos.x<contur.x)||(pos.y<contur.y))
            return (0.9+0.2*noise(pos))*conturColor
 ;if (maxPos.x<0.14)return (0.9+0.2*noise(pos))*wallColor
 ;vec2 window=floor(pc/w)
 ;float random=rand(squarer*s*maxPos.y+window)
 ;float randomZ=rand(squarer*s*maxPos.y+floor(pc.yy/w))
 ;float windows=floorFactor*sin(randomZ*5342.475379+(fract(975.568*randomZ)*0.15+0.05)*window.x)
 ;float blH=0.06*dist*600./iResolution.x/abs(dot(normalize(d.xy),normalize(norm.xy)))
 ;float blV=0.06*dist*600./iResolution.x/sqrt(abs(1.0-pow(abs(d.z),2.0)))
 ;windowColor +=vec3(1.0,1.0,1.0)
 ;windowColor*=smoothstep(.5-windowSize.x-blH,.5-windowSize.x+blH,winPos.x)
 ;windowColor*=smoothstep(.5+windowSize.x+blH,.5+windowSize.x-blH,winPos.x)
 ;windowColor*=smoothstep(.5-windowSize.y-blV,.5-windowSize.y+blV,winPos.y)
 ;windowColor*=smoothstep(.5+windowSize.y+blV,.5+windowSize.y-blV,winPos.y)
 ;if ((random <0.05*(3.5-2.5*floorFactor))||(windows>0.65)
 ){if (winPos.y<0.5)windowColor*=(1.0-0.4*fract(random*100.))
  ;if ((winPos.y>0.5)&&(winPos.x<0.5))windowColor*=(1.0-0.4*fract(random*10.0))
  ;return (.9+.2*noise(pos))*wallColor+(0.9+0.2*noise(pos))*windowColor
 ;} else windowColor*=0.08*fract(10.0*random)
 ;return (.9+.2*noise(pos))*wallColor+windowColor;}

bool con(vec2 u,vec2 b,vec2 c
){return any(lessThan(vec3(min(u.x,u.y),b),vec3(0,u)))||!(any(lessThan(min(abs(b-u),abs(u)),c)));}

vec3 textureRoof(vec2 pos, vec2 maxPos,vec2 squarer
){float wsize=0.025
 ;float randB=rand(squarer*2.0)
 ;vec3 wallColor=(0.3+1.4*fract(randB*100.))*vec3(.1)+(-0.7+1.4*fract(randB*1000.))*vec3(0.02,0.,0.)
 ;vec3 conturColor=wallColor*1.5/2.5
 ;vec2 contur=vec2(0.02)
 ;if ((maxPos.x-pos.x<contur.x)||(maxPos.y-pos.y<contur.y)||(pos.x<contur.x)||(pos.y<contur.y)
 )return (0.9+0.2*noise(pos))*conturColor
 ;float s=.06+.12*fract(randB*562526.2865)
 ;pos-=s;maxPos-=s*2.;if(con(pos,maxPos,contur))return(.9+.2*noise(pos))*conturColor
 ;pos-=s;maxPos-=s*2.;if(con(pos,maxPos,contur))return(.9+.2*noise(pos))*conturColor
 ;pos-=s;maxPos-=s*2.;if(con(pos,maxPos,contur))return(.9+.2*noise(pos))*conturColor
 ;return (.9+.2*noise(pos))*wallColor;}

void carloop(inout vec3 c,vec2 u,vec3 car1,vec3 car2,vec2 s,vec3 e,float n,float t,float o
){float carNumber=0.5
 ;float r=0.01
 ;for (float j=0.;j<10.; j++
 ){float i=.03+o+j*.094
  ;vec2 a=vec2(fract(i+iTime/4.),e.x)
  ;if(e.z>0.)a=a.yx;
  ;if(fract(n*5./i)>carNumber)c+=car1*smoothstep(r,0.,length(u-a))
  ;a=vec2(fract(i-iTime/4.),e.y)
  ;if(e.z>0.)a=a.yx;
  ;if(fract(n*10./i)>carNumber)c+=car2*smoothstep(r,0.,length(u-a))
  ;if(c.x>0.) break;}}

vec3 cars(vec2 squarer, vec2 u, float dist,float level
){vec3 c=vec3(0)
 ;float carInten=3.5/sqrt(dist)
 ;float r=0.01
 ;if (dist>2.0)r*=sqrt(dist/2.0)
 ;vec3 car1=vec3(.5,.5,1)*carInten
 ;vec3 car2=vec3(1.,.1,.1)*carInten
 ;float carNumber=0.5
 ;float n=noise((level+1.)*squarer*1.24435824)
 ;float t=iTime/4.
 ;carloop(c,u,car1,car2,vec2(   5,  10),vec3(.025,.975,0),n, t,0.)
 ;carloop(c,u,car1,car2,vec2(  10,   5),vec3(.975,.025,1),n,-t,0.)
 ;carloop(c,u,car1,car2,vec2( 100,1000),vec3(.045,.955,0),n, t,.047)
 ;carloop(c,u,car1,car2,vec2(1000, 100),vec3(.955,.045,1),n,-t,.047)
 ;return c;}

vec3 textureGround(vec2 squarer, vec2 pos,vec2 a,vec2 b,float dist
){vec3 color=(0.9+0.2*noise(pos))*vec3(0.1,0.15,0.1)
 ;float randB=rand(squarer*2.)
 ;vec3 wallColor=(.3+1.4*fract(randB*100.))*.1+(-.7+1.4*fract(randB*1000.))*vec3(.02,0,0)
 ;float fund=0.03
 ;float bl=0.01
 ;float f=smoothstep(a.x-fund-bl,a.x-fund,pos.x)
 ;f*=smoothstep(a.y-fund-bl,a.y-fund,pos.y)
 ;f*=smoothstep(b.y+fund+bl,b.y+fund,pos.y)
 ;f*=smoothstep(b.x+fund+bl,b.x+fund,pos.x)
 ;pos -= 0.0
 ;vec2 maxPos=vec2(1)
 ;vec2 contur=vec2(0.06,0.06)
 ;if((pos.x>0.&&pos.y>0.&&pos.x<maxPos.x&&pos.y<maxPos.y)&&((abs(maxPos.x-pos.x)<contur.x)||(abs(maxPos.y-pos.y)<contur.y)||(abs(pos.x)<contur.x)||(abs(pos.y)<contur.y)))
            color= vec3(0.1,0.1,0.1)*(0.9+0.2*noise(pos))
 ;pos -= 0.06
 ;maxPos=vec2(.88)
 ;contur=vec2(.01)
 ;if ((pos.x>0.0&&pos.y>0.0&&pos.x<maxPos.x&&pos.y<maxPos.y)&&((abs(maxPos.x-pos.x)<contur.x)||(abs(maxPos.y-pos.y)<contur.y)||(abs(pos.x)<contur.x)||(abs(pos.y)<contur.y))) color=vec3(0)
 ;color=mix(color,(0.9+0.2*noise(pos))*wallColor*1.5/2.5,f)
 ;pos+=0.06
#ifdef CARS
 ;if (min(pos.x,pos.y)<0.07||max(pos.x,pos.y)>0.93) color+=cars(squarer,pos,dist,0.);
#endif
 ;return color;}

vec2 cs(float a){return vec2(cos(a),sin(a));}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{vec3 camPos=vec3(5.0, 5.0, 3.5)
 ;vec3 d=normalize(vec3(fragDir.x, -fragDir.z, fragDir.y))
 ;float angle=.03*pow(abs(acos(d.x)),4.0)
 ;vec3 color=vec3(0.0)
 ;vec2 square=floor(camPos.xy)
 ;square.xy +=.5-.5*sign(d.xy)
 ;float mind=100.
 ;int k=0
 ;vec3 pol
 ;vec2 maxPos,crossG
 ;float tSky=-(camPos.z-3.9)/d.z
 ;vec2 crossSky=floor(camPos.xy + d.xy*tSky)
 ;for (int i=1; i<I_MAX; i++
 ){vec2 squarer=square-vec2(0.5,0.5)+0.5*sign(d.xy)
  ;if(crossSky==squarer&&crossSky!=floor(camPos.xy)
  ){color+=vec3(vec2(.5,.15)*abs(angle)*exp(-d.z*d.z*30.),.2);break;}
  ;float random=rand(squarer),tl,height=0.
  ;float quartalR=rand(floor(squarer/10.))
  ;if (floor(squarer/10.)==vec2(0)) quartalR=.399
  ;if (quartalR<.4
  ){height=-.15+.4*random+smoothstep(12.,7.,length(fract(squarer/10.)*10.-vec2(5)))
   *.8*random+.9*smoothstep(10.,0.,length(fract(squarer/10.)*10.-vec2(5)))
   ;height*=quartalR/.4;}
  ;float maxJ=2.
  ;float roof=1.
  ;if (height<0.3
  ){height=0.3*(0.7+1.8*fract(random*100.543264));maxJ=2.0
   ;if (fract(height*1000.)<0.04) height*=1.3;}
  ;if (height>0.5)maxJ=3.
  ;if (height>0.85)maxJ=4.
  ;if (fract(height*100.)<0.15){height=pow(maxJ-1.0,0.3)*height; maxJ=2.0; roof=0.0;}
  ;float maxheight=1.5*pow((maxJ-1.0),0.3)*height+roof*0.07
  ;if (camPos.z+d.z*(length(camPos.xy - square) +0.71 - sign(d.z)*0.71)/length(d.xy)<maxheight
  ){vec2 ar
   ;vec2 br
   ;float zz=0.
   ;float prevZZ=0.
   ;for(int nf=1;nf<8;nf++
   ){float j=float(nf)
    ;if(j>maxJ)break
    ;prevZZ=zz
    ;zz=1.5*pow(j,0.3)*height
    ;float dia=1.0/pow(j,0.3)
    ;if(j==maxJ
    ){if (roof == 0.0)break
     ;zz=1.5*pow((j-1.0),0.3)*height+0.03+0.04*fract(random*1535.347)
     ;dia=1.0/pow((j-1.0),0.3)-0.2-0.2*fract(random*10000.);}
    ;vec2 v1=vec2(0)
    ;vec2 v2=vec2(0)
    ;float randomF=fract(random*10.0)
    ;if(randomF<.25){ v1=vec2(fract(random*1000.),fract(random*100.));}
   ;if(randomF>.25&&randomF<.5 ){v1=vec2(fract(random*100.),0.);v2=vec2(0.0,fract(random*1000.));}
   ;if(randomF>.5 &&randomF<.75){v2=vec2(fract(random*1000.),fract(random*100.));}
   ;if(randomF>.75             ){v1=vec2(0.,fract(random*1000.)); v2=vec2(fract(random*100.),0.);}
   ;if(d.y<0.0){float y=v1.y;v1.y=v2.y;v2.y=y;}
   ;if(d.x<0.0){float x=v1.x;v1.x=v2.x;v2.x=x;}
   ;vec2 a2=square+sign(d.xy)*(0.5-0.37*(dia*1.0-1.0*v1))
   ;vec2 b2=square+sign(d.xy)*(0.5+0.37*(dia*1.0-1.0*v2))
   ;if (j==1.0
   ){ar=vec2(min(a2.x, b2.x),min(a2.y,b2.y))
    ;br=vec2(max(a2.x, b2.x),max(a2.y,b2.y));}
   ;vec3 pxy=polygonXY(zz,a2,b2,camPos,d)
   ;if (pxy.x<mind){mind=pxy.x; pol=pxy; k=1;maxPos=vec2(abs(a2.x-b2.x),abs(a2.y-b2.y));}
   ;vec3 pyz=polygonYZ(a2.x,vec2(a2.y,prevZZ),vec2(b2.y,zz),camPos,d)
   ;if (pyz.x<mind){mind=pyz.x; pol=pyz; k=2;maxPos=vec2(abs(a2.y-b2.y),zz-prevZZ);}
   ;vec3 pxz=polygonXZ(a2.y,vec2(a2.x,prevZZ),vec2(b2.x,zz),camPos,d)
   ;if (pxz.x<mind){mind=pxz.x; pol=pxz; k=3;maxPos=vec2(abs(a2.x-b2.x),zz-prevZZ);}}
   ;if ((mind<100.)&&(k==1)
   ){color += textureRoof(vec2(pol.y,pol.z),maxPos,squarer);if (mind>3.){color*=sqrt(3./mind);};break;}
   ;if ((mind<100.)&&(k==2)
   ){color += textureWall(vec2(pol.y,pol.z),maxPos,squarer,1.2075624928,height,mind,d,vec3(1,0,0))
    ;if (mind>3.0)color*=sqrt(3.0/mind);break;}
   ;if ((mind<100.)&&(k==3)
   ){color += textureWall(vec2(pol.y,pol.z),maxPos,squarer,.8093856205,height,mind,d,vec3(0,1,0))
    ;if (mind>3.0)color*=sqrt(3.0/mind);break;}
   ;tl=-camPos.z/d.z
   ;crossG=camPos.xy + d.xy*tl
   ;if (floor(crossG) == squarer
   ){mind=length(vec3(crossG,0.0)-camPos)
    ;color += textureGround(squarer,fract(crossG),fract(ar),fract(br),mind)
    ;if (mind>3.0)color*=sqrt(3.0/mind);break;}}
  ;if ((square.x+sign(d.x)-camPos.x)/d.x<(square.y+sign(d.y)-camPos.y)/d.y
  ){square.x += sign(d.x)*1.0;}else square.y += sign(d.y)*1.
  ;if(i==I_MAX-1&&d.z>-0.1)color += vec3(vec2(0.5,0.15)*abs(angle)*exp(-d.z*d.z*30.0),0.2);}
 ;fragColor=vec4(color,1);}

void main()
{
    mainImage(finalColor, gl_FragCoord.xy);
}
