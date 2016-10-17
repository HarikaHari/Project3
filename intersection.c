/*
  File Name    : intersection.c
  Assignment   : Raycasting
  Created by   : Harika Hari (hh453). */

#include "custom.h"
#include "vector.c"
#include "ppm.c"

//Returns the position of camera object from the JSON file
int getCameraPosition(OBJECT *objects){
    int i=0;
    while(i < ObjectsCount && objects[i].type !=0){
        if (objects[i].type == CAM) {
            return i;
        }
        i++;
    }
    return -1;
}


// fill in pixel color to our image - need to flip the y axis due to the placement of the viewplane

void colorPixel(double *color, int row, int col,Image *image){
 //store results in image->data which store pixels
    image->data[row * image->width*4 + col*4] = (char)color[0];
    image->data[row * image->width*4 + col*4+1] = (char)color[1];
    image->data[row * image->width*4 + col*4+2]= (char)color[2];
    image->data[row * image->width*4 + col*4+3]= 255;

}


//Ro means Ray origin - Rd means Ray direction (other end of the ray)
 
double planeIntersection(double *Ro, double *Rd, double *Pos, double *Norm){

    double a,d;
    normalize(Norm);
    a = VectorDotProduct(Norm, Rd);
    if (fabs(a) <0.0001) { 
	// checks with absolute value and verifies if the plane is parallel to ray 
        return -1;
    }
    Vector incident;
    VectorSubstraction(Pos, Ro, incident);
    d = VectorDotProduct(incident, Norm);
    double t = d/a; 
    if (t<0.0) {
	 // no intersection of ray with plane
        return -1;
    }
    return t; 
	}

double sphereIntersection(double *Ro, double *Rd, double *pos, double r){

    double a, b, c;
    double t0,t1; 
	//calculating the coefficients of the equation. 
    a = sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]);
    b = 2 * (Rd[0]*(Ro[0]-pos[0]) + Rd[1]*(Ro[1]-pos[1]) + Rd[2]*(Ro[2]-pos[2]));
    c = sqr(Ro[0]-pos[0]) + sqr(Ro[1]-pos[1]) + sqr(Ro[2]-pos[2]) - sqr(r);
    
    if (a > 1.0001 || a < .9999) {
        printf("a = %lf\n", a);
        fprintf(stderr, "Ray direction was not normalized\n");
        exit(1);
    }
	//calculate the determinant value to get the point of intersection
    double d = sqr(b) - 4*a*c;
    
    if (d < 0) 
	  return -1; //no intersection
    
    else if (d == 0) {
        t0 = -1*(b / (2*a));
        t1 = -1*(b / (2*a));
       }
    else {  
        t0 = (-1*b - sqrt(sqr(b) - 4*c))/2;
        t1 = (-1*b + sqrt(sqr(b) - 4*c))/2;
    }
        
    if (t0 < 0 && t1 < 0) 
        return -1;

    else if (t0 < 0 && t1 > 0) 
        return t1;
    
    else if (t0 > 0 && t1 < 0) 
        return t0;
    
    else { 
        if (t0 <= t1)
            return t0;
        else
            return t1;
    }
    
}

