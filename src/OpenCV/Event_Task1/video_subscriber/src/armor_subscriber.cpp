#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono> 

using namespace std;
using namespace cv;

//定义灯条结构体
struct Light
{
	RotatedRect rect;
	Point2f center;
	float width;
	float height;
	float angle;

	//通过传入一个旋转矩形，构建灯条结构体
	Light(RotatedRect light)
	{
		rect = light;
		center = light.center;
		width = light.size.width;
		height = light.size.height;
		angle = light.angle;

		//对角度进行变换，方便我们判断两灯条是否平行
		if (angle > 90)
		{
			angle -= 180.0;
		}
	}
};

class Subscriber : public rclcpp::Node
{
private:
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub;
    cv::Mat armor_img;
    
    // 相机内参和装甲板3D坐标
    Mat cameraMatrix, distCoeffs;
    vector<Point3f> armor3dPoints;

    void image_callback(const sensor_msgs::msg::Image::SharedPtr Img)
    {
        // 记录开始时间，用于计算处理延迟
        auto start_time = std::chrono::high_resolution_clock::now();

        // 将 ROS 图像消息转换为 OpenCV Mat
        cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(Img, "bgr8");
        Mat img = cv_ptr->image;

		//通道相减得到灰度图并转化为二值图
		vector<Mat> channels;
		split(img, channels); //得到三通道灰度图
		Mat gray_img;
		subtract(channels[0], channels[2], gray_img);//蓝色通道灰度图减去红色通道灰度图
		Mat binary_img;
		threshold(gray_img, binary_img, 80, 255, THRESH_BINARY);

		//膨胀操作
		Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
		dilate(binary_img, binary_img, kernel);

		//找轮廓
		vector<vector<Point>> lightcontours;
		findContours(binary_img, lightcontours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		//遍历轮廓提取灯条
		vector<Light> targetLight;
		for (const auto& contour : lightcontours)
		{
			//过滤掉面积太小
			if (contourArea(contour) < 10)
				continue;
			//椭圆拟合得到外接旋转矩形并以此构建灯条
			RotatedRect lightRect = fitEllipse(contour);
			Light light(lightRect);
			//限制灯条的宽高比例
			float ratio = light.height / light.width;
			if (ratio < 1.1 || ratio>15.0)
				continue;
			//过滤掉角度倾斜过大的灯条
			if (abs(light.angle) > 35.0)
				continue;

			targetLight.push_back(light);
		}
		//给灯条排序
		sort(targetLight.begin(), targetLight.end(), [](const Light& a, const Light& b)
			{return a.center.x < b.center.x; });

		//开始匹配灯条
		if (targetLight.size() >= 2)
		{
			for (size_t i = 0; i < targetLight.size(); i++)
			{
				for (size_t j = i+1; j < targetLight.size(); j++)
				{
					const Light& leftLight = targetLight[i];
					const Light& rightLight = targetLight[j];

					//两灯条须保持平行
					float angleDiff = abs(leftLight.angle - rightLight.angle);
					if (angleDiff > 25.0)
						continue;
					//限制长度差
					float lengthDiff = abs(leftLight.height - rightLight.height) / max(leftLight.height, rightLight.height);
					if (lengthDiff > 0.2)
						continue;
					//限制灯条中心点错位
					float len = (leftLight.height + rightLight.height) / 2.0;
					float yDiff = abs(leftLight.center.y - rightLight.center.y);
					if (yDiff > 0.7 * len)
						continue;
					//限制装甲板长宽比
					float xDiff = abs(rightLight.center.x - leftLight.center.x);
					float ratio = xDiff / len;
					if (ratio < 0.5 || ratio>6.0)
						continue;
					//限制装甲板的倾斜程度
					float armorAngle = atan2(yDiff, xDiff) * 180 / CV_PI;
					if (armorAngle > 20.0)
						continue;
					//匹配成功，确定装甲板中心
					Point2f armorCenter = Point2f((leftLight.center.x + rightLight.center.x) / 2, (leftLight.center.y + rightLight.center.y) / 2);

					//找到装甲板的最小外接矩形
					vector<Point2f> armorPoints;
					Point2f leftPoints[4];
					Point2f rightPoints[4];
				    //前面运行发现光晕导致矩形框绘制过大，于是在确定矩形框之前先缩小灯条矩形
					RotatedRect slimLeft = leftLight.rect;
					RotatedRect slimRight = rightLight.rect;

					//适当调整灯条的宽高
					slimLeft.size.width *= 0.4;
					slimRight.size.width *= 0.4;

					slimLeft.size.height *= 0.8;
					slimRight.size.height *= 0.8;
					
					//提取灯条4个顶点
					slimLeft.points(leftPoints);
					slimRight.points(rightPoints);

					for (int k = 0; k < 4; k++)
					{
						armorPoints.push_back(leftPoints[k]);
						armorPoints.push_back(rightPoints[k]);
					}
					//根据8个点确定最小外接矩形
					RotatedRect armorRect = minAreaRect(armorPoints);
					Point2f rectPoints[4];
					armorRect.points(rectPoints);

					//对像素坐标系上的顶点进行排序
					//先获取(左上，右上，左下，右下)顶点
					vector<Point2f> sortedPoints(rectPoints, rectPoints + 4);
					//先把左边的两顶点放在索引0和1的位置，右边的两顶点放在索引2和3的位置
					sort(sortedPoints.begin(), sortedPoints.end(), [](const Point2f& a, const Point2f& b)
						{return a.x < b.x; });
					Point2f tl, bl, tr, br;
					//对左边的两个顶点进行排序，y小的为左上顶点
					if (sortedPoints[0].y < sortedPoints[1].y)
					{
						tl = sortedPoints[0];
						bl = sortedPoints[1];
					}
					else
					{
						tl = sortedPoints[1];
						bl = sortedPoints[0];
					}
					//对右边的两个顶点进行排序，y小的是右上顶点
					if (sortedPoints[2].y < sortedPoints[3].y)
					{
						tr = sortedPoints[2];
						br = sortedPoints[3];
					}
					else
					{
						tr = sortedPoints[3];
						br = sortedPoints[2];
					}
					//将四个点按照指定顺序放入容器
					vector<Point2f> imagePoints = { tl,tr,br,bl };

					//旋转向量和平移向量
					Mat rvec, tvec;
                    // 只有在成功加载了相机内参的情况下才进行 PnP 解算
                    if (!cameraMatrix.empty() && !distCoeffs.empty()) {
					    bool pnpSuccess = solvePnP(armor3dPoints, imagePoints, cameraMatrix, distCoeffs, rvec, tvec, false, SOLVEPNP_IPPE);
					    
                        if (pnpSuccess)
					    {
					    	//获取装甲板相对于相机的坐标
					    	double x = tvec.at<double>(0, 0);
					    	double y = tvec.at<double>(1, 0);
					    	double z = tvec.at<double>(2, 0);
					    	//计算距离并转化成米
					    	double distance = sqrt(x * x + y * y + z * z)/1000.0;

					    	string Distance = format("Distance:%.2fm", distance);
					    	putText(img, Distance, Point(tl.x, tl.y - 10), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255), 2);
					    }
                    }

                    //绘制出装甲板的矩形外框
					for (int i = 0; i < 4; i++)
					{
						line(img, rectPoints[i], rectPoints[(i + 1) % 4], Scalar(255, 0, 0), 2);
					}
					circle(img, armorCenter, 4, Scalar(0, 0, 255), -1);
				}
			}
		}
        // 核心装甲板识别算法结束

        // 记录结束时间并计算延迟
        auto end_time = std::chrono::high_resolution_clock::now();
        double delay_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

        // 将延迟信息绘制在画面右上角
        string delayText = format("Process Time: %.2f ms", delay_ms);
        putText(img, delayText, Point(20, 40), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);

        //将处理好的画面交给 display 线程显示
        armor_img = img.clone();
    }

