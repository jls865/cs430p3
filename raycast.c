#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
// Polymorphism in C
//Creates the 3 objects using a parent object struct.
typedef struct {
  int type; // 0 = plane, 1 = sphere, 2 = camera
  double color[3];//color vector;
  double height;//height of the view plane read in on command
  double width;//width of the view plane read in on cmd
  union {
    struct {
      double* position;
      double* color;
      double* normal;
    } plane;
    struct {
      double* position;
      double radius;
      double* color;
    } sphere;
    struct {
      double width;
      double height;
    } camera;
  };
} Object;


typedef struct{
unsigned char r,g,b;

}ppm_data;

int line = 1;
Object object[128];
double cam_height, cam_width;


double sqr(double v) {
  return v*v;
}

double dot_product(double* v1, double* v2, int n){
    double result = 0.0;
    int i;
    for(i=0;i<n;i++){
        result += v1[i]*v2[i];
    }
    return result;
}
//hello

void normalize(double* v) {
  double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}
//writes the pixel data to the file in p6 format
int writePPM(char* destination, ppm_data* data, int width, int height){
    FILE* fh = fopen(destination, "w");
    //writes the header info of the p6 file
    fprintf(fh, "P6 %d %d 255\n", width, height);

    //writes the pdata to the  file
    fwrite(data, sizeof(ppm_data), width*height, fh);
    fclose(fh);
    return 0;
}

double sphere_intersection(double* Ro, double* Rd, double* C, double r){
    double a,b,c,quad,dist1,dist2;
    // uses the equations given, along with the quadratic equation
    a = sqr(Rd[0])+sqr(Rd[1])+sqr(Rd[2]);
    b = 2 * (Rd[0]*(Ro[0]-C[0]) + Rd[1]*(Ro[1]-C[1]) + Rd[2]*(Ro[2]-C[2]));
	c = sqr(Ro[0]-C[0]) + sqr(Ro[1]-C[1]) + sqr(Ro[2]-C[2]) - sqr(r);

    //quadratic equation is calculated
    quad = (sqr(b) - 4*a*c);
    if(quad <0){
        fprintf(stderr, "Negative quadratic not valid\n");
        return -1;
    }
    quad = sqrt(quad);

    // plus or minus quad.
    dist1 =(-b - quad)/(2*a);
    dist2 =(-b +quad)/(2*a);

    //checking minus part of quadratic
    if(dist1>0){
        return dist1;
    }
    //checking plus part of quadratic
    else if (dist2>0){
        return dist2;

    }else{return -1;}







}

double plane_intersection(double* Ro, double* Rd, double* P, double* n){
    normalize(n);
    normalize(Rd);
    double d, t, ,distance;
    //uses dot product to find the intersectiion points
    d = dot_product(Rd, n, 3);

    distance = - dot_product(P,n,3);
    if(t < 0){
        return -1;
    }

    // if greater than 0  then we have found a point on the plane.
    t = distance/d;
    if(t>0){
        return t;
    }else{return  -1;}

}


