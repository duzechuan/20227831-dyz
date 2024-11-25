#include <opencv2/opencv.hpp>
#include <imgproc/imgproc.hpp>
#include <iostream>
#include <string>
using namespace std;
using namespace cv;

int main() {
    // ����Ƶ�ļ�
    string s1, s2, s3, s4;
    s1 = "C:\\Users\\dw-42\\Desktop\\color-main\\color-main\\video\\move_red.avi";
    s2 = "C:\\Users\\dw-42\\Desktop\\color-main\\color-main\\video\\move_green.avi";
    s3 = "C:\\Users\\dw-42\\Desktop\\color-main\\color-main\\video\\random.avi";
    s4 = "C:\\Users\\dw-42\\Desktop\\color-main\\color-main\\video\\move_red_yellow.avi";
    //VideoCapture cap(0); // 0��ʾĬ�ϵ�����ͷ�����ж������ͷ����ʹ��1��2����������
    VideoCapture cap(s3);
    if (!cap.isOpened()) {
        cout << "Error: Could not open video!" << endl;
        return -1;
    }

    // ����HSV��ɫ�ռ��к�ɫ����ɫ�ͻ�ɫ�ķ�Χ
    Scalar lower_red1(0, 150, 150), upper_red1(10, 255, 255);   // ��ɫ��Χ
    Scalar lower_red2(170, 150, 150), upper_red2(180, 255, 255);
    Scalar lower_green(35, 100, 100), upper_green(85, 255, 255); // ��ɫ��Χ
    Scalar lower_yellow(20, 150, 150), upper_yellow(35, 255, 255); // ��ɫ��Χ

    Mat frame, hsv, mask_red1, mask_red2, mask_green, mask_yellow, mask;

    while (true) {
        // ��ȡ��Ƶ��ÿһ֡
        cap >> frame;
        if (frame.empty()) break;  // �����ȡ��ϣ��˳�ѭ��

        // ת����HSV�ռ�
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        // ��ȡ��ɫ����
        inRange(hsv, lower_red1, upper_red1, mask_red1);
        inRange(hsv, lower_red2, upper_red2, mask_red2);
        add(mask_red1, mask_red2, mask);  // �ϲ�������ɫ��mask

        // ��ȡ��ɫ����
        inRange(hsv, lower_green, upper_green, mask_green);

        // ��ȡ��ɫ����
        inRange(hsv, lower_yellow, upper_yellow, mask_yellow);

        // ��������ȥ������
        GaussianBlur(mask, mask, Size(5, 5), 0);
        GaussianBlur(mask_green, mask_green, Size(5, 5), 0);
        GaussianBlur(mask_yellow, mask_yellow, Size(5, 5), 0);

        // �����ɫ����ɫ�ͻ�ɫ��������
        int red_area = countNonZero(mask);
        int green_area = countNonZero(mask_green);
        int yellow_area = countNonZero(mask_yellow);

        string status,status2=" ";

        // �ж��źŵƵ�״̬��������״̬�ı�Ϊ��ɫ������������Ӧ����ɫ
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

        // ���Һ�ɫ��������������ƾ��ο�
        vector<vector<Point>> contours_red;
        findContours(mask, contours_red, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        for (size_t i = 0; i < contours_red.size(); i++) {
            Rect boundingBox = boundingRect(contours_red[i]);
            rectangle(frame, boundingBox, Scalar(0, 0, 255), 2); // ��ɫ��
        }

        // ������ɫ��������������ƾ��ο�
        vector<vector<Point>> contours_green;
        findContours(mask_green, contours_green, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        for (size_t i = 0; i < contours_green.size(); i++) {
            Rect boundingBox = boundingRect(contours_green[i]);
            rectangle(frame, boundingBox, Scalar(0, 255, 0), 2); // ��ɫ��
        }

        // ���һ�ɫ��������������ƾ��ο�
        vector<vector<Point>> contours_yellow;
        findContours(mask_yellow, contours_yellow, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        for (size_t i = 0; i < contours_yellow.size(); i++) {
            Rect yellowBox = boundingRect(contours_yellow[i]);
            bool isYellowInside = false;

            // ����ɫ�����Ƿ��ں�ɫ����ɫ������
            for (size_t j = 0; j < contours_red.size(); j++) {
                Rect redBox = boundingRect(contours_red[j]);
                if ((yellowBox & redBox) == yellowBox) { // �жϻ�ɫ���Ƿ���ȫ�ں�ɫ����
                    isYellowInside = true;
                    break;
                }
            }

            if (!isYellowInside) {
                for (size_t j = 0; j < contours_green.size(); j++) {
                    Rect greenBox = boundingRect(contours_green[j]);
                    if ((yellowBox & greenBox) == yellowBox) { // �жϻ�ɫ���Ƿ���ȫ����ɫ����
                        isYellowInside = true;
                        break;
                    }
                }
            }

            // �����ɫ����û�б���ɫ����ɫ�������������ƻ�ɫ��
            if (!isYellowInside) {
                rectangle(frame, yellowBox, Scalar(0, 255, 255), 2); // ��ɫ��
            }
        }

        // �����Ͻ���ʾ״̬��ʹ����ɫ����
        putText(frame, status+status2, Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);

        // ��ʾ������ͼ��
        imshow("Traffic Light Detection", frame);

        // ��ESC�˳�
        if (waitKey(30) == 27) break;
    }

    // �ͷ���Ƶ��Դ
    cap.release();
    destroyAllWindows();

    return 0;
}