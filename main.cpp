#include <stdio.h>
#include <cv.h>
#include <highgui.h>

int main( int argc, char *argv[] ){

	double snr = 0.002;

	// read file
	if( argc == 1 ){
		printf("usage of wiener\n");
		printf("input_file filter_file output_file\n");
		return 0;
	}else if( argc < 4 ){
		printf("input arguments wrong...\n");
		return 1;
	}

	IplImage *input_file = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	IplImage *filter_file = cvLoadImage(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

	if( input_file == NULL || filter_file == NULL ){
		printf("input files are wrong input_file = 0x%x, output_file = 0x%x\n", input_file, filter_file);
		return 1;
	}

	int height = input_file->height;
	int width = input_file->width;
	int h, w;

	// convert to CvMat
	CvMat *input_real = cvCreateMat( height, width, CV_64FC1 );
	CvMat *input_imag = cvCreateMat( height, width, CV_64FC1 );

	CvMat *output_real = cvCreateMat( height, width, CV_64FC1 );
	CvMat *output_imag = cvCreateMat( height, width, CV_64FC1 );

	CvMat *filter_real = cvCreateMat( height, width, CV_64FC1 );
	CvMat *filter_imag = cvCreateMat( height, width, CV_64FC1 );

	// cvDFTを使うためのデータ変換
	// cvDFTで扱うデータ形式は，double型cvMat,2channel
	CvMat *fft_mat = cvCreateMat( height, width, CV_64FC2 );

	// input file
	cvConvert( input_file, input_real ); // IplImage -> input_real
	cvSetZero( input_imag ); // input_imag = 0
	cvMerge( input_real, input_imag, NULL, NULL, fft_mat ); // ( input_real, input_imag ) -> fft_mat
	cvDFT( fft_mat, fft_mat, CV_DXT_FORWARD , 0);// FFT
	cvSplit( fft_mat, input_real, input_imag, NULL, NULL ); // fft_mat -> ( input_real, input_imag )

	// filter file
	cvSetZero( filter_real );

	// convert filter_file to filter_real
	for( h = 0; h < filter_file->height; ++h ){
		for( w = 0; w < filter_file->width; ++w ){

			int y = h - filter_file->height/2;
			int x = w - filter_file->width/2;
			y += (y<0) ? height : 0;
			x += (x<0) ? width : 0;

			CV_MAT_ELEM( *filter_real, double, y, x ) = CV_IMAGE_ELEM( filter_file, uchar, h, w );
		}
	}
	cvNormalize( filter_real, filter_real, 1.0, 0.0, CV_L1, NULL );
	cvSetZero( filter_imag ); // filter_imag = 0
	cvMerge( filter_real, filter_imag, NULL, NULL, fft_mat ); // ( filter_real, filter_imag ) -> fft_mat
	cvDFT( fft_mat, fft_mat, CV_DXT_FORWARD , 0);// FFT
	cvSplit( fft_mat, filter_real, filter_imag, NULL, NULL ); // fft_mat -> ( filter_real, filter_imag )

	// Wiener calculation
	for( h = 0; h < height; ++h ){
		for( w = 0; w < width; ++w ){
			double a = CV_MAT_ELEM( *input_real, double, h, w );
			double b = CV_MAT_ELEM( *input_imag, double, h, w );
			double c = CV_MAT_ELEM( *filter_real, double, h, w );
			double d = CV_MAT_ELEM( *filter_imag, double, h, w );

			CV_MAT_ELEM( *output_real, double, h, w ) = ( a*c + b*d ) / ( c*c + d*d + snr );
			CV_MAT_ELEM( *output_imag, double, h, w ) = ( b*c - a*d ) / ( c*c + d*d + snr );

		}
	}

	// IFT
	cvMerge( output_real, output_imag, NULL, NULL, fft_mat);
	cvDFT( fft_mat, fft_mat, CV_DXT_INV_SCALE, 0 );
	cvSplit( fft_mat, output_real, output_imag, NULL, NULL );

	// save file
	IplImage *output_file = cvCreateImage( cvSize( width, height), IPL_DEPTH_8U, 1 );
	cvConvert( output_real, output_file);
	cvSaveImage( argv[3], output_file, NULL );

	// cleaning

	return 0;
}

