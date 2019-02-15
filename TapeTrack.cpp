#include "TapeTrack.h"


using namespace grip;

int minHue = 0;
int maxHue = 140;
int minSat = 0;
int maxSat = 20;
int minLum = 230;
int maxLum = 245;
void on_trackbar(int value, void*){}

int main(int cargs, char* vargs[]) {
	cv::Mat frame;
        cv::VideoCapture cam("/dev/video1");
        TapeTrack gp;

	//Trackbar
        cv::namedWindow("Hue Tune", 1);
	cv::createTrackbar("Hue Min", "Hue Tune", &minHue, 255, on_trackbar);
	cv::createTrackbar("Hue Max", "Hue Tune", &maxHue, 255, on_trackbar); 
	cv::createTrackbar("Sat Min", "Hue Tune", &minSat, 255, on_trackbar); 
	cv::createTrackbar("Sat Max", "Hue Tune", &maxSat, 255, on_trackbar); 
	cv::createTrackbar("Lum Min", "Hue Tune", &minLum, 255, on_trackbar); 
	cv::createTrackbar("Lum Max", "Hue Tune", &maxLum, 255, on_trackbar); 
	cv::waitKey(0);

	for(;;) {
                cam >> frame;
                gp.Process(frame);
//              cv::imshow("result", frame);
//              if(cv::waitKey(30) >= 0) break;
        }
        return 0;
}


TapeTrack::TapeTrack() {
}
/**
* Runs an iteration of the pipeline and updates outputs.
*/
void TapeTrack::Process(cv::Mat& source0){
	//Step HSL_Threshold0:
	//input
	cv::Mat hslThresholdInput = source0;

	//Changed the hardcoded max from 140 to dynamic trackbar
	double hslThresholdHue[] = {minHue, maxHue};//{0.0, 0.0};
	double hslThresholdSaturation[] = {minSat, maxSat};//{76.4388, 89.64162992};
	double hslThresholdLuminance[] = {minLum, maxLum};
	hslThreshold(hslThresholdInput, hslThresholdHue, hslThresholdSaturation, hslThresholdLuminance, this->hslThresholdOutput);
	//Step Find_Contours0:
	//input
	cv::Mat findContoursInput = hslThresholdOutput;
	bool findContoursExternalOnly = false;  // default Boolean
	findContours(findContoursInput, findContoursExternalOnly, this->findContoursOutput);
	//Step Filter_Contours0:
	//input
	std::vector<std::vector<cv::Point> > filterContoursContours = findContoursOutput;
	double filterContoursMinArea = 700.0;  // default Double
	double filterContoursMinPerimeter = 0.0;  // default Double
	double filterContoursMinWidth = 0.0;  // default Double
	double filterContoursMaxWidth = 100000.0;  // default Double
	double filterContoursMinHeight = 0.0;  // default Double
	double filterContoursMaxHeight = 100000.0;  // default Double
	double filterContoursSolidity[] = {0.0, 100.0};
	double filterContoursMaxVertices = 2.147483647E9;  // default Double
	double filterContoursMinVertices = 0.0;  // default Double
	double filterContoursMinRatio = 0.0;  // default Double
	double filterContoursMaxRatio = 1000000.0;  // default Double
	filterContours(filterContoursContours, filterContoursMinArea, filterContoursMinPerimeter, filterContoursMinWidth, filterContoursMaxWidth, filterContoursMinHeight, filterContoursMaxHeight, filterContoursSolidity, filterContoursMaxVertices, filterContoursMinVertices, filterContoursMinRatio, filterContoursMaxRatio, this->filterContoursOutput);
	std::vector<std::vector<cv::Point> >* vert = GetFilterContoursOutput();
        std::cout << "Current contours (" << vert->size() << "):" << std::endl;
	int x [vert->size()];
	int y [vert->size()];
	int contCount = 0;
        for(std::vector<cv::Point> cont: *vert) {
                int totX = 0;
                int totY = 0;
                int ptCount = 0;
                for(cv::Point pt: cont) {
                        totX += pt.x;
                        totY += pt.y;
			ptCount++;
                }
                int avgX = totX / ptCount;
                int avgY = totY / ptCount;
		x[contCount] = avgX;
		y[contCount++] = avgY;
        }
	if(vert->size() == 2) {
		std::cout << "Diff: " << std::abs(x[0] - x[1]) << std::endl;
	}
}

