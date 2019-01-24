#ifndef _ZC_AHC_UTILITY_H_
#define _ZC_AHC_UTILITY_H_

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <set>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <Eigen/Core>

#include "AHCPlaneSeg.hpp" //zc: �������ڹ��� �����PlaneSegʵ��

using std::vector;
using std::set;
using ahc::PlaneSeg;

using Eigen::aligned_allocator;
using Eigen::Affine3d;
using Eigen::Affine3f;
using Eigen::Matrix3f;
using Eigen::Matrix3d;
using Eigen::Matrix3Xf;
using Eigen::Vector3d;
using Eigen::Vector3f;
using Eigen::Map;

using Eigen::JacobiSVD;
using Eigen::ComputeFullU;
using Eigen::ComputeFullV;

typedef vector<Affine3d, aligned_allocator<Affine3d>> Affine3dVec;
typedef vector<Affine3f, aligned_allocator<Affine3f>> Affine3fVec;

#define PI 3.14159265358979323846

//zc: ����ת��
const float M2MM = 1e3;
const float MM2M = 1e-3;

//��������׼, opengl ��Ⱦ���    //2017-1-10 23:09:04
const int WIN_WW = 640,
		  WIN_HH = 480;

class Cube;

//@brief ��� t3+R9 ��
void processPoses(const char *fn, const Affine3dVec &poses);

//@brief ���ƽ�����: ����,����,����, mse(��ɶ��?)
void printPlaneParams(const double *normals, const double *center, double curvature, double mse);
void printPlaneParams(const PlaneSeg &planeSeg);

//@brief ���, �������Ȳ��̶�
double dotProd(const double *v1, const double *v2, size_t len = 3);

//@brief ����ģ��
double norm(const double *v, size_t len = 3);

//@brief ����ŷ�Ͼ���
double dist(const double *v1, const double *v2, size_t len = 3);

//@brief ��� (v1 x v2)=v3; ֻ����ά3D;
//@param[out] v3, ���������; ��Ҫ�ⲿԤ�����ڴ�
void crossProd(const double *v1, const double *v2, double *v3);

//@brief ʩ����������, Ŀǰֻ��ǰ����, ���������ﲻ��(�ⲿֱ�������); Ĭ������Ҳ��3D��; ���벻���ǵ�λ����
//@param[in] v1, ���ο���
//@param[out] newv1: =v1/|v1| ��λ��; ��Ҫ�ⲿԤ�����ڴ�
//@param[out] newv2: v2 ���� v1 ������֮������; ��Ҫ�ⲿԤ�����ڴ�
void schmidtOrtho(const double *v1, const double *v2, double *newv1, double *newv2, size_t len = 3);

//@brief ����Ԫ��˹��ȥ��; ��: https://www.oschina.net/code/snippet_76_4375
//AΪϵ������xΪ�����������ɹ�������true�����򷵻�false������x��ա�
bool RGauss(const vector<vector<double> > & A, vector<double> & x);

//@brief ��ʵ���޸� pSegMat, ���� imshow
//@param[in] labelMat: �� ahc.PlaneFitter.membershipImg 
//@param[in] pSegMat: ���Թ۲�mat; Ϊɶ��ָ��: ��ʵ�ô�ֵҲ��, ���ǲ��ܴ�����, ����: cannot bind non-const lvalue reference of type 'int&' to an rvalue of type 'int'
//void showLabelMat(cv::Mat labelMat, cv::Mat *pSegMat = 0);
void annotateLabelMat(cv::Mat labelMat, cv::Mat *pSegMat = 0); //����

//@brief ���� ahc.PlaneFitter.run ����ģ�庯��, ����ͷ�ļ�
//@param[in] &pointsIn, ģ����, �Լ�ʵ�� (e.g., NullImage3D, OrganizedImage3D)
//@param[in] &idxVecs, index vector of vector 
//@param[in] doRefine Ĭ��=true [DEPRECATED]
template <class Image3D>
vector<PlaneSeg> zcRefinePlsegParam(const Image3D &pointsIn, const vector<vector<int>> &idxVecs/*, bool doRefine = true*/){
	vector<PlaneSeg> plvec; //��Ÿ���ƽ�����

	size_t plCnt = idxVecs.size();
	for(size_t i=0; i<plCnt; i++){
		const vector<int> &idxs = idxVecs[i];
		PlaneSeg tmpSeg(pointsIn, idxs);
		plvec.push_back(tmpSeg);

		//if(dbg2_)
			//printPlaneParams(tmpSeg);
	}//for

	return plvec;
}//zcRefinePlsegParam

