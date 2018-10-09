
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream> 
#include <io.h>
#include <Windows.h>
#include "opencv\cv.h"
#include "opencv\highgui.h"
#include <direct.h>
using namespace cv;
using namespace std;

extern int readDir(char *dirName, vector<string> &filesName);
#define M_PI 3.14159265358979323846

void coordinates34txt_1(Point2f src, float angle, Point2f & dst)
{
	dst.x = src.x*cos(angle) - src.y*sin(angle);
	dst.y = src.x*sin(angle) + src.y*cos(angle);

}


// 旋转中心，坐标为车牌中心，旋转区域车牌区域
void J_Rotate34txt_1(Mat src, int x1, int y1, int x2, int y2, float angle, Mat dst,
	int & l1, int & t1, int & r1, int & b1, Point & pt5, Point & pt6, Point & pt7, Point & pt8,int & flag)
{
	Point center;
	center.x = (x1 + x2) / 2 + 0.5; center.y = (y1 + y2) / 2 + 0.5;
	const double cosAngle = cos(angle);
	const double sinAngle = sin(angle);

	//原图像四个角的坐标变为以旋转中心的坐标系
	Point2d leftTop(x1 - center.x, -y1 + center.y); //(x1,y1)
	Point2d rightTop(x2 - center.x, -y1 + center.y); // (x2,y1)
	Point2d leftBottom(x1 - center.x, -y2 + center.y); //(x1,y2)
	Point2d rightBottom(x2 - center.x, -y2 + center.y); // (x2,y2)

	//以center为中心旋转后四个角的坐标
	Point2f transLeftTop, transRightTop, transLeftBottom, transRightBottom;



	coordinates34txt_1(leftTop, angle, transLeftTop);
	coordinates34txt_1(rightTop, angle, transRightTop);
	coordinates34txt_1(leftBottom, angle, transLeftBottom);
	coordinates34txt_1(rightBottom, angle, transRightBottom);



	double left = min({ transLeftTop.x, transRightTop.x, transLeftBottom.x, transRightBottom.x });
	double right = max({ transLeftTop.x, transRightTop.x, transLeftBottom.x, transRightBottom.x });
	double top = min({ transLeftTop.y, transRightTop.y, transLeftBottom.y, transRightBottom.y });
	double down = max({ transLeftTop.y, transRightTop.y, transLeftBottom.y, transRightBottom.y });

	int width = static_cast<int>(abs(left - right) + 0.5);
	int height = static_cast<int>(abs(top - down) + 0.5);

	// 左上角为原点的坐标
	Point2f pt1, pt2;
	pt1.x = transLeftTop.x + center.x, pt1.y = -transLeftTop.y + center.y;
	pt2.x = transRightTop.x + center.x, pt2.y = -transRightTop.y + center.y;
	Point2f pt3, pt4;
	pt3.x = transLeftBottom.x + center.x, pt3.y = -transLeftBottom.y + center.y;
	pt4.x = transRightBottom.x + center.x, pt4.y = -transRightBottom.y + center.y;



	int left1 = min({ pt1.x, pt2.x, pt3.x, pt4.x }) + 0.5;
	int right1 = max({ pt1.x, pt2.x, pt3.x, pt4.x }) + 0.5;
	int top1 = min({ pt1.y, pt2.y, pt3.y, pt4.y }) + 0.5;
	int down1 = max({ pt1.y, pt2.y, pt3.y, pt4.y }) + 0.5;

	if (left1<0 || left1>src.cols || top1<0 || top1>src.rows)
	{
		flag = 1;
		return;
	}
	if (right1<0 || right1>src.cols || down1<0 || down1>src.rows)
	{
		flag = 2;
		return;
	}

	if (left1+width>=src.cols)
	{
		flag = 3;
		return;
	}
	if (top1+height>=src.rows)
	{
		flag = 4;
		return;
	}

	const double num1 = -abs(left) * cosAngle - abs(top) * sinAngle + center.x;
	const double num2 = abs(left) * sinAngle - abs(top) * cosAngle + center.y;

	Vec3b *p;
	Mat roiplate(height,width,CV_8UC3);
	for (int i = 0; i < height; i++)
	{
		
		p = dst.ptr<Vec3b>(i + top1);
		for (int j = 0; j < width; j++)
		{
			//坐标变换
			int x = static_cast<int>(j  * cosAngle + i * sinAngle + num1 + 0.5);
			int y = static_cast<int>(-j * sinAngle + i * cosAngle + num2 + 0.5);

			if (x >= 0 && y >= 0 && x < src.cols && y < src.rows)
				p[j + left1] = src.ptr<Vec3b>(y)[x];
		}
	}


	l1 = left1; t1 = top1; r1 = right1; b1 = down1;

	pt5.x = pt3.x + 0.5; pt5.y = pt2.y + 0.5;
	pt6.x = pt4.x + 0.5; pt6.y = pt1.y + 0.5;
	pt7.x = pt1.x + 0.5; pt7.y = pt4.y + 0.5;
	pt8.x = pt2.x + 0.5; pt8.y = pt3.y + 0.5;



	if (pt5.x<0 || pt5.x>src.cols || pt5.y<0 || pt5.y>src.rows)
	{
		flag = 5;
		return;
	}
	if (pt6.x<0 || pt6.x>src.cols || pt6.y<0 || pt6.y>src.rows)
	{
		flag = 6;
		return;
	}
	if (pt7.x<0 || pt7.x>src.cols || pt7.y<0 || pt7.y>src.rows)
	{
		flag = 7;
		return;
	}
	if (pt8.x<0 || pt8.x>src.cols || pt8.y<0 || pt8.y>src.rows)
	{
		flag = 8;
		return;
	}

}
void DrawLine34txt_1(cv::Mat img, Point pt1, Point pt2, Point pt3, Point pt4)
{
	int thick = 1;

	CvScalar green = CV_RGB(0, 255, 0);
	cv::line(img, pt1, pt2, green, thick);
	cv::line(img, pt2, pt4, green, thick);
	cv::line(img, pt4, pt3, green, thick);
	cv::line(img, pt3, pt1, green, thick);
}

