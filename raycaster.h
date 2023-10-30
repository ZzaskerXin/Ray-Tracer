#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <fstream>
#include <string>
#include <sstream>
#include <unordered_set> 
#include <regex>
#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <cfloat>
#include <math.h>
using namespace std;

// Define needed Types

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {};
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {};

    Vec3 operator+(const Vec3& u) const {
        float new_x = x + u.x;
        float new_y = y + u.y;
        float new_z = z + u.z;
        Vec3 result_vec = Vec3(new_x, new_y, new_z);
        return result_vec;
    }

    Vec3 operator-(const Vec3& u) const {
        float new_x = x - u.x;
        float new_y = y - u.y;
        float new_z = z - u.z;
        Vec3 result_vec = Vec3(new_x, new_y, new_z);
        return result_vec;
    }
    Vec3 operator*(float num) const {
        float new_x = x * num;
        float new_y = y * num;
        float new_z = z * num;
        Vec3 result_vec = Vec3(new_x, new_y, new_z);
        return result_vec;
    }
    Vec3 operator/(float num) const {
        float new_x = x / num;
        float new_y = y / num;
        float new_z = z / num;
        Vec3 result_vec = Vec3(new_x, new_y, new_z);
        return result_vec;
    }
};

struct Color {
    float r, g, b;
    Color() : r(0), g(0), b(0) {};
    Color(float r, float g, float b) : r(r), g(g), b(b) {};
};

struct Sphere {
    Vec3 center;
    float r;
    Color Odc;
    Color Osc;
    float Ka;
    float Kd;
    float Ks;
    float n;
    int color_id;
    bool istexturedefined;
    int t_id;
};

struct Triangle
{
    Vec3 v1;
    Vec3 v2;
    Vec3 v3;
    Color Odc;
    Color Osc;
    float Ka;
    float Kd;
    float Ks;
    float n;
    Vec3 v1n;
    Vec3 v2n;
    Vec3 v3n;
    Vec3 v1t;
    Vec3 v2t;
    Vec3 v3t;
    int color_id;
    bool istexturedefined = false;
    bool isnormdefined = false;
    int t_id;
};


struct Ray {
    Vec3 pass;
    Vec3 dir;
};

struct Light {
    Vec3 pos;
    Color lcolor;
    float w;
};

// Help Functions

Vec3 Normalize(const Vec3& v) {
    float x, y, z;
    float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length != 0) {
        x = v.x / length;
        y = v.y / length;
        z = v.z / length;
        return Vec3(x, y, z);
    }
    return v;
}

Vec3 CrossProduct(const Vec3& u, const Vec3& v) {
    float x = u.y * v.z - u.z * v.y;
    float y = u.z * v.x - u.x * v.z;
    float z = u.x * v.y - u.y * v.x;

    return Vec3(x, y, z);
}

float DotProduct(const Vec3& u, const Vec3& v) {
    float result = u.x * v.x + u.y * v.y + u.z * v.z;
    return result;
}

int Shadow_Ray (Vec3& inter_p, Light& light, Vec3& L, Sphere& sphere, vector<Sphere>& spheres) {
    bool ifIntersected = false;
    
    float distance = numeric_limits<float>::max();

    Vec3 normalVector = Normalize((inter_p - sphere.center)/sphere.r);

    inter_p = inter_p + (normalVector * 0.001);

    float light_range = sqrt(DotProduct((inter_p - light.pos), (inter_p - light.pos)));
    for(long unsigned int i = 0; i < spheres.size(); i++) {
        // if (spheres[i].color_id != sphere.color_id) {
        float A = 1;
        float B = 2 * (L.x * (inter_p.x - spheres[i].center.x) + 
                       L.y * (inter_p.y - spheres[i].center.y) + 
                       L.z * (inter_p.z - spheres[i].center.z));
        float C = ((inter_p.x - spheres[i].center.x) * (inter_p.x - spheres[i].center.x)) + 
                  ((inter_p.y - spheres[i].center.y) * (inter_p.y - spheres[i].center.y)) + 
                  ((inter_p.z - spheres[i].center.z) * (inter_p.z - spheres[i].center.z)) - 
                  (spheres[i].r * spheres[i].r);

        float discriminant = (B * B) - (4 * A * C);
            
        if(discriminant < 0){
             continue;
        }

        else if(discriminant == 0){
            ifIntersected = true;
            float d = -1 * B / (2 * A);
            if(d < distance){
                distance = d;
            }
        }

        else{
            ifIntersected = true;
            float ta = (-1 * B + sqrt(discriminant)) / (2 * A);
            float tb = (-1 * B - sqrt(discriminant)) / (2 * A);

            if((ta < 0) && (tb < 0)){
                continue;
            }

            else if((ta > 0) && tb <= 0){
                float t = ta;
                if (t < distance){
                    distance = t;
                }
            }
            else if((ta < 0) && tb > 0){
                float t = tb;
                if (t < distance){
                    distance = t;
                }
            }

            else{
                float t;
                if(ta <= tb){
                    t = ta;
                }
                else{
                    t = tb;
                }
                if (t < distance){
                    distance = t;
                }
            }
        }
        // }   
    }
    

    if (light.w == 0) {
        if(ifIntersected) {
            if (distance < 0.1) {
                return 0;
            }
        }
        return 1;
    }

    else {
        float spheres_distance = sqrt(DotProduct(L * distance, L * distance));
        
        if (spheres_distance < light_range && distance > 0 && ifIntersected) {
            return 0;
        }
        return 1;
    }
}

