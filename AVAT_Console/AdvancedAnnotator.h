#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#include "VideoPlayer.h"
#include "tinyxml.h"

class CGroundTruth;
class CObjectInfo;

class CAdvancedAnnotator : public CVideoPlayer
{
public:
	CAdvancedAnnotator(void);
	virtual ~CAdvancedAnnotator(void);

	// XML control
	bool LoadXML( string strFileName );
	void LoadXmlDoc( string strName );
	void EnrollXmlRoot();
	TiXmlElement* EnrollXmlChild( TiXmlElement* pParent, string strChildName );
	void ReadOnceXmlObjectTypeInfo();
	void ReadOnceXmlFrameInfo();
	void SyncXmlFrameInfo();
	void ApplyNewObjectToXml(const int knIdx);
	void ApplyModifyObjectToXml(const int knIdx);
	void ApplyDeleteObjectToXml(const int knIdx);

	TiXmlDocument* m_pXMLDoc;
	TiXmlElement* m_pXMLRoot;
	TiXmlElement* m_pXMLSequence;
	TiXmlElement* m_pXMLObjectType;
	TiXmlElement* m_pXMLFrameInfo;
	string m_strXMLFileName;
	vector<CObjectInfo> m_vecType;

	// GT writing into the map
	virtual bool GoToSpecificFrame(int nSpecFrame);
	void LoadFrame(int nSpecFrame);

	map<int, vector<CGroundTruth> > m_mapGT;
	vector<CGroundTruth>* m_pvecGTSaved;
	bool m_flgVecAlloc;
	size_t m_idxSelectedRect;

	// Mouse & Keyboard Event
	static void CallbackMouse( int myEvent, int x, int y, int keyFlag, void* param);
	inline void SetDrawMouseLine(bool val) { m_bMouseLine = val; }
	void EventMouseDown(int x, int y, int keyFlag);
	void EventMouseMove(int x, int y, int keyFlag);
	void EventMouseRelease(int x, int y, int keyFlag);
	void EventKeyDelete();
	void EventKeyArrow(char ch);
	void EventKeyNumpad( int num );
	void DrawMouseLine(Mat& img);
	void DrawRects(Mat& img);
	Point2i m_ptMouseLine[4];
	bool m_bMouseLine;
	bool m_bMouseEvent;
	Point2i m_ptMousePress;
	Point2i m_ptMouseMove;
	Point2i m_ptMouseRelease;
	Rect_<int> m_rectDrawing;

	bool ShowObjectTypes();
	void AddObjectTypes(string strType, string strClass, string strSubClass="");
	void SelectObjectTypes();
};

class CGroundTruth
{
public:
	CGroundTruth() { Clear(); }
	CGroundTruth(Rect_<int> roi) { Clear(); rectROI=roi; }
	~CGroundTruth() { }
	inline void Clear()
	{
		Id = -1;
		uniqueID = -1;
		strObjType.clear();
		strObjClass.clear();
		strObjSubClass.clear();
		rectROI = Rect_<int>(-1,-1,0,0);
		eCondition = GT_NEW;
	}
	inline bool CompareSamePos(CGroundTruth objGT)
	{
		int cap = (objGT.rectROI & rectROI).area();
		int cup = (objGT.rectROI | rectROI).area();
		return cap==cup;
	}
	inline void operator=(CGroundTruth& copyFrom)
	{
		Id = copyFrom.Id;
		uniqueID = copyFrom.uniqueID;
		strObjType = copyFrom.strObjType;
		strObjClass = copyFrom.strObjClass;
		strObjSubClass = copyFrom.strObjSubClass;
		rectROI = copyFrom.rectROI;
		eCondition = copyFrom.eCondition;
	}

	int Id;
	int uniqueID;
	string strObjType;
	string strObjClass;
	string strObjSubClass;
	Rect_<int> rectROI;

	enum {GT_NEW=0, GT_SAVE, GT_MODIFY, GT_DELETE};
	int eCondition;
};

class CObjectInfo
{
public:
	CObjectInfo() {}
	~CObjectInfo() {}

	string strType;
	vector<string> strClass;
	vector<Range> rangeSubClass;
	vector<string> strSubClass;
};