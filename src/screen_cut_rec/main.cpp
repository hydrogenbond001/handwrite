#include <string>
#include <opencv2/opencv.hpp>
#include "OcrLite.h"
#include "OcrUtils.h"

std::string runOCR(const cv::Mat &img) {
    std::string modelsDir = "models";
    std::string modelDetPath = modelsDir + "/dbnet_op";
    std::string modelClsPath = modelsDir + "/angle_op";
    std::string modelRecPath = modelsDir + "/crnn_lite_op";
    std::string keysPath = modelsDir + "/keys.txt";

    int numThread = 1;
    int padding = 50;
    int maxSideLen = 1024;
    float boxScoreThresh = 0.6f;
    float boxThresh = 0.3f;
    float unClipRatio = 2.0f;
    bool doAngle = true;
    bool mostAngle = true;
    int flagGpu = -1;

    // 检查模型文件是否存在
    if (!isFileExists(modelDetPath + ".param") || !isFileExists(modelDetPath + ".bin") ||
        !isFileExists(modelClsPath + ".param") || !isFileExists(modelClsPath + ".bin") ||
        !isFileExists(modelRecPath + ".param") || !isFileExists(modelRecPath + ".bin") ||
        !isFileExists(keysPath)) {
        fprintf(stderr, "Some model files or keys file are missing!\n");
        return "";
    }

    // 初始化 OCR 模型
    OcrLite ocrLite;
    ocrLite.setNumThread(numThread);
    ocrLite.initLogger(
        true,  // isOutputConsole
        false, // isOutputPartImg
        true   // isOutputResultImg
    );

    bool initModelsRet = ocrLite.initModels(modelDetPath, modelClsPath, modelRecPath, keysPath);
    if (!initModelsRet) {
        fprintf(stderr, "OCR model initialization failed!\n");
        return "";
    }

    // OCR 识别
    OcrResult result = ocrLite.detect_img(img, padding, maxSideLen, boxScoreThresh, boxThresh, unClipRatio, doAngle, mostAngle);
    ocrLite.Logger("%s\n", result.strRes.c_str());

    return result.strRes;
}
