
include "GDALImageReaderThread.h"
include "iplimagetoqimage.h" //for IplImage converting to QImage
#include "commontoolfunctions.h" //for calculate distance by LB
#include <cv.h> //for OpenCV
#include <highgui.h>  //for OpenCV
#include <limits.h> //for INT_MAX, INT_MIN, ...
#include <float.h> //for FLT_MAX, FLT_MIN, ...

QMutex G_imageReadMutex;
QWaitCondition G_imageReadComplete;

RSI_ImageReaderThread::RSI_ImageReaderThread(RSI_MapCanvas* pCanvas)
	: m_pCanvas(pCanvas)
{
	RSI_ImageReaderThread::~RSI_ImageReaderThread()
		//线程函数
		RSI_ImageReaderThread::run()
		G_imageReadMutex.lock();
	m_pBand_GDAL_->RasterIO(GF_Read, 0, 0,
		m_nXSize_GDAL_, m_nYSize_GDAL_,
		m_pDataBufStart_GDAL_,
		m_nBufXSize_GDAL_, m_nBufYSize_GDAL_,
		m_eBufDataType_GDAL_,
		m_nBufPixSpace_GDAL_,
		m_nBufLineSpace_GDAL_);
	//G_imageReadComplete.wakeAll();
	G_imageReadMutex.unlock();
	//读图函数：使用GDAL和OpenCV读取图像文件
	RSI_ImageReaderThread::ReadFile(const QString& imageFileName)
		m_imageFileName = imageFileName;
	(m_imageFileName.isEmpty() || m_pCanvas == NULL)
		return
		//选择打开图像的驱动方式: GDAL or OpenCV
		const QString strOpenCV = tr("OpenCV"
		const QString strGDAL = tr("GDAL"
		IMG_OPEN_DRIVER_T  eOpenDriverType;
	QStringList items; //for input dialog
	items << strOpenCV << strGDAL;
	curDriverItemIdx = 0;
	//先根据扩展名推荐打开方式
	QString fileExtName = m_imageFileName.right(3).toLower();
	(fileExtName == QString("bmp"
		|| fileExtName == QString("png"
		|| fileExtName == QString("jpg"
		|| fileExtName == QString("peg"
		|| fileExtName == QString("jpe"
		|| fileExtName == QString("ppm"
		|| fileExtName == QString("pbm"
		|| fileExtName == QString("pgm"
		//如果是常用格式，推荐使用OpenCV打开: (*.bmp *.png *.jpg *.jpeg *.jpe *.ppm *.pgm *.pbm)
		eOpenDriverType = RSI_USE_OPENCV; //根据扩展名来推荐使用哪种打开方式
	curDriverItemIdx = 0;
	eOpenDriverType = RSI_USE_GDAL;
	curDriverItemIdx = 1;
	//弹出对话框让用户选择使用哪种驱动方式打开图像
	dlgOk;
	QString itemSel = QInputDialog::getItem(m_pCanvas, tr("Please select..."
		tr("Driver for image opener:"), items, curDriverItemIdx, false, &dlgOk);
	(dlgOk && !itemSel.isEmpty())
		(itemSel == strOpenCV)
		eOpenDriverType = RSI_USE_OPENCV;
	(itemSel == strGDAL)
		eOpenDriverType = RSI_USE_GDAL;
	return
		//进度条
		progressBarValue = 2;
	QProgressDialog progress(tr("Reading image file..."), tr("Abort"), 0, 100, m_pCanvas);
	QProgressBar prgBar(&progress);
	prgBar.setTextVisible(false//不显示百分比
		progress.setBar(&prgBar);
	progress.setWindowModality(Qt::ApplicationModal);
	progress.setCancelButton(NULL);
	progress.setWindowTitle(tr("Please wait..."
		progress.setValue(progressBarValue);
	//use GDAL and OpenCV open image files
	(eOpenDriverType == RSI_USE_OPENCV)
		//打开图像文件
		IplImage* pIplImg = NULL; //声明IplImage指针
	((pIplImg = cvLoadImage(m_imageFileName.toStdString().c_str(), 1)) == 0) //使用OpenCV highgui打开图像文件
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed!"), tr(
		return
		//读取图像信息
		QString rsiFileInfo; //图像信息
	QString tempStr;
	tempStr = tr("Size is %1 x %2 x [%3(bits) x %4(channels)]/n"
		.arg(pIplImg->width)
		.arg(pIplImg->height)
		.arg(pIplImg->depth)
		.arg(pIplImg->nChannels);
	rsiFileInfo += tempStr;
	uchar* imageDataBuffer = NULL;
	QImage* pQImage = IplImageToQImage(pIplImg, &imageDataBuffer); //转换成QImage
	//创建图层
	RSI_ImageLayer* pImageLayer = RSI_ImageLayer(*pQImage);
	pImageLayer->setLayerName(m_imageFileName);
	pImageLayer->setImageLayerInfo(rsiFileInfo);
	pImageLayer->setBufferToBeFree(imageDataBuffer); //记录到m_bufferToBeFree中，待关闭图像时释放buffer内存
	pImageLayer->setVisible(false//默认不显示
		pImageLayer->setFlag(QGraphicsItem::ItemIsMovable, false
		m_pCanvas->AddImageLayer(pImageLayer);
	//释放临时内存
	cvReleaseImage(&pIplImg);
	delete pQImage;
	_READ_IMAGE_FILE_OVER; //读取完毕直接返回
	(eOpenDriverType == RSI_USE_GDAL)
		//如果是遥感图像格式，则使用GDAL打开
		GDALAllRegister();
	GDALDataset* poDataset;  //GDAL数据集
	poDataset = (GDALDataset*)GDALOpen(m_imageFileName.toStdString().c_str(), GA_ReadOnly);
	(poDataset == NULL)
		//QMessageBox::warning(NULL, tr("Failed"), tr("GDAL Open Image File Failed!"), tr("OK"));
		return
		//读取遥感图像信息
		QString rsiFileInfo; //遥感图像信息
	QString tempStr;
	tempStr = tr("GDAL Driver: %1/%2/n"
		.arg(QString(poDataset->GetDriver()->GetDescription()))
		.arg(QString(poDataset->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME)));
	rsiFileInfo += tempStr;
	tempStr = tr("Size is %1 x %2 x %3(bands)/n"
		.arg(poDataset->GetRasterXSize())
		.arg(poDataset->GetRasterYSize())
		.arg(poDataset->GetRasterCount());
	rsiFileInfo += tempStr;
地理参考坐标系统:
	The returned string defines the projection coordinate system of the image in OpenGIS WKT format.
		double adfGeoTransform[6];
	QString prjRefStr(poDataset->GetProjectionRef());
	(prjRefStr.isEmpty())
		prjRefStr = tr("none"//如果此字段为空，那么投影方式可能是GCPs
		//非空时才读取仿射变换信息
		tempStr = tr("Projection(WKT format) is '%1'/n").arg(prjRefStr);
	rsiFileInfo += tempStr;
	//非空时才读取仿射变换信息
	仿射地理变换信息：
		adfGeoTransform[0] // top left x  
		adfGeoTransform[1] // w-e pixel resolution  
		adfGeoTransform[2] // rotation, 0 if image is "north up"  
		adfGeoTransform[3] // top left y  
		adfGeoTransform[4] // rotation, 0 if image is "north up"  
		adfGeoTransform[5] // n-s pixel resolution  
		计算某一点的地理坐标可以如下计算：
		Xgeo = GT(0) + Xpixel*GT(1) + Yline*GT(2)
		Ygeo = GT(3) + Xpixel*GT(4) + Yline*GT(5)
		poDataset->GetGeoTransform(adfGeoTransform);
	tempStr = tr("Origin Cords = (%1,%2)/n"
		.arg(adfGeoTransform[0], 0,
		.arg(adfGeoTransform[3], 0,
		rsiFileInfo += tempStr;
	tempStr = tr("Pixel Size(degrees) = (%1,%2)/n"
		.arg(adfGeoTransform[1], 0,
		.arg(adfGeoTransform[5], 0,
		rsiFileInfo += tempStr;
	//读取GCPs投影方式
	QString prjGCPStr(poDataset->GetGCPProjection());
	(prjGCPStr.isEmpty())
		prjGCPStr = tr("none"
		tempStr = tr("GCPs Prj is '%1'/n").arg(prjGCPStr);
	rsiFileInfo += tempStr;
	nGCPs = poDataset->GetGCPCount(); //获得GCP控制点的个数
	const GDAL_GCP* pGCPs = poDataset->GetGCPs(); //获得GCP控制点
	tempStr = tr("Number of GCPs: %1/n").arg(nGCPs);
	rsiFileInfo += tempStr;
	//由GCPs获得仿射变换参数
	GDALGCPsToGeoTransform(nGCPs, pGCPs, adfGeoTransform, TRUE);
	tempStr = tr("Origin Cords = (%1,%2)/n"
		.arg(adfGeoTransform[0], 0,
		.arg(adfGeoTransform[3], 0,
		rsiFileInfo += tempStr;
	tempStr = tr("Pixel Size(degrees) = (%1,%2)/n"
		.arg(adfGeoTransform[1], 0,
		.arg(adfGeoTransform[5], 0,
		rsiFileInfo += tempStr;
	//根据adfGeoTransform计算每个像素代表多少米距离
	double oriCordL = adfGeoTransform[0];
	double oriCordB = adfGeoTransform[3];
	double tempL, tempB;
	GDALApplyGeoTransform(adfGeoTransform, 100, 0, &tempL, &tempB); //计算(100,0)像素处的L,B坐标
	double tempDist = CalcDistanceByLB(oriCordL, oriCordB, tempL, tempB);
	double xMeterPerPix = tempDist * 1000.0 / 100.0; //单位为米
	GDALApplyGeoTransform(adfGeoTransform, 0, 100, &tempL, &tempB); //计算(0,100)像素处的L,B坐标
	tempDist = CalcDistanceByLB(oriCordL, oriCordB, tempL, tempB);
	double yMeterPerPix = tempDist * 1000.0 / 100.0; //单位为米
	tempStr = tr("Pixel Size(meters) = (%1,%2)/n"
		.arg(xMeterPerPix, 0,
		.arg(yMeterPerPix, 0,
		rsiFileInfo += tempStr;
	//读取图像大小
	nBandCount = poDataset->GetRasterCount();
	nImgSizeX = poDataset->GetRasterXSize();
	nImgSizeY = poDataset->GetRasterYSize();
	//读取图像数据点类型
	GDALRasterBand* preBand = poDataset->GetRasterBand(1); //预读取遥感的第一个波段
	GDALDataType prePixType = preBand->GetRasterDataType();
	prePixSize = GDALGetDataTypeSize(prePixType) / 8; //GDALGetDataTypeSize得到的是bit
	(nBandCount >= 3 && prePixSize == 1) //多于3个波段的图像给用户选择是否合并前三个波段用RGB显示
		//QMessageBox::StandardButton userAnswer = QMessageBox::Yes;
		QMessageBox::StandardButton userAnswer = QMessageBox::question(m_pCanvas,
		tr("Choose image show format"
		tr("%1 bands detected!/nWill you show it in RGB color format using the first three bands?").arg(nBandCount),
		QMessageBox::Yes | QMessageBox::No);
	(userAnswer == QMessageBox::Yes)
		//--------将前三个波段组合成RGB颜色显示, 使用QImage::Format_RGB32格式显示---------
		uchar* pafScanblock = (uchar*)malloc(4 * (nImgSizeX)*(nImgSizeY)); //Format_RGB32每个像素4个字节: RGBA
	//assert(pafScanblock != NULL);
	(pafScanblock == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		delete poDataset; //关闭数据集
	_READ_IMAGE_FILE_OVER;
	memset(pafScanblock, 0x00, 4 * (nImgSizeX)*(nImgSizeY));
	//读取图像
	nB = 0; nB < nBandCount; nB++)
		GDALRasterBand* poBand = poDataset->GetRasterBand(nB + 1); //遥感的一个波段
	assert(poBand != NULL);
	GDALDataType pixType = poBand->GetRasterDataType();
	pixSize = GDALGetDataTypeSize(pixType) / 8; //GDALGetDataTypeSize得到的是bit
	assert(pixSize == 1); //彩色图的每个通道的size应该是1个字节
	//用RasterIO()读取图像数据
	//poBand->RasterIO(GF_Read, 0, 0, nImgSizeX, nImgSizeY,
	//  pafScanblock+nB, nImgSizeX, nImgSizeY, pixType, 4, 0);
	G_imageReadMutex.lock();
	m_pBand_GDAL_ = poBand;
	m_nXSize_GDAL_ = nImgSizeX;
	m_nYSize_GDAL_ = nImgSizeY;
	m_pDataBufStart_GDAL_ = pafScanblock + nB; //注意！
	m_nBufXSize_GDAL_ = nImgSizeX;
	m_nBufYSize_GDAL_ = nImgSizeY;
	m_eBufDataType_GDAL_ = pixType;
	m_nBufPixSpace_GDAL_ = 4; //注意！
	m_nBufLineSpace_GDAL_ = 0;
	start(); //开始新线程
	G_imageReadMutex.unlock();
	while->isRunning())
		progressBarValue += 8;
	progress.setValue(progressBarValue % 100);
	QThread::msleep(1000); //注意：这里是让主线程sleep，而不是新线程
	//G_imageReadComplete.wait(&G_imageReadMutex);
	//progressBarValue += 20;
	//progress.setValue(progressBarValue % 100);
	//G_imageReadMutex.unlock();
	//组合成彩色QImage
	QImage* pQImage = NULL;
	pQImage = QImage(pafScanblock, nImgSizeX, nImgSizeY, QImage::Format_RGB32); //注意：QImage: each scanline of data in the image must also be 32-bit aligned
	//assert(pQImage != NULL);
	(pQImage == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		free(pafScanblock);
	delete poDataset; //关闭数据集
	_READ_IMAGE_FILE_OVER;
	//创建图层
	RSI_ImageLayer* pImageLayer = RSI_ImageLayer(*pQImage);
	(pImageLayer == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		free(pafScanblock);
	delete pQImage;
	delete poDataset; //关闭数据集
	_READ_IMAGE_FILE_OVER;
	pImageLayer->setLayerName(tr("3 bands composited color image") + QString() + m_imageFileName + QString(
		pImageLayer->setImageLayerInfo(rsiFileInfo);
	pImageLayer->setGeoTransform(adfGeoTransform);
	pImageLayer->setResolutionX(xMeterPerPix);
	pImageLayer->setResolutionY(yMeterPerPix);
	pImageLayer->setBufferToBeFree(pafScanblock); //记录到m_bufferToBeFree中，待关闭图像时释放buffer内存
	pImageLayer->setVisible(false//默认不显示
		pImageLayer->setFlag(QGraphicsItem::ItemIsMovable, false
		m_pCanvas->AddImageLayer(pImageLayer);
	//释放临时对象
	delete pQImage;
	delete poDataset; //关闭数据集
	_READ_IMAGE_FILE_OVER; //读取完毕直接返回
	//------------按照波段分别读取，每个波段显示成一个灰度图-------------
	//读取图像
	nB = 0; nB < nBandCount; nB++)
		GDALRasterBand* poBand = poDataset->GetRasterBand(nB + 1); //遥感的一个波段
	assert(poBand != NULL);
	GDALDataType pixType = poBand->GetRasterDataType();
	pixSize = GDALGetDataTypeSize(pixType) / 8; //GDALGetDataTypeSize得到的是bit
	//注意：QImage: each scanline of data in the image must also be 32-bit aligned
	nImgSizeXAligned32bit = Get32bitAlignedWidthBytes(nImgSizeX, pixSize) / pixSize; //对齐后的像素宽度
	uchar* pafScanblock = (uchar*)malloc(pixSize*(nImgSizeXAligned32bit)*(nImgSizeY));
	//assert(pafScanblock != NULL);
	(pafScanblock == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		delete poDataset; //关闭数据集
	_READ_IMAGE_FILE_OVER;
	//用RasterIO()读取图像数据
	//poBand->RasterIO(GF_Read, 0, 0, nImgSizeX, nImgSizeY,
	//  pafScanblock, nImgSizeXAligned32bit, nImgSizeY, pixType, 0, 0);
	G_imageReadMutex.lock();
	m_pBand_GDAL_ = poBand;
	m_nXSize_GDAL_ = nImgSizeX;
	m_nYSize_GDAL_ = nImgSizeY;
	m_pDataBufStart_GDAL_ = pafScanblock;
	m_nBufXSize_GDAL_ = nImgSizeXAligned32bit;
	m_nBufYSize_GDAL_ = nImgSizeY;
	m_eBufDataType_GDAL_ = pixType;
	m_nBufPixSpace_GDAL_ = 0;
	m_nBufLineSpace_GDAL_ = 0;
	start(); //开始新线程
	G_imageReadMutex.unlock();
	while->isRunning())
		progressBarValue += 8;
	progress.setValue(progressBarValue % 100);
	QThread::msleep(1000); //注意：这里是让主线程sleep，而不是新线程
	//G_imageReadComplete.wait(&G_imageReadMutex);
	//progressBarValue += 20;
	//progress.setValue(progressBarValue % 100);
	//G_imageReadMutex.unlock();
	//转化为QImage
	QImage* pQImage = CvtGDALBufferToGrayQImage(pafScanblock, nImgSizeX, nImgSizeY, pixType);
	(pQImage == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		free(pafScanblock);
	delete poDataset; //关闭数据集
	_READ_IMAGE_FILE_OVER;
	//创建图层
	RSI_ImageLayer* pImageLayer = RSI_ImageLayer(*pQImage);
	(pImageLayer == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		free(pafScanblock);
	delete pQImage;
	delete poDataset; //关闭数据集
	_READ_IMAGE_FILE_OVER;
	pImageLayer->setLayerName(tr("band") + QString().arg(nB + 1) + tr(": pixel type = ") + QString(GDALGetDataTypeName(pixType)) + QString() + QString().arg(pixSize) + tr(" bytes)"
		pImageLayer->setImageLayerInfo(rsiFileInfo);
	pImageLayer->setGeoTransform(adfGeoTransform);
	pImageLayer->setResolutionX(xMeterPerPix);
	pImageLayer->setResolutionY(yMeterPerPix);
	pImageLayer->setBufferToBeFree(pafScanblock); //记录到m_bufferToBeFree中，待关闭图像时释放buffer内存
	pImageLayer->setVisible(false//默认不显示
		pImageLayer->setFlag(QGraphicsItem::ItemIsMovable, false
		m_pCanvas->AddImageLayer(pImageLayer);
	//释放临时对象
	delete pQImage;
	delete poDataset; //关闭数据集
	_READ_IMAGE_FILE_OVER; //读取完毕直接返回
} //end if eOpenDriverType
_READ_IMAGE_FILE_OVER:
return
//将图像宽度字节数32位对齐，返回对齐后的字节数
RSI_ImageReaderThread::Get32bitAlignedWidthBytes(nImgPixWidth, nBytesPerPix)
return ((nImgPixWidth*nBytesPerPix / 4) + (((nImgPixWidth*nBytesPerPix) % 4) == 0 ? 0 : 1)) * 4;
//GDAL buffer 转 QImage
QImage*  RSI_ImageReaderThread::CvtGDALBufferToGrayQImage(uchar* &pafScanblock, nImgSizeX, nImgSizeY, GDALDataType pixType)
//准备颜色表
QVector<QRgb> vcolorTable;
i = 0; i < 256; i++)
	vcolorTable.push_back(qRgb(i, i, i));
pixSize = GDALGetDataTypeSize(pixType) / 8;
nImgSizeXAligned32bit = Get32bitAlignedWidthBytes(nImgSizeX, pixSize) / pixSize; //对齐后的像素宽度
nNewImgSizeXAligned32bit = Get32bitAlignedWidthBytes(nImgSizeX, 1);
//根据pixType转换
(pixType == GDT_Byte)
//do nothing
//add by baolc, 2010-06-10
//将16位数据转为8位灰度显示
uchar* pNew_pafScanblock_8bit = (uchar*)malloc(nNewImgSizeXAligned32bit * nImgSizeY);
(pNew_pafScanblock_8bit == NULL)
QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
return NULL;
(pixType == GDT_UInt16)
CvtImgDataTo8bitGrayImg<ushort>((ushort*)pafScanblock, nImgSizeX, nImgSizeXAligned32bit, nImgSizeY,
pNew_pafScanblock_8bit, nNewImgSizeXAligned32bit, USHRT_MAX, 0);
(pixType == GDT_Int16)
CvtImgDataTo8bitGrayImg<short>((short*)pafScanblock, nImgSizeX, nImgSizeXAligned32bit, nImgSizeY,
pNew_pafScanblock_8bit, nNewImgSizeXAligned32bit, SHRT_MAX, SHRT_MIN);
(pixType == GDT_UInt32)
CvtImgDataTo8bitGrayImg<>((*)pafScanblock, nImgSizeX, nImgSizeXAligned32bit, nImgSizeY,
pNew_pafScanblock_8bit, nNewImgSizeXAligned32bit, UINT_MAX, 0);
(pixType == GDT_Int32)
CvtImgDataTo8bitGrayImg<unsigned >((unsigned *)pafScanblock, nImgSizeX, nImgSizeXAligned32bit, nImgSizeY,
pNew_pafScanblock_8bit, nNewImgSizeXAligned32bit, INT_MAX, INT_MIN);
(pixType == GDT_Float32)
CvtImgDataTo8bitGrayImg<float>((float*)pafScanblock, nImgSizeX, nImgSizeXAligned32bit, nImgSizeY,
pNew_pafScanblock_8bit, nNewImgSizeXAligned32bit, FLT_MAX, FLT_MIN);
(pixType == GDT_Float64)
CvtImgDataTo8bitGrayImg<double>((double*)pafScanblock, nImgSizeX, nImgSizeXAligned32bit, nImgSizeY,
pNew_pafScanblock_8bit, nNewImgSizeXAligned32bit, DBL_MAX, DBL_MIN);
free(pafScanblock); //释放原来的内存
pafScanblock = pNew_pafScanblock_8bit; //指向新内存，注意，因为参数传进来是引用，所以可以改变其指向
QImage* pQImage = QImage(pafScanblock, nImgSizeX, nImgSizeY, QImage::Format_Indexed8); //注意：QImage: each scanline of data in the image must also be 32-bit aligned
(pQImage == NULL)
QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
pQImage->setColorTable(vcolorTable);
return pQImage;
//将非8位的灰度图转换为8位灰度图
templateclass T >
RSI_ImageReaderThread::CvtImgDataTo8bitGrayImg(T* pOrigData, nImgSizeX, nImgSizeXAligned32bit, nImgSizeY, uchar* pNew8bitImgBuf, nNewImgSizeXAligned32bit, T maxVal, T minVal)
oldIdxPos = 0;
newIdxPos = 0;
T minPixVal = maxVal;
T maxPixVal = minVal;
T curPixVal;
//找到最大值和最小值
niY = 0; niY < nImgSizeY; niY++)
	niX = 0; niX < nImgSizeX; niX++)
	oldIdxPos = niY * nImgSizeXAligned32bit + niX;
curPixVal = pOrigData[oldIdxPos];
(curPixVal > maxPixVal)
maxPixVal = curPixVal;
(curPixVal < minPixVal)
	minPixVal = curPixVal;
//根据最大值最小值进行归一化
double pixValRange = maxPixVal - minPixVal;
(pixValRange == 0)
pixValRange = 1;
niY = 0; niY < nImgSizeY; niY++)
	niX = 0; niX < nImgSizeX; niX++)
	oldIdxPos = niY * nImgSizeXAligned32bit + niX;
newIdxPos = niY * nNewImgSizeXAligned32bit + niX;
curPixVal = pOrigData[oldIdxPos];
pNew8bitImgBuf[newIdxPos] = (uchar)((curPixVal - minPixVal) / pixValRange * 255);