#include "zcAhcUtility.h"

#include <fstream>

#include <pcl/pcl_macros.h> //zc: RAD2DEG

using std::cout;
using std::endl;
using std::ofstream;

//@brief ��� t3+R9 ��
void processPoses(const char *fn, const Affine3dVec &poses){
	ofstream fout(fn);
	for(size_t i=0; i<poses.size(); i++){
		const Affine3d &pose = poses[i];
		Eigen::Quaterniond q (pose.rotation ());
		Eigen::Vector3d t (pose.translation ());
		//��Ӧǰ�����, �� t �ܴ�, ��Ϊ����ЧֵFLAG, csv������ //2016-12-8 15:05:41
		float invalidThresh = 9e2;
		if(t[0] > invalidThresh){
			fout << "0,0,0,0,0,0,0" <<endl;
		}
		else{
			fout << t[0] << "," << t[1] << "," << t[2]
			<< "," << q.w () << "," << q.x () << "," << q.y ()<< ","  << q.z () << std::endl;
		}
	}
	fout.close();
}//processPose

//@brief ���ƽ�����: ����,����,����, mse(��ɶ��?)
void printPlaneParams(const double *normals, const double *center, double curvature, double mse){
	printf("normal=(%f,%f,%f); center=(%f,%f,%f); curv=%f; mse=%f\n", normals[0],normals[1],normals[2], center[0],center[1],center[2], curvature, mse);
}

void printPlaneParams(const PlaneSeg &planeSeg){
	printPlaneParams(planeSeg.normal, planeSeg.center, planeSeg.curvature, planeSeg.mse);
}

//@brief ���, �������Ȳ��̶�
double dotProd(const double *v1, const double *v2, size_t len /*= 3*/){
	double sum = 0;
	for(size_t i=0; i<len; i++){
		sum += (v1[i]*v2[i]);
	}
	return sum;
}//dotProd

//@brief ����ģ��
double norm(const double *v, size_t len/* = 3*/){
	return sqrt(dotProd(v, v, len));
}//norm

//@brief ����ŷ�Ͼ���
double dist(const double *v1, const double *v2, size_t len /*= 3*/){
	double sum = 0;
	for(size_t i=0; i<len; i++){
		double v12sub = v1[i] - v2[i];
		sum += v12sub * v12sub;
	}
	sum = sqrt(sum);
	return sum;
}//dist

//@brief ��� (v1 x v2)=v3; ֻ����ά3D;
//@param[out] v3, ���������; ��Ҫ�ⲿԤ�����ڴ�
void crossProd(const double *v1, const double *v2, double *v3){
	//��: http://baike.baidu.com/view/973423.htm
	v3[0] = v1[1] * v2[2] - v1[2] * v2[1];
	v3[1] = v1[2] * v2[0] - v1[0] * v2[2];
	v3[2] = v1[0] * v2[1] - v1[1] * v2[0];
}//crossProd

//@brief ʩ����������, Ŀǰֻ��ǰ����, ���������ﲻ��(�ⲿֱ�������); Ĭ������Ҳ��3D��; ���벻���ǵ�λ����
//@param[in] v1, ���ο���
//@param[out] newv1: =v1/|v1| ��λ��; ��Ҫ�ⲿԤ�����ڴ�
//@param[out] newv2: v2 ���� v1 ������֮������; ��Ҫ�ⲿԤ�����ڴ�
void schmidtOrtho(const double *v1, const double *v2, double *newv1, double *newv2, size_t len /*= 3*/){
	//��: http://blog.csdn.net/mathmetics/article/details/21444077
	//1. v1��λ��
	double v1norm = norm(v1, len);
	for(size_t i=0; i<len; i++)
		newv1[i] = v1[i] / v1norm;

	//2. v2������, 
	double v2proj = dotProd(v2, newv1, len); //v2 �� v1�����ͶӰģ��
	for(size_t i=0; i<len; i++)
		newv2[i] = v2[i] - v2proj * newv1[i];
	//2.2 v2 ��λ��
	double newv2norm = norm(newv2, len);
	for(size_t i=0; i<len; i++)
		newv2[i] = newv2[i] / newv2norm;
}//schmidtOrtho


//@brief ����Ԫ��˹��ȥ��; ��: https://www.oschina.net/code/snippet_76_4375
//AΪϵ������xΪ�����������ɹ�������true�����򷵻�false������x��ա�
bool RGauss(const vector<vector<double> > & A, vector<double> & x){
	x.clear();
	//�������뺯���Ѿ���֤��ϵ�������ǶԵģ��Ͳ����A��
	int n = A.size();
	int m = A[0].size();
	x.resize(n);
	//����ϵ�����󣬷�ֹ�޸�ԭ����
	vector<vector<double> > Atemp(n);
	for (int i = 0; i < n; i++)
	{
		vector<double> temp(m);
		for (int j = 0; j < m; j++)
		{
			temp[j] = A[i][j];
		}
		Atemp[i] = temp;
		temp.clear();
	}
	for (int k = 0; k < n; k++)
	{
		//ѡ��Ԫ
		double max = -1;
		int l = -1;
		for (int i = k; i < n; i++)
		{
			if (abs(Atemp[i][k]) > max)
			{
				max = abs(Atemp[i][k]);
				l = i;
			}
		}
		if (l != k)
		{
			//����ϵ�������l�к�k��
			for (int i = 0; i < m; i++)
			{
				double temp = Atemp[l][i];
				Atemp[l][i] = Atemp[k][i];
				Atemp[k][i] = temp;
			}
		}
		//��Ԫ
		for (int i = k+1; i < n; i++)
		{
			double l = Atemp[i][k]/Atemp[k][k];
			for (int j = k; j < m; j++)
			{
				Atemp[i][j] = Atemp[i][j] - l*Atemp[k][j];
			}
		}
	}
	//�ش�
	x[n-1] = Atemp[n-1][m-1]/Atemp[n-1][m-2];
	for (int k = n-2; k >= 0; k--)
	{
		double s = 0.0;
		for (int j = k+1; j < n; j++)
		{
			s += Atemp[k][j]*x[j];
		}
		x[k] = (Atemp[k][m-1] - s)/Atemp[k][k];
	}
	return true;
}//RGauss

