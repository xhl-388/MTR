#include "geometry.h"

template<typename T, int R, int C>
Mat<T,R,C> Mat<T,R,C>::zero()
{
 	static_assert(R>0&&C>0);
	Mat res;
	std::fill(res.data,res.data+msize,0);
	return res;
}

template<typename T, int R, int C>
Mat<T,R,C> Mat<T,R,C>::identity()
{
	static_assert(R==C);
	static_assert(R>0);
	Mat<T,R,C> m;
	std::fill(m.data,m.data+msize,0);
	for(int i=0;i<R;i++)
		m[i][i]=1;
	return m;
}

template <typename T,int R,int C>
template <int C1>
Mat<T,R,C1> Mat<T,R,C>::operator*(const Mat<T,C,C1>& m) const
{
	Mat<T,R,C1> res;
	for (int i = 0; i < R; i++)
	{
		for (int j = 0; j < C1; j++)
		{
			T tmp = 0;
			for (int k = 0; k < C; k++)
			{
				std::cout<<data[i*C+k]<<","<<m[k][j]<<std::endl;
				tmp += (data[i*C+k] * m[k][j]);
			}
			res[i][j] = tmp;
		}
	}
	return res;
}

template<typename T, int R, int C>
template <typename T1>
Mat<T,R,C> Mat<T,R,C>::operator*(const T1& fac) const
{
    Mat res;
    for(int idx=0;idx<msize;idx++)
        res.data[idx]=data[idx]*fac;
    return res;
}

template<typename T, int R, int C>
Mat<T,R,C> Mat<T,R,C>::operator*(const T& fac) const
{
    Mat res;
    for(int idx=0;idx<msize;idx++)
        res.data[idx]=data[idx]*fac;
    return res;
}

template<typename T, int R, int C>
template <typename T1>
Mat<T,R,C> Mat<T,R,C>::operator+(const Mat<T1,R,C>& m) const
{
    Mat res;
    for(int idx=0;idx<msize;idx++)
        res.data[idx]=data[idx]+m.data[idx];
    return res;
}

template<typename T, int R, int C>
Mat<T,R,C> Mat<T,R,C>::operator+(const Mat& m) const
{
    Mat res;
    for(int idx=0;idx<msize;idx++)
        res.data[idx]=data[idx]+m.data[idx];
    return res;
}

template<typename T, int R, int C>
template <typename T1>
Mat<T,R,C> Mat<T,R,C>::operator-(const Mat<T1,R,C>& m) const
{
    Mat res;
    for(int idx=0;idx<msize;idx++)
        res.data[idx]=data[idx]-m.data[idx];
    return res;
}

template<typename T, int R, int C>
Mat<T,R,C> Mat<T,R,C>::operator-(const Mat& m) const
{
    Mat res;
    for(int idx=0;idx<msize;idx++)
        res.data[idx]=data[idx]-m.data[idx];
    return res;
}

template<typename T, int R, int C>
template <typename T1>
Mat<T,R,C> Mat<T,R,C>::crossProduct(const Mat<T1,R,C>& m) const
{
    return Vec<T,3>{y()*m.z()-z()*m.y(), z()*m.x()-x()*m.z(), x()*m.y()-y()*m.x()};
}

template<typename T, int R, int C>
Mat<T,R,C> Mat<T,R,C>::crossProduct(const Mat& m) const
{
    return Vec<T,3>{y()*m.z()-z()*m.y(), z()*m.x()-x()*m.z(), x()*m.y()-y()*m.x()};
}

template<typename T, int R, int C>
Mat<T,C,R> Mat<T,R,C>::transpose() const
{
	Mat<T,C,R> res;
    for(int i=0; i<R; i++)
        for(int j=0; j<C; j++)
            res[j][i] = data[i*C+j];
    return res;
}

template<typename T, int R, int C>
double Mat<T,R,C>::norm() const
{
    double tmp=0;
    for(int i=0;i<R;i++)
        tmp+=data[i]*data[i];
    return std::sqrt(tmp);
}   

template<typename T, int R, int C>
Mat<T,R,C> Mat<T,R,C>::normalized() const
{
    // return Vec<float,3>{1,1,1};
    return this->operator*(1.f/norm());
}

template<typename T, int R, int C>
Mat<T,R,C> Mat<T,R,C>::inverse() const
{
	static_assert(R==C);
    // augmenting the square matrix with the identity matrix of the same dimensions a => [ai]
	Mat<T,R,2*C> res=Mat<T,R,2*C>::zero();
    for(int i=0; i<R; i++)
        for(int j=0; j<C; j++)
            res[i][j] = data[i*C+j];
    for(int i=0; i<R; i++)
        res[i][i+C] = 1;
    // first pass
    for (int i=0; i<R-1; i++) {
        // normalize the first row
        for(int j=res.ncol()-1; j>=0; j--)
            res[i][j] /= res[i][i];				
        for (int k=i+1; k<R; k++) {
            float coeff = res[k][i];
            for (int j=0; j<res.ncol(); j++) {
                res[k][j] -= res[i][j]*coeff;
            }
        }
    }
    // normalize the last row
    for(int j=res.ncol()-1; j>=R-1; j--)
        res[R-1][j] /= res[R-1][R-1];		
    // second pass
    for (int i=R-1; i>0; i--) {
        for (int k=i-1; k>=0; k--) {
            float coeff = res[k][i];
            for (int j=0; j<res.ncol(); j++) {
                res[k][j] -= res[i][j]*coeff;
            }
        }
    }
    // cut the identity matrix back
    Mat<T,R,C> truncate;
    for(int i=0; i<R; i++)
        for(int j=0; j<C; j++)
            truncate[i][j] = res[i][j+C];
    return truncate;
}

