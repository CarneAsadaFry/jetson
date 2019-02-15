//Imports from opencv & others
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudacodec.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaobjdetect.hpp>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/cudastereo.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <math.h>
#include <unistd.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

using namespace std;
using namespace cv;
using namespace cv::cuda;

class GPUTrack {
	public:
		GPUTrack();
		void Process(GpuMat source);
	private:
		static GpuMat labThreshold(GpuMat source);
		static GpuMat bgrThreshold(GpuMat source);
		static GpuMat merge(GpuMat source1, GpuMat source2);
		static vector<vector<Point>> findContours(GpuMat source);
		static vector<vector<Point>> filterContours(vector<vector<Point>> source);
		static GpuMat display(GpuMat source);
		static GpuMat GPUInRange(GpuMat source, int lower[], int upper[]);
		static bool doProcess(vector<vector<Point>> contour);
};
