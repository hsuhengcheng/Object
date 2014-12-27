#pragma once
//OpenNI Header
#include "OpenNI.h"
//OpenCV Header
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace ObjectRecognition {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	//namespace
	using namespace std;
	using namespace openni;

	/// <summary>
	/// Form1 ���K�n
	///
	/// ĵ�i: �p�G�z�ܧ�o�����O���W�١A�N�����ܧ�P�o�����O�Ҩ̾ڤ��Ҧ� .resx �ɮ����p��
	///          Managed �귽�sĶ���u�㪺 'Resource File Name' �ݩʡC
	///          �_�h�A�o�ǳ]�p�u��
	///          �N�L�k�P�o�Ӫ�����p����a�y�t�Ƹ귽
	///          ���T���ʡC
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			if( OpenNI::initialize() != STATUS_OK )
			{
				MessageBox::Show("OpenNI Initial Error");
			}
			//open the device (kinect)
			//Device mDevice;
			//mDevice	=	(new openni::Device());
			mDevice = new Device();
			//Device	md = &mDevice;
			if( mDevice->open( ANY_DEVICE ) != STATUS_OK )
			{
				MessageBox::Show("Can't Open Device");
			}
			//open the depth stream
			mDepthStream = new VideoStream();
			if( mDevice->hasSensor( SENSOR_DEPTH ) )
			{
				if( mDepthStream->create( *mDevice, SENSOR_DEPTH ) == STATUS_OK )
				{

					//setting video mode
					VideoMode mMode;
					mMode.setResolution( 640, 480 );
					mMode.setFps( 30 );
					mMode.setPixelFormat( PIXEL_FORMAT_DEPTH_1_MM );

					if( mDepthStream->setVideoMode( mMode) != STATUS_OK )
					{
						MessageBox::Show("Can't apply VideoMode");
					}
				}
				else
				{
					MessageBox::Show("Can't create depth stream on device");
				}
			}
			else
			{
				MessageBox::Show("ERROR: This device does not have depth sensor");
			}
			mColorStream = new VideoStream();
			if( mDevice->hasSensor( SENSOR_COLOR ) )
			{
				if( mColorStream->create( *mDevice, SENSOR_COLOR ) == STATUS_OK )
				//if(1)
				{
					//setting video mode
					VideoMode mMode;
					mMode.setResolution( 640, 480 );
					mMode.setFps( 30 );
					mMode.setPixelFormat( PIXEL_FORMAT_RGB888 );

					if( mColorStream->setVideoMode( mMode) != STATUS_OK )
					{
						MessageBox::Show("Can't apply VideoMode");
					}

					//image registration 
					if( mDevice->isImageRegistrationModeSupported(
						IMAGE_REGISTRATION_DEPTH_TO_COLOR ) )
					{
						mDevice->setImageRegistrationMode( IMAGE_REGISTRATION_DEPTH_TO_COLOR );
					}
				}
				else
				{
					MessageBox::Show("Can't create color stream on device");
				}
				cv::namedWindow( "Depth Image",  CV_WINDOW_AUTOSIZE );
				cv::namedWindow( "Color Image",  CV_WINDOW_AUTOSIZE );
			}
			mDepthStream->start();
			mColorStream->start();
			start_bgworker();
			if (!this->SHOW_backgroundWorker->IsBusy)
			{
				this->SHOW_backgroundWorker->RunWorkerAsync();
			}
			//
			//TODO: �b���[�J�غc�禡�{���X
			//
		}
		void start_bgworker()
		{
			bg_stop = 0;
		}
		void stop_bgworker()
		{
			bg_stop = 1;
		}

	protected:
		/// <summary>
		/// �M������ϥΤ����귽�C
		/// </summary>
		~Form1()
		{
			stop_bgworker();
			if (components)
			{
				delete components;
			}
		}
	private: System::ComponentModel::BackgroundWorker^  SHOW_backgroundWorker;
	protected: 

	private:
		/// <summary>
		/// �]�p�u��һݪ��ܼơC
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// �����]�p�u��䴩�һݪ���k - �ФŨϥε{���X�s�边�ק�o�Ӥ�k�����e�C
		///
		/// </summary>
		void InitializeComponent(void)
		{
			this->SHOW_backgroundWorker = (gcnew System::ComponentModel::BackgroundWorker());
			this->SuspendLayout();
			// 
			// SHOW_backgroundWorker
			// 
			this->SHOW_backgroundWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &Form1::SHOW_backgroundWorker_DoWork);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 262);
			this->Name = L"Form1";
			this->Text = L"Form1";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->ResumeLayout(false);

		}
#pragma endregion
	public:
	int iMaxDepth;
	int bg_stop;
	Device *mDevice;
	VideoStream *mDepthStream;
	VideoStream *mColorStream;
	private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) 
			 {
				 iMaxDepth = mDepthStream->getMaxPixelValue();
			 }
	private: System::Void SHOW_backgroundWorker_DoWork(System::Object^  sender, System::ComponentModel::DoWorkEventArgs^  e) 
			 {
				 VideoFrameRef  mColorFrame;
				 VideoFrameRef  mDepthFrame;
				 while(1)
				 {

					if( bg_stop )
					{
						return;
					}
					//do something
					if( mColorStream->isValid() )
					{
						//get color frame
						if( mColorStream->readFrame( &mColorFrame ) == STATUS_OK )
						{
							//convert data to OpenCV format
							const cv::Mat mImageRGB(
								mColorFrame.getHeight(), mColorFrame.getWidth(),
								CV_8UC3, (void*)mColorFrame.getData() );
							// 7c. convert form RGB to BGR
							cv::Mat cImageBGR;
							cv::cvtColor( mImageRGB, cImageBGR, CV_RGB2BGR );
							// 7d. show image
							cv::imshow( "Color Image", cImageBGR );
						}
					}
					if( mDepthStream->readFrame( &mDepthFrame ) == STATUS_OK )
					{
						//convert data to OpenCV format
						const cv::Mat mImageDepth(
							mDepthFrame.getHeight(), mDepthFrame.getWidth(),
							CV_16UC1, (void*)mDepthFrame.getData() );
						//re-map depth data [0,Max] to [0,255]
						cv::Mat mScaledDepth;
						mImageDepth.convertTo( mScaledDepth, CV_8U, 255.0 / iMaxDepth );
						//show image
						cv::imshow( "Depth Image", mScaledDepth );
					}
				}
			 }
	};
}

