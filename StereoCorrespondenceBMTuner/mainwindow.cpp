#include "mainwindow.h"
#include "ui_mainwindow.h"

QImage Mat2QImage(cv::Mat const& src)
{
    cv::Mat temp; // make the same cv::Mat
    cvtColor(src, temp, cv::COLOR_BGR2RGB); // cvtColor Makes a copt, that what i need
    QImage dest((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    dest.bits(); // enforce deep copy, see documentation 
    // of QImage::QImage ( const uchar * data, int width, int height, Format format )
    return dest;
}

cv::Mat QImage2Mat(QImage const& src)
{
    cv::Mat tmp(src.height(), src.width(), CV_8UC3, (uchar*)src.bits(), src.bytesPerLine());
    cv::Mat result; // deep copy just in case (my lack of knowledge with open cv)
    cvtColor(tmp, result, cv::COLOR_RGB2BGR);
    return result;
}

template<typename T, typename U, typename V>
inline cv::Scalar cvJetColourMat(T v, U vmin, V vmax) {
    cv::Scalar c = cv::Scalar(1.0, 1.0, 1.0);  // white
    T dv;

    if (v < vmin)
        v = vmin;
    if (v > vmax)
        v = vmax;
    dv = vmax - vmin;

    if (v < (vmin + 0.25 * dv)) {
        c.val[0] = 0;
        c.val[1] = 4 * (v - vmin) / dv;
    }
    else if (v < (vmin + 0.5 * dv)) {
        c.val[0] = 0;
        c.val[2] = 1 + 4 * (vmin + 0.25 * dv - v) / dv;
    }
    else if (v < (vmin + 0.75 * dv)) {
        c.val[0] = 4 * (v - vmin - 0.5 * dv) / dv;
        c.val[2] = 0;
    }
    else {
        c.val[1] = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
        c.val[2] = 0;
    }
    return(c);
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bmState(cv::StereoBM::create())
{
    ui->setupUi(this);

    // the default values used in OpenCV are defined here:
    // https://github.com/Itseez/opencv/blob/master/modules/calib3d/src/stereobm.cpp
    bmState->setPreFilterSize(41);  // must be an odd between 5 and 255
    bmState->setPreFilterCap(31);  // must be within 1 and 63
    bmState->setBlockSize(41);  // must be odd, be within 5..255 and be not larger than image width or height
    bmState->setMinDisparity(-64);
    bmState->setNumDisparities(128);  // must be > 0 and divisible by 16
    bmState->setTextureThreshold(10);  // must be non-negative
    bmState->setUniquenessRatio(15);  // must be non-negative
    bmState->setSpeckleWindowSize(0);
    bmState->setSpeckleRange(0);
    bmState->setDisp12MaxDiff(-1);

    // we override the default values defined in the UI file with Qt Designer
    // to the ones defined above
    ui->horizontalSlider_pre_filter_size->setValue(bmState->getPreFilterSize());
    ui->horizontalSlider_pre_filter_cap->setValue(bmState->getPreFilterCap());
    ui->horizontalSlider_SAD_window_size->setValue(bmState->getBlockSize());
    ui->horizontalSlider_min_disparity->setValue(bmState->getMinDisparity());
    ui->horizontalSlider_num_of_disparity->setValue(bmState->getNumDisparities());
    ui->horizontalSlider_texture_threshold->setValue(bmState->getTextureThreshold());
    ui->horizontalSlider_uniqueness_ratio->setValue(bmState->getUniquenessRatio());
    ui->horizontalSlider_speckle_window_size->setValue(bmState->getSpeckleWindowSize());
    ui->horizontalSlider_speckle_range->setValue(bmState->getSpeckleRange());
    ui->horizontalSlider_disp_12_max_diff->setValue(bmState->getDisp12MaxDiff());
}

MainWindow::~MainWindow()
{
    delete ui;
}

// method called when the button to change the left image is clicked
// we prompt the user to select an image, and we display it
void MainWindow::on_pushButton_left_clicked()
{
    // we prompt the user with a file dialog,
    // to select the picture file from the left camera
    QString filename = QFileDialog::getOpenFileName(this, "Select left picture file", QDir::homePath(), NULL);
    if (filename.isNull() || filename.isEmpty())
        return;

    ///// Qt display stuff

    // we load the picture from the file, to display it in a QLabel in the GUI
    QImage left_picture;
    left_picture.load(filename);

    // some computation to resize the image if it is too big to fit in the GUI
    QPixmap left_pixmap = QPixmap::fromImage(left_picture);
    int max_width  = std::min(ui->label_image_left->maximumWidth(), left_picture.width());
    int max_height = std::min(ui->label_image_left->maximumHeight(), left_picture.height());
    ui->label_image_left->setPixmap(left_pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));

    set_SADWindowSize();  // the SAD window size parameter depends on the size of the image

    ///// OpenCV disparity map computation

    // we convert filename from QString to std::string (needed by OpenCV)
    std::string filename_s = filename.toUtf8().constData();

    // we load the picture in the OpenCV Mat format, to compute depth map
    cv::Mat mat = cv::imread(filename_s, cv::IMREAD_COLOR);
    cv::cvtColor(mat, mat, cv::COLOR_BGRA2GRAY);  // we convert to gray, needed to compute depth map
    this->left_image = mat;

    compute_depth_map();
}

// method called when the button to change the right image is clicked
// we prompt the user to select an image, and we display it
void MainWindow::on_pushButton_right_clicked()
{
    // we prompt the user with a file dialog,
    // to select the picture file from the left camera
    QString filename = QFileDialog::getOpenFileName(this, "Select right picture file", QDir::homePath(), NULL);
    if (filename.isNull() || filename.isEmpty())
        return;

    ///// Qt display stuff

    // we load the picture from the file, to display it in a QLabel in the GUI
    QImage right_picture;
    right_picture.load(filename);

    // some computation to resize the image if it is too big to fit in the GUI
    QPixmap right_pixmap = QPixmap::fromImage(right_picture);
    int max_width  = std::min(ui->label_image_right->maximumWidth(), right_picture.width());
    int max_height = std::min(ui->label_image_right->maximumHeight(), right_picture.height());
    ui->label_image_right->setPixmap(right_pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));

    set_SADWindowSize();  // the SAD window size parameter depends on the size of the image

    ///// OpenCV disparity map computation

    // we convert filename from QString to std::string (needed by OpenCV)
    std::string filename_s = filename.toUtf8().constData();

    // we load the picture in the OpenCV Mat format, to compute depth map
    cv::Mat mat = cv::imread(filename_s, cv::IMREAD_COLOR);
    cv::cvtColor(mat, mat, cv::COLOR_BGRA2GRAY);  // we convert to gray, needed to compute depth map
    this->right_image = mat;

    compute_depth_map();
}

// we compute the depth map, if both left image and right image have been added
void MainWindow::compute_depth_map() {
    // we check that both images have been loaded
    if (this->left_image.empty() || this->right_image.empty())
        return;

    // we check that both images have the same size (else OpenCV throws an error)
    if (left_image.rows != right_image.rows || left_image.cols != right_image.cols) {
        ui->label_depth_map->setText("Can't compute depth map: left and right images should be the same size");
        return;
    }

    // we compute the depth map
    cv::Mat disparity_16S, disparity;  // 16 bits, signed
    bmState->compute(left_image, right_image, disparity_16S);

    // we convert the depth map to a QPixmap, to display it in the QUI
    // first, we need to convert the disparity map to a more regular grayscale format
    // then, we convert to RGB, and finally, we can convert to a QImage and then a QPixmap
 
    // we normalize the values, so that they all fit in the range [0, 255]
    cv::normalize(disparity_16S, disparity_16S, 0, 255, cv::NORM_MINMAX, CV_8UC1);

    // Scaling down the disparity values and normalizing them                                                                                                                                                                                                                     disparity = (disparity/16.0f - (float)minDisparity)/((float)numDisparities);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                // Displaying the disparity map                                                                                                                                                                                                                                               cv::imshow("disparity",disparity);
    cv::normalize(disparity_16S, disparity, 0, 255, cv::NORM_MINMAX, CV_8UC1);

    // we convert from gray to color
    cv::Mat disp_color;
    applyColorMap(disparity, disp_color, cv::COLORMAP_JET);


    // we finally can convert the image to a QPixmap and display it
    QImage disparity_image = Mat2QImage(disp_color);
    //QImage img((uchar*)mat.data, mat.cols, mat.rows, mat.step1(), QImage::Format_RGB32);
    QPixmap disparity_pixmap = QPixmap::fromImage(disparity_image);

    // some computation to resize the image if it is too big to fit in the GUI
    int max_width = std::min(ui->label_depth_map->maximumWidth(), disparity_image.width());
    int max_height = std::min(ui->label_depth_map->maximumHeight(), disparity_image.height());
    ui->label_depth_map->setPixmap(disparity_pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));
}


