//
//  graphics_matrices.h
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-10-27.
//

#ifndef graphics_matrices_h
#define graphics_matrices_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define TOL 1E-6

typedef struct quaternion_t {
    float r;
    float i;
    float j;
    float k;
} quaternion_t;

typedef struct vector3_t {
    float x;
    float y;
    float z;
} vector3_t;

typedef struct matrix_t {
    float array[4][4];
} matrix_t;

typedef struct transformation_t {
    float a00, a01, a02, a03;
    float a10, a11, a12, a13;
    float a20, a21, a22, a23;
    float a30, a31, a32, a33;
} transformation_t;

transformation_t identity_matrix();
transformation_t translation_matrix(vector3_t u);
transformation_t scaling_matrix(vector3_t u);
transformation_t rotation_matrix(vector3_t axis, float theta);
transformation_t camera_transform(vector3_t eye_basis[3], vector3_t eye, vector3_t object);
transformation_t camera_matrix(vector3_t eye, vector3_t object, vector3_t up);
transformation_t perspective_matrix(float fov, float aspect_ratio, float near, float far);
transformation_t transform(transformation_t A, transformation_t B);

#endif /* graphics_matrices_h */
