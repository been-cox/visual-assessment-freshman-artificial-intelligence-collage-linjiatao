#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

int main()
{
	//棋盘格参数
	Size boardSize(10, 7);//内角点数
	double squareSize = 15.0;//格子大小

	//创建储存每张图片世界坐标和像素坐标的容器
	vector<vector<Point3f>> world_points;
	vector<vector<Point2f>> pixel_points;

	//获取世界坐标；
	vector<Point3f> world_point;
	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			world_point.push_back(Point3f(j * squareSize, i * squareSize, 0));
		}
	}

	//读取标定图片并获得像素坐标

	//把棋盘格图片的名字存进一个容器
	vector<String> image_names;
	glob("image_*", image_names);

	Mat gray;
	for (int i = 0; i < image_names.size(); i++)
	{
		//读取图像并转化为灰度图
		Mat image = imread(image_names[i]);
		cvtColor(image, gray, COLOR_BGR2GRAY);
		//获取像素坐标
		vector<Point2f> pixel_point;
		bool found = findChessboardCorners(gray, boardSize, pixel_point,
			CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE);

		//如果找到了，提高角点像素坐标的精度
		if (found)
		{
			cornerSubPix(gray, pixel_point, Size(11, 11), Size(-1, -1),
				TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));

			//验证角点检测正确
			//drawChessboardCorners(image, boardSize, pixel_point, found);
			//imshow("Corner", image);
			//waitKey(3000);

			//将像素坐标和世界坐标存入容器
			pixel_points.push_back(pixel_point);
			world_points.push_back(world_point);
		}
	}
	//开始进行相机标定
	Mat cameraMatrix, distCoeffs;//内参矩阵和畸变系数
	vector<Mat> rvecs, tvecs;//外参矩阵
	calibrateCamera(world_points, pixel_points, gray.size(), cameraMatrix, distCoeffs, rvecs, tvecs);

	//输出结果
	cout << "Camera Matrix:" << endl;
	cout << cameraMatrix << endl;
	cout << endl;
	cout << "Distortion Coefficients" << endl;
	cout << distCoeffs << endl;

	return 0;
}