void do_raycast(Object* objects, int width, int height, ppm_data* pdata){
    double x,y,ph,pw,t;
    int i,j,a,b,c,index;
    Object object;


    double v1[3] = {0,0,0};//makes the origin array
    double v2[3] = {0,0,0};//makes the direction array
    double v3[3] = {0,0,1};// makes the array we will add on to the z is +1 away from camera


    //sets cameras values because its position at 0,0,0, z +1 is accounted for in v3;
    x=0;
    y=0;
    index=0;
    //sets numbers so that it can loop width x height times(i x j)
    i = width;
    j = height;

    //get the pixel size by dividing camera width and height by inputs width and height
    ph = cam_height/height;
    pw = cam_width/width;


    for(a=0; a<j; a++){

        //finds the value for y
        v3[1]= (y- (cam_height/2)+ ph * (a +.5));

        for(b=0; b<i; b++){
                //gets the x value for the ray direction
            v3[0] = x-(cam_width/2) + pw * (b +.5);

            v2[0] = v3[0];
            v2[1] = v3[1];
            v2[2]=  v3[2];
            normalize(v3);

            for(c=0; objects[c]!=NULL; c++){
                t = 0;

                //plane
                if(objects[c].type ==0){
                  t=  plane_intersection(v1,v2,objects[c].plane.color,v3);

                }
                //sphere
                else if(objects[c].type == 1){
                  t=  sphere_intersection(v1,v2,objects[c].sphere.color,objects[c].sphere.radius);

                }
                //camera
                else if(objects[c].type == 2){
                    break;


                }
                else{fprintf(stderr, "how did this make it past the first error?");}
                    exit(1);
                }
            if(t<INFINITY && t>0){
                object = objects[i];
            }
        }
            if(t >0 && t != INFINITY){
                if(object.type ==1){
                    pdata[index].r = (int)(object.sphere.color[0]*255);
                    pdata[index].g = (int)(object.sphere.color[1]*255);
                    pdata[index].b = (int)(object.sphere.color[2]*255);
                }
                else if(object.type ==0){
                    pdata[index].r = (int)(object.plane.color[0]*255);
                    pdata[index].g = (int)(object.plane.color[1]*255);
                    pdata[index].b = (int)(object.plane.color[2]*255);

                }
            }else{
                pdata[index].r = 0;
                pdata[index].g = 0;
                pdata[index].b = 0;

            }
            index++;
        }



}



// next_c() wraps the getc() function and provides error checking and line
// number maintenance
int next_c(FILE* json) {
  int c = fgetc(json);
#ifdef DEBUG
  printf("next_c: '%c'\n", c);
#endif
  if (c == '\n') {
    line += 1;
  }
  if (c == EOF) {
    fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
    exit(1);
  }
  return c;
}


// expect_c() checks that the next character is d.  If it is not it emits
// an error.
void expect_c(FILE* json, int d) {
  int c = next_c(json);
  if (c == d) return;
  fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
  exit(1);
}


// skip_ws() skips white space in the file.
void skip_ws(FILE* json) {
  int c = next_c(json);
  while (isspace(c)) {
    c = next_c(json);
  }
  ungetc(c, json);
}


// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char* next_string(FILE* json) {
  char buffer[129];
  int c = next_c(json);
  if (c != '"') {
    fprintf(stderr, "Error: Expected string on line %d.\n", line);
    exit(1);
  }
  c = next_c(json);
  int i = 0;
  while (c != '"') {
    if (i >= 128) {
      fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
      exit(1);
    }
    if (c == '\\') {
      fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
      exit(1);
    }
    if (c < 32 || c > 126) {
      fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
      exit(1);
    }
    buffer[i] = c;
    i += 1;
    c = next_c(json);
  }
  buffer[i] = 0;
  return strdup(buffer);
}

double next_number(FILE* json) {
  double value;
  fscanf(json, "%lf", &value);
  // Error check this..
  return value;
}

double* next_vector(FILE* json) {
  double* v = malloc(3*sizeof(double));
  expect_c(json, '[');
  skip_ws(json);
  v[0] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[1] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[2] = next_number(json);
  skip_ws(json);
  expect_c(json, ']');
  return v;
}


