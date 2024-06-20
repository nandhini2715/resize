#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <fstream>



// Namespaces.
using namespace cv;
using namespace std;
using namespace dnn;


// Constants.
const int INPUT_WIDTH = 640;
const int INPUT_HEIGHT = 640;
const float SCORE_THRESHOLD = 0.6f;
const float NMS_THRESHOLD = 0.45f;
const float CONFIDENCE_THRESHOLD = 0.45f;

// Text parameters.
const float FONT_SCALE = 0.7f;
const int FONT_FACE = FONT_HERSHEY_SIMPLEX;
const int THICKNESS = 1;

// Colors.
Scalar BLACK = Scalar(0, 0, 0);
Scalar BLUE = Scalar(255, 178, 50);
Scalar YELLOW = Scalar(0, 255, 255);
Scalar RED = Scalar(0, 0, 255);

string home = getenv("HOME");
string DETECTIONS_OUTPUT_PATH = home + "/files/det.txt";
string TRACKINGS_OUTPUT_PATH = home + "/files/tracked.txt";
string VIDEO_OUTPUT_PATH = home + "/files/output.mp4";
string COCO_NAMES = home + "/files/coco.names";
//string NET_PATH = home + "/files/YOLOv5s.onnx";
string NET_PATH = home + "/files/yolov8s.onnx";
//string NET_PATH = home + "/files/yolov8x.onnx";

//string folderPath = "./imgs/%06d.jpg";
string folderPath="../data/vtest.avi";

int outputCodec = VideoWriter::fourcc('M', 'J', 'P', 'G');
double outputFps = 10;
Size outputSize(768, 576);

vector<Mat> inference(Mat&, Net&);
Mat formatYolov5(const Mat&);
Mat postProcessImage(Mat&, vector<Mat>&, const vector<string>&, vector<Detection>&);
void drawLabel(Mat&, string, int, int);
void writeDetectionsToFile(const vector<Detection>, const string&, const int);
void writeTracksToFile(const Mat&, const string&, const int);
void writeTracksToFile(const vector<Track> objects, const string &outputPath,
    const int frameNumber);
Scalar getColor(int);
Mat detectionToMat(vector<Detection>);

int main()
{
    // Load class list.
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_WARNING);
    vector<string> classList;
    ifstream ifs(COCO_NAMES);
    string line;
    while (getline(ifs, line))
    {
        classList.push_back(line);
    }

    VideoCapture capture;
    VideoWriter writer(VIDEO_OUTPUT_PATH, outputCodec, outputFps, outputSize);


    capture.open(folderPath);

    if (!capture.isOpened())
    {
        cout << "failed to open the image sequence: " << folderPath << endl;
        return -1;
    }

    // Load image sequence.
    Mat frame;
    int frameNumber = 0;
    double fps = capture.get(CAP_PROP_FPS);
    //cout<<"my fps is"<<fps;
    cv::ByteTracker::Params params;
    params.frameRate = fps;
    params.frameBuffer = 30;
    Ptr<ByteTracker> tracker = ByteTracker::create(params);
    while (capture.read(frame))
    {
        if (frame.empty())
        {
            cout << "Failed to read the frame." << endl;
            break;
        }

        // Load model.
        Net net;
        net = readNet(NET_PATH);
        // net.setPreferableBackend(dnn::DNN_TARGET_CPU);
        // net.setPreferableTarget(dnn::DNN_TARGET_CUDA_FP16);
        vector<Mat> detections; // Process the image.
        vector<Detection> objects;
        detections = inference(frame, net);
        Mat img = postProcessImage(frame, detections, classList,
                               objects);


        bool useArray = false;
        if (useArray) //Update method with input array and output array
        {
            Mat objectsMat = detectionToMat(objects);
            Mat trackedObjects;
            bool ok = tracker->update(objectsMat, trackedObjects);
            if (ok)
            {
                for (int i = 0; i < trackedObjects.rows; i++)
                {
                    int id_ = static_cast<int>(trackedObjects.at<float>(i, 6));
                    Scalar color = getColor(id_);
                    cv::Rect2f tlwh_(
                        trackedObjects.at<float>(i, 0),
                        trackedObjects.at<float>(i, 1),
                        trackedObjects.at<float>(i, 2),
                        trackedObjects.at<float>(i, 3));

                    rectangle(img, tlwh_, color, 2);
                    int xPoint = static_cast<int>(tlwh_.x);
                    int yPoint = static_cast<int>(tlwh_.y);
                    putText(img, to_string(id_), Point(xPoint, yPoint - 5), FONT_FACE, FONT_SCALE, RED);
                }
                writeTracksToFile(trackedObjects, TRACKINGS_OUTPUT_PATH, frameNumber);
            }
        }
        else if (!useArray)
        {
            vector<Track> trackedObjects;
            tracker->update(objects, trackedObjects); //Update method with vector of detection as input and vector of tracks as output
            for (auto& track : trackedObjects)
            {
                Scalar color = getColor(track.trackingId);
                rectangle(img, track.rect, color, 2);
                int xPoint = static_cast<int>(track.rect.x);
                int yPoint = static_cast<int>(track.rect.y);
                putText(img, to_string(track.trackingId), Point(xPoint, yPoint - 5), FONT_FACE, FONT_SCALE, RED);
            }
            writeTracksToFile(trackedObjects, TRACKINGS_OUTPUT_PATH, frameNumber);
        }
        writeDetectionsToFile(objects, DETECTIONS_OUTPUT_PATH, frameNumber);
        // Put efficiency information.
        // The function getPerfProfile returns the overall time for inference(t) and the timings for each of the layers(in layersTimes).
        vector<double> layersTimes;
        double freq = getTickFrequency() / 1000;
        double t = net.getPerfProfile(layersTimes) / freq;
        string label = format("Inference time : %.2f ms", t);
        putText(img, label, Point(20, 40), FONT_FACE, FONT_SCALE, RED);
        imshow("Output", img);
        writer.write(img);
        //waitKey(0);

        if (waitKey(1) == 27)
            break;

        frameNumber++;
    }
    writer.release();
    capture.release();
    cout<<"Output video generated";

    return 0;
}

