//
//  graphics_matrices.c
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-10-27.
//

#include "graphics_matrices.h"

/*
float inner_product(uint32_t n, float u[n], float v[n]) {
    float sum = 0;

    for(uint32_t i = 0; i < n; i++) {
        sum += u[i]*v[i];
    }
    return sum;
}

float vector_norm(uint32_t n, float u[n]) {
    return sqrt(inner_product(n, u, u));
}

void scalar_product(uint32_t n, float a, float u[n], float v[n]) {
    for(uint32_t i = 0; i < n; i++) {
        v[i] = a*u[i];
    }
}

void vector_normalise(uint32_t n, float u[n], float v[n]) {
    float norm = vector_norm(n, u);
    if(norm < TOL) {
        return;
    }
    scalar_product(n, 1/norm, u, v);
}


void vector_add(uint32_t n, float u[n], float v[n], float w[n]) {
    for(uint32_t i = 0; i < n; i++) {
        w[i] = u[i]+v[i];
    }
}

void vector_subtract(uint32_t n, float u[n], float v[n], float w[n]) {
    for(uint32_t i = 0; i < n; i++) {
        w[i] = u[i]-v[i];
    }
}

void cross_product(float u[3], float v[3], float w[3]) {
    w[0] = u[1]*v[2] - u[2]*v[1];
    w[1] = u[2]*v[0] - u[0]*v[2];
    w[2] = u[0]*v[1] - u[1]*v[0];
}
*/

float inner_product(vector3_t u, vector3_t v) {
    return u.x*v.x + u.y*v.y + u.z*v.z;
}

float vector_norm(vector3_t u) {
    return sqrt(inner_product(u, u));
}

vector3_t scalar_product(vector3_t u, float a) {
    return (vector3_t) {
        .x = a*u.x,
        .y = a*u.y,
        .z = a*u.z
    };
}

vector3_t vector_normalise(vector3_t u) {
    float norm = vector_norm(u);
    if(norm < TOL) {
        return (vector3_t) {
            0.0f,
            0.0f,
            0.0f
        };
    } else {
        return scalar_product(u, 1/norm);
    }
    
}


static inline vector3_t vector_add(vector3_t u, vector3_t v) {
    return (vector3_t) {
        .x = u.x + v.x, 
        .y = u.y + v.y, 
        .z = u.z + v.z
    };
}

vector3_t vector_subtract(vector3_t u, vector3_t v) {
    return (vector3_t) {
        .x = u.x - v.x, 
        .y = u.y - v.y, 
        .z = u.z - v.z
    };
}

vector3_t cross_product(vector3_t u, vector3_t v) {
    return (vector3_t){
        .x = u.y*v.z - u.z*v.y,
        .y = u.z*v.x - u.x*v.z,
        .z = u.x*v.y - u.y*v.x
    };
}

quaternion_t quaternion_conjugate(quaternion_t q) {
    return (quaternion_t){.r = q.r, .i = -q.i, .j = -q.j, .k = -q.k};
}

quaternion_t quaternion_add(quaternion_t p, quaternion_t q) {
    return (quaternion_t){.r = p.r + q.r, .i = p.i + q.i, .j = p.j + q.j, .k = p.k + q.k};
}

float quaternion_norm(quaternion_t q) {
    return sqrt(q.r*q.r + q.i*q.i + q.j*q.j + q.k*q.k);
}

quaternion_t quaternion_scalar_product(float a, quaternion_t q) {
    return (quaternion_t){.r = a*q.r, .i = a*q.i, .j = a*q.j, .k = a*q.k};
}

quaternion_t quaternion_normalise(quaternion_t q) {
    float norm = quaternion_norm(q);
    if(norm < TOL) {
        return q;
    }

    return quaternion_scalar_product(1/norm, q);
}

quaternion_t quaternion_product(quaternion_t p, quaternion_t q) {
    return (quaternion_t){
        .r = p.r*q.r - p.i*q.i - p.j*q.j - p.k*q.k,
        .i = p.r*q.i + p.i*q.r + p.j*q.k - p.k*q.j,
        .j = p.r*q.j - p.i*q.k + p.j*q.r + p.k*q.i,
        .k = p.r*q.k + p.i*q.j - p.j*q.i + p.k*q.r
    };
}