/**
 * This method is a generated getter for the output of a HSL_Threshold.
 * @return Mat output from HSL_Threshold.
 */
cv::Mat* TapeTrack::GetHslThresholdOutput(){
	return &(this->hslThresholdOutput);
}
/**
 * This method is a generated getter for the output of a Find_Contours.
 * @return ContoursReport output from Find_Contours.
 */
std::vector<std::vector<cv::Point> >* TapeTrack::GetFindContoursOutput(){
	return &(this->findContoursOutput);
}
/**
 * This method is a generated getter for the output of a Filter_Contours.
 * @return ContoursReport output from Filter_Contours.
 */
std::vector<std::vector<cv::Point> >* TapeTrack::GetFilterContoursOutput(){
	return &(this->filterContoursOutput);
}
	/**
	 * Segment an image based on hue, saturation, and luminance ranges.
	 *
	 * @param input The image on which to perform the HSL threshold.
	 * @param hue The min and max hue.
	 * @param sat The min and max saturation.
	 * @param lum The min and max luminance.
	 * @param output The image in which to store the output.
	 */
	//void hslThreshold(Mat *input, double hue[], double sat[], double lum[], Mat *out) {
	void TapeTrack::hslThreshold(cv::Mat &input, double hue[], double sat[], double lum[], cv::Mat &out) {
		cv::cvtColor(input, out, cv::COLOR_BGR2HLS);
		cv::inRange(out, cv::Scalar(hue[0], lum[0], sat[0]), cv::Scalar(hue[1], lum[1], sat[1]), out);
		cv::imshow("hsl", out);
		if(cv::waitKey(30) >= 0) return;
	}

	/**
	 * Finds contours in an image.
	 *
	 * @param input The image to find contours in.
	 * @param externalOnly if only external contours are to be found.
	 * @param contours vector of contours to put contours in.
	 */
	void TapeTrack::findContours(cv::Mat &input, bool externalOnly, std::vector<std::vector<cv::Point> > &contours) {
		std::vector<cv::Vec4i> hierarchy;
		contours.clear();
		int mode = externalOnly ? cv::RETR_EXTERNAL : cv::RETR_LIST;
		int method = cv::CHAIN_APPROX_SIMPLE;
		cv::findContours(input, contours, hierarchy, mode, method);
	}


	/**
	 * Filters through contours.
	 * @param inputContours is the input vector of contours.
	 * @param minArea is the minimum area of a contour that will be kept.
	 * @param minPerimeter is the minimum perimeter of a contour that will be kept.
	 * @param minWidth minimum width of a contour.
	 * @param maxWidth maximum width.
	 * @param minHeight minimum height.
	 * @param maxHeight  maximimum height.
	 * @param solidity the minimum and maximum solidity of a contour.
	 * @param minVertexCount minimum vertex Count of the contours.
	 * @param maxVertexCount maximum vertex Count.
	 * @param minRatio minimum ratio of width to height.
	 * @param maxRatio maximum ratio of width to height.
	 * @param output vector of filtered contours.
	 */
	void TapeTrack::filterContours(std::vector<std::vector<cv::Point> > &inputContours, double minArea, double minPerimeter, double minWidth, double maxWidth, double minHeight, double maxHeight, double solidity[], double maxVertexCount, double minVertexCount, double minRatio, double maxRatio, std::vector<std::vector<cv::Point> > &output) {
		std::vector<cv::Point> hull;
		output.clear();
		for (std::vector<cv::Point> contour: inputContours) {
			cv::Rect bb = boundingRect(contour);
			if (bb.width < minWidth || bb.width > maxWidth) continue;
			if (bb.height < minHeight || bb.height > maxHeight) continue;
			double area = cv::contourArea(contour);
			if (area < minArea) continue;
			if (arcLength(contour, true) < minPerimeter) continue;
			cv::convexHull(cv::Mat(contour, true), hull);
			double solid = 100 * area / cv::contourArea(hull);
			if (solid < solidity[0] || solid > solidity[1]) continue;
			if (contour.size() < minVertexCount || contour.size() > maxVertexCount)	continue;
			double ratio = (double) bb.width / (double) bb.height;
			if (ratio < minRatio || ratio > maxRatio) continue;
			output.push_back(contour);
		}
	}



 // end grip namespace