void PutText34txt_1(cv::Mat img, Point pt1, Point pt2, Point pt3, Point pt4)
{


	CvScalar green = CV_RGB(0, 255, 255);

	char file[16]; Point pt;
	sprintf(file, "1");
	pt.x = pt1.x ; pt.y = pt1.y;
	putText(img, file, pt, 1, 1, green);

	sprintf(file, "2");
	pt.x = pt2.x; pt.y = pt2.y;
	putText(img, file, pt, 1, 1, green);

	sprintf(file, "3");
	pt.x = pt3.x ; pt.y = pt3.y ;
	putText(img, file, pt, 1, 1, green);

	sprintf(file, "4");
	pt.x = pt4.x ; pt.y = pt4.y ;
	putText(img, file, pt, 1, 1, green);


}
// 只旋转车牌区域，以车牌中心为旋转中心，坐标原点
int test34_txt(int argc, char *argv[])
{


	string inputPath = "I:/车牌-fy";
	string inputPathtxt = "I:/车牌-fy/car_plate_v2train_68_510.txt";
	string outputPath = "I:/mtcnn-train/rotateResult";
	mkdir(outputPath.c_str());
	string outputsrc = "I:/mtcnn-train/rotateResult/src";
	mkdir(outputsrc.c_str());
	string outputtxt = "I:/mtcnn-train/rotateResult/src/000_one.txt";
	string outputdraw = "I:/mtcnn-train/rotateResult/draw";
	mkdir(outputdraw.c_str());

	fstream finRead;
	finRead.open(inputPathtxt, ios::in);
	if (!finRead.is_open())
	{
		cout << "finRead 文件数据打开错误！" << endl;
		system("pause");
		return false;
	}

	fstream finWrite;
	finWrite.open(outputtxt, ios::out);
	if (!finWrite.is_open())
	{
		cout << "finRead 文件数据打开错误！" << endl;
		system("pause");
		return false;
	}

	
	
	srand((unsigned)time(NULL));
	string line; int num = 0; int numnull = 0;
	while (getline(finRead, line))
	{
		if (line.empty())
		{
			continue;
		}
		//string line = "car1_0-10/0_169.jpg 1 66 453 222 509";
		cout << line.c_str() << endl;


		int npos = line.find_last_of('/');
		int npos2 = line.find_last_of('.');
		string name1 = line.substr(npos + 1, npos2 - npos - 1);

		int label; int x1, y1, x2, y2;
		string str123;
		stringstream str_s(line);
		str_s >> str123 >> label >> x1 >> y1 >> x2 >> y2;
		string v_img_ = inputPath + "/" + str123;

		Mat img = imread(v_img_.c_str());
		if (img.data == NULL)
		{

			printf("图像为空!\n");
			cout << v_img_.c_str() << endl;
			system("pause");
		}

		
		num++;
		

		int deg = rand() % 20 - 10;
		int  degarr1[] = { 0, deg };
		for (int j = 0; j < 2; j++)
		{
			
			int degree = degarr1[j];
			double radian = M_PI*degree*1.0 / 180;

			Mat dst = img.clone();
			Point pt1, pt2, pt3, pt4;
			Point pt5, pt6, pt7, pt8;
			pt1.x = x1; pt1.y = y1; pt2.x = x2; pt2.y = y2;
			int left = 0; int top = 0; int right = 0; int bottom = 0;
			int flag = 0;
			J_Rotate34txt_1(img, x1, y1, x2, y2, radian, dst, left, top, right, bottom, pt5, pt6, pt7, pt8,flag);

			if (flag == 1 || flag == 2 || flag == 5|| flag == 6 || flag == 7 || flag == 8)
			{
				printf("--------------flag=%d---------------\n",flag);
				numnull++;
				continue;
			}

			pt3.x = left; pt3.y = top; pt4.x = right; pt4.y = bottom;

			char intstr[128];
			sprintf(intstr, "-%da%d", j, degree);

			string str1 = outputsrc + "/" + name1 + intstr + ".jpg";
			imwrite(str1.c_str(), dst);

			finWrite << str1 << " " << label << " " << left << " " << top << " " << right << " " << bottom << " " <<
				pt5.x << " " << pt5.y << " " << pt6.x << " " << pt6.y << " " << pt7.x << " " << pt7.y << " " <<
				pt8.x << " " << pt8.y << endl;

			Mat drawimg = dst.clone();
			rectangle(drawimg, pt3, pt4, Scalar(0, 0, 255));
			DrawLine34txt_1(drawimg, pt5, pt6, pt7, pt8);
			PutText34txt_1(drawimg, pt5, pt6, pt7, pt8);



			string str2 = outputdraw + "/" + name1 + intstr + ".jpg";
			imwrite(str2.c_str(), drawimg);
		}


		int jjjjjj = 90;

	}

	finRead.close();
	finWrite.close();

	printf("图片数=%d,总个数=%d,丢弃数=%d\n",num,num*2-numnull,numnull);
	return 0;
}


