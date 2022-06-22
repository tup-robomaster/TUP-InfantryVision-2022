// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "inference.h"

/**
 * @brief Define names based depends on Unicode path support
 */

// static constexpr int INPUT_W = 640;    // Width of input
// static constexpr int INPUT_H = 384;    // Height of input
static constexpr int INPUT_W = 416;    // Width of input
static constexpr int INPUT_H = 416;    // Height of input
static constexpr int NUM_CLASSES = 2;  // Number of classes
static constexpr int NUM_COLORS = 2;   // Number of color
static constexpr int TOPK = 128;       // TopK
static constexpr float NMS_THRESH  = 0.1;
static constexpr float BBOX_CONF_THRESH = 0.6;
static constexpr float MERGE_CONF_ERROR = 0.15;
static constexpr float MERGE_MIN_IOU = 0.2;

static inline int argmax(const float *ptr, int len) 
{
    int max_arg = 0;
    for (int i = 1; i < len; i++) {
        if (ptr[i] > ptr[max_arg]) max_arg = i;
    }
    return max_arg;
}

/**
 * @brief Resize the image using letterbox
 * @param img Image before resize
 * @param transform_matrix Transform Matrix of Resize
 * @return Image after resize
 */
inline cv::Mat scaledResize(cv::Mat& img, Eigen::Matrix<float,3,3> &transform_matrix)
{
    float r = std::min(INPUT_W / (img.cols * 1.0), INPUT_H / (img.rows * 1.0));
    int unpad_w = r * img.cols;
    int unpad_h = r * img.rows;
    
    int dw = INPUT_W - unpad_w;
    int dh = INPUT_H - unpad_h;

    dw /= 2;
    dh /= 2;
    
    transform_matrix << 1.0 / r, 0, -dw / r,
                        0, 1.0 / r, -dh / r,
                        0, 0, 1;
    
    Mat re;
    cv::resize(img, re, Size(unpad_w,unpad_h));
    Mat out;
    cv::copyMakeBorder(re, out, dh, dh, dw, dw, BORDER_CONSTANT);

    return out;
}

/**
 * @brief Generate grids and stride.
 * @param target_w Width of input.
 * @param target_h Height of input.
 * @param strides A vector of stride.
 * @param grid_strides Grid stride generated in this function.
 */
static void generate_grids_and_stride(const int target_w, const int target_h,
                                        std::vector<int>& strides, std::vector<GridAndStride>& grid_strides)
{
    for (auto stride : strides)
    {
        int num_grid_w = target_w / stride;
        int num_grid_h = target_h / stride;

        for (int g1 = 0; g1 < num_grid_h; g1++)
        {
            for (int g0 = 0; g0 < num_grid_w; g0++)
            {
                grid_strides.push_back((GridAndStride){g0, g1, stride});
            }
        }
    }
}

/**
 * @brief Generate Proposal
 * @param grid_strides Grid strides
 * @param feat_ptr Original predition result.
 * @param prob_threshold Confidence Threshold.
 * @param objects Objects proposed.
 */
static void generateYoloxProposals(std::vector<GridAndStride> grid_strides, const float* feat_ptr,
                                    Eigen::Matrix<float,3,3> &transform_matrix,float prob_threshold,
                                    std::vector<BuffObject>& objects)
{

    const int num_anchors = grid_strides.size();
    //Travel all the anchors
    for (int anchor_idx = 0; anchor_idx < num_anchors; anchor_idx++)
    {
        const int grid0 = grid_strides[anchor_idx].grid0;
        const int grid1 = grid_strides[anchor_idx].grid1;
        const int stride = grid_strides[anchor_idx].stride;

	    const int basic_pos = anchor_idx * (11 + NUM_COLORS + NUM_CLASSES);

        // yolox/models/yolo_head.py decode logic
        //  outputs[..., :2] = (outputs[..., :2] + grids) * strides
        //  outputs[..., 2:4] = torch.exp(outputs[..., 2:4]) * strides
        float x_1 = (feat_ptr[basic_pos + 0] + grid0) * stride;
        float y_1 = (feat_ptr[basic_pos + 1] + grid1) * stride;
        float x_2 = (feat_ptr[basic_pos + 2] + grid0) * stride;
        float y_2 = (feat_ptr[basic_pos + 3] + grid1) * stride;
        float x_3 = (feat_ptr[basic_pos + 4] + grid0) * stride;
        float y_3 = (feat_ptr[basic_pos + 5] + grid1) * stride;
        float x_4 = (feat_ptr[basic_pos + 6] + grid0) * stride;
        float y_4 = (feat_ptr[basic_pos + 7] + grid1) * stride;
        float x_5 = (feat_ptr[basic_pos + 8] + grid0) * stride;
        float y_5 = (feat_ptr[basic_pos + 9] + grid1) * stride;

        int box_color = argmax(feat_ptr + basic_pos + 11, NUM_COLORS);
        int box_class = argmax(feat_ptr + basic_pos + 11 + NUM_COLORS, NUM_CLASSES);

        float box_objectness = (feat_ptr[basic_pos + 10]);
        
        float color_conf = (feat_ptr[basic_pos + 11 + box_color]);
        float cls_conf = (feat_ptr[basic_pos + 11 + NUM_COLORS + box_class]);

        // cout<<box_objectness<<endl;
        // float box_prob = (box_objectness + cls_conf + color_conf) / 3.0;
        float box_prob = box_objectness;

        if (box_prob >= prob_threshold)
        {
            BuffObject obj;

            Eigen::Matrix<float,3,5> apex_norm;
            Eigen::Matrix<float,3,5> apex_dst;

            apex_norm << x_1,x_2,x_3,x_4,x_5,
                        y_1,y_2,y_3,y_4,y_5,
                        1,1,1,1,1;
            
            apex_dst = transform_matrix * apex_norm;

            for (int i = 0; i < 5; i++)
                obj.apex[i] = cv::Point2f(apex_dst(0,i),apex_dst(1,i));
            for (int i = 0; i < 5; i++)
            {
                obj.apex[i] = cv::Point2f(apex_dst(0,i),apex_dst(1,i));
                obj.pts.push_back(obj.apex[i]);
            }
            vector<cv::Point2f> tmp(obj.apex,obj.apex + 5);
            obj.rect = cv::boundingRect(tmp);

            obj.cls = box_class;
            obj.color = box_color;
            obj.prob = box_prob;

            objects.push_back(obj);
        }

    } // point anchor loop
}