/////////////////// Sliders management (callbacks and constraints) //////////////////////

///// pre filter size

// must be an odd number
void MainWindow::on_horizontalSlider_pre_filter_size_valueChanged(int value)
{
    if ((value % 2) == 0) {
        value -= 1;
        ui->horizontalSlider_pre_filter_size->setValue(value);
    }

    bmState->setPreFilterSize(value);
    compute_depth_map();
}

///// pre filter cap

void MainWindow::on_horizontalSlider_pre_filter_cap_valueChanged(int value)
{
    bmState->setPreFilterCap(value);
    compute_depth_map();
}

///// SAD window size

// the SAD Window size should always be smaller than the size of the images
// so when a new image is loaded, we set the maximum value for the slider
void MainWindow::set_SADWindowSize() {
    int value = 255;  // max value allowed

    // we check that the value is not bigger than the size of the pictures
    if (! left_image.empty())
        value = std::min(value, std::min(left_image.cols, left_image.rows));
    if (! right_image.empty())
        value = std::min(value, std::min(right_image.cols, right_image.rows));

    // we check that the value is >= 5
    value = std::max(value, 5);

    ui->horizontalSlider_SAD_window_size->setMaximum(value);
}

// must be an odd number
void MainWindow::on_horizontalSlider_SAD_window_size_valueChanged(int value)
{
    if ((value % 2) == 0) {
        value -= 1;
        ui->horizontalSlider_SAD_window_size->setValue(value);
    }

    bmState->setBlockSize(value);
    compute_depth_map();
}