void read_scene(char* filename) {
  int c;
  FILE* json = fopen(filename, "r");

  if (json == NULL) {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
    exit(1);
  }

  skip_ws(json);

  // Find the beginning of the list
  expect_c(json, '[');

  skip_ws(json);

  // Find the objects
  int index = 0;
  while (1) {

    c = fgetc(json);
    if (c == ']') {
      fprintf(stderr, "Error: This is the worst scene file EVER.\n");
      fclose(json);
      return;
    }
    if (c == '{') {
      skip_ws(json);

      // Parse the object
      char* key = next_string(json);
      if (strcmp(key, "type") != 0) {
	fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n", line);
	exit(1);
      }

      skip_ws(json);

      expect_c(json, ':');

      skip_ws(json);

      char* value = next_string(json);
      index++;
      if (strcmp(value, "camera") == 0) {
            object[index].type=2;
      } else if (strcmp(value, "sphere") == 0){
            object[index].type =1;
      } else if (strcmp(value, "plane") == 0) {
            object[index].type=0;
      } else {
	fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
	exit(1);
      }

      skip_ws(json);
      while (1) {
	// , }
	c = next_c(json);
	if (c == '}') {
	  // stop parsing this object
	  break;
	} else if (c == ',') {
	  // read another field
	  skip_ws(json);
	  char* key = next_string(json);
	  skip_ws(json);
	  expect_c(json, ':');
	  skip_ws(json);
	  //if key word is found conditional
	  if ((strcmp(key, "width") == 0) ||
	      (strcmp(key, "height") == 0) ||
	      (strcmp(key, "radius") == 0)) {
            double value = next_number(json);
            //if its a sphere set value of radius
            if(object[index].type ==1){
                object[index].sphere.radius = value;
            }
            //if its a camera check height/width and assign values accordingly
            else if(object[index].type == 2){

                if((strcmp(key, "width") == 0)){
                    object[index].camera.width = value;
                    cam_width= value;
                }

                else if(strcmp(key, "height") == 0){
                    object[index].camera.height = value;
                    cam_height= value;
                }else{

                    fprintf(stderr, "Not a valid variable for Camera\n");}

            }else{
                    fprintf(stderr, "Not a valid type of object for width height or radius\n");}
        }
        //if the key is found conditionals
        else if ((strcmp(key, "color") == 0) ||
		     (strcmp(key, "position") == 0) ||
		     (strcmp(key, "normal") == 0)) {
                double* value = next_vector(json);
                // if its a plane assign vector value by checking key
                if(object[index].type == 0){
                    if(strcmp(key, "color") == 0){
                        object[index].plane.color = value;
                    }

                    else if(strcmp(key, "position") == 0){
                        object[index].plane.position = value;
                    }
                    else if(strcmp(key, "normal") == 0){
                        object[index].plane.normal = value;

                    }else{fprintf(stderr, "Not a valid variable for plane\n");}
                    //if its a sphere set the vectors.
                }else if(object[index].type == 1){

                    if(strcmp(key, "color")== 0){
                        object[index].sphere.color = value;
                    }
                    else if(strcmp(key, "position")== 0){
                        object[index].sphere.position = value;


                    }else{fprintf(stderr, "Not a valid variable for a sphere\n");}


	    } else {fprintf(stderr, "Not a valid type of object for color position or normal\n");}

	  } else {
	    fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n",
		    key, line);
        //char* value = next_string(json);
	  }
	  skip_ws(json);
	} else {
	  fprintf(stderr, "Error: Unexpected value on line %d\n", line);
	  exit(1);
	}
      }
      skip_ws(json);
      c = next_c(json);
      if (c == ',') {
	// noop
	skip_ws(json);
      } else if (c == ']') {
	fclose(json);
	return;
      } else {
	fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
	exit(1);
      }
    }
  }
}




int main(int argc, char** argv){
    Object scene[128];
    ppm_data* data;
    int width, height;
    //if too little arguments throw this
	if(argc<4){
        fprintf(stderr, "Not valid arguments for program Raycast.c");
        return -1;
	}
    //checks if JSON file is valid or not
	char* temp = argv[3];
	if(strstr(temp, ".json")==NULL){
        fprintf(stderr, "Not valid input file type");
        return -1;
	}
    //sets read in width/height
	width = atoi(argv[1]);
	height = atoi(argv[2]);

    //checks for bad input
	if(width <=0 || height <=0){
        fprintf(stderr, "Width and Height must be more than 0");
        return -1;
	}
	//calls functions to parse json file
	read_scene(temp);
	//sets string to  a temp to check if its a ppm file
	temp = argv[4];
	if(strstr(temp, ".ppm")== NULL){
        fprintf(stderr, "Not valid output file type");
        return -1;
    }
    //allocates the pixel data
    data = (ppm_data*)malloc(sizeof(ppm_data*) * width * height + 1);
    //raycast
    do_raycast(scene,width,height,data);


	writePPM(argv[4],data,width,height);
	return 0;


}


