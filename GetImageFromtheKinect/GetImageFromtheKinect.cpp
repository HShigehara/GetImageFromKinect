#include<iostream>
#include<sstream>

#include<Windows.h>
#include<mmsystem.h>
#include<NuiApi.h>

#include<opencv2/opencv.hpp>

#include<direct.h>

#define ERROR_CHECK(ret) \
	if(ret != S_OK){ \
		std::stringstream ss; \
		ss << "failed " #ret " " << std::hex << ret << std::endl; \
		throw std::runtime_error(ss.str().c_str()); \
		}

const NUI_IMAGE_RESOLUTION CAMERA_RESOLUTION = NUI_IMAGE_RESOLUTION_640x480;

class Kinect
{
private:
	INuiSensor* kinect;
	HANDLE imageStreamHandle;

	DWORD width;
	DWORD height;

	int savedCount = 0;
	char fileName[64];
	char directoryName[64];
	char listName[64];

	FILE *fp;

public:
	Kinect(){}

	~Kinect()
	{
		if (kinect != 0){
			kinect->NuiShutdown();
			kinect->Release();
		}
	}

	void initialize()
	{
		createInstance();
		ERROR_CHECK(kinect->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR));
		ERROR_CHECK(kinect->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, CAMERA_RESOLUTION, 0, 2, 0, &imageStreamHandle));
		::NuiImageResolutionToSize(CAMERA_RESOLUTION, width, height);
	}

	void run()
	{
		cv::Mat image;

		int savePredeterminedNum;
		std::cout << "撮影する枚数を入力してください => ";
		std::cin >> savePredeterminedNum;

		std::cout << savePredeterminedNum << " 枚キャプチャーします" << std::endl;

		if (savePredeterminedNum > 0){
			//SYSTEMTIME st;
			//GetLocalTime(&st);
			//sprintf_s(directoryName, "[%4d%02d%02d]%02d_%02d_%02d_capture", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			//_mkdir(directoryName); //ディレクトリの作成

			_mkdir("capture");

			//sprintf_s(listName, "%s/imglist.xml", directoryName);
			//fopen_s(&fp, "capture/imglist.xml", "w");
			//fprintf_s(fp, "<?xml version=\"1.0\"?>\n");
			//fprintf_s(fp, "<opencv_storage>\n");
			//fprintf_s(fp, "<images>\n");

			while (1){
				drawRgbImage(image);
				cv::namedWindow("Kinect", CV_WINDOW_NORMAL);
				cv::imshow("Kinect", image);
				int key = cv::waitKey(10);
				if (key == 'q' || savedCount == savePredeterminedNum){
					std::cout << "キャプチャーが完了しました" << std::endl;
					//fprintf_s(fp, "</images>\n");
					//fprintf_s(fp, "</opencv_storage>\n");
					//fclose(fp);
					break;
				}
				else if (key == 'p'){
					PlaySound(TEXT("shutter_nikon.wav"), NULL, (SND_ASYNC | SND_FILENAME));
					savedCount++;
					//sprintf_s(fileName, "%s/capture_%d.jpg", directoryName,savedCount);
					sprintf_s(fileName, "capture/capture_%d.bmp", savedCount);
					cv::imwrite(fileName, image);
					//fprintf_s(fp, "%s\n", fileName);
					std::cout << fileName << " Captured." << std::endl;
				}
			}
		}
		else{
			std::cout << "保存する枚数を正しく指定しなおしてください" << std::endl;
		}
	}

private:
	void createInstance()
	{
		int count = 0;
		ERROR_CHECK(::NuiGetSensorCount(&count));
		if (count == 0){
			throw std::runtime_error("Kinectを接続してください");
		}
		ERROR_CHECK(::NuiCreateSensorByIndex(0, &kinect));
		HRESULT status = kinect->NuiStatus();
		if (status != S_OK){
			throw std::runtime_error("Kinectが利用可能ではありません");
		}
	}

	void drawRgbImage(cv::Mat& image)
	{
		NUI_IMAGE_FRAME imageFrame = { 0 };
		ERROR_CHECK(kinect->NuiImageStreamGetNextFrame(imageStreamHandle, INFINITE, &imageFrame));
		NUI_LOCKED_RECT colorData;
		imageFrame.pFrameTexture->LockRect(0, &colorData, 0, 0);
		image = cv::Mat(height, width, CV_8UC4, colorData.pBits);
		ERROR_CHECK(kinect->NuiImageStreamReleaseFrame(imageStreamHandle, &imageFrame));
	}
};

void main()
{
	try{
		Kinect kinect;
		kinect.initialize();
		kinect.run();
	}
	catch (std::exception& ex){
		std::cout << ex.what() << std::endl;
	}
}