//@brief ��ʵ���޸� pSegMat, ���� imshow
//@param[in] labelMat: �� ahc.PlaneFitter.membershipImg 
//@param[in] pSegMat: ���Թ۲�mat; Ϊɶ��ָ��: ��ʵ�ô�ֵҲ��, ���ǲ��ܴ�����, ����: cannot bind non-const lvalue reference of type 'int&' to an rvalue of type 'int'
//void showLabelMat(cv::Mat labelMat, cv::Mat *pSegMat /*= 0*/){
void annotateLabelMat(cv::Mat labelMat, cv::Mat *pSegMat /*= 0*/){ //����
	using namespace cv;
	int *lbMatData = (int*)labelMat.data; //label mat raw data
	int matEcnt = labelMat.rows * labelMat.cols; //mat elem count
	set<int> lbSet(lbMatData, lbMatData + matEcnt); //label set, ��Ȼ�� -6~-1, ʲô����? ����! //2016-9-13 22:39:42
	int labelCnt = lbSet.size();
	//if(dbg2_)
		printf("labelCnt= %d\n", labelCnt);
	for(set<int>::const_iterator iter=lbSet.begin(); iter!=lbSet.end(); iter++){
		int currLabel = *iter;
		if(currLabel < 0)
			continue;
		Mat msk = (labelMat == currLabel);
		// 		Mat erodeKrnl = getStructuringElement(MORPH_RECT, Size(5,5));
		// 		erode(msk, msk, erodeKrnl); //��ʴmsk, �Ա��������, ������һ����Ҫcontour //����ʹ, �������������

		vector<vector<Point> > contours;
		findContours(msk, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		int maxContNum = 0;
		size_t maxCntIdx = 0; //���contour��idx
		for(size_t i=0; i<contours.size(); i++){ //���������
			if(contours[i].size() > maxContNum){
				maxContNum = contours[i].size();
				maxCntIdx = i;
			}
		}

		Rect bdRect = boundingRect(contours[maxCntIdx]);

		Scalar colorRed(0,0,255);
		Scalar colorWhite(255,255,255);
		//rectangle(*pSegMat, bdRect, colorWhite, 2);
		//putText(*pSegMat, std::to_string(long long(currLabel)), bdRect.tl(), FONT_HERSHEY_SIMPLEX, 1, colorWhite, 2);

		//+++++++++++++++���ö��� color, ���������ѿ� //7��, ��8��, û��ɫ //2016-12-27 11:04:11
		//const Scalar colorArr[] = {Scalar(0,0,255), Scalar(0,255,0), Scalar(0,255,255),
		//	Scalar(255,0,0), Scalar(255,0,255), Scalar(255,255,0), Scalar(255,255,255)};
		const Scalar colorArr[] = {Scalar(128,128,255), Scalar(128,255,128), Scalar(128,255,255),
			Scalar(255,128,128), Scalar(255,128,255), Scalar(255,255,128), Scalar(255,255,255)};

		static int cidx = -1;
		cidx++;
		cidx = cidx % 7;
		//rectangle(*pSegMat, bdRect, colorArr[cidx], 1);
// 		putText(*pSegMat, std::to_string(long long(currLabel)), bdRect.tl(), FONT_HERSHEY_SIMPLEX, 1, colorArr[cidx], 2);
// 		putText(*pSegMat, std::to_string(long long(currLabel)), bdRect.br(), FONT_HERSHEY_SIMPLEX, 1, colorArr[cidx], 2);

		//Point bdRectCenter(bdRect.x + bdRect.width / 2, bdRect.y + bdRect.height / 2);
// 		Point bdRectAnchor(bdRect.x + bdRect.width * 1. / 3, bdRect.y + bdRect.height * 2. / 3); //�����м�, �ķ���ƫ���½� 1/3 ��
// 		putText(*pSegMat, std::to_string(long long(currLabel)), bdRectAnchor, FONT_HERSHEY_SIMPLEX, 1, colorArr[cidx], 2); //Ҳ�����ھ��ο�����
        //��--����Ҳ���ÿ�, ���Ǹ�������--��  //2017-2-21 14:53:00
        Moments mu = moments(contours[maxCntIdx]);
        Point mcen(mu.m10/mu.m00, mu.m01/mu.m00);
		//putText(*pSegMat, std::to_string(long long(currLabel)), mcen, FONT_HERSHEY_SIMPLEX, 1, colorArr[cidx], 2); //����������
	}//for-lbSet
}//annotateLabelMat


//@brief zc: ͨ�� lblMat, ��δȷ��/�޷��������ֱ��, ȷ��Ϊ��������(�з���)
//@param[in] dmap, ԭ���ͼ, ����mm
//@param[in] orig, ԭ��, �� t3; ����m
//@param[in] axs, ����ֱ��, �� R9; ������, ������λ����; ��������ʱ�Ѿ�����������ϵ, ������ֵ��������ƻ�������
vector<double> zcAxLine2ray(const cv::Mat &dmap, const vector<double> &orig, const vector<double> &axs,
	double fx, double fy, double cx, double cy){
	vector<double> resAxs;

	const double STEP = 10; //�����������Ĳ���
	double ox = orig[0],
			oy = orig[1],
			oz = orig[2];
	for(size_t i=0; i<3; i++){
		double axx = axs[i * 3 + 0];
		double axy = axs[i * 3 + 1];
		double axz = axs[i * 3 + 2];
		
		double vx = ox + STEP*axx, //+STEP
			vy = oy + STEP*axy,
			vz = oz + STEP*axz;
		int u = (vx * fx) / vz + cx,
			v = (vy * fy) / vz + cy;
		ushort depth = dmap.at<ushort>(u, v);

		double vx_1 = ox - STEP*axx, //-STEP
			vy_1 = oy - STEP*axy,
			vz_1 = oz - STEP*axz;
		int u_1 = (vx_1 * fx) / vz_1 + cx,
			v_1 = (vy_1 * fy) / vz_1 + cy;
		ushort depth_1 = dmap.at<ushort>(u_1, v_1);

		int FLAG = abs(vz - depth) < abs(vz_1 - depth_1) ? 1 : -1;
		
		resAxs.push_back(FLAG * axx);
		resAxs.push_back(FLAG * axy);
		resAxs.push_back(FLAG * axz);
	}//for-i-3

	return resAxs;
}//zcAxLine2ray

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
	vector<vector<double>> &cubeCandiPoses, cv::OutputArray dbgMat /*= cv::noArray()*/){

	using namespace cv;
	
	//1. �������ƽ��, ���������ƽ�й�ϵ, �Ȳ�����Ҳ��ƽ�еĺ���
	size_t plcnt = plvec.size();
	double orthoThresh = 
		//0.0174524; //cos(89��), С�ڴ�Ϊ���� //��Ϊ���� pcl160, ��OpenNIGrabber��Ĭ���ڲ�,���µ��Ʋ�׼, ������ﲻ������ֵ
		0.0871557; //cos(85��) �ſ�, //����������ڲ�֮��, ���ַǳ���, ������Ȼ�ſ�
	double paralThresh = 
		//0.999847695; //cos(1��), ���ڴ�Ϊƽ��
		0.996194698; //cos(5��) �ſ�
	vector<set<int>> orthoMap(plcnt);  //������ϵ��, ������ֻ���������, �����һ��vecӦ�ǿ�; ����ʵ vector<set<int>> ������, �ݲ���
	vector<vector<int>> ortho3tuples; //�ҵ�����Ԫ�������, ��vec ������ vec ���� set, ��������; ��vec��Ȼsize=3
	vector<set<int>> paralMap(plcnt); //ƽ�й�ϵ��
	for(size_t i=0; i<plcnt; i++){
		const PlaneSeg &pl_i = plvec[i];
		const double *norm_i = pl_i.normal;
		for(size_t j=i+1; j<plcnt; j++){ //�� i+1, ֻ��������
			const PlaneSeg &pl_j = plvec[j];
			const double *norm_j = pl_j.normal;
			double cosNorm = dotProd(norm_i, norm_j, 3); //�� |a|=|b|=1, ��ֱ�� cos(a,b)=a.dot(b)

			//if(dbg2_)
				//printf("i,j=(%u,%u), cosNorm=%f; angle=%f\n", i, j, cosNorm, RAD2DEG(acos(cosNorm)));

			if(abs(cosNorm) < orthoThresh)
				orthoMap[i].insert(j);
			if(abs(cosNorm) > paralThresh)
				paralMap[i].insert(j);
		}//for-j
	}//for-i-plcnt

	//2. ��ÿ�� orthoMap[i] ��, ������������, ���ҵ�һ����Ԫ��! //�����ϡ���֪�����塿��ֻ���ҵ�������Ԫ��, ��>2, ��Ҫͨ��ƽ���������ų�
	for(size_t i=0; i<orthoMap.size(); i++){
		set<int> &ortho_i = orthoMap[i]; //��id=i������ƽ��id��
		if(ortho_i.size() < 2) //�� <2, �򹹲�����Ԫ��
			continue;
		set<int>::const_iterator iter_j = ortho_i.begin();
		for(; iter_j!=ortho_i.end(); iter_j++){
			int idx_j = *iter_j;
			set<int> &ortho_j = orthoMap[idx_j]; //�����ڴ��� idx_k, ���ҵ�, ���һ����Ԫ��
			if(ortho_j.size() == 0)
				continue;
			//set<int>::const_iterator iter_k = iter_j + 1; //��, �� '+'
			//set<int>::const_iterator iter_k = iter_j; iter_k++; //��, ����ԭʼ
			set<int>::const_iterator iter_k = std::next(iter_j);
			for(; iter_k!=ortho_i.end(); iter_k++){
				int idx_k = *iter_k;
				if(ortho_j.count(idx_k)){ //�ҵ���Ԫ��
					vector<int> tuple3;
					tuple3.push_back(i);
					tuple3.push_back(idx_j);
					tuple3.push_back(idx_k);
					ortho3tuples.push_back(tuple3);
				}
			}//for-iter_k
		}//for-iter_j
	}//for-i-orthoMap

	//3. �� ortho3tuples ���ܴ��ڡ��ٵġ���Ԫ��, �ж�����: ����ʵ�����ڲ�����Ԫ��-->����Ϊ: �����������潻��(3D)��labelMat(2D)��ĳlen��������������Чlabel��������
	//�ж����ݸ�Ϊ: ����label-set ���� tuple3-vec (��Ȼһ�� set һ�� vector, �� std::includes �㷨) //2016-12-7 20:55:25
	vector<vector<int>> tmp3tuples;
	for(size_t i=0; i<ortho3tuples.size(); i++){
		vector<int> &tuple3 = ortho3tuples[i];
		//1. ȡ����, ���� Ax=b �� [A|b] �������; ����Ԫ��˹, ������涥��
		vector<vector<double>> matA;
		vector<double> matb;
		for (int ii=0; ii<3; ii++){
			int plIdx = tuple3[ii];
			const PlaneSeg &plseg = plvec[plIdx];
			//ƽ�����ABCD: (ABC)=normal; D= -dot(normal, center) //ע�⸺��, b[i]=-D=dot...
			vector<double> tmpRow(plseg.normal, plseg.normal+3);
			double b_i = dotProd(plseg.normal, plseg.center);
			tmpRow.push_back(b_i); //ϵ������һ��, ���� Ai|bi
			matA.push_back(tmpRow);
		}
		vector<double> vertx; //���潻��, ������Ľ�; �߶��ǲ�����(m)���� ��
		RGauss(matA, vertx);

		//2. ����������������Ƿ���������Чlabel
		//3D->2D���ص�, ��int, ��׷�󾫶�
		int ou = (vertx[0] * fx) / vertx[2] + cx,
			ov = (vertx[1] * fy) / vertx[2] + cy;

		//int winSz = 20 / qvgaFactor_; //���򴰿ڳ���
		int winSz = 20; //��ɺ�����, ��ʱ��Ҫ qvgaFactor_ ��

		cv::Rect tmpRoi(ou - winSz/2, ov - winSz/2, winSz, winSz);
		if(tmpRoi != (tmpRoi & cv::Rect(0,0, lblMat.cols, lblMat.rows)) ) //��������ͼ��Χ��, ����
			continue;
		//else: ����, ����������С������ͼ����, ����
		//if(dbg1_){
		if(! dbgMat.empty()){ //�ĳɲ���ȫ�� flag
			cv::circle(dbgMat.getMatRef(), cv::Point(ou, ov), 2, 255, 1); //��СԲȦ
			cv::circle(dbgMat.getMatRef(), cv::Point(ou, ov), 7, cv::Scalar(0,0,255), 2); //���ԲȦ, ͬ��Բ, ���Թ۲����
		}

		//cv::Mat vertxNbrLmat(pf.membershipImg, tmpRoi); //���� label-mat
		cv::Mat vertxNbrLmat(lblMat, tmpRoi); //���� label-mat
		
		vertxNbrLmat = vertxNbrLmat.clone(); //clone �ܽ��set(labelMat) ����������? �� ��!
				//�ǲ�����, Ӧ����˵, ���� clone, ��roi����view���������ڴ�, ȡ [data, data+size] ʱ��ȡ��ԭ Mat һ��Ƭ��(�� 1*16), ����������Ҫ�ķ���(��4*4) //2016-12-28 11:22:14
		//cout<<"vertxNbrLmat:\n"<<vertxNbrLmat<<endl;
		//cv::Mat dbgRoiMat(seg, tmpRoi); //���Թ۲�С����

		int *vertxNbrLmatData = (int*)vertxNbrLmat.data; //label mat raw data
		set<int> nbrLabelSet(vertxNbrLmatData, vertxNbrLmatData + winSz * winSz);
		//int posCnt = 0; //���� label>0 ͳ����
		//for(set<int>::const_iterator it = nbrLabelSet.begin(); it != nbrLabelSet.end(); it++){
		//	if(*it >= 0) //0Ҳ����Чlabel
		//		posCnt++;
		//}
		if(std::includes(nbrLabelSet.begin(), nbrLabelSet.end(), tuple3.begin(), tuple3.end()) ){ //����֤, Ч���ܺ� 2016-12-9 00:09:40
		//if(posCnt >= 3){ //�϶�Ϊ��ʵ������������Ԫ��
			//if(dbg1_){
			if(! dbgMat.empty()){ //�ĳɲ���ȫ�� flag
				cv::circle(dbgMat.getMatRef(), cv::Point(ou, ov), 2, 255, 1); //��СԲȦ
				cv::circle(dbgMat.getMatRef(), cv::Point(ou, ov), 7, cv::Scalar(0,255,0), 2); //�̴�ԲȦ, ͬ��Բ, ���Թ۲����, //��ʾɸѡ���ն��µĶ���
			}

			tmp3tuples.push_back(tuple3);
			cubeCandiPoses.push_back(vertx); //�Ȱ�(R,t)��t���; ֮������ cubePoses ��Ҫ push, Ҫ��ÿ�� .insert(.end, dat, dat+3);

			//�Ķ�: cubeCandiPoses �ڴ� if����, һ�����, Ӧ�ú��� //2016-12-29 11:14:27
			vector<double> ax3orig; //��ʼ����, ����������
			ax3orig.reserve(9);
			//v2: ����������, ��: https://www.evernote.com/shard/s399/nl/67976577/48135b5e-7209-47c1-9330-934ac4fee823
#if 01	//v2.1 ���桾��������, ������û��ϵ
			for(size_t kk=0; kk<3; kk++){
				const double *pl_k_norm = plvec[tuple3[kk]].normal;
				ax3orig.insert(ax3orig.end(), pl_k_norm, pl_k_norm+3);
			}
#endif //����/���� ˭����ʼ��

			//���������������: 
			//https://en.wikipedia.org/wiki/Orthogonal_matrix#Nearest_orthogonal_matrix
			//http://math.stackexchange.com/questions/571817/given-three-vectors-how-to-find-an-orthonormal-basis-closest-to-them
			JacobiSVD<Matrix3d> svd(Map<Matrix3d>(ax3orig.data()), ComputeFullU | ComputeFullV);
			Matrix3d svdU = svd.matrixU();
			Matrix3d svdV = svd.matrixV();
			Matrix3d orthoAxs = svdU * svdV.transpose(); //����õ����� ax3orig �����Ż�������, det=��1, ��ȷ������ת����

			//�����֮ǰ�Ǻ�������β�ͬ: //2016-12-29 20:57:59
			//֮ǰ�ò��� PROCRUSTES ʱ, cubeCandiPoses �д���� ax3orig, @L930~940; �������һ��
			//���������ת����:
#if 01
			int FLAG = orthoAxs.determinant() > 0 ? 1 : -1; //det �������� ��1, ��ֱ����Ϊ flag, ���������...
			//orthoAxs.row(2) *= FLAG; //�� ��
			orthoAxs.col(2) *= FLAG; //����: ��-������ row�ڴ水col�����, ��������Ӧ�� .col(2); //̫ trick, ��ʱ����
#else
			if(orthoAxs.determinant() < 0)
				//todo-����ĳ����-suspend...
				;
#endif

			double *axs = orthoAxs.data();
			//axs = orthoAxs.transpose().data(); //ΪɶҪ .T? ��Ϊ ax3orig ���д�, �����з���svd, �������ǰ���ȡ��; ���뱣���� ax3orig ����, Ҫת��
				//�˴�ת����ȻӰ����, ������, �ؼ���ǰ�� .col(
				//��������Ҳ���ˣ���Ӧ .T�� ��Ϊ: ����������, ����ȡ��, ���� ax3orig ���д洢 //2016-12-30 01:05:06

			//���Ի�������
			if(! dbgMat.empty() && 0 == i){
				const Scalar colorCmy[] = {Scalar(255,255,0),
											Scalar(255,0,255),
											Scalar(0,255,255)};
				for(size_t i=0; i<3; i++){
					//Ӧ�ô���, ���������������
// 					double x = axs[i*3+0],
// 							y = axs[i*3+1],
// 							z = axs[i*3+2];
					double axLen = 0.30; //������ĳ���, ����m
					double x = axLen * axs[i*3+0] + vertx[0],
							y = axLen * axs[i*3+1] + vertx[1],
							z = axLen * axs[i*3+2] + vertx[2];

					//axes in 2D (u,v):
					int au = (int)(x / z * fx + cx),
						av = (int)(y / z * fy + cy);

					cv::line(dbgMat.getMatRef(), cv::Point(ou, ov), cv::Point(au, av), colorCmy[i]);
				}
			}

			vector<double> &currCrnr = cubeCandiPoses.back(); //tmp thing...
			currCrnr.insert(currCrnr.end(), axs, axs+9);
		}//if-nbrLabelSet-includes-tuple3
	}//for-ortho3tuples
	ortho3tuples.swap(tmp3tuples);

	return ortho3tuples;
}//zcFindOrtho3tup

