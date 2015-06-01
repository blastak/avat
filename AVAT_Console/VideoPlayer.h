#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#include "DBSequence.h"

class CVideoPlayer
{
public:
	CVideoPlayer(void);
	virtual ~CVideoPlayer(void);

	enum { DB_UNDEFINED=-1, DB_IMAGE=0, DB_VIDEO=1 };

	bool LoadDBFile( int eImageOrVideo, string strPathOrFileName );
	bool UnloadDBFile();

	bool ReadCurrFrame(Mat& img);
	virtual bool GoToSpecificFrame(int nSpecFrame);
	bool GoToNextFrame();
	bool GoToPrevFrame();

	bool operator++(int);
	bool operator--(int);
	bool operator>>(Mat& img);

protected:
	int m_eDBmode;
	int m_nTotalFrame;
	Size2i m_sizeFrame;
	int m_nCurrFrame;
	string m_strPathAbsolute;
	CDBSequence *m_objDBSeq;
};

