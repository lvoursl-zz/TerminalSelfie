#include <opencv2/opencv.hpp>
#include <fstream>


const int STEP = 6; // если 1, то лучше убрать фильтр, чтобы изображение было более четкое
cv::Mat dst;
cv::Vec3b bgrPixel;
cv::Vec3b sumOfbgrPixel;

void findAveragePixelColor(int x, int y) {

    sumOfbgrPixel.val[0] = 0;
    sumOfbgrPixel.val[1] = 0;
    sumOfbgrPixel.val[2] = 0;    
 
    for (int f = x; f < x + STEP; f++) {
        for (int p = y; p < y + STEP; p++) {
            // среднее значение цветов в радиусе STEP пикселей
            bgrPixel = dst.at<cv::Vec3b>(f, p);             
            
            sumOfbgrPixel.val[0] += bgrPixel.val[0];
            sumOfbgrPixel.val[1] += bgrPixel.val[1];
            sumOfbgrPixel.val[2] += bgrPixel.val[2];
        }
    }
}

int main(int argc, char* argv[]) {

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cout << "!!! Failed to open web-camera" << std::endl;
        return -1;
    }


    cv::Mat menu = cv::imread("menu.jpg");
    cv::imshow("Terminal selfie", menu);

    cv::Mat frame;
    std::string str = "";
    int counter = 0;
    int key = 0;
 
    for(;;) {
        counter++;
        if (!cap.read(frame))             
            break;

        // rotate 
        cv::Point2f src_center(frame.cols/2.0F, frame.rows/2.0F);
        cv::Mat rot_mat = getRotationMatrix2D(src_center, 180, 1.0);
        cv::warpAffine(frame, dst, rot_mat, frame.size());
        
        cv::Mat dst2 = dst;

        // filter; dst - final image
        cv::Laplacian(dst2, dst, CV_8UC1);

        if (counter % 2 == 0) system("clear");

        if (counter % 2 == 0) {
            for (int i = 0; i < dst.rows; i += STEP) {
                for(int j = 0; j < dst.cols; j += STEP) {                               

                    findAveragePixelColor(i, j);

                    str += "\x1b[38;2;" + std::to_string((int)sumOfbgrPixel[2]) 
                                    + ";" + std::to_string((int)sumOfbgrPixel[1]) 
                                    + ";" + std::to_string((int)sumOfbgrPixel[0]) 
                                    + "m" + "0" + "\x1b[0m";

                             

                     // white moooode
                     /*str += "\x1b[38;2;" + std::to_string(255 - (int)sumOfbgrPixel[2]) 
                                    + ";" + std::to_string(255 - (int)sumOfbgrPixel[1]) 
                                    + ";" + std::to_string(255 - (int)sumOfbgrPixel[0]) 
                                    + "m" + "0" + "\x1b[0m";     */                                             
                }

                str += "\n";
            } 

        }

        std::cout << str; 
        //str - image for bash-script

        if (counter == 10000) counter = 0;

        key = cv::waitKey(10);

        if (key == 101) {
            // ESC (press E)
            system("clear");
            break;
        } else if (key == 115) {
            // creating image (S)
            cv::Mat image = frame;
            cv::Mat final_image;

            for (int i = 0; i < dst.rows; i += STEP) {
                for(int j = 0; j < dst.cols; j += STEP) {                               

                    findAveragePixelColor(i, j);
                    rectangle( image,
                               cv::Point( i, j ),
                               cv::Point( i + STEP, j + STEP),
                               cv::Scalar( (int)sumOfbgrPixel[0], (int)sumOfbgrPixel[1], (int)sumOfbgrPixel[2] ),
                               -1
                                );
                }
            }

            cv::Point2f src_center(frame.cols/2.0F, frame.rows/2.0F);
            cv::Mat rot_mat = getRotationMatrix2D(src_center, 270, 1.0);
            cv::warpAffine(image, final_image, rot_mat, frame.size());

            final_image = final_image(cv::Rect(0,0,640,400));

            cv::imwrite( "selfie.jpg", final_image );
            system("clear");
            std::cout << "Selfie created!\nOpen selfie.jpg \n";
            break;
        } else if (key == 98) {
            // selfie to bash-script (B)
            std::ofstream output;
            output.open("selfie.sh", std::ios::out);
            output << "#!/bin/bash\nprintf " << "\"" << str << "\"";
            output.close();

            system("clear");
            std::cout << "Bash selfie created!\nUse ./selfie.sh \n";
            break;
        }

        str = "";
    }

    return 0;
}