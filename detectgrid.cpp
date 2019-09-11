#include "detectgrid.h"

using namespace cv;
using namespace std;

DetectGrid::DetectGrid()
{

}

/**
* Detect numeric digits in a sudoku grid in img Mat and return Rect instances where they are found
*/
vector<Rect> DetectGrid::findDigits(const Mat& img) {
    vector<Rect> digits;
    Rect imgRect = Rect(0, 0, img.cols, img.rows);

    // Find all contours
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //findContours( img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
    findContours( img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0) );
    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect( contours.size() );

    for( size_t i = 0; i < contours.size(); i++ )
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );

        if( hierarchy[i][3] < 0 && boundRect[i].height >= MIN_DIGIT_PIXELS) { // find "root" contours
            double aspectRatio = boundRect[i].height / double(boundRect[i].width);
            // check reasonable aspect ratio for digits
            if ((aspectRatio >= 1 && aspectRatio < 3.2)) {
                int widthToAdd = boundRect[i].height - boundRect[i].width + (2 * DIGIT_PADDING);
                int pointOffset = int(floor(double(widthToAdd / 2)));
                boundRect[i] = boundRect[i] - Point(pointOffset, DIGIT_PADDING);
                boundRect[i] = boundRect[i] + Size(widthToAdd, 2 * DIGIT_PADDING);
                boundRect[i] &= imgRect;

                // check white/black pixel ratio to avoid accidental noise getting picked up
                double wbRatio = countNonZero(img(boundRect[i])) / double(boundRect[i].width * boundRect[i].height);
                if (wbRatio > 0.1 && wbRatio < 0.4) {
                    digits.push_back(boundRect[i]); // AND boundRect and imgRect to ensure in boundary
                }
            }
        }
    }

    return digits;
}

/**
* Attempt to extract and warp sudoku grid
*/
void DetectGrid::extractGrid(const Mat& img, Mat& dst, vector<float>& gridPoints, float& scale) {
    Mat src_gray;
    blur( img, src_gray, Size(3,3) );
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    int largest_area=0;
    int largest_contour_index=0;
    vector<Point> largestContour;
    Rect bounding_rect;

    Mat denoised;
    fastNlMeansDenoising(src_gray, denoised, 10);

    adaptiveThreshold(~denoised, src_gray, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, -2);


    Canny( src_gray, canny_output, CANNY_THRESHOLD, CANNY_THRESHOLD * 2, 3 );
    findContours( canny_output, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0) );
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );

    for( size_t i = 0; i< contours.size(); i++ )
    {
        double a=contourArea( contours[i],false);  //  Find the area of contour
        if(a>largest_area){
            largest_area=static_cast<int>(a);
            largest_contour_index=static_cast<int>(i);                //Store the index of largest contour
            largestContour = contours[i];
            bounding_rect=boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
        }
    }

    double area = img.cols * img.rows;
    if (largest_area < area * MIN_GRID_PCT) {
        cout << "largest contour area is only " << (largest_area / area) * 100 << "% of source; aborting grid extraction" << endl;
        img.copyTo(dst);
        return;
    }

    // findCorners
    if (largest_contour_index < static_cast<int>(contours.size())) {
        Point2f corners[4];
        Point2f flatCorners[4];
        Size sz = findCorners(largestContour, corners);


        // draw contour quadrangle
        for( int j = 0; j < 4; j++ ) {
            gridPoints.push_back(corners[j].x * scale);
            gridPoints.push_back(corners[j].y * scale);
        }
        flatCorners[0] = Point2f(0, 0);
        flatCorners[1] = Point2f(sz.width, 0);
        flatCorners[2] = Point2f(sz.width, sz.height);
        flatCorners[3] = Point2f(0, sz.height);
        Mat lambda = getPerspectiveTransform(corners, flatCorners);

        Mat output;
        warpPerspective(denoised, output, lambda, sz);

        GaussianBlur(output, dst, Size(0, 0), 3);
        addWeighted(output, 1.5, dst, -0.5, 0, dst);
    } else {
        throw runtime_error("No grid contour found");
    }
}

/**
* Find corners within the largest contour for use in performing warp transform
*/
Size DetectGrid::findCorners(const vector<Point> largestContour, Point2f corners[]) {
    double dist;
    double maxDist[4] = {0, 0, 0, 0};

    Moments M = moments(largestContour, true);
    float cX = float(M.m10 / M.m00);
    float cY = float(M.m01 / M.m00);

    for (int i = 0; i < 4; i++) {
        maxDist[i] = 0.0;
        corners[i] = Point2f(cX, cY);
    }

    Point2f center(cX, cY);

    // find the most distant point in the contour within each quadrant
    for (size_t i = 0; i < largestContour.size(); i++) {
        Point2f p(largestContour[i].x, largestContour[i].y);
        dist = sqrt(pow(p.x - center.x, 2) + pow(p.y - center.y, 2));
        // cout << "(" << p.x << "," << p.y << ") is " << dist << " from (" << cX << "," << cY << ")" << endl;
        if (p.x < cX && p.y < cY && maxDist[0] < dist) {
            // top left
            maxDist[0] = dist;
            corners[0] = p;
        } else if (p.x > cX && p.y < cY && maxDist[1] < dist) {
            // top right
            maxDist[1] = dist;
            corners[1] = p;
        } else if (p.x > cX && p.y > cY && maxDist[2] < dist) {
            // bottom right
            maxDist[2] = dist;
            corners[2] = p;
        } else if (p.x < cX && p.y > cY && maxDist[3] < dist) {
            // bottom left
            maxDist[3] = dist;
            corners[3] = p;
        }
    }

    // determine the dimensions that we should warp this contour to
    double widthTop = sqrt(pow(corners[0].x - corners[1].x, 2) + pow(corners[0].y - corners[1].y, 2));
    double widthBottom = sqrt(pow(corners[2].x - corners[3].x, 2) + pow(corners[2].y - corners[3].y, 2));

    double heightLeft = sqrt(pow(corners[0].x - corners[3].x, 2) + pow(corners[0].y - corners[3].y, 2));
    double heightRight = sqrt(pow(corners[1].x - corners[2].x, 2) + pow(corners[1].y - corners[2].y, 2));


    return Size(static_cast<int>(max(widthTop, widthBottom)), static_cast<int>(max(heightLeft, heightRight)));
}