int test33_txt(int argc, char *argv[])
{


	string inputPath = "I:/车牌-fy";
	string inputPathtxt = "I:/车牌-fy/car_plate_v2train_68_510.txt";
	string outputPath = "I:/mtcnn-train/rotateResult";
	mkdir(outputPath.c_str());
	string outputsrc = "I:/mtcnn-train/rotateResult/src";
	mkdir(outputsrc.c_str());
	string outputtxt = "I:/mtcnn-train/rotateResult/src/000_one.txt";
	string outputdraw = "I:/mtcnn-train/rotateResult/draw";
	mkdir(outputdraw.c_str());

	fstream finRead;
	finRead.open(inputPathtxt, ios::in);
	if (!finRead.is_open())
	{
		cout << "finRead 文件数据打开错误！" << endl;
		system("pause");
		return false;
	}

	fstream finWrite;
	finWrite.open(outputtxt, ios::out);
	if (!finWrite.is_open())
	{
		cout << "finRead 文件数据打开错误！" << endl;
		system("pause");
		return false;
	}



	srand((unsigned)time(NULL));
	string line; int num = 0; int numnull = 0;
	int numpic = 0; int numno = 0; int iii = 0;
	while (getline(finRead, line))
	{
		if (line.empty())
		{
			continue;
		}


		//string line = "car3/14_vechiel_dt_10645.jpg 3 232 771 390 828";
		//int deg = 1;

		//int deg = rand() % 20 - 10;

		int deg = rand() % 60 - 30;


		cout << line.c_str() << endl;
		cout <<"deg=" <<deg << endl;
		numpic++;

		int npos = line.find_last_of('/');
		int npos2 = line.find_last_of('.');
		string name1 = line.substr(npos + 1, npos2 - npos - 1);
		iii++;
		char file1[26];
		sprintf(file1,"%d",iii);
		name1 = name1 + "_"+file1;
		int label; int x1, y1, x2, y2;
		string str123;
		stringstream str_s(line);
		str_s >> str123 >> label >> x1 >> y1 >> x2 >> y2;

		

		string v_img_ = inputPath + "/" + str123;

		Mat img = imread(v_img_.c_str());
		if (img.data == NULL)
		{

			printf("图像为空!\n");
			cout << v_img_.c_str() << endl;
			system("pause");
		}


		


		int  degarr1[] = { 0, deg };
		for (int j = 0; j < 2; j++)
		{
			

			if (j == 1 && (label == 5 || label == 6 || label == 8 || label == 10))
			{
				numno++;
				continue;
			}

			num++;

			int degree = degarr1[j];

			

			double radian = M_PI*degree*1.0 / 180;

			Mat dst = img.clone();
			Point pt1, pt2, pt3, pt4;
			Point pt5, pt6, pt7, pt8;
			pt1.x = x1; pt1.y = y1; pt2.x = x2; pt2.y = y2;
			int left = 0; int top = 0; int right = 0; int bottom = 0;
			int flag = 0;
			J_Rotate34txt_1(img, x1, y1, x2, y2, radian, dst, left, top, right, bottom, pt5, pt6, pt7, pt8, flag);

			if (flag == 1 || flag == 2 || flag == 3 || flag ==4 || flag == 5 || flag == 6 || flag == 7 || flag == 8)
			{
				printf("--------------flag=%d---------------\n", flag);
				numnull++;
				continue;
			}

			pt3.x = left; pt3.y = top; pt4.x = right; pt4.y = bottom;

			char intstr[128];
			sprintf(intstr, "-%da%d", j, degree);

			string str1 = outputsrc + "/" + name1 + intstr + ".jpg";
			imwrite(str1.c_str(), dst);

			finWrite << str1 << " " << label << " " << left << " " << top << " " << right << " " << bottom << " " <<
				pt5.x << " " << pt5.y << " " << pt6.x << " " << pt6.y << " " << pt7.x << " " << pt7.y << " " <<
				pt8.x << " " << pt8.y <<" " << degree  << endl;

			Mat drawimg = dst.clone();
			rectangle(drawimg, pt3, pt4, Scalar(0, 0, 255));
			DrawLine34txt_1(drawimg, pt5, pt6, pt7, pt8);
			PutText34txt_1(drawimg, pt5, pt6, pt7, pt8);



			string str2 = outputdraw + "/" + name1 + intstr + ".jpg";
			imwrite(str2.c_str(), drawimg);
		}


		int jjjjjj = 90;

	}

	finRead.close();
	finWrite.close();

	printf("图片数=%d,不旋转=%d,总个数=%d,丢弃数=%d\n", numpic,numno, num  - numnull, numnull);
	return 0;
}

