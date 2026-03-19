#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>

class Publisher : public rclcpp::Node
{
private:
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub;
    rclcpp::TimerBase::SharedPtr timer;  
    cv::VideoCapture cap;

    void timer_callback() 
    {
        cv::Mat img;
        cap >> img;

        // 如果视频播放完毕，自动重置到第一帧循环播放
        if (img.empty()) {
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);
            cap >> img;
            if(img.empty()) return;
        }

        // 转换为 ROS Image 消息并发布
        sensor_msgs::msg::Image::SharedPtr img_ptr = 
            cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", img).toImageMsg();
        
        image_pub->publish(*img_ptr);

    }

public:
    Publisher() : Node("Taurus_pub")
    {
        // 创建图像发布者
        image_pub = this->create_publisher<sensor_msgs::msg::Image>("Taurus_image", 10);

        // 打开视频文件 (请在视频所在目录ros2 run，保证视频能被读取到)
        cap.open("armor_blue.mp4");
        if (!cap.isOpened()) {
            RCLCPP_ERROR(this->get_logger(), "视频读取失败");
        }
        
        // 创建定时器，33毫秒发布一次
        timer= this->create_wall_timer(
            std::chrono::milliseconds(33),
            std::bind(&Publisher::timer_callback, this));
        
        RCLCPP_INFO(this->get_logger(), "视频流发布者已启动，以约 30FPS 发布...");
    }
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Publisher>();
    rclcpp::spin(node); 
    rclcpp::shutdown();
    return 0;
}