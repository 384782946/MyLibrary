
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
		//�̺߳���
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
	//��ͼ������ʹ��GDAL��OpenCV��ȡͼ���ļ�
	RSI_ImageReaderThread::ReadFile(const QString& imageFileName)
		m_imageFileName = imageFileName;
	(m_imageFileName.isEmpty() || m_pCanvas == NULL)
		return
		//ѡ���ͼ���������ʽ: GDAL or OpenCV
		const QString strOpenCV = tr("OpenCV"
		const QString strGDAL = tr("GDAL"
		IMG_OPEN_DRIVER_T  eOpenDriverType;
	QStringList items; //for input dialog
	items << strOpenCV << strGDAL;
	curDriverItemIdx = 0;
	//�ȸ�����չ���Ƽ��򿪷�ʽ
	QString fileExtName = m_imageFileName.right(3).toLower();
	(fileExtName == QString("bmp"
		|| fileExtName == QString("png"
		|| fileExtName == QString("jpg"
		|| fileExtName == QString("peg"
		|| fileExtName == QString("jpe"
		|| fileExtName == QString("ppm"
		|| fileExtName == QString("pbm"
		|| fileExtName == QString("pgm"
		//����ǳ��ø�ʽ���Ƽ�ʹ��OpenCV��: (*.bmp *.png *.jpg *.jpeg *.jpe *.ppm *.pgm *.pbm)
		eOpenDriverType = RSI_USE_OPENCV; //������չ�����Ƽ�ʹ�����ִ򿪷�ʽ
	curDriverItemIdx = 0;
	eOpenDriverType = RSI_USE_GDAL;
	curDriverItemIdx = 1;
	//�����Ի������û�ѡ��ʹ������������ʽ��ͼ��
	dlgOk;
	QString itemSel = QInputDialog::getItem(m_pCanvas, tr("Please select..."
		tr("Driver for image opener:"), items, curDriverItemIdx, false, &dlgOk);
	(dlgOk && !itemSel.isEmpty())
		(itemSel == strOpenCV)
		eOpenDriverType = RSI_USE_OPENCV;
	(itemSel == strGDAL)
		eOpenDriverType = RSI_USE_GDAL;
	return
		//������
		progressBarValue = 2;
	QProgressDialog progress(tr("Reading image file..."), tr("Abort"), 0, 100, m_pCanvas);
	QProgressBar prgBar(&progress);
	prgBar.setTextVisible(false//����ʾ�ٷֱ�
		progress.setBar(&prgBar);
	progress.setWindowModality(Qt::ApplicationModal);
	progress.setCancelButton(NULL);
	progress.setWindowTitle(tr("Please wait..."
		progress.setValue(progressBarValue);
	//use GDAL and OpenCV open image files
	(eOpenDriverType == RSI_USE_OPENCV)
		//��ͼ���ļ�
		IplImage* pIplImg = NULL; //����IplImageָ��
	((pIplImg = cvLoadImage(m_imageFileName.toStdString().c_str(), 1)) == 0) //ʹ��OpenCV highgui��ͼ���ļ�
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed!"), tr(
		return
		//��ȡͼ����Ϣ
		QString rsiFileInfo; //ͼ����Ϣ
	QString tempStr;
	tempStr = tr("Size is %1 x %2 x [%3(bits) x %4(channels)]/n"
		.arg(pIplImg->width)
		.arg(pIplImg->height)
		.arg(pIplImg->depth)
		.arg(pIplImg->nChannels);
	rsiFileInfo += tempStr;
	uchar* imageDataBuffer = NULL;
	QImage* pQImage = IplImageToQImage(pIplImg, &imageDataBuffer); //ת����QImage
	//����ͼ��
	RSI_ImageLayer* pImageLayer = RSI_ImageLayer(*pQImage);
	pImageLayer->setLayerName(m_imageFileName);
	pImageLayer->setImageLayerInfo(rsiFileInfo);
	pImageLayer->setBufferToBeFree(imageDataBuffer); //��¼��m_bufferToBeFree�У����ر�ͼ��ʱ�ͷ�buffer�ڴ�
	pImageLayer->setVisible(false//Ĭ�ϲ���ʾ
		pImageLayer->setFlag(QGraphicsItem::ItemIsMovable, false
		m_pCanvas->AddImageLayer(pImageLayer);
	//�ͷ���ʱ�ڴ�
	cvReleaseImage(&pIplImg);
	delete pQImage;
	_READ_IMAGE_FILE_OVER; //��ȡ���ֱ�ӷ���
	(eOpenDriverType == RSI_USE_GDAL)
		//�����ң��ͼ���ʽ����ʹ��GDAL��
		GDALAllRegister();
	GDALDataset* poDataset;  //GDAL���ݼ�
	poDataset = (GDALDataset*)GDALOpen(m_imageFileName.toStdString().c_str(), GA_ReadOnly);
	(poDataset == NULL)
		//QMessageBox::warning(NULL, tr("Failed"), tr("GDAL Open Image File Failed!"), tr("OK"));
		return
		//��ȡң��ͼ����Ϣ
		QString rsiFileInfo; //ң��ͼ����Ϣ
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
����ο�����ϵͳ:
	The returned string defines the projection coordinate system of the image in OpenGIS WKT format.
		double adfGeoTransform[6];
	QString prjRefStr(poDataset->GetProjectionRef());
	(prjRefStr.isEmpty())
		prjRefStr = tr("none"//������ֶ�Ϊ�գ���ôͶӰ��ʽ������GCPs
		//�ǿ�ʱ�Ŷ�ȡ����任��Ϣ
		tempStr = tr("Projection(WKT format) is '%1'/n").arg(prjRefStr);
	rsiFileInfo += tempStr;
	//�ǿ�ʱ�Ŷ�ȡ����任��Ϣ
	�������任��Ϣ��
		adfGeoTransform[0] // top left x  
		adfGeoTransform[1] // w-e pixel resolution  
		adfGeoTransform[2] // rotation, 0 if image is "north up"  
		adfGeoTransform[3] // top left y  
		adfGeoTransform[4] // rotation, 0 if image is "north up"  
		adfGeoTransform[5] // n-s pixel resolution  
		����ĳһ��ĵ�������������¼��㣺
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
	//��ȡGCPsͶӰ��ʽ
	QString prjGCPStr(poDataset->GetGCPProjection());
	(prjGCPStr.isEmpty())
		prjGCPStr = tr("none"
		tempStr = tr("GCPs Prj is '%1'/n").arg(prjGCPStr);
	rsiFileInfo += tempStr;
	nGCPs = poDataset->GetGCPCount(); //���GCP���Ƶ�ĸ���
	const GDAL_GCP* pGCPs = poDataset->GetGCPs(); //���GCP���Ƶ�
	tempStr = tr("Number of GCPs: %1/n").arg(nGCPs);
	rsiFileInfo += tempStr;
	//��GCPs��÷���任����
	GDALGCPsToGeoTransform(nGCPs, pGCPs, adfGeoTransform, TRUE);
	tempStr = tr("Origin Cords = (%1,%2)/n"
		.arg(adfGeoTransform[0], 0,
		.arg(adfGeoTransform[3], 0,
		rsiFileInfo += tempStr;
	tempStr = tr("Pixel Size(degrees) = (%1,%2)/n"
		.arg(adfGeoTransform[1], 0,
		.arg(adfGeoTransform[5], 0,
		rsiFileInfo += tempStr;
	//����adfGeoTransform����ÿ�����ش�������׾���
	double oriCordL = adfGeoTransform[0];
	double oriCordB = adfGeoTransform[3];
	double tempL, tempB;
	GDALApplyGeoTransform(adfGeoTransform, 100, 0, &tempL, &tempB); //����(100,0)���ش���L,B����
	double tempDist = CalcDistanceByLB(oriCordL, oriCordB, tempL, tempB);
	double xMeterPerPix = tempDist * 1000.0 / 100.0; //��λΪ��
	GDALApplyGeoTransform(adfGeoTransform, 0, 100, &tempL, &tempB); //����(0,100)���ش���L,B����
	tempDist = CalcDistanceByLB(oriCordL, oriCordB, tempL, tempB);
	double yMeterPerPix = tempDist * 1000.0 / 100.0; //��λΪ��
	tempStr = tr("Pixel Size(meters) = (%1,%2)/n"
		.arg(xMeterPerPix, 0,
		.arg(yMeterPerPix, 0,
		rsiFileInfo += tempStr;
	//��ȡͼ���С
	nBandCount = poDataset->GetRasterCount();
	nImgSizeX = poDataset->GetRasterXSize();
	nImgSizeY = poDataset->GetRasterYSize();
	//��ȡͼ�����ݵ�����
	GDALRasterBand* preBand = poDataset->GetRasterBand(1); //Ԥ��ȡң�еĵ�һ������
	GDALDataType prePixType = preBand->GetRasterDataType();
	prePixSize = GDALGetDataTypeSize(prePixType) / 8; //GDALGetDataTypeSize�õ�����bit
	(nBandCount >= 3 && prePixSize == 1) //����3�����ε�ͼ����û�ѡ���Ƿ�ϲ�ǰ����������RGB��ʾ
		//QMessageBox::StandardButton userAnswer = QMessageBox::Yes;
		QMessageBox::StandardButton userAnswer = QMessageBox::question(m_pCanvas,
		tr("Choose image show format"
		tr("%1 bands detected!/nWill you show it in RGB color format using the first three bands?").arg(nBandCount),
		QMessageBox::Yes | QMessageBox::No);
	(userAnswer == QMessageBox::Yes)
		//--------��ǰ����������ϳ�RGB��ɫ��ʾ, ʹ��QImage::Format_RGB32��ʽ��ʾ---------
		uchar* pafScanblock = (uchar*)malloc(4 * (nImgSizeX)*(nImgSizeY)); //Format_RGB32ÿ������4���ֽ�: RGBA
	//assert(pafScanblock != NULL);
	(pafScanblock == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		delete poDataset; //�ر����ݼ�
	_READ_IMAGE_FILE_OVER;
	memset(pafScanblock, 0x00, 4 * (nImgSizeX)*(nImgSizeY));
	//��ȡͼ��
	nB = 0; nB < nBandCount; nB++)
		GDALRasterBand* poBand = poDataset->GetRasterBand(nB + 1); //ң�е�һ������
	assert(poBand != NULL);
	GDALDataType pixType = poBand->GetRasterDataType();
	pixSize = GDALGetDataTypeSize(pixType) / 8; //GDALGetDataTypeSize�õ�����bit
	assert(pixSize == 1); //��ɫͼ��ÿ��ͨ����sizeӦ����1���ֽ�
	//��RasterIO()��ȡͼ������
	//poBand->RasterIO(GF_Read, 0, 0, nImgSizeX, nImgSizeY,
	//  pafScanblock+nB, nImgSizeX, nImgSizeY, pixType, 4, 0);
	G_imageReadMutex.lock();
	m_pBand_GDAL_ = poBand;
	m_nXSize_GDAL_ = nImgSizeX;
	m_nYSize_GDAL_ = nImgSizeY;
	m_pDataBufStart_GDAL_ = pafScanblock + nB; //ע�⣡
	m_nBufXSize_GDAL_ = nImgSizeX;
	m_nBufYSize_GDAL_ = nImgSizeY;
	m_eBufDataType_GDAL_ = pixType;
	m_nBufPixSpace_GDAL_ = 4; //ע�⣡
	m_nBufLineSpace_GDAL_ = 0;
	start(); //��ʼ���߳�
	G_imageReadMutex.unlock();
	while->isRunning())
		progressBarValue += 8;
	progress.setValue(progressBarValue % 100);
	QThread::msleep(1000); //ע�⣺�����������߳�sleep�����������߳�
	//G_imageReadComplete.wait(&G_imageReadMutex);
	//progressBarValue += 20;
	//progress.setValue(progressBarValue % 100);
	//G_imageReadMutex.unlock();
	//��ϳɲ�ɫQImage
	QImage* pQImage = NULL;
	pQImage = QImage(pafScanblock, nImgSizeX, nImgSizeY, QImage::Format_RGB32); //ע�⣺QImage: each scanline of data in the image must also be 32-bit aligned
	//assert(pQImage != NULL);
	(pQImage == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		free(pafScanblock);
	delete poDataset; //�ر����ݼ�
	_READ_IMAGE_FILE_OVER;
	//����ͼ��
	RSI_ImageLayer* pImageLayer = RSI_ImageLayer(*pQImage);
	(pImageLayer == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		free(pafScanblock);
	delete pQImage;
	delete poDataset; //�ر����ݼ�
	_READ_IMAGE_FILE_OVER;
	pImageLayer->setLayerName(tr("3 bands composited color image") + QString() + m_imageFileName + QString(
		pImageLayer->setImageLayerInfo(rsiFileInfo);
	pImageLayer->setGeoTransform(adfGeoTransform);
	pImageLayer->setResolutionX(xMeterPerPix);
	pImageLayer->setResolutionY(yMeterPerPix);
	pImageLayer->setBufferToBeFree(pafScanblock); //��¼��m_bufferToBeFree�У����ر�ͼ��ʱ�ͷ�buffer�ڴ�
	pImageLayer->setVisible(false//Ĭ�ϲ���ʾ
		pImageLayer->setFlag(QGraphicsItem::ItemIsMovable, false
		m_pCanvas->AddImageLayer(pImageLayer);
	//�ͷ���ʱ����
	delete pQImage;
	delete poDataset; //�ر����ݼ�
	_READ_IMAGE_FILE_OVER; //��ȡ���ֱ�ӷ���
	//------------���ղ��ηֱ��ȡ��ÿ��������ʾ��һ���Ҷ�ͼ-------------
	//��ȡͼ��
	nB = 0; nB < nBandCount; nB++)
		GDALRasterBand* poBand = poDataset->GetRasterBand(nB + 1); //ң�е�һ������
	assert(poBand != NULL);
	GDALDataType pixType = poBand->GetRasterDataType();
	pixSize = GDALGetDataTypeSize(pixType) / 8; //GDALGetDataTypeSize�õ�����bit
	//ע�⣺QImage: each scanline of data in the image must also be 32-bit aligned
	nImgSizeXAligned32bit = Get32bitAlignedWidthBytes(nImgSizeX, pixSize) / pixSize; //���������ؿ��
	uchar* pafScanblock = (uchar*)malloc(pixSize*(nImgSizeXAligned32bit)*(nImgSizeY));
	//assert(pafScanblock != NULL);
	(pafScanblock == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		delete poDataset; //�ر����ݼ�
	_READ_IMAGE_FILE_OVER;
	//��RasterIO()��ȡͼ������
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
	start(); //��ʼ���߳�
	G_imageReadMutex.unlock();
	while->isRunning())
		progressBarValue += 8;
	progress.setValue(progressBarValue % 100);
	QThread::msleep(1000); //ע�⣺�����������߳�sleep�����������߳�
	//G_imageReadComplete.wait(&G_imageReadMutex);
	//progressBarValue += 20;
	//progress.setValue(progressBarValue % 100);
	//G_imageReadMutex.unlock();
	//ת��ΪQImage
	QImage* pQImage = CvtGDALBufferToGrayQImage(pafScanblock, nImgSizeX, nImgSizeY, pixType);
	(pQImage == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		free(pafScanblock);
	delete poDataset; //�ر����ݼ�
	_READ_IMAGE_FILE_OVER;
	//����ͼ��
	RSI_ImageLayer* pImageLayer = RSI_ImageLayer(*pQImage);
	(pImageLayer == NULL)
		QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
		free(pafScanblock);
	delete pQImage;
	delete poDataset; //�ر����ݼ�
	_READ_IMAGE_FILE_OVER;
	pImageLayer->setLayerName(tr("band") + QString().arg(nB + 1) + tr(": pixel type = ") + QString(GDALGetDataTypeName(pixType)) + QString() + QString().arg(pixSize) + tr(" bytes)"
		pImageLayer->setImageLayerInfo(rsiFileInfo);
	pImageLayer->setGeoTransform(adfGeoTransform);
	pImageLayer->setResolutionX(xMeterPerPix);
	pImageLayer->setResolutionY(yMeterPerPix);
	pImageLayer->setBufferToBeFree(pafScanblock); //��¼��m_bufferToBeFree�У����ر�ͼ��ʱ�ͷ�buffer�ڴ�
	pImageLayer->setVisible(false//Ĭ�ϲ���ʾ
		pImageLayer->setFlag(QGraphicsItem::ItemIsMovable, false
		m_pCanvas->AddImageLayer(pImageLayer);
	//�ͷ���ʱ����
	delete pQImage;
	delete poDataset; //�ر����ݼ�
	_READ_IMAGE_FILE_OVER; //��ȡ���ֱ�ӷ���
} //end if eOpenDriverType
_READ_IMAGE_FILE_OVER:
return
//��ͼ�����ֽ���32λ���룬���ض������ֽ���
RSI_ImageReaderThread::Get32bitAlignedWidthBytes(nImgPixWidth, nBytesPerPix)
return ((nImgPixWidth*nBytesPerPix / 4) + (((nImgPixWidth*nBytesPerPix) % 4) == 0 ? 0 : 1)) * 4;
//GDAL buffer ת QImage
QImage*  RSI_ImageReaderThread::CvtGDALBufferToGrayQImage(uchar* &pafScanblock, nImgSizeX, nImgSizeY, GDALDataType pixType)
//׼����ɫ��
QVector<QRgb> vcolorTable;
i = 0; i < 256; i++)
	vcolorTable.push_back(qRgb(i, i, i));
pixSize = GDALGetDataTypeSize(pixType) / 8;
nImgSizeXAligned32bit = Get32bitAlignedWidthBytes(nImgSizeX, pixSize) / pixSize; //���������ؿ��
nNewImgSizeXAligned32bit = Get32bitAlignedWidthBytes(nImgSizeX, 1);
//����pixTypeת��
(pixType == GDT_Byte)
//do nothing
//add by baolc, 2010-06-10
//��16λ����תΪ8λ�Ҷ���ʾ
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
free(pafScanblock); //�ͷ�ԭ�����ڴ�
pafScanblock = pNew_pafScanblock_8bit; //ָ�����ڴ棬ע�⣬��Ϊ���������������ã����Կ��Ըı���ָ��
QImage* pQImage = QImage(pafScanblock, nImgSizeX, nImgSizeY, QImage::Format_Indexed8); //ע�⣺QImage: each scanline of data in the image must also be 32-bit aligned
(pQImage == NULL)
QMessageBox::warning(m_pCanvas, tr("Failed"), tr("Open Image File Failed! Out of memory!"), tr(
pQImage->setColorTable(vcolorTable);
return pQImage;
//����8λ�ĻҶ�ͼת��Ϊ8λ�Ҷ�ͼ
templateclass T >
RSI_ImageReaderThread::CvtImgDataTo8bitGrayImg(T* pOrigData, nImgSizeX, nImgSizeXAligned32bit, nImgSizeY, uchar* pNew8bitImgBuf, nNewImgSizeXAligned32bit, T maxVal, T minVal)
oldIdxPos = 0;
newIdxPos = 0;
T minPixVal = maxVal;
T maxPixVal = minVal;
T curPixVal;
//�ҵ����ֵ����Сֵ
niY = 0; niY < nImgSizeY; niY++)
	niX = 0; niX < nImgSizeX; niX++)
	oldIdxPos = niY * nImgSizeXAligned32bit + niX;
curPixVal = pOrigData[oldIdxPos];
(curPixVal > maxPixVal)
maxPixVal = curPixVal;
(curPixVal < minPixVal)
	minPixVal = curPixVal;
//�������ֵ��Сֵ���й�һ��
double pixValRange = maxPixVal - minPixVal;
(pixValRange == 0)
pixValRange = 1;
niY = 0; niY < nImgSizeY; niY++)
	niX = 0; niX < nImgSizeX; niX++)
	oldIdxPos = niY * nImgSizeXAligned32bit + niX;
newIdxPos = niY * nNewImgSizeXAligned32bit + niX;
curPixVal = pOrigData[oldIdxPos];
pNew8bitImgBuf[newIdxPos] = (uchar)((curPixVal - minPixVal) / pixValRange * 255);