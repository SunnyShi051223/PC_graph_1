#ifndef __VECTOR3D_H__
#define __VECTOR3D_H__
class Vector3d
{
public:
	float x_, y_, z_;
	Vector3d(float x = 0, float y = 0, float z = 0);
	float modul2() const;
	float modul() const;
	Vector3d cross(const Vector3d& v) const;
	float dot(const Vector3d& v) const;
	Vector3d operator-(const Vector3d& v) const;
	Vector3d operator+(const Vector3d& v) const;
	Vector3d operator*(float x) const;
	Vector3d operator/(float x) const;
	float mag() const;
	void normalize();
};
#endif