int test333_txt(int argc, char *argv[])
{


	string inputPath = "I:/车牌-fy";
	string inputPathtxt = "I:/车牌-fy/car_plate_v2train_68_510.txt";
	string outputPath = "I:/mtcnn-train/rotateResult";
	mkdir(outputPath.c_str());
	string outputsrc = "I:/mtcnn-train/rotateResult/src";
	mkdir(outputsrc.c_str());
	string outputtxt = "I:/mtcnn-train/rotateResult/src/000_one.txt";
	string outputdraw = "I:/mtcnn-train/rotateResult/draw";
	mkdir(outputdraw.c_str());

	fstream finRead;
	finRead.open(inputPathtxt, ios::in);
	if (!finRead.is_open())
	{
		cout << "finRead 文件数据打开错误！" << endl;
		system("pause");
		return false;
	}

	fstream finWrite;
	finWrite.open(outputtxt, ios::out);
	if (!finWrite.is_open())
	{
		cout << "finRead 文件数据打开错误！" << endl;
		system("pause");
		return false;
	}



	srand((unsigned)time(NULL));
	string line; int num = 0; int numnull = 0;
	int numpic = 0; int numno = 0; int iii = 0;
	while (getline(finRead, line))
	{
		if (line.empty())
		{
			continue;
		}


		//string line = "car3/14_vechiel_dt_10645.jpg 3 232 771 390 828";
		//int deg = 1;

		//int deg = rand() % 20 - 10;

		int deg1 = -rand() % 10 - 20;
		int deg2 = -rand() % 10 - 10;
		int deg3 = -rand() % 10 ;
		int deg4 = rand() % 10 ;
		int deg5 = rand() % 10 + 10;
		int deg6 = rand() % 10 + 20;
		
		cout << line.c_str() << endl;
		cout << "deg," << deg1 << "," << deg2 << "," << deg3 << "," << deg4 << "," << deg5 << "," << deg6 << endl;
		numpic++;

		int npos = line.find_last_of('/');
		int npos2 = line.find_last_of('.');
		string name1 = line.substr(npos + 1, npos2 - npos - 1);
		iii++;
		char file1[26];
		sprintf(file1, "%d", iii);
		name1 = name1 + "_" + file1;
		int label; int x1, y1, x2, y2;
		string str123;
		stringstream str_s(line);
		str_s >> str123 >> label >> x1 >> y1 >> x2 >> y2;



		string v_img_ = inputPath + "/" + str123;

		Mat img = imread(v_img_.c_str());
		if (img.data == NULL)
		{

			printf("图像为空!\n");
			cout << v_img_.c_str() << endl;
			system("pause");
		}

		int  degarr1[] = { deg1, deg2, deg3, 0, deg4, deg5, deg6 };
		for (int j = 0; j < 7; j++)
		{


			if (j == 1 && (label == 5 || label == 6 || label == 8 || label == 10))
			{
				numno++;
				continue;
			}

			num++;

			int degree = degarr1[j];



			double radian = M_PI*degree*1.0 / 180;

			Mat dst = img.clone();
			Point pt1, pt2, pt3, pt4;
			Point pt5, pt6, pt7, pt8;
			pt1.x = x1; pt1.y = y1; pt2.x = x2; pt2.y = y2;
			int left = 0; int top = 0; int right = 0; int bottom = 0;
			int flag = 0;
			J_Rotate34txt_1(img, x1, y1, x2, y2, radian, dst, left, top, right, bottom, pt5, pt6, pt7, pt8, flag);

			if (flag == 1 || flag == 2 || flag == 3 || flag == 4 || flag == 5 || flag == 6 || flag == 7 || flag == 8)
			{
				printf("--------------flag=%d---------------\n", flag);
				numnull++;
				continue;
			}

			pt3.x = left; pt3.y = top; pt4.x = right; pt4.y = bottom;

			char intstr[128];
			sprintf(intstr, "-%da%d", j, degree);

			string str1 = outputsrc + "/" + name1 + intstr + ".jpg";
			imwrite(str1.c_str(), dst);

			finWrite << str1 << " " << label << " " << left << " " << top << " " << right << " " << bottom << " " <<
				pt5.x << " " << pt5.y << " " << pt6.x << " " << pt6.y << " " << pt7.x << " " << pt7.y << " " <<
				pt8.x << " " << pt8.y << " " << degree << endl;

			Mat drawimg = dst.clone();
			rectangle(drawimg, pt3, pt4, Scalar(0, 0, 255));
			DrawLine34txt_1(drawimg, pt5, pt6, pt7, pt8);
			PutText34txt_1(drawimg, pt5, pt6, pt7, pt8);



			string str2 = outputdraw + "/" + name1 + intstr + ".jpg";
			imwrite(str2.c_str(), drawimg);
		}


		int jjjjjj = 90;

	}

	finRead.close();
	finWrite.close();

	printf("图片数=%d,不旋转=%d,总个数=%d,丢弃数=%d\n", numpic, numno, num - numnull, numnull);
	return 0;
}