/**
 * @brief Calculate intersection area between two objects.
 * @param a Object a.
 * @param b Object b.
 * @return Area of intersection.
 */
static inline float intersection_area(const BuffObject& a, const BuffObject& b)
{
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

static void qsort_descent_inplace(std::vector<BuffObject>& faceobjects, int left, int right)
{
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j)
    {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j)
        {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (left < j) qsort_descent_inplace(faceobjects, left, j);
        }
        #pragma omp section
        {
            if (i < right) qsort_descent_inplace(faceobjects, i, right);
        }
    }
}


static void qsort_descent_inplace(std::vector<BuffObject>& objects)
{
    if (objects.empty())
        return;

    qsort_descent_inplace(objects, 0, objects.size() - 1);
}


static void nms_sorted_bboxes(std::vector<BuffObject>& faceobjects, std::vector<int>& picked,
                            float nms_threshold)
{
    picked.clear();
    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++)
    {
        std::vector<cv::Point2f> object_apex_tmp(faceobjects[i].apex, faceobjects[i].apex + 5);
        areas[i] = contourArea(object_apex_tmp);
        // areas[i] = faceobjects[i].rect.area();
    }

    for (int i = 0; i < n; i++)
    {
        BuffObject& a = faceobjects[i];
        std::vector<cv::Point2f> apex_a(a.apex, a.apex + 5);
        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            BuffObject& b = faceobjects[picked[j]];
            std::vector<cv::Point2f> apex_b(b.apex, b.apex + 5);
            std::vector<cv::Point2f> apex_inter;
            // intersection over union
            // float inter_area = intersection_area(a, b);
            // float union_area = areas[i] + areas[picked[j]] - inter_area;
            //TODO:此处耗时较长，大约1ms，可以尝试使用其他方法计算IOU与多边形面积
            float inter_area = intersectConvexConvex(apex_a,apex_b,apex_inter);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            float iou = inter_area / union_area;

            if (iou > nms_threshold || isnan(iou))
            {
                keep = 0;
                //Stored for Merge
                if (iou > MERGE_MIN_IOU && abs(a.prob - b.prob) < MERGE_CONF_ERROR 
                                        && a.cls == b.cls && a.color == b.color)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        b.pts.push_back(a.apex[i]);
                    }
                }
                // cout<<b.pts_x.size()<<endl;
            }
        }

        if (keep)
            picked.push_back(i);
    }
}

/**
 * @brief Decode outputs.
 * @param prob Original predition output.
 * @param objects Vector of objects predicted.
 * @param img_w Width of Image.
 * @param img_h Height of Image.
 */
static void decodeOutputs(const float* prob, std::vector<BuffObject>& objects,
                            Eigen::Matrix<float,3,3> &transform_matrix, const int img_w, const int img_h)
{
        std::vector<BuffObject> proposals;
        std::vector<int> strides = {8, 16, 32};
        std::vector<GridAndStride> grid_strides;

        generate_grids_and_stride(INPUT_W, INPUT_H, strides, grid_strides);
        generateYoloxProposals(grid_strides, prob, transform_matrix, BBOX_CONF_THRESH, proposals);
        qsort_descent_inplace(proposals);

        if (proposals.size() >= TOPK) 
            proposals.resize(TOPK);
        std::vector<int> picked;
        nms_sorted_bboxes(proposals, picked, NMS_THRESH);
        int count = picked.size();
        objects.resize(count);

        for (int i = 0; i < count; i++)
        {
            objects[i] = proposals[picked[i]];
        }
}

