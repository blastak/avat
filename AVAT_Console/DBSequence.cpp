#include "DBSequence.h"
#include <io.h>

#define LOG_OUT(fmt) printf( "[%s:%d] %s\n",__FUNCTION__,__LINE__,fmt)

//////////////////////////////////////////////////////////////////////////
// CImgSeqInfo Implementation

void CImgSeq::UnloadDB()
{
	m_strPathAbsolute.clear();
	m_strImageFileList.clear();
	m_nFrameTotal = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nChannel = 0;
}

bool CImgSeq::LoadDB(string strFolderPath)
{
	m_strPathAbsolute = ExtractFolderAbsPath(strFolderPath);
	vector<string> strFileList = LoadFileList(m_strPathAbsolute);
	if(!strFileList.size())
	{
		LOG_OUT("there is no file or folder..\n");
		UnloadDB();
		return false;
	}

	m_strImageFileList = SelectImageFromFiles(strFileList);
	m_nFrameTotal = m_strImageFileList.size();

	if(m_nFrameTotal == 0)
	{
		LOG_OUT("there is no IMAGE file..\n");
		UnloadDB();
		return false;
	}

	Mat imgTemp = imread(m_strPathAbsolute+m_strImageFileList[0]);
	m_nHeight = imgTemp.rows;
	m_nWidth = imgTemp.cols;
	m_nChannel = imgTemp.channels();
	imgTemp.release();

	if(m_nHeight <= 0 || m_nWidth <= 0 || m_nChannel <= 0)
	{
		LOG_OUT("IMAGE file load error..\n");
		UnloadDB();
		return false;
	}

	return true;
}

string CImgSeq::ExtractFolderAbsPath( string strFolderPath ) const
{
	if(strFolderPath.size())
		if (strFolderPath[strFolderPath.size()-1] != '/')
			strFolderPath.push_back('/');
	char szAbsPath[_MAX_PATH];
	_fullpath(szAbsPath,strFolderPath.c_str(),_MAX_PATH);
	return szAbsPath;
}

vector<string> CImgSeq::LoadFileList( string strFolderPath ) const
{
	vector<string> vecRetVal;
	string strSingle;

	_finddata_t fd;
	string strSearch = strFolderPath + "*.*";

	long hFile = _findfirst(strSearch.c_str(), &fd);
	if(hFile == -1)
	{
		LOG_OUT("folder name error..\n");
	}
	else
	{
		do
		{
			strSingle = fd.name;
			vecRetVal.push_back(strSingle);
		} while (_findnext(hFile,&fd) != -1);

		_findclose(hFile);

		vecRetVal.erase(vecRetVal.begin());
		vecRetVal.erase(vecRetVal.begin());
	}
	return vecRetVal;
}

vector<string> CImgSeq::SelectImageFromFiles( vector<string>& strFileList ) const
{
	bool bFirst = false;
	string strExtFirst;
	for (size_t i=0; i<strFileList.size(); i++)
	{
		int idxPeriod = strFileList[i].rfind('.');
		string strExt = strFileList[i].substr(idxPeriod+1);
		if(!bFirst)
		{
			for (size_t j=0; j<m_strImgExtList.size(); j++)
			{
				if(strExt == m_strImgExtList[j])
				{
					strExtFirst = m_strImgExtList[j];
					bFirst = true;
					break;
				}
			}
		}
		
		if(strExt != strExtFirst)
		{
			strFileList.erase(strFileList.begin()+i);
			i--;
		}
	}
	return strFileList;
}

bool CImgSeq::ReadFrame( Mat& img, int nFrame )
{
	img = imread(m_strPathAbsolute + m_strImageFileList[nFrame]);
	return !img.empty();
}

//////////////////////////////////////////////////////////////////////////
// CVideoInfo Implementation

void CVideoSeq::UnloadDB()
{
	if(m_capVid.isOpened())
		m_capVid.release();
	m_strPathAbsolute.clear();
	m_strVideoFile.clear();
	m_nFrameTotal = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nChannel = 0;
}

bool CVideoSeq::LoadDB(string strFileName)
{
	ExtractAbsPathAndFileName(strFileName, m_strPathAbsolute, m_strVideoFile);

	m_capVid.open(m_strPathAbsolute+m_strVideoFile);
	bool bRetVal = m_capVid.isOpened();
	if(bRetVal)
	{
		m_nFrameTotal = (int)(m_capVid.get(CV_CAP_PROP_FRAME_COUNT));
		m_nWidth = (int)(m_capVid.get(CV_CAP_PROP_FRAME_WIDTH));
		m_nHeight = (int)(m_capVid.get(CV_CAP_PROP_FRAME_HEIGHT));
	}
	else
	{
		LOG_OUT("there is no file..\n");
		UnloadDB();
	}

	return bRetVal;
}

void CVideoSeq::ExtractAbsPathAndFileName( string strFileName, string& strAbsPath, string& strVideoFile )
{
	char szAbsPath[_MAX_PATH];
	_fullpath(szAbsPath,strFileName.c_str(),_MAX_PATH);

	strAbsPath = szAbsPath;
	int nPosSlash = strAbsPath.rfind('\\'); // linux에서 다를것이다.
	strVideoFile = strAbsPath.substr(nPosSlash+1);
	strAbsPath = strAbsPath.substr(0,nPosSlash+1);
}

bool CVideoSeq::ReadFrame( Mat& img, int nFrame )
{
	m_capVid.set(CV_CAP_PROP_POS_FRAMES,(double)nFrame);
	return m_capVid.read(img);
}