//@brief zc: ͨ�� lblMat, ��δȷ��/�޷��������ֱ��, ȷ��Ϊ��������(�з���)
//@param[in] dmap, ԭ���ͼ, ����mm
//@param[in] orig, ԭ��, �� t3; ����m
//@param[in] axs, ����ֱ��, �� R9; ������, ������λ����; ��������ʱ�Ѿ�����������ϵ, ������ֵ��������ƻ�������
vector<double> zcAxLine2ray(const cv::Mat &dmap, const vector<double> &orig, const vector<double> &axs,
	double fx, double fy, double cx, double cy);

//@brief zc: ����������������ƽ��, ���һ����Ԫ�� (�����ж���, e.g., ������һ����ֱ������¶���, ����������), 
//ʾ��ͼ��: http://codepad.org/he35YCTh
//@return ortho3tuples, ��pl-idx-tup3, ����ʵ��ƽ�����; ���ڿ����ò�����
//@param[in] plvec
//@param[in] lblMat ƽ�滮�� label ͼ, �� ahc.PlaneFitter.membershipImg 
//@param[out] cubeCandiPoses, vec-vec, ��ʼ�����ǿ�, N*12(t3+R9), ��del��R9���������ת����, ������������del��. �ĳ� R9����ת����(nearest orhto ���), ���д洢(row-major)??? ����ȷ����
//@param[out] prev ��������
//@param[in/out] dbgMat, e.g.: pf.run ����ĵ��Թ۲� Mat
vector<vector<int>> zcFindOrtho3tup(const vector<PlaneSeg> &plvec, const cv::Mat &lblMat,
	double fx, double fy, double cx, double cy,
	vector<vector<double>> &cubeCandiPoses, cv::OutputArray dbgMat = cv::noArray());

//@brief ����һ�� cubeCandiPoses-vec, ���һ�� �Ż���� cubePose, �� cu->cam
//@return ע���� 3d, kinfu �õ��� 3f, Ҫ��ȷת��
//@param[in] cubeCandiPoses, vec-vec, ÿ�� 12��(t3+R9); ֮ǰ zcFindOrtho3tup �Ĺ�����, ʹ R9 ��Ȼ����ת����; ��ʵ��� size()==2, �����ӽ�������
Affine3d getCuPoseFromCandiPoses(const vector<vector<double>> &cubeCandiPoses);

//@brief zc: �����ڲ�, �� 3d�������תΪ 2d����; ����int��
cv::Point getPxFrom3d(const Vector3d &pt3d, float fx, float fy, float cx, float cy);
//@brief ���� float ��
cv::Point2f getPx2fFrom3d(const Vector3d &pt3d, float fx, float fy, float cx, float cy);

//@brief zc: ���� crnrTR, dmap ���ʵ���������߶�(��ֱ��), �� 1+3 ������
//@param[in] crnrTR, t3+R9=12��, ȡ�� cubeCandiPoses; t��������: ��
//@param[in] cuSideVec, ���������߳���, ����: ��
//@param[in] dmap, ���ͼ, ushort
//@param[in] labelMat, �� ahc.PlaneFitter.membershipImg, 
//@param[out] pts4, ���ֵ: 1*12 vec, 4�������
//@return ���ҵ�4��:true; ����ĳЩԭ���ж�:false
//vector<double> getCu4Pts(const vector<double> &crnrTR, const vector<float> &cuSideVec,const cv::Mat &dmap, cv::Mat &labelMat, float fx, float fy, float cx, float cy);
bool getCu4Pts(const vector<double> &crnrTR, const vector<float> &cuSideVec,const cv::Mat &dmap, cv::Mat &labelMat, float fx, float fy, float cx, float cy, vector<double> &pts4);

//@brief zc: �ֶ���Ⱦ���������ͼ, ��ǰ������ opengl(pcl����, fboҲŪ����, ������), vtk(�޷�ʹ����ʵ�ڲ�), �˺��������ӵ���ԭ�� 000, �ʲ���������������
//2017-1-14 23:34:23
//@param[in] Cube, �������ϵ��, ������ģ��, 
//@param[in] intr=(fx,fy,cx,cy), ����ڲ�
//@return ���� 16uc1 �����ͼ
cv::Mat zcRenderCubeDmap(const Cube &cube, float fx, float fy, float cx, float cy, int step = 1);

//@brief zc: �ж�ֱ����ƽ���Ƿ��ཻ; �ο�: https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
//@param[in] (L0, L), ֱ�ߵ�бʽ
//@param[in] (plnorm, plp0), ƽ�淨����ĳ��(��ĳ��Ƭ����)
//@param[out] ptInters, ֱ����ƽ�潻��
bool isLinePlaneIntersect(const Vector3d &L0, const Vector3d &L, const Vector3d &plnorm, const Vector3d &plp0, Vector3d &ptInters);

