#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>

template <typename T,int R, int C>
class Mat
{
private:	
    T* data;
	enum{row=R,col=C,msize=R*C};
public:
    Mat();
    template <typename T1>
    Mat(const std::initializer_list<T1>& list);
	Mat(const std::initializer_list<T>& list);
	template <typename T1>
	Mat(const Mat<T1,R,C>& m);
	Mat(const Mat& m);
	Mat(Mat&& m);
	~Mat() { 
        if(data)
            delete[] data;
    }
	template <typename T1>
	Mat& operator=(const Mat<T1,R,C>& m);
	Mat& operator=(const Mat& m);
	Mat& operator=(Mat&& m);
    const T& at(int r,int c=0) const { return data[r*col+c];}
	T& at(int r,int c=0) { return data[r*col+c];}
	T* raw() {return data;}
	void operator<<(const std::string& s);
	int nrow() const { return row; }
	int ncol() const { return col; }
	T product(const Mat& m) const;
	template <typename t, int r, int c>
	friend std::ostream& operator<<(std::ostream& os,const Mat<t,r,c>& m);
	const T* operator[](int r) const;
	T* operator[](int r);
	const T& x() const;
	const T& y() const;
	const T& z() const;
	T& x();
	T& y();
	T& z();

	static Mat zero();
	static Mat identity();
	template <int C1>
	Mat<T,R,C1> operator*(const Mat<T,C,C1>& m) const;
    template <typename T1>
    Mat operator*(const T1& fac) const;
    Mat operator*(const T& fac) const;
    template <typename T1>
    Mat operator+(const Mat<T1,R,C>& m) const;
    Mat operator+(const Mat& m) const;
    template <typename T1>
    Mat operator-(const Mat<T1,R,C>& m) const;
    Mat operator-(const Mat& m) const;
    template <typename T1>
    Mat crossProduct(const Mat<T1,R,C>& m) const;   // now only for vector
    Mat crossProduct(const Mat& m) const;
	Mat<T,C,R> transpose() const; 
    double norm() const;    // now only for vector
    Mat normalized() const; //now only for vector
	Mat inverse() const;
};

template <typename T, int R>
using Vec=Mat<T,R,1>;

using Vec2f=Vec<float,2>;
using Vec3f=Vec<float,3>;
using Vec2i=Vec<int,2>;
using Vec3i=Vec<int,3>;

#endif