Color Shade_Ray (Sphere& sphere, vector<Light>& lights, vector<Sphere>& spheres, Vec3& inter_p, Vec3& eye) {
    Color mtlcolor;

    Color light_sum;

    for (long unsigned i = 0; i < lights.size(); i++) {

        Vec3 N = Normalize((inter_p - sphere.center) / sphere.r);


        Vec3 V = Normalize(eye - inter_p);

        Vec3 L;
        if (lights[i].w == 0) {
            L = Normalize(lights[i].pos * -1);
        } else {
            L = Normalize(lights[i].pos - inter_p);
        }

        // cout << L.x << " " << L.y  << " " << L.z << endl;

        Vec3 H = Normalize((L + V) / 2);

        float NL = DotProduct(N, L);
        float NH = DotProduct(N, H);

        if (NH < 0) {
            NH = 0;
        }

          if (NL < 0) {
            NL = 0;
        }

        float specular_r = sphere.Ks * sphere.Osc.r * pow(NH, sphere.n);
       
        float specular_g = sphere.Ks * sphere.Osc.g * pow(NH, sphere.n);

        float specular_b = sphere.Ks * sphere.Osc.b * pow(NH, sphere.n);     

        float diffuse_r = sphere.Kd * sphere.Odc.r * NL;

        float diffuse_g = sphere.Kd * sphere.Odc.g * NL;

        float diffuse_b = sphere.Kd * sphere.Odc.b * NL;


        int S = Shadow_Ray(inter_p, lights[i], L, sphere, spheres);
        
        light_sum.r += S * (lights[i].lcolor.r * (diffuse_r + specular_r));
        light_sum.g += S * (lights[i].lcolor.g * (diffuse_g + specular_g));
        light_sum.b += S * (lights[i].lcolor.b * (diffuse_b + specular_b));
    }

    mtlcolor.r = sphere.Ka * sphere.Odc.r + light_sum.r;
    mtlcolor.g = sphere.Ka * sphere.Odc.g + light_sum.g;
    mtlcolor.b = sphere.Ka * sphere.Odc.b + light_sum.b;

    if (mtlcolor.r > 1) {
        mtlcolor.r = 1;
    }
    if (mtlcolor.r < 0) {
        mtlcolor.r = 0;
    }
    if (mtlcolor.g > 1) {
        mtlcolor.g = 1;
    }
    if (mtlcolor.g < 0) {
        mtlcolor.g = 0;
    }
    if (mtlcolor.b > 1) {
        mtlcolor.b = 1;
    }
    if (mtlcolor.b < 0) {
        mtlcolor.b = 0;
    }
    return mtlcolor;
}

Color Shade_Ray_tri(Triangle& tri, vector<Light>& lights, vector<Triangle>& triangles, Vec3& inter_p, Vec3& eye, Vec3& normal) {
    Color mtlcolor;

    Color light_sum;

    for (long unsigned i = 0; i < lights.size(); i++) {
        Vec3 N = Normalize(normal);

        Vec3 V = Normalize(eye - inter_p);

        Vec3 L;
        if (lights[i].w == 0) {
            L = Normalize(lights[i].pos * -1);
        } else {
            L = Normalize(lights[i].pos - inter_p);
        }

        Vec3 H = Normalize((L + V) / 2);

        float NL = DotProduct(N, L);
        float NH = DotProduct(N, H);

        if (NH < 0) {
            NH = 0;
        }

          if (NL < 0) {
            NL = 0;
        }

        float specular_r = tri.Ks * tri.Osc.r * pow(NH, tri.n);
       
        float specular_g = tri.Ks * tri.Osc.g * pow(NH, tri.n);

        float specular_b = tri.Ks * tri.Osc.b * pow(NH, tri.n);     

        float diffuse_r = tri.Kd * tri.Odc.r * NL;

        float diffuse_g = tri.Kd * tri.Odc.g * NL;

        float diffuse_b = tri.Kd * tri.Odc.b * NL;

        light_sum.r += (lights[i].lcolor.r * (diffuse_r + specular_r));
        light_sum.g += (lights[i].lcolor.g * (diffuse_g + specular_g));
        light_sum.b += (lights[i].lcolor.b * (diffuse_b + specular_b));
    }
    mtlcolor.r = tri.Ka * tri.Odc.r + light_sum.r;
    mtlcolor.g = tri.Ka * tri.Odc.g + light_sum.g;
    mtlcolor.b = tri.Ka * tri.Odc.b + light_sum.b;

    if (mtlcolor.r > 1) {
        mtlcolor.r = 1;
    }
    if (mtlcolor.r < 0) {
        mtlcolor.r = 0;
    }
    if (mtlcolor.g > 1) {
        mtlcolor.g = 1;
    }
    if (mtlcolor.g < 0) {
        mtlcolor.g = 0;
    }
    if (mtlcolor.b > 1) {
        mtlcolor.b = 1;
    }
    if (mtlcolor.b < 0) {
        mtlcolor.b = 0;
    }
    cout << mtlcolor.r << mtlcolor.g << mtlcolor.b << endl;
    return mtlcolor;
}