double quadricIntersection (double *Ro, double *Rd, double *pos, double *coefficient) {

    double a, b, c;
    double t0,t1; 
		//calculating the coefficients of the equation. 
	a = coefficient[0]*sqr(Rd[0]) + coefficient[1]*sqr(Rd[1]) + coefficient[2]*sqr(Rd[2]) + coefficient[3]*Rd[0]*Rd[1] +
        coefficient[4]*Rd[0]*Rd[2] + coefficient[5]*Rd[1]*Rd[2]; 
	b = 2*coefficient[0]*Ro[0]*Rd[0] + 2*coefficient[1]*Ro[1]*Rd[1] + 2*coefficient[2]*Ro[2]*Rd[2] + 
		coefficient[3]*(Ro[0]*Rd[1] + Ro[1]*Rd[0]) + coefficient[4]*(Ro[0]*Rd[2] + Ro[2]*Rd[0]) + coefficient[5]*(Ro[1]*Rd[2] + Rd[1]*Ro[2]) + 
		coefficient[6]*Rd[0] + coefficient[7]*Rd[1] + coefficient[8]*Rd[2];
	c = coefficient[0]*sqr(Rd[0]) + coefficient[1]*sqr(Ro[1]) + coefficient[2]*sqr(Ro[2]) + 
		coefficient[3]*Ro[0]*Ro[1] + coefficient[4]*Ro[0]*Ro[2] + coefficient[5]*Ro[1]*Ro[2] + 
		coefficient[6]*Ro[0] + coefficient[7]*Ro[1] + coefficient[8]*Ro[2] + coefficient[9];
		
    if(a == 0)
	 return (-c/b); 
	//calculate the determinant value to get the point of intersection
    double d = (sqr(b) - 4*a*c);
    
    if (d < 0) 
	  return -1;  //no intersection since the det is imaginary 
    
    else if (d == 0) {
        t0 = -1*(b / (2*a));
        t1 = -1*(b / (2*a));
       }
    else {  
        t0 = (( - b - sqrt(d))/(2*a));
		t1 = (( - b + sqrt(d))/(2*a));
		}
        
    if (t0 < 0 && t1 < 0) 
        return -1;

    else if (t0 < 0 && t1 > 0) 
        return t1;
    
    else if (t0 > 0 && t1 < 0) 
        return t0;
    
    else { 
        if (t0 <= t1)
            return t0;
        else
            return t1;
    }		
}

void raycast(Image *image, double cameraWidth, double cameraHeight, OBJECT *objects, LIGHT *lights) {
   
    int x, y, counter; 
    
    Vector viewPlanePosition= {0,0,1}; //view plane position
    Vector Ro = {0,0,0}; //Camera position
	Vector Rd = {0,0,0}; // ray direction
    Vector point = {0,0,0}; //initial point on view plane
    
    double pixheight = cameraHeight / image->height;
    double pixwidth = cameraWidth / image->width;
    
   
    point[2] = viewPlanePosition[2]; // set viewplane to Z direction
    
    for(x=0;x<image->height;x++){
        point[1] = -(viewPlanePosition[1] - cameraHeight/2.0 + pixheight*(x + 0.5)); 
        for (y=0; y<image->width; y++) {
             point[0] = viewPlanePosition[0] - cameraWidth/2.0 + pixwidth*(y + 0.5);
            normalize(point);  
            Rd[0] = point[0];
            Rd[1] = point[1];
            Rd[2] = point[2];
            
            int objIndex =0;
            double objDistance = INFINITY;
            for (counter=0; objects[counter].type!=0; counter++) {
                double t =0;
                switch (objects[counter].type) {
                    case 0:
                        printf("no object found\n");
                        break;
                        
                    case CAM:
                        break;
                        
                    case SPH:
                        t = sphereIntersection(Ro, Rd, objects[counter].data.sphere.position, objects[counter].data.sphere.radius);
                        break;
                        
                    case PLN:
                        t = planeIntersection(Ro, Rd, objects[counter].data.plane.position, objects[counter].data.plane.normal);
                        break;
						
					case QUAD:
                        t = quadricIntersection(Ro, Rd, objects[counter].data.quadric.position, objects[counter].data.quadric.coefficients);
                        break;
                        
                    default:
                        exit(1);
                }
                if (t > 0 && t < objDistance) {
                    objDistance = t;
                    objIndex = counter;
                }
                
            }
			  // use this to compute the final color of the pixel	
			 double pixelColor[3] = {0,0,0};
			 
            if (objDistance > 0 && objDistance != INFINITY) {
				//there is an intersection and applying color to the intersection pixel
               computeIlluminationColor(Ro, Rd, objIndex, objDistance, pixelColor, objects, lights);
               colorPixel(pixelColor, x, y, image); 
            }
            else{
			//colouring the pixel to default color since there was no intersection
                Vector defaultColor ={1,1,1};
                colorPixel(defaultColor,x,y,image);
            }
            
        }
    }
    
}