Affine3d getCuPoseFromCandiPoses(const vector<vector<double>> &cubeCandiPoses){
	Affine3d res;

	// 	size_t crnrCnt = cubeCandiPoses.size();
	// 	for(size_t i=0; i<crnrCnt; i++){
	// 		vector<double> &rt = cubeCandiPoses[i];
	// 		rt[]
	// 	}

#if 1	//��������, �� [0] ֱ������������̬
	vector<double> rt = cubeCandiPoses[0];
	res.translation() = Map<Vector3d>(rt.data());
	res.linear() = Map<Matrix3d>(rt.data()+3);
#endif

	return res;
}//getCuPoseFromCandiPoses

//vector<double> getCu4Pts(const vector<double> &crnrTR, const vector<float> &cuSideVec,const cv::Mat &dmap, cv::Mat &labelMat, float fx, float fy, float cx, float cy){
bool getCu4Pts(const vector<double> &crnrTR, const vector<float> &cuSideVec,const cv::Mat &dmap, cv::Mat &labelMat, float fx, float fy, float cx, float cy, vector<double> &pts4){

	//vector<double> res; //����ֵ: 1*12 vec, 4������� //���� pts4
	Vector3d pt0(crnrTR.data()); //�����潻��
	pts4.insert(pts4.end(), pt0.data(), pt0.data()+3);

	const float STEP = 0.01; //1cm, 10mm, 
	for(size_t i=1; i<=3; i++){ //��ÿһ����
		Vector3d axi(crnrTR.data()+i*3); //�������(����ʵ���ɷ���αװ)

		const int stepCnt = 5;
		Vector3d negaDirect = pt0 - axi * STEP * stepCnt, //�������� step
			posiDirect = pt0 + axi * STEP * stepCnt; //ע�� +��-

		cv::Point px_n = getPxFrom3d(negaDirect, fx, fy, cx, cy);
		cv::Point px_p = getPxFrom3d(posiDirect, fx, fy, cx, cy);
#if 0   //dist_p & dist_n �����ж�����, ���ж����涥��ʱ, �����϶���ֵ��Ӧ�ú�С
		float dist_n = abs(dmap.at<ushort>(px_n.y, px_n.x) / M2MM - negaDirect[2]);
		float dist_p = abs(dmap.at<ushort>(px_p.y, px_p.x) / M2MM - posiDirect[2]);

		int direct = dist_p < dist_n ? +1 : -1; //ȷ�����߷���
		//float dist_ref = dist_p < dist_n ? dist_p : dist_n; //ȡС�����ο�
#elif 1
		set<int> nbrLblSet_n, nbrLblSet_p;
		const int nbrCnt = 4;
		const int margin = 5;
		cv::Point nbrDelta[nbrCnt] = {cv::Point(-margin, 0), cv::Point(margin, 0), 
			cv::Point(0, -margin), cv::Point(0, margin)};
		cv::Rect matRegion(0, 0, labelMat.cols, labelMat.rows); //���ڼ��������Ч��

		for(size_t inbr=0; inbr<nbrCnt; inbr++){
			cv::Point nbrPxi_n = px_n + nbrDelta[inbr];
			cv::Point nbrPxi_p = px_p + nbrDelta[inbr];
			if(matRegion.contains(nbrPxi_n)) //�����ڵ���ͼ��������
				nbrLblSet_n.insert(labelMat.at<int>(nbrPxi_n) );
			else
				return false;

			if(matRegion.contains(nbrPxi_p))
				nbrLblSet_p.insert(labelMat.at<int>(nbrPxi_p) );
			else
				return false; //�����򳬳�ͼ��߽�, ��Ϊ�ñ߲�����, ����ǰ֡δ�ҵ�4����
		}
		//��Ҫ�׵���Ч lbl (δ�� -1, ֻҪ <0)
		//nbrLblSet_n.erase(-1);
		//nbrLblSet_p.erase(-1);
		set<int>::iterator iter;
		for(iter=nbrLblSet_n.begin(); iter!=nbrLblSet_n.end();){
			if(*iter < 0)
				nbrLblSet_n.erase(iter++);
			else
				++iter;
		}
		for(iter=nbrLblSet_p.begin(); iter!=nbrLblSet_p.end();){
			if(*iter < 0)
				nbrLblSet_p.erase(iter++);
			else
				++iter;
		}

		int direct = 0;
		set<int> nbrLblSet_k;
		if(nbrLblSet_n.size()>1 && nbrLblSet_p.size()<=1){
			direct = -1;
			//nbrLblSet_k = nbrLblSet_n;
		}
		else if(nbrLblSet_n.size()<=1 && nbrLblSet_p.size()>1){
			direct = 1;
			//nbrLblSet_k = nbrLblSet_p;
		}
		else{
			printf("++FUCK: nbrLblSet_n.size, nbrLblSet_p.size: %d, %d\n", nbrLblSet_n.size(), nbrLblSet_p.size());
			return false; //�����쳣���, ���Ծ���, һ������δ�ҵ�
		}

		//nbrLblSet_k�Ĵ������������ label ������
		//����: error C3892: '_Dest' : you cannot assign to a variable that is const
		//std::set_union(nbrLblSet_p.begin(), nbrLblSet_p.end(), nbrLblSet_n.begin(), nbrLblSet_n.end(), nbrLblSet_k.begin());
		nbrLblSet_k = nbrLblSet_p;
		nbrLblSet_k.insert(nbrLblSet_n.begin(), nbrLblSet_n.end());
#endif

		int k = stepCnt;
		float sideLen = 0;
		while(1){ //С�鲽һֱ��
			k++;
			sideLen = k * STEP;
			Vector3d pt_k = pt0 + direct * sideLen * axi;
			cv::Point px_k = getPxFrom3d(pt_k, fx, fy, cx, cy);
#if 0   //�����ж�����
			float dist_k = abs(dmap.at<ushort>(px_k.y, px_k.x) / M2MM - pt_k[2]);
			printf("%f, ", dist_k);

			//if(dist_k > STEP) //ֱ�����߲��������ͼ, ��ֹС�鲽
			if(dist_k > STEP*2)
				break;
#elif 0 //nbrLblSet_k.size() < 2 �ж�Ҳ����, �����߹���̫��, �������� 250&300�ֲ���
			set<int> nbrLblSet_k;
			for(size_t inbr=0; inbr<nbrCnt; inbr++){
				cv::Point nbrPxi_k = px_k + nbrDelta[inbr];
				if(matRegion.contains(nbrPxi_k))
					nbrLblSet_k.insert(labelMat.at<int>(nbrPxi_k) );
			}
			//nbrLblSet_k.erase(-1);
			for(iter=nbrLblSet_k.begin(); iter!=nbrLblSet_k.end();){
				if(*iter < 0)
					nbrLblSet_k.erase(iter++);
				else
					++iter;
			}

			if(nbrLblSet_k.size() < 2)
				break;
#elif 1 //nbrLblSet_k �� while ֮ǰ, direct �ж�ʱ��������ֵ
			bool walkEnd = false;
			set<int> nbrLblSet_tmp;
			for(size_t inbr=0; inbr<nbrCnt; inbr++){
				cv::Point nbrPxi_k = px_k + nbrDelta[inbr];
				if(matRegion.contains(nbrPxi_k)){
					int nbrLbl = labelMat.at<int>(nbrPxi_k);
					nbrLblSet_tmp.insert(nbrLbl);

					if(nbrLbl >=0 && nbrLblSet_k.count(nbrLbl) == 0){
						//��-����������Ӧ��X��label, ��������Ч�ġ���label, ���ж��ߵ�ͷ��
						walkEnd = true;
						break;
					}
				}
				else
					return false;
			}
			for(iter=nbrLblSet_tmp.begin(); iter!=nbrLblSet_tmp.end();){
				if(*iter < 0)
					nbrLblSet_tmp.erase(iter++);
				else
					++iter;
			}
			if(nbrLblSet_tmp.size() <= 1)
				walkEnd = true;

			if(walkEnd)
				break;
#endif
		}
		//sideLen -= STEP;
		//printf("\n");

		//�����������߳������:
		float minLenDiff = 10.f; //m, Ĭ�ϳ�ֵ 10m, Ȩ������ֵ
		size_t j_idx = 0;
		for(size_t j=0; j<3; j++){
			float sdLenDiff = abs(sideLen - cuSideVec[j]);
			if(sdLenDiff < minLenDiff){
				minLenDiff = sdLenDiff;
				j_idx = j;
			}
		}
		Vector3d pti = pt0 + direct * cuSideVec[j_idx] * axi;
		pts4.insert(pts4.end(), pti.data(), pti.data()+3);
	}//for-i-3����

	//return res;
	return true; //���ߵ������Ȼ true
}//getCu4Pts