    void display()
    {
        cv::namedWindow("Armor Detection Video", cv::WINDOW_NORMAL);
        while (rclcpp::ok()) {
            if (!armor_img.empty()) 
            {
                cv::imshow("Armor Detection Video", armor_img);
            }
            
            if (cv::waitKey(5) == 27) 
            { // 按Esc退出
                cout << "已退出节点" << endl;
                rclcpp::shutdown();
                break;
            }
            rclcpp::spin_some(this->shared_from_this());
        }
        cv::destroyAllWindows();
    }

public:
    Subscriber() : Node("Taurus_sub")
    {
        //读取相机标定文件,和视频读取一样，为了保证正常读取，在文件所在目录ros2 run
	    FileStorage fs("camera_calib.yaml", FileStorage::READ);
	    if (!fs.isOpened())
	    {
	    	RCLCPP_WARN(this->get_logger(), "无法打开相机标定文件");
	    } 
        else
        {
	        fs["camera_matrix"] >> cameraMatrix;
	        fs["distortion_coeddicients"] >> distCoeffs;
	        fs.release();
        }

	    //定义装甲板的物理坐标
	    float armor_width = 135.0;
	    float armor_height = 55.0;
	    //定义装甲板的中心为3D世界的原点
	    armor3dPoints.push_back(Point3f(-armor_width / 2, -armor_height / 2, 0));
	    armor3dPoints.push_back(Point3f(armor_width / 2, -armor_height / 2, 0));
	    armor3dPoints.push_back(Point3f(armor_width / 2, armor_height / 2, 0));
	    armor3dPoints.push_back(Point3f(-armor_width / 2, armor_height / 2, 0));

        // 创建订阅者
        image_sub = this->create_subscription<sensor_msgs::msg::Image>(
            "Taurus_image", 10,
            std::bind(&Subscriber::image_callback, this, std::placeholders::_1));
            
        RCLCPP_INFO(this->get_logger(), "装甲板识别订阅者已启动");
    }

    void run()
    {
        display();
    }
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Subscriber>();
    node->run();
    rclcpp::shutdown();
    return 0;
}
//提示：ros2 run前要先执行export LD_PRELOAD=/lib/x86_64-linux-gnu/libpthread.so.0，优先加载系统库