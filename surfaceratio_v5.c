/*
	Surface Ratio
	by roberto toro
	http://brainfolding.sourceforge.net
	
	1 October 2009,		v5 (support for meshes with vertices outside the 256^3 cube)
	1 October 2008,		v4 (adds support to BrainVisa meshes)
	9 September 2008,	v3 (solves bug: ValsPerVertex was still not swapped!)
	1 September 2008,	v2 (solves bug: swap ints and floats if working on an intel machine)
	9 February 2008,	v1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define pi 3.14159265358979323846264338327510
#define min(a,b) (((a)<(b))?(a):(b))

#define kMAXNETRIS			100

#define kFreeSurferMesh		1
#define kBrainVisaMesh		2
#define kFreeSurferData		3
#define kSRatioFloatData	4
#define kMINCData		5
#define kTEXTData		6


typedef struct
{
	float	x,y,z;
}float3D;
typedef struct
{
	int	a,b,c;
}int3D;

float3D	*p;
int3D	*t;
int		np;
int		nt;
float	R;

int		endianness;
#define kMOTOROLA	1
#define kINTEL		2
void checkEndianness(void)
{
	char	b[]={1,0,0,0};
	int		num=*(int*)b;
	
	if(num==16777216)
		endianness=kMOTOROLA;
	else
		endianness=kINTEL;
	printf("%s\n",(endianness==kMOTOROLA)?"Motorola":"Intel");
}

void swapint(int *n)
{
	char 	*by=(char*)n;
	char	sw[4]={by[3],by[2],by[1],by[0]};
	
	*n=*(int*)sw;
}
void swapfloat(float *n)
{
	char 	*by=(char*)n;
	char	sw[4]={by[3],by[2],by[1],by[0]};
	
	*n=*(float*)sw;
}
void swaptriangles()
{
	int		i;
	for(i=0;i<nt;i++)
	{
		swapint(&t[i].a);
		swapint(&t[i].b);
		swapint(&t[i].c);
	}
}
void swapvertices()
{
	int		i;
	for(i=0;i<np;i++)
	{
		swapfloat(&p[i].x);
		swapfloat(&p[i].y);
		swapfloat(&p[i].z);
	}
}
int getformatindex(char *path)
{
	char	*formats[]={"orig","pial","white","mesh","sratio","sratiofloat","curv","obj","txt"};
	int		i,j,n=9; // n=9, because there are 8 recognised formats
	int		found,index;
	
	for(i=0;i<n;i++)
	{
		j=strlen(formats[i]);
		found=(strcmp(formats[i],path+(strlen(path)-j))==0);
		if(found)
			break;
	}
	
	index=-1;
	if(i==0 || i==1 || i==2)
	{
		index=kFreeSurferMesh;
		printf("FreeSurfer mesh\n");
	}
	else
	if(i==3)
	{
		index=kBrainVisaMesh;
		printf("BrainVisa mesh\n");
	}
	else
	if(i==4 || i==6)
	{
		index=kFreeSurferData;
		printf("FreeSurfer Data\n");
	}
	else
	if(i==5)
	{
		index=kSRatioFloatData;
		printf("SRatioFloat Data (call surfaceratio without arguments for more information)\n");
	}
	else
	if(i==7)
	{
		index=kMINCData;
		printf("MINC Surface Data\n");
	}
	else
	if(i==8)
	{
		index=kTEXTData;
		printf("Text output data\n");
	}
	return index;
}
int FreeSurfer_load_mesh(char *path)
{
    FILE	*f;
    int		id,a,b,c,i;
    char	date[256],info[256];

    f=fopen(path,"r");
	
	if(f==NULL)
		return 1;

	// read triangle/quad identifier: 3 bytes
    a=((int)(u_int8_t)fgetc(f))<<16;
    b=((int)(u_int8_t)fgetc(f))<<8;
    c=(u_int8_t)fgetc(f);
    id=a+b+c;
    if(id==16777214)	// triangle mesh
    {
		fgets(date,256,f);
        fgets(info,256,f);
		fread(&np,1,sizeof(int),f); if(endianness==kINTEL) swapint(&np);
		fread(&nt,1,sizeof(int),f);	if(endianness==kINTEL) swapint(&nt);
        // read vertices
        p=(float3D*)calloc(np,sizeof(float3D));
			if(p==NULL) printf("Cannot allocate memory for points [FreeSurfer_load_mesh]\n");
			else
			{
		fread((char*)p,np,3*sizeof(float),f);	if(endianness==kINTEL) swapvertices();
        // read triangles
        t=(int3D*)calloc(nt,sizeof(int3D));
			if(t==NULL) printf("Cannot allocate memory for triangles [FreeSurfer_load_mesh]\n");
			else
			{
		fread((char*)t,nt,3*sizeof(int),f);		if(endianness==kINTEL) swaptriangles();
			}
			}
    }
	fclose(f);

	for(i=0;i<np;i++)
	{
		p[i].x+=128;
		p[i].y+=128;
		p[i].z+=128;
	}
	
	return 0;
}
int BrainVisa_load_mesh(char *path)
{
    FILE	*f;
	char	tmp[6];
    int		i;
    int		endian,ignore;
    
    f=fopen(path,"r");
	if(f==NULL){printf("ERROR: Cannot open file\n");return 1;}
	
	// READ HEADER
	// get format (ascii, binar)
    fread(tmp,5,sizeof(char),f); tmp[5]=(char)0;
    if(strcmp(tmp,"binar")==0)
    {
    	for(i=0;i<4;i++) tmp[i]=fgetc(f); tmp[4]=(char)0;
		endian=-1;
		if(strcmp(tmp,"ABCD")==0)	endian=kMOTOROLA;	
		if(strcmp(tmp,"DCBA")==0)	endian=kINTEL;
		if(endian==-1){ printf("ERROR: Not ABCD nor DCBA order...exit.\n"); return 1;}
		fread(&ignore,4,sizeof(char),f);		// ignore "VOID" string length
		fread(&ignore,4,sizeof(char),f);		// ignore "VOID" string
		fread(&ignore,1,sizeof(int),f);		// verify number of vertices per polygon
		if(endian!=endianness)
			swapint(&ignore);
		if(ignore!=3){ printf("ERROR: Only able to read triangle meshes. This mesh has %i vertices per polygon.\n",ignore); return 1;}
		fread(&ignore,1,sizeof(int),f);		// ignore time steps
		fread(&ignore,1,sizeof(int),f);		// ignore time step index
		
		// READ VERTICES
		fread(&np,1,sizeof(int),f);			// read number of vertices
		if(endian!=endianness)
			swapint(&np);
		p = (float3D*)calloc(np,sizeof(float3D));
		if(p==NULL){printf("ERROR: Not enough memory for mesh vertices\n");return 1;}
		printf("%i vertices\n",np);
		fread((char*)p,np,3*sizeof(float),f);	if(endian!=endianness) swapvertices();	
		
		// IGNORE NORMAL VECTORS
		fseek(f,sizeof(int),SEEK_CUR);		// ignore normal vectors
		fseek(f,np*sizeof(float3D),SEEK_CUR);
		fread(&ignore,1,sizeof(int),f);		// ignore number of texture coordinates
		
		// READ TRIANGLES
		fread(&nt,1,sizeof(int),f);			// read number of triangles
		if(endian!=endianness)
			swapint(&nt);
		t = (int3D*)calloc(nt,sizeof(int3D));
		if(t==NULL){printf("ERROR: Not enough memory for mesh triangles\n"); return 1;}
		printf("%i triangles\n",nt);
		fread((char*)t,nt,3*sizeof(int),f);		if(endian!=endianness) swaptriangles();
	}
	else
    if(strcmp(tmp,"ascii")==0)
    {
    	fscanf(f," %*s ");			// ignore VOID
    	fscanf(f," %*i %*i %*i ");	// ignore 3 integers
    	
    	// READ 3-D COORDINATES
    	fscanf(f," %i ",&np);
		p = (float3D*)calloc(np,sizeof(float3D));
    	for(i=0;i<np;i++)
    		fscanf(f," ( %f , %f , %f ) ", &p[i].x,&p[i].y,&p[i].z);
    	
    	fscanf(f," %*i ");			// ignore number of normal vectors
    	for(i=0;i<np;i++)			// ignore normal vectors
    		fscanf(f," ( %*f , %*f , %*f ) ");
    	fscanf(f," %*i ");			// ignore an integer
    	
    	// READ TRIANGLES
    	fscanf(f," %i ",&nt);
		t = (int3D*)calloc(nt,sizeof(int3D));
    	for(i=0;i<nt;i++)
    		fscanf(f," ( %i , %i , %i ) ", &t[i].a,&t[i].b,&t[i].c);
    }
    else
    {
    	printf("ERROR: Cannot read '%s' format.\n",tmp);
    	return 1;
    }
	
    
	fclose(f);
    
    return 0;
}

int MINC_load_mesh(char *path)
{
	FILE	*f;
	char	tmp[6];
	int		i;
	
	f=fopen(path,"r");
	if(f==NULL){printf("ERROR: Cannot open file\n");return 1;}

	fscanf(f,"%*s %*f %*f %*f %*i %*i");	// IGNORE HEADER

	// READ 3-D COORDINATES
	fscanf(f," %i\n",&np);
	p = (float3D*)calloc(np,sizeof(float3D));
	printf("# of points: %i\n",np);

	for(i=0;i<np;i++) {
		fscanf(f,"%f %f %f", &p[i].x,&p[i].y,&p[i].z);
	}

	for(i=0;i<np;i++) {						// ignore normal vectors
		fscanf(f,"%*f %*f %*f");
	}

	fscanf(f,"%i ",&nt);					// num  triangles
	printf("#triangles: %i\n", nt);
	t = (int3D*)calloc(nt,sizeof(int3D));

	fscanf(f,"%*s %*s %*s %*s %*s");		// ignore polygon header

	for(i=0;i<nt;i++) {						// ignore vertex/polygon colours
		fscanf(f," %*i ");
	}

	for(i=0;i<nt;i++) {						// read connectivity
		fscanf(f," %i %i %i ", &t[i].a,&t[i].b,&t[i].c);
	}
	
	fclose(f);
	
	for(i=0;i<np;i++) {
		p[i].x+=128;
		p[i].y+=128;
		p[i].z+=128;
	}

	return 0;

}

int FreeSurfer_save_data(char *path, float *d, int np)
{
	FILE	*f;
    int		id=16777215,a,b,c,FaceCount=0,ValsPerVertex=1,i,n;
    float	x;

    f=fopen(path,"w");
	
	if(f==NULL)
		return 1;
	
	// write data identifier: 3 bytes
    a=id>>16;
    b=(id&0xff00)>>8;
    c=(id&0xff);
    fputc((char)a,f);
    fputc((char)b,f);
    fputc((char)c,f);
    
	n=np;
	if(endianness==kINTEL)
	{
		swapint(&n);
		swapint(&FaceCount);
		swapint(&ValsPerVertex);
	}

    // write number of vertices
	fwrite(&n,1,sizeof(int),f);
	
	// write empty FaceCount and ValsPerVertex
	fwrite(&FaceCount,1,sizeof(int),f);
	fwrite(&ValsPerVertex,1,sizeof(int),f);

	if(endianness==kINTEL)
	{
		for(i=0;i<np;i++)
		{
			x=d[i];
			swapfloat(&x);
			fwrite(&x,1,sizeof(float),f);
		}
	}
	else
		fwrite(d,np,sizeof(float),f);
	fclose(f);

	return 0;
}
int SRatioFloat_save_data(char *path, float *d, int np)
{
	FILE	*f;

    f=fopen(path,"w");
	
	if(f==NULL)
		return 1;
	
	// write data identifier: 3 bytes
    fprintf(f,"%i\n",np);
    if(endianness==kMOTOROLA)
    	fprintf(f,"Motorola\n");
    else
    if(endianness==kINTEL)
     	fprintf(f,"Intel\n");
    else
    {
    	printf("ERROR: What's this computer!! (Intel? Motorola?)\n");
    	return 1;
    }
	fwrite(d,np,sizeof(float),f);
	fclose(f);

	return 0;
}

int TEXT_save_data(char *path, float *d, int np)
{
    FILE	*f;
    int i; 
    f=fopen(path,"w");
    if(f==NULL)
        return 1;
	
	for(i=0;i<np;i++)
	{
		fprintf(f, "%f\n", d[i]);
	}
	fclose(f);
    return 0;
}

#pragma mark -
float3D sub3D(float3D a, float3D b)
{
	return (float3D){a.x-b.x,a.y-b.y,a.z-b.z};
}
float norm3D(float3D a)
{
	return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}
// triangle area using Heron's formula
float triangle_area(float3D p0, float3D p1, float3D p2)
{
	float	a,b,c;	// side lengths
	float	s;		// semiperimeter
	float	area;
	
	a=norm3D(sub3D(p0,p1));
	b=norm3D(sub3D(p1,p2));
	c=norm3D(sub3D(p2,p0));
	s=(a+b+c)/2.0;
	
	area=sqrt(s*(s-a)*(s-b)*(s-c));
	
	return area;
}
float* surfaceratio(float r)
{
	int		i,j,x,y,z,a,b,c,nan=0;
	int		min[3],max[3];
	float	*lf;
	int		*tt;
	float	area,asum,*avol;
	char	str[512];
	
	lf=(float*)calloc(np,sizeof(float));
	
	R=r;
	
	min[0]=max[0]=p[0].x;
	min[1]=max[1]=p[0].y;
	min[2]=max[2]=p[0].z;
	for(i=0;i<np;i++)
	{
		if(p[i].x<min[0])	min[0]=p[i].x;
		if(p[i].y<min[1])	min[1]=p[i].y;
		if(p[i].z<min[2])	min[2]=p[i].z;

		if(p[i].x>max[0])	max[0]=p[i].x;
		if(p[i].y>max[1])	max[1]=p[i].y;
		if(p[i].z>max[2])	max[2]=p[i].z;
	}
	avol=(float*)calloc((max[0]-min[0])*(max[1]-min[1])*(max[2]-min[2]),sizeof(float));
	
	for(i=0;i<nt;i++)
	{
		area=triangle_area(p[t[i].a],p[t[i].b],p[t[i].c]);
		tt=(int*)&t[i];
		for(j=0;j<3;j++)
		{
			x=(int)p[tt[j]].x;
			y=(int)p[tt[j]].y;
			z=(int)p[tt[j]].z;
			if(x>=min[0]&&x<max[0] && y>=min[1]&&y<max[1] && z>=min[2]&&z<max[2])
				avol[(z-min[2])*(max[1]-min[1])*(max[0]-min[0])+(y-min[1])*(max[0]-min[0])+(x-min[0])]+=area;
		}
	}
	
	for(i=0;i<np;i++)
	{
		if(i%100==0)
		{
		sprintf(str,"%i/%i",i,np);
		printf("%s",str);
		for(j=0;j<strlen(str);j++)
			printf("\b");
		fflush(stdout);
		}

		asum=0;
		for(x=-R;x<=R;x++)
		for(y=-R;y<=R;y++)
		for(z=-R;z<=R;z++)
			if(x*x+y*y+z*z<R*R)
			{
				a=(int)p[i].x+x;
				b=(int)p[i].y+y;
				c=(int)p[i].z+z;
				if(a>=min[0]&&a<max[0] && b>=min[1]&&b<max[1] && c>=min[2]&&c<max[2])
					asum+=avol[(c-min[2])*(max[1]-min[1])*(max[0]-min[0])+(b-min[1])*(max[0]-min[0])+(a-min[0])];
			}
		// the area of a triangle completely inside the sphere
		// will be added 3 times (one per vertex)
		// the area of a triangle partially inside the sphere		
		// will be accounted proportionally to the number
		// of vertices inside the sphere
		lf[i]=asum/(pi*R*R)/3.0;
		
		if(!(lf[i]==lf[i]))
			nan++;
	}
	printf("\n");
	free(avol);
	
	if(nan)
		printf("ERROR: there are %i NaN\n",nan);
	
	return lf;
}
int main(int argc, char *argv[])
{
	// argv[1]	input freesurfer mesh (ex., lh.pial)
	// argv[2]	output sratio file in "curv" file format (ex., lh.sratio)
	// argv[3]	sphere radius. Default 20mm
	
	int		err,format_in, format_out;
	float	*sr;
	float	radius;
	
	// Parse arguments or display usage
	printf("\n	Surface Ratio v5, Roberto Toro 1 October 2009\n");
	if(argc!=3 && argc!=4)
	{
		printf("	Usage:\n");
		printf("		surfaceratio input-pial-surface output-surfaceratio-vector\n");
		printf("		or\n");
		printf("		surfaceratio input-pial-surface output-surfaceratio-vector radius\n\n");
		printf("	In the first case a sphere radius of 20mm is used.\n\n");
		printf("	On input, FreeSurfer and BrainVisa mesh formats are accepted.\n");
		printf("	On output, the surface ratio vector can be written as a 'sratio' file, which\n");
		printf("	is encoded in FreeSurfer 'curv' format, or in 'sratiofloat' format, easier to read from\n");
		printf("	C code, in which the first line indicates the number of values (ascii),\n");
		printf("	the second line indicates if the encoding is Intel or Motorola (ascii), and the remaning\n");
		printf("	data is a block of float values.\n\n");
		printf("	The different possibilities are recognised from the file extensions (.orig, .pial, .white,\n");
		printf("	.mesh, .sratio and .sratiofloat).\n\n");
		return 0;
	}
	else
	if(argc==4)
		radius=atof(argv[3]);
	else
	if(argc==3)
		radius=20;
	
	// Check endianness
	checkEndianness();
	
	format_in=getformatindex(argv[1]);
	format_out=getformatindex(argv[2]);
	
	// Load pial surface
	switch(format_in)
	{
		case kFreeSurferMesh:
			err=FreeSurfer_load_mesh(argv[1]);
			break;
		case kBrainVisaMesh:
			err=BrainVisa_load_mesh(argv[1]);
			break;
		case kMINCData:
			err=MINC_load_mesh(argv[1]);
			break;
		default:
			printf("ERROR: Input mesh format not recognised\n");
			return 1;
	}
	if(err!=0)
	{
		printf("ERROR: cannot read file: %s\n",argv[1]);
		return 1;
	}

	// Compute Surface Ratio
	sr=surfaceratio(radius);
	
	// Save result
	switch(format_out)
	{
		case kFreeSurferData:
			FreeSurfer_save_data(argv[2], sr, np);
			break;
		case kSRatioFloatData:
			SRatioFloat_save_data(argv[2],sr,np);
			break;
		case kTEXTData:
			TEXT_save_data(argv[2],sr,np);
			break;
		default:
			printf("ERROR: Output data format not recognised\n");
			break;
	}
	if(err!=0)
	{
		printf("ERROR: cannot write to file: %s\n",argv[2]);
		return 1;
	}
	
	free(sr);
	free(p);
	free(t);
	
	return 0;
}