cv::Point getPxFrom3d(const Vector3d &pt3d, float fx, float fy, float cx, float cy){
	//vector<int> res;
	cv::Point res;

	double x = pt3d[0],
		y = pt3d[1],
		z = pt3d[2];

	//res.push_back((int)(x / z * fx + cx));
	//res.push_back((int)(y / z * fy + cy));
	res.x = (int)(x / z * fx + cx /*+ 0.5f*/); //�� round ���� flooring //���ǸĻ� flooring, ��Ϊ px->pt3d ʱ��û�п��� round
	res.y = (int)(y / z * fy + cy /*+ 0.5f*/);

	return res;
}//getPxFrom3d

cv::Point2f getPx2fFrom3d(const Vector3d &pt3d, float fx, float fy, float cx, float cy){
	cv::Point2f res;

	double x = pt3d[0],
		y = pt3d[1],
		z = pt3d[2];

	res.x = x / z * fx + cx;
	res.y = y / z * fy + cy;

	return res;
}//getPx2fFrom3d

cv::Mat zcRenderCubeDmap(const Cube &cube, float fx, float fy, float cx, float cy, int step /*= 1*/){
	using namespace cv;

	Mat res = Mat::zeros(WIN_HH_MAX / step, WIN_WW_MAX / step, CV_16UC1); //��ʼȫ��

#if 1   //V1: ��ͼ���Ч��, ������ cu mask, �ٽ��� mask �ڲ����� //2017-1-15 00:28:47
	const vector<Vector3d> &cuVerts8_cam = cube.cuVerts8_;
	size_t vertCnt = cuVerts8_cam.size(); //==8, ���˴��Ի�ȡһ��
	CV_Assert(vertCnt == 8);

	vector<Point> pxs8;
	for(size_t i=0; i<vertCnt; i++){
		Point pxi = getPxFrom3d(cuVerts8_cam[i], fx, fy, cx, cy);
		pxs8.push_back(pxi);
	}

	vector<Point> hull;
	convexHull(pxs8, hull); //Ĭ�� clockwise=false, returnPoints=true
	
	//Mat cuMask = Mat::zeros(res.size(), CV_8UC1);
	Mat cuMask = Mat::zeros(WIN_HH_MAX, WIN_WW_MAX, CV_8UC1); //�� res ����
	fillConvexPoly(cuMask, hull, 255);
	//imshow("zcRenderCubeDmap-cuMask", cuMask); //����: http://imgur.com/bXTx1VG

	const double INVALID_DEPTH = 1e11; //�ü���ֵ����Чֵ
	for(int v = 0; v < WIN_HH_MAX; v += step){
		for(int u = 0; u < WIN_WW_MAX; u += step){
			if(0 == cuMask.at<uchar>(v, u)) //��Ч��������, ��ͼ���Ч��
				continue;

			double depth = INVALID_DEPTH;

			//�����ػ������ view ray //��������һ����ԭ�� 000
			double x = (u - cx) / fx,
				   y = (v - cy) / fy,
				   z = 1;
			Vector3d vray(x, y, z);
			
			//6������, ÿ������桾��ࡿ��ʾһ��:
#if 0	//�߼����ܶ�, �����������������Ұ����ʱ�Ŷ�; isVrayFacetIntersect �������߼нǺͷ���: 100~140ms; ����ˮƽ���߷�:4~11ms
			for(int fi=0; fi<3; fi++){
				//Ŀǰ��� facet �ǰ��������: 01, 23, 45; �˼����ͨ��, �ݶ�
				int which = cube.facetVec_[2*fi].center_.z() < cube.facetVec_[2*fi+1].center_.z() ? 2*fi : 2*fi+1;
#elif 1	//6��ȫ���, 180~220ms
			for(int fi = 0; fi < 6; fi++){
				int which = fi;
#endif
				Vector3d ptInters(-1,-1,-1); //�ݶ� <0 Ϊ��Чֵ, ��Ϊ�����ϲ�Ӧλ���������
				bool isInters = cube.isVrayFacetIntersect(vray, which, ptInters);
				//double z = ptInters[2]; //����zֵ
				//if(isInters && 0 <= z && z < depth)
				//    depth = z;
				if(isInters){
					double z = ptInters[2]; //����zֵ
					if(0 <= z && z < depth)
						depth = z;
				}
			}//for-fi

			if(depth != INVALID_DEPTH){
				int resv = v / step,
					resu = u / step;
				//res.at<ushort>(v, u) = (ushort)(M2MM * depth + 0.5f); //round, �� flooring
				res.at<ushort>(resv, resu) = (ushort)(M2MM * depth + 0.5f); //round, �� flooring
			}
		}//for-u
	}//for-v
	//cv::imshow("res_step", res);
	//cv::pyrUp(res, res); //wrong: will be blurred
	cv::resize(res, res, res.size()*2, 0, 0, cv::INTER_NEAREST);
#endif

	return res;
}//zcRenderCubeDmap

bool isLinePlaneIntersect(const Vector3d &L0, const Vector3d &L, const Vector3d &plnorm, const Vector3d &plp0, Vector3d &ptInters){
	bool res = false;

	//d=(p0-l0)*n/(l*n)
	double fenmu = L.dot(plnorm); //��ĸ
	if(abs(fenmu) < 1e-8) //��ĸΪ��, ����ƽ�л���������, ���� false
		res = false;
	else{
		res = true;

		double fenzi = (plp0 - L0).dot(plnorm);
		double d = fenzi / fenmu;
		ptInters = d * L + L0; //��ý���
	}

	return res;
}//isLinePlaneIntersect

void zcDashLine(CV_IN_OUT cv::Mat& img, cv::Point pt1, cv::Point pt2, const cv::Scalar& color){
    //�ο�����:
    //http://docs.opencv.org/2.4/modules/core/doc/drawing_functions.html
    //http://stackoverflow.com/questions/20605678/accessing-the-values-of-a-line-in-opencv
    //http://answers.opencv.org/question/10902/how-to-plot-dotted-line-using-opencv/
    using namespace cv;

    LineIterator it(img, pt1, pt2, 8);            // get a line iterator
    for(int i = 0; i < it.count; i++,it++)
        //if ( i % 3 != 0 ){         // every 5'th pixel gets dropped, blue stipple line
        if(0 < i%10 && i%10 < 6){
            //(*it)[0] = 200;
            if(img.channels() == 1)
                img.at<uchar>(it.pos()) = color[0];
            else if(img.channels() == 3){
                Vec3b &pxVal = img.at<Vec3b>(it.pos());
                pxVal[0] = color[0];
                pxVal[1] = color[1];
                pxVal[2] = color[2];
            }
        }

}//zcDashLine

Cube::Cube(const Cube &cuOther, const Affine3d &affine){
	//8����:
	for(size_t i=0; i<8; i++){
		Vector3d verti = affine * cuOther.cuVerts8_[i];
		cuVerts8_.push_back(verti);
	}
	//6����: //���涥�����: 0142/3675, 0253/1476, 0361/2574;
	for(size_t i=0; i<6; i++){
		Facet faceti = cuOther.facetVec_[i];
		addFacet(faceti.vertIds_);
	}
}//Cube-ctor

bool Cube::isLineFacetIntersect(const Vector3d &L0, const Vector3d &L, const Facet &facet, Vector3d &ptInters) const{
	bool res = isLinePlaneIntersect(L0, L, facet.normal_, facet.center_, ptInters);
	if(res){ //����ƽ���ཻ, �ٿ������Ƿ�����Ƭ��
		const vector<int> &vertIds = facet.vertIds_;

#if 0	//�˷������ܼ�����͹�������; ����������, �����߸�����, �нǺ�= 360��; ���������� <360��
		vector<Vector3d> pt2vertVec;
		vector<double> pt2vertNormVec;
		for(int i=0; i<4; i++){
			Vector3d pt2vi = cuVerts8_[vertIds[i]] - ptInters;
			pt2vertVec.push_back(pt2vi);
			pt2vertNormVec.push_back(pt2vi.norm());
		}

		double angRad = 0;
		const int iinc[4] = {1,2,3,0};
		for(int i=0; i<4; i++){
			angRad += acos(pt2vertVec[i].dot(pt2vertVec[iinc[i]]) / (pt2vertNormVec[i] * pt2vertNormVec[iinc[i]]) );
		}

		if(2*PI - angRad > 1e-8)
			res = false;
#elif 1	//��: http://blog.chinaunix.net/uid-30332431-id-5140349.html
		//�ּ��ʼ�: ���жϵ��ڶ�����ڡ�
		int c = 0; //��ż�� flag
		const int iinc[4] = {1,2,3,0};
		for(int i = 0; i < 4; i++){
			Vector3d vi = cuVerts8_[vertIds[i]],
					 vj = cuVerts8_[vertIds[iinc[i]]];
			//�ݶ�ֻ�� XYƽ��:
			double ptx = ptInters[0],
					pty = ptInters[1];
			double vix = vi[0],
					viy = vi[1],
					vjx = vj[0],
					vjy = vj[1];

			if((viy > pty) != (vjy > pty)
				&& (ptx < (vjx - vix) * (pty - viy) / (vjy - viy) + vix))
				c = !c; //1-odd-��Ƭ��; 0-even-��Ƭ��
		}
		res = c; //int->bool
#endif
	}//if(res)//����ƽ���ཻ, �ٿ������Ƿ�����Ƭ��

	return res;
}//Cube::isLineFacetIntersect

void Cube::addFacet(const vector<int> &vertIds){
	const int fvertCnt = 4;
	CV_Assert(vertIds.size() == fvertCnt); //������Ƭ, �����Ķ���

	//1, ���� facet
	Facet tmpFacet;
	tmpFacet.vertIds_ = vertIds;

	//�ڲ����¼��㷨����:
	Vector3d edge1 = cuVerts8_[vertIds[1]] - cuVerts8_[vertIds[0]],
			 edge2 = cuVerts8_[vertIds[3]] - cuVerts8_[vertIds[0]];
	Vector3d plnorm = edge1.cross(edge2); //������
	plnorm.normalize();
	tmpFacet.normal_ = plnorm;

	//����: �Խ����е�
	tmpFacet.center_ = (cuVerts8_[vertIds[0]] + cuVerts8_[vertIds[2]]) / 2;

	facetVec_.push_back(tmpFacet);

#if 0
	//2, ��� vertAdjFacetIds_
	if(vertAdjFacetIds_.size() == 0)
		vertAdjFacetIds_.resize(8);
	size_t facetId = facetVec_.size() - 1; //����Ƭid=0~7
	for(size_t i=0; i<fvertCnt; i++)
		vertAdjFacetIds_[i].insert(facetId);
#endif
}//Cube::addFacet

void Cube::addEdgeId(vector<int> &edge){
	edgeIds_.push_back(edge);
}//Cube::addEdgeId

void Cube::drawContour(cv::Mat dstCanvas, double fx, double fy, double cx, double cy, const cv::Scalar& color, bool hideLines /*= false*/){
    using namespace cv;
	if(dstCanvas.empty())
		dstCanvas = Mat::zeros(WIN_HH_MAX, WIN_WW_MAX, CV_8UC3); //��������, ���ɴ����ƺڱ���, �ߴ�Ĭ�� (WIN_HH, WIN_WW)
	
	//1, �ҳ�Ҫ�������Ķ���
	const size_t vertCnt = this->cuVerts8_.size(); //Ӧ==8, �������� .size() ��ȡ, �Է��д�
	CV_Assert(vertCnt == 8);

    set<int> occVertIds; //���ڵ����㼯��, ���ڱ����߻��ƻ���ʾ
    //����ÿһ������
	for(size_t i=0; i<vertCnt; i++){
		Vector3d vi = cuVerts8_[i];
        //�жϴ˶����Ƿ���ǰ�������������ڵ�, 
        //��������, �����������˶���, ����������Ƭ�ཻ, ���ڵ�

        //6������, ÿ������桾��ࡿ��ʾһ��:
        for(int fi=0; fi<3; fi++){
            //Ŀǰ��� facet �ǰ��������: 01, 23, 45; �˼����ͨ��, �ݶ�
            int which = facetVec_[2*fi].center_.z() < facetVec_[2*fi+1].center_.z() ? 2*fi : 2*fi+1;
            //�Թ������˶������Ƭ, (�������ǿ��ܵ�)
            if(facetVec_[which].isContainVert(i))
                continue;

            bool isInters = isVrayFacetIntersect(vi, which);
            if(isInters){
                occVertIds.insert(i);
                break;
            }
        }//for-fi-3
	}//for-vertCnt

    //hidLineSet �����߶κϼ�, ��set�����������˵�, ������ʾ�߶�
    //���Ʋ���: ���ѻ�����, �������ʵ��;
    set<set<int>> hidLineSet;
	const size_t facetCnt = this->facetVec_.size(); //Ӧ==6, Ҳ���� .size() ��ȡ, �Է��д�
	for(size_t i=0; i<facetCnt; i++){
		Facet &fi = facetVec_[i]; //��Ƭ i
        vector<int> &vidvec = fi.vertIds_; //���϶���
        const size_t vcnt = vidvec.size(); //Ӧ==4, Ҳ���� .size() ��ȡ, �Է��д�

        //�ж��Ƿ���Ƭ���ڵ�, ��ֻҪ����һ�����㱻�ڵ�, ������Ҳ���ڵ�
        bool isFaceOccluded = false;
        for(size_t fvId=0; fvId<vcnt; fvId++){
            if(occVertIds.count(vidvec[fvId]) > 0){
                isFaceOccluded = true;
                break;
            }
        }

		for(size_t j=0; j<vcnt; j++){
            size_t j1 = (j+1) % vcnt;
			Vector3d vertj = cuVerts8_[vidvec[j]],
                     vertj1 = cuVerts8_[vidvec[j1]];

            //�ж��߶��Ƿ���Ҫ�ػ�, ����
            set<int> lineSeg;
            lineSeg.insert(vidvec[j]);
            lineSeg.insert(vidvec[j1]);
            if(isFaceOccluded && hidLineSet.count(lineSeg) > 0)
                continue;

            Point ptj(vertj.x() / vertj.z() * fx + cx,
                      vertj.y() / vertj.z() * fy + cy);
            Point ptj1(vertj1.x() / vertj1.z() * fx + cx,
                      vertj1.y() / vertj1.z() * fy + cy);

            if(!isFaceOccluded)
                cv::line(dstCanvas, ptj, ptj1, color); //��8uc3, ����ɫ; �� 8uc1 ���ɫ
            else if(!hideLines){ //��������, ���������
                zcDashLine(dstCanvas, ptj, ptj1, color);
                //�ҵǼǴ�������
                hidLineSet.insert(lineSeg);
            }

		}//for-vcnt
	}//for-facetCnt

}//drawContour

bool Cube::isTrihedronVisible(const Affine3d &camPose, float fx, float fy, float cx, float cy, bool dbgPrint /*= false*/){
	using namespace cv;

	//camPose.inverse() * this->cuVerts8_
	Affine3d camPose_inv = camPose.inverse();
	//1, �ҵ�����ĵ�P �� idx, ����������ɼ�
	double minDep = 1e7; //��ʼ������ֵ
	int nearestVtxIdx = -1;
	vector<Vector3d> cuVerts8_c;
	for(size_t i=0; i < this->cuVerts8_.size(); i++){
		Vector3d v_cam_i = camPose_inv * this->cuVerts8_[i];
		if(v_cam_i.z() < minDep){
			minDep = v_cam_i.z();
			nearestVtxIdx = i;
		}
		cuVerts8_c.push_back(v_cam_i);
	}
	
	//2, �����ڱ���Ϣ, �ҵ���P���ڵ�,
	vector<int> adjVtxIdx; //nearest ��P ���ڵ�, ������, [0] ���� P����, size()=1+3=4;
	adjVtxIdx.push_back(nearestVtxIdx);
	for(size_t i=0; i < this->edgeIds_.size(); i++){
		vector<int> edge_i = this->edgeIds_[i];
		//��� vec �Ƿ� nearestVtxIdx, �ݲ��� std::find 
		int otherIdx = -1;
		bool edgeContainsP = false;
		if(edge_i[0] == nearestVtxIdx)
			otherIdx = edge_i[1];
		if(edge_i[1] == nearestVtxIdx)
			otherIdx = edge_i[0];

		if(otherIdx != -1)
			adjVtxIdx.push_back(otherIdx);

		if(adjVtxIdx.size() == 4)
			break; //�ҵ������Ͳ�����(ѭ�����Ѿ�add��һ��), ����ʵ��û��
	}
	
	//3, �ĵ�ͶӰ������ƽ��, �ж�: 
	//	a, �Ƿ�ȫ�Ƕ۽�, ��������, �������治�ɼ�, ���� false
	//	b, �۽�Ҳ����̫��, �����ݶ���ֵ THRESH=140~160��, ���̫��, ʵ�����ͼ�л���һ����ȫ����, ���ɼ�
	//�ۺ�, ��: ���нǱ��� 90��< theta < THRESH, ���򷵻� false
	vector<Point2f> pts; //���ص�, Ӧ��4��; �ݶ� float, ���� int
	//��ͶӰ:
	for(size_t i=0; i < adjVtxIdx.size(); i++){
		int ptIdx = adjVtxIdx[i]; //cu (this) ��Ķ�����;
		Vector3d &vtx_i = cuVerts8_c[ptIdx];
		//double z_i = cuVerts8_c[ptIdx].z();
		Point2f px_i;
		px_i.x = vtx_i.x() * fx / vtx_i.z() + cx;
		px_i.y = vtx_i.y() * fy / vtx_i.z() + cy;

		pts.push_back(px_i);
	}

	//���ж�: 90��< theta < THRESH; ����, д��
	Point2f px0 = pts[0];
	for(size_t i=1; i <= 3; i++){
		Point2f px_a = pts[i],
				px_b = pts[i==3 ? 1 : i+1];

		Point2f ray_a = px_a - px0,
				ray_b = px_b - px0;

		float dotAB = ray_a.dot(ray_b);
		if(dotAB > 0) //���
			return false;

		float cosAngle = dotAB / (cv::norm(ray_a) * cv::norm(ray_b));
		//if(dbgPrint)
		//	printf("cos[%d]:= %f\n", i, cosAngle);

		const float cosThresh = //-0.93969262; //160��
								-0.766044; //140��
		if(cosAngle < cosThresh) //�н�̫��, Ҳ����
			return false;
	}

	return true; //�ߵ����˵��������ɼ�
}//isTrihedronVisible


