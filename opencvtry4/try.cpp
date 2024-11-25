#include <opencv2/opencv.hpp>
#include <imgproc/imgproc.hpp>
#include <iostream>
#include <string>
using namespace std;
using namespace cv;

int main() {
    // 打开视频文件
    string s1, s2, s3, s4;
    s1 = "C:\\Users\\dw-42\\Desktop\\color-main\\color-main\\video\\move_red.avi";
    s2 = "C:\\Users\\dw-42\\Desktop\\color-main\\color-main\\video\\move_green.avi";
    s3 = "C:\\Users\\dw-42\\Desktop\\color-main\\color-main\\video\\random.avi";
    s4 = "C:\\Users\\dw-42\\Desktop\\color-main\\color-main\\video\\move_red_yellow.avi";
    //VideoCapture cap(0); // 0表示默认的摄像头，若有多个摄像头可以使用1、2等其他索引
    VideoCapture cap(s3);
    if (!cap.isOpened()) {
        cout << "Error: Could not open video!" << endl;
        return -1;
    }

    // 定义HSV颜色空间中红色、绿色和黄色的范围
    Scalar lower_red1(0, 150, 150), upper_red1(10, 255, 255);   // 红色范围
    Scalar lower_red2(170, 150, 150), upper_red2(180, 255, 255);
    Scalar lower_green(35, 100, 100), upper_green(85, 255, 255); // 绿色范围
    Scalar lower_yellow(20, 150, 150), upper_yellow(35, 255, 255); // 黄色范围

    Mat frame, hsv, mask_red1, mask_red2, mask_green, mask_yellow, mask;

    while (true) {
        // 读取视频的每一帧
        cap >> frame;
        if (frame.empty()) break;  // 如果读取完毕，退出循环

        // 转换到HSV空间
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        // 提取红色部分
        inRange(hsv, lower_red1, upper_red1, mask_red1);
        inRange(hsv, lower_red2, upper_red2, mask_red2);
        add(mask_red1, mask_red2, mask);  // 合并两个红色的mask

        // 提取绿色部分
        inRange(hsv, lower_green, upper_green, mask_green);

        // 提取黄色部分
        inRange(hsv, lower_yellow, upper_yellow, mask_yellow);

        // 进行噪声去除处理
        GaussianBlur(mask, mask, Size(5, 5), 0);
        GaussianBlur(mask_green, mask_green, Size(5, 5), 0);
        GaussianBlur(mask_yellow, mask_yellow, Size(5, 5), 0);

        // 计算红色、绿色和黄色区域的面积
        int red_area = countNonZero(mask);
        int green_area = countNonZero(mask_green);
        int yellow_area = countNonZero(mask_yellow);

        string status,status2=" ";

        // 判断信号灯的状态，并设置状态文本为颜色区域最大面积对应的颜色
        int max_area = max({ red_area, green_area,yellow_area });
        if (max_area == red_area) {
            status = "RED";
        }
        else if (max_area == green_area) {
            status = "GREEN";
        }
        else if (max_area == yellow_area) {
            status = "YELLOW";
        }
        else {
            status = "NOT";
        }
        if (yellow_area > 400) {
            status2 = "YELLOW OR TOOCLOSE";
        }

        // 查找红色区域的轮廓并绘制矩形框
        vector<vector<Point>> contours_red;
        findContours(mask, contours_red, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        for (size_t i = 0; i < contours_red.size(); i++) {
            Rect boundingBox = boundingRect(contours_red[i]);
            rectangle(frame, boundingBox, Scalar(0, 0, 255), 2); // 红色框
        }

        // 查找绿色区域的轮廓并绘制矩形框
        vector<vector<Point>> contours_green;
        findContours(mask_green, contours_green, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        for (size_t i = 0; i < contours_green.size(); i++) {
            Rect boundingBox = boundingRect(contours_green[i]);
            rectangle(frame, boundingBox, Scalar(0, 255, 0), 2); // 绿色框
        }

        // 查找黄色区域的轮廓并绘制矩形框
        vector<vector<Point>> contours_yellow;
        findContours(mask_yellow, contours_yellow, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        for (size_t i = 0; i < contours_yellow.size(); i++) {
            Rect yellowBox = boundingRect(contours_yellow[i]);
            bool isYellowInside = false;

            // 检查黄色区域是否在红色或绿色区域内
            for (size_t j = 0; j < contours_red.size(); j++) {
                Rect redBox = boundingRect(contours_red[j]);
                if ((yellowBox & redBox) == yellowBox) { // 判断黄色框是否完全在红色框内
                    isYellowInside = true;
                    break;
                }
            }

            if (!isYellowInside) {
                for (size_t j = 0; j < contours_green.size(); j++) {
                    Rect greenBox = boundingRect(contours_green[j]);
                    if ((yellowBox & greenBox) == yellowBox) { // 判断黄色框是否完全在绿色框内
                        isYellowInside = true;
                        break;
                    }
                }
            }

            // 如果黄色区域没有被红色或绿色区域包含，则绘制黄色框
            if (!isYellowInside) {
                rectangle(frame, yellowBox, Scalar(0, 255, 255), 2); // 黄色框
            }
        }

        // 在左上角显示状态，使用蓝色字体
        putText(frame, status+status2, Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);

        // 显示处理后的图像
        imshow("Traffic Light Detection", frame);

        // 按ESC退出
        if (waitKey(30) == 27) break;
    }

    // 释放视频资源
    cap.release();
    destroyAllWindows();

    return 0;
}