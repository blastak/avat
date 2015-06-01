#include "VideoPlayer.h"

#define LOG_OUT(fmt) printf( "[%s:%d] %s\n",__FUNCTION__,__LINE__,fmt)

CVideoPlayer::CVideoPlayer(void): m_nCurrFrame(0)
{
	m_eDBmode = DB_UNDEFINED;
	m_objDBSeq = NULL;
}

CVideoPlayer::~CVideoPlayer(void)
{
	UnloadDBFile();
}

bool CVideoPlayer::UnloadDBFile()
{
	m_eDBmode = DB_UNDEFINED;
	if(m_objDBSeq != NULL)
	{
		delete m_objDBSeq;
		m_objDBSeq = NULL;
	}
	return true;
}

bool CVideoPlayer::LoadDBFile( int eImageOrVideo, string strPathOrFileName )
{
	bool retVal = true;
	if(m_objDBSeq == NULL)
	{
		retVal = false;
		m_eDBmode = eImageOrVideo;
		switch(eImageOrVideo)
		{
		case DB_IMAGE:
			m_objDBSeq = new CImgSeq();
			break;
		case DB_VIDEO:
			m_objDBSeq = new CVideoSeq();
			break;
		default:
			m_eDBmode = DB_UNDEFINED;
			LOG_OUT("Argument [eImageOrVideo] can be DB_IMAGE or DB_VIDEO...\n");
			break;
		}

		if(m_objDBSeq)
		{
			if(!m_objDBSeq->LoadDB(strPathOrFileName))
			{
				UnloadDBFile();
				retVal = false;
			}
			else
			{
				m_nCurrFrame = 0;
				m_nTotalFrame = m_objDBSeq->GetTotalFrame();
				m_strPathAbsolute = m_objDBSeq->GetPathAbsolute();
				m_sizeFrame = m_objDBSeq->GetFrameSize();
				retVal = true;
			}
		}
	}
	else
	{
		LOG_OUT("Run UnloadDBFile() first...\n");
	}

	return retVal;
}

bool CVideoPlayer::ReadCurrFrame( Mat& img )
{
	if(!m_objDBSeq)
		return false;

	return m_objDBSeq->ReadFrame(img, m_nCurrFrame);
}

bool CVideoPlayer::GoToSpecificFrame( int nSpecFrame )
{
	if(nSpecFrame < 0 || nSpecFrame >= m_nTotalFrame)
	{
		char szErr[100];
		sprintf_s(szErr, "frame is out-of-range : it should be 0-%d..\n",m_nTotalFrame-1);
		LOG_OUT(szErr);
		return false;
	}

	m_nCurrFrame = nSpecFrame;
	return true;
}

bool CVideoPlayer::GoToNextFrame()
{
	return GoToSpecificFrame(m_nCurrFrame+1);
}

bool CVideoPlayer::GoToPrevFrame()
{
	return GoToSpecificFrame(m_nCurrFrame-1);
}

bool CVideoPlayer::operator++(int)
{
	return GoToNextFrame();
}

bool CVideoPlayer::operator--(int)
{
	return GoToPrevFrame();
}

bool CVideoPlayer::operator>>( Mat& img )
{
	return ReadCurrFrame(img);
}