//@brief ��������, Ŀǰ������ Cube.drawContour ������������; �β��б���� cv::line
//����ο�: http://answers.opencv.org/question/10902/how-to-plot-dotted-line-using-opencv/
void zcDashLine(CV_IN_OUT cv::Mat& img, cv::Point pt1, cv::Point pt2, const cv::Scalar& color/*, int thickness=1, int lineType=8, int shift=0*/);

//@brief zc: ��������Ƭ��
class Facet{
	//TODO  //2017-1-14 23:27:07
public:
	vector<int> vertIds_;
	Vector3d center_, normal_;

	//@brief �ж���Ƭ�Ƿ���ĳ���� id
	bool isContainVert(const int vertId) const{
		const size_t vCnt = vertIds_.size();
		for(size_t i=0; i<vCnt; i++){
			if(vertId == vertIds_[i])
				return true;
		}

		return false;
	}//isContainVert
};
class Cube{
private:
	//vector<Vector3d> cuVerts8_;
	//vector<Facet>

	//@brief ÿ�����㱻�����湲��, �� vec-vec ��¼���� [i] �������� id, 8*3; �˱���Ŀǰ������ drawContour
	//[DEPRECATED] ������Ƭ�Ƿ������������, ���� isContainVert �ж� //2017-1-17 23:28:50
	vector<set<int>> vertAdjFacetIds_;

public:
	vector<Vector3d> cuVerts8_; //1x8
	//void setVerts8(const vector<Vector3d> &cuVerts8){ cuVerts8_ = cuVerts8; }
	//vector<int*> faceVertIdVec_; //6x4
	vector<Facet> facetVec_;
	vector<vector<int>> edgeIds_;

	Cube(){}
	Cube(const Cube &cuOther, const Affine3d &affine);

	//@brief ���ݶ������ vertIds, ��� facet
	//@param[in] vertIds, ������� 0~7 ��ĳ4��
	void addFacet(const vector<int> &vertIds);
	void addEdgeId(vector<int> &edge);

	bool isLineFacetIntersect(const Vector3d &L0, const Vector3d &L, const Facet &facet, Vector3d &ptInters) const;
	bool isLineFacetIntersect(const Vector3d &L0, const Vector3d &L, int faceId, Vector3d &ptInters) const{
		return isLineFacetIntersect(L0, L, facetVec_[faceId], ptInters);
	}
	//@brief ����(��ԭ���붥��L), �Ƿ�����Ƭ faceId �ཻ
	//@param[in] L, �Ƕ���, ���������ԭ��, ����Ҳͬʱ����ԭ�㷢��������
	bool isVrayFacetIntersect(const Vector3d &L, int faceId, Vector3d &ptInters) const{
		Vector3d L0(0,0,0);
		return isLineFacetIntersect(L0, L, facetVec_[faceId], ptInters);
	}//isVrayFacetIntersect

	//@brief ��������潻��汾, �� "Vector3d &ptInters" ����
	bool isVrayFacetIntersect(const Vector3d &L, int faceId) const{
		Vector3d ptInters_tmp; //ռλ��, �������
		Vector3d L0(0,0,0);
		return isLineFacetIntersect(L0, L, facetVec_[faceId], ptInters_tmp);
	}//isVrayFacetIntersect


	//@brief �ж��Ƿ��Ѿ�������ʼ������
	bool isValid(){ return this->cuVerts8_.size() != 0; }

	//@brief ���Ƶ� 2D cv::mat
	//@param[out], dstCanvas, �����ƻ���, ��������, �ڱ���; ����������, ���ӻ���
	//@param[in], (fx,fy, cx,cy) ����ڲ�
    //@param[in], color, �������߿�������ɫ
	//@param[in], hideLines, ����ʱ�Ƿ�����
	void drawContour(cv::Mat dstCanvas, double fx, double fy, double cx, double cy, const cv::Scalar& color, bool hideLines = false);

	//@brief �ж������ĳ�ӽ� camPose ��, �Ƿ�������ɼ�, �Ա���Ϊͼ�Ż��Ĺؼ�֡ @2017-11-12 19:52:40
	//@param[in] camPose, ����һ�������̬ (cam->global / cam2g)
	//@param[in] fx, fy, cx, cy
	bool isTrihedronVisible(const Affine3d &camPose, float fx, float fy, float cx, float cy, bool dbgPrint = false);
};


#endif //_ZC_AHC_UTILITY_H_