//calculate the color of objects with illumination
void computeIlluminationColor(Vector Ro, Vector Rd, int objIndex, double objDistance, double *pixelColor, OBJECT *objects, LIGHT *lights) {
    
	int i;
	Vector light_Ro = {0,0,0};
    Vector light_Rd = {0,0,0};
    
    // finding light rays Ro and Rd
    VectorScale(Rd, objDistance, light_Ro);
    VectorAddition(light_Ro, Ro, light_Ro);
    
    for (i=0; lights[i].color != NULL; i++) {
        
		double normal[3] = {0,0,0}, diffuseTemp[3] = {0,0,0}, specularTemp[3] = {0,0,0};
		int lightIndex = -1, counter =0;  
		double lightDistance = INFINITY;
		
        // find new ray direction
        VectorSubstraction(lights[i].position, light_Ro, light_Rd);
        double distance = vectorLength(light_Rd);       
        
            for (counter=0; objects[counter].type!=0; counter++) {
                double t =0;
                switch (objects[counter].type) {
                    case 0:
                        printf("no object found\n");
                        break;
                        
                    case CAM:
                        break;
                        
                    case SPH:
                        t = sphereIntersection(light_Ro, light_Rd, objects[counter].data.sphere.position, objects[counter].data.sphere.radius);
                        break;
                        
                    case PLN:
                        t = planeIntersection(light_Ro, light_Rd, objects[counter].data.plane.position, objects[counter].data.plane.normal);
                        break;
						
					case QUAD:
                        t = quadricIntersection(light_Ro, light_Rd, objects[counter].data.quadric.position, objects[counter].data.quadric.coefficients);
                        break;
                        
                    default:
                        exit(1);
                }
                if (t > 0 && t < lightDistance) {
                    lightDistance = t;
                    lightIndex = counter;
                }
			}
         // start of shadow test
        if (lightIndex == -1) {
           
            if (objects[objIndex].type == PLN) {
                VectorCopy(objects[objIndex].data.plane.normal, normal);
                VectorCopy(objects[objIndex].data.plane.diffuse_color, diffuseTemp);
                VectorCopy(objects[objIndex].data.plane.specular_color, specularTemp);
            } else if (objects[objIndex].type == SPH) {
                VectorSubstraction(light_Ro, objects[objIndex].data.sphere.position, normal);
				VectorCopy(objects[objIndex].data.sphere.diffuse_color, diffuseTemp);
                VectorCopy(objects[objIndex].data.sphere.specular_color, specularTemp);
            } else {
                fprintf(stderr, "Error: not a valid object\n");
                exit(1);
            }
        }
    }
}


void calculateDiffuseColor(double *N, double *L, double *IL, double *KD, double *out_color) {
    // K_a*I_a should be added to the beginning of this whole thing, which is a constant and ambient light
    double n_dot_l = VectorDotProduct(N, L);
    if (n_dot_l > 0) {
        double diffuse_product[3];
        diffuse_product[0] = KD[0] * IL[0];
        diffuse_product[1] = KD[1] * IL[1];
        diffuse_product[2] = KD[2] * IL[2];
        // multiply by n_dot_l and store in out_color
        VectorScale(diffuse_product, n_dot_l, out_color);
    }
    else {
        // would normally return K_a*I_a here...
        out_color[0] = 0;
        out_color[1] = 0;
        out_color[2] = 0;
    }
}

void calculateSpecularColor(double ns, double *L, double *R, double *N, double *V, double *KS, double *IL, double *out_color) {
    double v_dot_r = VectorDotProduct(L, R);
    double n_dot_l = VectorDotProduct(N, L);
    if (v_dot_r > 0 && n_dot_l > 0) {
        double vr_to_the_ns = pow(v_dot_r, ns);
        double spec_product[3];
        spec_product[0] = KS[0] * IL[0];
        spec_product[1] = KS[1] * IL[1];
        spec_product[2] = KS[2] * IL[2];
        VectorScale(spec_product, vr_to_the_ns, out_color);
    }
    else {
        out_color[0] = 0;
        out_color[1] = 0;
        out_color[2] = 0;
    }
}