/**
* Detect horizontal/vertical lines of a sudoku grid in img Mat and copy expanded lines to dst Mat
*/
void extractLines(const Mat& img, Mat& dst, bool horizontal) {
    // Clone the source image
    Mat clone = img.clone();

    LineTestFn lineTest;
    ExpandRectFn expandRect;
    Size size;

    // Setup correct structure size, line test and rect expansion for horizontal vs. vertical
    if (horizontal) {
        size = Size(img.cols / 9, 1);
        lineTest = [](Rect& rect, Mat& mat) { return rect.height / double(mat.rows) < 0.05 && rect.width / double(mat.cols) > 0.111; };
        expandRect = [](Rect& rect, Mat& mat) {
            Rect expanded = rect;
            if (expanded.y > 1) { expanded.y -= 2; }

            if (expanded.y + expanded.height < mat.rows) {
                expanded.height += min(4, mat.rows - expanded.y - expanded.height);
            }
            expanded.x = 0;
            expanded.width = mat.cols;
            return expanded;
        };
    } else {
        size = Size(1, img.rows / 9);
        lineTest = [](Rect& rect, Mat& mat) { return rect.width / double(mat.cols) < 0.05 && rect.height / double(mat.rows) > 0.111; };
        expandRect = [](Rect& rect, Mat& mat) {
            Rect expanded = rect;
            if (expanded.x > 1) { expanded.x -= 2; }

            if (expanded.x + expanded.width < mat.cols) {
                expanded.width += min(4, mat.cols - expanded.x - expanded.width);
            }
            expanded.y = 0;
            expanded.height = mat.rows;
            return expanded;
        };
    }
    // Create structure element for extracting lines through morphology operations
    Mat structure = getStructuringElement(MORPH_RECT, size);

    // Apply morphology operations
    erode(clone, clone, structure, Point(-1, -1));
    dilate(clone, clone, structure, Point(-1, -1));

    // Find all contours
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( clone, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect( contours.size() );

    // Mark contours which pass line test in the destination image
    for( size_t i = 0; i < contours.size(); i++ )
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
        if (lineTest(boundRect[i], clone)) {
            Rect expanded = expandRect(boundRect[i], clone);
            dst(expanded) |= 255; // set the expanded rect to white
        }
    }
}

/**
* Detect Sudoku board and digits in the "raw" Mat
*/
vector<Rect> DetectGrid::FindDigitRects(const Mat& raw, Mat& cleaned, vector<float>& gridPoints, float &scale) {
    // Check if image is loaded fine
    if(!raw.data)
        cerr << "Problem loading image!!!" << endl;

    Mat src;
    raw.copyTo(src);

    // Transform source image to gray if it is not
    Mat gray;
    if (src.channels() == 3)
    {
        cvtColor(src, gray, COLOR_BGR2GRAY);
    }
    else
    {
        gray = src;
    }

    // make sure image is a reasonable size
    if(gray.rows > MAX_PUZZLE_SIZE || gray.cols > MAX_PUZZLE_SIZE) {
        scale = max(gray.rows, gray.cols) / float(MAX_PUZZLE_SIZE);
        resize(gray, gray, Size(static_cast<int>(gray.cols / scale), static_cast<int>(gray.rows / scale)), 0, 0, INTER_AREA);
    } else if (gray.rows < MIN_PUZZLE_SIZE || gray.cols < MIN_PUZZLE_SIZE) {
        scale = min(gray.rows, gray.cols) / float(MAX_PUZZLE_SIZE);
        resize(gray, gray, Size(static_cast<int>(gray.cols / scale), static_cast<int>(gray.rows / scale)), 0, 0, INTER_CUBIC);
    }

    Mat grid = Mat::zeros( gray.size(), gray.type() );
    extractGrid(gray, grid, gridPoints, scale);

    // Apply adaptiveThreshold at the bitwise_not of gray, notice the ~ symbol
    Mat bw;
    adaptiveThreshold(~grid, bw, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, -2);

    Mat lines = Mat::zeros( bw.size(), bw.type() );
    extractLines(bw, lines, true);
    extractLines(bw, lines, false);

    imshow("lines", lines);

    // subtract grid lines from the black/white image
    // so they don't interfere with digit detection
    Mat clean = bw - lines;
    blur(clean, clean, Size(1, 1));

    // find digits
    vector<Rect> digits = findDigits(clean);

    clean.copyTo(cleaned);

    return digits;
}
