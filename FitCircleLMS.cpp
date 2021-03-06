#include <stdio.h>
#include <vector>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;


const double PI = 3.141592653;
const size_t N = 360;

void circlePoints(double x0, double y0, double radius, Point2d* points)
{
    double range = PI / 16;
    double rad = range / N;
    for (size_t i = 0; i < N; i++)
    {
        points[i].x = x0 + cos(rad*i)*radius;
        points[i].y = y0 + sin(rad*i)*radius;
    }
}

/* A circle solver by transforming the standard circle equation to a linear system and then applying Least Square Method

    The standard equation of a circle is shown below
    (xi-x0)^2+(yi-y0)^2 = r^2           (i=1,2...,n)
    where, x0 and y0 are center coordinate (x0, y0) of the unknown circle, and r is the radius of the circle
                points (xi,yi) are used to fit the unknown circle

    The equation can be rewritten as below
    2*xi*x0 + 2*yi*y0 + (r^2-x0^2-y0^2) = xi^2+yi^2         (i=1,2...,n)

    Suppose C = (r^2-x0^2-y0^2), we can get
    2*xi*x0 + 2*yi*y0 + C = xi^2+yi^2         (i=1,2...,n)

    Therefore, we can rewrite the above equation with matrix
    AX = Y,
    where A = [2x1    2y1    1;
                       2x2    2y2    1;
                       ...       ...       ...;
                       2xn    2yn    1]

               X = [x0;y0;C]

               Y = [x1^2+y1^2;
                       x2^2+y2^2;
                               ...
                       xn^2+yn^2;]

    Finally, X = (A.transpose()*A).inverse()*A.transpose()*Y,
                                 [A11,A12, A13;
    A.transpose()*A = A21, A22, A23;
                                 A31, A32, A33]
    with, A11 = 4*sum(xi^2);
            A12 = 4*sum(xi*yi);
            A13 = 2*sum(xi);
            A21 = A12;
            A22 = 4*sum(yi^2);
            A23 = 2*sum(yi);
            A31 = A13
            A32 = A23;
            A33 = n
*/
void FitCircleLMS(Point2d* points, size_t N, Point2d* centerOut, double* radiusOut)
{
    cv::Mat A(N, 3, CV_64FC1, cv::Scalar::all(0));
    cv::Mat Y(N, 1, CV_64FC1, cv::Scalar::all(0));

    for (size_t i = 0; i < N; i++)
    {
        A.at<double>(i, 0) = 2 * points[i].x;
        A.at<double>(i, 1) = 2 *points[i].y;
        A.at<double>(i, 2) = 1;
        Y.at<double>(i, 0) = pow(points[i].x, 2) + pow(points[i].y, 2);
    }

    // X = [x0;y0;pow(r,2)-pow(x0,2), pow(y0,2)]
    cv::Mat X = (A.t()*A).inv()*A.t()*Y; 

    // C =r^2 - x0^2 - y0^2
    *radiusOut = X.at<double>(2, 0) + pow(X.at<double>(0, 0), 2) + pow(X.at<double>(1, 0), 2);
    if (*radiusOut < 0)
    {
        centerOut->x = 0;
        centerOut->y = 0;
        *radiusOut = 0;
    }
    else
    {
        centerOut->x = X.at<double>(0, 0);
        centerOut->y = X.at<double>(1, 0);
        *radiusOut = sqrt(*radiusOut);
    }
}

void readTxt(const char* file, std::vector<Point2d>& v)
{
    FILE* fp = NULL;
    fopen_s(&fp, file, "r");
    Point2d point;

    while (fscanf_s(fp, "%lf\t%lf\n", &point.x, &point.y) != EOF)
        v.push_back(point);
}

int main()
{
    Point2d* points = new Point2d[N];
    double x0 = 500, y0 = 500, radius = 300;

    circlePoints(x0, y0, radius, points);

    Point2d centerOut{ 0 };
    double radiusOut{0};
    printf("%f\t%f\t%f\n", centerOut.x, centerOut.y, radiusOut);
    FitCircleLMS(points, N, &centerOut, &radiusOut);
    printf("%f\t%f\t%f\n", centerOut.x, centerOut.y, radiusOut);
}