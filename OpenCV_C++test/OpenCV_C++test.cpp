#include<opencv2/opencv.hpp>
#include<iostream>
#include<algorithm>

using namespace std;
using namespace cv;

const int NumFrame = 10;//历史帧
const int NumKnn = 3;//knn的界
const double thresh = 25;//阈值

//--------------------------排序规则，后期用于提取轮廓
bool cmp(vector<Point>p1, vector<Point>p2)
{
	return contourArea(p1) > contourArea(p2);
}

//--------------------------历史帧
struct HistoryFrame
{
	bool IsBackground[NumFrame];//是否是背景
	uchar HistoryGray[NumFrame];//背景图的灰度值
};


int main()
{
	cout << "摄像头读取按'5'键退出";
	VideoCapture capture(0);//打开摄像头

	//	VideoCapture capture("C:\\Users\\18700\\Downloads\\tiki taka.mp4");//读取视频
	Mat frame;//定义帧
	Mat KnnFrame;//背景消去的图

	int rows = capture.get(CAP_PROP_FRAME_HEIGHT);//图片行
	int cols = capture.get(CAP_PROP_FRAME_WIDTH);//图片列
	int nn = rows * cols;


	//为每一个像素点定义一个历史帧并初始化
	HistoryFrame* HF = new HistoryFrame[nn];
	for (int i = 0; i < rows * cols; i++) {
		for (int j = 0; j < NumFrame; j++) {
			HF[i].IsBackground[j] = 0;
			HF[i].HistoryGray[j] = 0;
		}
	}


	KnnFrame.create(rows, cols, CV_8UC1);//设置背景消去图为单通道
	int CurFrame = 0;//当前帧
	int StartFrame = 10;//开始帧


	while (capture.read(frame)) {
		KnnFrame.setTo(Scalar(255));//将所有像素点设置为255
		Mat frame1 = frame;//记录帧（后期需要在原视频上做标记）
		cvtColor(frame, frame, COLOR_BGR2GRAY);//将原图转为灰度图
		//摄像头读取按'5'键退出
		int c = waitKey(10);
		if (c == '5')
			break;
		//判断每个像素点是背景还是前景（KNN规则）
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				uchar gray = frame.at<uchar>(i, j);
				int fit1 = 0;
				int fit2 = 0;
				for (int k = 0; k < NumFrame; k++) {
					if (fabs(gray - HF[i * cols + j].HistoryGray[k]) < thresh) {
						fit1++;
						if (HF[i * cols + j].IsBackground[k]) {
							fit2++;
						}
					}
				}
				if (fit2 >= NumKnn)KnnFrame.at<uchar>(i, j) = 0;//如果是背景的次数超过设定值则判断为背景
				//更新该像素点的历史信息
				int index = CurFrame % NumFrame;
				HF[i * cols + j].IsBackground[index] = fit1 >= NumKnn ? 1 : 0;
				HF[i * cols + j].HistoryGray[index] = gray;
			}

		}
		CurFrame++;
		medianBlur(KnnFrame, KnnFrame, 5);//均值滤波，去除椒盐噪声
		//舍弃前10帧，因为没有真实的历史信息
		if (CurFrame > StartFrame) {
			vector<vector<Point>>contours;//定义轮廓
			findContours(KnnFrame, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);//寻找轮廓
			sort(contours.begin(), contours.end(), cmp);
			for (int i = 0; i < contours.size(); i++) {
				if (contourArea(contours[i]) < contourArea(contours[0]) / 12.0) {
					break;
				}
				Rect rect = boundingRect(contours[i]);
				rectangle(frame1, rect, Scalar(0, 0, 255), 1, 8);//画矩形
			}
			imshow("frame1", frame1);
			imshow("KNN", KnnFrame);
		}
		if ((waitKey(10)) == 27)break;
	}
}
/*
* #include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

const string window_name = "用户界面";

#define USE_CAMERA
//#define USE_VIDEO

int main()
{
	Mat frame;

	double brightness = 0;		//亮度
	double contrast = 0;		//对比度
	double saturation = 0;		//饱和度
	double hue = 0;				//色调
	double gain = 0;			//增益
	double exposure = 0;		//曝光
	double white_balance = 0;	//白平衡

	double pos_msec = 0;		//当前视频位置(ms)
	double pos_frame = 0;		//从0开始下一帧的索引
	double pos_avi_ratio = 0;	//视频中的相对位置(范围为0.0到1.0)
	double frame_width = 0;		//视频帧的像素宽度
	double frame_height = 0;	//视频帧的像素高度
	double fps = 0;				//帧速率
	double frame_count = 0;		//视频总帧数
	double video_duration = 0.00;	//视频时长
	double format = 0;			//格式

#ifdef USE_VIDEO
	const string file_name = "201910915314.avi";
	VideoCapture capture(file_name);

	frame_width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
	frame_height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
	fps = capture.get(cv::CAP_PROP_FPS);
	frame_count = capture.get(cv::CAP_PROP_FRAME_COUNT);
	format = capture.get(cv::CAP_PROP_FORMAT);
	pos_avi_ratio = capture.get(cv::CAP_PROP_POS_AVI_RATIO);
	video_duration = frame_count / fps;

	cout << "---------------------------------------------" << endl;
	cout << "视频中的相对位置(范围为0.0到1.0):" << pos_avi_ratio << endl;
	cout << "视频帧的像素宽度:" << frame_width << endl;
	cout << "视频帧的像素高度:" << frame_height << endl;
	cout << "录制视频的帧速率(帧/秒):" << fps << endl;
	cout << "视频文件总帧数:" << frame_count << endl;
	cout << "图像的格式:" << format << endl;
	cout << "视频时长:" << video_duration << endl;
	cout << "---------------------------------------------" << endl;
#endif // USE_VIDEO

#ifdef USE_CAMERA
	VideoCapture capture(0);
	brightness = capture.get(cv::CAP_PROP_BRIGHTNESS);
	contrast = capture.get(cv::CAP_PROP_CONTRAST);
	saturation = capture.get(cv::CAP_PROP_SATURATION);
	hue = capture.get(cv::CAP_PROP_HUE);
	gain = capture.get(cv::CAP_PROP_GAIN);
	exposure = capture.get(cv::CAP_PROP_EXPOSURE);
	white_balance = capture.get(cv::CAP_PROP_WHITE_BALANCE_BLUE_U);

	std::cout << "---------------------------------------------" << endl;
	std::cout << "摄像头亮度：" << brightness << endl;
	std::cout << "摄像头对比度：" << contrast << endl;
	std::cout << "摄像头饱和度：" << saturation << endl;
	std::cout << "摄像头色调：" << hue << endl;
	std::cout << "摄像头增益：" << gain << endl;
	std::cout << "摄像头曝光度：" << exposure << endl;
	std::cout << "摄像头白平衡：" << white_balance << endl;
	std::cout << "---------------------------------------------" << endl;
#endif // USE_CAMERA

	namedWindow(window_name, WINDOW_AUTOSIZE);
	while (capture.isOpened())
	{
		capture >> frame;

#ifdef USE_VIDEO
		pos_msec = capture.get(cv::CAP_PROP_POS_MSEC);
		pos_frame = capture.get(cv::CAP_PROP_POS_FRAMES);
		pos_avi_ratio = capture.get(cv::CAP_PROP_POS_AVI_RATIO);
		cout << "---------------------------------------------" << endl;
		cout << "视频文件中当前位置(ms):" << pos_msec << endl;
		cout << "从0开始下一帧的索引:" << pos_frame << endl;
		cout << "视频中的相对位置(范围为0.0到1.0):" << pos_avi_ratio << endl;
		cout << "---------------------------------------------" << endl;
#endif // USE_VIDEO

		imshow(window_name, frame);
		if (waitKey(60) == 27)
		{
			break;
		}
	}
	capture.release();
	destroyAllWindows();
	return 0;
}
*/