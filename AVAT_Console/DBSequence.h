#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//* @class  : CImgSeqInfo
//* @author : Hyoungrae Kim
//* @since : 2015-03-30
class CDBSequence
{
public:
	CDBSequence():m_strPathAbsolute(""),
		m_nFrameTotal(0),m_nWidth(0),m_nHeight(0),
		m_nChannel(0) { }
	virtual ~CDBSequence() { }

	virtual bool LoadDB(string) = 0;
	virtual void UnloadDB() = 0;
	virtual bool ReadFrame(Mat& img, int nFrame) = 0;
	inline int GetTotalFrame() const { return m_nFrameTotal; }
	inline string GetPathAbsolute() const { return m_strPathAbsolute; }
	inline Size2i GetFrameSize() const { return Size2i(m_nWidth,m_nHeight); }
	virtual string GetFileName() = 0;

protected:
	string m_strPathAbsolute;
	int m_nFrameTotal;
	int m_nWidth;
	int m_nHeight;
	int m_nChannel;
};


//* @class  : CImgSeqInfo
//* @author : Hyoungrae Kim
//* @date : 2015-03-30
class CImgSeq : public CDBSequence
{
public:
	CImgSeq() {
		string supportExt[] = {"bmp","jpg","png","gif","tif","jpeg","tiff"};
		vector<string> v(supportExt,supportExt+sizeof(supportExt)/sizeof(supportExt[0]));
		m_strImgExtList = v;
	}
	virtual ~CImgSeq() { UnloadDB(); }
	virtual bool LoadDB(string);
	virtual void UnloadDB();
	virtual bool ReadFrame(Mat& img, int nFrame);
	virtual string GetFileName() { return m_strImageFileList[0]; }

private:
	string ExtractFolderAbsPath( string strFolderPath ) const;
	vector<string> LoadFileList( string strFolderPath ) const;
	vector<string> SelectImageFromFiles( vector<string>& strFileList ) const;
	vector<string> m_strImgExtList;
	vector<string> m_strImageFileList;
};


//* @class  : CVideoInfo
//* @author : Hyoungrae Kim
//* @date : 2015-03-30
class CVideoSeq : public CDBSequence
{
public:
	CVideoSeq() { }
	virtual ~CVideoSeq() { UnloadDB(); }
	virtual bool LoadDB(string);
	virtual void UnloadDB();
	virtual bool ReadFrame(Mat& img, int nFrame);
	virtual string GetFileName() { return m_strVideoFile; }

private:
	void ExtractAbsPathAndFileName( string strFileName, string& strAbsPath, string& strVideoFile );
	string m_strVideoFile;
	VideoCapture m_capVid;
};