quaternion_t quaternion_rotation(vector3_t axis, float theta) {
    float cosine = cos(theta*0.5);
    float sine = sin(theta*0.5);
    
    float reciprocal_norm = 1/vector_norm(axis);

    return (quaternion_t){
        .r = cosine,
        .i = sine*axis.x*reciprocal_norm,
        .j = sine*axis.y*reciprocal_norm,
        .k = sine*axis.z*reciprocal_norm
    };
}
/*
quaternion_t vector_division(quaternion_t p, quaternion_t q) {
    quaternion_t axis = {};
}
*/
void matrix_product(float A[4][4], float B[4][4], float C[4][4]) {
    for(uint32_t i = 0; i < 4; i++) {
        for (uint32_t j = 0; j < 4; j++) {
            C[i][j] = A[i][0]*B[0][j]
                    + A[i][1]*B[1][j]
                    + A[i][2]*B[2][j]
                    + A[i][3]*B[3][j];
        }
    };
}

/*
matrix_t matrix_product(matrix_t A, matrix_t B) {
    matrix_t r;

    for(uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 4; j++) {
            r.array[i][j] = A.array[i][0]*B.array[0][j]
                          + A.array[i][1]*B.array[1][j]
                          + A.array[i][2]*B.array[2][j]
                          + A.array[i][3]*B.array[3][j];
        }
    }

    return r;
}
*/

transformation_t identity_matrix() {
    return (transformation_t){
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

transformation_t translation_matrix(vector3_t u) {
    return (transformation_t){
        1.0f, 0.0f, 0.0f,  u.x,
        0.0f, 1.0f, 0.0f,  u.y,
        0.0f, 0.0f, 1.0f,  u.z,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

transformation_t scaling_matrix(vector3_t u) {
    return (transformation_t){
         u.x, 0.0f, 0.0f, 0.0f,
        0.0f,  u.y, 0.0f, 0.0f,
        0.0f, 0.0f,  u.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

transformation_t quaternion_matrix(quaternion_t q) {
    q = quaternion_normalise(q);
    return (transformation_t){
        1 - 2*(q.j*q.j + q.k*q.k), 2*(q.i*q.j - q.k*q.r)    , 2*(q.i*q.k + q.j*q.r)    , 0.0f,
        2*(q.i*q.j + q.k*q.r)    , 1 - 2*(q.i*q.i + q.k*q.k), 2*(q.j*q.k - q.i*q.r)    , 0.0f,
        2*(q.i*q.k - q.j*q.r)    , 2*(q.j*q.k + q.i*q.r)    , 1 - 2*(q.i*q.i + q.j*q.j), 0.0f,
        0.0f                     , 0.0f                     , 0.0f                     , 1.0f
    };
}

transformation_t rotation_matrix(vector3_t axis, float theta) {
    quaternion_t q = quaternion_rotation(axis, theta);
    return quaternion_matrix(q);
}

transformation_t camera_transform(vector3_t eye_basis[3], vector3_t eye, vector3_t object) {
    return (transformation_t) {
        eye_basis[0].x, eye_basis[0].y, eye_basis[0].z, -(eye_basis[0].x*eye.x + eye_basis[0].x*eye.y + eye_basis[0].z*eye.z),
        eye_basis[1].x, eye_basis[1].y, eye_basis[1].z, -(eye_basis[1].x*eye.x + eye_basis[1].x*eye.y + eye_basis[1].z*eye.z),
        eye_basis[2].x, eye_basis[2].y, eye_basis[2].z, -(eye_basis[2].x*eye.x + eye_basis[2].x*eye.y + eye_basis[2].z*eye.z),
                     0,              0,              0,                                                                     1
    };
}

transformation_t camera_matrix(vector3_t eye, vector3_t object, vector3_t up) {
    vector3_t Z = vector_normalise(vector_subtract(object, eye));
    vector3_t X = vector_normalise(cross_product(Z, up));
    vector3_t Y = cross_product(Z, X);
    
    vector3_t basis[3] = {X, Y, Z};
    return camera_transform(basis, eye, object);
}

transformation_t perspective_matrix(float fov, float aspect_ratio, float near, float far) {
    float tan_half_angle = tan(fov/2);
    float phi = 1/tan_half_angle;
    
    float A = far/(far - near);
    float B = -(near*far)/(far - near);
    
    return (transformation_t) {
        phi/aspect_ratio, 0.0f, 0.0f, 0.0f,
                    0.0f,  phi, 0.0f, 0.0f,
                    0.0f, 0.0f,    A,    B,
                    0.0f, 0.0f, 1.0f, 0.0f
    };
}