// Function to determine if a ray intersects with a sphere
Color Trace_Ray(Ray& ray, vector<Sphere>& spheres, Color& bkgcolor, Vec3& eye, vector<Light>& lights, vector<Triangle> triangles, vector<Color**> texture_list, float texture_width, float texture_height) {
    float distance = numeric_limits<float>::max();
    float tri_distance = numeric_limits<float>::max();
    
    Color return_color = bkgcolor;

    int intersect = 0;
    int tri_intersect = 0;

    int id = -1;
    int tri_id = -1;
    
    Vec3 inter_point = Vec3(0, 0, 0);
    Vec3 inter_point_tri = Vec3(0, 0, 0);

    Vec3 norm_dir = Normalize(ray.dir);
    Vec3 normal;
    float tri_alpha, tri_beta, tri_gamma;

    for (long unsigned int i = 0; i < spheres.size(); i++) {
        float A = 1;
        float B = 2 * (norm_dir.x * (eye.x - spheres[i].center.x) + norm_dir.y * (eye.y - spheres[i].center.y) + norm_dir.z * (eye.z - spheres[i].center.z));
        float C = ((eye.x - spheres[i].center.x) * (eye.x - spheres[i].center.x)) + ((eye.y - spheres[i].center.y) * (eye.y - spheres[i].center.y)) + ((eye.z - spheres[i].center.z) * (eye.z - spheres[i].center.z)) - (spheres[i].r * spheres[i].r);
    
        float discriminant = (B * B) - (4 * A * C);
        
        if(discriminant < 0){
            continue;
        }

        else if(discriminant == 0){
            intersect = 1;
            float d = -1 * B / (2 * A);
            if(d < distance){
                distance = d;
                id = spheres[i].color_id;
            }
        }
        else{
            intersect = 1;
            float ta = (-1 * B + sqrt(discriminant)) / (2 * A);
            float tb = (-1 * B - sqrt(discriminant)) / (2 * A);

            if((ta <= 0) && (tb <= 0)){
                continue;
            }
            else if((ta > 0) && tb < 0){
                float t = ta;
                if (t < distance){
                    distance = t;
                    id = spheres[i].color_id;
                }
            }
            else if((ta < 0) && tb > 0){
                float t = tb;
                if (t < distance){
                    distance = t;
                    id = spheres[i].color_id;
                }
            }
            else{
                float t;
                if(ta <= tb){
                    t = ta;
                }
                else{
                    t = tb;
                }
                if (t < distance){
                    distance = t;
                    id = spheres[i].color_id;
                }
            }
        }
    }

    for (long unsigned int i = 0; i < triangles.size(); i++) {
        Vec3 p1 = triangles[i].v1;
        Vec3 p2 = triangles[i].v2;
        Vec3 p3 = triangles[i].v3;

        Vec3 e1 = p2 - p1;
        Vec3 e2 = p3 - p1;
        Vec3 ep;

        Vec3 tri_n = CrossProduct(e1, e2);

        float tri_A = tri_n.x;
        float tri_B = tri_n.y;
        float tri_C = tri_n.z;
        float tri_D = -1 * (tri_A * p1.x + tri_B * p1.y + tri_C * p1.z);

        float denominator = tri_A * norm_dir.x + tri_B * norm_dir.y + tri_C * norm_dir.z;

        if (denominator != 0) {
            float d = -1 * (tri_A * eye.x + tri_B * eye.y + tri_C * eye.z + tri_D) / denominator;
            if (d >= 0) {
                ep = (eye + (norm_dir * d)) - p1;

                float d11 = DotProduct(e1, e1);
                float d12 = DotProduct(e1, e2);
                float d22 = DotProduct(e2, e2);
                float d1p = DotProduct(e1, ep);
                float d2p = DotProduct(e2, ep);

                float det = d11 * d22 - d12 * d12;
                float alpha, beta, gamma;
                if (det != 0) {
                    beta = (d22 * d1p - d12 * d2p) / det;
                    gamma = (d11 * d2p - d12 * d1p) / det;
                    alpha = 1.0 - beta - gamma;
                    if (alpha > 0 && alpha < 1 && beta > 0 && beta < 1 && gamma > 0 && gamma < 1) {
                        // cout << alpha << " " << beta << " " << gamma << endl;
                        tri_intersect = 1;
                        if (d < tri_distance) {
                            tri_distance = d;
                            tri_id = triangles[i].color_id;

                            if (triangles[i].isnormdefined == true) {
                                Vec3 n1 = triangles[i].v1n;
                                Vec3 n2 = triangles[i].v2n;
                                Vec3 n3 = triangles[i].v3n;
                                normal = Normalize(n1 * alpha + n2 * beta + n3 * gamma);
                                tri_alpha = alpha;
                                tri_beta = beta;
                                tri_gamma = gamma;
                            }
                            else {
                                normal = tri_n;
                                tri_alpha = alpha;
                                tri_beta = beta;    
                                tri_gamma = gamma;
                            }
                        }
                    }
                }
            }
        }
    }

    inter_point = eye + norm_dir * distance;
    inter_point_tri = eye + norm_dir * tri_distance;
    // for (long unsigned int i = 0; i < spheres.size(); i++) {
    //     if (spheres[i].color_id == id) {
    //         return_color = Shade_Ray(spheres[i], lights, spheres, inter_point, eye);
    //     }
    // }
    // return return_color;
    if (intersect == 1 || tri_intersect == 1) {
        if (intersect == 1) {
            for (long unsigned int i = 0; i < spheres.size(); i++) {
                if (spheres[i].color_id == id) {
                    if (spheres[i].istexturedefined == true) {
                        Vec3 nvec = Normalize((inter_point - spheres[i].center) / spheres[i].r);
                        float theta, phi, u, v;
                        phi = acos(nvec.z);
                        theta = atan2(nvec.y, nvec.x);
                        if (theta > 0) {
                            u = theta / (2 * M_PI);
                        }
                        else {
                            u = (theta + 2 * M_PI) / (2 * M_PI);
                        }
                        // u = 0.5 + theta / (2 * M_PI);

                        v = phi / M_PI;

                        int scaled_width = floor(u * (texture_width - 1));
                        int scaled_height = floor(v * (texture_height - 1));
                        for (long unsigned int a = 0; a < texture_list.size(); a++) {
                            if (a == spheres[i].t_id) {
                                spheres[i].Odc.r = (texture_list[a][scaled_height][scaled_width].r / 255);
                                spheres[i].Odc.g = (texture_list[a][scaled_height][scaled_width].g / 255);
                                spheres[i].Odc.b = (texture_list[a][scaled_height][scaled_width].b / 255);
                            }
                        }
                    }
                }
            }
        }
        if (tri_intersect == 1) {
            for (long unsigned int i = 0; i < triangles.size(); i++) {
                if (triangles[i].color_id == tri_id) {
                    if (triangles[i].istexturedefined == true) {
                        Vec3 t1 = triangles[i].v1t;
                        Vec3 t2 = triangles[i].v2t;
                        Vec3 t3 = triangles[i].v3t;
                        float u, v;
                        u = tri_alpha * t1.x + tri_beta * t2.x + tri_gamma * t3.x;
                        v = tri_alpha * t1.y + tri_beta * t2.y + tri_gamma * t3.y;

                        int scaled_width = floor(u * (texture_width));
                        int scaled_height = floor(v * (texture_height));

                        for (long unsigned int a = 0; a < texture_list.size(); a++) {
                            if (a == triangles[i].t_id) {
                                triangles[i].Odc.r = (texture_list[a][scaled_height][scaled_width].r / 255);
                                triangles[i].Odc.g = (texture_list[a][scaled_height][scaled_width].g / 255);
                                triangles[i].Odc.b = (texture_list[a][scaled_height][scaled_width].b / 255);
                            }
                        }
                    }
                }
            }
        }
        if (distance <= tri_distance) {
            for (long unsigned int i = 0; i < spheres.size(); i++) {
                if (spheres[i].color_id == id) {
                    return_color = Shade_Ray(spheres[i], lights, spheres, inter_point, eye);
                }
            }
        }
        else {
            for (long unsigned int i = 0; i < triangles.size(); i++) {
                if (triangles[i].color_id == tri_id) {
                    cout << "Render tri" << endl;
                    return_color = Shade_Ray_tri(triangles[i], lights, triangles, inter_point_tri, eye, normal);
                }
            }
        }
    }
    return return_color;
}
#endif
