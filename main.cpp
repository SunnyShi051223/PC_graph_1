#include <iostream>
#include <algorithm>
#include <vector>
#include <GL/glut.h>
#include "light.h"
#include "sphere.h"


//视口viewport
const int MAX_RECURSION_DEPTH = 1;
const float VIEWPORT_X = 15;
const float VIEWPORT_Y = 11.25;
const float FOCAL_LENGTH = -2.5;
const int WINDOW_W = 800;
const int WINDOW_H = 600;
const Ray camera(Vector3d(0.0f, 0.0f, 3.0f), Vector3d(0.0f, 0.0f, -1.0f));
const Vector3d upperRightCorner(VIEWPORT_X / 2, VIEWPORT_Y / 2, FOCAL_LENGTH);
const Vector3d bottomLeftCorner(-1 * VIEWPORT_X / 2, -1 * VIEWPORT_Y / 2, FOCAL_LENGTH);

//光源
Light light;
Color ambientLight;


//场景中的物体
std::vector<Object*> objects;
GLubyte pixelData[WINDOW_H][WINDOW_W][3];

void ConstructScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen
	light = Light(Color(1.0f, 1.0f, 1.0f), Vector3d(-3.0f, 15.0f, -0.5f));
	ambientLight = Color(0.2f, 0.2f, 0.2f);
	Sphere* sphere = new Sphere;
	if (sphere == NULL) {
		std::cerr << "Can't allocate memory for Sphere." << std::endl;
		exit(-1);
	}
	sphere->center_ = Vector3d(0.0f, -1.25f, 0.0f);
	sphere->radius_ = 1.0;
	sphere->material_.diffuseColor_ = Color(0.6f, 0.6f, 0.0f);
	sphere->material_.ambientColor_ = Color(0.6f, 0.6f, 0.0f);
	sphere->material_.specularColor_ = Color(0.3f, 0.3f, 0.3f);
	sphere->material_.alpha_ = 0.6;
	sphere->material_.refractIndex_ = 1.5;
	sphere->material_.specExponent_ = 10.0;
	objects.push_back(sphere);
}

Hit DetectSceneHit(Ray& ray)
{
	Hit ret, hit;
	ret.material_.color_ = Color(25.0f, 25.0f, 25.0f);
	ret.t_ = std::numeric_limits<float>::infinity();
	for (std::vector<Object*>::iterator iter = objects.begin(); iter != objects.end();
		iter++) {
		Sphere* sphere = (Sphere*)(*iter);
		sphere->intersect(ray, hit);
		if (hit.t_ < ret.t_) {
			ret.material_ = hit.material_;
			ret.N_ = hit.N_;
			ret.P_ = hit.P_;
			ret.t_ = hit.t_;
		}
	}
	return ret;
}

