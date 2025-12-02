#include <iostream>
#include <algorithm>
#include <vector>
#include <GL/glut.h>
#include "light.h"
#include "sphere.h"

#include "plane.h"

//视口viewport
const int MAX_RECURSION_DEPTH = 3; //new
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	light = Light(Color(1.0f, 1.0f, 1.0f), Vector3d(-3.0f, 15.0f, 10.0f)); // 调整光源位置以便产生更好看的阴影
	ambientLight = Color(0.1f, 0.1f, 0.1f); // 调暗环境光，增强对比

	// 清除旧物体（如果需要多次调用的话，防止内存泄漏）
	for (auto obj : objects) delete obj;
	objects.clear();

	// --- 物体 1: 镜面球体 ---
	Sphere* sphere = new Sphere;
	sphere->center_ = Vector3d(0.0f, 0.0f, 0.0f); // 放在中心
	sphere->radius_ = 1.0;
	sphere->material_.diffuseColor_ = Color(0.7f, 0.1f, 0.1f); // 红色
	sphere->material_.ambientColor_ = Color(0.1f, 0.0f, 0.0f);
	sphere->material_.specularColor_ = Color(1.0f, 1.0f, 1.0f);
	sphere->material_.specExponent_ = 50.0; // 更亮更集中的高光
	sphere->material_.reflectionCoeff_ = 0.5; // 50% 反射率
	objects.push_back(sphere);

	// --- 物体 2: 另外一个球体 (测试遮挡和相互反射) ---
	Sphere* sphere2 = new Sphere;
	sphere2->center_ = Vector3d(1.5f, -0.5f, 1.0f);
	sphere2->radius_ = 0.5;
	sphere2->material_.diffuseColor_ = Color(0.1f, 0.1f, 0.8f); // 蓝色
	sphere2->material_.ambientColor_ = Color(0.0f, 0.0f, 0.1f);
	sphere2->material_.specularColor_ = Color(1.0f, 1.0f, 1.0f);
	sphere2->material_.specExponent_ = 20.0;
	sphere2->material_.reflectionCoeff_ = 0.2;
	objects.push_back(sphere2);

	// --- 物体 3: 地板平面 ---
	// 法线向上 (0,1,0)，位置在 y = -1.0 处
	Plane* floor = new Plane(Vector3d(0.0f, -1.0f, 0.0f), Vector3d(0.0f, 1.0f, 0.0f));
	floor->material_.diffuseColor_ = Color(0.5f, 0.5f, 0.5f); // 灰色地板
	floor->material_.ambientColor_ = Color(0.1f, 0.1f, 0.1f);
	floor->material_.specularColor_ = Color(0.0f, 0.0f, 0.0f); // 地板通常没有强高光
	floor->material_.specExponent_ = 1.0;
	floor->material_.reflectionCoeff_ = 0.3; // 地板也有一定反射，能看到球的倒影
	objects.push_back(floor);
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

//Determines the color of the passed in ray
// main.cpp 中替换 RayCast
Hit RayCast(Ray& ray, int depth)
{
	Hit hit;
	// 递归深度耗尽或未击中物体，返回背景色（或环境光）
	if (depth == 0) {
		hit.material_.color_ = Color(0.0f, 0.0f, 0.0f); // 黑色背景
		hit.t_ = std::numeric_limits<float>::infinity();
		return hit;
	}

	Hit rayHit = DetectSceneHit(ray);

	// 如果击中了物体
	if (rayHit.t_ != std::numeric_limits<float>::infinity()) {

		// --- 1. 基础环境光 ---
		Color finalColor = rayHit.material_.ambientColor_ * ambientLight;

		// 准备向量
		Vector3d L = light.P_ - rayHit.P_;
		float distToLight = L.modul(); // 光源距离
		L.normalize();
		Vector3d V = ray.origin_ - rayHit.P_;
		V.normalize();
		Vector3d N = rayHit.N_;
		N.normalize();

		// --- 2. 阴影检测 (Shadows) ---
		// 发射一条从击中点指向光源的射线
		// 偏移一点点(0.001)避免打到物体自己(Shadow Acne)
		Ray shadowRay(rayHit.P_ + L * 0.001f, L);
		Hit shadowHit = DetectSceneHit(shadowRay);

		// 如果阴影射线击中了物体，且该物体比光源更近，说明我们在阴影里
		bool inShadow = (shadowHit.t_ < distToLight);

		if (!inShadow) {
			// 不在阴影中，计算漫反射和镜面反射 (Blinn-Phong)
			float nDotL = N.dot(L);
			if (nDotL > 0) {
				// Diffuse
				Color diffuse = (rayHit.material_.diffuseColor_ * light.color_) * nDotL;
				finalColor = finalColor + diffuse;

				// Specular
				Vector3d H = L + V;
				H.normalize();
				float nDotH = N.dot(H);
				if (nDotH > 0) {
					float specFactor = pow(nDotH, rayHit.material_.specExponent_);
					Color specular = (rayHit.material_.specularColor_ * light.color_) * specFactor;
					finalColor = finalColor + specular;
				}
			}
		}

		// --- 3. 递归反射 (Reflections) ---
		if (rayHit.material_.reflectionCoeff_ > 0) {
			// 计算反射向量 R = V - 2(V·N)N 
			// 注意：这里的 ray.directionVector_ 是入射方向 I，公式通常为 I - 2(I·N)N
			Vector3d I = ray.directionVector_;
			I.normalize();
			Vector3d R = I - N * (2.0f * I.dot(N));
			R.normalize();

			// 递归调用 RayCast
			Ray reflectRay(rayHit.P_ + R * 0.001f, R);
			Hit reflectHit = RayCast(reflectRay, depth - 1);

			// 混合反射颜色
			if (reflectHit.t_ != std::numeric_limits<float>::infinity()) {
				Color reflectedColor = reflectHit.material_.color_ * rayHit.material_.reflectionCoeff_;
				finalColor = finalColor + reflectedColor;
			}
		}

		rayHit.material_.color_ = finalColor;
		return rayHit;
	}
	else {
		// 背景色
		hit.material_.color_ = Color(0.1f, 0.1f, 0.1f);
		hit.t_ = std::numeric_limits<float>::infinity();
		return hit;
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