template<typename T, int R, int C>
Mat<T,R,C>::Mat()
{
	static_assert(R>0&&C>0);
	data=new T[R*C];
}

template<typename T, int R, int C>
template <typename T1>
Mat<T,R,C>::Mat(const std::initializer_list<T1>& list):Mat()
{
    int idx=0;
	for(auto itm:list)
	{
		data[idx]=static_cast<T>(itm);
		idx++;
		if(idx>=msize)
			break;
	}
}

template<typename T, int R, int C>
Mat<T,R,C>::Mat(const std::initializer_list<T>& list):Mat()
{
	int idx=0;
	for(auto itm:list)
	{
		data[idx]=itm;
		idx++;
		if(idx>=msize)
			break;
	}
}

template<typename T, int R, int C>
template <typename T1>
Mat<T,R,C>::Mat(const Mat<T1,R,C>& m):Mat()
{
	for(int idx=0;idx<msize;idx++)
	{
		data[idx]=m[idx/C][idx%C];
	}
}

template<typename T, int R, int C>
Mat<T,R,C>::Mat(const Mat<T,R,C>& m):Mat()
{
	for(int idx=0;idx<msize;idx++)
	{
		data[idx]=m.data[idx];
	}
}

template<typename T, int R, int C>
Mat<T,R,C>::Mat(Mat&& m)
{
	data=m.data;
	m.data=nullptr;
}

template <typename T, int R, int C>
template <typename T1>
Mat<T,R,C>& Mat<T,R,C>::operator=(const Mat<T1,R,C>& m)
{
	for(int idx=0;idx<msize;idx++)
	{
		data[idx]=m[idx/C][idx%C];
	}
	return *this;
}

template <typename T, int R, int C>
Mat<T,R,C>& Mat<T,R,C>::operator=(const Mat<T,R,C>& m)
{
	if(this!=&m)
	{
		for(int idx=0;idx<msize;idx++)
		{
			data[idx]=m.data[idx];
		}
	}
	return *this;
}

template<typename T, int R, int C>
Mat<T,R,C>& Mat<T,R,C>::operator=(Mat&& m)
{
	if(this!=&m)
	{
		delete[] data;
		data=m.data;
		m.data=nullptr;
	}
	return *this;
}

template<typename T, int R, int C>
void Mat<T,R,C>::operator<<(const std::string& s)
{
	T val;
	char trash;
	std::istringstream iss(s);
	int idx=0;
	while(iss>>val&&idx<msize){
		iss>>trash;
		data[idx]=val;
		idx++;
	}
}

template <typename T, int R, int C>
T Mat<T,R,C>::product(const Mat& m) const
{
	T res=0;
	for(int idx=0;idx<msize;idx++)
	{
		res+=data[idx]*m.data[idx];
	}
	return res;
}

template <typename T, int R, int C>
std::ostream& operator<<(std::ostream& os,const Mat<T,R,C>& m)
{
	os<<"( ";
	for(int i=0;i<R;i++)
	{
		os<<"\t";
		for(int j=0;j<C;j++)
		{
			os<<m.at(i,j);
			if(j!=C-1)
				os<<"\t,";
		}
		os<<"\t";
		if(i!=R-1)
			os<<std::endl;
	}
	os<<" )\n";
	return os;
}

template <typename T, int R ,int C>
const T* Mat<T,R,C>::operator[](int idx) const
{
	assert(idx<R);
	return data+idx*C;
}

template <typename T, int R ,int C>
T* Mat<T,R,C>::operator[](int idx)
{
	assert(idx<R);
	return data+idx*C;
}

template <typename T, int R ,int C>
const T& Mat<T,R,C>::x() const {
	static_assert(C==1);
	return data[0];
}
template <typename T, int R ,int C>
const T& Mat<T,R,C>::y() const { 
	static_assert(C==1&&R>1);
	return data[1];
}
template <typename T, int R ,int C>
const T& Mat<T,R,C>::z() const {
	static_assert(C==1&&R>2);
	return data[2];
}
template <typename T, int R ,int C>
T& Mat<T,R,C>::x() {
	static_assert(C==1);
	return data[0];
}
template <typename T, int R ,int C>
T& Mat<T,R,C>::y() {
	static_assert(C==1&&R>1);
	return data[1];
}
template <typename T, int R ,int C>
T& Mat<T,R,C>::z() {
	static_assert(C==1&&R>2);
	return data[2];
}