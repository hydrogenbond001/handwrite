#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp> 
#include <unistd.h>   // sleep()
#include <iostream>  
#include <net.h>
#include <fstream>
#include <numeric>
#include <stdio.h>
#include <time.h>

using namespace cv; 
template<class ForwardIterator>
inline static size_t argmax(ForwardIterator first, ForwardIterator last) {
    return std::distance(first, std::max_element(first, last));
}
clock_t start, end;
double cpu_time_used;

int main()
{

    const float meanValues[3] = {127.5, 127.5, 127.5};
    const float normValues[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};
    const int dstHeight = 32;
    ncnn::Net net;
    std::vector<std::string> keys;


    start = clock();// 记录开始时间
    Mat src = imread("./models/3.jpg");//default : BGR
   // cv::Mat src;
   // cvtColor(bgrSrc, src, cv::COLOR_BGR2RGB);// convert to RGB 



    // Step 2: 加载 .param 和 .bin 文件
    net.load_param("./models/crnn_lite_op.param");
    net.load_model("./models/crnn_lite_op.bin");

    std::ifstream in("./models/keys.txt");
    std::string line;
    if (in) {
        while (getline(in, line)) {// line中不包括每行的换行符
            keys.push_back(line);
        }
    } else {
        printf("The keys.txt file was not found\n");
        return false;
    }
    if (keys.size() != 5531) {
        fprintf(stderr, "missing keys\n");
        return false;
    }
    printf("total keys size(%lu)\n", keys.size());





    float scale = (float) dstHeight / (float) src.rows;
    int dstWidth = int((float) src.cols * scale);

    cv::Mat srcResize;
    resize(src, srcResize, cv::Size(dstWidth, dstHeight));

    ncnn::Mat input = ncnn::Mat::from_pixels(
            srcResize.data, ncnn::Mat::PIXEL_RGB,
            srcResize.cols, srcResize.rows);

    input.substract_mean_normalize(meanValues, normValues);

    ncnn::Extractor extractor = net.create_extractor();
    //net.num_threads  = 6;
    extractor.input("input", input);

    ncnn::Mat out;
    extractor.extract("out", out);

    float *floatArray = (float *) out.data;
    std::vector<float> outputData(floatArray, floatArray + out.h * out.w);



    int keySize = keys.size();
    std::string strRes;
    std::vector<float> scores;
    int lastIndex = 0;
    int maxIndex;
    float maxValue;

    for (int i = 0; i < out.h; i++) {
        maxIndex = 0;
        maxValue = -1000.f;
        //do softmax
        std::vector<float> exps(out.w);
        for (int j = 0; j < out.w; j++) {
            float expSingle = exp(outputData[i * out.w + j]);
            exps.at(j) = expSingle;
        }
        float partition = accumulate(exps.begin(), exps.end(), 0.0);//row sum
        maxIndex = int(argmax(exps.begin(), exps.end()));
        maxValue = float(*std::max_element(exps.begin(), exps.end())) / partition;
        if (maxIndex > 0 && maxIndex < keySize && (!(i > 0 && maxIndex == lastIndex))) {
            scores.emplace_back(maxValue);
            strRes.append(keys[maxIndex - 1]);
            std::cout <<strRes<< std::endl;
        }
        lastIndex = maxIndex;
    } 
  //  cv::imwrite("out.jpg", bgr);
    end = clock();    // 记录结束时间

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC * 1000;  // 转换为毫秒
    printf("Function execution time: %.3f ms\n", cpu_time_used);
    return 0;
}