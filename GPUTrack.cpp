#include "GPUTrack.h"
#define twohundredsixtytwo 44 
#define PORT 5805 
#define MAXLINE 1024
//Let me do things

using namespace std;
using namespace cv;
using namespace cv::cuda;

int labLower[3] = {121, 74, 90};
int labUpper[3] = {255, 176, 165};
int bgrLower[3] = {151, 214, 0};
int bgrUpper[3] = {183, 255, 185};
int area = 500;
int sockfd;
struct sockaddr_in servaddr;



//Event listener inhereted from opencv. Used for sliding bars on lab/bgr params.
void on_trackbar(int value, void*) {}

GPUTrack::GPUTrack() {
	namedWindow("Tune Params", 1);
	createTrackbar("Hue Min", "Tune Params", &labLower[0], 255, on_trackbar);
        createTrackbar("Hue Max", "Tune Params", &labUpper[0], 255, on_trackbar);
        createTrackbar("Sat Min", "Tune Params", &labLower[1], 255, on_trackbar);
        createTrackbar("Sat Max", "Tune Params", &labUpper[1], 255, on_trackbar);
        createTrackbar("Lum Min", "Tune Params", &labLower[2], 255, on_trackbar);
        createTrackbar("Lum Max", "Tune Params", &labUpper[2], 255, on_trackbar);
        createTrackbar("Blu Min", "Tune Params", &bgrLower[0], 255, on_trackbar);
        createTrackbar("Blu Max", "Tune Params", &bgrUpper[0], 255, on_trackbar);
        createTrackbar("Grn Min", "Tune Params", &bgrLower[1], 255, on_trackbar);
        createTrackbar("Grn Max", "Tune Params", &bgrUpper[1], 255, on_trackbar);
        createTrackbar("Red Min", "Tune Params", &bgrLower[2], 255, on_trackbar);
        createTrackbar("Red Max", "Tune Params", &bgrUpper[2], 255, on_trackbar);
	createTrackbar("ConArea", "Tune Params", &area, 5000, on_trackbar);
        waitKey(0);

}

int main(int cargs, char* vargs[]) {
	static GpuMat source(1080, 1920, CV_8UC3);
	Mat image;
	VideoCapture cam("/dev/video1");
//	cam.set(8, 3); //Sets mode to YUYV 
//	cam.set(3, 1920);
//	cam.set(4, 1080);
	GPUTrack* pipe = new GPUTrack();

	//Garbage
	int frame[32];
	31[frame] = 52;
	cout << frame[31] << endl;	

	//Setting up socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket creation failed");
		return false;
	}
	
	memset(&servaddr, 0, sizeof(servaddr));
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = INADDR_ANY;
	inet_aton("10.16.61.2", &servaddr.sin_addr);

	//Central loop
	for(;;) {
		cam >> image;
		source.upload(image);
		pipe->Process(source);
	}

	return 0;
}

void GPUTrack::Process(GpuMat source) {
	GpuMat labSource = labThreshold(source);
	GpuMat bgrSource = bgrThreshold(source);
	source = merge(labSource, bgrSource);
	vector<vector<Point>> contours = findContours(source);
	contours = filterContours(contours);
	doProcess(contours);
}

GpuMat GPUTrack::labThreshold(GpuMat source) {
	GpuMat labmat;
	cuda::cvtColor(source, labmat, twohundredsixtytwo);
	labmat = GPUInRange(labmat, labLower, labUpper);
	return labmat;
}

GpuMat GPUTrack::bgrThreshold(GpuMat source) {
	return GPUInRange(source, bgrLower, bgrUpper);
}

GpuMat GPUTrack::merge(GpuMat source1, GpuMat source2) {
	GpuMat result;
	cuda::bitwise_and(source1, source2, result); 
	return result;  
}

vector<vector<Point>> GPUTrack::findContours(GpuMat source) {
	Mat out;
	vector<Vec4i> hierarchy;
	int mode = RETR_LIST;
	int method = CHAIN_APPROX_SIMPLE;
	vector<vector<Point>> contours;

	source.download(out);
	
	Rect roi = Rect(0, 200, 640, 280);
//	out = out(roi);

	imshow("Window", out);
	waitKey(27);

//	cv::findContours(out, contours, hierarchy, mode, method); 
	return contours;
}

vector<vector<Point>> GPUTrack::filterContours(vector<vector<Point>> source) {
	vector<vector<Point>> filtered;
	for(vector<Point> contour : source) {
		if (contourArea(contour) > area)
			filtered.push_back(contour);
	}
	return filtered;
}

bool GPUTrack::doProcess(vector<vector<Point>> contour) {
	double throttle = 0.0;
	double turn = 0.0;
	if(contour.size() == 2) {
		Point topA(100000, 100000);
		Point topB(100000, 100000);
		for(Point p : contour[0]) {
			if(p.y < topA.y) {
				topA.x = p.x;
				topA.y = p.y;
			}
		}
		for(Point p : contour[1]) {
			if(p.y < topB.y) {
				topB.x = p.x;
				topB.y = p.y;
			}
		}
		if(abs(abs(topB.x - topA.x) - 500) > 30) {
			throttle = .15 + .001 * (500 - abs(topB.x - topA.x));
			throttle = throttle > 1 ? 1 : (throttle < -1 ? -1 : throttle);
		}
		cout << abs(topB.x - topA.x) << " " << throttle << endl;
	}
	char buf[MAXLINE];
	char str[25] = "";
	sprintf(str, "%.6lf", throttle);
	
	sendto(sockfd, str, strlen(str), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
	return true;
}

GpuMat GPUTrack::GPUInRange(GpuMat source, int lower[], int upper[])  {
	GpuMat sourceLayers[3];
	GpuMat temp;
	GpuMat thresh;
//Split 3 channels
	cuda::split(source, sourceLayers);

//Threshold channels
	cuda::threshold(sourceLayers[0], thresh, lower[0] - 1, 255, THRESH_BINARY);
	cuda::threshold(sourceLayers[0], temp, upper[0], 255, THRESH_BINARY_INV);
	cuda::bitwise_and(thresh, temp, thresh);
	cuda::threshold(sourceLayers[1], temp, lower[1] - 1, 255, THRESH_BINARY);
	cuda::bitwise_and(thresh, temp, thresh);
	cuda::threshold(sourceLayers[1], temp, upper[1], 255, THRESH_BINARY_INV);
	cuda::bitwise_and(thresh, temp, thresh);
	cuda::threshold(sourceLayers[2], temp, lower[2] - 1, 255, THRESH_BINARY);
	cuda::bitwise_and(thresh, temp, thresh);
	cuda::threshold(sourceLayers[2], temp, upper[2], 255, THRESH_BINARY_INV);
	cuda::bitwise_and(thresh, temp, thresh);

	return thresh;
}