///// Minimum disparity

void MainWindow::on_horizontalSlider_min_disparity_valueChanged(int value)
{
    bmState->setMinDisparity(value);
    compute_depth_map();
}

///// Number of disparities

// callback when slider for number of disparities is moved
// we must allow only values that are divisible by 16
void MainWindow::on_horizontalSlider_num_of_disparity_sliderMoved(int value)
{
    set_num_of_disparity_slider_to_multiple_16(value);
}

// callback when slider for number of disparities is changed
// (for the case when the slider is not moved (just a click), because then the callback above is not called)
// we must allow only values that are divisible by 16
void MainWindow::on_horizontalSlider_num_of_disparity_valueChanged(int value)
{
    set_num_of_disparity_slider_to_multiple_16(value);
}

void MainWindow::set_num_of_disparity_slider_to_multiple_16(int value) {
    if ((value % 16) != 0) {
        value -= (value % 16);
        ui->horizontalSlider_num_of_disparity->setValue(value);
    }

    bmState->setNumDisparities(value);
    compute_depth_map();
}

///// Texture threshold

void MainWindow::on_horizontalSlider_texture_threshold_valueChanged(int value)
{
    bmState->setTextureThreshold(value);
    compute_depth_map();
}

///// Uniqueness ratio

void MainWindow::on_horizontalSlider_uniqueness_ratio_valueChanged(int value)
{
    bmState->setUniquenessRatio(value);
    compute_depth_map();
}

///// Speckle window size

void MainWindow::on_horizontalSlider_speckle_window_size_valueChanged(int value)
{
    bmState->setSpeckleWindowSize(value);
    compute_depth_map();
}

///// Speckle range

void MainWindow::on_horizontalSlider_speckle_range_valueChanged(int value)
{
    bmState->setSpeckleRange(value);
    compute_depth_map();
}

///// Disparity 12 maximum difference

void MainWindow::on_horizontalSlider_disp_12_max_diff_valueChanged(int value)
{
    bmState->setDisp12MaxDiff(value);
    compute_depth_map();
}