vector<Mat> inference(Mat &inputImage, Net &net)
{
    // Convert to blob.
    Mat blob;
    dnn::blobFromImage(inputImage, blob, 1. / 255., Size(INPUT_WIDTH, INPUT_HEIGHT), Scalar(), true, false);

    net.setInput(blob);

    // Forward propagate.
    vector<Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    return outputs;
}

Mat postProcessImage(Mat &inputImage, vector<Mat> &output, const vector<string> &className,
    vector<Detection> &object)
{
    // Initialize vectors to hold respective outputs while unwrapping detections.
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;
    //cout << "Original Height and Width :" << inputImage.rows << "x" << inputImage.cols << endl;
    // Resizing factor.
    float xFactor = static_cast<float>(1.0f * inputImage.cols / INPUT_WIDTH);
    float yFactor = static_cast<float>(1.0f * inputImage.rows / INPUT_HEIGHT);
    bool yolov8 = false;

    int rows = output[0].size[1];
    int dimensions = output[0].size[2];

    if (dimensions > rows) // Check if shape[2] is more than shape[1] (yolov8)
    {
        yolov8 = true;
        rows = output[0].size[2];
        dimensions = output[0].size[1];
        output[0] = output[0].reshape(1, dimensions);
        cv::transpose(output[0], output[0]);
    }

    // Iterate through detections.
    float *data = (float*)output[0].data;
    Point classId;
    double maxClassScore;
    for (int i = 0; i < rows; ++i)
    {
        float confidence;
        float *classes_scores = 0;
        if (yolov8)
        {
            classes_scores = data + 4;
        }
        else
        {
            confidence = data[4];
            // Discard bad detections and continue.
            if (confidence >= CONFIDENCE_THRESHOLD)
            {
                classes_scores = data + 5;
            }
        }
        Mat scores(1, 80, CV_32FC1, classes_scores); //80 classes
        // Perform minMaxLoc and acquire the index of best class  score.
        minMaxLoc(scores, 0, &maxClassScore, 0, &classId);
        // Continue if the class score is above the threshold.
        if (maxClassScore > SCORE_THRESHOLD)
        {
            // Store class ID and confidence in the pre-defined respective vectors.
            if (yolov8)
            {
                confidences.push_back(static_cast<float>(maxClassScore));
            }
            else
            {
                confidences.push_back(confidence);
            }

            classIds.push_back(classId.x);
            // Center.
            float cx = data[0];
            float cy = data[1];
            // Box dimension.
            float w = data[2];
            float h = data[3];
            // Bounding box coordinates.
            int left = int((cx - 0.5 * w) * xFactor);
            int top = int((cy - 0.5 * h) * yFactor);
            int width = int(w * xFactor);
            int height = int(h * yFactor);
            // Store good detections in the boxes vector.
            boxes.push_back(Rect(left, top, width, height));
        }
        // Jump to the next row.
        data += dimensions;
    }
    vector<int> nmsResult;
    dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nmsResult);
    for (size_t i = 0; i < nmsResult.size(); i++)
    {
        int idx = nmsResult[i];
        Rect box = boxes[idx];
        int top = box.y;
        int left = box.x;
        int width = box.width;
        int height = box.height;
        // Draw bounding box.
        rectangle(inputImage, Point(left, top), Point(left + width, top + height), BLUE, 1 * THICKNESS);
        // Get the label for the class name and its confidence.
        string label = format("%.2f", confidences[idx]);
        label = className[classIds[idx]] + ":" + label;
        // Draw class labels.
        drawLabel(inputImage, label, left, top);
        Detection det(box, classIds[idx], confidences[idx]);
        object.push_back(det);
    }

    return inputImage;
}

void drawLabel(Mat &inputImage, string label, int left, int top)
{
    // Display the label at the top of the bounding box.
    int baseLine;
    Size labelSize = getTextSize(label, FONT_FACE, FONT_SCALE, THICKNESS, &baseLine);
    top = max(top, labelSize.height);
    // Top left corner.
    Point tlc = Point(left, top);
    // Bottom right corner.
    Point brc = Point(left + labelSize.width, top + labelSize.height + baseLine);
    // Draw white rectangle.
    rectangle(inputImage, tlc, brc, BLACK, FILLED);
    // Put the label on the black rectangle.
    putText(inputImage, label, Point(left, top + labelSize.height), FONT_FACE, FONT_SCALE, YELLOW, THICKNESS);
}