// Determines the color of the passed in ray
Hit RayCast(Ray& ray, int depth)
{
    Hit hit;
    // Returns background color if maximum recursion depth was hit
    if (depth == 0) {
        hit.material_.color_ = ambientLight;
        hit.t_ = std::numeric_limits<float>::infinity();
        return hit;
    }

    // Find object intersection (Primary Ray)
    Hit rayHit = DetectSceneHit(ray);

    if (rayHit.t_ != std::numeric_limits<float>::infinity()) {
        // --- 核心修改开始 ---

        // 1. 计算光照向量与距离
        // 我们需要保留未归一化的向量来计算由于光源的距离
        Vector3d lightVec = light.P_ - rayHit.P_;
        float distToLight = lightVec.length(); // 获取撞击点到光源的距离

        // L: 光源方向向量 (归一化)
        Vector3d L = lightVec;
        L.normalize();

        // V: 视线方向向量
        Vector3d V = ray.origin_ - rayHit.P_;
        V.normalize();

        // N: 法线向量
        Vector3d N = rayHit.N_;
        N.normalize();

        // 2. 阴影检测 (Shadow Ray)
        bool inShadow = false;

        // 构建阴影射线
        Ray shadowRay;
        // 【关键】偏移起点 (Shadow Acne Bias)：
        // 沿着光线方向微小移动起点，防止射线击中物体自身导致的“斑点”
        shadowRay.origin_ = rayHit.P_ + L * 0.001f; 
        shadowRay.directionVector_ = L;

        // 发射阴影射线检测遮挡
        Hit shadowHit = DetectSceneHit(shadowRay);

        // 如果击中了物体，并且击中点在光源之前 (t < distToLight)，则说明有遮挡
        if (shadowHit.t_ < distToLight) {
            inShadow = true;
        }

        // 3. 环境光 (Ambient)
        // 环境光永远存在，不受阴影影响
        Color finalColor = rayHit.material_.ambientColor_ * ambientLight;

        // 4. 漫反射 (Diffuse) & 5. 镜面反射 (Specular)
        // 只有当“不在阴影中”时，才叠加直射光照效果
        if (!inShadow) {
            float nDotL = N.dot(L);
            
            if (nDotL > 0) {
                // --- 计算漫反射 ---
                Color diffuseBase = rayHit.material_.diffuseColor_ * light.color_;
                finalColor = finalColor + (diffuseBase * nDotL);

                // --- 计算镜面反射 (Blinn-Phong) ---
                Vector3d H = L + V;
                H.normalize();
                float nDotH = N.dot(H);
                
                if (nDotH > 0) {
                    float specFactor = pow(nDotH, rayHit.material_.specExponent_);
                    Color specBase = rayHit.material_.specularColor_ * light.color_;
                    finalColor = finalColor + (specBase * specFactor);
                }
            }
        }

        // 赋值最终颜色
        rayHit.material_.color_ = finalColor;

        // --- 核心修改结束 ---
        
        return rayHit;
    }
    else {
        // 背景色
        rayHit.material_.color_ = Color(0.1f, 0.1f, 0.1f); // 稍微调暗一点背景以凸显光照
        return rayHit;
    }
}

void RenderScene()
{
	//Iterate through all screen pixels
	for (int i = 0; i < WINDOW_W; i++) {
		for (int j = 0; j < WINDOW_H; j++) {
			//Set background color
			pixelData[j][i][0] = 25;
			pixelData[j][i][1] = 25;
			pixelData[j][i][2] = 25;
			//Construct viewing ray
			float x = bottomLeftCorner.x_ + (upperRightCorner.x_ - bottomLeftCorner.x_)
				* ((i + 0.5) / WINDOW_W);
			float y = bottomLeftCorner.y_ + (upperRightCorner.y_ - bottomLeftCorner.y_)
				* ((j + 0.5) / WINDOW_H);
			Vector3d point(x, y, upperRightCorner.z_);
			Ray cameraRay;
			cameraRay.origin_ = camera.origin_;
			cameraRay.directionVector_ = point - (Vector3d)camera.origin_;
			Hit hit = RayCast(cameraRay, MAX_RECURSION_DEPTH);
			//If an object was hit, set the color for that pixel appropriately
			pixelData[j][i][0] = std::min((int)(hit.material_.color_.r_ * 255), 255);
			pixelData[j][i][1] = std::min((int)(hit.material_.color_.g_ * 255), 255);
			pixelData[j][i][2] = std::min((int)(hit.material_.color_.b_ * 255), 255);
		}
	}
	glDrawPixels(WINDOW_W, WINDOW_H, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
}


void DisplayFunc(void)
{
	ConstructScene();
	RenderScene();
	glFlush();
}

void MouseFunc(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
			exit(-1);
		break;
	default:
		break;
	}
}


int main(int argc, char* argv[])
{
	std::cout << "Ray Casting" << std::endl;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowSize(WINDOW_W, WINDOW_H);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Ray Casting - 3D scene");
	glutDisplayFunc(DisplayFunc);
	glutMouseFunc(MouseFunc); // register myMouse function
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f); // background is light gray
	glutMainLoop();
	for (unsigned int i = 0; i < objects.size(); i++) {
		delete objects[i];
	}
	return 0;

}