BuffDetector::BuffDetector()
{

}

BuffDetector::~BuffDetector()
{
}

//TODO:change to your dir
bool BuffDetector::initModel(string path)
{
    ie.SetConfig({{CONFIG_KEY(CACHE_DIR), "../.cache"}});
    // ie.SetConfig({{CONFIG_KEY(GPU_THROUGHPUT_STREAMS),"GPU_THROUGHPUT_AUTO"}});
    ie.SetConfig({{CONFIG_KEY(GPU_THROUGHPUT_STREAMS),"1"}}  );
    // Step 1. Read a model in OpenVINO Intermediate Representation (.xml and
    // .bin files) or ONNX (.onnx file) format
    network = ie.ReadNetwork(path);
    if (network.getOutputsInfo().size() != 1)
        throw std::logic_error("Sample supports topologies with 1 output only");

    // Step 2. Configure input & output
    //  Prepare input blobs
    InputInfo::Ptr input_info = network.getInputsInfo().begin()->second;
    input_name = network.getInputsInfo().begin()->first;


    //  Prepare output blobs
    if (network.getOutputsInfo().empty())
    {
        std::cerr << "Network outputs info is empty" << std::endl;
        return EXIT_FAILURE;
    }
    DataPtr output_info = network.getOutputsInfo().begin()->second;
    output_name = network.getOutputsInfo().begin()->first;

    // output_info->setPrecision(Precision::FP16);
    // Step 3. Loading a model to the device
    // executable_network = ie.LoadNetwork(network, "MULTI:GPU");
    executable_network = ie.LoadNetwork(network, "GPU");
    // executable_network = ie.LoadNetwork(network, "CPU");

    // Step 4. Create an infer request
    infer_request = executable_network.CreateInferRequest();
    const Blob::Ptr output_blob = infer_request.GetBlob(output_name);
    moutput = as<MemoryBlob>(output_blob);
    // Blob::Ptr input = infer_request.GetBlob(input_name);     // just wrap Mat data by Blob::Ptr
    if (!moutput)
    {
        throw std::logic_error("We expect output to be inherited from MemoryBlob, "
                                "but by fact we were not able to cast output to MemoryBlob");
    }
    // locked memory holder should be alive all time while access to its buffer
    // happens
    return true;
}

bool BuffDetector::detect(Mat &src,std::vector<BuffObject>& objects)
{
    if (src.empty())
    {
        return false;
    }
    cv::Mat pr_img = scaledResize(src,transfrom_matrix);
#ifdef SHOW_INPUT
    namedWindow("network_input",0);
    imshow("network_input",pr_img);
    waitKey(1);
#endif //SHOW_INPUT
    cv::Mat pre;
    cv::Mat pre_split[3];
    pr_img.convertTo(pre,CV_32F);
    cv::split(pre,pre_split);

    Blob::Ptr imgBlob = infer_request.GetBlob(input_name);     // just wrap Mat data by Blob::Ptr
    InferenceEngine::MemoryBlob::Ptr mblob = InferenceEngine::as<InferenceEngine::MemoryBlob>(imgBlob);
    // locked memory holder should be alive all time while access to its buffer happens
    auto mblobHolder = mblob->wmap();
    float *blob_data = mblobHolder.as<float *>();

    auto img_offset = INPUT_W * INPUT_H;
    //Copy img into blob
    for(int c = 0;c < 3;c++)
    {
        memcpy(blob_data, pre_split[c].data, INPUT_W * INPUT_H * sizeof(float));
        blob_data += img_offset;
    }

    infer_request.Infer();
    // -----------------------------------------------------------------------------------------------------
    // --------------------------- Step 8. Process output----------------
    // const Blob::Ptr output_blob = infer_request.GetBlob(output_name);
    // MemoryBlob::CPtr moutput = as<MemoryBlob>(output_blob);

    auto moutputHolder = moutput->rmap();
    const float* net_pred = moutputHolder.as<const PrecisionTrait<Precision::FP32>::value_type*>();
    int img_w = src.cols;
    int img_h = src.rows;
    decodeOutputs(net_pred, objects, transfrom_matrix, img_w, img_h);
    for (auto object = objects.begin(); object != objects.end(); ++object)
    {
        if ((*object).pts.size() >= 10)
        {
            auto N = (*object).pts.size();
            cv::Point2f pts_final[5];

            for (int i = 0; i < N; i++)
            {
                pts_final[i % 5]+=(*object).pts[i];
            }

            for (int i = 0; i < 5; i++)
            {
                pts_final[i].x = pts_final[i].x / (N / 5);
                pts_final[i].y = pts_final[i].y / (N / 5);
            }

            (*object).apex[0] = pts_final[0];
            (*object).apex[1] = pts_final[1];
            (*object).apex[2] = pts_final[2];
            (*object).apex[3] = pts_final[3];
            (*object).apex[4] = pts_final[4];
        }
        // (*object).area = (int)(calcTetragonArea((*object).apex));
    }
    if (objects.size() != 0)
        return true;
    else 
        return false;

}