void writeDetectionsToFile(const vector<Detection> objects, const string &outputPath,
    const int frameNumber)
{
    // Open the output file for writing
    ofstream outputFile(outputPath, ios_base::app);
    if (!outputFile.is_open())
    {
        cout << "Failed to open the output file: " << outputPath << endl;
        return;
    }
    // Check if the output file is empty
    ifstream file(outputPath);
    bool isEmpty = file.peek() == ifstream::traits_type::eof();
    // Write the header row of the file is empty
    if (isEmpty)
    {
        outputFile << "frame, trackId, x, y, width, height, score, classId" << endl;
    }

    // Iterate over the detections and write the data to the output file
    for (const auto& object : objects)
    {
        // Extract the detection data (frame, trackId, x, y, width, height, score, classId)
        float y = object.rect.y;
        float x = object.rect.x;
        float width = object.rect.width;
        float height = object.rect.height;
        float score = object.classScore;
        int classId = object.classLabel;

        // Write the data to the output file
        outputFile << frameNumber << ", " << -1 << ", " << x << ", " << y << ", " << width << ", " << height << ", " << score << ", "<<classId << endl;
    }

    // Close the output file
    outputFile.close();

    //cout << "\n Detection data saved to: " << outputPath << endl;
}

void writeTracksToFile(const Mat& objects, const string &outputPath,
    const int frameNumber)
{
    // Open the output file for writing
    ofstream outputFile(outputPath, ios_base::app);
    if (!outputFile.is_open())
    {
        cout << "Failed to open the output file: " << outputPath << endl;
        return;
    }
    // Check if the output file is empty
    ifstream file(outputPath);
    bool isEmpty = file.peek() == ifstream::traits_type::eof();
    // Write the header row of the file is empty
    if (isEmpty)
    {
        outputFile << "frame, trackId, x, y, width, height, score, classId" << endl;
    }

    // Iterate over the detections and write the data to the output file
    for (int i = 0; i < objects.rows; ++i)
    {
        // Extract the detection data (frame, trackId, x1, y1, width, height, score, classId)
        float x = objects.at<float>(i, 0);
        float y = objects.at<float>(i, 1);
        float width = objects.at<float>(i, 2);
        float height = objects.at<float>(i, 3);
        int classId = static_cast<int>(objects.at<float>(i, 4));
        float score = objects.at<float>(i, 5);
        int trackId = static_cast<int>(objects.at<float>(i, 6));

        // Write the data to the output file
        outputFile << frameNumber << ", " << trackId << ", " << x << ", " << y << ", " << width << ", " << height << ", " << score << ", " << classId << endl;
    }

    // Close the output file
    outputFile.close();
    //cout << "\n Detection data saved to: " << outputPath << endl;
}

void writeTracksToFile(const vector<Track> objects, const string &outputPath,
    const int frameNumber)
{
    // Open the output file for writing
    ofstream outputFile(outputPath, ios_base::app);
    if (!outputFile.is_open())
    {
        cout << "Failed to open the output file: " << outputPath << endl;
        return;
    }
    // Check if the output file is empty
    ifstream file(outputPath);
    bool isEmpty = file.peek() == ifstream::traits_type::eof();
    // Write the header row of the file is empty
    if (isEmpty)
    {
        outputFile << "frame, trackId, x, y, width, height, score, classId" << endl;
    }


    // Iterate over the detections and write the data to the output file
    for (auto track : objects)
    {
        // Extract the detection data (frame, trackId, x1, y1, width, height, score, classId)
        float x = track.rect.x;
        float y = track.rect.y;
        float width = track.rect.width;
        float height = track.rect.height;
        int classId = track.classLabel;
        float score = track.classScore;
        int trackId = track.trackingId;


        // Write the data to the output file
        outputFile << frameNumber << ", " << trackId << ", " << x << ", " << y << ", " << width << ", " << height << ", " << score << ", " << classId << endl;
    }
    // Close the output file
    outputFile.close();
    //cout << "\n Detection data saved to: " << outputPath << endl;
}

Scalar getColor(const int idx)
{
    int value = idx + 3;
    return Scalar(37 * value % 255, 17 * value % 255, 29 * value % 255);
}

Mat detectionToMat(vector<Detection> objs)
{
    Mat output(static_cast<int>(objs.size()), 6, CV_32F);
    for (int i = 0; i < static_cast<int>(objs.size()); ++i)
    {
        const Detection& detection = objs[i];
        cv::Mat row = output.row(i);

        row.at<float>(0) = detection.rect.x;
        row.at<float>(1) = detection.rect.y;
        row.at<float>(2) = detection.rect.width;
        row.at<float>(3) = detection.rect.height;
        row.at<float>(4) = static_cast<float>(detection.classLabel);
        row.at<float>(5) = detection.classScore;
    }

    return output;
}