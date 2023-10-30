#include "raycaster.h"


int main(int argc, char* argv[]) {

    // Check the format of the command line arguments
    if (argc != 2) {
        cerr << "Correct form: " << argv[0] << " input_filename" << endl;
        return 1;
    }

    string inputFilename = argv[1];
    if (inputFilename.find(".") == string::npos) {
        cerr << "Invalid file name" << endl;
        return 1;
    }

    size_t dotIndex = inputFilename.find_last_of(".");

    string fileExtension = inputFilename.substr(dotIndex);

    // Check if the file has valid extension
    if (fileExtension != ".txt") {
        cerr << "Invalid extension" << endl;
        return 1;
    }
    Vec3 eye;
    Vec3 viewdir;
    Vec3 updir;
    float vfov = 0.0;
    Color bkgcolor;
    Color Od(-1, -1, -1);
    Color Os(-1, -1, -1);
    float Ka;
    float Kd;
    float Ks;
    float pn;
    int sphere_id = 0;
    int tri_id = 0;
    vector<Light> lights;
    vector<Sphere> spheres;
    vector<Triangle> triangles;
    vector<Vec3> vertex;
    vector<Vec3> vn;
    vector<Vec3> vt;
    int width = 0, height = 0;
    int texture_file_width;
    int texture_file_height;
    vector<Color**> texture_pixels_list;
    bool istextured = false;
    bool ismtlcolor = false;
    int text_id = -1;
    
    // set includes all the required keywords
    unordered_set<string> requiredKeywords = {
        "imsize",
        "eye",
        "viewdir",
        "updir",
        "vfov",
        "bkgcolor",
    };

    // set to store found keywords
    unordered_set<string> foundKeywords;

    string line;

    // open the input file
    ifstream input;
    input.open(inputFilename);
    
    if (!input.is_open()) {
      cerr << "Fail to open input file " << inputFilename << endl;
      return 1;
    }

    while (getline(input, line)) {
        cout << line << endl;
       
        istringstream iss(line);
        string keyword;
        
        if (!(iss >> keyword)) {
            continue; 
        }

        if (requiredKeywords.find(keyword) != requiredKeywords.end()) {
            foundKeywords.insert(keyword);
        }

        if (keyword == "imsize") {
            iss >> width >> height;
            if (width <= 0 || height <= 0) {
                cerr << "Width and height can not be negative or 0" << endl;
                return 1;
            }
        }
        else if (keyword == "eye") {
            iss >> eye.x >> eye.y >> eye.z;
        }
        else if (keyword == "viewdir") {
            iss >> viewdir.x >> viewdir.y >> viewdir.z;
        }
        else if (keyword == "updir") {
            iss >> updir.x >> updir.y >> updir.z;
        }
        else if (keyword == "vfov") {
            iss >> vfov;
            if (vfov <= 0) {
                cerr << "Vfov can not be negative or 0" << endl;
                return 1;
            }
        }
        else if (keyword == "bkgcolor") {
            iss >> bkgcolor.r >> bkgcolor.g >> bkgcolor.b;
            if (bkgcolor.r < 0 || bkgcolor.g < 0 || bkgcolor.b < 0) {
                cerr << "Bkgcolor components can not be negative" << endl;
                return 1;
            }
        }
        else if (keyword == "mtlcolor") {
            iss >> Od.r >> Od.g >> Od.b >> Os.r >> Os.g >> Os.b >> Ka >> Kd >> Ks >> pn;
            if (Od.r < 0 || Od.g < 0 || Od.b < 0 || Os.r < 0 || Os.g < 0 || Os.b < 0 || Od.r > 1 || Od.g > 1 || Od.b > 1 || Os.r > 1 || Os.g > 1 || Os.b > 1 ||
            Ks < 0 || Ks > 1 || Kd < 0 || Kd > 1 || Ka < 0 || Ka > 1) {
                cerr << "Mtlcolor syntax error" << endl;
                return 1;
            }
            ismtlcolor = true;
        }
        else if (keyword == "sphere") {
            if (ismtlcolor == false) {
                cerr << "Mtlcolor needs to be read first" << endl;
                return 1;
            }
            Sphere newsphere;
            iss >> newsphere.center.x >> newsphere.center.y >> newsphere.center.z >> newsphere.r;
            if (newsphere.r <= 0) {
                cerr << "Radius can not be negative or 0" << endl;
                return 1;
            }
            newsphere.color_id = sphere_id;
            sphere_id = sphere_id + 1;
            newsphere.Osc = Os;
            newsphere.Odc = Od;
            newsphere.Ka = Ka;
            newsphere.Kd = Kd;
            newsphere.Ks = Ks;
            newsphere.n = pn;
            newsphere.istexturedefined = istextured;
            newsphere.t_id = text_id;
            spheres.push_back(newsphere);
        }
        else if (keyword == "light") {
            Light newlight;
            iss >> newlight.pos.x >> newlight.pos.y >> newlight.pos.z >> newlight.w >> newlight.lcolor.r >> newlight.lcolor.g >> newlight.lcolor.b;
            if (newlight.lcolor.r > 1 || newlight.lcolor.g > 1 || newlight.lcolor.b > 1 || newlight.lcolor.r < 0 || newlight.lcolor.g < 0 || newlight.lcolor.b < 0 || (newlight.w != 0 && newlight.w != 1)) {
                cerr << "r g b elements should be between 0 and 1 and w should only be 1 or 0" << endl;
                return 1;
            }
            lights.push_back(newlight);
        }
        else if (keyword == "v") {
            Vec3 V;
            iss >> V.x >> V.y >> V.z;
            vertex.push_back(V);
        }
        else if (keyword == "vn") {
            Vec3 Vn;
            iss >> Vn.x >> Vn.y >> Vn.z;
            vn.push_back(Vn);

        }
        else if (keyword == "vt") {
            Vec3 Vt;
            Vt.z = 0;
            iss >> Vt.x >> Vt.y;
            vt.push_back(Vt);

        }
        else if (keyword == "f") {
            if (ismtlcolor == false) {
                cerr << "Mtlcolor needs to be read first" << endl;
                return 1;
            }
            Triangle tri;
            int v1, v2, v3, t1, t2, t3, n1, n2, n3;
            tri.Odc = Od;
            tri.Osc = Os;
            tri.Ka = Ka;
            tri.Kd = Kd;
            tri.Ks = Ks;
            tri.n = pn;
            tri.color_id = tri_id;
            tri_id = tri_id + 1;
            if (sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3) == 9) {
                tri.v1 = vertex[v1 - 1];
                tri.v2 = vertex[v2 - 1];
                tri.v3 = vertex[v3 - 1];

                tri.v1n = vn[n1 - 1];
                tri.v2n = vn[n2 - 1];
                tri.v3n = vn[n3 - 1];

                tri.v1t = vt[t1 - 1];
                tri.v2t = vt[t2 - 1];
                tri.v3t = vt[t3 - 1];

                tri.isnormdefined = true;
                tri.istexturedefined = true;
                tri.t_id = text_id;
            } else if (sscanf(line.c_str(), "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3) == 6) {
                tri.v1 = vertex[v1 - 1];
                tri.v2 = vertex[v2 - 1];
                tri.v3 = vertex[v3 - 1];

                tri.v1n = vn[n1 - 1];
                tri.v2n = vn[n2 - 1];
                tri.v3n = vn[n3 - 1];

                tri.isnormdefined = true;
            } else if (sscanf(line.c_str(), "f %d/%d %d/%d %d/%d", &v1, &t1, &v2, &t2, &v3, &t3) == 6) {
                tri.v1 = vertex[v1 - 1];
                tri.v2 = vertex[v2 - 1];
                tri.v3 = vertex[v3 - 1];

                tri.v1t = vt[t1 - 1];
                tri.v2t = vt[t2 - 1];
                tri.v3t = vt[t3 - 1];

                tri.istexturedefined = true;
                tri.t_id = text_id;
            } else if (sscanf(line.c_str(), "f %d %d %d", &v1, &v2, &v3) == 3) {
                tri.v1 = vertex[v1 - 1];
                tri.v2 = vertex[v2 - 1];
                tri.v3 = vertex[v3 - 1];
            } else {
                cerr << "Invalid triangle syntax " << line << endl;
                return 1;
            }
            triangles.push_back(tri);
        }
        else if (keyword == "texture") {
            Color** texture_pixels;
            string texture_file_name;
            iss >> texture_file_name;
            texture_file_name.erase(texture_file_name.find_last_not_of(" \n\r\t")+1);
            ifstream texture_file;
            texture_file.open(texture_file_name);
            if (!texture_file.is_open()) {
                cerr << "Fail to open texture file " << texture_file_name << endl;
                return 1;
            }
            string texture_file_header;
            getline(texture_file, texture_file_header);
            vector<string> words;
            stringstream ss(texture_file_header);
            string word;
            while (ss >> word) {
                words.push_back(word);
            }
            if (words[0] != "P3") {
                cout << "PPM file format wrong" << endl;
                return 1;
            }
            texture_file_width = stoi(words[1]);
            texture_file_height = stoi(words[2]);

            texture_pixels = new Color*[texture_file_height];
            for(int i = 0; i < texture_file_height; ++i) {
                texture_pixels[i] = new Color[texture_file_width];
            }
            for (int i = 0; i < texture_file_height; i++) {
                for (int j = 0; j < texture_file_width; j++) {      
                    float tr, tg, tb;
                    texture_file >> tr >> tg >> tb;
                    texture_pixels[i][j] = Color(tr, tg, tb);
                }
            }
            cout << texture_pixels[2][2].r << " " << texture_pixels[2][2].g << " " << texture_pixels[2][2].b << endl;
            texture_file.close();
            istextured = true;
            texture_pixels_list.push_back(texture_pixels);
            text_id += 1;
        }

        else {
            cerr << "Invalid syntax: " << line << endl;
            return 1;
        }
    }

    for (const string& reqKeyword : requiredKeywords) {
        if (foundKeywords.find(reqKeyword) == foundKeywords.end()) {
            cerr << "Required data " << reqKeyword << " is not found in the input file" << endl;
            return 1;
        }
    }


    
    string fileNameWithoutExtension = inputFilename.substr(0, dotIndex);
    string outputFilename = fileNameWithoutExtension + ".ppm";

    ofstream output;
    output.open(outputFilename);
    if (!output.is_open()) {
        cerr << "Fail to open output file" << outputFilename << endl;
        return 1;
    }

    // Write the Header of the PPM File
    output << "P3\n";
    output << "# ASCII PPM Image\n";
    output << width << " " << height << "\n";
    output << "255\n";

    // Normalized viewdirection to calculate u and v for viewing window
    Vec3 n;
    n = Normalize(Vec3(viewdir.x, viewdir.y, viewdir.z));

    Vec3 u = CrossProduct(n, updir);
    u = Normalize(u);

    Vec3 v = CrossProduct(u, n);
    v = Normalize(v);

    float aspect_ratio = (1 * width) / (1 * height);

    float d = 5;

    float vfov_radian = vfov * (M_PI / 180);

    float h;
    float w;

    h = 2 * d * tan(vfov_radian / 2);
    w = h * aspect_ratio;

    //Calculate the four corners of the viewing window
    Vec3 ul = eye + n * d - u * (w / 2) + v * (h / 2);
    Vec3 ur = eye + n * d + u * (w / 2) + v * (h / 2);
    Vec3 ll = eye + n * d - u * (w / 2) - v * (h / 2);
    Vec3 lr = eye + n * d + u * (w / 2) - v * (h / 2);

    // Parameters to determine the ray
    Vec3 ch = (ur - ul) / (2 * width);
    Vec3 cv = (ll - ul) / (2 * height);

    Vec3 dh = (ur - ul) / (width - 1);
    Vec3 dv = (ll - ul) / (height - 1);

    // Vector to store the generated color for each pixels
    Color color_pixels[width][height];

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++){
            Ray ray;
            ray.pass = ul + (dh * j) + (dv * i) + ch + cv;
            ray.dir = ray.pass - eye;
            color_pixels[j][i] = Trace_Ray(ray, spheres, bkgcolor, eye, lights, triangles, texture_pixels_list, texture_file_width, texture_file_height);
        }
    }

    // Convert the color components from range 0-1 to 0-255
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            output << int((color_pixels[j][i].r) * 255) << " " << int((color_pixels[j][i].g) * 255) << " " << int((color_pixels[j][i].b) * 255) << endl;
        }
    }

    output << "\n";

    input.close();
    output.close();

    // for (Light& light : lights) {
    //     cout << light.lcolor.r << endl;
    //     cout << light.lcolor.g << endl;
    //     cout << light.lcolor.b << endl;
    // }

    // for (Sphere& sphere : spheres) {
    //     cout << sphere.Odc.r << endl;
    //     cout << sphere.Odc.g << endl;
    //     cout << sphere.Odc.b << endl; 
    //     cout << sphere.Osc.r << endl;
    //     cout << sphere.Osc.g << endl;
    //     cout << sphere.Osc.b << endl;
    //     cout << sphere.Ka << endl;
    //     cout << sphere.Kd << endl;
    //     cout << sphere.Ks << endl;
    //     cout << sphere.n << endl;
    // }

    // cout << lights.size() << spheres.size() << endl;

    // for (Triangle& tri: triangles) {
    //     cout << tri.Ka << endl;
    //     cout << tri.Kd << endl;
    //     cout << tri.Ks << endl;
    //     cout << tri.n << endl;
    //     cout << tri.Odc.r << endl;
    //     cout << tri.Odc.g << endl;
    //     cout << tri.Odc.b << endl;
    //     cout << tri.Osc.r << endl;
    //     cout << tri.Osc.g << endl;
    //     cout << tri.Osc.b << endl;
    //     cout << tri.v1.x << " " << tri.v1.y << " " << tri.v1.z << endl;
    //     cout << tri.v2.x << " " << tri.v2.y << " " << tri.v2.z << endl;
    //     cout << tri.v3.x << " " << tri.v3.y << " " << tri.v3.z << endl;
    //     cout << tri.v1n.x << " " << tri.v1n.y << " " << tri.v1n.z << endl;
    //     cout << tri.v2n.x << " " << tri.v2n.y << " " << tri.v2n.z << endl;
    //     cout << tri.v3n.x << " " << tri.v3n.y << " " << tri.v3n.z << endl;
    // }
    // for (Light& l: lights) {
    //     cout << l.pos.x << " " << l.pos.y << " " << l.pos.z << endl;
    // }
    // cout << triangles.size() << endl;
    return 0;
}
